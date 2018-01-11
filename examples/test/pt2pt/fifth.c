#include "mpi.h"

int main( argc, argv )
int argc;
char **argv;
{
    int rank, np, data = 777;
    int st_source, st_tag, st_count;
    MPI_Request handle[4];
    MPI_Status status[4];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &np );

    if (np < 4) {
      MPI_Finalize();
      printf( "4 processors or more required, %d done\n", rank );
      return(0);
    }

    if (rank == 0) {
      MPI_Isend( &data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &handle[0] );
      MPI_Irecv( &data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &handle[1] );
      MPI_Isend( &data, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &handle[2] );
      MPI_Irecv( &data, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &handle[3] );
      MPI_Waitall ( 4, handle, status );
    }
    else if (rank == 1) {
      MPI_Irecv( &data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &handle[0] );
      MPI_Isend( &data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &handle[1] );
      MPI_Isend( &data, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &handle[2] );
      MPI_Irecv( &data, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &handle[3] );
      MPI_Waitall ( 4, handle, status );
    }
    else if (rank == 2) {
      MPI_Isend( &data, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &handle[0] );
      MPI_Irecv( &data, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &handle[1] );
      MPI_Irecv( &data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &handle[2] );
      MPI_Isend( &data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &handle[3] );
      MPI_Waitall ( 4, handle, status );
    }
    else if (rank == 3) {
      MPI_Irecv( &data, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &handle[0] );
      MPI_Isend( &data, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &handle[1] );
      MPI_Irecv( &data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &handle[2] );
      MPI_Isend( &data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &handle[3] );
      MPI_Waitall ( 4, handle, status );
    }
    Test_Waitforall( );
    MPI_Finalize();
}
