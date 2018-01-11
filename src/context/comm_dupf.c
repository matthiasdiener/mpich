/* comm_dup.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_dup_ PMPI_COMM_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_dup_ pmpi_comm_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_dup_ pmpi_comm_dup
#else
#define mpi_comm_dup_ pmpi_comm_dup_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_dup_ MPI_COMM_DUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_dup_ mpi_comm_dup__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_dup_ mpi_comm_dup
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_dup_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_comm_dup_ ( comm, comm_out, __ierr )
MPI_Fint *comm; 
MPI_Fint *comm_out;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm_out;

    *__ierr = MPI_Comm_dup( MPI_Comm_f2c(*comm), &l_comm_out );
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
