/*
 *  $Id: dmpi.c,v 1.21 1994/11/23 16:25:53 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: dmpi.c,v 1.21 1994/11/23 16:25:53 gropp Exp $";
#endif /* lint */

/*  dmpi.c - routines in mpir that are called by the device */

#include "mpiimpl.h"
#include "mpisys.h"

#define MPIR_MIN(a,b) (a) < (b) ? (a) : (b)
#define DEVICE_PREFERS_MEMCPY 1

/* called by device when a message arrives.  Returns 1 if there is posted
 * receive, 0 otherwise.
 *
 * This puts the responsibility for searching the unexpected queue and
 * posted-receive queues on the device.  If it is operating asynchronously
 * with the user code, the device must provide the necessary locking mechanism.
 */

DMPI_msg_arrived( src, tag, context_id, dmpi_recv_handle, foundflag )
int               src, tag, *foundflag;
MPIR_CONTEXT      context_id;
MPIR_RHANDLE      **dmpi_recv_handle;
{
    int          found;
    MPIR_RHANDLE *handleptr;

    MPIR_search_posted_queue( src, tag, context_id, &found, 1, 
			      dmpi_recv_handle);
    if ( found )
    {
	*foundflag = 1;	
	/* note this overwrites any wild-card values in the posted handle */
	handleptr         	= *dmpi_recv_handle;
	handleptr->source 	= src;
	handleptr->tag  	= tag;
	/* count is set in the put and get routines */
    }
    else
    {
	/* allocate handle and put in unexpected queue */
	*dmpi_recv_handle       = 
	    ( MPIR_RHANDLE * ) MPIR_SBalloc ( MPIR_rhandles );
	handleptr         	= *dmpi_recv_handle;
	handleptr->handle_type  = MPIR_RECV;
	handleptr->source 	= src;
	handleptr->tag  	= tag;
	handleptr->contextid    = context_id;
	handleptr->datatype     = MPI_BYTE;
	handleptr->completed    = MPIR_NO;
	MPID_Alloc_recv_handle( handleptr->comm->ADIctx,
			        &(handleptr->dev_rhandle) );
	
	MPIR_enqueue( &MPIR_unexpected_recvs, (void * ) *dmpi_recv_handle,
                      MPIR_QSHANDLE );
	*foundflag = 0;
    }
}



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

#ifdef MPID_HAS_HETERO
/* Returns 2 if the data needs XDR conversion,
           1 if the data needs byteswapping,
	   0 if none of the above... */
int 
MPIR_Dest_needs_conversion(dest)
int dest;
{
  if ( (MPID_Dest_byte_order(MPIR_tid) == MPID_H_XDR) ||
       (MPID_Dest_byte_order(dest) == MPID_H_XDR))
    return 2;
  else if (MPID_Dest_byte_order(MPIR_tid) != MPID_Dest_byte_order(dest))
    return 1;
  else
    return 0;
}


int 
MPIR_Comm_needs_conversion(comm)
MPI_Comm comm;
{
  int i;
  for (i = 0; i < comm->local_group->np; i++) {
    if (MPIR_Dest_needs_conversion(comm->local_group->lrank_to_grank[i]))
      return 2;
  }
  return 0;
}
#endif

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
      MPIR_Type_convert_copy( handle->comm, 
			     addr, handle->bufpos, handle->datatype,
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

/*
   Let the device tell the API that a handle can be freed (this handle
   was generated by an unexpected receive and inserted by DMPI_msg_arrived.
 */
DMPI_free_unexpected( dmpi_recv_handle  )
MPIR_RHANDLE      *dmpi_recv_handle;
{
MPIR_SBfree( MPIR_rhandles, dmpi_recv_handle );
}


int
MPIR_Send_setup(request)
MPI_Request *request;
{
  register MPIR_SHANDLE *shandle;
  register int errno = MPI_SUCCESS;
  int dest_type;

  shandle = &(*request)->shandle;
  if (shandle->dest == MPI_PROC_NULL) return errno;

  shandle->active       = 1;

  /* XXXX - marks development comments to be taken out after design is 
            ironed out */
 
  if (shandle->datatype->is_contig)
#ifdef MPID_HAS_HETERO
    if ((MPID_IS_HETERO == 1) &&
	(dest_type = MPIR_Dest_needs_conversion(shandle->dest))) {
      /* This is a heterogeneous case - can't send from the user's
	 buffer   because we have to swap. This would be faster if the 
	 device swapped as it copied, but this is a MPIR level 
	 heterogeneous implementation. */
      shandle->dev_shandle.bytes_as_contig =
	shandle->count * shandle->datatype->extent;
      /* XDR buffer must be a multiple of 4 in size! */
      if ((dest_type == 2) && 
	  (shandle->datatype->extent <= 4)) 
	shandle->bufpos = (char *)MALLOC(shandle->count * 4);
      else
	shandle->bufpos = (char *)MALLOC(shandle->dev_shandle.bytes_as_contig);

      shandle->dev_shandle.start = shandle->bufpos;
      MPIR_Type_convert_copy( shandle->comm, shandle->dev_shandle.start,
			     shandle->bufadd, shandle->datatype, 
			     shandle->count, shandle->dest,
			     &shandle->msgrep);
    } else 
#endif
    {
#ifdef MPID_HAS_HETERO
      shandle->msgrep = MPIR_MSGREP_RECEIVER;
#endif
      shandle->dev_shandle.bytes_as_contig =
	shandle->count * shandle->datatype->extent;
      if (shandle->dev_shandle.bytes_as_contig > 0 && shandle->bufadd == 0)
	  errno = MPI_ERR_BUFFER;
      shandle->dev_shandle.start = shandle->bufadd;
      shandle->bufpos		 = 0;
    }
  else
#ifdef MPID_PACK_IN_ADVANCE
  {
    /* Heterogeneous case handled in MPIR_Pack and MPIR_Pack_Hvector */
    if (errno = 
	MPIR_PackMessage(shandle->bufadd, shandle->count, 
			 shandle->datatype, shandle->dest, *request )) {
      MPIR_ERROR( MPI_COMM_WORLD, errno, 
		 "Could not pack message in MPIR_Send_setup" );
    }
  }
#else
  {
    shandle->dev_shandle.start = 0; /* Heterogeneous case handled in 
				       get_into_contig for short messages.
				       This breaks on long messages because
				       the p4 device passes a -1 for maxlen
				       to get_from_contig XXX...*/
  }
#endif
  return errno;

}


int 
MPIR_SendBufferFree( request )
MPI_Request request;
{
  FREE( request->chandle.bufpos );
  return MPI_SUCCESS;
}


int 
MPIR_Receive_setup(request)
MPI_Request *request;
{
  MPIR_RHANDLE *rhandle;
  int errno = MPI_SUCCESS;


  rhandle = &(*request)->rhandle;
  if (rhandle->source == MPI_PROC_NULL) return errno;
  rhandle->active       = 1;
  
  if (rhandle->datatype->is_contig) {
    rhandle->dev_rhandle.start = rhandle->bufadd;
    rhandle->dev_rhandle.bytes_as_contig =
      rhandle->count * rhandle->datatype->extent;
    if (rhandle->dev_rhandle.bytes_as_contig > 0 && 
	rhandle->bufadd == 0) 
	errno = MPI_ERR_BUFFER;
    rhandle->bufpos                      = 0;
  }
#ifdef MPID_RETURN_PACKED
  else {
    if (errno = 
	MPIR_SetupUnPackMessage( rhandle->bufadd, rhandle->count, 
				rhandle->datatype, rhandle->source, *request )) {
      MPIR_ERROR( MPI_COMM_WORLD, errno, 
		 "Could not pack message in MPI_Receive_setup" );
    }
  }
#else
  else 
    rhandle->dev_rhandle.start = 0;
#endif

  return errno;
}

/*
   This is drawn directly from the code in src/pt2pt/unpack; both should be
   modified together.
 */
int MPIR_PrintDatatype ( fp, count, type, in_offset, out_offset )
FILE         *fp;
int          count;
MPI_Datatype type;
int          in_offset, out_offset;
{
  int i,j,k;
  int pad = 0;
  int errno = MPI_SUCCESS;
  int tmp_offset;
/*   char *lbuf = (char *)buf, *lin = (char *)in; */

  if (in_offset == 0 && out_offset == 0) 
      fprintf( fp, "Commands to unpack datatype:\n" );
  /* Unpack contiguous data */
  if (type->is_contig) {
      fprintf( fp, "Contiguous type:" );
      fprintf( fp, " Copy %d <- %d for %d bytes\n", out_offset, in_offset,
	       type->size * count );
      return errno;
      }

  /* For each of the count arguments, unpack data */
  switch (type->dte_type) {

  /* Contiguous types */
  case MPIR_CONTIG:
        fprintf( fp, "MPIR_CONTIG:\n" );
	errno = MPIR_PrintDatatype ( fp, count * type->count, type->old_type, 
				     in_offset, out_offset );
	break;

  /* Vector types */
  case MPIR_VECTOR:
  case MPIR_HVECTOR:
	fprintf( fp, "MPIR_(H)VECTOR:\n" );
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	tmp_offset = out_offset;
	for (i=0; i<count; i++) {
	  out_offset = tmp_offset;
	  for (j=0; j<type->count; j++) {
		if (errno = MPIR_PrintDatatype ( fp, type->blocklen, 
					 type->old_type, 
				       in_offset, out_offset )) return errno;
		out_offset  += (type->stride);
		if ((j+1) != type->count)
		  in_offset += 
		      ((type->blocklen * type->old_type->size) + type->pad);
	  }
	  in_offset += ((type->blocklen * type->old_type->size) + pad);
	  tmp_offset += type->extent;
	}
	break;

  /* Indexed types */
  case MPIR_INDEXED:
  case MPIR_HINDEXED:
	fprintf( fp, "MPIR_(H)INDEXED:\n" );
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	for (i=0; i<count; i++) {
	  for (j=0;j<type->count; j++) {
		tmp_offset  = out_offset + type->indices[j];
		if (errno = MPIR_PrintDatatype (fp, type->blocklens[j], 
					 type->old_type, 
					 in_offset, tmp_offset)) return errno;
		if ((j+1) != type->count)
		  in_offset += 
		    ((type->blocklens[j]*type->old_type->size)+type->pad);
	  }
	  in_offset += ((type->blocklens[j]*type->old_type->size) + pad);
	  out_offset += type->extent;
	}
	break;

  /* Struct type */
  case MPIR_STRUCT:
	fprintf( fp, "MPIR_(H)STRUCT:\n" );
	if (count > 1)
	  pad = (type->align - (type->size % type->align)) % type->align;
	for (i=0; i<count; i++) {
	  for (j=0;j<type->count; j++) {
		tmp_offset  = out_offset + type->indices[j];
		if (errno = MPIR_PrintDatatype( fp, type->blocklens[j],
					type->old_types[j], 
				       in_offset, tmp_offset)) return errno;
		if ((j+1) != type->count)
		  in_offset += 
		      ((type->blocklens[j] * type->old_types[j]->size) +
						 type->pads[j]);
	  }
	  in_offset+=((type->blocklens[type->count-1]*
				   type->old_types[type->count-1]->size)+pad);
	  out_offset +=type->extent;
	}
	break;

  default:
	errno = MPI_ERR_TYPE;
	break;
  }

  /* Everything fell through, must have been successful */
  return errno;
}
