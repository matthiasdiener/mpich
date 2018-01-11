/*
 *  $Id: dmpipk.c,v 1.17 1995/05/11 17:45:39 gropp Exp $
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: dmpipk.c,v 1.17 1995/05/11 17:45:39 gropp Exp $";
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
    sizeof(int) == 4) {
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
	 sizeof(double) == 8) {
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
    sizeof(int) == 4) {
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
	 sizeof(double) == 8) {
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
	outbuf += 1;
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
      printf( "Packing packed data!\n" );
      }
  /* Use the generic pack routine */
  MPIR_Pack_size( count, datatype, MPI_COMM_WORLD, &size );
  if (size == 0) {
      request->chandle.bufpos			   = 0;
      request->shandle.dev_shandle.bytes_as_contig = 0;
      return mpi_errno;
      }
  request->chandle.bufpos = (char *)MALLOC( size );
  if (!request->chandle.bufpos) {
      return MPI_ERR_EXHAUSTED;
      }
  mpi_errno = MPIR_Pack( request->chandle.comm, buf, count, datatype, 
			   request->chandle.bufpos, size, &size );
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
*/
int MPIR_UnPackMessage( buf, count, datatype, source, request, act_count )
char         *buf;
int          count, source;
MPI_Datatype datatype;
MPI_Request  request;
int          *act_count;
{
int mpi_errno = MPI_SUCCESS;

/* Use generic unpack */
/* printf( "Using generic unpack\n" ); */
/* Need to update act_count to bytes WRITTEN */
if (mpi_errno = MPIR_Unpack( request->chandle.comm, 
			    request->chandle.bufpos, *act_count, 
			    count, datatype, request->rhandle.msgrep,
			    buf, act_count )) 
    return mpi_errno;
FREE( request->chandle.bufpos );
return MPI_SUCCESS;
}

