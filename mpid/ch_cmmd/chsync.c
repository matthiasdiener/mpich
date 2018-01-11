/*
 *  $Id: chsync.c,v 1.13 1995/01/07 20:03:41 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vc[] = "$Id: chsync.c,v 1.13 1995/01/07 20:03:41 gropp Exp $";
#endif

#include "mpid.h"


static int DebugFlag = 0;

void MPID_SetSyncDebugFlag( ctx, f )
void *ctx;
int f;
{
DebugFlag = f;
}

/*
   This file contains routines to keep track of synchronous send messages;
   they must be explicitly acknowledged.

   Heterogeneous systems use integer id's; homogeneous systems just use the
   address of the dmpi_send_handle.
 */

/* Define MPID_TEST_SYNC to test the code that uses the addresses of the
   dmpi handles */
#define MPID_TEST_SYNC
#if defined(MPID_HAS_HETERO) && !defined(MPID_TEST_SYNC)
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

MPID_Aint MPID_CMMD_Get_Sync_Id( dmpi_handle, mpid_handle )
MPIR_SHANDLE *dmpi_handle;
MPID_SHANDLE *mpid_handle;
{
MPID_SyncId *new;
int         id = CurId;

new		      = NEW(MPID_SyncId);    /* This should use SBalloc code */
if (!new) MPID_CMMD_Abort( MPI_ERR_EXHAUSTED );
new->id		      = CurId++;
new->dmpi_send_handle = dmpi_handle;
new->mpid_send_handle = mpid_handle;
new->next	      = head;
head		      = new;

return id;
}

int MPID_CMMD_Lookup_SyncAck( sync_id, dmpi_send_handle, mpid_send_handle )
int          sync_id;
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
int         sync_id;
int         from;
{
MPIR_SHANDLE *dmpi_send_handle;
MPID_SHANDLE *mpid_send_handle;

/* This is an acknowledgement of a synchronous send; look it up and
   mark as completed */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
    "[%d]SYNC received sync ack message for mode=%x from %d (%s:%d)\n",  
	   MPID_MyWorldRank, sync_id, from, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
MPID_CMMD_Lookup_SyncAck( sync_id, &dmpi_send_handle, &mpid_send_handle );
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
dmpi_send_handle->completed = MPIR_YES;
return MPI_SUCCESS;
}

/* return an acknowledgment */
void MPID_SyncReturnAck( sync_id, from )
MPID_Aint sync_id;
int       from;
{
MPID_PKT_SYNC_ACK_T  pkt;

pkt.mode       = MPID_PKT_SYNC_ACK;
pkt.sync_id    = sync_id;

/* Rest of packet is irrelavent */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
   "[%d]SYNC Starting a send of tag = %d, dest = %d, mode=",
	    MPID_MyWorldRank, MPID_PT2PT_TAG, from );
    MPID_Print_mode( stdout, (MPID_PKT_T *)&pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    MPID_Print_packet( stdout, (MPID_PKT_T *)&pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
MPID_SendControl( &pkt, sizeof(MPID_PKT_SYNC_ACK_T), from );
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

#else
MPID_Aint MPID_CMMD_Get_Sync_Id( dmpi_handle, mpid_handle )
MPIR_SHANDLE *dmpi_handle;
MPID_SHANDLE *mpid_handle;
{
return (MPID_Aint) dmpi_handle;
}

int MPID_CMMD_Lookup_SyncAck( sync_id, dmpi_send_handle, mpid_send_handle )
MPID_Aint    sync_id;
MPIR_SHANDLE **dmpi_send_handle;
MPID_SHANDLE **mpid_send_handle;
{
*dmpi_send_handle = (MPIR_SHANDLE *)sync_id;
*mpid_send_handle = &((*dmpi_send_handle)->dev_shandle);
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
if (DebugFlag) {
    printf( 
    "[%d]SYNC received sync ack message for mode=%x from %d (%s:%d)\n",  
	   MPID_MyWorldRank, sync_id, from, __FILE__, __LINE__ );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
MPID_CMMD_Lookup_SyncAck( sync_id, &dmpi_send_handle, &mpid_send_handle );
if (!dmpi_send_handle) {
    fprintf( stderr, "Error in processing sync ack!\n" );
    return MPI_ERR_INTERN;
    }
dmpi_send_handle->completed = MPIR_YES;
return MPI_SUCCESS;
}

/* return an acknowledgment */
void MPID_SyncReturnAck( sync_id, from )
MPID_Aint sync_id;
int       from;
{
MPID_PKT_SYNC_ACK_T  pkt;

pkt.mode       = MPID_PKT_SYNC_ACK;
pkt.sync_id    = sync_id;

/* Rest of packet is irrelavent */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (DebugFlag) {
    printf( 
   "[%d]SYNC Starting a send of tag = %d, dest = %d, mode=",
	    MPID_MyWorldRank, MPID_PT2PT_TAG, from );
    MPID_Print_mode( stdout, &pkt );
    fprintf( stdout, "(%s:%d)\n", __FILE__, __LINE__ );
    MPID_Print_packet( stdout, &pkt );
    fflush( stdout );
    }
#endif                  /* #DEBUG_END# */
MPID_SendControl( &pkt, sizeof(MPID_PKT_SYNC_ACK_T), from );
}

/* Look through entire list for this API handle */
void MPID_Sync_discard( dmpi )
MPIR_SHANDLE *dmpi;
{
/* Do something to indicated cancelled. */
}
#endif
