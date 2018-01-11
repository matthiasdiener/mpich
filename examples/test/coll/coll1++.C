/* 
 *    $Id: coll1++.C,v 1.1 1994/05/12 08:22:06 doss Exp $ 
 */

#include <mpi++.h>
#include <stdio.h>

main (int argc, char **argv)
{
  int size, rank, i;
  MPI_Comm_world   world;
  MPI_Request      handle;
  MPI_Status       status;
  int             *table, errors=0;

  // Initialize and get info about world
  world.Init(argc, argv);
  world.Size(size);
  world.Rank(rank);
 
  // Make data table
  table = new int[size];
  table[rank] = rank + 1;
 
  // The first barrier
  world.Barrier();
 
  // Broadcast the data
  for ( i=0; i<size; i++ )
	world.Bcast( &table[i], 1, MPI_INT, i );
 
  // See if we have the correct answers 
  for ( i=0; i<size; i++ )
	if (table[i] != i+1) errors++;
 
  // The last barrier
  world.Barrier();
 
  // End it
  world.Finalize();
  delete table;

  if (errors)
	printf( "[%d] done with ERRORS!\n", rank );
  else
	printf( "[%d] SUCCESSFUL!\n", rank );

  return (MPI_SUCCESS);
}
