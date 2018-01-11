/* startall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
#else
#include "mpisys.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_startall_ PMPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ pmpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ pmpi_startall
#else
#define mpi_startall_ pmpi_startall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_startall_ MPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ mpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ mpi_startall
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_startall_ ANSI_ARGS(( int *, MPI_Request [], int * ));

void mpi_startall_( count, array_of_requests, __ierr )
int*count;
MPI_Request array_of_requests[];
int *__ierr;
{
#ifdef POINTER_64_BITS
    int i;
    MPI_Request *r;

    if (*count > 0) {
	MPIR_FALLOC(r,(MPI_Request*)MALLOC(sizeof(MPI_Request) * *count),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Out of space in MPI_STARTALL" );
	for (i=0; i<*count; i++) {
	    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
	}
	*__ierr = MPI_Startall(*count,r);
	FREE( r );
    }
    else 
	*__ierr = MPI_Startall(*count,(MPI_Request *)0);
#else
    *__ierr = MPI_Startall(*count,array_of_requests);
#endif
}


