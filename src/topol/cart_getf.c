/* cart_get.c */
/* Fortran interface file for sun4 */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) a
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_get_ PMPI_CART_GET
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get
#else
#define mpi_cart_get_ pmpi_cart_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_get_ MPI_CART_GET
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get
#endif
#endif

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
