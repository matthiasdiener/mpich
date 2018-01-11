/* cart_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_get_ PMPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get
#else
#define mpi_cart_get_ pmpi_cart_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_get_ MPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_get_ ANSI_ARGS(( MPI_Comm, int *, int *, int *, int *, int * ));

void mpi_cart_get_ ( comm, maxdims, dims, periods, coords, __ierr )
MPI_Comm comm;
int*maxdims;
int *dims, *periods, *coords;
int *__ierr;
{
    MPI_Comm lcomm = (MPI_Comm) MPIR_ToPointer( *((int *)comm) );
    int lperiods[20], i;
    if (*maxdims > 20) {
	*__ierr = MPIR_ERROR( lcomm, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    *__ierr = MPI_Cart_get( lcomm, *maxdims,dims,lperiods,coords);
    for (i=0; i<*maxdims; i++) 
	periods[i] = MPIR_TO_FLOG(lperiods[i]);
}

