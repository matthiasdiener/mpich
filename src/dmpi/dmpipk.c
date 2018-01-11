/*
 *  $Id: dmpipk.c,v 1.24 1996/06/07 15:12:29 gropp Exp $
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpidmpi.h"
#else
#include "mpisys.h"
#endif

/* 
   This file contains the first pass at routines to pack and unpack datatypes
   for the ADI.  THESE WILL CHANGE

   In order to aid in debugging, it is possible to cause the datatype
   pack/unpack actions to be written out.
 */

/* Pack for a send.  Eventually, this will need to handle the Heterogeneous 
   case - XXXX.  */
void MPIR_Pack_Hvector( comm, buf, count, datatype, dest, outbuf )
MPI_Comm    comm;
char        *buf;
int         count, dest;
MPI_Datatype datatype;
char        *outbuf;
{
int count1 = datatype->count,           /* Number of blocks */
    blocklen = datatype->blocklen;      /* Number of elements in each block */
MPI_Aint    stride   = datatype->stride;  /* Bytes between blocks */
int extent = datatype->old_type->extent;  /* Extent of underlying type */
int blen   = blocklen * extent;
int c, i, j;

/* We can't use c = count * count1 since that moves the location of the second
   of the count elements after the stride from the first, rather than after the
   last element */
c = count1;

/* Handle the special case of 4 or 8 byte items, with appropriate 
   alignment.  We do this to avoid the cost of a memcpy call for each
   element.
 */
if (blen == 4 && ((MPI_Aint)buf & 0x3) == 0 && (stride & 0x3) == 0 && 
    sizeof(int) == 4 && ((MPI_Aint)outbuf & 0x3) == 0) {
    register int *outb = (int *)outbuf, *inb = (int *)buf;
    stride = stride >> 2;
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    outb[i] = *inb;
	    inb    += stride;
	    }
	inb  -= stride;
	inb  += 1;
	outb += c;
	}
    }
else if (blen == 8 && ((MPI_Aint)buf & 0x7) == 0 && (stride & 0x7) == 0 && 
	 sizeof(double) == 8 && ((MPI_Aint)outbuf & 0x7) == 0) {
    register double *outb = (double *)outbuf, *inb = (double *)buf;
    stride = stride >> 3;
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    outb[i] = *inb;
	    inb    += stride;
	    }
	inb -= stride;
	inb += 1;
	outb += c;
	}
    }
else {
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    memcpy( outbuf, buf, blen );
	    outbuf += blen; 
	    buf    += stride;
	    }
	buf -= stride;
	buf += blen;
	}
    }
}

void MPIR_UnPack_Hvector( inbuf, count, datatype, source, outbuf )
char        *inbuf;
int         count, source;
MPI_Datatype datatype;
char        *outbuf;
{
int count1 = datatype->count,            /* Number of blocks */
    blocklen = datatype->blocklen;       /* Number of elements in each block */
MPI_Aint    stride   = datatype->stride; /* Bytes between blocks */
int extent = datatype->old_type->extent;  /* Extent of underlying type */
int blen   = blocklen * extent;
register int c, i;
int          j;

/* We can't use c = count * count1 since that moves the location of the second
   of the count elements after the stride from the first, rather than after the
   last element */
c = count1;
if (blen == 4 && ((MPI_Aint)inbuf & 0x3) == 0 && (stride & 0x3) == 0 && 
    sizeof(int) == 4 && ((MPI_Aint)outbuf & 0x3) == 0 ) {
    register int *outb = (int *)outbuf, *inb = (int *)inbuf;
    stride = stride >> 2;
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    *outb = inb[i];
	    outb  += stride;
	    }
	outb -= stride;
	outb += 1;
	inb  += c;
	}
    }
else if (blen == 8 && ((MPI_Aint)inbuf & 0x7) == 0 && (stride & 0x7) == 0 && 
	 sizeof(double) == 8 && ((MPI_Aint)outbuf & 0x7) == 0) {
    register double *outb = (double *)outbuf, *inb = (double *)inbuf;
    stride = stride >> 3;
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    *outb   = inb[i];
	    outb    += stride;
	    }
	outb -= stride;
	outb += 1;
	inb += c;
	}
    }
else {
    for (j=0; j<count; j++) {
	for (i=0; i<c; i++) {
	    memcpy( outbuf, inbuf, blen );
	    outbuf += stride;
	    inbuf  += blen;
	    }
	outbuf -= stride;
	outbuf += blen;
	}
    }
}

/* Get the length needed for the Hvector as a contiguous lump */
int MPIR_HvectorLen( count, datatype )
int count;
MPI_Datatype datatype;
{
return datatype->size * count;
}

#ifndef MPI_ADI2
/*
    These routines pack/unpack data for messages.  They KNOW how the 
    message request areas are arranged, and how the send/receive routines
    known when to use these routines.
    
    dest_type is the format to use (XDR/Swap/Unchanged)
 */
int MPIR_PackMessage( buf, count, datatype, dest, dest_type, request )
char         *buf;
int          count, dest, dest_type;
MPI_Datatype datatype;
MPI_Request  request;
{
  int size;
  int mpi_errno = MPI_SUCCESS;
  
  if (!datatype->committed) 
    return MPI_ERR_TYPE | MPIR_ERR_UNCOMMITTED;

  if (count == 0) {
      request->chandle.bufpos			   = 0;
      request->shandle.dev_shandle.bytes_as_contig = 0;
      return MPI_SUCCESS;
      }

  if (datatype->dte_type == MPIR_PACKED) {
      /* Data requires no further modification */
      /* PRINTF( "Packing packed data!\n" ); */
      /* Mark this data appropriately and indicate that it is packed in the 
         request 
       */
#ifdef MPID_HAS_HETERO
      /* Hetero buffers have a 4 byte, network byte order, header
	 indicating type of representation */
      if (dest_type == MPIR_MSGFORM_XDR)
	  request->chandle.msgrep = MPIR_MSGREP_XDR;
      /* Otherwise in receivers order (may change when senders order 
	 supported) */
#endif
      request->chandle.bufpos			   = 0;
      request->shandle.dev_shandle.start           = buf;
      request->shandle.dev_shandle.bytes_as_contig = count;
      return MPI_SUCCESS;
      }
  /* Use the generic pack routine */
  MPIR_Pack_size( count, datatype, request->chandle.comm, dest_type, &size );
  if (size == 0) {
      request->chandle.bufpos			   = 0;
      request->shandle.dev_shandle.bytes_as_contig = 0;
      return mpi_errno;
      }
  request->chandle.bufpos = (char *)MALLOC( size );
  if (!request->chandle.bufpos) {
      return MPI_ERR_EXHAUSTED;
      }
  mpi_errno = MPIR_Pack( request->chandle.comm, dest_type, 
			   buf, count, datatype, 
			   request->chandle.bufpos, size, &size );
#ifdef MPID_HAS_HETERO
  if (dest_type == MPIR_MSGFORM_XDR)
      request->chandle.msgrep = MPIR_MSGREP_XDR;
/*  if ((MPID_IS_HETERO == 1) &&
      MPIR_Comm_needs_conversion(request->chandle.comm))
      request->chandle.msgrep = MPIR_MSGREP_XDR; 
 */
#endif
  MPI_PACKED->ref_count ++;
  if (!request->shandle.persistent) 
      MPIR_Type_free( &request->shandle.datatype );
  request->shandle.datatype			 = MPI_PACKED;
  request->shandle.dev_shandle.start		 = request->chandle.bufpos;
  request->shandle.dev_shandle.bytes_as_contig = size;
  return mpi_errno;
}

/* This is unused; Use MPIR_SendBufferFree */ 
int MPIR_EndPackMessage( request )
MPI_Request request;
{
FREE( request->chandle.bufpos );
return MPI_SUCCESS;
}

int MPIR_SetupUnPackMessage( buf, count, datatype, source, request )
char         *buf;
int          count, source;
MPI_Datatype datatype;
MPI_Request  request;
{
int len;

if (!datatype->committed) 
    return MPI_ERR_TYPE | MPIR_ERR_UNCOMMITTED;

/* We can't use datatype->size * count, since the actual code may require 
   padding */
(void) MPI_Pack_size( count, datatype, request->rhandle.comm, &len );

if (len > 0) {
    request->chandle.bufpos = (char *)MALLOC( len );
    if (!request->chandle.bufpos) {
	return MPI_ERR_EXHAUSTED;
	}
    }
else
    request->chandle.bufpos = 0;

request->rhandle.dev_rhandle.bytes_as_contig = len;
request->rhandle.dev_rhandle.start           = request->chandle.bufpos;
return MPI_SUCCESS;
}

/* 
   Set act_count only if changed (on input, is number of bytes) 
   On output, it should be the number of bytes in OUTPUT (placed into
   status.count field).
*/
int MPIR_UnPackMessage( buf, count, datatype, source, request, act_count )
char         *buf;
int          count, source;
MPI_Datatype datatype;
MPI_Request  request;
int          *act_count;
{
int mpi_errno = MPI_SUCCESS;
int dest_len;

/* Use generic unpack */
/* PRINTF( "Using generic unpack\n" ); */
/* Need to update act_count to bytes WRITTEN */
if (datatype->dte_type == MPIR_PACKED) {
    /* Data requires no further modification */
    memcpy( buf, request->chandle.bufpos, *act_count );
    }
else {
    mpi_errno = MPIR_Unpack( request->chandle.comm, 
				request->chandle.bufpos, *act_count, 
				count, datatype, request->rhandle.msgrep,
			        buf, act_count, &dest_len );
    *act_count = dest_len;
    }
FREE( request->chandle.bufpos );
return mpi_errno;
}
#endif
