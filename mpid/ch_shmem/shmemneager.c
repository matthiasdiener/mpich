#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"

/*
   Nonblocking, eager shared-memory send/recv.
 */

/* Prototype definitions */
int MPID_SHMEM_Eagern_send ANSI_ARGS(( void *, int, int, int, int, int, 
				       MPID_Msgrep_t ));
int MPID_SHMEM_Eagern_isend ANSI_ARGS(( void *, int, int, int, int, int, 
					MPID_Msgrep_t, MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_cancel_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_wait_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_test_send ANSI_ARGS(( MPIR_SHANDLE * ));
int MPID_SHMEM_Eagern_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
int MPID_SHMEM_Eagern_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));
void MPID_SHMEM_Eagern_delete ANSI_ARGS(( MPID_Protocol * ));

/* 
 * Blocking operations come from chbeager.c
 */
extern int MPID_SHMEM_Eagerb_send ANSI_ARGS(( void *, int, int, int, int, 
					   int, MPID_Msgrep_t ));
extern int MPID_SHMEM_Eagerb_recv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_irecv ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_save ANSI_ARGS(( MPIR_RHANDLE *, int, void * ));
extern int MPID_SHMEM_Eagerb_unxrecv_start ANSI_ARGS(( MPIR_RHANDLE *, void * ));

/*
 * Definitions of the actual functions
 */

int MPID_SHMEM_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
			 msgrep, shandle )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
MPIR_SHANDLE *shandle;
{
    int                       pkt_len;
    MPID_PKT_SEND_ADDRESS_T   *pkt;
    
    pkt = (MPID_PKT_SEND_ADDRESS_T *)MPID_SHMEM_GetSendPkt(0);
    /* GetSendPkt hangs untill successful */

    pkt->mode	    = MPID_PKT_SEND_ADDRESS;
    pkt->context_id = context_id;
    pkt->lrank	    = src_lrank;
    pkt->tag	    = tag;
    pkt->len	    = len;
	
    DEBUG_PRINT_SEND_PKT("S Sending extra-long message",pkt);

    /* Place in shared memory */
    pkt->address = MPID_SetupGetAddress( buf, &len, dest );
    MEMCPY( pkt->address, buf, len );

    /* Send as packet only */
    MPID_SHMEM_SendControl( pkt, sizeof(MPID_PKT_SEND_ADDRESS_T), dest );

    shandle->wait	 = 0;
    shandle->test	 = 0;
    shandle->is_complete = 1;

    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_send( buf, len, src_lrank, tag, context_id, dest,
			    msgrep )
void          *buf;
int           len, tag, context_id, src_lrank, dest;
MPID_Msgrep_t msgrep;
{
    MPIR_SHANDLE shandle;

    DEBUG_INIT_STRUCT(&shandle,sizeof(shandle));
    MPIR_SET_COOKIE((&shandle),MPIR_REQUEST_COOKIE)
    MPID_SHMEM_Eagern_isend( buf, len, src_lrank, tag, context_id, dest,
			     msgrep, &shandle );
    /* Note that isend is (probably) complete */
    if (!shandle.is_complete) {
	DEBUG_TEST_FCN(shandle.wait,"req->wait");
	shandle.wait( &shandle );
    }
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_cancel_send( shandle )
MPIR_SHANDLE *shandle;
{
    return 0;
}

int MPID_SHMEM_Eagern_test_send( shandle )
MPIR_SHANDLE *shandle;
{
    /* Test for completion */
    return MPI_SUCCESS;
}

int MPID_SHMEM_Eagern_wait_send( shandle )
MPIR_SHANDLE *shandle;
{
    return MPI_SUCCESS;
}

/*
 * This is the routine called when a packet of type MPID_PKT_SEND_ADDRESS is
 * seen.  It receives the data as shown (final interface not set yet)
 */
int MPID_SHMEM_Eagern_recv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T   *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS;

    msglen = pkt->len;

    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    rhandle->s.count	 = msglen;
    rhandle->s.MPI_ERROR = err;
    /* source/tag? */
    MEMCPY( rhandle->buf, pkt->address, msglen );
    MPID_FreeGetAddress( pkt->address );
    if (rhandle->finish) {
	(rhandle->finish)( rhandle );
    }
    MPID_SHMEM_FreeRecvPkt( (MPID_PKT_T *)pkt );
    rhandle->is_complete = 1;
    
    return err;
}

/* This routine is called when a message arrives and was expected */
int MPID_SHMEM_Eagern_irecv( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;
    int    msglen, err = MPI_SUCCESS;

    msglen = pkt->len;

    /* Check for truncation */
    MPID_CHK_MSGLEN(rhandle,msglen,err)
    /* Note that if we truncate, We really must receive the message in two 
       parts; the part that we can store, and the part that we discard.
       This case is not yet handled. */
    rhandle->s.count	  = msglen;
    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = err;
    MEMCPY( rhandle->buf, pkt->address, msglen );
    MPID_FreeGetAddress( pkt->address );
    if (rhandle->finish)
	(rhandle->finish)( rhandle );
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;

    MPID_SHMEM_FreeRecvPkt( (MPID_PKT_T *)pkt );
    
    return err;
}

/* Save an unexpected message in rhandle */
int MPID_SHMEM_Eagern_save( rhandle, from, in_pkt )
MPIR_RHANDLE *rhandle;
int          from;
void         *in_pkt;
{
    MPID_PKT_SEND_ADDRESS_T *pkt = (MPID_PKT_SEND_ADDRESS_T *)in_pkt;

    rhandle->s.MPI_TAG	  = pkt->tag;
    rhandle->s.MPI_SOURCE = pkt->lrank;
    rhandle->s.MPI_ERROR  = 0;
    rhandle->s.count      = pkt->len;
    rhandle->is_complete  = 0;
    /* Save the address */
    rhandle->start        = pkt->address;
    MPID_SHMEM_FreeRecvPkt( (MPID_PKT_T *)pkt );
    rhandle->push = MPID_SHMEM_Eagern_unxrecv_start;
    return 0;
}
/* 
 * This routine is called when it is time to receive an unexpected
 * message
 */
int MPID_SHMEM_Eagern_unxrecv_start( rhandle, in_runex )
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
	MPID_FreeGetAddress( runex->start );
    }
    MPID_RecvFree( runex );
    rhandle->s		 = runex->s;
    rhandle->wait	 = 0;
    rhandle->test	 = 0;
    rhandle->push	 = 0;
    rhandle->is_complete = 1;
    if (rhandle->finish) 
	(rhandle->finish)( rhandle );

    return err;
}

void MPID_SHMEM_Eagern_delete( p )
MPID_Protocol *p;
{
    FREE( p );
}

MPID_Protocol *MPID_SHMEM_Eagern_setup()
{
    MPID_Protocol *p;

    p = (MPID_Protocol *) MALLOC( sizeof(MPID_Protocol) );
    if (!p) return 0;
    p->send	   = MPID_SHMEM_Eagern_send;
    p->recv	   = MPID_SHMEM_Eagern_recv;
    p->isend	   = MPID_SHMEM_Eagern_isend;
    p->wait_send   = 0;
    p->push_send   = 0;
    p->cancel_send = MPID_SHMEM_Eagern_cancel_send;
    p->irecv	   = MPID_SHMEM_Eagern_irecv;
    p->wait_recv   = 0;
    p->push_recv   = 0;
    p->cancel_recv = 0;
    p->do_ack      = 0;
    p->unex        = MPID_SHMEM_Eagern_save;
    p->delete      = MPID_SHMEM_Eagern_delete;

    return p;
}