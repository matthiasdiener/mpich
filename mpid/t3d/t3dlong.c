#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "packets.h"
#include "t3dpriv.h"
#include "t3dlong.h"

static char *save_stack;
static int   modified_stack;
extern char *t3d_heap_limit;
extern char *get_stack();
extern MPID_Device *ch_t3d_dev;

extern int t3d_lsi_window_head;
extern int t3d_lsi_window_tail;
extern int t3d_num_lsi;
extern volatile T3D_Long_Send_Info *t3d_lsi;


void MPID_T3D_Check_Target(void *target,int length)
{
  extern char *t3d_heap_limit;
  int tpl = (int)target + length;
  int tml = (int)target - length;

  save_stack = get_stack();
  modified_stack = 0;
  if ( (tml < (int)save_stack) && (tpl > (int)t3d_heap_limit) )
    if ( abs((int)save_stack - tml) < abs(tpl - (int)t3d_heap_limit) )
    {
      shmem_stack( (void*)tml);
      modified_stack = 1;
    }
  else
    malloc_brk( (char*)tpl );
  t3d_heap_limit = (char*)sbrk(0);
}

void MPID_T3D_Reset_Stack()
{
  if (modified_stack)
    set_stack( save_stack );
}


int MPID_T3D_Progress_Long_Sends()
{
  int mpierr;
  long flag = 1;
  int index = t3d_lsi_window_head;
  MPIR_SHANDLE *shandle;
  int msglen;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Progress_Long_Sends");
  while (index != t3d_lsi_window_tail)
  {
    if ( (t3d_lsi[index].shandle != NULL) && (t3d_lsi[index].target_completer != NULL) )
    {
      shandle = (MPIR_SHANDLE*)t3d_lsi[index].shandle;
      shandle->errval = 0;
      if (shandle->bytes_as_contig > t3d_lsi[index].length)
	shandle->errval = MPI_ERR_TRUNCATE;
      /* NOTE:
	 must verify that buffer is mapped in local virtual space on T3D */
      if (t3d_lsi[index].length > 0)
      {
	MPID_T3D_Check_Target(t3d_lsi[index].target_buffer,t3d_lsi[index].length);
	msglen = T3D_MSG_LEN_32(t3d_lsi[index].length);
	shmem_put32(t3d_lsi[index].target_buffer,
		    t3d_lsi[index].local_buffer,
		    msglen,
		    t3d_lsi[index].dest);
	MPID_T3D_Reset_Stack();
      }
      MPID_T3D_Check_Target(t3d_lsi[index].target_completer,sizeof(long));
      shmem_put((long*)t3d_lsi[index].target_completer,
                (long*)&flag,
                1,
                t3d_lsi[index].dest);
      MPID_T3D_Reset_Stack();
      /* clean up on local side */
      if (t3d_lsi[index].mustfreebuf)
	free(t3d_lsi[index].local_buffer);
      shandle->is_complete = 1;
      t3d_lsi[index].shandle = NULL;
      t3d_lsi[index].target_buffer = NULL;
      while (t3d_lsi_window_head != t3d_lsi_window_tail)
      {
	if (t3d_lsi[t3d_lsi_window_head].shandle == NULL)
	  t3d_lsi_window_head = (t3d_lsi_window_head + 1) % t3d_num_lsi;
	else
	  break;
      }
      break;
    }
    else
      index = (index + 1) % t3d_num_lsi;
  }
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Progress_Long_Sends");
  return MPI_SUCCESS;
}


int MPID_T3D_Eagerb_send_long(void *buf,
			      int   len,
			      int   src_lrank,
			      int   tag,
			      int   context_id,
			      int   dest,
			      MPID_Msgrep_t msgrep)
{
  T3D_PKT_LONG_T pkt;
  T3D_PKT_LONG_T *destpkt;
  int msglen;
  int buf_loc;
  int flag=1;
  int mpierr = 0;
  extern MPID_Device *ch_t3d_dev;
  extern T3D_Long_Send_Info *blocking_lsi;
  
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagern_send_long");

  while (t3d_dest_flags[dest] != T3D_BUF_AVAIL)
    MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);

  destpkt = (T3D_PKT_LONG_T*) &(t3d_recv_bufs[MPID_MyWorldRank]);

  t3d_dest_flags[dest] = T3D_BUF_IN_USE;
  pkt.mode       = T3D_PKT_LONG;
  pkt.context_id = context_id;
  pkt.lrank      = src_lrank;
  pkt.tag        = tag;
  pkt.len        = len;
  pkt.source     = MPID_MyWorldRank;
  pkt.status     = T3D_BUF_IN_USE;
  pkt.pLSI       = blocking_lsi;

  /* if the buffer isn't aligned on a 32-bit boundary or an even multiple of 4 bytes in length
     we must allocate a buffer in which to send the message */
  if (len > 0)
  {
    blocking_lsi->mustfreebuf = ! ( T3D_IS_4BYTE_ALIGNED(buf) ) || ! ( T3D_IS_4BYTE_LENGTH( len ) ); 
    if (!blocking_lsi->mustfreebuf)
      blocking_lsi->local_buffer = buf;
    else
    {
      blocking_lsi->local_buffer = (void*)malloc(T3D_MSG_LEN_64(len) * 8);
      if (!blocking_lsi->local_buffer)
	return MPI_ERR_INTERN;
      memcpy(blocking_lsi->local_buffer,buf,len);
    }
  }
  else
  {
    blocking_lsi->mustfreebuf = 0;
    blocking_lsi->local_buffer = NULL;
  }
  blocking_lsi->target_buffer = NULL;
  blocking_lsi->target_completer = NULL;

  /* send the packet header only */
  msglen = T3D_MSG_LEN_64(sizeof(T3D_PKT_HEAD_SIZE_T));
  shmem_put((long*)&(destpkt->pLSI),
	    (long*)&(pkt.pLSI),
	    msglen,
	    dest);
  
  while ((volatile)blocking_lsi->target_completer == NULL)
    MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);

  
  msglen = T3D_MSG_LEN_32(blocking_lsi->length);
  if (blocking_lsi->length > 0)
  {
    MPID_T3D_Check_Target(blocking_lsi->target_buffer,msglen);
    shmem_put32((long*)blocking_lsi->target_buffer,
		(long*)blocking_lsi->local_buffer,
		msglen,
		dest);
    MPID_T3D_Reset_Stack();
  }
  MPID_T3D_Check_Target(blocking_lsi->target_completer,sizeof(long));
  shmem_put((long*)blocking_lsi->target_completer,
	    (long*)&flag,
	    1,
	    dest);
  MPID_T3D_Reset_Stack();
  if (blocking_lsi->mustfreebuf)
    free(blocking_lsi->local_buffer);
  
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagern_send_long");
  return MPI_SUCCESS;
}

int MPID_T3D_Eagern_isend_long(void *buf,
			       int  len,
			       int  src_lrank,
			       int  tag,
			       int  context_id,
			       int  dest,
			       MPID_Msgrep_t msgrep,
			       MPIR_SHANDLE *shandle)
{
  int mpi_errno;
  T3D_PKT_LONG_T pkt;
  T3D_PKT_LONG_T *destpkt;
  int msglen;  
  int nextslot = (t3d_lsi_window_tail + 1) % t3d_num_lsi;
  volatile T3D_Long_Send_Info *lsi = &t3d_lsi[t3d_lsi_window_tail];

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagern_isend_long");
  if (nextslot != t3d_lsi_window_head)
  {
    while (t3d_dest_flags[dest] != T3D_BUF_AVAIL)
      MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);
    
    destpkt = (T3D_PKT_LONG_T*) &(t3d_recv_bufs[MPID_MyWorldRank]);
    
    t3d_dest_flags[dest] = T3D_BUF_IN_USE;
    pkt.mode       = T3D_PKT_LONG;
    pkt.context_id = context_id;
    pkt.lrank      = src_lrank;
    pkt.tag        = tag;
    pkt.len        = len;
    pkt.source     = MPID_MyWorldRank;
    pkt.status     = T3D_BUF_IN_USE;
    pkt.pLSI       = (T3D_Long_Send_Info*)lsi;  /* must override (volatile) */
    
    lsi->dest = dest;
    lsi->shandle = shandle;
    if (len > 0)
    {
      lsi->mustfreebuf = ! ( T3D_IS_4BYTE_ALIGNED(buf) ) || ! ( T3D_IS_4BYTE_LENGTH( len ) );
      if (!lsi->mustfreebuf)
	lsi->local_buffer = buf;
      else
      {
	lsi->local_buffer = (void*)malloc(T3D_MSG_LEN_64(len) * 8);
	if (!lsi->local_buffer)
	  return MPI_ERR_INTERN;
	memcpy(lsi->local_buffer,buf,len);
      }
    }
    else
    {
      lsi->mustfreebuf = 0;
      lsi->local_buffer = NULL;
    }
    lsi->target_buffer = NULL;
    lsi->target_completer = NULL;
    t3d_lsi_window_tail = nextslot;
    /* send the packet header only */
    msglen = T3D_MSG_LEN_64(sizeof(T3D_PKT_HEAD_SIZE_T));
    shmem_put((long*)&(destpkt->pLSI),
              (long*)&(pkt.pLSI),
              msglen,
              dest);
    /* we don't send the data here.  It is in Progress_Long_Sends */
    
    mpi_errno = MPI_SUCCESS;
  }
  else
  {
    /* window is full.  we can block or we can error.
       For now, let's block */
    printf("[%d] I had to block on an Isendlong()!\n",MPID_MyWorldRank);  fflush(stdout);
    mpi_errno = MPID_T3D_Eagerb_send_long(buf,len,src_lrank,tag,context_id,dest,msgrep);
    shandle->is_complete = 1;
  }

  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagern_isend_long");
  return mpi_errno;
}

int MPID_T3D_Eagerb_recv_long(MPIR_RHANDLE *rhandle,
			      int           from,
			      void         *in_pkt)
{
  T3D_PKT_LONG_T *pkt = (T3D_PKT_LONG_T*)in_pkt;
  int              msglen;
  int              buf_loc;
  int              mustfreebuf;
  int              flag = T3D_BUF_AVAIL;
  int              mpierr = MPI_SUCCESS;
  T3D_Long_Send_Info tmp_lsi;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_recv_long");
  msglen                = pkt->len;
  rhandle->s.MPI_TAG    = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  MPID_CHK_MSGLEN(rhandle,msglen,mpierr);
  rhandle->s.count      = msglen;
  rhandle->s.MPI_ERROR  = mpierr;
  if (msglen > 0)
  {
    mustfreebuf = ! (T3D_IS_4BYTE_ALIGNED(rhandle->buf)) || ! ( T3D_IS_4BYTE_LENGTH( msglen ) );
    if (!mustfreebuf)
      tmp_lsi.target_buffer = rhandle->buf;
    else
      tmp_lsi.target_buffer  = malloc(T3D_MSG_LEN_64(msglen) * 8);
    if (!tmp_lsi.target_buffer)
    {
      rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
      return 1;
    }
  }
  else
  {
    mustfreebuf = 0;
    tmp_lsi.target_buffer = NULL;
  }
  tmp_lsi.target_completer = &(rhandle->is_complete);
  tmp_lsi.length           = msglen;  /* the smaller of the two - send size and receive size */
  shmem_put(&(pkt->pLSI->length),
	    &tmp_lsi.length,
	    4,
	    pkt->lrank);

  pkt->status = T3D_BUF_AVAIL;
  shmem_put(&(t3d_dest_flags[MPID_MyWorldRank]),&flag,1,pkt->source);

  while (!(volatile)(rhandle->is_complete))
    MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);

  if (mustfreebuf)
  {
    memcpy(rhandle->buf,tmp_lsi.target_buffer,msglen);
    free(tmp_lsi.target_buffer);
  }
  if (rhandle->finish)
    (rhandle->finish)(rhandle);

  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_recv_long");
  return mpierr;
}

int MPID_T3D_Eagerb_save_long(MPIR_RHANDLE *rhandle,
			      int           from,
			      void         *in_pkt)
{
  T3D_PKT_LONG_T *pkt = (T3D_PKT_LONG_T *)in_pkt;
  int buf_loc;
  int msglen;
  T3D_Long_Send_Info *tmp_lsi;
  int flag = T3D_BUF_AVAIL;
  
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_save_long");
  rhandle->s.MPI_TAG    = pkt->tag;
  rhandle->s.MPI_SOURCE = pkt->lrank;
  rhandle->s.MPI_ERROR  = MPI_SUCCESS;
  rhandle->s.count      = pkt->len;
  rhandle->push         = MPID_T3D_Eagerb_unxrecv_start_long;

  tmp_lsi = (T3D_Long_Send_Info*)malloc(sizeof(T3D_Long_Send_Info));
  rhandle->start = (void*)tmp_lsi;
  if (NULL == tmp_lsi)
  {
      rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
      return 1;
  }
  tmp_lsi->shandle          = (void*)(pkt->pLSI);
  tmp_lsi->target_completer = NULL;
  tmp_lsi->length           = pkt->len;

  pkt->status = T3D_BUF_AVAIL;  
  shmem_put(&(t3d_dest_flags[MPID_MyWorldRank]),&flag,1,pkt->source);

  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_save_long");
  return MPI_SUCCESS;
}

int MPID_T3D_Eagerb_unxrecv_start_long(MPIR_RHANDLE *rhandle,
				       void         *in_runex)
{
  MPIR_RHANDLE *runex = (MPIR_RHANDLE *)in_runex;
  T3D_Long_Send_Info *tmp_lsi;
  T3D_Long_Send_Info *src_lsi;
  int msglen;
  int mustfreebuf=0;
  int mpierr = MPI_SUCCESS;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_unxrecv_start_long");
  msglen = runex->s.count;
  tmp_lsi = (T3D_Long_Send_Info*)runex->start;
  src_lsi = (T3D_Long_Send_Info*)tmp_lsi->shandle;
  MPID_CHK_MSGLEN(rhandle,msglen,mpierr);

  /* must check for 4-byte alignment and 4-byte multiple length */
  if (msglen > 0)
  {
    mustfreebuf = ! (T3D_IS_4BYTE_ALIGNED(rhandle->buf)) || ! ( T3D_IS_4BYTE_LENGTH( msglen ) );
    if (!mustfreebuf)
      tmp_lsi->target_buffer = rhandle->buf;
    else
      tmp_lsi->target_buffer  = malloc(T3D_MSG_LEN_64(msglen) * 8);
    if (!tmp_lsi->target_buffer)
    {
      rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
      return 1;
    }
  }
  else
  {
    mustfreebuf = 0;
    tmp_lsi->target_buffer = NULL;
  }
  tmp_lsi->target_completer = &(rhandle->is_complete);
  tmp_lsi->length           = msglen;  /* the smaller of the two - send size and receive size */
  shmem_put(&(src_lsi->length),
	    &(tmp_lsi->length),
	    4,
            runex->s.MPI_SOURCE);

  while (!(volatile)(rhandle->is_complete))
    MPID_CH_Check_incoming(ch_t3d_dev,MPID_NOTBLOCKING);

  if (mustfreebuf)
  {
    memcpy(rhandle->buf,tmp_lsi->target_buffer,msglen);
    free(tmp_lsi->target_buffer);
  }

  if (runex->start)
    free(runex->start);

  runex->start = NULL;
  rhandle->s           = runex->s;
  rhandle->s.MPI_ERROR = mpierr;
  rhandle->s.count     = msglen;
  rhandle->wait        = NULL;
  rhandle->push        = NULL;
  rhandle->is_complete = 1;
  if (rhandle->finish)
    (rhandle->finish)(rhandle);
  MPID_RecvFree( runex );
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_unxrecv_start_long");
  return mpierr;
}

void MPID_T3D_Eagerb_long_delete(MPID_Protocol *p)
{
  DEBUG_PRINT_MSG("Entering MPID_T3D_Eagerb_long_delete");
  free(p);
  DEBUG_PRINT_MSG("Leaving MPID_T3D_Eagerb_long_delete");
}


MPID_Protocol *MPID_T3D_Long_setup()
{
  MPID_Protocol *p;

  DEBUG_PRINT_MSG("Entering MPID_T3D_Long_setup");
  p = (MPID_Protocol *)malloc(sizeof(MPID_Protocol));
  if (!p)
    return NULL;

  p->send            = MPID_T3D_Eagerb_send_long;
  p->recv            = MPID_T3D_Eagerb_recv_long;
  p->isend           = MPID_T3D_Eagern_isend_long;
  p->wait_send       = NULL;
  p->push_send       = NULL;
  p->cancel_send     = NULL;
  p->irecv           = NULL;
  p->wait_recv       = NULL;
  p->push_recv       = NULL;
  p->cancel_recv     = NULL;
  p->do_ack          = NULL;
  p->unex            = MPID_T3D_Eagerb_save_long;
  p->delete          = MPID_T3D_Eagerb_long_delete;

  DEBUG_PRINT_MSG("Leaving MPID_T3D_Long_setup");
  return p;
}


