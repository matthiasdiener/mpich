#include <stdio.h>
#include "mpi.h"


int main(argc, argv)
int argc;
char *argv[];
{
    int easy;
    int rank;
    int size;
    int a;
    int b;
    MPI_Request request;
    MPI_Status  status;
    double t1;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    easy = 1;

    MPI_Barrier( MPI_COMM_WORLD );
    if (rank == 0)
    {
	MPI_Irecv(&a, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
	MPI_Recv(&b, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
	MPI_Wait(&request, &status);
	printf("rank = %d, a = %d, b = %d\n", rank, a, b);
    }
    else
    {
	t1 = MPI_Wtime();
	while (MPI_Wtime() - t1 < easy) ;
	a = 1;
	b = 2;
	MPI_Send(&a, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Send(&b, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    

    MPI_Finalize();
    return 0;
}
