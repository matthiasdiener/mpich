/* ic_merge.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ PMPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge
#else
#define mpi_intercomm_merge_ pmpi_intercomm_merge_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ MPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_intercomm_merge_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                      MPI_Fint * ));

void mpi_intercomm_merge_ ( comm, high, comm_out, __ierr )
MPI_Fint *comm;
MPI_Fint *high;
MPI_Fint *comm_out;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm_out;

    *__ierr = MPI_Intercomm_merge( MPI_Comm_f2c(*comm), (int)*high, 
                                   &l_comm_out);
    *comm_out = MPI_Comm_c2f(l_comm_out);
}
