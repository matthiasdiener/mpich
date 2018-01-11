/*
 *  $Id: unpack.c,v 1.24 1997/01/17 22:59:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

/*@
    MPI_Unpack - Unpack a datatype into contiguous memory

Input Parameters:
. inbuf - input buffer start (choice) 
. insize - size of input buffer, in bytes (integer) 
. position - current position in bytes (integer) 
. outcount - number of items to be unpacked (integer) 
. datatype - datatype of each output data item (handle) 
. comm - communicator for packed message (handle) 

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
#ifndef MPI_ADI2
  int dest_len, actlen;
#endif

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COUNT(comm,insize) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm_ptr,mpi_errno,myname );

  if (!dtype_ptr->committed) {
      return MPIR_ERROR( comm_ptr, MPI_ERR_UNCOMMITTED, myname );
  }

#ifdef MPI_ADI2
  /* The data WAS received with MPI_PACKED format, and so was SENT with
     the format of the communicator */
  /* We need to compute the PACKED msgrep from the comm msgFORM. */
  out_pos = 0;
  MPID_Unpack( inbuf, insize, MPID_Msgrep_from_comm( comm_ptr ), position,
	       outbuf, outcount, dtype_ptr, &out_pos, 
	       comm_ptr, MPI_ANY_SOURCE, &mpi_errno );
  TR_POP;
  MPIR_RETURN(comm_ptr,mpi_errno,myname);
#else  
  /* Is the inbuf big enough for the type that's being unpacked? */
  /* 
    We can't use this test because Pack_size is not exact but is rather
    an upper bound.
  MPIR_Pack_size ( outcount, datatype, comm, comm_ptr->msgrep, &size );
  if (((*position) + size) > insize)
	return MPIR_ERROR(comm_ptr, MPI_ERR_LIMIT, 
				       "Input buffer too small in MPI_UNPACK");
   */
  /* Note that for pack/unpack, the data representation is stored in the 
     communicator */
  /* If you change this, you must ALSO change sendrecv_rep (it needed
     dest_len) */
  mpi_errno = MPIR_Unpack( comm, 
			  (char *)inbuf + (*position), insize - *position, 
			  outcount, datatype, comm_ptr->msgrep, outbuf, &actlen, 
			  &dest_len );
  if (mpi_errno) MPIR_ERROR(comm_ptr,mpi_errno,myname );
  (*position) += actlen;
  return (mpi_errno);
#endif
}
