/*
 * Program to test that the "synchronous send" semantics
 * of point to point communications in MPI is (probably) satisfied. 
 * Two messages are send in one order; the destination uses MPI_Iprobe
 * to look for the SECOND message before doing a receive on the first.
 * To give a finite-termination, a fixed amount of time is used for
 * the Iprobe test.
 *
 * This program has been patterned off of "overtake.s"
 *
 *				William Gropp
 *				gropp@mcs.anl.gov
 */

#include <stdio.h>
#include "test.h"
#include "mpi.h"

#define SIZE 10000
/* Amount of time in seconds to wait for the receipt of the second Ssend
   message */
#define MAX_TIME 20
static int src  = 1;
static int dest = 0;

void Generate_Data(buffer, buff_size)
int *buffer;
int buff_size;
{
    int i;

    for (i = 0; i < buff_size; i++)
	buffer[i] = i+1;
}

int main(argc, argv)
int argc;
char **argv;
{
    int rank; /* My Rank (0 or 1) */
    int act_size = 1000;
    int flag;
    int buffer[SIZE];
    double t0;
    char *Current_Test = NULL;
    MPI_Status status;
    MPI_Request r1, r2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == src) { 
	Generate_Data(buffer, SIZE);
	MPI_Recv( buffer, 0, MPI_INT, dest, 0, MPI_COMM_WORLD, &status );
	MPI_Send( buffer, 0, MPI_INT, dest, 0, MPI_COMM_WORLD );
	MPI_Issend( buffer, act_size, MPI_INT, dest, 1, MPI_COMM_WORLD, &r1 );
	MPI_Issend( buffer, act_size, MPI_INT, dest, 2, MPI_COMM_WORLD, &r2 );
	MPI_Wait( &r1, &status );
	MPI_Wait( &r2, &status );
	Test_Waitforall( );
	MPI_Finalize();

    } else if (rank == dest) {
	Test_Init("overtake", rank);
	/* Test 1 */
	Current_Test = "Overtaking Test (Normal Send   -> Normal Recieve)";
	MPI_Send( buffer, 0, MPI_INT, src, 0, MPI_COMM_WORLD );
	MPI_Recv( buffer, 0, MPI_INT, src, 0, MPI_COMM_WORLD, &status );
	t0 = MPI_Wtime();
	flag = 0;
	while (MPI_Wtime() - t0 < MAX_TIME) {
	    MPI_Iprobe( src, 2, MPI_COMM_WORLD, &flag, &status );
	    if (flag) {
		Test_Failed(Current_Test);
		break;
		}
	    }
	if (!flag) 
	    Test_Passed(Current_Test);
	MPI_Recv( buffer, act_size, MPI_INT, src, 1, MPI_COMM_WORLD, &status );
	MPI_Recv( buffer, act_size, MPI_INT, src, 2, MPI_COMM_WORLD, &status );

	Test_Waitforall( );
	MPI_Finalize();
	{
	    int rval = Summarize_Test_Results(); /* Returns number of tests;
						    that failed */
	    Test_Finalize();
	    return rval;
	}
    } else {
	fprintf(stderr, "*** This program uses exactly 2 processes! ***\n");
	exit(-1);
    }

    return 0;
}



