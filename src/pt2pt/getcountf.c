/* getcount.c */
/* Fortran interface file */
#include "mpiimpl.h"

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
#define mpi_get_count_ PMPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ pmpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ pmpi_get_count
#else
#define mpi_get_count_ pmpi_get_count_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_count_ MPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ mpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ mpi_get_count
#endif
#endif

 void mpi_get_count_( status, datatype, count, __ierr )
MPI_Status   *status;
MPI_Datatype datatype;
int          *count;
int *__ierr;
{
*__ierr = MPI_Get_count(status,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),count);
}
