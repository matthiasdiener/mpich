#include <mpi.h>
#include "mpe.h"
#include <stdio.h>

static MPE_XGraph *graph1, *graph2;
static int width = 400, height = 400;

int main(argc, argv)
int argc ;
char *argv[] ;
{
  int      rank, size ;

  MPI_Init (&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size) ;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank) ;

  /* Open the graphics display */
  MPE_OpenGraphics( &graph1, MPI_COMM_WORLD, (char *)0, 
		    -1, -1, width, height, 0 );

  MPI_Barrier( MPI_COMM_WORLD );
  MPE_OpenGraphics( &graph2, MPI_COMM_WORLD, (char *)0, 
		    -1, -1, width, height, 0 );

  if (rank != 0) {
      MPE_DrawLine( graph1, 0, 0, width, height, MPE_BLACK );
      MPE_DrawLine( graph2, 0, height-1, width, 0, MPE_BLACK );
      MPE_Update( graph1 );
      MPE_Update( graph2 );
      }
  if (rank == 0) {
      printf( "Press return to exit...\n" );
      getc( stdin );
      }
  MPE_CloseGraphics( graph1 );
  MPE_CloseGraphics( graph2 );
  MPI_Finalize();
}
