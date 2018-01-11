/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
#endif

#include "mpid.h"
#include "mpiddebug.h"

/* #define DEBUG(a) {a} */
#define DEBUG(a)

MPID_INFO *MPID_procinfo = 0;
MPID_H_TYPE MPID_byte_order;
static char *(ByteOrderName[]) = { "None", "LSB", "MSB", "XDR" };
int MPID_IS_HETERO = 0;

int MPID_CH_init_hetero( argc, argv )
int  *argc;
char ***argv;
{
int  i, use_xdr;
char *work;

/* Eventually, this will also need to check for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. 
   We can check just the basic signed types: short, int, long, float, double 
   Eventually, we should probably also check the long long and long double
   types.
   We do this by collecting an array of word sizes, with each processor
   contributing the 5 lengths.  This is combined with the "byte order"
   field.

   We still need to identify IEEE and non-IEEE systems.  Perhaps we'll
   just use a field from configure (only CRAY vector systems are non IEEE
   of the systems we interact with).

   We ALSO need to identify Fortran sizes, at least sizeof(REAL), 
   and change the heterogenous code to handle Fortran LOGICALs
 */
/*
   We look for the argument -mpixdr to force use of XDR for debugging and
   timing comparision.
 */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,
              "[%d] Checking for heterogeneous systems...\n", 
	      MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)MALLOC( MPID_WorldSize * sizeof(MPID_INFO) );
CHKPTRN(MPID_procinfo);
for (i=0; i<MPID_WorldSize; i++) {
    MPID_procinfo[i].byte_order	 = MPID_H_NONE;
    MPID_procinfo[i].short_size	 = 0;
    MPID_procinfo[i].int_size	 = 0;
    MPID_procinfo[i].long_size	 = 0;
    MPID_procinfo[i].float_size	 = 0;
    MPID_procinfo[i].double_size = 0;
    MPID_procinfo[i].float_type  = 0;
    }
/* Set my byte ordering and convert if necessary.  */

/* Set the floating point type.  IEEE is 0, Cray is 2, others as we add them
   (MPID_FLOAT_TYPE?) Not yet set: VAX floating point,
   IBM 360/370 floating point, and other. */
#ifdef MPID_FLOAT_CRAY
MPID_procinfo[MPID_MyWorldRank].float_type = 2;
#endif
use_xdr = 0;
/* printf ("Checking args for -mpixdr\n" ); */
for (i=1; i<*argc; i++) {
    /* printf( "Arg[%d] is %s\n", i, (*argv)[i] ? (*argv)[i] : "<NULL>" ); */
    if ((*argv)[i] && strcmp( (*argv)[i], "-mpixdr" ) == 0) {
	/* printf( "Found -mpixdr\n" ); */
	use_xdr = 1;
	break;
	}
    }
if (use_xdr) 
    MPID_byte_order = MPID_H_XDR;
else {
    i = SYGetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Byte order is %d\n",MPID_MyWorldRank, i );)
#endif                  /* #DEBUG_END# */
    if (i == 1)      MPID_byte_order = MPID_H_LSB;
    else if (i == 2) MPID_byte_order = MPID_H_MSB;
    else             MPID_byte_order = MPID_H_XDR;
    }
MPID_procinfo[MPID_MyWorldRank].byte_order  = MPID_byte_order;
MPID_procinfo[MPID_MyWorldRank].short_size  = sizeof(short);
MPID_procinfo[MPID_MyWorldRank].int_size    = sizeof(int);
MPID_procinfo[MPID_MyWorldRank].long_size   = sizeof(long);
MPID_procinfo[MPID_MyWorldRank].float_size  = sizeof(float);
MPID_procinfo[MPID_MyWorldRank].double_size = sizeof(double);

/* Everyone uses the same format (MSB) */
/* This should use network byte order OR the native collective operation 
   with heterogeneous support */
/* if (i == 1) 
    SYByteSwapInt( (int*)&MPID_procinfo[MPID_MyWorldRank].byte_order, 1 );
 */
/* Get everyone else's */
work = (char *)MALLOC( MPID_WorldSize * sizeof(MPID_INFO) );
CHKPTRN( work );
/* ASSUMES MPID_INFO is ints */
PIgimax( MPID_procinfo, 7 * MPID_WorldSize, work, PSAllProcs );
FREE( work );

/* See if they are all the same and different from XDR*/
MPID_IS_HETERO = MPID_procinfo[0].byte_order == MPID_H_XDR;
for (i=1; i<MPID_WorldSize; i++) {
    if (MPID_procinfo[0].byte_order  != MPID_procinfo[i].byte_order ||
	MPID_procinfo[i].byte_order  == MPID_H_XDR ||
	MPID_procinfo[0].short_size  != MPID_procinfo[i].short_size ||
	MPID_procinfo[0].int_size    != MPID_procinfo[i].int_size ||
	MPID_procinfo[0].long_size   != MPID_procinfo[i].long_size ||
	MPID_procinfo[0].float_size  != MPID_procinfo[i].float_size ||
	MPID_procinfo[0].double_size != MPID_procinfo[i].double_size ||
	MPID_procinfo[0].float_type  != MPID_procinfo[i].float_type) {
    	MPID_IS_HETERO = 1;
	break;
        }
    }
/* 
   When deciding to use XDR, we need to check for size as well (if 
   [myid].xxx_size != [j].xxx_size, set [j].byte_order = XDR).  Note that 
   this is reflexive; if j decides that i needs XDR, i will also decide that
   j needs XDR;
 */
if (MPID_IS_HETERO) {
    for (i=0; i<MPID_WorldSize; i++) {
	if (i == MPID_MyWorldRank) continue;
	if (MPID_procinfo[MPID_MyWorldRank].short_size  != 
	    MPID_procinfo[i].short_size ||
	    MPID_procinfo[MPID_MyWorldRank].int_size    != 
	    MPID_procinfo[i].int_size ||
	    MPID_procinfo[MPID_MyWorldRank].long_size   != 
	    MPID_procinfo[i].long_size ||
	    MPID_procinfo[MPID_MyWorldRank].float_size  != 
	    MPID_procinfo[i].float_size ||
	    MPID_procinfo[MPID_MyWorldRank].double_size != 
	    MPID_procinfo[i].double_size ||
	    MPID_procinfo[MPID_MyWorldRank].float_type !=
	    MPID_procinfo[i].float_type) {
	    MPID_procinfo[i].byte_order = MPID_H_XDR;
	    }
	}
    }
#ifdef FOO
if (MPID_IS_HETERO && MPID_MyWorldRank == 0) {
    printf( "Warning: heterogenity only partially supported\n" );
    printf( "Ordering short int long float double sizes float-type\n" );
    for (i=0; i<MPID_WorldSize; i++) {
	printf( "<%d> %s %d %d %d %d %d %d\n", i,
	        ByteOrderName[MPID_procinfo[i].byte_order],
	        MPID_procinfo[i].short_size, 
	        MPID_procinfo[i].int_size, 
	        MPID_procinfo[i].long_size, 
	        MPID_procinfo[i].float_size, 
	        MPID_procinfo[i].double_size,
	        MPID_procinfo[i].float_type );
	}
    }
#endif
return MPI_SUCCESS;
}

int MPID_CH_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}


/* This routine is ensure that the elements in the packet HEADER can
   be read by the receiver without further processing (unless XDR is
   used, in which case we use network byte order)
   This routine is defined ONLY for heterogeneous systems

   Note that different packets have different lengths and layouts; 
   this makes the conversion more troublesome.  I'm still thinking
   about how to do this.
 */
#include <sys/types.h>
#include <netinet/in.h>
/* These need to use 32bit ints.  The 4's here are sizeof(int32) */

void MPID_CH_Pkt_pack( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
int i;
unsigned int *d;
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    
    if (MPID_procinfo[dest].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/4; i++) {
	    *d = htonl(*d);
	    d++;
	    }
	}
    else {
	/* Need to swap to receiver's order.  We ALWAYS reorder at the
	   sender's end (this is because a message can be received with 
	   MPI_Recv instead of MPI_Recv/MPI_Unpack, and hence requires us 
	   to use a format that matches the receiver's ordering without 
	   requiring a user-unpack.  */
	SYByteSwapInt( (int*)pkt, size / 4 );
	}
    }
}

void MPID_CH_Pkt_unpack( pkt, size, from )
MPID_PKT_T *pkt;
int        size, from;
{
int i;
unsigned int *d;
if (MPID_IS_HETERO &&
    MPID_procinfo[from].byte_order != MPID_byte_order) {
    
    if (MPID_procinfo[from].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/4; i++) {
	    *d = ntohl(*d);
	    d++;
	    }
	}
    }
}


int MPID_CH_Hetero_free()
{
if (MPID_procinfo) 
    FREE( MPID_procinfo );
return MPI_SUCCESS;
}

#if !defined(MPID_TEST_SYNC)
typedef struct _MPID_SyncId {
    int id;
    MPIR_SHANDLE *dmpi_send_handle;
    MPID_SHANDLE *mpid_send_handle;
    struct _MPID_SyncId *next;
    } MPID_SyncId;

static MPID_SyncId *head = 0;
static int         CurId = 1;
/* Get a message id for this message and add to the list */
/* 
   KNOWN BUG: With a huge number of messages, we could eventually get rollover
   in the CurId; to manage this, we would need to send informational messages
   to our friends occasionally
 */

MPID_Aint MPID_CH_Get_Sync_Id( dmpi_handle, mpid_handle )
MPIR_SHANDLE *dmpi_handle;
MPID_SHANDLE *mpid_handle;
{
MPID_SyncId *new;
int         id = CurId;

new		      = NEW(MPID_SyncId);    /* This should use SBalloc code */
if (!new) MPID_CH_Abort( MPI_ERR_EXHAUSTED );
new->id		      = CurId++;
new->dmpi_send_handle = dmpi_handle;
new->mpid_send_handle = mpid_handle;
new->next	      = head;
head		      = new;

return id;
}

int MPID_CH_Lookup_SyncAck( sync_id, dmpi_send_handle, mpid_send_handle )
MPID_Aint    sync_id;
MPIR_SHANDLE **dmpi_send_handle;
MPID_SHANDLE **mpid_send_handle;
{
MPID_SyncId *cur, *last;

last		  = 0;
cur		  = head;
*dmpi_send_handle = 0;
*mpid_send_handle = 0;
while (cur) {
    if (cur->id == sync_id) {
	/* Note that the fields may be NULL if this message was cancelled */
	*dmpi_send_handle = cur->dmpi_send_handle;
	*mpid_send_handle = cur->mpid_send_handle;
	if (last) last->next = cur->next;
	else      head       = cur->next;
	/* See if we can easily recover the id */
	if (CurId == sync_id + 1) CurId--;
	FREE( cur );
	return MPI_SUCCESS;
	}
    last = cur;
    cur  = cur->next;
    }
/* Error, did not find id! */
if (!dmpi_send_handle) {
    fprintf( stderr, "Error in processing sync id %x!\n", sync_id );
    }
return MPI_SUCCESS;
}

/* Process a synchronization acknowledgement */
int MPID_SyncAck( sync_id, from )
MPID_Aint   sync_id;
int         from;
{
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;

/* This is an acknowledgement of a synchronous send; look it up and
   mark as completed */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    printf( 
    "[%d]SYNC received sync ack message for mode=%x from %d (%s:%d)\n",  
	   MPID_MyWorldRank, sync_id, from, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
MPID_CH_Lookup_SyncAck( sync_id, &dmpi_send_handle, &mpid_send_handle );
if (!dmpi_send_handle) {
    fprintf( stderr, "Error in processing sync ack!\n" );
    return MPI_ERR_INTERN;
    }
/* Note that the proper way to send synchronous messages when MPID_USE_RNDV is
   set is to use MPID_USE_RNDV and not to use this code at all, thus
   it should always be correct to set the completed fields to YES.
   If, for some reason, a non-blocking send operation was used, if needs
   to be waited on here.
 */
DMPI_mark_send_completed(dmpi_send_handle);
return MPI_SUCCESS;
}

/* return an acknowledgment */
void MPID_SyncReturnAck( sync_id, from )
MPID_Aint sync_id;
int       from;
{
MPID_PKT_SEND_DECL(MPID_PKT_SYNC_ACK_T,pkt);

MPID_PKT_SEND_ALLOC(MPID_PKT_SYNC_ACK_T,pkt);
MPID_PKT_SEND_SET(pkt,mode,MPID_PKT_SYNC_ACK);
MPID_PKT_SEND_SET(pkt,sync_id,sync_id);

DEBUG_PRINT_BASIC_SEND_PKT("SYNC Starting a send",pkt)
MPID_SendControl( MPID_PKT_SEND_ADDR(pkt), sizeof(MPID_PKT_SYNC_ACK_T), from );
}

/* Look through entire list for this API handle */
void MPID_Sync_discard( dmpi )
MPIR_SHANDLE *dmpi;
{
MPID_SyncId *cur;

cur		  = head;
while (cur) {
    if (cur->dmpi_send_handle == dmpi) {
	cur->dmpi_send_handle = 0;
 	cur->mpid_send_handle = 0;
	/* We leave this here just in case a reply is received, so that 
	   we can distinquish between invalid syncAck messages and
	   syncAcks for cancelled messages.  */
	return;
	}
    cur  = cur->next;
    }
/* Error, did not find id! Ignored for now. */
}
#endif
