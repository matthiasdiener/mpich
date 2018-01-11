/* cart_rank.c */
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
#define mpi_cart_rank_ PMPI_CART_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_rank_ pmpi_cart_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_rank_ pmpi_cart_rank
#else
#define mpi_cart_rank_ pmpi_cart_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_rank_ MPI_CART_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_rank_ mpi_cart_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_rank_ mpi_cart_rank
#endif
#endif

 void mpi_cart_rank_ ( comm, coords, rank, __ierr )
MPI_Comm comm;
int *coords;
int *rank;
int *__ierr;
{
*__ierr = MPI_Cart_rank(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),coords,rank);
}
