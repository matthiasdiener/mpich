/* waitany.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpisys.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_waitany_ PMPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitany_ pmpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitany_ pmpi_waitany
#else
#define mpi_waitany_ pmpi_waitany_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_waitany_ MPI_WAITANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitany_ mpi_waitany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitany_ mpi_waitany
#endif
#endif

 void mpi_waitany_(count, array_of_requests, index, status, __ierr )
int*count;
MPI_Request array_of_requests[];
int         *index;
MPI_Status  *status;
int *__ierr;
{
#ifdef POINTER_64_BITS
int i;
MPI_Request *r = (MPI_Request*)MALLOC(sizeof(MPI_Request)* *count);
if (!r) {
    *__ierr = MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_WAITANY" );
    return;
    }
for (i=0; i<*count; i++) {
    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
*__ierr = MPI_Waitany(*count,r,index,status);
if (!*__ierr) {
    /* By checking for r[i] = 0, we handle persistant requests */
    if (r[*index] == MPI_REQUEST_NULL) {
	MPIR_RmPointer( *((int *)(array_of_requests) + *index) );
	*((int *)(array_of_requests)+*index) = 0;
	}
    }
FREE( r );

#else
*__ierr = MPI_Waitany(*count,array_of_requests,index,status);
#endif

/* See the description of waitany in the standard; the Fortran index ranges
   are from 1, not zero */
if (*index >= 0) *index = *index + 1;
}
