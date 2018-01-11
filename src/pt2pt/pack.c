/*
 *  $Id: pack.c,v 1.11 1995/05/09 18:58:52 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: pack.c,v 1.11 1995/05/09 18:58:52 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

/*@
    MPI_Pack - Packs a datatype into contiguous memory

Input Parameters:
. inbuf - input buffer start (choice) 
. incount - number of input data items (integer) 
. datatype - datatype of each input data item (handle) 
. outcount - output buffer size, in bytes (integer) 
. position - current position in buffer, in bytes (integer) 
. comm - communicator for packed message (handle) 

Output Parameter:
. outbuf - output buffer start (choice) 

.seealso: MPI_Unpack, MPI_Pack_size

@*/
int MPI_Pack ( inbuf, incount, type, outbuf, outcount, position, comm )
void         *inbuf;
int           incount;
MPI_Datatype  type;
void         *outbuf;
int           outcount;
int          *position;
MPI_Comm      comm;
{
  int size, pad, mpi_errno = MPI_SUCCESS;

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,type) ||
      MPIR_TEST_COUNT(comm,incount) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_PACK" );
  
  /* What kind of padding is necessary? */
  pad = (type->align - ((*position) % type->align)) % type->align;

  /* Is there enough room to finish packing the type? */
  MPIR_Pack_size ( incount, type, comm, &size );
  if (((*position) + pad + size) > outcount)
	return MPIR_ERROR(comm, MPI_ERR_LIMIT, "Buffer too small in MPI_PACK");

  /* Figure the pad and adjust position */
  (*position) += pad;
  mpi_errno = MPIR_Pack(comm, inbuf, incount, type, 
			(char *)outbuf + (*position), size - *position, 
			 &size );
  if (mpi_errno) MPIR_ERROR(comm,mpi_errno,"Error in MPI_PACK" );
  (*position) += size;
  return (mpi_errno);
}
