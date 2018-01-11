/*
 *  $Id: type_hvec.c,v 1.20 1996/04/11 20:25:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "sbcnst2.h"
#define MPIR_SBalloc MPID_SBalloc
/* pt2pt for MPIR_Type_dup */
#include "mpipt2pt.h"
#else
#include "mpisys.h"
#endif

/*@
    MPI_Type_hvector - Creates a vector (strided) datatype with offset in bytes

Input Parameters:
. count - number of blocks (nonnegative integer) 
. blocklength - number of elements in each block 
(nonnegative integer) 
. stride - number of bytes between start of each block (integer) 
. old_type - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE
.N MPI_ERR_COUNT
.N MPI_ERR_ARG
.N MPI_ERR_EXHAUSTED
@*/
int MPI_Type_hvector( count, blocklen, stride, old_type, newtype )
int          count;
int 	     blocklen;
MPI_Aint     stride;
MPI_Datatype old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  int           mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  MPIR_GET_REAL_DATATYPE(old_type)
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (count   <  0)                  && (mpi_errno = MPI_ERR_COUNT) ) ||
   ( (blocklen <  0)                 && (mpi_errno = MPI_ERR_ARG) )   ||
   ( (old_type->dte_type == MPIR_UB) && (mpi_errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (mpi_errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_TYPE_HVECTOR" );

  /* Are we making a null datatype? */
  if (count*blocklen == 0) {
      return MPI_Type_contiguous( 0, MPI_INT, newtype );
      }
	
  /* Handle the case where blocklen & stride make a contiguous type */
  if ( ((blocklen * old_type->extent) == stride) ||
	   (count                         == 1) )
	return MPI_Type_contiguous ( count * blocklen, old_type, newtype );

  /* Create and fill in the datatype */
  MPIR_ALLOC(dteptr,(MPI_Datatype) MPIR_SBalloc( MPIR_dtes ),MPI_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "Out of space in MPI_TYPE_HVECTOR" );
  *newtype = dteptr;
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_HVECTOR;
  dteptr->committed   = 0;
  dteptr->basic       = 0;
  dteptr->permanent   = 0;
  dteptr->is_contig   = 0;
  dteptr->ref_count   = 1;
  dteptr->align       = old_type->align;
  dteptr->elements    = count * blocklen * old_type->elements;
  dteptr->stride      = stride;
  dteptr->blocklen    = blocklen;
  dteptr->old_type    = (MPI_Datatype)MPIR_Type_dup (old_type);
  dteptr->count       = count;
  dteptr->has_ub      = old_type->has_ub;
  dteptr->has_lb      = old_type->has_lb;

  /* Set the upper/lower bounds and the extent and size */
  dteptr->extent      = ((count-1) * stride) + (blocklen * old_type->extent);
  if (dteptr->extent < 0) {
	dteptr->ub     = old_type->lb;
	dteptr->lb     = dteptr->ub + dteptr->extent;
	dteptr->real_ub= old_type->real_lb;
	dteptr->real_lb= dteptr->real_ub + 
	    ((count-1) * stride) + 
		(blocklen * (old_type->real_ub - old_type->real_lb));
	dteptr->extent = -dteptr->extent;
  }
  else {
	dteptr->lb     = old_type->lb;
	if (dteptr->has_ub) 
	    dteptr->ub = old_type->ub;
	else
	    dteptr->ub = dteptr->lb + dteptr->extent;
	dteptr->real_lb = old_type->real_lb;
	dteptr->real_ub = dteptr->real_lb + ((count-1) * stride) + 
		(blocklen * (old_type->real_ub - old_type->real_lb));  
  }
  dteptr->size        = count * blocklen * dteptr->old_type->size;
  
  return (mpi_errno);
}
