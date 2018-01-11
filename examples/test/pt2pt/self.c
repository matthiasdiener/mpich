#include "mpi.h"
#include <stdio.h>
int main( argc, argv )
int argc;
char **argv;
{
int           sendbuf[10];
int           sendcount = 10;
int           recvbuf[10];
int           recvcount = 10;
int           source = 0, recvtag = 2;
int           dest = 0, sendtag = 2;

    int               mpi_errno = MPI_SUCCESS;
    MPI_Status        status_array[2];
    MPI_Request       req[2];

    MPI_Init( &argc, &argv );
    if (mpi_errno = MPI_Irecv ( recvbuf, recvcount, MPI_INT,
			    source, recvtag, MPI_COMM_WORLD, &req[1] )) 
	return mpi_errno;
    if (mpi_errno = MPI_Isend ( sendbuf, sendcount, MPI_INT, dest,   
			    sendtag, MPI_COMM_WORLD, &req[0] )) 
	return mpi_errno;

    fprintf( stdout, "[%d] Starting waitall\n", 0 );
    mpi_errno = MPI_Waitall ( 2, req, status_array );
    fprintf( stdout, "[%d] Ending waitall\n", 0 );

    MPI_Finalize();
    return (mpi_errno);
}
