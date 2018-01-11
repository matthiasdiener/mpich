/* waitall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_waitall_ PMPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ pmpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ pmpi_waitall
#else
#define mpi_waitall_ pmpi_waitall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_waitall_ MPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ mpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ mpi_waitall
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_waitall_ ANSI_ARGS(( int *, MPI_Request [], MPI_Status [], int *));

void mpi_waitall_(count, array_of_requests, array_of_statuses, __ierr )
int*count;
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
int *__ierr;
{
#ifdef POINTER_64_BITS
    int i;
    MPI_Request *r;

    if (*count > 0) {
	MPIR_FALLOC(r,(MPI_Request*)MALLOC(sizeof(MPI_Request) * *count),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Out of space in MPI_WAITALL" );

	for (i=0; i<*count; i++) {
	    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
	}

	*__ierr = MPI_Waitall(*count,r,array_of_statuses);
	/* By checking for r[i] = 0, we handle persistant requests */
	for (i=0; i<*count; i++) {
	    if (r[i] == MPI_REQUEST_NULL) {
		MPIR_RmPointer( *((int *)(array_of_requests) + i) );
		*((int *)(array_of_requests)+i) = 0;
	    }
	}
	FREE( r );
    }
    else 
	*__ierr = MPI_Waitall(*count,(MPI_Request *)0, array_of_statuses );

#else
    *__ierr = MPI_Waitall(*count,array_of_requests,array_of_statuses);
#endif
}




