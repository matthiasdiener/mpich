/* testany.c */
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

void mpi_testany_( count, array_of_requests, index, flag, status, __ierr )
int*count;
MPI_Request array_of_requests[];
int         *index, *flag;
MPI_Status  *status;
int *__ierr;
{
#ifdef POINTER_64_BITS
int i;
MPI_Request *r = (MPI_Request*)malloc(sizeof(MPI_Request)**count);
for (i=0; i<*count; i++) {
    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
*__ierr = MPI_Testany(*count,r,index,flag,status);
/* Must not do this if request is persistant */
/*
   MPIR_RmPointer( *((int *)(array_of_requests) + *index) );
   *((int *)(array_of_requests)+*index) = 0;
 */
free( r );

#else
*__ierr = MPI_Testany(*count,array_of_requests,index,flag,status);
#endif
*flag = MPIR_TO_FLOG(*flag);
/* See the description of waitany in the standard; the Fortran index ranges
   are from 1, not zero */
if (*index >= 0) *index = *index + 1;
}
