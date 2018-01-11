/*
 *  $Id: comm_name_get.c,v 1.3 1997/02/18 23:06:13 gropp Exp $
 *
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */
/* Update log
 *
 * Nov 28 1996 jcownie@dolphinics.com: Implement MPI-2 communicator naming function.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif


/*+

MPI_Comm_get_name - return the print name from the communicator

+*/
int MPI_Comm_get_name( comm, namep )
MPI_Comm comm;
char **namep;
{
  struct MPIR_COMMUNICATOR *comm_ptr = MPIR_GET_COMM_PTR(comm);
  static char myname[] = "MPI_COMM_GET_NAME";

  TR_PUSH(myname);

  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  if (comm_ptr->comm_name)
    {
      *namep =  comm_ptr->comm_name;
    }
  else
    {
      *namep = "";		/* The standard says null string... */
      /*return MPI_SUCCESS;*/	/* We really want to return something else
				 * MPI_ERR_UNNAMED ?
				 */
    }
  TR_POP;
  return MPI_SUCCESS;
}

