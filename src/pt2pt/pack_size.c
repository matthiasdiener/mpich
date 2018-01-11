/*
 *  $Id: pack_size.c,v 1.13 1997/01/07 01:45:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#include "mpiimpl.h"
#ifndef MPI_ADI2
#include "mpisys.h"
#endif

extern struct MPIR_DATATYPE MPIR_I_DCOMPLEX;

/*@
    MPI_Pack_size - Returns the upper bound on the amount of space needed to
                    pack a message

Input Parameters:
. incount - count argument to packing call (integer) 
. datatype - datatype argument to packing call (handle) 
. comm - communicator argument to packing call (handle) 

Output Parameter:
. size - upper bound on size of packed message, in bytes (integer) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_TYPE
.N MPI_ERR_ARG

@*/
int MPI_Pack_size ( incount, datatype, comm, size )
int           incount;
MPI_Datatype  datatype;
MPI_Comm      comm;
int          *size;
{
  int mpi_errno;
  struct MPIR_COMMUNICATOR *comm_ptr;
  struct MPIR_DATATYPE     *dtype_ptr;
  static char myname[] = "MPI_PACK_SIZE";

  TR_PUSH(myname);

  comm_ptr = MPIR_GET_COMM_PTR(comm);
  MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname);

  dtype_ptr = MPIR_GET_DTYPE_PTR(datatype);
  MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);
  
  if (MPIR_TEST_ARG(size) || MPIR_TEST_COUNT(comm,incount))
      return MPIR_ERROR(comm_ptr, mpi_errno, myname );

#ifdef MPI_ADI2
  /* Msgform is the form for ALL messages; we need to convert it into
     a Msgrep which may be different for each system.  Eventually, 
     Msgform should just be one of the Msgrep cases.
     In addition, this should probably not refer to XDR explicitly.
   */
  MPID_Pack_size( incount, dtype_ptr, comm_ptr->msgform, 
/*		  (comm_ptr->msgform == MPID_MSGFORM_OK) ? MPID_MSG_OK : 
		  MPID_MSG_XDR, */ size );
  (*size) += MPIR_I_DCOMPLEX.size;
#else
  /* Figure out size needed to pack type and add the biggest size
	 of other types to give an upper bound */
  MPIR_Pack_size( incount, datatype, comm, comm_ptr->msgrep, size );
  (*size) += MPIR_I_DCOMPLEX.size;
#endif

  TR_POP;
  return MPI_SUCCESS;
}


