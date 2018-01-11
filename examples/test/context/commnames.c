/*
 * Check that we can put names on communicators and get them back.
 */

#include <stdio.h>

#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
  char * commName;

  MPI_Init( &argc, &argv );
  
  if (MPI_Comm_get_name(MPI_COMM_WORLD, &commName) != MPI_SUCCESS)
    {
      printf("Failed to get a name from COMM_WORLD\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

  if (strcmp("MPI_COMM_WORLD", commName))
    {
      printf("Name on MPI_COMM_WORLD is \"%s\" should be \"MPI_COMM_WORLD\"\n", commName);
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

  /* Check that we can replace it */
  if (MPI_Comm_set_name(MPI_COMM_WORLD,"foobar") != MPI_SUCCESS)
    {
      printf("Failed to put a name onto COMM_WORLD\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

  if (MPI_Comm_get_name(MPI_COMM_WORLD, &commName) != MPI_SUCCESS)
    {
      printf("Failed to get a name from COMM_WORLD after changing it\n");
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

  if (strcmp("foobar", commName))
    {
      printf("Name on MPI_COMM_WORLD is \"%s\" should be \"foobar\"\n", 
	     commName );
      MPI_Abort(MPI_COMM_WORLD, -1);
    }

  printf("Name tests OK\n");
  MPI_Finalize();
  return 0;
}
