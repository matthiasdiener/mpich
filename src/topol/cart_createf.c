/*
 *  $Id: cart_createf.c,v 1.3 1998/01/29 14:29:22 gropp Exp $
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 * Custom Fortran wrapper
 */

#include "mpiimpl.h"
#include "mpifort.h"

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
void mpi_cart_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint * ));

void mpi_cart_create_ (comm_old, ndims, dims, periods, reorder, comm_cart, 
		      ierr ) 
MPI_Fint *comm_old;
MPI_Fint *ndims;     
MPI_Fint *dims;     
MPI_Fint *periods;
MPI_Fint *reorder;
MPI_Fint *comm_cart;
MPI_Fint *ierr;
{
    MPI_Comm l_comm_cart;
    int lperiods[20];
    int ldims[20];
    int i;

    if ((int)*ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_old_ptr;
	comm_old_ptr = MPIR_GET_COMM_PTR(*comm_old);
	*ierr = MPIR_ERROR( comm_old_ptr, MPI_ERR_LIMIT, 
			    "Too many dimensions" );
	return;
	}
    for (i=0; i<(int)*ndims; i++) {
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);
	ldims[i] = (int)dims[i];
    }

    *ierr = MPI_Cart_create( MPI_Comm_f2c(*comm_old), 
			     (int)*ndims, ldims, 
			     lperiods, MPIR_FROM_FLOG(*reorder), 
			     &l_comm_cart);
    *comm_cart = MPI_Comm_c2f(l_comm_cart);
}
