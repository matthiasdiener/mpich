#include <stdio.h>

#include "mpi.h"

/*
   This is a simple test that can be used on heterogeneous systems that
   use XDR encoding to check for correct lengths. 
 */
int main( argc, argv )
int argc;
char **argv;
{
    int rank, c;
    MPI_Status status;
    char buf[10];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if (rank == 0) {
	MPI_Recv( buf, 10, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, 
		  &status );
	MPI_Get_count( &status, MPI_CHAR, &c );
	if (c != 10) { 
	    printf( "Did not get correct count; expected 10, got %d\n", c );
	    }
	}
    else if (rank == 1) {
	MPI_Send( buf, 10, MPI_CHAR, 0, 0, MPI_COMM_WORLD );
	}
    MPI_Finalize();
return 0;
}
