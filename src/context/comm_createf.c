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
void mpi_comm_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
				  MPI_Fint * ));

void mpi_comm_create_ ( comm, group, comm_out, __ierr )
MPI_Fint *comm;
MPI_Fint *group;
MPI_Fint *comm_out;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm_out;

    *__ierr = MPI_Comm_create( MPI_Comm_f2c(*comm), MPI_Group_f2c(*group),
                               &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
