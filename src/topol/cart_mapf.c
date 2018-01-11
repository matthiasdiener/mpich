/* cart_map.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_map_ PMPI_CART_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_map_ pmpi_cart_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_map_ pmpi_cart_map
#else
#define mpi_cart_map_ pmpi_cart_map_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_map_ MPI_CART_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_map_ mpi_cart_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_map_ mpi_cart_map
#endif
#endif

 void mpi_cart_map_ ( comm_old, ndims, dims, periods, newrank, __ierr )
MPI_Comm comm_old;
int*ndims;
int     *dims;
int     *periods;
int     *newrank;
int *__ierr;
{
    int lperiods[20], i;
    if (*ndims > 20) {
	*__ierr = MPIR_ERROR( comm_old, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    for (i=0; i<*ndims; i++) 
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);
    *__ierr = MPI_Cart_map(
	(MPI_Comm)MPIR_ToPointer(*((int*)comm_old)),*ndims,dims,
			   lperiods,newrank);
}
