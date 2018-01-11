#include "mpi.h"

main(int argc, char **argv)
{
    MPI_Status stat;
    int numprocs, myid;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (myid == 0)
    {
	for (i = 1; i < numprocs; i++)
	{
	    MPI_Recv((void *)&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
	    printf("got %d from node %d\n", buf, stat.MPI_SOURCE);
	}
    }
    else
    {
	MPI_Send((void *)&buf, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
    }
}
