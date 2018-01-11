/* 
 *    $Id: first++.C,v 1.1 1994/05/12 08:22:06 doss Exp $ 
 */

#include <mpi++.h>
#include <iostream.h>

main (int argc, char **argv)
{
  int rank, to, from, tag, count, np;
  double data[500];
  int src, dest;
  int st_source, st_tag, st_count;
  MPI_Request handle;
  MPI_Status status;
 
  MPI_COMM_WORLD.Init( argc, argv );
  MPI_COMM_WORLD.Rank( rank );
  cout << "Process " << rank << " is alive!" << endl;
  
  src  = 1;
  dest = 0;
 
  if (rank == src)
  {
	to      = dest;
	count   = 127;          /* one int, that is */
	tag     = 2000;
	data[0] = 777;
	MPI_COMM_WORLD.Send( data, count, MPI_DOUBLE, to, tag );
	cout << rank << " sent " << data[0] << endl;
  }
  else if (rank == dest) {
	tag   = 2000;
	count = 127;            /* one int */
	from  = MPI_ANY_SOURCE;
	MPI_COMM_WORLD.Irecv(data, count, MPI_DOUBLE, from, tag, handle );
	MPI_Wait( &handle, &status );
	st_source = status.MPI_SOURCE;
	st_tag    = status.MPI_TAG;
	/* MPI_Get_count(  &status, MPI_DOUBLE, &st_count ); */
	
	cout << "Status info: source = " << st_source << "tag = " << st_tag << endl;
	cout << rank << " received " << data[0] << endl;
  }
 
  // End it
  MPI_COMM_WORLD.Finalize();
  cout << "Process " << rank << " exiting" << endl;
  return (MPI_SUCCESS);
}
