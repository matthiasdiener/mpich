/*
 *  $Id: adi2ssend.c,v 1.2 1996/07/17 18:04:59 gropp Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "mpid.h"
#include "mpiddev.h"

/***************************************************************************/
/*
 * Multi-protocol, Multi-device support for 2nd generation ADI.
 * We start with support for blocking, contiguous sends.
 * Note the 'msgrep' field; this gives a hook for heterogeneous systems
 * which can be ignored on homogeneous systems.
 * 
 * For the synchronous send, we always use a Rendezvous send.
 */
/***************************************************************************/

void MPID_SsendContig( comm, buf, len, src_lrank, tag, context_id, 
			      dest_grank, msgrep, error_code )
MPI_Comm      comm;
void          *buf;
int           len, src_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }
    *error_code = (*(dev->rndv->send))( buf, len, src_lrank, tag, context_id, 
					dest_grank, msgrep );
}
void MPID_IssendContig( comm, buf, len, src_lrank, tag, context_id, 
			      dest_grank, msgrep, request, error_code )
MPI_Comm      comm;
void          *buf;
int           len, src_lrank, tag, context_id, dest_grank, *error_code;
MPID_Msgrep_t msgrep;
MPI_Request   request;
{
    MPID_Device *dev = MPID_devset->dev[dest_grank];

    /* The one error test that makes sense here */
    if (buf == 0 && len > 0) {
	*error_code = MPI_ERR_BUFFER;
	return;
    }
    *error_code = (*(dev->rndv->isend))( buf, len, src_lrank, tag, context_id, 
					 dest_grank, msgrep, 
					 (MPIR_SHANDLE *)request );
}
