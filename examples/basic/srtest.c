#include "mpi.h"
#include <stdio.h>

#define BUFLEN 512

int main(argc,argv)
int argc;
char *argv[];
{
    int i, myid, numprocs, rc;
    char buffer[BUFLEN];
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    strcpy(buffer,"hello there");
    if (myid == 0)
    {
	p3_dprintf("sending %s \n",buffer);
	MPI_Send(buffer, strlen(buffer)+1, MPI_CHAR, 1, 99, MPI_COMM_WORLD);
	p3_dprintf("receiving \n");
	MPI_Recv(buffer, BUFLEN, MPI_CHAR, 1, 99, MPI_COMM_WORLD, &status);
	p3_dprintf("received %s \n",buffer);
    }
    else
    {
	p3_dprintf("receiving  \n");
	MPI_Recv(buffer, BUFLEN, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
	p3_dprintf("received %s \n",buffer);
	MPI_Send(buffer, strlen(buffer)+1, MPI_CHAR, 0, 99, MPI_COMM_WORLD);
	p3_dprintf("sent %s \n",buffer);
    }
    MPI_Finalize();
}
