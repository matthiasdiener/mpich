/*
 *  $Id: type_hind.c,v 1.18 1995/12/21 21:36:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_hind.c,v 1.18 1995/12/21 21:36:16 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_hindexed - Creates an indexed datatype with offsets in bytes

Input Parameters:
. count - number of blocks -- also number of entries in indices and blocklens
. blocklens - number of elements in each block (array of nonnegative integers) 
. indices - byte displacement of each block (array of MPI_Aint) 
. old_type - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 

.N fortran

Also see the discussion for MPI_Type_indexed about the 'indices' in Fortran.

@*/
int MPI_Type_hindexed( count, blocklens, indices, old_type, newtype )
int           count;
int           blocklens[];
MPI_Aint      indices[];
MPI_Datatype  old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  MPI_Aint      ub, lb, high, low, real_ub, real_lb, real_init;
  int           i, mpi_errno = MPI_SUCCESS;
  int           total_count;
  
  /* Check for bad arguments */
  MPIR_GET_REAL_DATATYPE(old_type)
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
  dteptr->dte_type    = MPIR_HINDEXED;
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
  low                 = indices[0];
  high                = indices[0] + (blocklens[0] * old_type->extent);
  real_lb             = indices[0];
  real_ub             = real_lb;
  for (i = 0; i < count; i++)  {
	dteptr->indices[i]    = indices[i];
	dteptr->blocklens[i]  = blocklens[i];
	ub = indices[i] + (blocklens[i] * old_type->extent) ;
	lb = indices[i];
	if (ub > lb) {
	  if ( high < ub ) high = ub;
	  if ( low  > lb ) low  = lb;
	}
	else {
	  if ( high < lb ) high = lb;
	  if ( low  > ub ) low  = ub;
	}
	if (indices[i] < real_lb) real_lb = indices[i];
	if (indices[i] + 
	   (blocklens[i] * (old_type->real_ub - old_type->real_lb)) > real_ub)
	    real_ub = indices[i] + 
	   (blocklens[i] * (old_type->real_ub - old_type->real_lb));

	dteptr->elements     += blocklens[i];
  }

  /* Set the upper/lower bounds and the extent and size */
  if (old_type->has_lb) 
      dteptr->lb = old_type->lb;
  else
      dteptr->lb = low;
  if (old_type->has_ub)
      dteptr->ub = old_type->ub;
  else
      dteptr->ub = high;
  dteptr->extent  = high - low;
  dteptr->size	  = dteptr->elements * old_type->size;
  dteptr->real_ub = real_ub;
  dteptr->real_lb = real_lb;

  /* 
    dteptr->elements contains the number of elements in the top level
	type.  to get the total elements, we multiply by the number of elements
	in the old type.
  */
  dteptr->elements   *= old_type->elements;
  
  return (mpi_errno);
}
