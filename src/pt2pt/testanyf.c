/* testany.c */
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
#define mpi_testany_ PMPI_TESTANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testany_ pmpi_testany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testany_ pmpi_testany
#else
#define mpi_testany_ pmpi_testany_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_testany_ MPI_TESTANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testany_ mpi_testany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testany_ mpi_testany
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_testany_ ANSI_ARGS(( int *, MPI_Request [], int *, int *, 
			      MPI_Status *, int * ));
void mpi_testany_( count, array_of_requests, index, flag, status, __ierr )
int*count;
MPI_Request array_of_requests[];
int         *index, *flag;
MPI_Status  *status;
int *__ierr;
{
#ifdef POINTER_64_BITS
    int i;
    MPI_Request *r;

    if (*count > 0) {
	MPIR_FALLOC(r,(MPI_Request*)MALLOC(sizeof(MPI_Request) * *count),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Out of space in MPI_TESTANY" );
	for (i=0; i<*count; i++) {
	    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
	}
    }
    else 
	r = 0;

    *__ierr = MPI_Testany(*count,r,index,flag,status);
    if (*flag && !*__ierr) {
	/* By checking for r[i] = 0, we handle persistant requests */
	if (r[*index] == MPI_REQUEST_NULL) {
	    MPIR_RmPointer( *((int *)(array_of_requests) + *index) );
	    *((int *)(array_of_requests)+*index) = 0;
	}
    }
    if (r) {
	FREE( r );
    }

#else
    *__ierr = MPI_Testany(*count,array_of_requests,index,flag,status);
#endif

    *flag = MPIR_TO_FLOG(*flag);
    /* See the description of waitany in the standard; the Fortran index ranges
       are from 1, not zero */
    if (*index >= 0) *index = *index + 1;
}



