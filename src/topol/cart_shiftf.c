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
void mpi_cart_shift_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                 MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_cart_shift_( comm, direction, shift, source, dest, ierr )
MPI_Fint *comm;
MPI_Fint *direction; 
MPI_Fint *shift;
MPI_Fint *ierr;
MPI_Fint *source; 
MPI_Fint *dest;
{
    int lsource;
    int ldest;

    *ierr =     MPI_Cart_shift( MPI_Comm_f2c(*comm), (int)*direction, 
                                (int)*shift, &lsource, &ldest );
    *source = (MPI_Fint)lsource;
    *dest = (MPI_Fint)ldest;
}
