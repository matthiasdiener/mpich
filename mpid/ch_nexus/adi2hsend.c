/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2hsend.c,v 1.3 1997/04/01 19:36:17 thiruvat Exp $
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
    nexus_startpoint_t send_sp;
    nexus_buffer_t send_buf;
    int num_puts;
    int junk;
    int buf_size;
    int remote_size;

    /*
     * Make sure the send is valid
     */
    if (buf == NULL && count > 0 && datatype->is_contig)
    {
	*error_code = MPI_ERR_BUFFER;
	return ;
    }

    send_sp = Nexus_nodes[dest_grank];

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
    remote_size = 0;
    MPID_Pack_exact_size(count,
    			 datatype,
			 remote_formats[dest_grank],
			 &buf_size,
			 &remote_size);
    buf_size += nexus_sizeof_int(1) * 4;

    nexus_buffer_init(&send_buf, buf_size, num_puts);
    
    nexus_put_int(&send_buf, &src_lrank, 1);
    nexus_put_int(&send_buf, &tag, 1);
    nexus_put_int(&send_buf, &context_id, 1);
    nexus_put_int(&send_buf, &remote_size, 1);
#ifdef DEBUG2
    nexus_printf("MPID_SendDatatype(): sending count %d from context %d tag %d\n", remote_size, context_id, tag);
#endif
    MPID_Pack_buffer(buf, count, datatype, &send_buf);

    nexus_send_rsr(&send_buf,
		   &send_sp,
		   SEND_DATATYPE_HANDLER_ID,
		   NEXUS_TRUE,
		   NEXUS_FALSE);
    
    *error_code = 0;
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
    MPID_SendDatatype(comm, buf, count, datatype, src_lrank, tag, context_id, dest_grank, error_code);

    request->shandle.is_complete = 1;
}
