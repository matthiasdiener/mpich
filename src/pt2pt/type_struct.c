/*
 *  $Id: type_struct.c,v 1.24 1995/07/25 02:48:59 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_struct.c,v 1.24 1995/07/25 02:48:59 gropp Exp $";
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

Notes:
If an upperbound is set explicitly by using the MPI datatype 'MPI_UB', the
corresponding index must be positive.

The MPI standard originally made vague statements about padding and alignment;
this was intended to allow the simple definition of structures that could
be sent with a count greater than one.  For example,
.vb
    struct { int a; char b; } foo;
.ve
may have 'sizeof(foo) > sizeof(int) + sizeof(char)'; for example, 
'sizeof(foo) == 2*sizeof(int)'.  The initial version of the MPI standard
defined the extent of a datatype as including an `epsilon` that would have 
allowed an implementation to make the extent an MPI datatype
for this structure equal to '2*sizeof(int)'.  However, since different systems 
might define different paddings, a clarification to the standard made epsilon 
zero.  Thus, if you define a structure datatype and wish to send or receive
multiple items, you should explicitly include an 'MPI_UB' entry as the
last member of the structure.  For example, the following code can be used
for the structure foo
.vb
    blen[0] = 1; indices[0] = 0; oldtypes[0] = MPI_INT;
    blen[1] = 1; indices[1] = &foo.b - &foo; oldtypes[1] = MPI_CHAR;
    blen[2] = 1; indices[2] = sizeof(foo); oldtypes[2] = MPI_UB;
    MPI_Type_struct( 3, blen, indices, oldtypes, &newtype );
.ve
@*/
int MPI_Type_struct( count, blocklens, indices, old_types, newtype )
int           count;
int 	      blocklens[];
MPI_Aint      indices[];      
MPI_Datatype  old_types[];
MPI_Datatype *newtype;
{
  MPI_Datatype    dteptr;
  MPI_Aint        ub, lb, high, low, real_ub, real_lb, real_init;
  MPIR_BOOL       high_init = MPIR_NO, low_init = MPIR_NO;
  int             i, mpi_errno = MPI_SUCCESS;
  int             ub_marker, lb_marker;
  MPIR_BOOL       ub_found = MPIR_NO, lb_found = MPIR_NO;
  int             size, total_count;

  /* Check for bad arguments */
  if ( count < 0 )
	return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_COUNT,
			       	  "Negative count in MPI_TYPE_STRUCT" );

  if (count == 0) 
      return MPI_Type_contiguous( 0, MPI_INT, newtype );

  /* Check blocklens and old_types arrays and find number of bound */
  /* markers */
  total_count = 0;
  for (i=0; i<count; i++) {
    total_count += blocklens[i];
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
  if (total_count == 0) {
      return MPI_Type_contiguous( 0, MPI_INT, newtype );
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
  dteptr->has_ub      = 0;
  dteptr->has_lb      = 0;

  /* Create indices and blocklens arrays and fill them */
  dteptr->indices     = ( MPI_Aint * ) MALLOC( count * sizeof( MPI_Aint ) );
  dteptr->blocklens   = ( int * )      MALLOC( count * sizeof( int ) );
  dteptr->old_types   =
	( MPI_Datatype * ) MALLOC(count*sizeof(MPI_Datatype));
  if (!dteptr->indices || !dteptr->blocklens || !dteptr->old_types) 
      return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_TYPE_STRUCT" );
  high = low = ub = lb = 0;
  real_ub   = real_lb = 0;
  real_init = 0;
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
	  /* Since the datatype is NOT a UB or LB, save the real limits */
	  if (!real_init) {
	      real_init = 1;
	      real_lb = old_types[i]->real_lb;
	      real_ub = old_types[i]->real_ub;
	      }
	  else {
	      if (old_types[i]->real_lb < real_lb) 
		  real_lb = old_types[i]->real_lb;
	      if (old_types[i]->real_ub > real_ub) 
		  real_ub = old_types[i]->real_ub;
	      }
	  /* Next, check to see if datatype has an MPI_LB or MPI_UB
	     within it... */
	  if (old_types[i]->has_ub) {
	      ub_found = 1;
	      ub_marker = old_types[i]->ub;
	      }
	  if (old_types[i]->has_lb) {
	      lb_found = 1;
	      lb_marker = old_types[i]->lb;
	      }
	  /* Get the ub/lb from the datatype (if a MPI_UB or MPI_LB was
	     found, then these values will be ignored). */
	  ub = indices[i] + (blocklens[i] * old_types[i]->extent) ;
	  lb = indices[i];
	  if (!high_init) { high = ub; high_init = MPIR_YES; }
	  else if (ub > high) high = ub;
	  if (!low_init ) { low  = lb; low_init  = MPIR_YES; }
	  else if (lb < low) low = lb;
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
      dteptr->size   += size; 
      }
  dteptr->size     += (blocklens[i] * old_types[i]->size);
  
  /* Set the upper/lower bounds and the extent and size */
  if (lb_found) {
      dteptr->lb     = lb_marker;
      dteptr->has_lb = 1;
      }
  else 
      dteptr->lb = (low_init  ? low : 0);
  if (ub_found) {
      dteptr->ub     = ub_marker;
      dteptr->has_ub = 1;
      }
  else 
      dteptr->ub = (high_init ? high: 0);
  dteptr->extent      = dteptr->ub - dteptr->lb ;
  dteptr->real_ub     = real_ub;
  dteptr->real_ub     = real_lb;

  /* Extent has NO padding */

  return (mpi_errno);
}
