/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*  dmpipkt.c - routines in mpir that are called by the device for incremental 
    transfers.  NOT IN USE
 */

#include "dmpi.h"
#include "mpiimpl.h"
#include "mpisys.h"

#define MPIR_MIN(a,b) (a) < (b) ? (a) : (b)
#define DEVICE_PREFERS_MEMCPY 1
#if 0
/*
 *  This code implements one of the interfaces between the device and the
 *  MPI code for moving data between the two systems.  This code delivers
 *  to the device contiguous chuncks of data, packing or unpacking as
 *  required.
 */

/* get length of data to be sent
 */
DMPI_get_totallen( handle, len )
MPIR_SHANDLE *handle;
int          *len;
{
    *len = (handle->count)*(handle->datatype->size);
				/* correct for contig messages, at least*/
    handle->totallen       = *len;
    handle->bufpos         = handle->bufadd;
    handle->transfer_count = 0;
}

/* tell mpi how many bytes have been read 
 */
DMPI_put_totallen( handle, len )
MPIR_RHANDLE *handle;
int          len;
{
    handle->totallen       = len;
    handle->bufpos         = handle->bufadd;
    handle->transfer_count = 0;
}
#endif 

#if 0
/* SENDS */


/* device has send buffer ready, wants mpi to copy data to it

   Should be called only for len > 0
 */
int DMPI_get_into_contig( handle, addr, maxlen, actlen )
MPIR_SHANDLE *handle;
void         *addr;
int          maxlen;
int          *actlen;
{
  int len_left;

len_left = (handle->count)*(handle->datatype->size) - 
    handle->transfer_count;

if (!handle->bufpos) {
    return MPI_ERR_BUFFER;
    }
*actlen = MPIR_MIN(len_left,maxlen);

#ifdef DEVICE_PREFERS_MEMCPY
#ifdef MPID_HAS_HETERO
    if ((MPID_IS_HETERO == 1) && 
	MPIR_Dest_needs_conversion(handle->dest)) {
      /* Make sure actlen is a multiple of the datatype size */
      *actlen = (*actlen) / (handle->datatype->size);
      *actlen = (*actlen) * (handle->datatype->size);
      /* If XDR is used, the length may be changed AGAIN */
      *actlen = MPIR_Type_convert_copy( handle->comm, 
			     addr, (maxlen?), handle->bufpos, handle->datatype,
			     (*actlen)/(handle->datatype->size),
			     handle->dest, &handle->msgrep);
    }
    else 
#endif
    {
      handle->msgrep = MPIR_MSGREP_RECEIVER;
      memcpy( addr, handle->bufpos, *actlen );
    }
#else
    /* device-defined macro for data movement, for example load-store through
       vector registers. */
#endif
    handle->bufpos         += *actlen;
    handle->transfer_count += *actlen;

return MPI_SUCCESS;
}

/* device will copy mpi's contiguous buffer, wants to know where it is
 */
int DMPI_get_from_contig( handle, addr, maxlen, actlen )
MPIR_SHANDLE *handle;
void         **addr;
int          maxlen;
int          *actlen;
{
    /* XXX - This needs to handle the heterogeneous case and be fixed for the
       case where maxlen = -1 */
    if (!handle->bufpos) return MPI_ERR_BUFFER;

    *addr   = handle->bufpos;
    *actlen = (handle->count)*(handle->datatype->size);
    if (*actlen > maxlen) *actlen = maxlen;
    handle->bufpos         += *actlen;
    handle->transfer_count += *actlen;
/* XXX - This really needs to be fleshed out! */
#ifdef MPID_HAS_HETERO 
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "DMPI_get_from_contig doesn't support heterogeneous systems. \
 Complain Loudly to mpi-bugs@mcs.anl.gov.");
    handle->msgrep = MPIR_MSGREP_RECEIVER;
#endif
    return MPI_SUCCESS;
}

/* RECEIVES */
/* device will put data into mpi's contiguous buffer, wants to know where it is
 */
DMPI_put_into_contig( handle, addr, maxlen, actlen )
MPIR_RHANDLE *handle;
void         **addr;
int          maxlen;
int          *actlen;
{
    *addr                  = handle->bufpos;
    *actlen                = (handle->actcount)*(handle->datatype->size);
    if (*actlen > maxlen) *actlen = maxlen;
    handle->bufpos         += *actlen;
    handle->transfer_count += *actlen;
}

/* 
 * device has contiguous buffer ready, wants mpi to copy data from it
 * XXXX - This routine should handle de-xdring for receives.
 */
DMPI_put_from_contig( handle, addr, maxlen )
MPIR_RHANDLE *handle;
void         *addr;
int          maxlen;
{
    int actlen;

    actlen = MPIR_MIN( maxlen, handle->totallen - handle->transfer_count);

#ifdef DEVICE_PREFERS_MEMCPY
    memcpy( handle->bufpos, addr, actlen );
#else
    /* device-defined macro for data movement, for example load-store through
       vector registers */
#endif
    handle->bufpos         += actlen;
    handle->transfer_count += actlen;
    handle->actcount        =  handle->transfer_count / 
	(handle->datatype->size);
}
#endif
