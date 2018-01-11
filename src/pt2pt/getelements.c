/*
 *  $Id: getelements.c,v 1.7 1996/04/11 20:18:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpidmpi.h"
#else
#include "mpisys.h"
#endif
/*@
  MPI_Get_elements - Returns the number of basic elements
                     in a datatype

Input Parameters:
. status - return status of receive operation (Status) 
. datatype - datatype used by receive operation (handle) 

Output Parameter:
. count - number of received basic elements (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_TYPE

@*/
int MPI_Get_elements ( status, datatype, elements )
MPI_Status    *status;
MPI_Datatype  datatype;
int          *elements;
{
  int count, mpi_errno;

  if (MPIR_TEST_DATATYPE(MPI_COMM_WORLD,datatype))
	return MPIR_ERROR( MPI_COMM_WORLD, mpi_errno, 
			   "Error in MPI_GET_ELEMENTS" );

  MPIR_GET_REAL_DATATYPE(datatype)
  /* Find the number of elements */
  MPI_Get_count (status, datatype, &count);
  if (count == MPI_UNDEFINED) {
      /* To do this correctly, we need to run through the datatype,
	 processing basic types until we run out of data.  
	 We can do this in part by computing how many full versions
	 of datatype will fit, and make use of the datatype->elements
	 field.  If there isn't an EXACT fit, we need to look into
	 the datatype for more details about the exact mapping to
	 elements.  We might be able to do this with a version of
	 MPIR_Unpack2.
       */
#ifdef FOO
      *elements = count;
	/* HACK ALERT -- the code in this if is not correct */
	/*               but for now ... */
	double cnt = 
	  (double) status->count / (double) datatype->size;
	(*elements) = (int) ( cnt * (double) datatype->elements );
#endif
      {
      int srclen, destlen, used_len;
      int i_dummy;
      
      srclen   = status->count;
      /* Need to set count so that we'll exit when we run out of 
	 items.  It could be ceil(status->count/datatype->size) .
	 Alternately, we could check that used_len >= srclen - epsilon
	 (in case there isn't enough for the last item).

	 Why isn't this correct?
       */
      if (datatype->size > 0)
	  count = 1 + (srclen / datatype->size);
      else {
	  *elements = srclen ? MPI_UNDEFINED : 0;
	  return MPI_SUCCESS;
      }
      *elements = 0;
      used_len  = 0;
      MPIR_Unpack2( (char *)&i_dummy, count, datatype, 
		    MPIR_Elementcnt, (void *)elements, (char *)&i_dummy,
		    srclen, &destlen, &used_len );
      /* If anything is left, return undefined */
      if (used_len != srclen)
	  *elements = MPI_UNDEFINED;
      }
  }
  else
	(*elements) = count * datatype->elements;

  return (MPI_SUCCESS);
}

