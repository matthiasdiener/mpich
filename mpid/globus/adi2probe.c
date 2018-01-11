/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2probe.c,v 1.3 1998/06/08 19:39:54 karonis Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */


#include "mpid.h"
#include "dev.h"
#include "mpid_debug.h"
#include "../util/queue.h"
#include "nexuspriv.h"

static void MPID_Get_partial_elements(int req_nelem,
				struct MPIR_DATATYPE *req_datatype,
				int dataformat,
				int *nbytes_remaining,
				int *elements,
				globus_bool_t *done);


void MPID_Iprobe(struct MPIR_COMMUNICATOR *comm,
		 int tag,
		 int context_id,
		 int src_lrank,
		 int *found,
		 int *error_code,
		 MPI_Status *status)
{
    MPIR_RHANDLE *rhandle = NULL;

    /* Let's sneak a poll in here in case someone sits on an Iprobe */
    globus_poll();

    MPID_Search_unexpected_queue(src_lrank,
				 tag,
				 context_id,
				 GLOBUS_FALSE,
				 &rhandle);
    if (rhandle)
    {
	*found = GLOBUS_TRUE;
	/* begin NICK */
	/* status->count = rhandle->count; */
	/* end NICK */
	status->MPI_SOURCE = rhandle->s.MPI_SOURCE;
	status->MPI_TAG = rhandle->s.MPI_TAG;
	status->MPI_ERROR = rhandle->s.MPI_ERROR;

	/* begin NICK */
	/* new Nexus device fields in MPI_Status */
	status->count = rhandle->dataorigin_nonpacksize;
	status->private_count = rhandle->dataorigin_format;
	SET_STATUSCOUNT_ISDATAORIGIN(status->private_count)
	/* end NICK */
    }
    else
    {
	*found = GLOBUS_FALSE;
    }
}

void MPID_Probe(struct MPIR_COMMUNICATOR *comm,
		int tag,
		int context_id,
		int src_lrank,
		int *error_code,
		MPI_Status *status)
{
    int found;

    *error_code = 0;

    while(1)
    {
	MPID_Iprobe(comm, tag, context_id, src_lrank, &found, error_code, status);
	if (found || *error_code)
	{
	    break;
	}
	globus_poll_blocking();
    }
}

int MPID_Get_count(MPI_Status *status, 
		    MPI_Datatype datatype,
		    int *count)
{

struct MPIR_DATATYPE *dtype_ptr; 
static char myname[] = "MPID_GET_COUNT";
int dataorigin_unitsize;
int dataorigin_format;
int dummy;

    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

/* globus_nexus_printf("NICK: enter MPID_Get_count() status->count = %d status->private_count = %0x dtype_ptr->size = %d\n", status->count, status->private_count, dtype_ptr->size); */
    if (dtype_ptr->size == 0)
    {
/* globus_nexus_printf("NICK: MPID_Get_count(): dtype_ptr->size == 0\n"); */
	/* if (status->count > 0 || status->dataorigin_nonpacksize > 0) */
	if (status->count > 0)
	    (*count) = MPI_UNDEFINED;
	else
          /* This is ambiguous */
          (*count) = 0;
    }
    else if (status->count == 0)
    {
        /* NICK */
        /* this is a KLUDGE.  When the user either sends/recvs to/from        */
        /* MPI_PROC_NULL the mpich source code sets fields in MPI_Status,     */
        /* including status->count = 0, *without* calling any of our MPID_xxx */
        /* routines at all. This is a problem for us because we have added    */
        /* fields to MPI_Status (status->private_count) that tell us how to   */
        /* interpret status->count,  (either datorigin non-pack size, local   */
        /* byte count, or data has not been received by the handler and       */
        /* status->count reflects the number of bytes the user has provided   */
        /* for a receive buffer.  Without the opporunity to appropriately     */
        /* set status->private_count when mpich sets status->count = 0, it    */
        /* causes an error in our code here when we attempt to interpret      */
        /* status->private_count because it has uninitialized gargabe in it.  */
        /* The KLUDGE solution here is to check when status->count == 0 and   */
        /* simply set *count = 0.  This is not too bad because if you inspect */
        /* the code below you will see that whenever status->count == 0 then  */
        /* *count is set to 0, independent of the value in                    */
        /* status->private_count.                                             */
 
        (*count) = 0;

    }
    else if (STATUSCOUNT_ISZERO(status->private_count))
    {
	/* When the user either sends/recvs to/from        */
	/* MPI_PROC_NULL the mpich source simply zero's the status->count */
	/* by calling MPID_ZERO_STATUS_COUNT, which we have defined (mpid.h)  */
	/* to not only set status->count = 0, but to also set                 */
	/* to ISZERO to tell us that the count field has been intentionally   */
	/* set to zero and that all count-related functions should return 0.  */

	(*count) = 0;
    }
    /* else */
    /* { */
	/* if (status->count != 0 || status->dataorigin_nonpacksize == 0) */
	else if (STATUSCOUNT_ISLOCAL(status->private_count))
	{
/* globus_nexus_printf("NICK: MPID_Get_count(): RECV has been processed\n"); */
	    /* RECV has been processed on this message ... */
	    /* status->count represents local byte count of received message */
	    if ((status->count % (dtype_ptr->size)) != 0)
		(*count) = MPI_UNDEFINED;
	    else
		(*count) = status->count / (dtype_ptr->size);
	}
	else if (STATUSCOUNT_ISDATAORIGIN(status->private_count))
	{
/* globus_nexus_printf("NICK: MPID_Get_count(): RECV has NOT been processed\n"); */
	    /* RECV has NOT been processed but the data has been received   */
	    /* by the handler.  This is the first chance that we've had to  */
	    /* see the data request type, so we may now calculate the bytes */
	    /* it will take to store the data on the receiving side.        */
	    /* status->count represents dataorigin_nonpacksize.  we must    */
	    /* extract dataorigin_format from status->private_count               */

	    dataorigin_format = 
		STATUSCOUNT_EXTRACT_FORMAT(status->private_count);
	    dummy = 0;
	    dataorigin_unitsize = 0;
	    MPID_Pack_exact_size(1,
				dtype_ptr,
				/* status->dataorigin_format, */
				dataorigin_format,
				&dummy,
				&dataorigin_unitsize);
/* globus_nexus_printf("NICK: MPID_Get_count(): dataorigin_unitsize %d\n", dataorigin_unitsize); */
	    /* if (status->dataorigin_nonpacksize % dataorigin_unitsize) */
	    if (status->count % dataorigin_unitsize)
		(*count) = MPI_UNDEFINED;
	    else
		/* (*count) = status->dataorigin_nonpacksize / dataorigin_unitsize; */
		(*count) = status->count / dataorigin_unitsize;
	} 
	else if (STATUSCOUNT_ISRCVBUFSIZE(status->private_count))
	{
	    globus_nexus_printf("FATAL ERROR: MPID_Get_count(): encountered invalid status->private_count %0 which is STATUSCOUNT_RCVBUFSIZE\n");
	    exit(1);
	}
	else
	{
	    globus_nexus_printf("FATAL ERROR: MPID_Get_count(): encountered invalid status->private_count %0x\n", 
		status->private_count);
	    exit(1);
	} /* endif */
    /* }  */
/* globus_nexus_printf("NICK: exit MPID_Get_count() with count %d\n", *count); */

    return MPI_SUCCESS;

} /* end MPID_Get_count() */

int MPID_Get_elements(MPI_Status *status, 
		    MPI_Datatype  datatype,
		    int *elements)
{

struct MPIR_DATATYPE *dtype_ptr;
static char myname[] = "MPID_GET_ELEMENTS";
int count;
int nfull;
int nbytes_remaining;
int dummy;
int dataorigin_unitsize;
int dataorigin_format;
globus_bool_t done;

    dtype_ptr   = MPIR_GET_DTYPE_PTR(datatype);
    MPIR_TEST_DTYPE(datatype,dtype_ptr,MPIR_COMM_WORLD,myname);

    MPID_Get_count(status, datatype, &count);
    if (count == MPI_UNDEFINED)
    {
	/* data was missing from the end of the last message received */
	/* ... need to calculate elements by hand.                    */

	/* if (status->count != 0 || status->dataorigin_nonpacksize == 0) */
	if (STATUSCOUNT_ISLOCAL(status->private_count))
	{
	    /* RECV has been processed on this message ... */
            /* status->count represents local byte count of received message */

	    /* number of elements in first N-1 complete items */
	    nfull = status->count/dtype_ptr->size;
	    *elements = nfull * dtype_ptr->elements;

	    /* now calculating remaining elements by hand */
	    if (nbytes_remaining = status->count - (nfull * dtype_ptr->size))
	    {
		done = GLOBUS_FALSE;
		MPID_Get_partial_elements(1,
					dtype_ptr,
					NEXUS_DC_FORMAT_LOCAL,
					&nbytes_remaining,
					elements,
					&done);
	    }
	    else
	    {
		globus_fatal("MPID_Get_elements(): encountered incoming message that has already had been processed by RECV with incomplete last array element, but nbytes_remaining in last element == 0\n");
	    } /* endif */
	}
	else if (STATUSCOUNT_ISDATAORIGIN(status->private_count))
	{
	    /* RECV has NOT been processed but the data has been received    */
	    /* by the handler.  This is the first chance that we've had to   */
	    /* see the data request type, so we may now count elements using */
	    /* dataorigin format and bytecount.                              */

	    dummy = 0;
	    dataorigin_unitsize = 0;
            dataorigin_format = 
    	    	STATUSCOUNT_EXTRACT_FORMAT(status->private_count);
	    MPID_Pack_exact_size(1,
				dtype_ptr,
				/* status->dataorigin_format, */
				dataorigin_format,
				&dummy,
				&dataorigin_unitsize);

	    /* number of elements in first N-1 complete items */
	    /* nfull = status->dataorigin_nonpacksize / dataorigin_unitsize; */
	    nfull = status->count / dataorigin_unitsize;
	    *elements = nfull * dtype_ptr->elements;
	    
	    /* now calculating remaining elements by hand */
	    if (nbytes_remaining = 
	/* status->dataorigin_nonpacksize  - (nfull * dataorigin_unitsize)) */
		status->count  - (nfull * dataorigin_unitsize))
	    {
		done = GLOBUS_FALSE;
		MPID_Get_partial_elements(1,
					dtype_ptr,
					/* status->dataorigin_format, */
					dataorigin_format,
					&nbytes_remaining,
					elements,
					&done);
	    }
	    else
	    {
		globus_fatal("MPID_Get_elements(): encountered incoming message that has already had NOT been processed by RECV with incomplete last array element, but nbytes_remaining in last element == 0\n");
	    } /* endif */
	} 
	else if (STATUSCOUNT_ISRCVBUFSIZE(status->private_count))
	{
	    globus_nexus_printf("FATAL ERROR: MPID_Get_elements(): encountered invalid status->private_count %0 which is STATUSCOUNT_RCVBUFSIZE\n");
	    exit(1);
	}
    	else
    	{
            globus_nexus_printf("FATAL ERROR: MPID_Get_elements(): encountered invalid status->private_count %0x\n", 
                status->private_count);
            exit(1);
	} /* endif */
    }
    else
	/* complete data received */
        (*elements) = count * dtype_ptr->elements;

    return MPI_SUCCESS;


} /* end MPID_Get_elements() */

/*
 *
 * It is assumed that *done == GLOBUS_FALSE upon entering this function.
 *
 */

static void MPID_Get_partial_elements(int req_nelem,
				struct MPIR_DATATYPE *req_datatype,
				int dataformat,
				int *nbytes_remaining,
				int *elements,
				globus_bool_t *done)
{
    int i, j;
    int unitsize;
    int nelem;
    int inbuf_nelem;
    int dummy;

    switch (req_datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
      case MPIR_UCHAR:
      case MPIR_PACKED:
      case MPIR_BYTE:
      case MPIR_SHORT:
      case MPIR_USHORT:
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
      case MPIR_UINT:
      case MPIR_LONG:
      case MPIR_ULONG:
      case MPIR_FLOAT:
      case MPIR_COMPLEX:
      case MPIR_DOUBLE:
      case MPIR_DOUBLE_COMPLEX:
	    unitsize = 0;
	    dummy = 0;
	    MPID_Pack_exact_size(1,
			req_datatype,
			dataformat,
			&dummy,
			&unitsize);

	    inbuf_nelem = (*nbytes_remaining) / unitsize;
	    if (nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	    {
		*nbytes_remaining = *nbytes_remaining - (nelem * unitsize);
		*elements = *elements + nelem;
	    } /* endif */

	    if (nelem < req_nelem)
	    {
		*done = GLOBUS_TRUE;
		if (*nbytes_remaining > 0)
		{
		    globus_nexus_printf("WARNING: after counting %d elements of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT counted (all byte counts in dataorigin format)\n", nelem, req_datatype->dte_type, nelem * unitsize, *nbytes_remaining);
		} /* endif */
	    } /* endif */
	    break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
	MPID_Get_partial_elements(req_nelem * req_datatype->count,
				req_datatype->old_type,
				dataformat,
				nbytes_remaining,
				elements,
				done);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	for (i = 0; !(*done) && i < req_nelem; i++)
	{
	    for (j = 0; !(*done) && j < req_datatype->count; j++)
	    {
		MPID_Get_partial_elements(req_datatype->blocklen,
				       req_datatype->old_type,
				       dataformat,
				       nbytes_remaining,
				       elements,
				       done);
	    } /* endfor */
	} /* endfor */
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; !(*done) && i < req_nelem; i ++)
	{
	    for (j = 0; !(*done) && j < req_datatype->count; j ++)
	    {
		MPID_Get_partial_elements(req_datatype->blocklens[j],
					req_datatype->old_types[j],
					dataformat,
					nbytes_remaining,
					elements,
					done);
	    } /* endfor */
	} /* endfor */
	break;
      case MPIR_STRUCT:
	for (i = 0; !(*done) && i < req_nelem; i ++)
	{
	    for (j = 0; !(*done) && j < req_datatype->count; j ++)
	    {
		MPID_Get_partial_elements(req_datatype->blocklens[j],
					req_datatype->old_types[j],
					dataformat,
					nbytes_remaining,
					elements,
					done);
	    } /* endfor */
	} /* endfor */
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      /* 
       * Nexus cannot deal with these types directly, and they cannot be
       * broken down into more simple parts.  Return an error to the
       * calling procedure to indicate the problem.
       */
      /* case MPIR_LOGICAL: */
      case MPIR_LONGDOUBLE:
	/* *error_code = MPI_ERR_TYPE; */
	return ;
      /* 
       * This is not a legal type.  Return an error code.
       */
      default:
	/* *error_code = MPI_ERR_INTERN; */
	return ;
    }

} /* end MPID_Get_partial_elements() */
