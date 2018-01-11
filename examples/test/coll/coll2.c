#include "mpi.h"
#include <stdio.h>

#define MAX_PROCESSES 10

int main( argc, argv )
int argc;
char **argv;
{
    int              rank, size, i,j;
    int              table[MAX_PROCESSES][MAX_PROCESSES];
    int              errors=0;
    int              participants;
    MPI_Comm         testcomm;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    /* A maximum of MAX_PROCESSES processes can participate */
    if ( size > MAX_PROCESSES ) participants = MAX_PROCESSES;
    else              participants = size;
    /* Set the particpants so that it divides the MAX_PROCESSES */
    while (MAX_PROCESSES % participants) participants--;
    /* Create the communicator */
    MPI_Comm_split( MPI_COMM_WORLD, rank < participants, rank, &testcomm );

    if (MAX_PROCESSES % participants) {
	fprintf( stderr, "Number of processors must divide %d\n",
		MAX_PROCESSES );
	MPI_Abort( MPI_COMM_WORLD, 1 );
	}
    if ( (rank < participants) ) {

      /* Determine what rows are my responsibility */
      int block_size = MAX_PROCESSES / participants;
      int begin_row  = rank * block_size;
      int end_row    = (rank+1) * block_size;
      int send_count = block_size * MAX_PROCESSES;
      int recv_count = send_count;

      /* Paint my rows my color */
      for (i=begin_row; i<end_row ;i++)
	for (j=0; j<MAX_PROCESSES; j++)
	  table[i][j] = rank + 10;

      /* Gather everybody's result together - sort of like an */
      /* inefficient allgather */
      for (i=0; i<participants; i++)
	MPI_Gather(&table[begin_row][0], send_count, MPI_INT, 
		   &table[0][0],         recv_count, MPI_INT, i, 
		   testcomm );

      /* Everybody should have the same table now,  */
      /* This test does not in any way guarantee there are no errors */
      /* Print out a table or devise a smart test to make sure it's correct */
      for (i=0; i<MAX_PROCESSES;i++) {
	if ( (table[i][0] - table[i][MAX_PROCESSES-1] !=0) ) 
	  errors++;
      }
    } 

    MPI_Comm_free( &testcomm );
    Test_Waitforall( );
    MPI_Finalize();
    if (errors)
      printf( "[%d] done with ERRORS(%d)!\n", rank, errors );
    return errors;
}
