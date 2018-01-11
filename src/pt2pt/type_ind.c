/*
 *  $Id: type_ind.c,v 1.20 1996/04/11 20:25:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "sbcnst2.h"
#define MPIR_SBalloc MPID_SBalloc
#else
#include "mpisys.h"
#endif

/*@
    MPI_Type_indexed - Creates an indexed datatype

Input Parameters:
. count - number of blocks -- also number of entries in indices and blocklens
. blocklens - number of elements in each block (array of nonnegative integers) 
. indices - displacement of each block in multiples of old_type (array of 
  integers)
. old_type - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 

.N fortran

The indices are displacements, and are based on a zero origin.  A common error
is to do something like to following
.vb
    integer a(100)
    integer blens(10), indices(10)
    do i=1,10
         blens(i)   = 1
10       indices(i) = 1 + (i-1)*10
    call MPI_TYPE_INDEXED(10,blens,indices,MPI_INTEGER,newtype,ierr)
    call MPI_TYPE_COMMIT(newtype,ierr)
    call MPI_SEND(a,1,newtype,...)
.ve
expecting this to send 'a(1),a(11),...' because the indices have values 
'1,11,...'.   Because these are `displacements` from the beginning of 'a',
it actually sends 'a(1+1),a(1+11),...'.

If you wish to consider the displacements as indices into a Fortran array,
consider declaring the Fortran array with a zero origin
.vb
    integer a(0:99)
.ve

.N Errors
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_EXHAUSTED
@*/
int MPI_Type_indexed( count, blocklens, indices, old_type, newtype )
int           count;
int 	      blocklens[];
int 	      indices[];
MPI_Datatype  old_type;
MPI_Datatype *newtype;
{
  MPI_Aint      *hindices;
  int           i, mpi_errno = MPI_SUCCESS;
  int           total_count;
  MPIR_ERROR_DECL;

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
  for (i=0; i<count; i++) {
      total_count += blocklens[i];
      if (blocklens[i] < 0) {
	  return MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_ARG,
			     "Invalid blocklens in MPI_TYPE_INDEXED" );
      }
  }
  if (total_count == 0) {
      return MPI_Type_contiguous( 0, MPI_INT, newtype );
      }

  /* Generate a call to MPI_Type_hindexed instead.  This means allocating
     a temporary displacement array, multiplying all displacements
     by extent(old_type), and using that */
  MPIR_ALLOC(hindices,(MPI_Aint *)MALLOC(count*sizeof(MPI_Aint)),
	     MPI_COMM_WORLD,MPI_ERR_EXHAUSTED,"Error in MPI_TYPE_INDEXED");
  for (i=0; i<count; i++) {
      hindices[i] = (MPI_Aint)indices[i] * old_type->extent;
  }
  MPIR_ERROR_PUSH(MPI_COMM_WORLD);
  mpi_errno = MPI_Type_hindexed( count, blocklens, hindices, old_type, 
				 newtype );
  MPIR_ERROR_POP(MPI_COMM_WORLD);
  FREE(hindices);
  MPIR_RETURN(MPI_COMM_WORLD,mpi_errno, "Error in MPI_TYPE_INDEXED");
#ifdef FOO
  /* Create and fill in the datatype */
  MPIR_ALLOC(dteptr,(MPI_Datatype) MPIR_SBalloc( MPIR_dtes ), MPI_COMM_WORLD, 
	     MPI_ERR_EXHAUSTED, "Out of space in MPI_TYPE_INDEXED" );
  *newtype = dteptr;
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_INDEXED;
  dteptr->committed   = 0;
  dteptr->basic       = 0;
  dteptr->permanent   = 0;
  dteptr->is_contig   = 0;
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
  /* Using *= on hpux with -O optimization causes this routine to fail (!) */
  dteptr->elements   = dteptr->elements * old_type->elements;

  return (mpi_errno);
#endif
}
