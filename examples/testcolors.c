#include <stdio.h>
#include "mpi.h"
#include "../mpe/mpe.h"

#define NCOLORS 8

main( argc, argv )
int argc;
char **argv;
{
  MPE_XGraph *handle;
  int        cidx, i, j, myid, incr, np;
  MPE_Color colorArray[NCOLORS];

  MPI_Init( &argc, &argv );
  
  MPE_Open_graphics( &handle, MPI_COMM_WORLD, (char *)0, -1, -1,
		   (NCOLORS+2)*20, 20, 0 );

  MPE_Close_graphics( &handle );
  MPI_Finalize();
  getchar();
}

