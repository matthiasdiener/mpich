/* comm_test_ic.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifndef POINTER_64_BITS
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

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_test_inter_ ANSI_ARGS(( MPI_Comm, int *, int * ));

void mpi_comm_test_inter_ ( comm, flag, __ierr )
MPI_Comm  comm;
int      *flag;
int *__ierr;
{
    *__ierr = MPI_Comm_test_inter(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),flag);
    *flag = MPIR_TO_FLOG(*flag);
}
