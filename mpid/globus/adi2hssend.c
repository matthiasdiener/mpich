/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hssend.c,v 1.2 1998/05/18 18:13:23 gropp Exp $
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
    /* NICK */
    /* globus_nexus_startpoint_t rhandle_sp; */
    MPID_cond_list *cond_l;
    MPIR_RHANDLE rhandle;
    union MPIR_HANDLE rhandleAsRequest;

    globus_mutex_lock(&message_queue_lock);
    MPID_Get_Cond_Variable(cond_l);

    rhandle.cond = &cond_l->cond;
    rhandle.is_complete = GLOBUS_FALSE;

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
	globus_cond_wait(rhandle.cond, &message_queue_lock);
    }

    MPID_Free_Cond_Variable(cond_l);
    globus_mutex_unlock(&message_queue_lock);
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
    /* begin NICK */
    /* globus_nexus_startpoint_t send_sp; */
    globus_nexus_startpoint_t *send_sp;
    int my_format;
    int dataorigin_nonpacksize;
    /* end NICK */
    globus_nexus_startpoint_t ssend_reply_sp;
    globus_nexus_buffer_t send_buf;

    int num_puts = 0;
    int junk;
    int buf_size;
    /* NICK */
    /* int remote_size; */

/* globus_nexus_printf("NICK: entering MPID_IssendDatatype() count %d type %d src %d tag %d context %d dest %d\n", count, datatype->dte_type, src_lrank, tag, context_id, dest_grank); */
    rhandle->is_complete = 0;
    /* NICK */
    /* send_sp = Nexus_nodes[dest_grank]; */
    send_sp = &Nexus_nodes[dest_grank];

    globus_nexus_endpoint_init(&(rhandle->endpoint), &default_ep_attr);
    globus_nexus_endpoint_set_user_pointer(&(rhandle->endpoint), (void *)rhandle);
    
    globus_nexus_startpoint_bind(&ssend_reply_sp, &(rhandle->endpoint));

#ifdef USE_DIRECT
    MPID_Pack_buffer_preprocess(count, datatype, &num_puts, &junk);
#else
    num_puts = 0;
#endif
    buf_size = 0;
    /* NICK */
    /* remote_size = 0; */
    dataorigin_nonpacksize = 0;
    MPID_Pack_exact_size(count,
    			 datatype,
			 /* NICK */
			 /* remote_formats[dest_grank], */
			 NEXUS_DC_FORMAT_LOCAL,
			 &buf_size, /* packed size */
			 /* &remote_size); */
			 &dataorigin_nonpacksize); /* non packed size ... */
						/* sizeof(n) = n*sizeof(1) */

    /* NICK */
    /* buf_size += (globus_nexus_sizeof_int(1) * 4 */
		 /* + globus_nexus_sizeof_startpoint(&ssend_reply_sp, 1)); */

    globus_nexus_buffer_init(&send_buf, 
			buf_size
			/* begin NICK */
			+ globus_nexus_sizeof_int(1) * 5
			+ globus_nexus_sizeof_startpoint(&ssend_reply_sp, 1), 
			/* end NICK */
			/* 0); */
			num_puts);


    globus_nexus_put_int(&send_buf, &comm->local_rank, 1);
    globus_nexus_put_int(&send_buf, &tag, 1);
    globus_nexus_put_int(&send_buf, &context_id, 1);
    /* begin NICK */
    /* globus_nexus_put_int(&send_buf, &remote_size, 1); */
    globus_nexus_put_int(&send_buf, &dataorigin_nonpacksize, 1);
    my_format = NEXUS_DC_FORMAT_LOCAL;
    globus_nexus_put_int(&send_buf, &my_format, 1);
    /* end NICK */
    globus_nexus_put_startpoint_transfer(&send_buf, &ssend_reply_sp, 1);
    MPID_Pack_buffer(buf, count, datatype, &send_buf);

    globus_nexus_send_rsr(&send_buf,
		   /* NICK */
		   /* &send_sp, */
		   send_sp,
		   SSEND_DATATYPE_HANDLER_ID,
		   GLOBUS_TRUE,
		   GLOBUS_FALSE);
    
    /*
     * the endpoint is not getting destroyed here...it should be done
     * in the return handler.
     */
/* globus_nexus_printf("NICK: exiting MPID_IssendDatatype() count %d type %d src %d tag %d context %d dest %d\n", count, datatype->dte_type, src_lrank, tag, context_id, dest_grank); */
}



