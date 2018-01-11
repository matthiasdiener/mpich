/*
 *  $Id: type_hvec.c,v 1.12 1994/10/24 22:02:54 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_hvec.c,v 1.12 1994/10/24 22:02:54 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_hvector - Creates a vector (strided) datatype with offset in bytes

Input Parameters:
. count - number of blocks (nonnegative integer) 
. blocklength - number of elements in each block 
(nonnegative integer) 
. stride - number of bytes between start of each block (integer) 
. oldtype - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 
@*/
int MPI_Type_hvector( count, blocklen, stride, old_type, newtype )
int          count;
int 	     blocklen;
MPI_Aint     stride;
MPI_Datatype old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  int           errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (old_type == (*newtype))        && (errno = MPI_ERR_TYPE) )  ||
   ( (count   <= 0)                  && (errno = MPI_ERR_COUNT) ) ||
   ( (blocklen <= 0)                 && (errno = MPI_ERR_ARG) )   ||
   ( (old_type->dte_type == MPIR_UB) && (errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, errno,
					  "Error in MPI_TYPE_HVECTOR" );
	
  /* Handle the case where blocklen & stride make a contiguous type */
  if ( ((blocklen * old_type->extent) == stride) ||
	   (count                         == 1) )
	return MPI_Type_contiguous ( count * blocklen, old_type, newtype );

  /* Create and fill in the datatype */
  dteptr = (*newtype) = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
  if (!dteptr) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_HVECTOR" );
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_HVECTOR;
  dteptr->committed   = MPIR_NO;
  dteptr->basic       = MPIR_FALSE;
  dteptr->permanent   = MPIR_FALSE;
  dteptr->is_contig   = MPIR_FALSE;
  dteptr->ref_count   = 1;
  dteptr->align       = old_type->align;
  dteptr->elements    = count * blocklen * old_type->elements;
  dteptr->stride      = stride;
  dteptr->blocklen    = blocklen;
  dteptr->old_type    = (MPI_Datatype)MPIR_Type_dup (old_type);
  dteptr->count       = count;
  dteptr->pad         = ((old_type->align -
                        (old_type->size % old_type->align)) % old_type->align);

  /* Set the upper/lower bounds and the extent and size */
  dteptr->extent      = ((count-1) * stride) + (blocklen * old_type->extent);
  if (dteptr->extent < 0) {
	dteptr->ub     = old_type->lb;
	dteptr->lb     = dteptr->ub + dteptr->extent;
	dteptr->extent = -dteptr->extent;
  }
  else {
	dteptr->lb     = old_type->lb;
	dteptr->ub     = dteptr->lb + dteptr->extent;
  }
  dteptr->size        = (count * blocklen * dteptr->old_type->size) +
	                    (((count * blocklen) - 1) * dteptr->pad);
  
  return (errno);
}
