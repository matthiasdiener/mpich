/* waitsome.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_waitsome_ PMPI_WAITSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitsome_ pmpi_waitsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitsome_ pmpi_waitsome
#else
#define mpi_waitsome_ pmpi_waitsome_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_waitsome_ MPI_WAITSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitsome_ mpi_waitsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitsome_ mpi_waitsome
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_waitsome_ ANSI_ARGS(( int *, MPI_Request [], int *, int [],
			       MPI_Status [], int * ));

void mpi_waitsome_( incount, array_of_requests, outcount, array_of_indices, 
    array_of_statuses, __ierr )
int*incount, *outcount, array_of_indices[];
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
int *__ierr;
{
    int i;
#ifdef POINTER_64_BITS
    MPI_Request *r;

    MPIR_FALLOC(r,(MPI_Request*)MALLOC(sizeof(MPI_Request)* *incount),
		MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		"Out of space in MPI_WAITSOME" );

    for (i=0; i<*incount; i++) {
	r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
    *__ierr = MPI_Waitsome(*incount,r,outcount,array_of_indices,
			   array_of_statuses);
/* By checking for r[a[i]] = 0, we handle persistant requests */
    for (i=0; i<*outcount; i++) {
	if (array_of_indices[i] >= 0) {
	    if (r[array_of_indices[i]] == 0) {
		MPIR_RmPointer( *((int *)(array_of_requests) + 
				  array_of_indices[i]) );
		*((int *)(array_of_requests)+array_of_indices[i]) = 0;
	    }
	}
    }
    FREE( r );

#else
    *__ierr = MPI_Waitsome(*incount,array_of_requests,outcount,
			   array_of_indices,array_of_statuses);
#endif
    for (i=0; i<*outcount; i++) {
	if (array_of_indices[i] >= 0)
	    array_of_indices[i] = array_of_indices[i] + 1;
    }
}
