/*
 *  $Id: type_contig.c,v 1.19 1996/01/03 19:03:16 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_contig.c,v 1.19 1996/01/03 19:03:16 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"


/*@
    MPI_Type_contiguous - Creates a contiguous datatype

Input Parameters:
. count - replication count (nonnegative integer) 
. oldtype - old datatype (handle) 

Output Parameter:
. newtype - new datatype (handle) 

.N fortran
@*/
int MPI_Type_contiguous( count, old_type, newtype )
int          count;
MPI_Datatype old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  int mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  MPIR_GET_REAL_DATATYPE(old_type)
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (count   <  0)                  && (mpi_errno = MPI_ERR_COUNT) ) ||
   ( (old_type->dte_type == MPIR_UB) && (mpi_errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (mpi_errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_TYPE_CONTIGUOUS" );

  /* Are we making a null datatype? */
  if (count == 0) {
      /* (*newtype) = MPI_DATATYPE_NULL; */
      dteptr = (*newtype) = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
      MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
      dteptr->dte_type    = MPIR_CONTIG;
      dteptr->committed   = MPIR_NO;
      dteptr->basic       = MPIR_FALSE;
      dteptr->permanent   = MPIR_FALSE;
      dteptr->ref_count   = 1;
      dteptr->align       = 4;
      dteptr->stride      = 1;
      dteptr->blocklen    = 1;
      dteptr->is_contig   = 1;
      dteptr->elements    = 0;
      dteptr->has_ub      = 0;
      dteptr->has_lb      = 0;
      dteptr->count       = 0;
      dteptr->lb          = 0;
      dteptr->has_lb      = 0;
      dteptr->extent      = 0;
      dteptr->ub          = 0;
      dteptr->has_ub      = 0;
      dteptr->size        = 0;
      dteptr->real_lb     = 0;
      dteptr->real_ub     = 0;
      dteptr->old_type    = (MPI_Datatype)MPIR_Type_dup (old_type);
      return (mpi_errno);
      }

  /* Create and fill in the datatype */
  dteptr = (*newtype) = (MPI_Datatype) MPIR_SBalloc( MPIR_dtes );
  MPIR_SET_COOKIE(dteptr,MPIR_DATATYPE_COOKIE)
  dteptr->dte_type    = MPIR_CONTIG;
  dteptr->committed   = MPIR_NO;
  dteptr->basic       = MPIR_FALSE;
  dteptr->permanent   = MPIR_FALSE;
  dteptr->ref_count   = 1;
  dteptr->align       = old_type->align;
  dteptr->stride      = 1;
  dteptr->blocklen    = 1;
  dteptr->is_contig   = old_type->is_contig;
  dteptr->elements    = count * old_type->elements;
  dteptr->has_ub      = 0;
  dteptr->has_lb      = 0;

  /* Take care of contiguous vs non-contiguous case.
     Note that some datatypes (MPIR_STRUCT) that are marked 
     as contiguous (byt the code in MPI_Type_commit) may have not have
     an old_type.
   */
  if (old_type->is_contig && old_type->old_type) {
	dteptr->old_type  = (MPI_Datatype)MPIR_Type_dup (old_type->old_type);
	dteptr->count     = count * old_type->count;
  }
  else {
	dteptr->old_type  = (MPI_Datatype)MPIR_Type_dup (old_type);
	dteptr->count     = count;
  }

  /* Set the upper/lower bounds and the extent and size */
  dteptr->lb          = dteptr->old_type->lb;
  dteptr->has_lb      = dteptr->old_type->has_lb;
  dteptr->extent      = dteptr->count * dteptr->old_type->extent;
  if (dteptr->old_type->has_ub) {
      dteptr->ub     = dteptr->old_type->ub;
      dteptr->has_ub = 1;
      }
  else 
      dteptr->ub          = dteptr->lb + dteptr->extent;
  dteptr->size        = (dteptr->count * dteptr->old_type->size);
  dteptr->real_lb     = dteptr->old_type->real_lb;
  /* The value for real_ub here is an overestimate, but getting the
     "best" value is a bit complicated.  Note that for count == 1,
      this formula gives old_type->real_ub, independent of real_lb. */
  dteptr->real_ub     = dteptr->count * 
      (dteptr->old_type->real_ub - dteptr->old_type->real_lb) + 
	  dteptr->old_type->real_lb;
  return (mpi_errno);
}
