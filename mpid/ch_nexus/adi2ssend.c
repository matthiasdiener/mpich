/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2ssend.c,v 1.2 1997/04/01 19:36:20 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

/*
 * Note: This code is not used nor supported yet.
 */

#include "mpid.h"
#include "dev.h"

void MPID_SsendContig(struct MPIR_COMMUNICATOR *comm,
		      void *buf,
		      int len,
		      int src_lrank,
		      int tag,
		      int context_id,
		      int dest_grank,
		      MPID_Msgrep_t msgrep,
		      int *error_code)
{
    /* ??? */
}

void MPID_IssendContig(struct MPIR_COMMUNICATOR *comm,
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
    /* ??? */
}
