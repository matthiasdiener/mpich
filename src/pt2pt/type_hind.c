/*
 *  $Id: type_hind.c,v 1.11 1994/10/24 22:02:53 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_hind.c,v 1.11 1994/10/24 22:02:53 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_hindexed - Creates an indexed datatype with offsets in bytes

Input Parameters:
. count - number of blocks -- also number of entries in 
array_of_displacements  and
array_of_blocklengths  (integer) 
. array_of_blocklengths - number of elements in each block 
(array of nonnegative integers) 
. array_of_displacements - byte displacement of each block 
(array of integer) 
. oldtype - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 
@*/
int MPI_Type_hindexed( count, blocklens, indices, old_type, newtype )
int           count;
int           blocklens[];
MPI_Aint      indices[];
MPI_Datatype  old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  MPI_Aint      ub, lb, high, low;
  int           i, errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (old_type == (*newtype))        && (errno = MPI_ERR_TYPE) )  ||
   ( (count    <= 0)                 && (errno = MPI_ERR_COUNT) ) ||
   ( (old_type->dte_type == MPIR_UB) && (errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, errno,
					  "Error in MPI_TYPE_INDEXED" );
	
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
  dteptr->pad         = ((old_type->align -
                        (old_type->size % old_type->align)) % old_type->align);

  /* Create indices and blocklens arrays and fill them */
  dteptr->indices     = ( int * ) MALLOC( count * sizeof( int ) );
  dteptr->blocklens   = ( int * ) MALLOC( count * sizeof( int ) );
  if (!dteptr->indices || !dteptr->blocklens) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_HINDEXED" );
  low                 = indices[0];
  high                = indices[0] + (blocklens[0] * old_type->extent);
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
	dteptr->elements     += blocklens[i];
  }

  /* Set the upper/lower bounds and the extent and size */
  dteptr->lb          = low;
  dteptr->ub          = high;
  dteptr->extent      = high - low;
  dteptr->size        = ((dteptr->elements * old_type->size) +
	                    ((dteptr->elements - 1) * dteptr->pad));

  /* 
    dteptr->elements contains the number of elements in the top level
	type.  to get the total elements, we multiply by the number of elements
	in the old type.
  */
  dteptr->elements   *= old_type->elements;
  
  return (errno);
}
