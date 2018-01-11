#include "mpi.h"
#define BUFSIZE 100000
main(int argc, char **argv)
{
  int myid, nnodes, size;
  double start_time, end_time, elapsed_time;
  char buffer[BUFSIZE];
  MPI_Request request;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nnodes);
  printf("node %d of %d checking in\n", myid, nnodes);
  for (size = 1; size < BUFSIZE; size*=2) {
    if (myid == 0) {
      printf("size = %d\n", size);

      printf("node 0 sending mpi_isend\n");
      MPI_Isend(buffer, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &request);
      printf("node 0 sent mpi_isend. waiting\n");
      MPI_Wait(&request, &status);
      printf("node 0 finished waiting\n");
    } else if (myid == 1) {
      printf("node 1 receiving\n");
      MPI_Recv(buffer, size, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &statu
s);
      printf("node 1 received\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  exit(0);

}
