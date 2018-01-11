/* comm_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_size_ PMPI_COMM_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_size_ pmpi_comm_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_size_ pmpi_comm_size
#else
#define mpi_comm_size_ pmpi_comm_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_size_ MPI_COMM_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_size_ mpi_comm_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_size_ mpi_comm_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_size_ ANSI_ARGS(( MPI_Fint *, int *, MPI_Fint * ));

void mpi_comm_size_ ( comm, size, __ierr )
MPI_Fint *comm;
MPI_Fint *size;
MPI_Fint *__ierr;
{
    int l_size;

    *__ierr = MPI_Comm_size( MPI_Comm_f2c(*comm), &l_size );
    *size = (MPI_Fint)l_size;
}

