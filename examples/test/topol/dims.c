/* -*- Mode: C; c-basic-offset:4 ; -*- */

#include "mpi.h"
#include <stdio.h>

int main( int argc, char *argv[] )
{
    int dims[10];
    int err, errcnt = 0;

    MPI_Init( &argc, &argv );

    MPI_Errhandler_set( MPI_COMM_WORLD, MPI_ERRORS_RETURN );

    /* Try for error checks */
    dims[0] = 2;
    dims[1] = 2;
    dims[2] = 0;
    err = MPI_Dims_create( 26, 3, dims );
    if (err == MPI_SUCCESS) {
	printf( "Doubled dims did not return error\n" );
	errcnt++;
    }

    if (errcnt) {
	printf( " %d errors found\n", errcnt );
    }
    else {
	printf( " No Errors\n" );
    }
    MPI_Finalize( );
    return 0;
}
