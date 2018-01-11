/*
 *  $Id: type_util.c,v 1.14 1995/12/21 21:41:28 gropp Exp $
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
  MPI_Datatype dtype = *datatype;

  /* Check for bad arguments */
  if (MPIR_TEST_ARG(datatype) || 
      MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,dtype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_TYPE_FREE" );

  /* Freeing null datatypes succeeds silently */
  if ( dtype == MPI_DATATYPE_NULL )
	return (MPI_SUCCESS);

  /* We can't free permanent objects unless finalize has been called */
  if  ( ( dtype->permanent ) && MPIR_Has_been_initialized == 1) {
      if (dtype->ref_count > 1) 
	  dtype->ref_count--;
      return MPI_SUCCESS;
      }

  /* Free datatype */
  if ( dtype->ref_count <= 1 ) {

	/* Free malloc'd memory for various datatypes */
	if ( (dtype->dte_type == MPIR_INDEXED)  ||
		 (dtype->dte_type == MPIR_HINDEXED) || 
		 (dtype->dte_type == MPIR_STRUCT)   ) {
	  FREE ( dtype->indices );
	  FREE ( dtype->blocklens );
	}

	/* Free the old_type if not a struct */
	if ( (dtype->dte_type != MPIR_STRUCT) && (!dtype->basic) )
	  MPIR_Type_free ( &(dtype->old_type) );
	
	/* Free the old_types of a struct */
	if (dtype->dte_type == MPIR_STRUCT) {
	  int i;

	  /* Decrease the reference count */
	  for (i=0; i<dtype->count; i++)
		MPIR_Type_free ( &(dtype->old_types[i]) );

	  /* Free the malloc'd memory */
	  FREE ( dtype->old_types );
	}

	/* Free the datatype structure */
	MPIR_SET_COOKIE(dtype,0);
	/* If the type is permanent and in static storage, we can't
	   free it here... Until all permanent datatypes are placed
	   into static storage, this will leave some storage leaks.
	   */
	if  ( !dtype->permanent ) {
	    MPIR_SBfree ( MPIR_dtes, dtype );
	    }
  }
  else 
	dtype->ref_count--;

  /* We have to do this because the permanent types are constants */
  if ( !dtype->permanent )
      (*datatype) = MPI_DATATYPE_NULL;
  return (mpi_errno);
}

/* Free the parts of a structure datatype */
void MPIR_Type_free_struct( dtype )
MPI_Datatype dtype;
{
/* Free malloc'd memory for various datatypes */
if ( (dtype->dte_type == MPIR_INDEXED)  ||
    (dtype->dte_type == MPIR_HINDEXED) || 
    (dtype->dte_type == MPIR_STRUCT)   ) {
    FREE ( dtype->indices );
    FREE ( dtype->blocklens );
    }

/* Free the old_type if not a struct */
if ( (dtype->dte_type != MPIR_STRUCT) && (!dtype->basic) )
    MPIR_Type_free ( &(dtype->old_type) );

/* Free the old_types of a struct */
if (dtype->dte_type == MPIR_STRUCT) {
    int i;

    /* Decrease the reference count */
    for (i=0; i<dtype->count; i++)
	MPIR_Type_free ( &(dtype->old_types[i]) );
    
    /* Free the malloc'd memory */
    FREE ( dtype->old_types );
    }

/* Free the datatype structure */
MPIR_SET_COOKIE(dtype,0);
}

/*
   This routine returns the "real" lb and ub, ignoring any explicitly set 
   TYPE_LB or TYPE_UB.  This is needed when allocating space for a 
   datatype that includes all of the "holes" (note that MPI_TYPE_SIZE
   gives only the number of bytes that the selected elements occupy).
   This is needed for some of the collective routines.

   STILL NEEDS TO BE IMPLEMENTED IN THE TYPE ROUTINES
 */
void MPIR_Type_get_limits( dtype, lb, ub )
MPI_Datatype dtype;
MPI_Aint *lb, *ub;
{
/*
    *lb = dtype->real_lb;
    *ub = dtype->real_ub;
 */
    MPIR_GET_REAL_DATATYPE(dtype)
    *lb = dtype->lb;
    *ub = dtype->ub;
}
