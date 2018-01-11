/*
 *  $Id: pack.c,v 1.16 1996/06/07 15:07:30 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

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

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TYPE
.N MPI_ERR_COUNT
.N MPI_ERR_ARG

.seealso: MPI_Unpack, MPI_Pack_size

@*/
int MPI_Pack ( inbuf, incount, datatype, outbuf, outcount, position, comm )
void         *inbuf;
int           incount;
MPI_Datatype  datatype;
void         *outbuf;
int           outcount;
int          *position;
MPI_Comm      comm;
{
  int mpi_errno = MPI_SUCCESS;
#ifndef MPI_ADI2
  int size;
#endif

  /* NOT ENOUGH ERROR CHECKING AT PRESENT */
  if (MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,datatype) ||
      MPIR_TEST_COUNT(comm,incount) || MPIR_TEST_ARG(position) ||
      ( (*position < 0 ) && (mpi_errno = MPI_ERR_ARG) ) ) 
      return MPIR_ERROR(comm,mpi_errno,"Error in MPI_PACK" );

#ifdef MPI_ADI2
  /* Msgform is the form for ALL messages; we need to convert it into
     a Msgrep which may be different for each system.  Eventually, 
     Msgform should just be one of the Msgrep cases.
     In addition, this should probably not refer to XDR explicitly.
   */
  MPID_Pack( inbuf, incount, datatype, 
	     ((char *)outbuf) + *position, outcount-*position, position,
	     comm, MPI_ANY_SOURCE, -1, comm->msgform, 
/*	     (comm->msgform == MPID_MSGFORM_OK) ? MPID_MSG_OK : 
		  MPID_MSG_XDR, */ &mpi_errno );
  MPIR_RETURN(comm,mpi_errno,"Error in MPI_PACK");
#else  
  /* What kind of padding is necessary? */
  /* pad = (datatype->align - 
     ((*position) % datatype->align)) % datatype->align; */

  /* Is there enough room to finish packing the type? */
  MPIR_Pack_size ( incount, datatype, comm, comm->msgrep, &size );
  if (((*position) /* + pad */ + size) > outcount)
	return MPIR_ERROR(comm, MPI_ERR_ARG, "Buffer too small in MPI_PACK");

  /* Figure the pad and adjust position */
  /* (*position) += pad; */
  mpi_errno = MPIR_Pack(comm, comm->msgrep, inbuf, incount, datatype, 
			(char *)outbuf + (*position), outcount - *position, 
			 &size );
  if (mpi_errno) MPIR_ERROR(comm,mpi_errno,"Error in MPI_PACK" );
  (*position) += size;
  return (mpi_errno);
#endif
}
