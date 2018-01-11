#include <stdio.h>
#include "mpi.h"
#include "mpe.h"


main( argc, argv )
int argc;
char **argv;
{
  int i, j, myid, np;
  MPI_Status status;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myid );
  MPI_Comm_size( MPI_COMM_WORLD, &np );
  
  for (j=0; j<1; j++) {
    if (myid & 0x1 || myid==(np-1)) {
      if (myid==(np-1)) {
	i = myid;
      } else {
	i = myid-1;
      }
      fprintf( stderr, "%d sending to %d\n", myid, i );
      MPI_Send( &i, 1, MPI_INT, i, 0, MPI_COMM_WORLD );
    }
    
    if (!(myid & 0x1) || myid==(np-1)) {
      if (myid==(np-1)) {
	i = myid;
      } else {
	i = myid + 1;
      }
      fprintf( stderr, "%d receiving from %d\n", myid, i );
      MPI_Recv( &i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status );
    }
  }

  MPI_Finalize();
}
