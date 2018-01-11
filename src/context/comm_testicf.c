/* comm_test_ic.c */
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
#define mpi_comm_test_inter_ PMPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_test_inter_ pmpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_test_inter_ pmpi_comm_test_inter
#else
#define mpi_comm_test_inter_ pmpi_comm_test_inter_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_test_inter_ MPI_COMM_TEST_INTER
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_test_inter_ mpi_comm_test_inter__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_test_inter_ mpi_comm_test_inter
#endif
#endif

 void mpi_comm_test_inter_ ( comm, flag, __ierr )
MPI_Comm  comm;
int      *flag;
int *__ierr;
{
*__ierr = MPI_Comm_test_inter(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),flag);
}
