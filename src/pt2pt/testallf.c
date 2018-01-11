/* testall.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_testall_ PMPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testall_ pmpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testall_ pmpi_testall
#else
#define mpi_testall_ pmpi_testall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_testall_ MPI_TESTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testall_ mpi_testall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testall_ mpi_testall
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_testall_ ANSI_ARGS(( int *, MPI_Request [], int *, MPI_Status [],
			      int * ));

void mpi_testall_( count, array_of_requests, flag, array_of_statuses, __ierr )
int*count;
MPI_Request array_of_requests[];
int        *flag;
MPI_Status *array_of_statuses;
int *__ierr;
{
#ifdef POINTER_64_BITS
    int i;
    MPI_Request *r;

    if (*count > 0) {
	MPIR_FALLOC(r,(MPI_Request*)MALLOC(sizeof(MPI_Request)* *count),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Out of space in MPI_TESTALL" );

	for (i=0; i<*count; i++) {
	    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
	}
	*__ierr = MPI_Testall(*count,r,flag,array_of_statuses);
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
	*__ierr = MPI_Testall(*count,(MPI_Request *)0,flag,array_of_statuses);

#else
    *__ierr = MPI_Testall(*count,array_of_requests,flag,array_of_statuses);
#endif
    *flag = MPIR_TO_FLOG(*flag);
}

