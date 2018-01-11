/*
 *  $Id: chbeager.c,v 1.2 1998/02/17 20:44:17 gropp Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
/* flow.h includs the optional flow control for eager delivery */
#include "flow.h"

/*
   Blocking, eager send/recv.
   These are ALWAYS for long messages.  Short messages are always
   handled in eager mode.
 */

/* Prototype definitions */
int MPID_CH_Eagerb_send ANSI_ARGS(( void *, int, int, int, int, int, 
				    MPID_Msgrep_t ));
int MPID_CH_Eagerb_isend ANSI_ARGS(( void *, int, int, int, int, int, 
				     MPID_Msgrep_t, MPIR_SHANDLE * ));
int MPID_CH_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_CH_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_CH_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_CH_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
int MPID_CH_Eagerb_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
void MPID_CH_Eagerb_delete ANSI_ARGS(( MPID_Protocol * ));
/*
 * Definitions of the actual functions
 */
int MPID_CH_Eagerb_send( buf, len, src_lrank, tag, context_id, dest,
			 msgrep )
void *buf;
int  len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
{
    int              pkt_len;
    MPID_PKT_LONG_T  pkt;
    
#ifdef MPID_FLOW_CONTROL
    while (!MPID_FLOW_MEM_OK(len,dest)) {
	/* Wait for a flow packet */
#ifdef MPID_DEBUG_ALL
	if (MPID_DebugFlag || MPID_DebugFlow) {
	    FPRINTF( MPID_DEBUG_FILE, 
		     "[%d] S Waiting for flow control packet from %d\n",
		     MPID_MyWorldRank, dest );
	}
#endif
	MPID_DeviceCheck( MPID_BLOCKING );
    }
    MPID_FLOW_MEM_SEND(len,dest);
#endif
    pkt.mode	   = MPID_PKT_LONG;
    pkt_len	   = sizeof(MPID_PKT_LONG_T); 
    pkt.context_id = context_id;
    pkt.lrank	   = src_lrank;
    pkt.tag	   = tag;
    pkt.len	   = len;
    MPID_DO_HETERO(pkt.msgrep = (int)msgrep);
    MPID_FLOW_MEM_ADD(&pkt,dest);
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",&pkt)

    MPID_PKT_PACK( &pkt, sizeof(MPID_PKT_HEAD_T), dest );

    /* Send as packet only */
    /* It is important to call drain_tiny(1) because, even though this is
       a blocking send, it might still block if some other process is
       not receiving.  In general, we really need a way to force 
       processes to unclog their channels.  On some systems, like the
       IBM SPx, this is impossible (without an unacceptable 
       performance burden). */
    MPID_DRAIN_INCOMING_FOR_TINY(1);
    MPID_SendControlBlock( &pkt, pkt_len, dest );

    /* Send the body of the message */
    MPID_SendChannel( buf, len, dest );

    return MPI_SUCCESS;
}

/*
 * This is the routine called when a packet of type MPID_PKT_LONG is
 * seen.  It receives the data as shown (final interface not set yet)
 */
int MPID_CH_Eagerb_recv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_LONG_T   *pkt = (MPID_PKT_LONG_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS;

    msglen = pkt->len;

    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->msgrep);
    MPID_FLOW_MEM_GET(pkt,from);
    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    MPID_FLOW_MEM_READ(msglen,from);
    MPID_FLOW_MEM_RECV(msglen,from);
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_ERROR = err;
    /* source/tag? */
    MPID_RecvFromChannel( rhandle->buf, msglen, from );
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    rhandle->is_complete = 1;
    
    return err;
}

/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_CH_Eagerb_unxrecv_start( rhandle, in_runex )
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
    MPID_FLOW_MEM_RECV(msglen,runex->from);
    MPID_DO_HETERO(rhandle->msgrep = runex->msgrep);
    rhandle->s		 = runex->s;
    rhandle->s.count     = msglen;
    rhandle->s.MPI_ERROR = err;
    MPID_RecvFree( runex );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );

    return err;
}

/* Save an unexpected message in rhandle */
int MPID_CH_Eagerb_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_T *pkt = (MPID_PKT_T *)in_pkt;

    rhandle->s.MPI_TAG	  = pkt->head.tag;
    rhandle->s.MPI_SOURCE = pkt->head.lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->head.len;
    rhandle->from         = from; /* Needed for flow control */
    rhandle->is_complete  = 1;
    /* Need to save msgrep for heterogeneous systems */
    MPID_DO_HETERO(rhandle->msgrep = (MPID_Msgrep_t)pkt->head.msgrep);
    if (pkt->head.len > 0) {
	rhandle->start	  = (void *)MALLOC( pkt->head.len );
	rhandle->is_complete  = 1;
	if (!rhandle->start) {
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	    /* This is really pretty fatal, because we haven't received
	       the actual message, leaving it in the system */
	    return 1;
	}
	MPID_FLOW_MEM_READ(pkt->head.len,from);
	MPID_RecvFromChannel( rhandle->start, pkt->head.len, from );
    }
    rhandle->push = MPID_CH_Eagerb_unxrecv_start;
    return 0;
}

int MPID_CH_Eagerb_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle )
void *buf;
int  len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
{
    int mpi_errno;
    shandle->is_complete = 1;
    mpi_errno = MPID_CH_Eagerb_send( buf, len, src_lrank, tag, context_id, 
				     dest, msgrep );
    if (shandle->finish) 
	(shandle->finish)( shandle );
    return mpi_errno;
}

int MPID_CH_Eagerb_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    return 0;
}

/* This routine is called when a message arrives and was expected */
int MPID_CH_Eagerb_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_LONG_T *pkt = (MPID_PKT_LONG_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS;

    msglen = pkt->len;

    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    MPID_FLOW_MEM_GET(pkt,from);
    MPID_FLOW_MEM_READ(msglen,from);
    MPID_FLOW_MEM_RECV(msglen,from);
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;
    MPID_RecvFromChannel( rhandle->buf, msglen, from );
    if (rhandle->finish)
	(rhandle->finish)( rhandle );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    
    return err;
}


#ifdef FOO
/* There is common code to de-list an unmatched message */
int MPID_CH_Eagerb_cancel_recv( )
{
return 0;
}

int MPID_CH_Eagerb_test_send( )
{
    return 1;
}

int MPID_CH_Eagerb_wait_send( )
{
    return 1;
}

/* Either it is already present, or it isn't here */
int MPID_CH_Eagerb_test_recv( )
{
    return 0;
}

/* This code should do what ? */
int MPID_CH_Eagerb_wait_recv( )
{
    return 0;
}

#endif

void MPID_CH_Eagerb_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_CH_Eagerb_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_CH_Eagerb_send;
    p->recv	   = MPID_CH_Eagerb_recv;
    p->isend	   = MPID_CH_Eagerb_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = MPID_CH_Eagerb_cancel_send;
    p->irecv	   = MPID_CH_Eagerb_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_CH_Eagerb_save;
    p->delete      = MPID_CH_Eagerb_delete;

    return p;
}
