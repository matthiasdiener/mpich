/* cart_shift.c */
/* Custom Fortran interface file */
/*
 *  
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_shift_ ANSI_ARGS(( MPI_Comm *, int *, int *, int *, 
				 int *, int * ));

void mpi_cart_shift_( comm, direction, shift, source, dest, ierr )
MPI_Comm         *comm;
int              *direction; 
int              *shift;
int              *source, *dest, *ierr;
{
    *ierr =     MPI_Cart_shift( *comm, *direction, *shift, source, dest );
}
