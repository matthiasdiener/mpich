/*
 *  $Id: adi2hssend.c,v 1.2 1996/07/17 18:04:59 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"

/***************************************************************************/
/*
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * This file has support for noncontiguous sends for systems that do not 
 * have native support for complex datatypes.
 */
/***************************************************************************/

void MPID_SsendDatatype( comm, buf, count, datatype, src_lrank, tag, 
			context_id, dest_grank, error_code )
MPI_Comm     comm;
MPI_Datatype datatype;
void         *buf;
int          count, src_lrank, tag, context_id, dest_grank, *error_code;
{
    int             len, contig_size;
    void            *mybuf;
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_Msg_pack_t msgact = MPID_MSG_OK;

    /*
     * Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, 
     * Create a local buffer, use SendContig, and then free the buffer.
     */

    MPIR_DATATYPE_GET_SIZE(datatype,contig_size);
    MPID_DO_HETERO(MPID_Msg_rep( comm, dest_grank, datatype, 
				 &msgrep, &msgact ));
    
    if (contig_size > 0
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator.
	 */
	len = contig_size * count;
	MPID_SsendContig( comm, buf, len, src_lrank, tag, context_id, 
		 dest_grank, msgrep, error_code );
	return;
    }

    mybuf = 0;
    MPID_PackMessage( buf, count, datatype, comm, dest_grank, msgrep, msgact,
		      (void **)&mybuf, &len, error_code );
    if (*error_code) return;

    MPID_SsendContig( comm, mybuf, len, src_lrank, tag, context_id, 
		     dest_grank, msgrep, error_code );
    if (mybuf) {
	FREE( mybuf );
    }
}

/*
 * Noncontiguous datatype issend
 * This is a simple implementation.  Note that in the rendezvous case, the
 * "pack" could be deferred until the "ok to send" message arrives.  To
 * implement this, the individual "send" routines would have to know how to
 * handle general datatypes.  We'll leave that for later.
 */
void MPID_IssendDatatype( comm, buf, count, datatype, src_lrank, tag, 
			  context_id, dest_grank, request, error_code )
MPI_Comm     comm;
MPI_Datatype datatype;
void         *buf;
int          count, src_lrank, tag, context_id, dest_grank, *error_code;
MPI_Request  request;
{
    int             len, contig_size;
    char            *mybuf;
    MPID_Msgrep_t   msgrep = MPID_MSGREP_RECEIVER;
    MPID_Msg_pack_t msgact = MPID_MSG_OK;

    /*
     * Alogrithm:
     * First, see if we can just send the data (contiguous or, for
     * heterogeneous, packed).
     * Otherwise, 
     * Create a local buffer, use SendContig, and then free the buffer.
     */

    MPID_DO_HETERO(MPID_Msg_rep( comm, dest_grank, datatype, 
				 &msgrep, &msgact ));
    MPIR_DATATYPE_GET_SIZE(datatype,contig_size);
    if (contig_size > 0 
	MPID_DO_HETERO(&& msgact == MPID_MSG_OK)) {
	/* Just drop through into the contiguous send routine 
	   For packed data, the representation format is that in the
	   communicator.
	 */
	len = contig_size * count;
	MPID_IssendContig( comm, buf, len, src_lrank, tag, context_id, 
			  dest_grank, msgrep, request, error_code );
	return;
    }

    mybuf = 0;
    MPID_PackMessage( buf, count, datatype, comm, dest_grank, msgrep, msgact,
		      (void **)&mybuf, &len, error_code );
    if (*error_code) return;

    MPID_IssendContig( comm, mybuf, len, src_lrank, tag, context_id, 
		      dest_grank, msgrep, request, error_code );
    if (request->shandle.is_complete) {
	if (mybuf) { FREE( mybuf ); }
	}
    else {
	request->shandle.start  = mybuf;
	request->shandle.finish = MPID_PackMessageFree;
    }

    /* Note that, from the users perspective, the message is now complete
       (!) since the data is out of the input buffer (!) */
}
