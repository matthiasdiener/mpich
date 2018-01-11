#include "mpi.h"

int main( argc, argv )
int argc;
char **argv;
{
    int              rank, size, i;
    MPI_Request      handle;
    MPI_Status       status;
    int             *table;
    int              errors=0;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    /* Make data table */
    table = (int *) calloc (size, sizeof(int));
    table[rank] = rank + 1;

    MPI_Barrier ( MPI_COMM_WORLD );
    /* Broadcast the data */
    for ( i=0; i<size; i++ ) 
      MPI_Bcast( &table[i], 1, MPI_INT, i, MPI_COMM_WORLD );

    /* See if we have the correct answers */
    for ( i=0; i<size; i++ )
      if (table[i] != i+1) errors++;

    MPI_Barrier ( MPI_COMM_WORLD );

    MPI_Finalize();
    if (errors)
      printf( "[%d] done with ERRORS!\n", rank );
    return errors;
}


