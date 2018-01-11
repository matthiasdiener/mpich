/*
 * Program to test that datatypes that are freed with MPI_TYPE_FREE
 * are not actually deleted until communication that they are a part of
 * has completed.
 *
 */

#include <stdio.h>
#include "test.h"
#include "mpi.h"

#define SIZE 10000
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
    int tag, count, i;
    MPI_Request handle;
    int act_size = 0;
    int flag;
    double data[100];
    double t0;
    char *Current_Test = NULL;
    MPI_Status status;
    MPI_Datatype rowtype;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    tag    = 2001;
    count  = 1;
    for (i = 0; i < 100; i++)
	data[i] = i;
    MPI_Type_vector( 10, 1, 10, MPI_DOUBLE, &rowtype );
    MPI_Type_commit( &rowtype );
    if (rank == src) { 
	MPI_Irecv(data, count, rowtype, dest, tag, MPI_COMM_WORLD,
		 &handle ); 
	MPI_Type_free( &rowtype );
	MPI_Recv( (void *)0, 0, MPI_INT, dest, tag+1, 
		  MPI_COMM_WORLD, &status );
	MPI_Wait( &handle, &status );

    } else if (rank == dest) {
	MPI_Isend( data, count, rowtype, src, tag, MPI_COMM_WORLD, 
		  &handle );
	MPI_Type_free( &rowtype );
	MPI_Send( (void *)0, 0, MPI_INT, src, tag+1, MPI_COMM_WORLD );
	MPI_Wait( &handle, &status );
	}

    Test_Waitforall( );
    MPI_Finalize();

    return 0;
}



