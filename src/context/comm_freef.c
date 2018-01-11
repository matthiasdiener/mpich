/* comm_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_free_ PMPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free
#else
#define mpi_comm_free_ pmpi_comm_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_free_ MPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_comm_free_ ( comm, __ierr )
MPI_Fint *comm;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm = MPI_Comm_f2c(*comm);
    *__ierr = MPI_Comm_free(&l_comm);
    *comm = MPI_Comm_c2f(l_comm);
}
