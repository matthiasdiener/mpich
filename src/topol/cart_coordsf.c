/* cart_coords.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_coords_ PMPI_CART_COORDS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_coords_ pmpi_cart_coords__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_coords_ pmpi_cart_coords
#else
#define mpi_cart_coords_ pmpi_cart_coords_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_coords_ MPI_CART_COORDS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_coords_ mpi_cart_coords__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_coords_ mpi_cart_coords
#endif
#endif

 void mpi_cart_coords_ ( comm, rank, maxdims, coords, __ierr )
MPI_Comm  comm;
int*rank;
int*maxdims;
int      *coords;
int *__ierr;
{
*__ierr = MPI_Cart_coords(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*rank,*maxdims,coords);
}
