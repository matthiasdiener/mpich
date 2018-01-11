/* comm_test_ic.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

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
void mpi_comm_test_inter_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint * ));

void mpi_comm_test_inter_ ( comm, flag, __ierr )
MPI_Fint *comm;
MPI_Fint *flag;
MPI_Fint *__ierr;
{
    int l_flag;
    *__ierr = MPI_Comm_test_inter( MPI_Comm_f2c(*comm), &l_flag);
    *flag = MPIR_TO_FLOG(l_flag);
}
