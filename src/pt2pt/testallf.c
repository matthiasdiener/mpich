/* testall.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) a
#define MPIR_RmPointer(a)
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

 void mpi_testall_( count, array_of_requests, flag, array_of_statuses, __ierr )
int*count;
MPI_Request array_of_requests[];
int        *flag;
MPI_Status *array_of_statuses;
int *__ierr;
{
#ifdef POINTER_64_BITS
int i;
MPI_Request *r = (MPI_Request*)malloc(sizeof(MPI_Request)**count);
for (i=0; i<*count; i++) {
    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
*__ierr = MPI_Testall(*count,r,flag,array_of_statuses);
/* Must not do this if request is persistant */
for (i=0; i<*count; i++) {
    if (r[i] == MPI_REQUEST_NULL) {
        MPIR_RmPointer( *((int *)(array_of_requests) + i) );
        *((int *)(array_of_requests)+i) = 0;
        }
    }
free( r );

#else
*__ierr = MPI_Testall(*count,array_of_requests,flag,array_of_statuses);
#endif
*flag = MPIR_TO_FLOG(*flag);
}
