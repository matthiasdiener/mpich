#include "mpi.h"

/* Header for testing procedures */

#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif

#if defined(NEEDS_STDLIB_PROTOTYPES)
#include "protofix.h"
#endif

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
