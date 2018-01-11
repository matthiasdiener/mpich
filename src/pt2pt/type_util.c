/*
 *  $Id: type_util.c,v 1.9 1994/12/21 14:33:54 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

#ifndef MPIR_TRUE
#define MPIR_TRUE  1
#define MPIR_FALSE 0
#endif


/*+
    MPIR_Type_dup - Utility function used to "touch" a type

  Algorithm:
  Only non-permanent types can be touched.  Since we don't free
  permanent types until the finalize stage, the reference count
  is not used to determine whether or not the type is freed
  during normal program execution.
+*/
MPI_Datatype MPIR_Type_dup ( datatype )
MPI_Datatype datatype;
{
  /* We increment the reference count even for the permanent types, so that 
     an eventual free (in MPI_Finalize) will correctly free these types */
  datatype->ref_count++;
  return (datatype);
}


/*+
  MPIR_Type_permanent - Utility function to mark a type as permanent
+*/
int MPIR_Type_permanent ( datatype )
MPI_Datatype datatype;
{
  if (datatype)
    datatype->permanent = MPIR_YES;
  return (MPI_SUCCESS);
}

/* 
   This version is used to free types that may include permanent types
   (as part of a derived datatype)

   Note that it is not necessary to commit a datatype (for example, one 
   that is used to define another datatype); but to free it, we must not
   require that it have been committed.
 */
int MPIR_Type_free ( datatype )
MPI_Datatype *datatype;
{
  int mpi_errno = MPI_SUCCESS;

  /* Check for bad arguments */
  if (MPIR_TEST_ARG(datatype) || 
      MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,*datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_TYPE_FREE" );

  /* Freeing null datatypes succeeds silently */
  if ( (*datatype) == MPI_DATATYPE_NULL )
	return (MPI_SUCCESS);

  /* We can't free permanent objects unless finalize has been called */
  if  ( ( (*datatype)->permanent ) && MPIR_Has_been_initialized == 1) {
      if ((*datatype)->ref_count > 1) 
	  (*datatype)->ref_count--;
      return MPI_SUCCESS;
      }

  /* Free datatype */
  if ( (*datatype)->ref_count <= 1 ) {

	/* Free malloc'd memory for various datatypes */
	if ( ((*datatype)->dte_type == MPIR_INDEXED)  ||
		 ((*datatype)->dte_type == MPIR_HINDEXED) || 
		 ((*datatype)->dte_type == MPIR_STRUCT)   ) {
	  FREE ( (*datatype)->indices );
	  FREE ( (*datatype)->blocklens );
	}

	/* Free malloc'd memory for pads in struct */
	if ( ((*datatype)->dte_type == MPIR_STRUCT) && (*datatype)->pads ) {
	    FREE( (*datatype)->pads );
	    }
	
	/* Free the old_type if not a struct */
	if ( ((*datatype)->dte_type != MPIR_STRUCT) && (!(*datatype)->basic) )
	  MPIR_Type_free ( &((*datatype)->old_type) );
	
	/* Free the old_types of a struct */
	if ((*datatype)->dte_type == MPIR_STRUCT) {
	  int i;

	  /* Decrease the reference count */
	  for (i=0; i<(*datatype)->count; i++)
		MPIR_Type_free ( &((*datatype)->old_types[i]) );

	  /* Free the malloc'd memory */
	  FREE ( (*datatype)->old_types );
	}

	/* Free the datatype structure */
	MPIR_SET_COOKIE((*datatype),0);
	MPIR_SBfree ( MPIR_dtes, (*datatype) );
  }
  else 
	(*datatype)->ref_count--;

  (*datatype) = MPI_DATATYPE_NULL;
  return (mpi_errno);
}
