/* cart_sub.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_sub_ PMPI_CART_SUB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_sub_ pmpi_cart_sub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_sub_ pmpi_cart_sub
#else
#define mpi_cart_sub_ pmpi_cart_sub_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_sub_ MPI_CART_SUB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_sub_ mpi_cart_sub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_sub_ mpi_cart_sub
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_sub_ ANSI_ARGS(( MPI_Comm *, int *, int *, int * ));

void mpi_cart_sub_ ( comm, remain_dims, comm_new, __ierr )
MPI_Comm *comm;
int      *remain_dims;
int      *comm_new;
int      *__ierr;
{
    int lremain_dims[20], i, ndims;

    MPI_Cartdim_get( *comm, &ndims );
    if (ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_ptr;
	comm_ptr = MPIR_GET_COMM_PTR(*comm);
	*__ierr = MPIR_ERROR( comm_ptr, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    for (i=0; i<ndims; i++) 
	lremain_dims[i] = MPIR_FROM_FLOG(remain_dims[i]);

    *__ierr = MPI_Cart_sub( *comm, lremain_dims, comm_new );
}
