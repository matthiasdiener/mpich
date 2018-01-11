/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hsend.c,v 1.2 1998/05/18 18:13:22 gropp Exp $
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

void MPID_SendDatatype(struct MPIR_COMMUNICATOR *comm,
		       void *buf,
		       int count,
		       struct MPIR_DATATYPE *datatype,
		       int src_lrank,
		       int tag,
		       int context_id,
		       int dest_grank,
		       int *error_code)
{
    /* begin NICK */
    /* globus_nexus_startpoint_t send_sp; */
    globus_nexus_startpoint_t *send_sp;
    int my_format;
    int dataorigin_nonpacksize;
    /* end NICK */
    globus_nexus_buffer_t send_buf;
    int num_puts;
    int junk;
    int buf_size;
    /* NICK */
    /* int remote_size; */

/* globus_nexus_printf("NICK: enter MPID_SendDatatype(): SEND destrank %d tag %d context %d: count %d type %d: fromrank %d\n", dest_grank, tag, context_id, count, datatype->dte_type, src_lrank); */
    /*
     * Make sure the send is valid
     */
    if (buf == NULL && count > 0 && datatype->is_contig)
    {
	*error_code = MPI_ERR_BUFFER;
	return ;
    }

    /* NICK */
    /* send_sp = MPID_Nexus_nodes[dest_grank]; */
    /* send_sp = &(MPID_Nexus_nodes[dest_grank]); */
    send_sp = &(Nexus_nodes[dest_grank]);

    /*
     * Count how many nexus_direct_put_TYPE()'s will happen for
     * MPID_Pack_buffer().
     */
#ifdef USE_DIRECT
    MPID_Pack_buffer_preprocess(count, datatype, &num_puts, &junk);
#else
    num_puts = 0;
#endif
    buf_size = 0;
    dataorigin_nonpacksize = 0;
    MPID_Pack_exact_size(count,
    			 datatype,
			 /* NICK */
			 /* MPID_remote_formats[dest_grank], */
			 /* remote_formats[dest_grank], */
			 NEXUS_DC_FORMAT_LOCAL,
			 &buf_size,     /* packed size */
			 &dataorigin_nonpacksize); /* not packed size, ... */
					         /* sizeof(n) = n * sizeof(1) */
/* globus_nexus_printf("NICK: MPID_SendDatatype(): tag %d context %d: just calculated dataorigin_nonpacksize %d using local format %d type %d count %d\n", tag, context_id, dataorigin_nonpacksize, NEXUS_DC_FORMAT_LOCAL, datatype->dte_type, count); */
    /* NICK */
    /* buf_size += globus_nexus_sizeof_int(1) * 4; */

    globus_nexus_buffer_init(&send_buf, 
		    buf_size+(globus_nexus_sizeof_int(1) * 5), 
		    num_puts);

    
    globus_nexus_put_int(&send_buf, &src_lrank, 1);
    globus_nexus_put_int(&send_buf, &tag, 1);
    globus_nexus_put_int(&send_buf, &context_id, 1);
    /* begin NICK */
    /* globus_nexus_put_int(&send_buf, &remote_size, 1); */
    globus_nexus_put_int(&send_buf, &dataorigin_nonpacksize, 1);
    my_format = NEXUS_DC_FORMAT_LOCAL;
    globus_nexus_put_int(&send_buf, &(my_format), 1);
/* globus_nexus_printf("NICK: MPID_SendDatatype() just put sender %d tag %d context_id %d ndatabytes %d dataorigin_format %d\n", src_lrank, tag, context_id, buf_size,my_format); */
    /* end NICK */
#ifdef DEBUG2
    globus_nexus_printf("MPID_SendDatatype(): sending count %d from context %d tag %d\n", local_non_packedsize, context_id, tag);
#endif

/* globus_nexus_printf("NICK: MPID_SendDatatype(): tag %d context %d: packing %d bytes with with unit size %d (count %d) of type %d\n", tag, context_id, buf_size, (count == 0 ? 0 :buf_size/count), count, datatype->dte_type); */

    MPID_Pack_buffer(buf, count, datatype, &send_buf);

    globus_nexus_send_rsr(&send_buf,
		   /* NICK */
		   /* &send_sp, */
		   send_sp,
		   SEND_DATATYPE_HANDLER_ID,
		   GLOBUS_TRUE,
		   GLOBUS_FALSE);
    globus_poll();
    
    *error_code = 0;
/* globus_nexus_printf("NICK: exit MPID_SendDatatype(): SEND destrank %d tag %d context %d: count %d type %d: fromrank %d\n", dest_grank, tag, context_id, count, datatype->dte_type, src_lrank); */
}

void MPID_IsendDatatype(struct MPIR_COMMUNICATOR *comm,
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
/* globus_nexus_printf("NICK: entering MPID_IsendDatatype() count %d from %d to %d tag %d context %d ... calling MPID_SendDatatype()\n", count, src_lrank, dest_grank, tag, context_id); */
    MPID_SendDatatype(comm, buf, count, datatype, src_lrank, tag, context_id, dest_grank, error_code);

    request->shandle.is_complete = 1;
/* globus_nexus_printf("NICK: exiting MPID_IsendDatatype() count %d from %d to %d tag %d context %d\n", count, src_lrank, dest_grank, tag, context_id); */
}
