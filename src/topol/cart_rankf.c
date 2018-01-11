/* cart_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
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

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_rank_ ANSI_ARGS(( MPI_Comm, int *, int *, int * ));
void mpi_cart_rank_ ( comm, coords, rank, __ierr )
MPI_Comm comm;
int *coords;
int *rank;
int *__ierr;
{
    *__ierr = MPI_Cart_rank(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),coords,rank);
}
