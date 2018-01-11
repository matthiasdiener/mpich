/* cart_sub.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_sub_ PMPI_CART_SUB
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_sub_ pmpi_cart_sub
#else
#define mpi_cart_sub_ pmpi_cart_sub_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_sub_ MPI_CART_SUB
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_sub_ mpi_cart_sub
#endif
#endif

 void mpi_cart_sub_ ( comm, remain_dims, comm_new, __ierr )
MPI_Comm comm;
int      *remain_dims;
int      *comm_new;
int      *__ierr;
{
    int lremain_dims[20], i, ndims;
    MPI_Comm lcomm = (MPI_Comm) MPIR_ToPointer(*((int*)comm));
    MPI_Comm lcomm_new;

    MPI_Cartdim_get( lcomm, &ndims );
    if (ndims > 20) {
	*__ierr = MPIR_ERROR( lcomm, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    for (i=0; i<ndims; i++) 
	lremain_dims[i] = MPIR_FROM_FLOG(remain_dims[i]);

    *__ierr = MPI_Cart_sub( lcomm, lremain_dims, &lcomm_new );
    if (*__ierr == 0) 
	*(int *)comm_new = MPIR_FromPointer( lcomm_new );
}
