#include <stdio.h>

#include "mpi.h"
extern int __NUMNODES, __MYPROCID;

/*
   Utility programs for mpptest
 */
int GetRepititions( double, double, int, int, int, int );
int ComputeGoodReps( double t1, int len1, double t2, int len2, int len );

/* 
    T1 is time to send len1,
    T2 is time to send len2, 
    len is lenght we'd like the number of repititions for
    reps (input) is the default to use
 */
int GetRepititions( double T1, double T2, int Len1, int Len2, int len, 
		    int reps )
{
    if (__MYPROCID == 0) {
	if (T1 > 0 && T2 > 0) 
	    reps = ComputeGoodReps( T1, Len1, T2, Len2, len );
    }
    MPI_Bcast(&reps, 1, MPI_INT, 0, MPI_COMM_WORLD );
    return reps;
}
