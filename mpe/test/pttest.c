#include "mpi.h"
#include <stdio.h>

/* 
   Test the Get/Return tags routines
 */

int main( argc, argv )
int  argc;
char **argv;
{
int t = 0, t1 = 0, t2 = 0, errs = 0, toterrs, rank;
MPI_Comm mycomm, mycomm2;
int rc;

MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );

if (rc = MPE_GetTags( MPI_COMM_WORLD, 10, &mycomm, &t )) {
    errs++;
    printf( "Error calling MPE_GetTags\n" );
    }
if (mycomm == MPI_COMM_WORLD) {
    errs++;
    printf( "Error, did not dup comm\n" );
    }
if (t == 0) {
    errs++;
    printf( "Error, did not get valid tags\n" );
    }
if (rc = MPE_GetTags( mycomm, 1, &mycomm2, &t1 )) {
    errs++;
    printf( "Error calling MPE_GetTags\n" );
    }
if (mycomm != mycomm2) {
    errs++;
    printf( "Error, dup'ed comm with tag\n" );
    }
if (t1 != t - 1) {
    errs++;
    printf( "Error, did not get expected tag\n" );
    }
if (rc = MPE_GetTags( MPI_COMM_WORLD, 10, &mycomm2, &t2 )) {
    errs++;
    printf( "Error calling MPE_GetTags\n" );
    }
if (t2 != t) {
    errs++;
    printf( "Error, second dup of MPI_COMM_WORLD did not give same tag\n" );
    }
if (rc = MPE_ReturnTags( mycomm2, t2, 10 )) {
    errs++;
    printf( "Error calling MPE_ReturnTags\n" );
    }
if (rc = MPE_ReturnTags( mycomm, 1, t1 )) {
    errs++;
    printf( "Error calling MPE_ReturnTags\n" );
    }
if (rc = MPE_ReturnTags( mycomm, 10, t )) {
    errs++;
    printf( "Error calling MPE_ReturnTags\n" );
    }
MPI_Comm_free( &mycomm2 );
MPI_Comm_free( &mycomm );
MPI_Reduce( &errs, &toterrs, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
if (rank == 0) {
    if (toterrs == 0) 
	printf( "No errors\n" );
    else
	printf( "** Found %d errors\n", toterrs );
    }
/* MPE_TagsEnd(); */
MPI_Finalize();
return 0;
}

