/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2req.c,v 1.2 1997/04/01 19:36:19 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include "mpid.h"
#include "dev.h"
#include "reqalloc.h"
#include "../util/queue.h"

void MPID_Request_free(MPI_Request request)
{
    int mpi_errno;

    switch(request->handle_type)
    {
      case MPIR_SEND:
	free(request);
	break;
      case MPIR_RECV:
	if (MPID_RecvIcomplete(request, (MPI_Status *)NULL, &mpi_errno))
	{
	    free(request);
	}
	break;
      case MPIR_PERSISTENT_SEND:
      case MPIR_PERSISTENT_RECV:
	printf("Not done - persistent_{send,recv}_free\n");
	break;
      default:
	nexus_fatal("Internal error--unknown request type\n");
    }
}
