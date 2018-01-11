/* comm_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_rank_ PMPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ pmpi_comm_rank
#else
#define mpi_comm_rank_ pmpi_comm_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_rank_ MPI_COMM_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_rank_ mpi_comm_rank
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_rank_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_comm_rank_ ( comm, rank, __ierr )
MPI_Fint *comm;
MPI_Fint *rank;
MPI_Fint *__ierr;
{
    int l_rank;
    *__ierr = MPI_Comm_rank( MPI_Comm_f2c(*comm), &l_rank);
    *rank = (MPI_Fint)l_rank;
}
