#include "mpi.h"

/* 
   This test checks for the ability of an MPI implementation to detect
   and recover from truncating messages (receiver's buffer too small).

   The standard does NOT require that the implementation recover, but
   it does require detection.  Note that some recent clarifications to
   the standard for the wait/test some/all were made to allow MPI 
   implementations to report which receives truncated data (when that
   occurs).
*/

int main(argc, argv)
    int argc;
    char **argv;
{
int myrank, mysize;
int rc;

MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Comm_size(MPI_COMM_WORLD, &mysize);

MPI_Errhandler_set( MPI_COMM_WORLD, MPI_ERRORS_RETURN );

if (mysize < 2) {
    fprintf( stderr, "Must provide at least two processes\n" );
    MPI_Finalize();
    return 1;
    }

src = 0;
dest = mysize - 1; 
if (myrank == src) {
    MPI_Send( buf, 1000, MPI_INT, dest, 0, MPI_COMM_WORLD );
    }
else if (myrank == dest) {
    rc = MPI_Recv( rbuf, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status );
    rc = MPI_Irecv( rbuf, 1, MPI_INT, src, 1, MPI_COMM_WORLD, &request );
    rc = MPI_Wait( &request, &status );
    }

MPI_Finalize();
return 0;
}
