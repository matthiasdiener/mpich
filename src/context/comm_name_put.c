/*
 *  $Id: comm_name_put.c,v 1.3 1997/02/18 23:06:13 gropp Exp $
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

static int MPIR_Name_put ANSI_ARGS((struct MPIR_COMMUNICATOR *, char *));

/*+

MPI_Comm_set_name - give a print name to the communicator

+*/
int MPI_Comm_set_name( com, name )
MPI_Comm com;
char *   name;
{
  struct MPIR_COMMUNICATOR *comm = MPIR_GET_COMM_PTR(com);

  MPIR_TEST_MPI_COMM(com,comm,comm,"MPI_COMM_SET_NAME");

  return MPIR_Name_put (comm, name);
}

static int MPIR_Name_put (comm, name)
struct MPIR_COMMUNICATOR *comm;
char * name;
{
  /* Release any previous name */
  if (comm->comm_name)
    {
      FREE(comm->comm_name);
      comm->comm_name = 0;
    }

  /* Assign a new name */
  if (name)
    {
      char * new_string;

      MPIR_ALLOC(new_string,(char *)MALLOC(strlen(name)+1),comm,MPI_ERR_EXHAUSTED,
		 "Error setting communicator name" );
      strcpy(new_string, name);
      comm->comm_name = new_string;
    }

  /* And also name the collective communicator if it exists */
  if (comm->comm_coll != comm)
    {
      char collName[MPI_MAX_NAME_STRING+1];

      strncpy (collName,name,MPI_MAX_NAME_STRING);
      strncat (collName,"_collective", MPI_MAX_NAME_STRING-strlen(collName));
      MPIR_Name_put (comm->comm_coll, collName);
    }

  /* Bump the sequence number so that the debugger will notice something changed */
  ++MPIR_All_communicators.sequence_number;

  return MPI_SUCCESS;
}

