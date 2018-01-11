/*
 *  $Id: pkutil.c,v 1.18 1997/02/20 20:43:30 lusk Exp $
 *
 *  (C) 1995 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

/* 
   This file contains the top-level routines for packing and unpacking general
   datatypes.  

   This is implemented by a routine that walks through the datatypes, 
   calling a pack/unpack routine for simple, contiguous datatypes (contiguous 
   in this sense means all bytes are in use between lb and ub; there are no
   holes).  This pack/unpack routine is passed as an argument; if the 
   argument is null, then memcpy is used.

   In addition, the pack/unpack routine returns the number of bytes 
   produced/consumed; this allows easier use of methods that change the size 
   of the data (e.g., XDR).  Also note that a routine that prints can be used;
   this makes it easier to provide debugging routines for the pack/unpack 
   codes. 

   The form of the contiguous pack routine is

   int packcontig( dest, src, datatype, num, packctx )
   num items of MPI type datatype are packed into dest from src, with the 
   number of bytes added to dest being returned.
   packctx is an anonymous pointer that can be used to hold any other state.

   The contiguous unpack routine is

   int unpackcontig( src, count, type, typesize, dest, 
                     srclen, srcreadlen, destlen,  unpackctx )
   count items of MPI type datatype are unpacked from src into dest, with the
   number of bytes consumed from dest being set in destlen.  
   inbytes is the 
   number of bytes available in dest, and is used for detecting buffer 
   overruns.  
   srclen is the number of bytes available in src.
   srcreadlen is the number of bytes consumed from src.  The return
   value is the MPI error code
 */

#include <stdio.h>
#define MPID_INCLUDE_STDIO

#include "mpiimpl.h"
#ifdef MPI_ADI2
#ifdef malloc
#undef malloc
#undef free
#undef calloc
#endif
#endif

#ifdef MPI_ADI2
#include "mpidmpi.h"
#define MPIR_Type_XDR_encode MPID_Type_XDR_encode
#define MPIR_Type_XDR_decode MPID_Type_XDR_decode
#define MPIR_Mem_XDR_Init    MPID_Mem_XDR_Init
#define MPIR_Mem_XDR_Free    MPID_Mem_XDR_Free

int MPIR_Type_XDR_encode ANSI_ARGS(( unsigned char *, unsigned char *, 
				     struct MPIR_DATATYPE *, int, void * ));
int MPIR_Type_XDR_decode ANSI_ARGS(( unsigned char *, int, 
				     struct MPIR_DATATYPE*, int, 
			  unsigned char *, int, int *, int *, void * ));

#endif
#ifdef MPID_HAS_HETERO
#ifdef HAS_XDR
#include "rpc/rpc.h"
int MPIR_Mem_XDR_Init ANSI_ARGS((char *, int, enum xdr_op, XDR * ));
int MPIR_Mem_XDR_Free ANSI_ARGS((XDR *));
#endif
int MPIR_Type_swap_copy ANSI_ARGS((unsigned char *, unsigned char *,
				   struct MPIR_DATATYPE *, int, void *));
#endif

#ifdef MPI_ADI2
#define MPIR_MSGFORM_XDR MPID_MSG_XDR
#define MPIR_MSGFORM_OK  MPID_MSG_OK
/* Need to determine swap form? */
#define MPIR_MSGFORM_SWAP -1

#define MPIR_MSGREP_SENDER MPID_MSGREP_SENDER
#define MPIR_MSGREP_XDR    MPID_MSGREP_XDR
#define MPIR_MSGREP_RECEIVER MPID_MSGREP_RECEIVER

#define MPIR_Type_swap_copy MPID_Type_swap_copy
#define MPIR_Mem_convert_len MPID_Mem_convert_len
int MPIR_Type_swap_copy ANSI_ARGS((unsigned char *, unsigned char *,
				   struct MPIR_DATATYPE *, int, void *));
#endif

/*
   This code assumes that we can use char * pointers (previous code 
   incremented pointers by considering them integers, which is even 
   less portable).  Systems that, for example, use word-oriented pointers
   may want to use different code.

   This code is used in dmpi/dmpipk.c to pack data for a device that
   only supports contiguous messages.

   In addition, XDR has an initial header that this does not handle.

   The same would be true for code that truncated 8 byte longs to 4 bytes.
 */

#ifndef MPI_ADI2
/* 
    A point-to-point version can also use the general pack2 by also detecting

	packcontig = MPIR_Type_swap_copy;

    dest_type is REPRESENTATION type for the destination (i.e., XDR, 
    Byte swapped) 
 */
int MPIR_Pack( comm, dest_type, buf, count, type, dest, destsize, totlen )
MPI_Comm comm;
void *buf;
int dest_type, count, destsize;
MPI_Datatype type;
void *dest;
int  *totlen;
{
int (*packcontig) ANSI_ARGS((unsigned char *, unsigned char *, 
			     struct MPIR_DATATYPE *, 
			     int, void * )) = 0;
void *packctx = 0;
int err;
int outlen;
#ifdef HAS_XDR
XDR xdr_ctx;
#endif

/* MPIR_GET_REAL_DATATYPE(type) */
#ifdef MPID_HAS_HETERO
    switch (dest_type) {
	case MPIR_MSGFORM_XDR:
#if HAS_XDR
	/* Need to initialize xdr buffer */
	MPIR_Mem_XDR_Init( dest, destsize, XDR_ENCODE, &xdr_ctx );
	packctx	   = (void *)&xdr_ctx;
	packcontig = MPIR_Type_XDR_encode;
	/* Could set a free function ... */
#else
	return MPIR_ERROR( comm, MPI_ERR_TYPE, 
		      "Conversion requires XDR which is not available" );
#endif
	break;

	case MPIR_MSGFORM_SWAP:
	packcontig = MPIR_Type_swap_copy;
	break;

	case MPIR_MSGFORM_OK:
	/* Use default routines */
	break;
	}
#endif
outlen = 0;
*totlen = 0;
err = MPIR_Pack2( buf, count, maxcount, type, packcontig, packctx, dest, 
		  &outlen, totlen );
#if HAS_XDR
if (packcontig == MPIR_Type_XDR_encode) 
    MPIR_Mem_XDR_Free( &xdr_ctx ); 
#endif
return err;
}
#endif

/* Unpack may need to know more about whether the buffer is packed in some
   particular format.
   srcsize is size of src in bytes on input.
   act_len is amount of data consumed (used to increment the "position"
   value in MPI_Unpack.
   Normally unchanged; if on input it does not specify enough data 
   for (count,type), then it may less than count*(size)
   
   dest_len is the amount of data written to dest; this is needed to 
   keep things like status.count updated.
   
*/
int MPIR_Unpack ( comm_ptr, src, srcsize, count, dtype_ptr, msgrep, 
		  dest, act_len, dest_len )
struct MPIR_COMMUNICATOR *comm_ptr;
void         *src, *dest;
int          srcsize, count;
#ifdef MPI_ADI2
MPID_Msgrep_t msgrep;
#else
int          msgrep;
#endif
struct MPIR_DATATYPE *dtype_ptr;
int          *act_len, *dest_len;
{
int (*unpackcontig) ANSI_ARGS((unsigned char *, int, struct MPIR_DATATYPE*, 
			       int, unsigned char *, int, int *, int *, 
			       void *)) = 0;
void *unpackctx = 0;
int err, used_len;
#ifdef HAS_XDR
XDR xdr_ctx;
#endif

#ifdef MPID_HAS_HETERO
    if (msgrep == MPIR_MSGREP_XDR
/* || (MPID_IS_HETERO == 1 &&
	MPIR_Comm_needs_conversion(comm))*/
) {
#if HAS_XDR
	/* MPIR_Mem_XDR_Init( src, ?, XDR_DECODE, &xdr_ctx ); */
	MPIR_Mem_XDR_Init( src, srcsize, XDR_DECODE, &xdr_ctx );
 	unpackctx    = (void *)&xdr_ctx;
	unpackcontig = MPIR_Type_XDR_decode;
#else
    return MPIR_ERROR( comm_ptr, MPI_ERR_TYPE, 
		       "Conversion requires XDR which is not available" );
#endif
    }
#endif
*dest_len = 0;
used_len  = 0;
err = MPIR_Unpack2( (char *)src, count, dtype_ptr, unpackcontig, unpackctx, 
		    (char *)dest, srcsize, dest_len, &used_len );
*act_len = used_len;
#ifdef HAS_XDR
if (unpackcontig == MPIR_Type_XDR_decode) 
    MPIR_Mem_XDR_Free( &xdr_ctx ); 
#endif
return err;
}

#ifdef FOO
#ifdef MPID_HAS_HETERO
int 
MPIR_Type_convert_copy2(comm, dbuf, sbuf, type, count, dest, decode)
MPI_Comm     comm;
char         *dbuf, *sbuf;
MPI_Datatype type;
int          count, dest, *decode;
{
  int len, outlen, totlen;
  int (*packcontig)() = 0;
  void *packctx = 0;

  /* The encoding routines will take care of the copying */
  if ((MPID_Dest_byte_order(MPIR_tid) == MPID_H_XDR) ||
      (MPID_Dest_byte_order(dest) == MPID_H_XDR) ||
      (type == MPI_PACKED)) {
#ifdef HAS_XDR
    *decode = MPIR_MSGREP_XDR;
    packcontig = MPIR_Type_XDR_encode;
#else
    MPIR_ERROR( comm, MPI_ERR_TYPE, 
"Conversion requires XDR which is not available" );
#endif
  } else {
    *decode = MPIR_MSGREP_RECEIVER;
    packcontig = MPIR_Type_swap_copy;
  }
return MPIR_Pack2( sbuf, count, maxcount, type, packcontig, packctx, dbuf, &outlen, &totlen );
}
#endif
#endif

#ifndef MPI_ADI2
/*+
    MPIR_Pack_size - Returns the exact amount of space needed to 
	                 pack a message (well, almost; returns
			 the space the will be enough)

			 dest_type gives the format to use.  0 is unconverted
+*/
int MPIR_Pack_size ( incount, type, comm, dest_type, size )
int           incount, dest_type;
MPI_Datatype  type;
MPI_Comm      comm;
int          *size;
{
struct MPIR_DATATYPE *dtype_ptr;

/*MPIR_GET_REAL_DATATYPE(type) */
  /* 
     Figure out size needed to pack type.  This uses the same logic as
     MPIR_Pack, except that it can handle, through dest_type, 
     specific formats for specific destinations. 
   */
if(dest_type > MPIR_MSGREP_SENDER) 
    /* Dest type is SWAP (a MSGFORM, not MSGREP).  Change to receiver rep */
    dest_type = MPIR_MSGREP_RECEIVER;
#ifdef MPID_HAS_HETERO
  (*size) = MPIR_Mem_convert_len( dest_type, type, incount );
#else
dtype_ptr   = MPIR_GET_DTYPE_PTR(type);

  (*size) = dtype_ptr->size * incount;
#endif
return MPI_SUCCESS;
}
#endif

/* 
   Input Parameters:
   buf - Source of data
   count - number of items to pack
   maxcount - size of DESTINATION buffer in BYTES
   type - MPI datatype of item
   packcontig - function to perform packing of contiguous data.  If null,
   use memcpy.
   packctx - context for packcontig
   
   Output parameters:
   dest - Destination buffer
   outlen - number of bytes used in dest (for each call)
   totlen - total number of bytes used in dest (cumulative)
   
   Returns:
   MPI error code

   Previous versions of this tried to maintain the source padding in the
   destination.  This is incompatible with XDR encoding, and isn't really 
   necessary.
 */
int MPIR_Pack2( buf, count, maxcount, type, packcontig, packctx, dest, 
		outlen, totlen )
char         *buf;
int          count, maxcount;
struct MPIR_DATATYPE *type;
char         *dest;
int          (*packcontig) ANSI_ARGS((unsigned char *, unsigned char *, 
				      struct MPIR_DATATYPE*, int, void *));
void         *packctx;
int          *outlen, *totlen;
{
    int i,j;
    int mpi_errno = MPI_SUCCESS;
    char *tmp_buf;
    int  len;
    int  myoutlen = 0;

  /* Pack contiguous data */
    if (type->is_contig) {
	len = type->size * count;
	if (buf == 0 && len > 0)
	    return MPI_ERR_BUFFER;
	if (!packcontig) {
	    if (len > maxcount) 
		return MPI_ERR_BUFFER;
	    memcpy( dest, buf, len );
	    *outlen = len;
	    *totlen += len;
	    return MPI_SUCCESS;
	}
	else if (type->basic) {
	    len = (*packcontig)( (unsigned char *)dest, (unsigned char *)buf, 
				 type, count, packctx );
	    if (len < 0) {
		/* This may happen when an XDR routine fails */
		MPIR_ERROR( MPIR_COMM_WORLD, MPI_ERR_OTHER, 
			    "Error in converting data to network form" );
		/* If we continue, send no data */
		len = 0;
	    }
	    *outlen = len;
	    *totlen += len;
	    return MPI_SUCCESS;
	}
    }

  /* For each of the count arguments, pack data */
  switch (type->dte_type) {

  /* Contiguous types */
  case MPIR_CONTIG:
	mpi_errno = 
	    MPIR_Pack2( buf, count * type->count, maxcount, type->old_type, 
		        packcontig, packctx, dest, outlen, totlen );
	break;

  /* Vector types */
  case MPIR_VECTOR:
  case MPIR_HVECTOR:
	/* We want to be able to use the hvec copy here.... */
#ifdef MPID_HAS_HETERO
	if (!MPID_IS_HETERO) 
#endif
	    {
	    if (type->old_type->is_contig && !packcontig) {
		MPIR_Pack_Hvector( MPIR_COMM_WORLD, buf, count, type, 
				   -1, dest );
	        *outlen = count * type->size;
	        *totlen += *outlen;
	        return MPI_SUCCESS;
		}
	    }
	tmp_buf = buf;
	for (i=0; i<count; i++) {
	  buf = tmp_buf;
	  for (j=0; j<type->count; j++) {
	      if ((mpi_errno = MPIR_Pack2 ( buf, type->blocklen, maxcount,
					  type->old_type, packcontig, packctx,
					  dest, outlen, totlen ))) break;
	      buf      += (type->stride);
	      dest     += *outlen;
	      maxcount -= *outlen;
	      myoutlen += *outlen;
	      }
	  tmp_buf += type->extent;
	  }
	*outlen = myoutlen;
	break;

  /* Indexed types */
  case MPIR_INDEXED:
  case MPIR_HINDEXED:
	for (i=0; i<count; i++) {
	    for (j=0;j<type->count; j++) {
		tmp_buf  = buf + type->indices[j];
		if ((mpi_errno = MPIR_Pack2 (tmp_buf, type->blocklens[j], 
					     maxcount, 
					     type->old_type, 
					     packcontig, packctx, 
					     dest, outlen, totlen))) break;
		dest	 += *outlen;
		maxcount -= *outlen;
		myoutlen += *outlen;
	  }
	  buf += type->extent;
	}
	*outlen = myoutlen;
	break;

  /* Struct type */
  case MPIR_STRUCT:
	for (i=0; i<count; i++) {
	  for (j=0;j<type->count; j++) {
		tmp_buf  = buf + type->indices[j];
		if ((mpi_errno = MPIR_Pack2(tmp_buf,type->blocklens[j],
					    maxcount,
					    type->old_types[j], 
					    packcontig, packctx, 
					    dest, outlen, totlen))) break;
		dest	 += *outlen;
		maxcount -= *outlen;
		myoutlen += *outlen;
	  }
	  buf  += type->extent;
	}
	*outlen = myoutlen;
	break;

  default:
	mpi_errno = MPI_ERR_TYPE;
	break;
  }

  /* Everything fell through, must have been successful */
  return mpi_errno;
}

/*
   This code assumes that we can use char * pointers (previous code 
   incremented pointers by considering them integers, which is even 
   less portable).  Systems that, for example, use word-oriented pointers
   may want to use different code.

   This code is used in dmpi/dmpipk.c to unpack data from a device that
   only supports contiguous messages.

   Input Parameters:
   src - source buffer
   srclen - size of input buffer
   count,type - number of items of type to be read
   unpackcontig,unpackctx - routine to move data from src to dest.  If null,
        memcpy is used 
   dest - destination buffer
   
   Output Parameters:
   dest_len - Number of bytes written to dest.  
   used_len - Number of bytes consumed in src
 */
int MPIR_Unpack2 ( src, count, type, unpackcontig, unpackctx, dest, srclen, 
		   dest_len, used_len )
char         *src, *dest;
int          count, srclen;
int          (*unpackcontig) ANSI_ARGS((unsigned char *, int, 
					struct MPIR_DATATYPE*, 
					int, unsigned char *, int, int *, 
					int *, void *));
void         *unpackctx;
struct MPIR_DATATYPE *type;
int          *dest_len;
int          *used_len;
{
  int i,j;
  int mpi_errno = MPI_SUCCESS;
  char *tmp_buf;
  int  len, srcreadlen, destlen;

#ifdef FOO
  if (MPIR_TEST_IS_DATATYPE(MPI_COMM_WORLD,type))
	return MPIR_ERROR(MPIR_COMM_WORLD, mpi_errno, 
			  "Internal Error in MPIR_UNPACK");
#endif

  /* Unpack contiguous data */
  if (type->is_contig) {
      if (!unpackcontig) {
	  len	     = type->size * count;
	  /* If the length is greater than supplied, process only what is
	     available */
	  if (len > srclen) len = srclen;
	  *used_len  = len;
	  *dest_len += len;
	  if (len == 0) {
	      return mpi_errno;
	      }
	  if (dest == 0) 
	      return MPI_ERR_BUFFER;
	  memcpy ( dest, src, len );
	  return MPI_SUCCESS;
	  }
      else if (type->basic) {
	  /* This requires a basic type so that the size is correct */
	  /* Need to check the element size argument... */
	  mpi_errno = (*unpackcontig)( (unsigned char *)src, count, type, 
				       type->size, 
				       (unsigned char *)dest, srclen, 
				       &srcreadlen, &destlen, unpackctx );
	  *dest_len += destlen;
	  *used_len  = srcreadlen;
	  return mpi_errno;
	  }
      }

  /* For each of the count arguments, unpack data */
  switch (type->dte_type) {

  /* Contiguous types */
  case MPIR_CONTIG:
	mpi_errno = MPIR_Unpack2 ( src, count * type->count, type->old_type, 
				   unpackcontig, unpackctx, dest, srclen, 
				   dest_len, used_len );
	break;

  /* Vector types */
  case MPIR_VECTOR:
  case MPIR_HVECTOR:
#ifdef MPID_HAS_HETERO
	if (!MPID_IS_HETERO) 
#endif
	    {
	    if (type->old_type->is_contig && !unpackcontig) {
		MPIR_UnPack_Hvector( src, count, type, -1, dest );
		len	   = count * type->size;
		*dest_len += len;
		*used_len = len;
	        return MPI_SUCCESS;
		}
	    }
	tmp_buf = dest;
	for (i=0; i<count; i++) {
	  dest = tmp_buf;
	  for (j=0; j<type->count; j++) {
	      len = 0;
	      if ((mpi_errno = MPIR_Unpack2 (src, type->blocklen, 
					    type->old_type, 
					    unpackcontig, unpackctx, 
					    dest, srclen, dest_len, &len )))
		  return mpi_errno;
	      dest	 += (type->stride);
	      src	 += len;
	      srclen     -= len;
	      *used_len  += len;
	      }
	  tmp_buf += type->extent;
	  }
	break;
	
  /* Indexed types */
  case MPIR_INDEXED:
  case MPIR_HINDEXED:
	for (i=0; i<count; i++) {
	    for (j=0;j<type->count; j++) {
		tmp_buf  = dest + type->indices[j];
		len      = 0;
		if ((mpi_errno = MPIR_Unpack2 (src, type->blocklens[j], 
					      type->old_type, 
					      unpackcontig, unpackctx,
					      tmp_buf, srclen, dest_len, 
					      &len )) )
		    return mpi_errno;
		src	  += len;
		srclen    -= len;
		*used_len += len;
		}
	    dest += type->extent;
	    }
	break;

  /* Struct type */
  case MPIR_STRUCT:
	for (i=0; i<count; i++) {
	    /* PRINTF( ".struct.[%d]\n", i ); */
	    for (j=0;j<type->count; j++) {
		tmp_buf  = dest + type->indices[j];
		len      = 0;
		if ((mpi_errno = MPIR_Unpack2(src,type->blocklens[j],
					     type->old_types[j], 
					     unpackcontig, unpackctx, 
					     tmp_buf, srclen, dest_len,
					     &len ))) {
		    /* PRINTF( ".!error return %d\n", mpi_errno ); */
		    return mpi_errno;
		    }
		src	  += len;
		srclen    -= len;
		*used_len += len;
		}
	    dest += type->extent;
	    }
	break;

  default:
	mpi_errno = MPI_ERR_TYPE;
	break;
  }

  /* Everything fell through, must have been successful */
  return mpi_errno;
}

/* 
   This is a special unpack function that gives us the number of
   basic elements in a datatype.  If we have received only part of
   a datatype, this gives the correct value.
 */
int MPIR_Elementcnt( src, num, datatype, inbytes, dest, srclen, srcreadlen, 
		     destlen, ctx )
unsigned char *dest, *src;
struct MPIR_DATATYPE  *datatype;
int           num, inbytes, srclen, *srcreadlen, *destlen;
void          *ctx;
{
int len = datatype->size * num;
int *totelm = (int *)ctx;

/* PRINTF( "Counting datatype of size %d (srclen = %d)\n", 
	datatype->size, srclen ); */
if (*totelm >= 0) {
    /* Once we decide on undefined, don't change it */
    if (len > srclen) {
	if (datatype->size > 0) {
	    num = srclen / datatype->size;
	    len = datatype->size * num;
	    *totelm	 = *totelm + num;
	    }
	else {
	    *totelm = MPI_UNDEFINED;
	    }
	}
    else {
	*totelm	 = *totelm + num;
	}
    }
*srcreadlen = len;
*destlen    = len;
return MPI_SUCCESS;
}

/*
   These routines allow a single thread to writeout the memory move operations
   that will be performed with a given MPI datatype.

   If one of the offests is 0, then we use a fake value.
 */
static FILE *datatype_fp = 0;
static char *i_offset, *o_offset;
static char i_dummy;

/* The interface makes these unsigned chars */
int MPIR_Printcontig( dest, src, datatype, num, ctx )
unsigned char         *dest, *src;
struct MPIR_DATATYPE *datatype;
int          num;
void         *ctx;
{
    int len = datatype->size * num;

/* gcc doesn't like subtracting from a POINTER to unsigned(!) */
    FPRINTF( datatype_fp, "Copy %x <- %x for %d bytes\n", 
	     ((char *)dest)-o_offset, ((char *)src)-i_offset, len );
    return len;
}

int MPIR_Printcontig2( src, num, datatype, inbytes, dest, ctx )
char         *dest, *src;
struct MPIR_DATATYPE *datatype;
int          num, inbytes;
void         *ctx;
{
    int len = datatype->size * num;

    FPRINTF( datatype_fp, "Copy %x <- %x for %d bytes\n", 
	     dest-o_offset, src-i_offset, len );
    return len;
}

int MPIR_Printcontig2a( src, num, datatype, inbytes, dest, srclen, 
		       srcreadlen, destlen, ctx )
unsigned char *dest, *src;
struct MPIR_DATATYPE *datatype;
int          num, inbytes, srclen, *srcreadlen, *destlen;
void         *ctx;
{
    int len = datatype->size * num;
    
    FPRINTF( datatype_fp, "Copy %x <- %x for %d bytes\n", 
	     (char *)dest-o_offset, (char *)src-i_offset, len );
    *srcreadlen = len;
    *destlen    = len;
    return MPI_SUCCESS;
}

int MPIR_PrintDatatypePack( fp, count, type, in_offset, out_offset )
FILE         *fp;
int          count;
struct MPIR_DATATYPE *type;
long         in_offset, out_offset;
{
int outlen, totlen;
char *src, *dest;

datatype_fp = fp ? fp : stdout;
i_offset = (char *)0;
o_offset = (char *)0;
src      = (char *)in_offset;
dest     = (char *)out_offset;
if (!in_offset) {
    i_offset = &i_dummy;
    src      = i_offset;
    }
if (!out_offset) {
    o_offset = &i_dummy;
    dest     = o_offset;
    }
MPIR_Pack2( src, count, 100000000, type, MPIR_Printcontig, (void *)0, dest, 
	    &outlen, &totlen );
return MPI_SUCCESS;
}

int MPIR_PrintDatatypeUnpack( fp, count, type, in_offset, out_offset )
FILE         *fp;
int          count;
MPI_Datatype type;
long         in_offset, out_offset;
{
    struct MPIR_DATATYPE *dtype_ptr;
    int      srclen, destlen, used_len;
    char     *src, *dest;
    int      size;

    datatype_fp = fp ? fp : stdout;
    i_offset = (char *)0;
    o_offset = (char *)0;
    src      = (char *)in_offset;
    dest     = (char *)out_offset;
    MPI_Type_size( type, &size );
    srclen   = count * size;
    if (!in_offset) {
	i_offset = &i_dummy;
	src      = i_offset;
    }
    if (!out_offset) {
	o_offset = &i_dummy;
	dest     = o_offset;
    }
    dtype_ptr   = MPIR_GET_DTYPE_PTR(type);
    MPIR_Unpack2( src, count, dtype_ptr, MPIR_Printcontig2a, (void *)0, dest, 
		  srclen, &destlen, &used_len );
    return MPI_SUCCESS;
}
