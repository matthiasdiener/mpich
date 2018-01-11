/* comm_rsize.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_remote_size_ PMPI_COMM_REMOTE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_size_ pmpi_comm_remote_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_size_ pmpi_comm_remote_size
#else
#define mpi_comm_remote_size_ pmpi_comm_remote_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_remote_size_ MPI_COMM_REMOTE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_size_ mpi_comm_remote_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_size_ mpi_comm_remote_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_remote_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                       MPI_Fint * ));

void mpi_comm_remote_size_ ( comm, size, __ierr )
MPI_Fint *comm;
MPI_Fint *size;
MPI_Fint *__ierr;
{
    int l_size;

    *__ierr = MPI_Comm_remote_size( MPI_Comm_f2c(*comm), &l_size);
    *size = l_size;
}
