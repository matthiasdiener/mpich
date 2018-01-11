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
void mpi_comm_split_ ANSI_ARGS(( MPI_Comm *, int *, int *, MPI_Comm *, 
				 int * ));

void mpi_comm_split_ ( comm, color, key, comm_out, __ierr )
MPI_Comm  *comm;
int*color,*key;
MPI_Comm *comm_out;
int *__ierr;
{
    *__ierr = MPI_Comm_split( *comm, *color,*key, comm_out);
}
