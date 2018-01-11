/*
 *  $Id: unpack.c,v 1.4 1998/04/28 21:47:33 swider Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
    MPI_Unpack - Unpack a datatype into contiguous memory

Input Parameters:
+ inbuf - input buffer start (choice) 
. insize - size of input buffer, in bytes (integer) 
. position - current position in bytes (integer) 
. outcount - number of items to be unpacked (integer) 
. datatype - datatype of each output data item (handle) 
- comm - communicator for packed message (handle) 

Output Parameter:
. outbuf - output buffer start (choice) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_ARG

.seealso: MPI_Pack, MPI_Pack_size
@*/
int MPI_Unpack ( inbuf, insize, position, outbuf, outcount, datatype, comm )
void         *inbuf;
int           insize;
int          *position;
void         *outbuf;
int           outcount;
MPI_Datatype  datatype;
MPI_Comm      comm;
{
  int mpi_errno = MPI_SUCCESS;
  int out_pos;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *dtype_ptr;
  static char myname[] = "MPI_UNPACK";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COUNT(comm,insize) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm_ptr,mpi_errno,myname );

  /******************************************************************
   ****** This error check was put in by Debbie Swider on 11/17/97 **
   ******************************************************************/
  
  /*** check to see that number of items to be unpacked is not < 0 ***/
  if (MPIR_TEST_OUTCOUNT(comm,outcount)) {
     return MPIR_ERROR(comm_ptr,mpi_errno,myname);
  }
 
  if (!dtype_ptr->committed) {
      return MPIR_ERROR( comm_ptr, MPI_ERR_UNCOMMITTED, myname );
  }

  /* The data WAS received with MPI_PACKED format, and so was SENT with
     the format of the communicator */
  /* We need to compute the PACKED msgrep from the comm msgFORM. */
  out_pos = 0;
  MPID_Unpack( inbuf, insize, MPID_Msgrep_from_comm( comm_ptr ), position,
	       outbuf, outcount, dtype_ptr, &out_pos, 
	       comm_ptr, MPI_ANY_SOURCE, &mpi_errno );
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
}
