#include <stdio.h>
#include "mpi.h"
#include "test.h"
#include <math.h>

/*
 * This routine tests that if MPI_WTIME_IS_GLOBAL is set, the timer
 * IS in fact global.  We have some suspicions about certain vendor systems
 */
int main(argc, argv)
int argc;
char **argv;
{
    int    err = 0;
    void *v;
    int  flag;
    int  vval;
    int  rank, size, i;
    double t1, t2, t3, wtick;
    MPI_Status status;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    MPI_Attr_get( MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &v, &flag );
#ifdef DEBUG
    if (v) vval = *(int*)v; else vval = 0;
    printf( "WTIME flag = %d; val = %d\n", flag, vval );
#endif
    if (flag) {
	/* Wtime need not be set */
	vval = *(int*)v;
	if (vval < 0 || vval > 1) {
	    err++;
	    fprintf( stderr, "Invalid value for WTIME_IS_GLOBAL (got %d)\n", 
		     vval );
	}
    }
    if (flag && vval) {
	/* Wtime is global is true.  Check it */
#ifdef DEBUG
	printf( "WTIME_IS_GLOBAL\n" );
#endif	
	if (rank == 0) {
	    wtick = MPI_Wtick();
	    for (i=1; i<size; i++) {
		MPI_Send( MPI_BOTTOM, 0, MPI_INT, i, 0, MPI_COMM_WORLD );
		MPI_Recv( MPI_BOTTOM, 0, MPI_INT, i, 1, MPI_COMM_WORLD, 
			  &status );
		t1 = MPI_Wtime();
		MPI_Send( MPI_BOTTOM, 0, MPI_INT, i, 2, MPI_COMM_WORLD );
		MPI_Recv( &t2, 1, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &status );
		t3 = MPI_Wtime();
		if( fabs( 0.5 * (t1 + t3) - t2 ) > 2 * (t3 - t1 + wtick)) {
		    err++;
		    printf( "Process %d has %f; Process 0 has %f\n",
			    i, t2, 0.5 * (t1 + t3) );
		}
	    }
	    for (i=1; i<size; i++) {
		MPI_Send( MPI_BOTTOM, 0, MPI_INT, i, 3, MPI_COMM_WORLD );
	    }
	}
	else {
	    MPI_Recv( MPI_BOTTOM, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status );
	    MPI_Send( MPI_BOTTOM, 0, MPI_INT, 0, 1, MPI_COMM_WORLD );
	    MPI_Recv( MPI_BOTTOM, 0, MPI_INT, 0, 2, MPI_COMM_WORLD, &status );
	    t2 = MPI_Wtime();
	    MPI_Send( &t2, 1, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD );
	    MPI_Recv( MPI_BOTTOM, 0, MPI_INT, 0, 3, MPI_COMM_WORLD, &status );
	}
    }
    if (rank == 0) {
	if (err > 0) {
	    printf( "Errors in MPI_WTIME_IS_GLOBAL\n" );
	}
	else {
	    printf( "No errors detected in MPI_WTIME_IS_GLOBAL\n" );
	}
    }
    MPI_Finalize( );
    
    return err;
}
