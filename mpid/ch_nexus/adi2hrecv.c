/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hrecv.c,v 1.4 1997/04/02 17:36:41 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
#include "../util/queue.h"
#include "../ch2/reqalloc.h"
#include "nexuspriv.h"

void MPID_RecvDatatype(struct MPIR_COMMUNICATOR *comm,
                       void *buf,
                       int count,
                       struct MPIR_DATATYPE *datatype,
                       int src_lrank,
                       int tag,
                       int context_id,
                       MPI_Status *status,
                       int *error_code)
{
  MPIR_RHANDLE *rhandle;
  MPIR_RHANDLE *unexpected;
  MPID_cond_list *cond_l;

  /*
   * Make sure the receive is valid
   */
  if (buf == NULL && count > 0 && datatype->is_contig)
    {
      *error_code = MPI_ERR_BUFFER;
      return ;
    }

  /*
   * Check queue for message that has already arrived that matches
   * this request.  If one does, take it, otherwise wait for one to
   * show up.
   */
  nexus_mutex_lock(&message_queue_lock);
  rhandle = MPID_RecvAlloc();
  rhandle->sptr = NULL;
  rhandle->buf = buf;
  MPIR_REF_INCR(datatype);
  rhandle->type = datatype;
  rhandle->s.count = rhandle->count = count * datatype->size;
#ifdef DEBUG2
    nexus_printf(
        "MPID_Recv_Datatype tag = %d context = %d (rhandle->)ct = %d\n",
        tag, context_id, count);
#endif

  MPID_Get_Cond_Variable(cond_l);
  rhandle->cond = &cond_l->cond;
  rhandle->is_complete = NEXUS_FALSE;
  rhandle->s.MPI_ERROR = 0;

  MPID_Search_unexpected_queue_and_post(src_lrank,
                                        tag,
                                        context_id,
                                        rhandle,
                                        &unexpected);
  if (unexpected)
    {
      nexus_mutex_unlock(&message_queue_lock);
      if (unexpected->sptr)
        {
          nexus_buffer_t send_buf;
          /*
           * There was a synchronous send for this message.  We
           * can notify any waiting threads and set the send to
           * complete.
           */
            
          nexus_buffer_init(&send_buf, 0, 0);
          nexus_send_rsr(&send_buf,
                         unexpected->sptr,
                         SSEND_DONE_ID,
                         NEXUS_TRUE,
                         NEXUS_FALSE);


        }
      if (unexpected->count > count * datatype->size)
        {
#ifdef _DEBUG
          nexus_printf("%s:%d ", __FILE__, __LINE__);
          nexus_printf(" MPID_Recvdatatype: truncation detected\n");
          nexus_printf("%s:%d ", __FILE__, __LINE__);
          nexus_printf(" MPID_Recvdatatype: unexpected->count = %d count = %d",
                       unexpected->count, count);
#endif
          unexpected->s.MPI_ERROR = *error_code = MPI_ERR_TRUNCATE;
        }

      else {
#ifdef DEBUG
        nexus_printf("unexpected->count in MPID_RecvDataType = %d\n",
		unexpected->count);
#endif
        MPID_Unpack_buffer(buf,
                           unexpected->count / datatype->size,
                           datatype,
                           &unexpected->recv_buf);
      }
      /* GKT: status->count = count * datatype->size; */
      rhandle->s.count = status->count = unexpected->count;
      status->MPI_SOURCE = unexpected->s.MPI_SOURCE;
      status->MPI_TAG = unexpected->s.MPI_TAG;
      unexpected->is_complete = NEXUS_TRUE;
      if (unexpected->cond) {
        nexus_cond_signal(unexpected->cond);
      }
    }
  else
    {
      while (!rhandle->is_complete)
        {
          nexus_cond_wait(rhandle->cond, &message_queue_lock);
        }
      nexus_mutex_unlock(&message_queue_lock);

      MPID_Free_Cond_Variable(cond_l);

      *error_code = rhandle->s.MPI_ERROR;
#ifdef DEBUG
      nexus_printf("status->count set in MPID_RecvDatatype to %d should be %d?\n",
                   count * datatype->size, rhandle->count);
#endif
      /* GKT: status->count = count * datatype->size; */
      status->count = rhandle->count;
      status->MPI_SOURCE = rhandle->s.MPI_SOURCE;
      status->MPI_TAG = rhandle->s.MPI_TAG;
    }

  MPIR_REF_DECR(datatype);
}

void MPID_IrecvDatatype(struct MPIR_COMMUNICATOR *comm,
                        void *buf,
                        int count,
                        struct MPIR_DATATYPE *datatype,
                        int src_lrank,
                        int tag,
                        int context_id,
                        MPI_Request request,
                        int *error_code)
{
  MPIR_RHANDLE *rhandle = &request->rhandle;
  MPIR_RHANDLE *unexpected;

  /*
   * Make sure the receive is valid
   */
  if (buf == NULL && count > 0 && datatype->is_contig)
    {
      *error_code = MPI_ERR_BUFFER;
      return ;
    }

  nexus_mutex_lock(&message_queue_lock);

  rhandle->buf = buf;
  MPIR_REF_INCR(datatype);
  rhandle->type = datatype;
  rhandle->s.count = rhandle->count = count * datatype->size;
#ifdef DEBUG2
    nexus_printf(
        "MPID_Irecv_Datatype tag = %d context = %d (rhandle->)ct = %d\n",
        tag, context_id, count);
#endif

  rhandle->cond = NULL;
  rhandle->is_complete = NEXUS_FALSE;
  MPID_Search_unexpected_queue_and_post(src_lrank,
                                        tag,
                                        context_id,
                                        rhandle,
                                        &unexpected);
  if (unexpected)
    {
      nexus_mutex_unlock(&message_queue_lock);
      if (unexpected->sptr)
        {
          nexus_buffer_t send_buf;
          /*
           * There was a synchronous send for this message.  We
           * can notify any waiting threads and set the send to
           * complete.
           */

          nexus_buffer_init(&send_buf, 0, 0);
          nexus_send_rsr(&send_buf,
                         unexpected->sptr,
                         SSEND_DONE_ID,
                         NEXUS_TRUE,
                         NEXUS_FALSE);
        }

      if (unexpected->count > count * datatype->size)
        {
#ifdef DEBUG
          nexus_printf("%s:%d ", __FILE__, __LINE__);
          nexus_printf(" MPID_Irecvdatatype: truncation detected\n");
          nexus_printf("%s:%d ", __FILE__, __LINE__);
          nexus_printf(" MPID_Irecvdatatype: unexpected->count = %d count = %d",
                       unexpected->count, count);
#endif
          *error_code = unexpected->s.MPI_ERROR = *error_code = MPI_ERR_TRUNCATE;
        }

      else {
        MPID_Unpack_buffer(buf,
                           unexpected->count / datatype->size,
                           datatype,
                           &unexpected->recv_buf);

      }
      rhandle->is_complete = NEXUS_TRUE;
      rhandle->s.MPI_SOURCE = unexpected->s.MPI_SOURCE;
      rhandle->s.MPI_TAG = unexpected->s.MPI_TAG;
      rhandle->s.MPI_ERROR = unexpected->s.MPI_ERROR;
      /* GKT: rhandle->s.count = count * datatype->size; */
      rhandle->s.count = unexpected->count;

      unexpected->is_complete = NEXUS_FALSE;
      if (unexpected->cond) {
        nexus_cond_signal(unexpected->cond);
      }

      return ;
    }

  nexus_mutex_unlock(&message_queue_lock);
}



