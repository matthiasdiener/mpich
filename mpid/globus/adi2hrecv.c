/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hrecv.c,v 1.2 1998/05/18 18:13:20 gropp Exp $
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
    /* begin NICK */
    int dataorigin_maxnonpacksize;
    int dataorigin_unitnonpacksize;
    int local_nonpacksize;
    int dummy;
    int recvd_nbytes;
    /* end NICK */
/* globus_nexus_printf("NICK: entering MPID_RecvDatatype() count %d datatype %d src %d tag %d context %d\n", count, datatype->dte_type, src_lrank, tag, context_id); */

  /*
   * Make sure the receive is valid
   */
  if (buf == NULL && count > 0 && datatype->is_contig)
    {
/* globus_nexus_printf("NICK: MPID_RecvDatatype() setting error_code = MPI_ERR_BUFFER\n"); */
      *error_code = MPI_ERR_BUFFER;
/* globus_nexus_printf("NICK: exiting MPID_RecvDatatype() with error\n"); */
      return ;
    }

  /*
   * Check queue for message that has already arrived that matches
   * this request.  If one does, take it, otherwise wait for one to
   * show up.
   */
  /* NICK */
  /* globus_mutex_lock(&MPID_message_queue_lock); */
  globus_mutex_lock(&message_queue_lock);
  /* rhandle = MPID_RecvAlloc(); */
  MPID_RecvAlloc(rhandle);
  rhandle->sptr = NULL;
  rhandle->buf = buf;
  MPIR_REF_INCR(datatype);
  rhandle->type = datatype;
  rhandle->s.count = rhandle->count = count * datatype->size;
  SET_STATUSCOUNT_ISRCVBUFSIZE(rhandle->s.private_count)
#ifdef DEBUG2
    globus_nexus_printf(
        "MPID_Recv_Datatype tag = %d context = %d (rhandle->)ct = %d\n",
        tag, context_id, count);
#endif

  MPID_Get_Cond_Variable(cond_l);
  rhandle->cond = &cond_l->cond;
  rhandle->is_complete = GLOBUS_FALSE;
  rhandle->s.MPI_ERROR = 0;

  MPID_Search_unexpected_queue_and_post(src_lrank,
                                        tag,
                                        context_id,
                                        rhandle,
                                        &unexpected);
  if (unexpected)
    {
/* globus_nexus_printf("NICK: MPID_RecvDatatype(): RCV fromrank %d tag %d context %d: count %d type %d: ... data already arrived\n", src_lrank, tag, context_id, count, datatype->dte_type); */

      /* NICK */
      /* globus_mutex_unlock(&MPID_message_queue_lock); */
      globus_mutex_unlock(&message_queue_lock);
      if (unexpected->sptr)
        {
          globus_nexus_buffer_t send_buf;
          /*
           * There was a synchronous send for this message.  We
           * can notify any waiting threads and set the send to
           * complete.
           */
            
          globus_nexus_buffer_init(&send_buf, 0, 0);
          globus_nexus_send_rsr(&send_buf,
                         unexpected->sptr,
                         SSEND_DONE_ID,
                         GLOBUS_TRUE,
                         GLOBUS_FALSE);

	globus_poll();

        }

	MPID_extract_data(unexpected,
			&unexpected->recv_buf,
			count,
			datatype,
			buf,
			error_code,
			&recvd_nbytes);
	globus_nexus_buffer_destroy(&unexpected->recv_buf);
	unexpected->count = unexpected->s.count = recvd_nbytes;
        SET_STATUSCOUNT_ISLOCAL(unexpected->s.private_count)
      /* GKT: status->count = count * datatype->size; */
      rhandle->s.count = status->count = unexpected->count;
      SET_STATUSCOUNT_ISLOCAL(rhandle->s.private_count)
      /* begin NICK */
      /* status->dataorigin_format = unexpected->dataorigin_format; */
      /* status->dataorigin_nonpacksize = unexpected->dataorigin_nonpacksize; */
      /* set byte in status->private_count to indicate that status->count */
      /* should be interpreted as local byte count ... whenever status->count */
      /* can be interpreted in this way, we do not need to store              */
      /* dataorigin_format in status->private_count as well ... we can ignore */
      /* dataorigin_format and dataorigin_nonpacksize                   */
      SET_STATUSCOUNT_ISLOCAL(status->private_count)
      /* end NICK */
      status->MPI_SOURCE = unexpected->s.MPI_SOURCE;
      status->MPI_TAG = unexpected->s.MPI_TAG;
      unexpected->is_complete = GLOBUS_TRUE;
      if (unexpected->cond) {
        globus_cond_signal(unexpected->cond);
      }
    }
  else
    {
/* globus_nexus_printf("NICK: MPID_RecvDatatype(): RCV fromrank %d tag %d context %d: count %d type %d: ... data not here yet\n", src_lrank, tag, context_id, count, datatype->dte_type); */
      while (!rhandle->is_complete)
        {
	  /* NICK */
          /* globus_cond_wait(rhandle->cond, &MPID_message_queue_lock); */
          globus_cond_wait(rhandle->cond, &message_queue_lock);
        }
      /* NICK */
      /* globus_mutex_unlock(&MPID_message_queue_lock); */
      globus_mutex_unlock(&message_queue_lock);

      MPID_Free_Cond_Variable(cond_l);

/* globus_nexus_printf("NICK: MPID_RecvDatatype() setting error_code = rhandle->s.MPI_ERROR\n"); */
      *error_code = rhandle->s.MPI_ERROR;
#ifdef DEBUG
      globus_nexus_printf("status->count set in MPID_RecvDatatype to %d should be %d?\n",
                   count * datatype->size, rhandle->count);
#endif
      /* GKT: status->count = count * datatype->size; */
      status->count = rhandle->s.count = rhandle->count;
      SET_STATUSCOUNT_ISLOCAL(rhandle->s.private_count)
      /* begin NICK */
      /* status->dataorigin_format = rhandle->dataorigin_format; */
      /* status->dataorigin_nonpacksize = rhandle->dataorigin_nonpacksize; */
      /* set byte in status->private_count to indicate that status->count */ 
      /* should be interpreted as local byte count ... whenever status->count */
      /* can be interpreted in this way, we do not need to store              */
      /* dataorigin_format in status->private_count as well ... we can ignore */
      /* dataorigin_format and dataorigin_nonpacksize                   */
      SET_STATUSCOUNT_ISLOCAL(status->private_count)
      /* end NICK */
      status->MPI_SOURCE = rhandle->s.MPI_SOURCE;
      status->MPI_TAG = rhandle->s.MPI_TAG;
    }

  MPIR_REF_DECR(datatype);
/* globus_nexus_printf("NICK: exiting MPID_RecvDatatype()\n"); */
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
  /* begin NICK */
  int dataorigin_maxnonpacksize;
  int dataorigin_unitnonpacksize;
  int local_nonpacksize;
  int dummy;
  int recvd_nbytes;
  /* end NICK */

/* globus_nexus_printf("NICK: entering MPID_Irecv_Datatype()\n"); */
  /*
   * Make sure the receive is valid
   */
  if (buf == NULL && count > 0 && datatype->is_contig)
    {
      *error_code = MPI_ERR_BUFFER;
/* globus_nexus_printf("NICK: exiting MPID_Irecv_Datatype() with error\n"); */
      return ;
    }

  /* NICK */
  /* globus_mutex_lock(&MPID_message_queue_lock); */
  globus_mutex_lock(&message_queue_lock);

  rhandle->buf = buf;
  MPIR_REF_INCR(datatype);
  rhandle->type = datatype;
  rhandle->s.count = rhandle->count = count * datatype->size;
  SET_STATUSCOUNT_ISRCVBUFSIZE(rhandle->s.private_count)
#ifdef DEBUG2
    globus_nexus_printf(
        "MPID_Irecv_Datatype tag = %d context = %d (rhandle->)ct = %d\n",
        tag, context_id, count);
#endif

  rhandle->cond = NULL;
  rhandle->is_complete = GLOBUS_FALSE;
  MPID_Search_unexpected_queue_and_post(src_lrank,
                                        tag,
                                        context_id,
                                        rhandle,
                                        &unexpected);
  if (unexpected)
    {
      /* NICK */
      /* globus_mutex_unlock(&MPID_message_queue_lock); */
      globus_mutex_unlock(&message_queue_lock);
      if (unexpected->sptr)
        {
          globus_nexus_buffer_t send_buf;
          /*
           * There was a synchronous send for this message.  We
           * can notify any waiting threads and set the send to
           * complete.
           */

          globus_nexus_buffer_init(&send_buf, 0, 0);
          globus_nexus_send_rsr(&send_buf,
                         unexpected->sptr,
                         SSEND_DONE_ID,
                         GLOBUS_TRUE,
                         GLOBUS_FALSE);
          globus_poll();
        }

	MPID_extract_data(unexpected,
			&unexpected->recv_buf,
			count,
			datatype,
			buf,
			error_code,
			&recvd_nbytes);
	globus_nexus_buffer_destroy(&unexpected->recv_buf);
	unexpected->count = unexpected->s.count = recvd_nbytes;
        SET_STATUSCOUNT_ISLOCAL(unexpected->s.private_count)

      rhandle->is_complete = GLOBUS_TRUE;
      rhandle->s.MPI_SOURCE = unexpected->s.MPI_SOURCE;
      rhandle->s.MPI_TAG = unexpected->s.MPI_TAG;
      rhandle->s.MPI_ERROR = unexpected->s.MPI_ERROR;
      /* GKT: rhandle->s.count = count * datatype->size; */
      rhandle->s.count = unexpected->count;
      SET_STATUSCOUNT_ISLOCAL(rhandle->s.private_count)
      /* begin NICK */
      /* rhandle->dataorigin_format = unexpected->dataorigin_format; */
      /* rhandle->dataorigin_nonpacksize = unexpected->dataorigin_nonpacksize; */
      /* end NICK */

      unexpected->is_complete = GLOBUS_FALSE;
      if (unexpected->cond) {
        globus_cond_signal(unexpected->cond);
      }

/* globus_nexus_printf("NICK: exiting MPID_Irecv_Datatype()\n"); */
      return ;
    }

  /* NICK */
  /* globus_mutex_unlock(&MPID_message_queue_lock); */
  globus_mutex_unlock(&message_queue_lock);
/* globus_nexus_printf("NICK:exiting MPID_Irecv_Datatype()\n"); */
}
