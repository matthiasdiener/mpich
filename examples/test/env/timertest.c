#include <stdio.h>
#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
    double t1, t2;
    double tick;
    int    i;

    MPI_Init( &argc, &argv );
    t1 = MPI_Wtime();
    t2 = MPI_Wtime();
    fprintf( stderr, "Two successive calls to MPI_Wtime gave: (%f) (%f)\n", 
		t1, t2 );
    fprintf( stderr, "Five approximations to one second:\n");
    for (i = 0; i < 5; i++)
    {
	t1 = MPI_Wtime();
	sleep(1);
	t2 = MPI_Wtime();
	fprintf( stderr, "%f seconds\n", t2 - t1 );
    } 
    tick = MPI_Wtick();
    fprintf( stderr, "MPI_Wtick gave: (%10.8f)\n", tick );

    MPI_Finalize( );

    return 0;
}
