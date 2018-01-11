/*
 *  $Id: type_ind.c,v 1.18 1995/07/25 02:49:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_ind.c,v 1.18 1995/07/25 02:49:07 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_indexed - Creates an indexed datatype

Input Parameters:
. count - number of blocks -- also number of entries in 
array_of_displacements  and array_of_blocklengths  (nonnegative integer) 
. array_of_blocklengths - number of elements per block 
(array of nonnegative integers) 
. array_of_displacements - displacement for each block, 
in multiples of old_type  extent (array of integer) 
. old_type - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 
@*/
int MPI_Type_indexed( count, blocklens, indices, old_type, newtype )
int           count;
int 	      blocklens[];
int 	      indices[];
MPI_Datatype  old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  MPI_Aint      ub, lb, high, low, tmp;
  int           i, mpi_errno = MPI_SUCCESS;
  int           total_count;

  /* Check for bad arguments */
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (count    <  0)                 && (mpi_errno = MPI_ERR_COUNT) ) ||
   ( (old_type->dte_type == MPIR_UB) && (mpi_errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (mpi_errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_TYPE_INDEXED" );
	
  /* Are we making a null datatype? */
  total_count = 0;
  for (i=0; i<count; i++)
      total_count += blocklens[i];
  if (total_count == 0) {
      return MPI_Type_contiguous( 0, MPI_INT, newtype );
      }

  /* Create and fill in the datatype */
  dteptr = (*newtype) = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
  if (!dteptr) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_HVECTOR" );
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_INDEXED;
  dteptr->committed   = MPIR_NO;
  dteptr->basic       = MPIR_FALSE;
  dteptr->permanent   = MPIR_FALSE;
  dteptr->is_contig   = MPIR_FALSE;
  dteptr->ref_count   = 1;
  dteptr->align       = old_type->align;
  dteptr->old_type    = (MPI_Datatype)MPIR_Type_dup (old_type);
  dteptr->count       = count;
  dteptr->elements    = 0;
  dteptr->has_ub      = old_type->has_ub;
  dteptr->has_lb      = old_type->has_lb;

  /* Create indices and blocklens arrays and fill them */
  dteptr->indices     = ( MPI_Aint * ) MALLOC( count * sizeof( MPI_Aint ) );
  dteptr->blocklens   = ( int * ) MALLOC( count * sizeof( int ) );
  if (!dteptr->indices || !dteptr->blocklens) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_HINDEXED" );
  high                = (indices[0] + blocklens[0]) * old_type->extent;
  low                 = (indices[0] * old_type->extent);
  if (high < low) {
	tmp  = high;
	high = low;
	low  = tmp;
  }
  for (i = 0; i < count; i++)  {
	dteptr->indices[i]    = indices[i] * old_type->extent;
	dteptr->blocklens[i]  = blocklens[i];
	ub = dteptr->indices[i] + (blocklens[i] * old_type->extent) ;
	lb = dteptr->indices[i];
	if (ub > lb) {
	  if ( high < ub ) high = ub;
	  if ( low  > lb ) low  = lb;
	}
	else {
	  if ( high < lb ) high = lb;
	  if ( low  > ub ) low  = ub;
	}
	dteptr->elements     += blocklens[i];
  }

  /* Set the upper/lower bounds and the extent and size */
  /* Set the upper/lower bounds and the extent and size */
  if (old_type->has_lb) 
      dteptr->lb = old_type->lb;
  else
      dteptr->lb = low;
  if (old_type->has_ub)
      dteptr->ub = old_type->ub;
  else
      dteptr->ub = high;
  dteptr->extent      = high - low;
  dteptr->size        = dteptr->elements * old_type->size;

  /* 
    dteptr->elements contains the number of elements in the top level
	type.  to get the total elements, we multiply by the number of elements
	in the old type.
  */
  dteptr->elements   *= old_type->elements;

  return (mpi_errno);
}
