#include "mpi.h"

/*
 * This tests for the existence of MPI_Pcontrol; nothing more.
 */
int main( argc, argv )
int argc;
char **argv;
{
    MPI_Init( &argc, &argv );
    
    MPI_Pcontrol( 0 );
    printf( "Pcontrol test passed\n" );
    MPI_Finalize();
    return 0;
}
