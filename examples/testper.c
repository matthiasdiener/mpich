#include "mpi.h"
#include <stdio.h>

/* Test that persistent operations in a persistant pipeline */

#define MAX_P          128
main( argc, argv )
int  argc;
char **argv;
{
MPI_Request request[2*MAX_P];
int       rank, size, i, j,
          cnt;                        /* number of times in loop */
double    time;                       /* Computation time */
int       sendbuf, recvbuf;
int       pipe, left, right, periodic;
MPI_Comm  commring;
MPI_Status statuses[2];

MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );

/* Get the best ring in the topology */
/*
periodic = 1;
MPI_Cart_create( MPI_COMM_WORLD, 1, &size, &periodic, 1, &commring );
MPI_Cart_shift( commring, 0, 1, &left, &right );
 */
commring = MPI_COMM_WORLD;
right = (rank + 1) % size;
left  = (rank - 1 + size) % size;
/* Generate the requests */
/* This requires that the ranks line up with the natural ordering... */
for (i=0; i<size-1; i++) {
    MPI_Send_init( &sendbuf, 1, MPI_INT, right, i, commring, 
	             &request[2*i] );
    MPI_Recv_init( &recvbuf, 1, MPI_INT, left, i, commring, &request[2*i+1] );
    }

time = MPI_Wtime();
for (cnt=0; cnt<10; cnt++) {
    /* Load the initial sendbuffer */
    sendbuf = cnt * size  + rank;
    for (pipe=0; pipe<size; pipe++) {
	if (pipe != size-1) 
	    MPI_Startall( 2, &request[2*pipe] );
	/* Check the data */
        /* Should have cnt*size + (rank - pipe + size)%size */
        if (sendbuf != cnt*size + ((rank - pipe + size)%size)) {
	    printf( "[%d] in iteration %d(%d), got %d expected %d\n", 
                    rank, cnt, pipe, sendbuf, cnt*size +((rank+pipe)%size) );
	    }
	/* Push pipe */
	if (pipe != size-1) 
	    MPI_Waitall( 2, &request[2*pipe], statuses );
        sendbuf = recvbuf;
	}
    }
time = MPI_Wtime() - time;
if (rank == 0) {
    printf( "Circulated 10 times in %f seconds\n", time );
    }
for (i=0; i<2*(size-1); i++) {
    MPI_Request_free( &request[i] );
    }

MPI_Finalize();
}
