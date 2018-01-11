/* Routines to swap integral types of different byte sizes */

#include "mpiimpl.h"

#ifdef MPID_HAS_HETERO
#ifdef HAS_XDR
#include "rpc/rpc.h"
#endif

/* Macro to swap 2 bytes */
#define SWAP2(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); } 

/* Byte swap an array of length n of N byte integal elements */
/* A good compiler should unroll the inner loops. Letting the compiler do it
   gives us portability.  Note that we might want to isolate the 
   cases N = 2, 4, 8 (and 16 for long double and perhaps long long)
 */
void 
MPIR_BSwap_N_inplace(b, N, n)
unsigned char *b;
int n, N;
{
  int i, j;
  for (i = 0; i < n*N; i += N)
    for (j = 0; j < N/2; j ++)
      SWAP2(b[i + j], b[i + N - j - 1]);
  
}

/* Byte swap an array of length n of N byte integal elements */

void 
MPIR_BSwap_N_copy(d, s, N, n)
unsigned char *d, *s;
int n, N;
{
  int i, j;
  for (i = 0; i < n * N; i += N)
    for (j = 0; j < N; j++)
      d[i+j] =  s[i + N - j - 1];
  
}

int MPIR_Type_swap_copy(d, s, t, N, ctx) 
unsigned char *d, *s;
MPI_Datatype  t;
int           N;
void          *ctx;
{
  int len = t->size * N;

  /* Some compilers issue warnings for using sizeof(...) as an int (!) */
  switch (t->dte_type) {
      case MPIR_CHAR:
      case MPIR_UCHAR:
      case MPIR_BYTE:
      case MPIR_PACKED:
      memcpy( d, s, len );
      break;
      case MPIR_SHORT:
      case MPIR_USHORT:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(short), N );
      break;
      case MPIR_INT:
      case MPIR_UINT:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(int), N );
      break;
      case MPIR_LONG:
      case MPIR_ULONG:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(long), N );
      break;
      case MPIR_FLOAT:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(float), N );
      break;
      case MPIR_DOUBLE:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(double), N );
      break;
#ifdef HAVE_LONG_DOUBLE
      case MPIR_LONGDOUBLE:
      MPIR_BSwap_N_copy( d, s, (int)sizeof(long double), N );
      break;
#endif
      /* Does not handle Fortran int/float/double */
      default:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to swap unsupported type"); 
      memcpy(d, s, len);
      break;
      }
return len;
}

void MPIR_Type_swap_inplace(b, t, N)
unsigned char *b;
MPI_Datatype t;
int N;
{
  switch (t->dte_type) {
      case MPIR_CHAR:
      case MPIR_UCHAR:
      case MPIR_BYTE:
      case MPIR_PACKED:
      break;
      case MPIR_SHORT:
      case MPIR_USHORT:
      MPIR_BSwap_N_inplace( b, (int)sizeof(short), N );
      break;
      case MPIR_INT:
      case MPIR_UINT:
      MPIR_BSwap_N_inplace( b, (int)sizeof(int), N );
      break;
      case MPIR_LONG:
      case MPIR_ULONG:
      MPIR_BSwap_N_inplace( b, (int)sizeof(long), N );
      break;
      case MPIR_FLOAT:
      MPIR_BSwap_N_inplace( b, (int)sizeof(float), N );
      break;
      case MPIR_DOUBLE:
      MPIR_BSwap_N_inplace( b, (int)sizeof(double), N );
      break;
#ifdef HAVE_LONG_DOUBLE
      case MPIR_LONGDOUBLE:
      MPIR_BSwap_N_inplace( b, (int)sizeof(long double), N );
      break;
#endif
      /* Does not handle Fortran int/float/double */
      default:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to convert unsupported type"); 
      break;
      }
}

int MPIR_Mem_convert_len( dest_type, datatype, count )
int dest_type, count;
MPI_Datatype datatype;
{
#ifdef HAS_XDR
if (dest_type == 2) 
    return MPIR_Mem_XDR_Len( datatype, count );
#endif
return datatype->size * count;
}

#ifdef HAS_XDR
/* XXXX- Both of these routines need a way of returning the true size
   in bytes to the device, so it knows how much to send! */

/* 
   Here are the lengths that XDR uses for encoding data.  Note that
   everything takes at least 4 bytes 
 */
#define XDR_PAD 4
#define XDR_INT_LEN 4
#define XDR_LNG_LEN 4
#define XDR_FLT_LEN 4
#define XDR_DBL_LEN 8
#define XDR_CHR_LEN 4

int MPIR_Mem_XDR_Len( datatype, count )
MPI_Datatype datatype;
int count;
{
/* XDR buffer must be a multiple of 4 in size! */
/* This is a very, very pessimistic value.  It assumes that the data 
   consists entirely of MPI_CHAR or MPI_BYTE.  Eventually, we must 
   run through the data and get the correct xdr size (or store it in the
   datatype structure for heterogeneous systems).
   4 is basically the maximum expansion size...
 */
return (4) * count * datatype->size;
}

/* initialize an xdr buffer */
int MPIR_Mem_XDR_Init( buf, size, xdr_dir, xdr_ctx )
char *buf;
int  size;
enum xdr_op xdr_dir;
XDR  *xdr_ctx;
{
xdrmem_create(xdr_ctx, buf, size, xdr_dir );
return MPI_SUCCESS;
}

int MPIR_Mem_XDR_Free( xdr_ctx )
XDR *xdr_ctx;
{
xdr_destroy( xdr_ctx );
return MPI_SUCCESS;
}

/* 
   XDR has the strange design that the number of elements (but not the
   type!) preceeds a set of values.  This makes it hard to use directly
   with MPI, which requires that only the type signature of the 
   individual elements matches.  

   Because of this, we can't use the xdr_array routines.  Instead,
   we use the individual routines.
 */

/* CHANGE: This routine used to return success or failure (the value of
   which was ignored).  It has been changed to return
   number of bytes in dest (if successful)
   MPI_ERR_INTERN if error
 */
int MPIR_Mem_XDR_Encode(d, s, t, N, elsize, xdr_ctx) 
unsigned char *d, *s;    /* dest, source */
xdrproc_t     t;         /* type */
int           N, elsize; /* count and element size */
XDR           *xdr_ctx;
{ 
    int   rval;
    int   total; 
    int   i;

    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return -1;
	}
    total = xdr_getpos( xdr_ctx );
    for (i=0; i<N; i++) {
	rval = t( xdr_ctx, s );
	if (!rval) return MPI_ERR_INTERN;
	s += elsize;
	}
    total = xdr_getpos( xdr_ctx ) - total;
    return total;
}
/* Special byte version */
int MPIR_Mem_XDR_ByteEncode(d, s, N, xdr_ctx ) 
unsigned char *d, *s;    /* dest, source */
int           N;         /* count */
XDR           *xdr_ctx;
{ 
    int   rval;
    int   total; 

    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return -1;
	}
    total = 0;
    total = xdr_getpos( xdr_ctx );
    rval = xdr_opaque( xdr_ctx, (char *)s, N ); 
    if (!rval) { 
	return -1;
	}
    total = xdr_getpos( xdr_ctx );
    return total;
}

/* 
 * We need to return two different lengths:
 * Length advanced in the destination (N * size) (destlen)
 * Length advanced in the source (N * xdrsize)   (srclen)
 * Note that we don't just use N * <appropriate size> because if act_bytes
 * is shorter than N * xdrsize, we'll stop early.
 */
int MPIR_Mem_XDR_Decode(d, s, t, N, size, act_bytes, srclen, destlen, 
			xdr_ctx ) 
unsigned char *d, *s;    /* dest and source */
xdrproc_t     t;         /* type */
int           N, size;   /* count and element size */
int           act_bytes; /* Number of bytes in message */
int           *srclen, *destlen;
XDR           *xdr_ctx;
{ 
    int rval = 1;
    int total; 
    int i;
    
    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return MPI_ERR_INTERN;
	}
    total = 0;
    *srclen  = xdr_getpos(xdr_ctx);
    for (i=0; i<N; i++) {
	rval = t( xdr_ctx, d );
	/* Break if at end of buffer or error */
	if (!rval) break;
	total += size;
	d    += size;
	}
    *destlen = total;
    *srclen  = xdr_getpos(xdr_ctx) - *srclen;
    /* if (!rval) error; */
    return MPI_SUCCESS;
}

/* Special byte version */
int MPIR_Mem_XDR_ByteDecode(d, s, N, act_bytes, srclen, destlen, xdr_ctx ) 
unsigned char *d, *s;    /* dest and source */
int           N;         /* count */
int           act_bytes; /* Number of bytes in message */
int           *srclen, *destlen;
XDR           *xdr_ctx;
{ 
    int rval = 1;
    int total; 
    int xdr_len;
    
    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return MPI_ERR_INTERN;
	}

    total = xdr_getpos( xdr_ctx );
    xdr_len = N;
    /* printf( "xdr_len in decode is %d with a size of %d\n", xdr_len, N ); */
    rval = xdr_opaque( xdr_ctx, (char *)d, xdr_len );
    /* printf( "after decode, xdr_len = %d\n", xdr_len ); */
    *srclen = xdr_getpos( xdr_ctx ) - total;
    *destlen = N;
    if (!rval) return MPI_ERR_INTERN;
    return MPI_SUCCESS;
}


int MPIR_Type_XDR_encode(d, s, t, N, ctx) 
unsigned char *d, *s;
MPI_Datatype  t;
int           N;
void          *ctx;
{
  int len; 
  XDR *xdr_ctx = (XDR *)ctx;

  if (N == 0 || t->size == 0) return 0;

  switch (t->dte_type) {
      case MPIR_CHAR:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_char, N, (int)sizeof(char), xdr_ctx);
      break;
      case MPIR_UCHAR:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_char, N, (int)sizeof(unsigned char), xdr_ctx);
      break;
      case MPIR_BYTE:
      case MPIR_PACKED:
      len = MPIR_Mem_XDR_ByteEncode(d, s, N, xdr_ctx);
      break;
      case MPIR_SHORT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_short, N, (int)sizeof(short), xdr_ctx);
      break;
      case MPIR_USHORT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_short, N, (int)sizeof(unsigned short), xdr_ctx);
      break;
      case MPIR_INT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_int, N, (int)sizeof(int), xdr_ctx);
      break;
      case MPIR_UINT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_int, N, (int)sizeof(unsigned int), xdr_ctx);
      break;
      case MPIR_LONG:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_long, N, (int)sizeof(long), xdr_ctx);
      break;
      case MPIR_ULONG:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_long, N, (int)sizeof(unsigned long), xdr_ctx);
      break;
      case MPIR_FLOAT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_float, N, (int)sizeof(float), xdr_ctx);
      break;
      case MPIR_DOUBLE:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_double, N, (int)sizeof(double), xdr_ctx);
      break;
      case MPIR_LONGDOUBLE:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
          "Unfortuantely, XDR does not support the long double type. Sorry.");
      len = MPIR_Mem_XDR_Encode(d, s, xdr_char, N, (int)sizeof(char));
      break;
      default:
	  len = 0;
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
		 "Tried to encode unsupported type");
      break;
    }
/* printf( "XDR encoded %d items of type %d into %d bytes\n", 
        N, t->dte_type, len ); */
return len;
}

/* 
   act_bytes is the number of bytes provided on input, and size of destination
   on output.

   Returns error code.
   Number of bytes read from s is set in srclen; number of bytes
   stored in d is set in destlen.
   Needs to return the other as well.
 */
int MPIR_Type_XDR_decode(s, N, t, elm_size, d, srclen, destlen, ctx )
unsigned char *d, *s;
MPI_Datatype  t;
int           N, elm_size, *srclen, *destlen;
void          *ctx;
{
  int act_size;
  int mpi_errno = MPI_SUCCESS;
  XDR *xdr_ctx = (XDR *)ctx;

  /* printf( "Decoding %d items of type %d\n", N, t->dte_type ); */
  if (N == 0 || t->size == 0) {
      return 0;
      }

  /* The assumption in unpack is that the user does not exceed the limits of
     the input buffer THIS IS A BAD ASSUMPTION! */
  act_size = 12 * (N + 1);
  switch (t->dte_type) {
      case MPIR_CHAR:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_char, N, (int)sizeof(char), 
				    act_size, srclen, destlen, xdr_ctx );
      break;
      case MPIR_UCHAR:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_u_char, N, 
				      (int)sizeof(unsigned char), act_size, 
				      srclen, destlen, xdr_ctx  );    
      break;
      case MPIR_BYTE:
      case MPIR_PACKED:
      mpi_errno = MPIR_Mem_XDR_ByteDecode(d, s, N, act_size, srclen, destlen, 
					  xdr_ctx  );
      break;
      case MPIR_SHORT:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_short, N, (int)sizeof(short), 
				      act_size, srclen, destlen, xdr_ctx  );
      break;
      case MPIR_USHORT:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_u_short, N, 
				      (int)sizeof(unsigned short), act_size, 
				      srclen, destlen, xdr_ctx  );
      break;
      case MPIR_INT:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_int, N, (int)sizeof(int), act_size,
				      srclen, destlen, xdr_ctx  );
      break;
      case MPIR_UINT:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_u_int, N, 
				      (int)sizeof(unsigned int), act_size, 
				      srclen, destlen, xdr_ctx  );
      break;
      case MPIR_LONG:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_long, N, (int)sizeof(long), 
				      act_size, srclen, destlen, xdr_ctx  );
      break;
      case MPIR_ULONG:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_u_long, N, 
				      (int)sizeof(unsigned long), act_size, 
				      srclen, destlen, xdr_ctx  );
      break;
      case MPIR_FLOAT:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_float, N, (int)sizeof(float), 
				      act_size, srclen, destlen, xdr_ctx  );
      break;
      case MPIR_DOUBLE:
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_double, N, (int)sizeof(double), 
				    act_size, srclen, destlen, xdr_ctx  );
      break;
      case MPIR_LONGDOUBLE:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
          "Unfortuantely, XDR does not support the long double type. Sorry.");
      mpi_errno = MPIR_Mem_XDR_Decode(d, s, xdr_char, N, (int)sizeof(char), 
				      act_size, srclen, destlen, xdr_ctx  );
      break;
      default:
      *srclen  = 0;
      *destlen = 0;
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
		 "Tried to decode unsupported type");
      break;
      }
  if (mpi_errno) {
      MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INTERN,
		 "Error converting data sent with XDR" );
      }
return mpi_errno;
}

#endif
/* 
 * This should only be called if MPIR_Dest_needs_conversion == 1 or 2
 * of MPIR_Comm_needs_conversion == 2.
 * decode is set to the outgoing representation of the data.
 */

int 
MPIR_Type_convert_copy(comm, dbuf, dbufsize, sbuf, type, count, dest, decode)
MPI_Comm comm;
void *dbuf, *sbuf;
MPI_Datatype type;
int dbufsize, count, dest, *decode;
{
  int len;
  /* The encoding routines will take care of the copying */
  if ((MPID_Dest_byte_order(MPIR_tid) == MPID_H_XDR) ||
      (MPID_Dest_byte_order(dest) == MPID_H_XDR) ||
      (type == MPI_PACKED)) {
#ifdef HAS_XDR
      XDR xdr_ctx;
      *decode = MPIR_MSGREP_XDR;
      MPIR_Mem_XDR_Init( dbuf, dbufsize, XDR_ENCODE, &xdr_ctx );
      len = MPIR_Type_XDR_encode(dbuf, sbuf, type, count, (void *)&xdr_ctx);
      MPIR_Mem_XDR_Free( &xdr_ctx ); 
      if (len < 0) {
	  MPIR_ERROR( comm, MPI_ERR_OTHER, 
		   "Error in converting data to network form" );
	  /* If we continue, send no data */
	  len = 0;
	  }
#else
    MPIR_ERROR( comm, MPI_ERR_TYPE, 
"Conversion requires XDR which is not available" );
#endif
  } else {
    *decode = MPIR_MSGREP_RECEIVER;
    len = MPIR_Type_swap_copy(dbuf, sbuf, type, count, (void *) 0);
  }
return len;
}

#endif /* MPID_HAS_HETERO */
