/* 	$Id: grouptest.c,v 1.6 1994/10/24 22:03:20 gropp Exp $	 */

#ifndef lint
static char vcid[] = "$Id: grouptest.c,v 1.6 1994/10/24 22:03:20 gropp Exp $";
#endif /* lint */
#include "mpi.h"

/*
    grouptest -
*/
int main( argc, argv )
int argc;
char **argv;
{
    int rank, size, i;
    MPI_Group group1, group2, group3, groupall, groupunion, groupintersection, newgroup;
    MPI_Comm newcomm;
    int ranks1[100], ranks2[100], ranks3[100];
    int nranks1=0, nranks2=0, nranks3=0;
    int grouprank;

    MPI_Init( &argc, &argv );
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_group( MPI_COMM_WORLD, &groupall );

    /* Divide groups */
    for (i=0; i<size; i++) 
      if ( (i%3)==0 )
	ranks1[nranks1++] = i;
      else if ( (i%3)==1 )
	ranks2[nranks2++] = i;
      else
	ranks3[nranks3++] = i;

    MPI_Group_incl ( groupall, nranks1, ranks1, &group1 );
    MPI_Group_incl ( groupall, nranks2, ranks2, &group2 );
    MPI_Group_incl ( groupall, nranks3, ranks3, &group3 );

    MPI_Group_difference ( groupall, group2, &groupunion );

    MPI_Comm_create ( MPI_COMM_WORLD, group3, &newcomm );
    if (newcomm != MPI_COMM_NULL)
    {
	/* If we don't belong to group3, this would fail */
	MPI_Comm_group ( newcomm, &newgroup );
    }

    printf("Successful.\n");
    MPI_Finalize();
    return 0;
}


