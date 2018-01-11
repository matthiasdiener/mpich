/* attr_delval.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_attr_delete_ PMPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete
#else
#define mpi_attr_delete_ pmpi_attr_delete_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_delete_ MPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_attr_delete_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_attr_delete_ ( comm, keyval, __ierr )
MPI_Fint *comm;
MPI_Fint *keyval;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Attr_delete( MPI_Comm_f2c(*comm), (int)*keyval);
}
