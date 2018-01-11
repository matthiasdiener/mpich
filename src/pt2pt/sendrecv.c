/*
 *  $Id: sendrecv.c,v 1.7 1994/08/10 14:06:30 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: sendrecv.c,v 1.7 1994/08/10 14:06:30 gropp Exp $";
#endif /* lint */

#include "mpiimpl.h"

/*@
    MPI_Sendrecv - Sends and receives a message

Input Parameters:
. sendbuf - initial address of send buffer (choice) 
. sendcount - number of elements in send buffer (integer) 
. sendtype - type of elements in send buffer (handle) 
. dest - rank of destination (integer) 
. sendtag - send tag (integer) 
. recvcount - number of elements in receive buffer (integer) 
. recvtype - type of elements in receive buffer (handle) 
. source - rank of source (integer) 
. recvtag - receive tag (integer) 
. comm - communicator (handle) 

Output Parameters:
. recvbuf - initial address of receive buffer (choice) 
. status - status object (Status).  This refers to the receive operation.
  

@*/
int MPI_Sendrecv( sendbuf, sendcount, sendtype, dest, sendtag, 
                  recvbuf, recvcount, recvtype, source, recvtag, 
                  comm, status )
void         *sendbuf;
int           sendcount;
MPI_Datatype  sendtype;
int           dest, sendtag;
void         *recvbuf;
int           recvcount;
MPI_Datatype  recvtype;
int           source, recvtag;
MPI_Comm      comm;
MPI_Status   *status;
{
    int               errno = MPI_SUCCESS;
    MPI_Status        status_array[2];
    MPI_Request       req[2];

    /* Let the Isend/Irecv check arguments */

    /* Comments on this:
       We can probably do an Irecv/Send/Wait on Irecv (blocking send)
       but what we really like to do is "send if odd, recv if even, 
       followed by send if even, recv if odd".  We can't do that, 
       because we don't require that these match up in any particular
       way (that is, there is no way to assert the "parity" of the 
       partners).  Note that the IBM "mp_bsendrecv" DOES require that
       only mp_bsendrecv be used.  

       Should there be a send/recv bit in the send mode? 
     */
    if (errno = MPI_Irecv ( recvbuf, recvcount, recvtype,
			    source, recvtag, comm, &req[1] )) return errno;
    if (errno = MPI_Isend ( sendbuf, sendcount, sendtype, dest,   
			    sendtag, comm, &req[0] )) return errno;
    errno = MPI_Waitall ( 2, req, status_array );

    (*status) = status_array[1];
    return (errno);
}
