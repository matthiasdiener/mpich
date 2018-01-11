/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2recv.c,v 1.2 1998/05/18 18:13:33 gropp Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */


#include "mpid.h"
#include "dev.h"
#include "../util/queue.h"
#include "nexuspriv.h"

void MPID_RecvContig(struct MPIR_COMMUNICATOR *comm,
		     void *buf,
		     int maxlen,
		     int src_lrank,
		     int tag,
		     int context_id,
		     MPI_Status *status,
		     int *error_code)
{
    MPIR_RHANDLE rhandle;
    MPI_Request request = (MPI_Request)&rhandle;

    *error_code = 0;
    MPID_IrecvContig(comm,
    		     buf,
		     maxlen,
		     src_lrank,
		     tag,
		     context_id,
		     request,
		     error_code);
    if (!*error_code)
    {
	MPID_RecvComplete(request, status, error_code);
    }
}

void MPID_IrecvContig(struct MPIR_COMMUNICATOR *comm,
		      void *buf,
		      int maxlen,
		      int src_lrank,
		      int tag,
		      int context_id,
		      MPI_Request request,
		      int *error_code)
{
    MPIR_RHANDLE *rhandle = &request->rhandle;
    
    if (!buf && maxlen > 0)
    {
	*error_code = MPI_ERR_BUFFER;
	return ;
    }

    /* set fields in rhandle */
    rhandle->len = maxlen;
    rhandle->buf = buf;
    rhandle->is_complete = 0;
	/* any others? */

    /* check queue for matching receive */
}

int MPID_RecvIcomplete(MPI_Request request,
		       MPI_Status *status,
		       int *error_code)
{
    MPIR_RHANDLE *rhandle = &request->rhandle;

    /*
     * Prevent any other thread from messing with this request while it
     * is in the requestQ.
     */
    globus_mutex_lock(&message_queue_lock);

    if (rhandle->is_complete)
    {
	/* cleanup dynamic stuff in rhandle */

	if (status)
	{
	    *status = rhandle->s;
	}
	*error_code = rhandle->s.MPI_ERROR;
        globus_mutex_unlock(&message_queue_lock);
	return GLOBUS_TRUE;
    }

    globus_mutex_unlock(&message_queue_lock);
    return GLOBUS_FALSE;
}

void MPID_RecvComplete(MPI_Request request,
		       MPI_Status *status,
		       int *error_code)
{
    MPIR_RHANDLE *rhandle = &request->rhandle;
    MPID_cond_list *cond_l;

    globus_mutex_lock(&message_queue_lock);

    MPID_Get_Cond_Variable(cond_l);
    rhandle->cond = &cond_l->cond;

    /* wait for request to complete */
    while (!rhandle->is_complete)
    {
	globus_cond_wait(rhandle->cond, &message_queue_lock);
    }

    if (rhandle->type)
      MPIR_REF_DECR(rhandle->type);

    MPID_Free_Cond_Variable(cond_l);
    globus_mutex_unlock(&message_queue_lock);

    if (status)
    {
	status->count = rhandle->s.count;
	status->MPI_SOURCE = rhandle->s.MPI_SOURCE;
	status->MPI_TAG = rhandle->s.MPI_TAG;
        status->MPI_ERROR = rhandle->s.MPI_ERROR;
        /* begin NICK */
        /* status->dataorigin_format = rhandle->dataorigin_format; */
        /* status->dataorigin_nonpacksize = rhandle->dataorigin_nonpacksize; */

      /* set byte in status->private_count to indicate that status->count */ 
      /* should be interpreted as local byte count ... whenever status->count */
      /* can be interpreted in this way, we do not need to store              */
      /* dataorigin_format in status->private_count as well ... we can ignore */
      /* dataorigin_format and dataorigin_nonpacksize                   */
      SET_STATUSCOUNT_ISLOCAL(status->private_count)
        /* end NICK */
    }
    *error_code = rhandle->s.MPI_ERROR;
}
