/* comm_compare.c */
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
#define mpi_comm_compare_ PMPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_compare_ pmpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_compare_ pmpi_comm_compare
#else
#define mpi_comm_compare_ pmpi_comm_compare_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_compare_ MPI_COMM_COMPARE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_compare_ mpi_comm_compare__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_compare_ mpi_comm_compare
#endif
#endif

 void mpi_comm_compare_ ( comm1, comm2, result, __ierr )
MPI_Comm  comm1;
MPI_Comm  comm2;
int       *result;
int *__ierr;
{
*__ierr = MPI_Comm_compare(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm1) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm2) ),result);
}
