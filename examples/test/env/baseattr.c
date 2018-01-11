#include <stdio.h>
#include "mpi.h"

int main(argc, argv)
int argc;
char **argv;
{
int    err = 0;
int host, tagub, hasio;
void *v;
int  flag;
int  vval;
int  rank, size;

MPI_Init( &argc, &argv );
MPI_Comm_size( MPI_COMM_WORLD, &size );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Attr_get( MPI_COMM_WORLD, MPI_TAG_UB, &v, &flag );
if (!flag || (vval = *(int*)v)< 32767) {
    err++;
    fprintf( stderr, "Could not get TAG_UB or got too-small value\n" );
    }
MPI_Attr_get( MPI_COMM_WORLD, MPI_HOST, &v, &flag );
vval = *(int*)v;
if (!flag || ((vval < 0 || vval >= size) && vval != MPI_PROC_NULL)) {
    err++;
    fprintf( stderr, "Could not get HOST or got invalid value\n" );
    }
MPI_Attr_get( MPI_COMM_WORLD, MPI_IO, &v, &flag );
vval = *(int*)v;
if (!flag || ((vval < 0 || vval >= size) && vval != MPI_ANY_SOURCE &&
	      vval != MPI_PROC_NULL)) {
    err++;
    fprintf( stderr, "Could not get IO or got invalid value\n" );
    }
Test_Waitforall( );
MPI_Finalize( );

return err;
}
