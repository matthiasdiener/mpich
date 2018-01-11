/* cart_shift.c */
/* Custom Fortran interface file */
/*
 *  
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

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
#define mpi_cart_shift_ PMPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_shift_ pmpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_shift_ pmpi_cart_shift
#else
#define mpi_cart_shift_ pmpi_cart_shift_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_shift_ MPI_CART_SHIFT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_shift_ mpi_cart_shift__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_shift_ mpi_cart_shift
#endif
#endif

void mpi_cart_shift_( comm, direction, shift, source, dest )
MPI_Comm         comm;
int              *direction; 
int              *shift;
int              *source, *dest;
{
    MPI_Cart_shift( (MPI_Comm)MPIR_ToPointer(*(int*)comm), 
		    *direction, *shift, source, dest );
}
