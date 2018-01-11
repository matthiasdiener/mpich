/* cart_map.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
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

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_map_ ANSI_ARGS(( MPI_Comm *, int *, int *, int *, int *, int * ));

void mpi_cart_map_ ( comm_old, ndims, dims, periods, newrank, __ierr )
MPI_Comm *comm_old;
int     *ndims;
int     *dims;
int     *periods;
int     *newrank;
int     *__ierr;
{
    int lperiods[20], i;
    if (*ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_old_ptr;
	comm_old_ptr = MPIR_GET_COMM_PTR(*comm_old);
	*__ierr = MPIR_ERROR( comm_old_ptr, MPI_ERR_LIMIT, 
			      "Too many dimensions" );
	return;
	}
    for (i=0; i<*ndims; i++) 
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);
    *__ierr = MPI_Cart_map( *comm_old, *ndims,dims,
			   lperiods,newrank);
}
