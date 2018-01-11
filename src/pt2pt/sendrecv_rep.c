/*
 *  $Id: sendrecv_rep.c,v 1.15 1997/03/29 16:06:38 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
/* pt2pt for MPIR_Unpack */
#include "mpipt2pt.h"
#else
#include "mpisys.h"
#endif

/*@
    MPI_Sendrecv_replace - Sends and receives using a single buffer

Input Parameters:
. count - number of elements in send and receive buffer (integer) 
. datatype - type of elements in send and receive buffer (handle) 
. dest - rank of destination (integer) 
. sendtag - send message tag (integer) 
. source - rank of source (integer) 
. recvtag - receive message tag (integer) 
. comm - communicator (handle) 

Output Parameters:
. buf - initial address of send and receive buffer (choice) 
. status - status object (Status) 

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM
.N MPI_ERR_COUNT
.N MPI_ERR_TYPE
.N MPI_ERR_TAG
.N MPI_ERR_RANK
.N MPI_ERR_TRUNCATE
.N MPI_ERR_EXHAUSTED

@*/
int MPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, 
			  source, recvtag, comm, status )
void         *buf;
int           count, dest, sendtag, source, recvtag;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPI_Status   *status;
{
    int          mpi_errno = MPI_SUCCESS;
    int          buflen;
    void         *rbuf;
    MPI_Status   status_array[2];
    MPI_Request  req[2];
    MPIR_ERROR_DECL;
    struct MPIR_DATATYPE *dtype_ptr;
    struct MPIR_COMMUNICATOR *comm_ptr;
    static char myname[] = "MPI_SENDRECV_REPLACE";

    TR_PUSH(myname);

    comm_ptr = MPIR_GET_COMM_PTR(comm);
    MPIR_TEST_MPI_COMM(comm,comm_ptr,comm_ptr,myname );

    /* Check for invalid arguments */
    if ( MPIR_TEST_COUNT(comm,count) )
      return MPIR_ERROR( comm_ptr, mpi_errno, myname );
    /* Let the other send/recv routines find the remaining errors. */

    /* Allocate a temporary buffer that is long enough to receive the 
       message even if it has holes in it.  Perhaps a better way to 
       do this is if contiguous, then as here, else use pack/unpack
       to send contiguous data... 
     */
    MPIR_ERROR_PUSH(comm_ptr);

    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,comm_ptr,myname);

    if (count == 0 || dtype_ptr->is_contig) {
	buflen = dtype_ptr->extent * count;
	MPIR_CALL_POP(MPI_Isend ( buf,  count, datatype, dest,   
			       sendtag, comm, &req[0] ),comm_ptr,myname);
	if (buflen > 0) {
	    MPIR_ALLOC_POP(rbuf,(void *)MALLOC( buflen ),
			   comm_ptr, MPI_ERR_EXHAUSTED, myname );
	    }
	else
	    rbuf = (void *)0;
	
	MPIR_CALL_POP(MPI_Irecv ( rbuf, count, datatype, source, 
			    recvtag, comm, &req[1] ),comm_ptr,myname);
	MPIR_CALL_POP(MPI_Waitall ( 2, req, status_array ),comm_ptr,myname);
	if (rbuf) {
	    memcpy( buf, rbuf, buflen );
	    FREE( rbuf );
	    }
	(*status) = status_array[1];
	}
    else {
	int dest_len, act_len, position;
	/* non-contiguous data will be packed and unpacked */
	MPIR_CALL_POP(MPI_Pack_size( count, datatype, comm, &buflen ),
		      comm_ptr,myname);
	if (buflen > 0) {
	    MPIR_ALLOC_POP(rbuf,(void *)MALLOC( buflen ),
			   comm_ptr, MPI_ERR_EXHAUSTED, myname );
	    }
	else
	    rbuf = (void *)0;

	position = 0;
        /* The following call ultimately calls MPID_Pack (The ADI-2 interface
           requires support for Pack and Unpack).  It is important that it
           does so, because below we unpack with MPID_Unpack */
	MPIR_CALL_POP(MPI_Pack( buf, count, datatype, rbuf, buflen, 
				&position, comm ),comm_ptr,myname);
	mpi_errno = MPI_Sendrecv_replace( rbuf, position, MPI_PACKED, dest, 
					  sendtag, source, recvtag, comm, 
					  status );
	if (mpi_errno) {
	    if (rbuf) FREE( rbuf );
	    return MPIR_ERROR(comm_ptr,mpi_errno,myname);
	    }
	/* We need to use MPIR_Unpack because we need the DESTINATION 
	   length */
	act_len	 = 0;
	dest_len = 0;
#ifdef MPI_ADI2
	position = 0;
        MPID_Unpack( rbuf, status->count, MPID_Msgrep_from_comm(comm_ptr),
                     &position, buf, count, dtype_ptr, &dest_len,
                     comm_ptr, MPI_ANY_SOURCE, &mpi_errno);
#else
	MPIR_Unpack( comm, rbuf, buflen, count, datatype, comm_ptr->msgrep, 
		     buf, &act_len, &dest_len );
#endif
	if (rbuf) {
	    FREE( rbuf );
	    }
	/* Need to update the count field to reflect the number of UNPACKED
	   bytes */
	status->count = dest_len;
	}
    TR_POP;
    return MPI_SUCCESS;
}
