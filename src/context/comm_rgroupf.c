/* comm_rgroup.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_comm_remote_group_ PMPI_COMM_REMOTE_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_group_ pmpi_comm_remote_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_group_ pmpi_comm_remote_group
#else
#define mpi_comm_remote_group_ pmpi_comm_remote_group_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_remote_group_ MPI_COMM_REMOTE_GROUP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_remote_group_ mpi_comm_remote_group__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_remote_group_ mpi_comm_remote_group
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_comm_remote_group_ ANSI_ARGS(( MPI_Comm *, MPI_Group *, int * ));

void mpi_comm_remote_group_ ( comm, group, __ierr )
MPI_Comm  *comm;
MPI_Group *group;
int *__ierr;
{
    *__ierr = MPI_Comm_remote_group( *comm, group );
}
