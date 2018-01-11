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
void mpi_comm_remote_size_ ANSI_ARGS(( MPI_Comm *, int *, int * ));

void mpi_comm_remote_size_ ( comm, size, __ierr )
MPI_Comm  *comm;
int      *size;
int *__ierr;
{
    *__ierr = MPI_Comm_remote_size( *comm, size);
}
