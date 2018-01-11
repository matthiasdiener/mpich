#include <stdio.h>
#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
int    err = 0;
double t1, t2;
double tick;

MPI_Init( &argc, &argv );
t1 = MPI_Wtime();
t2 = MPI_Wtime();
if (t2 - t1 > 0.1 || t2 - t1 < 0.0) {
    err++;
    fprintf( stderr, 
"Two successive calls to MPI_Wtime gave strange results: (%f) (%f)\n", 
t1, t2 );
    }
tick = MPI_Wtick();
if (tick > 1.0 || tick < 0.0) {
    err++;
    fprintf( stderr, "MPI_Wtick gave a strange result: (%f)\n", tick );
    }
MPI_Finalize( );

return err;
}
