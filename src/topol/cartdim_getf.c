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
void mpi_cartdim_get_ ANSI_ARGS(( MPI_Comm *, int *, int * ));

void mpi_cartdim_get_ ( comm, ndims, __ierr )
MPI_Comm  *comm;
int      *ndims;
int *__ierr;
{
    *__ierr = MPI_Cartdim_get( *comm, ndims );
}
