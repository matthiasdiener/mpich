/*@
    third - test program that tests queueing by sending messages with various
            tags, receiving them in particular order.
@*/

#include "mpi.h"

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size, to, from, tag, count, np, i;
    int src, dest, waiter;
    int st_source, st_tag, st_count;
    MPI_Request handle;
    MPI_Status status;
    char data[100];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    src  = size - 1;
    dest = 0;
    /* waiter = dest; */  	/* Receiver delays, so msgs unexpected */
    /* waiter = src;  */  	/* Sender delays, so recvs posted      */
    waiter = 10000;		/* nobody waits */

    if (rank == src)
    {
	if (waiter == src)
	    sleep(10);
	to     = dest;
	tag    = 2001;
	sprintf(data,"First message, type 2001");
	count = strlen(data) + 1;
	MPI_Send( data, count, MPI_CHAR, to, tag, MPI_COMM_WORLD );
	printf("%d sent :%s:\n", rank, data );

	tag    = 2002;
	sprintf(data,"Second message, type 2002");
	count = strlen(data) + 1;
	MPI_Send( data, count, MPI_CHAR, to, tag, MPI_COMM_WORLD );
	printf("%d sent :%s:\n", rank, data );
    }
    else
    if (rank == dest)
    {
	if (waiter == dest)
	    sleep(10);
	from  = MPI_ANY_SOURCE;
	count = 100;		

	tag   = 2002;
	MPI_Recv(data, count, MPI_CHAR, from, tag, MPI_COMM_WORLD, &status ); 
	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_CHAR, &st_count );
	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
	printf( "%d received :%s:\n", rank, data);

	tag   = 2001;
	MPI_Recv(data, count, MPI_CHAR, from, tag, MPI_COMM_WORLD, &status ); 
	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count( &status, MPI_CHAR, &st_count );
	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
	printf( "%d received :%s:\n", rank, data);
    }
    MPI_Finalize();
    printf( "Process %d exiting\n", rank );
}
