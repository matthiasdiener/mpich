
#include <stdio.h>
#include "mpi.h"

int main( int argc, char **args )
{
  int mytid;
    printf("doing mpi_init\n");
    MPI_Init(&argc,&args);

MPI_Comm_rank(MPI_COMM_WORLD,&mytid);
if (mytid < 2) exit(0);
    MPI_Finalize();
}
