/* comm_split.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_split_ PMPI_COMM_SPLIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_split_ pmpi_comm_split__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_split_ pmpi_comm_split
#else
#define mpi_comm_split_ pmpi_comm_split_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_split_ MPI_COMM_SPLIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_split_ mpi_comm_split__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_split_ mpi_comm_split
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_split_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint * ));

void mpi_comm_split_ ( comm, color, key, comm_out, __ierr )
MPI_Fint *comm;
MPI_Fint *color, *key;
MPI_Fint *comm_out;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm_out;

    *__ierr = MPI_Comm_split( MPI_Comm_f2c(*comm), (int)*color, (int)*key, 
                              &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
