/*
 *  $Id: type_struct.c,v 1.17 1994/12/21 14:33:27 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_struct.c,v 1.17 1994/12/21 14:33:27 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Type_struct - Creates a struct datatype

Input Parameters:
. count - number of blocks (integer) -- also number of 
entries in arrays array_of_types ,
array_of_displacements  and array_of_blocklengths  
. blocklens - number of elements in each block (array)
. indices - byte displacement of each block (array)
. old_types - type of elements in each block (array 
of handles to datatype objects) 

Output Parameter:
. newtype - new datatype (handle) 
@*/
int MPI_Type_struct( count, blocklens, indices, old_types, newtype )
int           count;
int 	      blocklens[];
MPI_Aint      indices[];      
MPI_Datatype  old_types[];
MPI_Datatype *newtype;
{
  MPI_Datatype    dteptr;
  MPI_Aint        ub, lb, high, low;
  MPIR_BOOL       high_init = MPIR_NO, low_init = MPIR_NO;
  int             i, mpi_errno = MPI_SUCCESS;
  int             ub_marker, lb_marker;
  MPIR_BOOL       ub_found = MPIR_NO, lb_found = MPIR_NO;
  int pad, size;

  /* Check for bad arguments */
  if ( count < 0 )
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_COUNT,
			       	  "Negative count in MPI_TYPE_STRUCT" );
    
  /* Check blocklens and old_types arrays and find number of bound */
  /* markers */
  for (i=0; i<count; i++) {
    if ( blocklens[i] < 0)
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER,
                        "Negative block length in MPI_TYPE_STRUCT");
    if ( old_types[i] == MPI_DATATYPE_NULL )
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_TYPE,
                        "Null type in MPI_TYPE_STRUCT");
    if (MPIR_TEST_IS_DATATYPE( MPI_COMM_WORLD, old_types[i] ))
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_TYPE,
			 "Invalid old datatype in MPI_TYPE_STRUCT" );
  }
    
  /* Create and fill in the datatype */
  dteptr = (*newtype) = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
  if (!dteptr) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_STRUCT" );
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_STRUCT;
  dteptr->committed   = MPIR_NO;
  dteptr->basic       = MPIR_FALSE;
  dteptr->permanent   = MPIR_FALSE;
  dteptr->is_contig   = MPIR_FALSE;
  dteptr->ref_count   = 1;
  dteptr->count       = count;
  dteptr->elements    = 0;
  dteptr->size        = 0;
  dteptr->align       = 1;

  /* Create indices and blocklens arrays and fill them */
  dteptr->indices     = ( int * ) MALLOC( count     * sizeof( int ) );
  dteptr->blocklens   = ( int * ) MALLOC( count     * sizeof( int ) );
  if (count > 1) 
      dteptr->pads        = ( int * ) MALLOC( (count-1) * sizeof( int ) );
  else
      dteptr->pads        = 0;
  dteptr->old_types   =
	( MPI_Datatype * ) MALLOC(count*sizeof(MPI_Datatype));
  if (!dteptr->indices || !dteptr->blocklens || 
      (count > 1 && ! dteptr->pads) || !dteptr->old_types) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_STRUCT" );
  high = low = ub = lb = 0;
  for (i = 0; i < count; i++)  {
	dteptr->old_types[i]  = (MPI_Datatype)MPIR_Type_dup (old_types[i]);
	dteptr->indices[i]    = indices[i];
	dteptr->blocklens[i]  = blocklens[i];
    if (dteptr->align < old_types[i]->align)
      dteptr->align       = old_types[i]->align;
    if ( old_types[i]->dte_type == MPIR_UB ) {
      if (ub_found) {
        if (indices[i] > ub_marker)
          ub_marker = indices[i];
      }
      else {
        ub_marker = indices[i];
        ub_found  = MPIR_YES;
      }
    }
    else if ( old_types[i]->dte_type == MPIR_LB ) {
      if (lb_found) {
        if ( indices[i] < lb_marker )
          lb_marker = indices[i];
      }
      else {
        lb_marker = indices[i];
        lb_found  = MPIR_YES;
      }
    }
    else {
      ub = indices[i] + (blocklens[i] * old_types[i]->extent) ;
      lb = indices[i];
      if (!high_init) { high = ub; high_init = MPIR_YES; }
      if (!low_init ) { low  = lb; low_init  = MPIR_YES; }
      if (ub > lb) {
        if ( high < ub ) high = ub;
        if ( low  > lb ) low  = lb;
      }
      else {
        if ( high < lb ) high = lb;
        if ( low  > ub ) low  = ub;
      }
      dteptr->elements += (blocklens[i] * old_types[i]->elements);
    }
  }
  
  for (i=0; i<(count-1); i++) {
    size = old_types[i]->size * blocklens[i];
    if ( blocklens[i] > 1 ) {
      pad = (old_types[i]->align - (old_types[i]->size %
             old_types[i]->align)) % old_types[i]->align;
      size += ((blocklens[i]-1) * pad );
    }
	dteptr->pads[i] = ((old_types[i+1]->align - 
        (size % old_types[i+1]->align)) % old_types[i+1]->align);
	dteptr->size   += (size + dteptr->pads[i]); 
  }
  dteptr->size     += (blocklens[i] * old_types[i]->size);
  if ( blocklens[i] > 1 ) {
    pad = (old_types[i]->align - (old_types[i]->size %
                                  old_types[i]->align)) % old_types[i]->align;
    dteptr->size += ((blocklens[i]-1) * pad );
  }
  
  /* Set the upper/lower bounds and the extent and size */
  dteptr->lb          = lb_found ? lb_marker : (low_init  ? low : 0);
  dteptr->ub          = ub_found ? ub_marker : (high_init ? high: 0);
  dteptr->extent      = dteptr->ub - dteptr->lb ;
  
  /* Adjust extent with padding */
  dteptr->extent += ((dteptr->align - (dteptr->extent % dteptr->align))
    % dteptr->align);

  return (mpi_errno);
}
