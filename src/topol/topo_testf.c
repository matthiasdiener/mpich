/* topo_test.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_topo_test_ PMPI_TOPO_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_topo_test_ pmpi_topo_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_topo_test_ pmpi_topo_test
#else
#define mpi_topo_test_ pmpi_topo_test_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_topo_test_ MPI_TOPO_TEST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_topo_test_ mpi_topo_test__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_topo_test_ mpi_topo_test
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_topo_test_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_topo_test_ ( comm, top_type, __ierr )
MPI_Fint *comm;
MPI_Fint *top_type; 
MPI_Fint *__ierr;
{
    int ltop_type;
    *__ierr = MPI_Topo_test( MPI_Comm_f2c(*comm), &ltop_type);
    *top_type = (MPI_Fint)ltop_type;
}
