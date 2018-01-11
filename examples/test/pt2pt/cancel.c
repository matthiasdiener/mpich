/*
 * This file shows a typical use of MPI_Cancel to free IRecv's that
 * are not wanted.  We check for both successful and unsuccessful 
 * cancels
 */

#include "mpi.h"
#include <stdio.h>

int main( argc, argv )
int  argc; 
char **argv;
{
    MPI_Request r1;
    int         size, rank;
    int         err = 0;
    int         partner, buf[10], flag;
    MPI_Status  status;

    MPI_Init( &argc, &argv );

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    
    if (size < 2) {
	printf( "Cancel test requires at least 2 processes\n" );
	MPI_Abort( MPI_COMM_WORLD, 1 );
    }

    /* 
     * Here is the test.  First, we ensure an unsatisfied Irecv:
     *       process 0             process size-1
     *       Sendrecv              Sendrecv
     *       Irecv                    ----
     *       Cancel                   ----
     *       Sendrecv              Sendrecv
     * Next, we confirm receipt before canceling
     *       Irecv                 Send
     *       Sendrecv              Sendrecv
     *       Cancel
     */
    if (rank == 0) {
	partner = size - 1;
	/* Cancel succeeds */
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
	MPI_Irecv( buf, 10, MPI_INT, partner, 0, MPI_COMM_WORLD, &r1 );
	MPI_Cancel( &r1 );
	MPI_Wait( &r1, &status );
	MPI_Test_cancelled( &status, &flag );
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
	if (!flag) {
	    err++;
	    printf( "Cancel of a receive failed where it should succeed.\n" );
	}

	/* Cancel fails */
	MPI_Irecv( buf, 10, MPI_INT, partner, 2, MPI_COMM_WORLD, &r1 );
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
	MPI_Cancel( &r1 );
	MPI_Test( &r1, &flag, &status );
	MPI_Test_cancelled( &status, &flag );
	if (flag) {
	    err++;
	    printf( "Cancel of a receive succeeded where it shouldn't.\n" );
	}

	if (err) {
	    printf( "Test failed with %d errors.\n", err );
	}
	else {
	    printf( "Test passed\n" );
	}
    }
    else if (rank == size - 1) {
	partner = 0;
	/* Cancel succeeds */
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
	/* Cancel fails */
	MPI_Send( buf, 3, MPI_INT, partner, 2, MPI_COMM_WORLD );
	MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_BOTTOM, 0, MPI_INT, partner, 1,
		      MPI_COMM_WORLD, &status );
    }
    MPI_Finalize();
    return 0;
}
