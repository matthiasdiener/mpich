/* comm_compare.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_compare_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint * ));

void mpi_comm_compare_ ( comm1, comm2, result, __ierr )
MPI_Fint *comm1;
MPI_Fint *comm2;
MPI_Fint *result;
MPI_Fint *__ierr;
{
    int l_result;

    *__ierr = MPI_Comm_compare( MPI_Comm_f2c(*comm1), 
                                MPI_Comm_f2c(*comm2), &l_result);
    *result = l_result;
}
