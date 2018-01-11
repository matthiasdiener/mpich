/*
 *  $Id: chshort.c,v 1.1.1.1 1997/09/17 20:39:19 gropp Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"

/* Prototype definitions */
int MPID_CH_Eagerb_send_short ANSI_ARGS(( void *, int, int, int, int, int, 
					  MPID_Msgrep_t ));
int MPID_CH_Eagerb_isend_short ANSI_ARGS(( void *, int, int, int, int, int, 
					   MPID_Msgrep_t, MPIR_SHANDLE * ));
int MPID_CH_Eagerb_recv_short ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_CH_Eagerb_save_short ANSI_ARGS(( MPIR_RHANDLE *, int, void *));
int MPID_CH_Eagerb_unxrecv_start_short ANSI_ARGS(( MPIR_RHANDLE *, void * ));
void MPID_CH_Eagerb_short_delete ANSI_ARGS(( MPID_Protocol * ));
/*
 * Definitions of the actual functions
 */

int MPID_CH_Eagerb_send_short( buf, len, src_lrank, tag, context_id, dest,
			       msgrep )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
{
    MPID_PKT_SHORT_T pkt;

    /* These references are ordered to match the order they appear in the 
       structure */
    pkt.mode	   = MPID_PKT_SHORT;
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_DO_HETERO(pkt.msgrep = (int)msgrep);

    DEBUG_PRINT_SEND_PKT("S Sending",&pkt);

    MPID_PKT_PACK( &pkt, sizeof(MPID_PKT_HEAD_T), dest );
    
    if (len > 0) {
	MEMCPY( pkt.buffer, buf, len );
	DEBUG_PRINT_PKT_DATA("S Getting data from buf",&pkt);
    }
    /* Always use a blocking send for short messages.
       (May fail with systems that do not provide adequate
       buffering.  These systems should switch to non-blocking sends)
     */
    DEBUG_PRINT_SEND_PKT("S Sending message in a single packet",&pkt);

    /* In case the message is marked as non-blocking, indicate that we don't
       need to wait on it.  We may also want to use nonblocking operations
       to send the envelopes.... */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    MPID_SendControlBlock( &pkt, len + sizeof(MPID_PKT_HEAD_T), dest );
    DEBUG_PRINT_MSG("S Sent message in a single packet");

    return MPI_SUCCESS;
}

int MPID_CH_Eagerb_isend_short( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
{
    int mpi_errno;
    shandle->is_complete = 1;
    mpi_errno = MPID_CH_Eagerb_send_short( buf, len, src_lrank, tag, 
					   context_id, dest, msgrep );
    /* Instead of this, the calling code should test from not-complete,
       and set finish if needed */
#ifdef FOO
    if (shandle->finish) 
	(shandle->finish)( shandle );
#endif
    return mpi_errno;
}

int MPID_CH_Eagerb_recv_short( rhandle, from_grank, in_pkt )
MPIR_RHANDLE *rhandle;
int          from_grank;
void         *in_pkt;
{
    MPID_PKT_SHORT_T *pkt = (MPID_PKT_SHORT_T *)in_pkt;
    int          msglen;
    int          err = MPI_SUCCESS;
    
    msglen		  = pkt->len;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    if (msglen > 0) {
	MEMCPY( rhandle->buf, pkt->buffer, msglen ); 
    }
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_ERROR  = err;
    if (rhandle->finish) {
	MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
	(rhandle->finish)( rhandle );
    }
    rhandle->is_complete = 1;

    return err;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_CH_Eagerb_unxrecv_start_short( rhandle, in_runex )
MPIR_RHANDLE *rhandle;
void         *in_runex;
{
    MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
    int          msglen, err = 0;

    msglen = runex->s.count;
    MPID_CHK_MSGLEN(rhandle,msglen,err);
    /* Copy the data from the local area and free that area */
    if (runex->s.count > 0) {
	MEMCPY( rhandle->buf, runex->start, msglen );
	FREE( runex->start );
    }
    MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
    rhandle->s		 = runex->s;
    rhandle->s.count     = msglen;
    rhandle->s.MPI_ERROR = err;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );
    MPID_RecvFree( runex );

    return err;
}

/* Save an unexpected message in rhandle */
int MPID_CH_Eagerb_save_short( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SHORT_T   *pkt = (MPID_PKT_SHORT_T *)in_pkt;

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 1;
    /* Need to save msgrep for heterogeneous systems */
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
    if (pkt->len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->len );
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    return 1;
	}
	MEMCPY( rhandle->start, pkt->buffer, pkt->len );
    }
    rhandle->push = MPID_CH_Eagerb_unxrecv_start_short;
    return 0;
}

void MPID_CH_Eagerb_short_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_CH_Short_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_CH_Eagerb_send_short;
    p->recv	   = MPID_CH_Eagerb_recv_short;
    p->isend	   = MPID_CH_Eagerb_isend_short;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = 0;
    p->irecv	   = 0;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_CH_Eagerb_save_short;
    p->delete      = MPID_CH_Eagerb_short_delete;

    return p;
}
