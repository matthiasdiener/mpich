/*
 *  $Id: type_contig.c,v 1.13 1994/12/30 17:20:48 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: type_contig.c,v 1.13 1994/12/30 17:20:48 gropp Exp $";
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

@*/
int MPI_Type_contiguous( count, old_type, newtype )
int          count;
MPI_Datatype old_type;
MPI_Datatype *newtype;
{
  MPI_Datatype  dteptr;
  int mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if ( MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,old_type) ||
   ( (count   <= 0)                  && (mpi_errno = MPI_ERR_COUNT) ) ||
   ( (old_type->dte_type == MPIR_UB) && (mpi_errno = MPI_ERR_TYPE) )  ||
   ( (old_type->dte_type == MPIR_LB) && (mpi_errno = MPI_ERR_TYPE) ) )
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno,
					  "Error in MPI_TYPE_CONTIGUOUS" );
	
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

  /* Take care of contiguous vs non-contiguous case.
     Note that some datatypes (MPIR_STRUCT) that are marked 
     as contiguous (byt the code in MPI_Type_commit) may have not have
     an old_type.
   */
  if (old_type->is_contig && old_type->old_type) {
	dteptr->old_type  = (MPI_Datatype)MPIR_Type_dup (old_type->old_type);
	dteptr->count     = count * old_type->count;
	dteptr->pad       = 0;
  }
  else {
	dteptr->old_type  = (MPI_Datatype)MPIR_Type_dup (old_type);
	dteptr->count     = count;
	dteptr->pad       = ((old_type->align -
             (old_type->size % old_type->align)) % old_type->align);
  }

  /* Set the upper/lower bounds and the extent and size */
  dteptr->lb          = dteptr->old_type->lb;
  dteptr->extent      = dteptr->count * dteptr->old_type->extent;
  dteptr->ub          = dteptr->lb + dteptr->extent;
  dteptr->size        = (dteptr->count * dteptr->old_type->size) +
	                    ((dteptr->count - 1) * dteptr->pad);
  
  return (mpi_errno);
}
