#include <mpi.h>

main(argc, argv)
int argc;
char **argv;
{
  int rank, new_world_rank, size, order;
  int tmpint = 0;
  MPI_Comm new_world;
  MPI_Status s;

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  order = size - rank - 1;
  MPI_Comm_split(MPI_COMM_WORLD, 0, order, &new_world);
	
  MPI_Comm_rank ( new_world, &new_world_rank );

  if (new_world_rank==0) {
     MPI_Send(&tmpint, 1, MPI_INT, 1, 0, new_world);
     printf("%d(%d): Sent message to: %d\n", new_world_rank, rank, 1);
  }
  else if (new_world_rank == 1) {
     MPI_Recv(&tmpint, 1, MPI_INT, 0, 0, new_world,&s);
     printf("%d(%d): Recv message from: -> %d(%d) <- these 2 should equal\n", 
	     new_world_rank, rank, 0, s.MPI_SOURCE);

  }
  MPI_Comm_free( &new_world );
  Test_Waitforall( );
  MPI_Finalize();
}
