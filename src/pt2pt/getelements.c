/*
 *  $Id: getelements.c,v 1.4 1995/09/18 15:39:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: getelements.c,v 1.4 1995/09/18 15:39:26 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*
   This routine is passed to MPIR_Unpack2 to compute the number of
   elements and to return the number of bytes processed in the input.
   See src/dmpi/pkutil.c (routine MPIR_PrintDatatypeUnpack) for an 
   example of its use.

   This assumes that this is called only for primitive datatypes.
 */
int MPIR_Elementcount( src, num, datatype, inbytes, dest, ctx )
char         *dest, *src;
MPI_Datatype datatype;
int          num, inbytes;
void         *ctx;
{
int len = datatype->size * num;
int *elementcnt = (int *)ctx;

*elementcnt = *elementcnt + num;
return len;
}


/*@
  MPI_Get_elements - Returns the number of basic elements
                     in a datatype

Input Parameters:
. status - return status of receive operation (Status) 
. datatype - datatype used by receive operation (handle) 

Output Parameter:
. count - number of received basic elements (integer) 
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
  
  /* Find the number of elements */
  MPI_Get_count (status, datatype, &count);
  if (count == MPI_UNDEFINED) {
      *elements = count;
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
	/* HACK ALERT -- the code in this if is not correct */
	/*               but for now ... */
	double cnt = 
	  (double) status->count / (double) datatype->size;
	(*elements) = (int) ( cnt * (double) datatype->elements );

      {
      int srclen, destlen, used_len;
      i_dummy;
      
      srclen   = status->count;
      /* Need to set count so that we'll exit when we run out of 
	 items.  It could be ceil(status->count/datatype->size) .
	 Alternately, we could check that used_len >= srclen - epsilon
	 (in case there isn't enough for the last item).

	 Why isn't this correct?
       */
      MPIR_Unpack2( (char *)&i_dummy, count, datatype, 
		    MPIR_Elementcnt, (void *)elements, (char *)&i_dummy,
		    srclen, &destlen, &used_len );
      }
#endif
  }
  else
	(*elements) = count * datatype->elements;

  return (MPI_SUCCESS);
}

