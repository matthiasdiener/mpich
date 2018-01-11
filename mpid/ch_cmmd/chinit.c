/*
 *  $Id: chinit.c,v 1.28 1995/02/06 22:12:37 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#if defined(rs6000) && !defined(_ALL_SOURCE)
/* AIX's version of netinet/in.h REQUIRES but DOES NOT CHECK 
   for _ALL_SOURCE! 
   (This is needed below incase sys/types.h is loaded in these includes)
 */
#define _ALL_SOURCE
#endif

#include "mpid.h"
#include "system/system.h"
/* #CMMD DECLARATION# */

MPID_INFO *MPID_procinfo = 0;
MPID_H_TYPE MPID_byte_order;
int MPID_IS_HETERO = 0;
void (*MPID_ErrorHandler)() = MPID_DefaultErrorHandler;

/* For tracing channel operations by ADI underlayer */
FILE *MPID_TRACE_FILE = 0;

/* For debugging statements */
FILE *MPID_DEBUG_FILE = 0;

#ifdef MPID_PKT_VAR_SIZE
int MPID_PKT_DATA_SIZE = MPID_PKT_MAX_DATA_SIZE;
int MPID_SetPktSize( len )
int len;
{
if (len < 0) return MPID_PKT_MAX_DATA_SIZE;
if (len > MPID_PKT_MAX_DATA_SIZE) len = MPID_PKT_MAX_DATA_SIZE;
MPID_PKT_DATA_SIZE = len;
return len;
}
#else
int MPID_SetPktSize( len )
int len;
{
return MPID_PKT_MAX_DATA_SIZE;
}
#endif

/* #define DEBUG(a) {a} */
#define DEBUG(a)

static int DebugSpace = 0;
int MPID_DebugFlag = 0;

MPID_SetSpaceDebugFlag( flag )
int flag;
{
DebugSpace = flag;
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
if (flag) {
    /* Check the validity of the malloc arena on every use of trmalloc/free */
    trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}
void MPID_SetDebugFlag( ctx, f )
void *ctx;
int f;
{
MPID_DebugFlag = f;
}
void MPID_Set_tracefile( name )
char *name;
{
char filename[1024];

if (strcmp( name, "-" ) == 0) {
    MPID_TRACE_FILE = stdout;
    return;
    }
if (strchr( name, '%' )) {
    sprintf( filename, name, MPID_MyWorldRank );
    MPID_TRACE_FILE = fopen( filename, "w" );
    }
else
    MPID_TRACE_FILE = fopen( name, "w" );

/* Is this the correct thing to do? */
if (!MPID_TRACE_FILE)
    MPID_TRACE_FILE = stdout;
}


#ifndef MPID_STAT_NONE
int MPID_n_short       = 0,         /* short messages */
    MPID_n_long        = 0,         /* long messages */
    MPID_n_unexpected  = 0,         /* unexpected messages */
    MPID_n_syncack     = 0;         /* Syncronization acknowledgments */
#endif

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are out-standing                                      */
/***************************************************************************/
int MPID_n_pending     = 0;         /* Number of uncompleted split requests */

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/
void MPID_CMMD_Myrank( rank )
int *rank;
{
*rank = MPID_MyWorldRank;
}

void MPID_CMMD_Mysize( size )
int *size;
{
*size = MPID_WorldSize;
}

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    This version currently returns null, as all data is static.
 */
void *MPID_CMMD_Init( argc, argv )
int  *argc;
char ***argv;
{
#ifdef MPID_HAS_HETERO     /* #HETERO_START# */
int  i;
char *work;
#endif                     /* #HETERO_END# */

/* Set the file for Debugging output.  The actual output is controlled
   by MPIDDebugFlag */
if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;

PIiInit( argc, argv );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Finished init\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

/* Turn off the resource monitors */
#if !defined(euih) && !defined(eui)
/* If we are euih or SP2 eui, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
SYSetResourceClockOff();
#endif

#ifdef MPID_HAS_HETERO           /* #HETERO_START# */
/* Eventually, this will also need to check for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,
              "[%d] Checking for heterogeneous systems...\n", 
	      MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)MALLOC( MPID_WorldSize * sizeof(MPID_INFO) );
CHKPTRN(MPID_procinfo);
for (i=0; i<MPID_WorldSize; i++) {
    MPID_procinfo[i].byte_order = MPID_H_NONE;
    }
/* Set my byte ordering and convert if necessary.  Eventually, this
   should use xdr */
i = SYGetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Byte order is %d\n",MPID_MyWorldRank, i );)
#endif                  /* #DEBUG_END# */
if (i == 1)      MPID_byte_order = MPID_H_LSB;
else if (i == 2) MPID_byte_order = MPID_H_MSB;
else             MPID_byte_order = MPID_H_XDR;
MPID_procinfo[MPID_MyWorldRank].byte_order = MPID_byte_order;
/* Everyone uses the same format (MSB) */
if (i == 1) 
    SYByteSwapInt( (int*)&MPID_procinfo[MPID_MyWorldRank].byte_order, 1 );

/* Get everyone else's */
work = (char *)MALLOC( MPID_WorldSize * sizeof(MPID_INFO) );
CHKPTRN( work );
/* ASSUMES MPID_INFO is ints */
PIgimax( MPID_procinfo, MPID_WorldSize, work, PSAllProcs );
FREE( work );

/* See if they are all the same */
MPID_IS_HETERO = 0;
for (i=1; i<MPID_WorldSize; i++) {
    if (MPID_procinfo[0].byte_order != MPID_procinfo[i].byte_order) {
    	MPID_IS_HETERO = 1;
    	break;
        }
    }
#endif                  /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_CMMD_Init_recv_code();
MPID_CMMD_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] leaving chinit\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_CMMD_Abort( code )
int code;
{
fprintf( stderr, "Aborting program!\n" );
fflush( stderr );
fflush( stdout );
SYexitall( (char *)0, code );
}

void MPID_CMMD_End()
{
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d] Entering MPID_End\n", MPID_MyWorldRank );
    }
#endif                  /* #DEBUG_END# */
/* Finish off any pending transactions */
MPID_CMMD_Complete_pending();

if (MPID_GetMsgDebugFlag()) {
    MPID_PrintMsgDebug();
    }
if (MPID_procinfo) 
    FREE( MPID_procinfo );
#ifdef CHAMELEON_COMM       /* #CHAMELEON_START# */
if (DebugSpace)
    trdump( stdout );
#endif                      /* #CHAMELEON_END# */
/* We should really generate an error or warning message if there 
   are uncompleted operations... */
PIiFinish();
}

void MPID_CMMD_Node_name( name, len )
char *name;
int  len;
{
PINodeName( name, len );
}

void MPID_CMMD_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

double MPID_CMMD_Wtime()
{
return SYGetElapsedTime();
}

/* This returns a value that is correct but not the best value that
   could be returned */
double MPID_CMMD_Wtick()
{
double t1, t2;
int    cnt;

cnt = 1000;
t1  = MPID_CMMD_Wtime();
while (cnt-- && (t2 = MPID_CMMD_Wtime()) <= t1) ;
if (!cnt) return 1.0e6;
return t2 - t1;
}

void MPID_CMMD_Error_handler( r )
void (*r)();
{
if (r)
    MPID_ErrorHandler = r;
else
    MPID_ErrorHandler = MPID_DefaultErrorHandler;
}

void MPID_DefaultErrorHandler( code, str )
int  code;
char *str;
{
if (str) 
    fprintf( stderr, "%s\n", str );
MPID_CMMD_Abort( code );
}

int MPID_CMMD_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}


#ifdef MPID_HAS_HETERO       /* #HETERO_START# */
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
MPID_CMMD_Pkt_pack( pkt, size, dest )
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
	for (i=0; i<size/sizeof(int); i++) {
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
	SYByteSwapInt( (int*)pkt, size / sizeof(int) );
	}
    }
}
MPID_CMMD_Pkt_unpack( pkt, size, from )
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
	for (i=0; i<size/sizeof(int); i++) {
	    *d = ntohl(*d);
	    d++;
	    }
	}
    }
}
#endif                       /* #HETERO_END# */


/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_CMMD_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
int i; char *aa = (char *)address;

if (msg)
    printf( "[%d]%s\n", MPID_MyWorldRank, msg );
if (len < 78 && address) {
    for (i=0; i<len; i++) {
	printf( "%x", aa[i] );
	}
    printf( "\n" );
    }
fflush( stdout );
}
#endif                  /* #DEBUG_END# */

/*
   Data about messages
 */
static int DebugMsgFlag = 0;
void MPID_SetMsgDebugFlag( f )
int f;
{
DebugMsgFlag = f;
}
int MPID_GetMsgDebugFlag()
{
return DebugMsgFlag;
}
void MPID_PrintMsgDebug()
{
#ifndef MPID_STAT_NONE
fprintf( stdout, "[%d] short = %d, long = %d, unexpected = %d, ack = %d\n",
	 MPID_MyWorldRank, MPID_n_short, MPID_n_long, MPID_n_unexpected, 
	 MPID_n_syncack );
#endif
}
