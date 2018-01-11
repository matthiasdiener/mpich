/*
 *  $Id: cart_createf.c,v 1.9 1995/05/09 18:56:11 gropp Exp $
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 * Custom Fortran wrapper
 */

#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)a
#define MPIR_RmPointer(a)
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

void mpi_cart_create_ ( comm_old, ndims, dims, periods, reorder, comm_cart, 
		      ierr ) 
MPI_Comm         comm_old;
int              *ndims;     
int              *dims;     
int              *periods;
int              *reorder;
MPI_Comm         *comm_cart;
int              *ierr;
{
    int lperiods[20], i;
    MPI_Comm lcomm_old = (MPI_Comm) MPIR_ToPointer( *((int *)comm_old) );
    MPI_Comm lcomm_cart;

    if (*ndims > 20) {
	*ierr = MPIR_ERROR( lcomm_old, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    for (i=0; i<*ndims; i++) 
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);

    *ierr = MPI_Cart_create( lcomm_old, 
			     *ndims, dims, 
			     lperiods, MPIR_FROM_FLOG(*reorder), 
			     &lcomm_cart ); 
    if (*ierr == 0) 
	*(int *)comm_cart = MPIR_FromPointer( lcomm_cart );
        
}
