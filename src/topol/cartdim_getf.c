/* cartdim_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cartdim_get_ PMPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get
#else
#define mpi_cartdim_get_ pmpi_cartdim_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cartdim_get_ MPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cartdim_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_cartdim_get_ ( comm, ndims, __ierr )
MPI_Fint *comm;
MPI_Fint *ndims;
MPI_Fint *__ierr;
{
    int lndims;

    *__ierr = MPI_Cartdim_get( MPI_Comm_f2c(*comm), &lndims );
    *ndims = (MPI_Fint)lndims;
}
