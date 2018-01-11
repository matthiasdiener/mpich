/* comm_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_create_ PMPI_COMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_create_ pmpi_comm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_create_ pmpi_comm_create
#else
#define mpi_comm_create_ pmpi_comm_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_create_ MPI_COMM_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_create_ mpi_comm_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_create_ mpi_comm_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_create_ ANSI_ARGS(( MPI_Comm *, MPI_Group *, MPI_Comm *, 
				  int * ));

void mpi_comm_create_ ( comm, group, comm_out, __ierr )
MPI_Comm  *comm;
MPI_Group *group;
MPI_Comm *comm_out;
int *__ierr;
{
    *__ierr = MPI_Comm_create( *comm,*group,comm_out);
}
