#include "mpi.h"
#define SIZE 10

int main(argc,argv)
int argc;
char *argv[];
{
  int num_procs,my_id;
  int buf[SIZE][SIZE];
  int sendto, from;
  MPI_Status status;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
  
  sendto = (my_id + 1) % num_procs;
  from   = (my_id + num_procs - 1) % num_procs;
  printf("%d   Send to %d   Recv from %d\n",my_id,sendto,from);

  MPI_Sendrecv_replace(buf, SIZE*SIZE, MPI_INT,
		       sendto, my_id,
		       from, from,
		       MPI_COMM_WORLD, &status); 

  printf("%d Done ....\n",my_id);

  MPI_Finalize();
}
