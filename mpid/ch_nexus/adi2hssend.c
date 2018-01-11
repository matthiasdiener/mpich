/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hssend.c,v 1.3 1997/04/01 19:36:18 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
#include "mpimem.h"
#include "nexuspriv.h"

void MPID_SsendDatatype(struct MPIR_COMMUNICATOR *comm,
			void *buf,
			int count,
			struct MPIR_DATATYPE *datatype,
			int src_lrank,
			int tag,
			int context_id,
			int dest_grank,
			int *error_code)
{
    nexus_startpoint_t rhandle_sp;
    MPID_cond_list *cond_l;
    MPIR_RHANDLE rhandle;
    union MPIR_HANDLE rhandleAsRequest;

    nexus_mutex_lock(&message_queue_lock);
    MPID_Get_Cond_Variable(cond_l);

    rhandle.cond = &cond_l->cond;
    rhandle.is_complete = NEXUS_FALSE;

    /* Create an MPI_Request structure so we can cascade to Issend routine
       properly */

    rhandleAsRequest.rhandle = rhandle;
    
    MPID_IssendDatatype(comm,
    			buf,
			count,
			datatype,
			src_lrank,
			tag,
			context_id,
			dest_grank,
			&rhandleAsRequest,
			error_code);

    while (!rhandle.is_complete)
    {
	nexus_cond_wait(rhandle.cond, &message_queue_lock);
    }

    MPID_Free_Cond_Variable(cond_l);
    nexus_mutex_unlock(&message_queue_lock);
}

void MPID_IssendDatatype(struct MPIR_COMMUNICATOR *comm,
			 void *buf,
			 int count,
			 struct MPIR_DATATYPE *datatype,
			 int src_lrank,
			 int tag,
			 int context_id,
			 int dest_grank,
			 MPI_Request request,
			 int *error_code)
{
    MPIR_RHANDLE *rhandle = &request->rhandle;
    nexus_startpoint_t send_sp;
    nexus_startpoint_t ssend_reply_sp;
    nexus_buffer_t send_buf;

    int num_puts = 0;
    int junk;
    int buf_size;
    int remote_size;

    rhandle->is_complete = 0;
    send_sp = Nexus_nodes[dest_grank];

    nexus_endpoint_init(&(rhandle->endpoint), &default_ep_attr);
    nexus_endpoint_set_user_pointer(&(rhandle->endpoint), (void *)rhandle);
    
    nexus_startpoint_bind(&ssend_reply_sp, &(rhandle->endpoint));

#ifdef USE_DIRECT
    MPID_Pack_buffer_preprocess(count, datatype, &num_puts, &junk);
#else
    num_puts = 0;
#endif
    buf_size = 0;
    remote_size = 0;
    MPID_Pack_exact_size(count,
    			 datatype,
			 remote_formats[dest_grank],
			 &buf_size,
			 &remote_size);
    buf_size += (nexus_sizeof_int(1) * 4
		 + nexus_sizeof_startpoint(&ssend_reply_sp, 1));

    nexus_buffer_init(&send_buf, buf_size, 0);


    nexus_put_int(&send_buf, &comm->local_rank, 1);
    nexus_put_int(&send_buf, &tag, 1);
    nexus_put_int(&send_buf, &context_id, 1);
    nexus_put_int(&send_buf, &remote_size, 1);
    nexus_put_startpoint_transfer(&send_buf, &ssend_reply_sp, 1);
    MPID_Pack_buffer(buf, count, datatype, &send_buf);

    nexus_send_rsr(&send_buf,
		   &send_sp,
		   SSEND_DATATYPE_HANDLER_ID,
		   NEXUS_TRUE,
		   NEXUS_FALSE);
    
    /*
     * the endpoint is not getting destroyed here...it should be done
     * in the return handler.
     */
}



