/*
 *  $Id: cart_createf.c,v 1.14 1996/12/09 20:44:15 gropp Exp $
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 * Custom Fortran wrapper
 */

#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_create_ PMPI_CART_CREATE 
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_create_ pmpi_cart_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_create_ pmpi_cart_create
#else
#define mpi_cart_create_ pmpi_cart_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_create_ MPI_CART_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_create_ mpi_cart_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_create_ mpi_cart_create
#endif
#endif

/*
MPI_Cart_create - Make a new communicator to which topology information
                  has been attached

*/
/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_create_ ANSI_ARGS(( MPI_Comm *, int *, int *, int *, int *, 
				  MPI_Comm *, int * ));

void mpi_cart_create_ ( comm_old, ndims, dims, periods, reorder, comm_cart, 
		      ierr ) 
MPI_Comm         *comm_old;
int              *ndims;     
int              *dims;     
int              *periods;
int              *reorder;
MPI_Comm         *comm_cart;
int              *ierr;
{
    int lperiods[20], i;

    if (*ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_old_ptr;
	comm_old_ptr = MPIR_GET_COMM_PTR(*comm_old);
	*ierr = MPIR_ERROR( comm_old_ptr, MPI_ERR_LIMIT, 
			    "Too many dimensions" );
	return;
	}
    for (i=0; i<*ndims; i++) 
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);

    *ierr = MPI_Cart_create( *comm_old, 
			     *ndims, dims, 
			     lperiods, MPIR_FROM_FLOG(*reorder), 
			     comm_cart ); 
}
