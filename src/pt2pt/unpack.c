/*
 *  $Id: unpack.c,v 1.21 1996/07/17 18:04:00 gropp Exp $
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
#ifndef MPI_ADI2
  int dest_len, actlen;
#endif

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,datatype) ||
      MPIR_TEST_COUNT(comm,insize) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );

#ifdef MPI_ADI2
  /* The data WAS received with MPI_PACKED format, and so was SENT with
     the format of the communicator */
  /* We need to compute the PACKED msgrep from the comm msgFORM. */
  out_pos = 0;
  MPID_Unpack( inbuf, insize, MPID_Msgrep_from_comm( comm ), position,
	       outbuf, outcount, datatype, &out_pos, 
	       comm, MPI_ANY_SOURCE, &mpi_errno );
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_UNPACK");
#else  
  /* Is the inbuf big enough for the type that's being unpacked? */
  /* 
    We can't use this test because Pack_size is not exact but is rather
    an upper bound.
  MPIR_Pack_size ( outcount, datatype, comm, comm->msgrep, &size );
  if (((*position) + size) > insize)
	return MPIR_ERROR(comm, MPI_ERR_LIMIT, 
				       "Input buffer too small in MPI_UNPACK");
   */
  /* Note that for pack/unpack, the data representation is stored in the 
     communicator */
  /* If you change this, you must ALSO change sendrecv_rep (it needed
     dest_len) */
  mpi_errno = MPIR_Unpack( comm, 
			  (char *)inbuf + (*position), insize - *position, 
			  outcount, datatype, comm->msgrep, outbuf, &actlen, 
			  &dest_len );
  if (mpi_errno) MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );
  (*position) += actlen;
  return (mpi_errno);
#endif
}
