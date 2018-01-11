/*
 *  $Id: dmpipk.c,v 1.13 1994/12/11 16:52:34 gropp Exp $
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: dmpipk.c,v 1.13 1994/12/11 16:52:34 gropp Exp $";
#endif

#include "mpiimpl.h"
#include "mpisys.h"

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
int c, i;

c = count * count1;
/* Handle the special case of 4 or 8 byte items, with appropriate 
   alignment.  We do this to avoid the cost of a memcpy call for each
   element.
 */
if (blen == 4 && ((MPI_Aint)buf & 0x3) == 0 && (stride & 0x3) == 0 && 
    sizeof(int) == 4) {
    register int *outb = (int *)outbuf, *inb = (int *)buf;
    stride = stride >> 2;
    for (i=0; i<c; i++) {
	outb[i] = *inb;
	inb    += stride;
	}
    }
else if (blen == 8 && ((MPI_Aint)buf & 0x7) == 0 && (stride & 0x7) == 0 && 
	 sizeof(double) == 8) {
    register double *outb = (double *)outbuf, *inb = (double *)buf;
    stride = stride >> 3;
    for (i=0; i<c; i++) {
	outb[i] = *inb;
	inb    += stride;
	}
    }
else {
    for (i=0; i<c; i++) {
	memcpy( outbuf, buf, blen );
	outbuf += blen; 
	buf    += stride;
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

c = count * count1;
if (blen == 4 && ((MPI_Aint)inbuf & 0x3) == 0 && (stride & 0x3) == 0 && 
    sizeof(int) == 4) {
    register int *outb = (int *)outbuf, *inb = (int *)inbuf;
    stride = stride >> 2;
    for (i=0; i<c; i++) {
	*outb = inb[i];
	outb  += stride;
	}
    }
else if (blen == 8 && ((MPI_Aint)inbuf & 0x7) == 0 && (stride & 0x7) == 0 && 
	 sizeof(double) == 8) {
    register double *outb = (double *)outbuf, *inb = (double *)inbuf;
    stride = stride >> 3;
    for (i=0; i<c; i++) {
	*outb   = inb[i];
	outb    += stride;
	}
    }
else {
    for (i=0; i<c; i++) {
	memcpy( outbuf, inbuf, blen );
	outbuf += stride;
	inbuf  += blen;
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

/*
    These routines pack/unpack data for messages.  They KNOW how the 
    message request areas are arranged, and how the send/receive routines
    known when to use these routines.
 */
int MPIR_PackMessage( buf, count, datatype, dest, request )
char *buf;
int  count, dest;
MPI_Datatype datatype;
MPI_Request request;
{
  int size;
  int errno = MPI_SUCCESS;
  
  if (!datatype->committed) 
    return MPI_ERR_TYPE | MPIR_ERR_UNCOMMITTED;

  if (count == 0) return MPI_SUCCESS;

  if (datatype->dte_type == MPIR_HVECTOR && datatype->old_type->is_contig) {
    request->chandle.bufpos = (char *)MALLOC( datatype->size * count );
    if (!request->chandle.bufpos) {
      return MPI_ERR_EXHAUSTED;
    }
    MPIR_Pack_Hvector(request->chandle.comm, buf, count, datatype, dest, 
		      request->chandle.bufpos );
#ifdef MPID_HAS_HETERO
    if ((MPID_IS_HETERO == 1) && 
	MPIR_Comm_needs_conversion(request->chandle.comm))
      request->chandle.msgrep = MPIR_MSGREP_XDR;
#endif
    /* When we replace the datatypes, we need to be careful
       about the reference counts */
    datatype->old_type->ref_count++;
    if (!request->shandle.persistent) 
	MPIR_Type_free( &request->shandle.datatype );
    request->shandle.datatype			 = datatype->old_type;
    request->shandle.dev_shandle.start		 = request->chandle.bufpos;
    request->shandle.dev_shandle.bytes_as_contig = count * datatype->size;
    if (request->shandle.dev_shandle.bytes_as_contig > 0 && 
	request->chandle.bufpos == 0) errno = MPI_ERR_BUFFER;
    return errno;
    }
  else {
    /* Use the generic pack routine */
    MPIR_Pack_size( count, datatype, MPI_COMM_WORLD, &size );
    if (size == 0) {
	request->chandle.bufpos = 0;
	return errno;
	}
    request->chandle.bufpos = (char *)MALLOC( size );
    if (!request->chandle.bufpos) {
	return MPI_ERR_EXHAUSTED;
	}
    errno = MPIR_Pack( request->chandle.comm, buf, count, datatype, 
	      request->chandle.bufpos );
#ifdef MPID_HAS_HETERO
    if ((MPID_IS_HETERO == 1) &&
	MPIR_Comm_needs_conversion(request->chandle.comm))
      request->chandle.msgrep = MPIR_MSGREP_XDR;
#endif
    MPI_PACKED->ref_count ++;
    if (!request->shandle.persistent) 
	MPIR_Type_free( &request->shandle.datatype );
    request->shandle.datatype			 = MPI_PACKED;
    request->shandle.dev_shandle.start		 = request->chandle.bufpos;
    request->shandle.dev_shandle.bytes_as_contig = size;
    return errno;
  }
/* return MPI_ERR_INTERN; */
}

/* This is unused; Use MPIR_SendBufferFree */ 
int MPIR_EndPackMessage( request )
MPI_Request request;
{
FREE( request->chandle.bufpos );
return MPI_SUCCESS;
}

int MPIR_SetupUnPackMessage( buf, count, datatype, source, request )
char *buf;
int  count, source;
MPI_Datatype datatype;
MPI_Request request;
{
int len;
if (!datatype->committed) 
    return MPI_ERR_TYPE | MPIR_ERR_UNCOMMITTED;

/* We can't use datatype->size * count, since the actual code may require 
   padding */
(void) MPI_Pack_size( count, datatype, MPI_COMM_WORLD, &len );

request->chandle.bufpos = (char *)MALLOC( len );
if (!request->chandle.bufpos) {
    return MPI_ERR_EXHAUSTED;
    }
request->rhandle.dev_rhandle.bytes_as_contig = len;
request->rhandle.dev_rhandle.start           = request->chandle.bufpos;
return MPI_SUCCESS;
}

int MPIR_UnPackMessage( buf, count, datatype, source, request )
char *buf;
int  count, source;
MPI_Datatype datatype;
MPI_Request request;
{
int errno = MPI_SUCCESS;
if (datatype->dte_type == MPIR_HVECTOR && datatype->old_type->is_contig) {
    MPIR_UnPack_Hvector( request->chandle.bufpos, count, datatype, 
			 source, buf );
    FREE( request->chandle.bufpos );
    return errno;
    }
else {
    /* Use generic unpack */
    if (errno = MPIR_Unpack( buf, count, datatype, request->chandle.bufpos ))
	return errno;
    FREE( request->chandle.bufpos );
    return MPI_SUCCESS;
    }
/* return MPI_ERR_INTERN; */
}
