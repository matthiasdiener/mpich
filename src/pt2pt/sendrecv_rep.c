/*
 *  $Id: sendrecv_rep.c,v 1.6 1994/07/13 04:04:08 lusk Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: sendrecv_rep.c,v 1.6 1994/07/13 04:04:08 lusk Exp $";
#endif /* lint */

#include "mpiimpl.h"
#include "mpisys.h"

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
@*/
int MPI_Sendrecv_replace( buf, count, datatype, dest, sendtag, 
			  source, recvtag, comm, status )
void         *buf;
int           count, dest, sendtag, source, recvtag;
MPI_Datatype  datatype;
MPI_Comm      comm;
MPI_Status   *status;
{
    int          errno = MPI_SUCCESS;
    int          buflen;
    void         *rbuf;
    MPI_Status   status_array[2];
    MPI_Request  req[2];

    /* Check for invalid arguments */
    if ( MPIR_TEST_COMM(comm,comm) || MPIR_TEST_DATATYPE(comm,datatype) ||
	 MPIR_TEST_COUNT(comm,count) )
      return MPIR_ERROR( comm, errno, "Error in MPI_SENDRECV_REPL" );
    /* Let the other send/recv routines find the remaining errors. */

    /* Allocate a temporary buffer that is long enough to receive the 
       message even if it has holes in it.  Perhaps a better way to 
       do this is if contiguous, then as here, else use pack/unpack
       to send contiguous data... 
     */
    if (count == 0 || datatype->is_contig) {
	buflen = datatype->extent * count;
	if (errno = MPI_Isend ( buf,  count, datatype, dest,   
			       sendtag, comm, &req[0] )) return errno;
	if (buflen > 0) {
	    rbuf = (void *)MALLOC( buflen );
	    if (!rbuf) {
		return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
				  "Error in MPI_SENDRECV_REPL" );
		}
	    }
	else
	    rbuf = (void *)0;
	
	if (errno = MPI_Irecv ( rbuf, count, datatype, source, 
			    recvtag, comm, &req[1] )) return errno;
	errno = MPI_Waitall ( 2, req, status_array );
	if (rbuf) {
	    memcpy( buf, rbuf, buflen );
	    FREE( rbuf );
	    }
	(*status) = status_array[1];
	}
    else {
	int position;
	/* non-contiguous data will be packed and unpacked */
	MPI_Pack_size( count, datatype, comm, &buflen );
	if (buflen > 0) {
	    rbuf = (void *)MALLOC( buflen );
	    if (!rbuf) {
		return MPIR_ERROR( comm, MPI_ERR_EXHAUSTED, 
				  "Error in MPI_SENDRECV_REPL" );
		}
	    }
	else
	    rbuf = (void *)0;

	position = 0;
	MPI_Pack( buf, count, datatype, rbuf, buflen, &position, comm );
	errno = MPI_Sendrecv_replace( rbuf, buflen, MPI_PACKED, dest, sendtag, 
			  source, recvtag, comm, status );
	if (errno) {
	    if (rbuf) FREE( rbuf );
	    return errno;
	    }
	position = 0;
	MPI_Unpack( rbuf, buflen, &position, buf, count, datatype, comm );
	if (rbuf) {
	    FREE( rbuf );
	    }
	/* Still need to update status value? */
	}
    return errno;
}
