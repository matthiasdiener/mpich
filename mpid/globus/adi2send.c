/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2send.c,v 1.2 1998/05/18 18:13:37 gropp Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
#include "nexuspriv.h"

#define MPID_BsendConig MPID_SendContig

void MPID_SendContig(struct MPIR_COMMUNICATOR *comm,
		     void *buf,
		     int len,
		     int src_lrank,
		     int tag,
		     int context_id,
		     int dest_grank,
		     MPID_Msgrep_t msgrep,
		     int *error_code)
{
    globus_nexus_startpoint_t send_sp;
    int buf_size;
    globus_nexus_buffer_t send_buf;
/* globus_nexus_printf("NICK: enter MPID_SendContig()\n"); */

    if (!buf && len > 0)
    {
	 *error_code = MPI_ERR_BUFFER;
	return ;
    }

    send_sp = Nexus_nodes[dest_grank];

#ifdef USE_DIRECT
    buf_size = globus_nexus_sizeof_int(1) * 2;
    globus_nexus_buffer_init(&send_buf, buf_size, 1);
#else
    buf_size = globus_nexus_sizeof_int(1) * 2 + globus_nexus_sizeof_byte(len);
    globus_nexus_buffer_init(&send_buf, buf_size, 0);
#endif

    globus_nexus_put_int(&send_buf, &tag, 1);
    globus_nexus_put_int(&send_buf, &context_id, 1);
#ifdef USE_DIRECT
    nexus_direct_put_byte(&send_buf, buf, len);
#else
    globus_nexus_put_byte(&send_buf, buf, len);
#endif

    globus_nexus_send_rsr(&send_buf,
		   &send_sp,
		   SEND_CONTIG_HANDLER_ID,
		   GLOBUS_TRUE,
		   GLOBUS_FALSE);
}

void MPID_IsendContig(struct MPIR_COMMUNICATOR *comm,
		      void *buf,
		      int len,
		      int src_lrank,
		      int tag,
		      int context_id,
		      int dest_grank,
		      MPID_Msgrep_t msgrep,
		      MPI_Request request,
		      int *error_code)
{
    MPID_SendContig(comm, buf, len, src_lrank, tag, context_id, dest_grank, msgrep, error_code);

    request->shandle.is_complete = 1;
    /* cleanup any dynamically allocated part of request->shandle */
}

int MPID_SendIcomplete(MPI_Request request, int *error_code)
{
    /*
     * This function is probably not going to advance thesends completion.
     * Either it has already completed, or we have not received the
     * acknowledgement from a synchronous send.  There really is nothing
     * we can do to speed this up.
     */
#ifdef BUILD_LITE

    /*
     * This may be of some help in the non-threaded case to check if any
     * new messages have arrived since the last globus_poll() which could
     * have been some time ago.
     */
    globus_poll();
#endif

    /*
     * Although we don't expect things to be speeded up, it is important,
     * nonetheless, to return whatever the true completion status is for
     * the send handle.
     */
    return request->shandle.is_complete; 
}
    
void MPID_SendComplete(MPI_Request request, int *error_code)
{
    MPIR_SHANDLE *shandle = &request->shandle;
    MPID_cond_list *cond_l;

/*    globus_nexus_printf("MPID_SendComplete: shandle = %p\n", shandle); */
    globus_poll();
    globus_mutex_lock(&message_queue_lock);
    MPID_Get_Cond_Variable(cond_l);
    shandle->cond = &cond_l->cond;

    while (!shandle->is_complete)
    {
	globus_cond_wait(shandle->cond, &message_queue_lock);
    }

    MPID_Free_Cond_Variable(cond_l);
    globus_mutex_unlock(&message_queue_lock);

    return ;
}
