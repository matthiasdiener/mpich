/* waitall.c */
/* Custom Fortran interface file */
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

 void mpi_waitall_(count, array_of_requests, array_of_statuses, __ierr )
int*count;
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
int *__ierr;
{
#ifdef POINTER_64_BITS
int i;
MPI_Request *r = (MPI_Request*)MALLOC(sizeof(MPI_Request) * *count);
if (!r) {
    *__ierr = MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_WAITALL" );
    return;
    }
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

#else
*__ierr = MPI_Waitall(*count,array_of_requests,array_of_statuses);
#endif
}




