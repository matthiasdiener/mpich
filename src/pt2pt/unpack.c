/*
 *  $Id: unpack.c,v 1.18 1996/01/03 19:03:38 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: unpack.c,v 1.18 1996/01/03 19:03:38 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

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

.seealso: MPI_Pack, MPI_Pack_size
@*/
int MPI_Unpack ( inbuf, insize, position, outbuf, outcount, type, comm )
void         *inbuf;
int           insize;
int          *position;
void         *outbuf;
int           outcount;
MPI_Datatype  type;
MPI_Comm      comm;
{
  int size, mpi_errno = MPI_SUCCESS, actlen;
  int msgrep = 0, dest_len;

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,type) ||
      MPIR_TEST_COUNT(comm,insize) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );
  
  /* Is the inbuf big enough for the type that's being unpacked? */
  /* 
    We can't use this test because Pack_size is not exact but is rather
    an upper bound.
  MPIR_Pack_size ( outcount, type, comm, comm->msgrep, &size );
  if (((*position) + size) > insize)
	return MPIR_ERROR(comm, MPI_ERR_LIMIT, 
				       "Input buffer too small in MPI_UNPACK");
   */
  /* Note that for pack/unpack, the data representation is stored in the 
     communicator */
  mpi_errno = MPIR_Unpack( comm, 
			  (char *)inbuf + (*position), insize - *position, 
			  outcount, type, comm->msgrep, outbuf, &actlen, 
			  &dest_len );
  if (mpi_errno) MPIR_ERROR(comm,mpi_errno,"Error in MPI_UNPACK" );
  (*position) += actlen;
  return (mpi_errno);
}
