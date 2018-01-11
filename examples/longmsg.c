/*
    longmsg - test program that sends an array of floats from the first process
             of a group to the last, using send and recv
*/

#include "mpi.h"

int main( argc, argv )
int argc;
char **argv;
{
    int rank, size, to, from, tag, count, np, i;
    int src, dest;
    int st_source, st_tag, st_count;
    MPI_Request handle;
    MPI_Status status;
    double data[4000];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    printf( "Process %d of %d is alive\n", rank, size );

    src  = size - 1;
    dest = 0;

    count  = 1000;
    if (rank == src)
    {
	to     = dest;
	tag    = 2001;
	for (i = 0; i < count; i++)
	    data[i] = i;
	MPI_Send( data, count, MPI_DOUBLE, to, tag, MPI_COMM_WORLD );
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
    }
    else
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	from  = MPI_ANY_SOURCE;
	for (i=0; i<count; i++) data[i] = 0.0;
	MPI_Recv(data, count, MPI_DOUBLE, from, tag, MPI_COMM_WORLD,
		 &status ); 

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count(  status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
	for (i = 0; i < count ; i ++) {
	    if (data[i] != (double)i) {
		printf( "data[%d] is wrong\n", i );
		break;
		}
	    }
    }
    if (rank == 0) {
	printf( "Now test nonblocking operations...\n" );
	}
    if (rank == src)
    {
	to     = dest;
	tag    = 2001;
	for (i = 0; i < count; i++)
	    data[i] = i;
	MPI_Isend( data, count, MPI_DOUBLE, to, tag, MPI_COMM_WORLD, &handle );
	MPI_Wait( &handle, &status );
	printf("%d sent", rank );
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
    }
    else
    if (rank == dest)
    {
	tag   = MPI_ANY_TAG;
	from  = MPI_ANY_SOURCE;
	for (i=0; i<count; i++) data[i] = 0.0;
	MPI_Irecv(data, count, MPI_DOUBLE, from, tag, MPI_COMM_WORLD,
		 &handle ); 
	MPI_Wait( &handle, &status );

	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	MPI_Get_count(  status, MPI_DOUBLE, &st_count );

	printf( "Status info: source = %d, tag = %d, count = %d\n",
	        st_source, st_tag, st_count );
	printf( "%d received", rank);
	for (i = 0; i < 10; i++) printf(" %f",data[i]);	printf("\n");
	for (i = 0; i < count ; i ++) {
	    if (data[i] != (double)i) {
		printf( "data[%d] is wrong\n", i );
		break;
		}
	    }
    }
    MPI_Finalize();
    printf( "Process %d exiting\n", rank );
}
