#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "packets.h"
#include "t3dpriv.h"
#include "t3dshort.h"

int MPID_T3D_Eagerb_send_short(void *buf,
			       int   len,
			       int   src_lrank,
			       int   tag,
			       int   context_id,
			       int   dest,
			       MPID_Msgrep_t msgrep)
{
  T3D_PKT_SHORT_T pkt;
  T3D_PKT_SHORT_T *destpkt;
  int msglen;
  int buf_loc;
  extern MPID_Device *ch_t3d_dev;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_send_short");

  while (t3d_dest_flags[dest] != T3D_BUF_AVAIL)
    MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);

  destpkt = (T3D_PKT_SHORT_T*) &(t3d_recv_bufs[MPID_MyWorldRank]);

  t3d_dest_flags[dest] = T3D_BUF_IN_USE;
  pkt.mode       = T3D_PKT_SHORT;
  pkt.context_id = context_id;
  pkt.lrank      = src_lrank;
  pkt.tag        = tag;
  pkt.len        = len;
  pkt.source     = MPID_MyWorldRank;
  pkt.status     = T3D_BUF_IN_USE;

  if (0 < len)
  {
    buf_loc  = T3D_BUFFER_LENGTH - ( sizeof(short) * T3D_MSG_LEN_32(len) );
    msglen   = T3D_MSG_LEN_32( sizeof(T3D_PKT_HEAD_SIZE_T) ) + T3D_MSG_LEN_32(len);
    memcpy(&(pkt.buffer[buf_loc]), buf, len);
    shmem_put32((short*)&(destpkt->buffer[buf_loc]),
		(short*)&(pkt.buffer[buf_loc]),
		msglen,
		dest);
  }
  else
  {
    /* I only want to send the necessary information (not the pointer) */
    msglen = T3D_MSG_LEN_64(sizeof(T3D_PKT_HEAD_ZERO_SIZE_T));
    shmem_put((long*)&(destpkt->mode),
	      (long*)&(pkt.mode),
	      msglen,
	      dest);
  }
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_send_short");
  return MPI_SUCCESS;
}

int MPID_T3D_Eagerb_isend_short(void *buf,
				int  len,
				int  src_lrank,
				int  tag,
				int  context_id,
				int  dest,
				MPID_Msgrep_t msgrep,
				MPIR_SHANDLE *shandle)
{
  int mpi_errno;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_isend_short");
  mpi_errno = MPID_T3D_Eagerb_send_short(buf,len,src_lrank,tag,context_id,dest,msgrep);
  shandle->is_complete = 1;
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_isend_short");
  return mpi_errno;
}

int MPID_T3D_Eagerb_recv_short(MPIR_RHANDLE *rhandle,
			       int           from,
			       void         *in_pkt)
{
  T3D_PKT_SHORT_T *pkt = (T3D_PKT_SHORT_T*)in_pkt;
  int              msglen;
  int              buf_loc;
  int              flag = T3D_BUF_AVAIL;
  int              mpierr = MPI_SUCCESS;
  
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_recv_short");
  msglen                = pkt->len;
  rhandle->s.MPI_TAG    = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  MPID_CHK_MSGLEN(rhandle,msglen,mpierr);
  if (msglen > 0)
  {
    buf_loc  = T3D_BUFFER_LENGTH - ( sizeof(short) * T3D_MSG_LEN_32(msglen) );
    memcpy(rhandle->buf,&(pkt->buffer[buf_loc]),msglen);
  }
  rhandle->s.count   = msglen;
  rhandle->s.MPI_ERROR = mpierr;
  if (rhandle->finish)
    (rhandle->finish)(rhandle);
  rhandle->is_complete = 1;

  /*
     To make the packet available again, I set the flag to available in shmem
     and set the flag corresponding to my rank on the source process to available
  */
  pkt->status = T3D_BUF_AVAIL;
  shmem_put(&(t3d_dest_flags[MPID_MyWorldRank]),&flag,1,pkt->source);
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_recv_short");
  return mpierr;
}

int MPID_T3D_Eagerb_save_short(MPIR_RHANDLE *rhandle,
			       int           from,
			       void         *in_pkt)
{
  T3D_PKT_SHORT_T *pkt = (T3D_PKT_SHORT_T *)in_pkt;
  int buf_loc;
  int flag = T3D_BUF_AVAIL;
  
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_save_short");
  rhandle->s.MPI_TAG    = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR  = MPI_SUCCESS;
  rhandle->s.count      = pkt->len;
  rhandle->is_complete  = 1;
  rhandle->push = MPID_T3D_Eagerb_unxrecv_start_short;
  rhandle->start        = NULL;
  if (0 < pkt->len)
  {
    buf_loc  = T3D_BUFFER_LENGTH - ( sizeof(short) * T3D_MSG_LEN_32(pkt->len) );
    rhandle->start  = malloc(pkt->len);
    if (!rhandle->start)
    {
      rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
      return 1;
    }
    memcpy(rhandle->start,&(pkt->buffer[buf_loc]),pkt->len);
  }
  /*
     Similarly to the recv, to make the packet available again, I set the flag to
     available in shmem and set the flag corresponding to my rank on the source
     process to available.
  */
  pkt->status = T3D_BUF_AVAIL;
  shmem_put(&(t3d_dest_flags[MPID_MyWorldRank]),&flag,1,pkt->source);
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_save_short");
  return MPI_SUCCESS;
}

int MPID_T3D_Eagerb_unxrecv_start_short(MPIR_RHANDLE *rhandle,
					void         *in_runex)
{
  MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
  int msglen;
  int mpierr = MPI_SUCCESS;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_unxrecv_start_short");
  msglen = runex->s.count;
  MPID_CHK_MSGLEN(rhandle,msglen,mpierr);
  if (0 < msglen)
  {
    memcpy(rhandle->buf,runex->start,msglen);
    free(runex->start);
    runex->start = NULL;
  }
  rhandle->s           = runex->s;
  rhandle->s.MPI_ERROR = mpierr;
  rhandle->wait        = NULL;
  rhandle->test        = NULL;
  rhandle->push        = NULL;
  rhandle->is_complete = 1;
  if (rhandle->finish)
    (rhandle->finish)(rhandle);
  MPID_RecvFree( runex );
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_unxrecv_start_short");
  return mpierr;
}

void MPID_T3D_Eagerb_short_delete(MPID_Protocol *p)
{
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_short_delete");
  free(p);
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_short_delete");
}


MPID_Protocol *MPID_T3D_Short_setup()
{
  MPID_Protocol *p;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Short_setup");
  p = (MPID_Protocol *)malloc(sizeof(MPID_Protocol));
  if (!p)
    return NULL;

  p->send            = MPID_T3D_Eagerb_send_short;
  p->recv            = MPID_T3D_Eagerb_recv_short;
  p->isend           = MPID_T3D_Eagerb_isend_short;
  p->wait_send       = NULL;
  p->push_send       = NULL;
  p->cancel_send     = NULL;
  p->irecv           = NULL;
  p->wait_recv       = NULL;
  p->push_recv       = NULL;
  p->cancel_recv     = NULL;
  p->do_ack          = NULL;
  p->unex            = MPID_T3D_Eagerb_save_short;
  p->delete          = MPID_T3D_Eagerb_short_delete;

  DEBUG_PRINT_MSG("Leaving MPID_T3D_Short_setup");
  return p;
}
