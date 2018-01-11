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

  switch (t->dte_type) {
      case MPIR_CHAR:
      case MPIR_UCHAR:
      case MPIR_BYTE:
      case MPIR_PACKED:
      memcpy( d, s, len );
      break;
      case MPIR_SHORT:
      case MPIR_USHORT:
      MPIR_BSwap_N_copy( d, s, sizeof(short), N );
      break;
      case MPIR_INT:
      case MPIR_UINT:
      MPIR_BSwap_N_copy( d, s, sizeof(int), N );
      break;
      case MPIR_LONG:
      case MPIR_ULONG:
      MPIR_BSwap_N_copy( d, s, sizeof(long), N );
      break;
      case MPIR_FLOAT:
      MPIR_BSwap_N_copy( d, s, sizeof(float), N );
      break;
      case MPIR_DOUBLE:
      MPIR_BSwap_N_copy( d, s, sizeof(double), N );
      break;
#ifdef HAVE_LONG_DOUBLE
      case MPIR_LONGDOUBLE:
      MPIR_BSwap_N_copy( d, s, sizeof(long double), N );
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
      MPIR_BSwap_N_inplace( b, sizeof(short), N );
      break;
      case MPIR_INT:
      case MPIR_UINT:
      MPIR_BSwap_N_inplace( b, sizeof(int), N );
      break;
      case MPIR_LONG:
      case MPIR_ULONG:
      MPIR_BSwap_N_inplace( b, sizeof(long), N );
      break;
      case MPIR_FLOAT:
      MPIR_BSwap_N_inplace( b, sizeof(float), N );
      break;
      case MPIR_DOUBLE:
      MPIR_BSwap_N_inplace( b, sizeof(double), N );
      break;
#ifdef HAVE_LONG_DOUBLE
      case MPIR_LONGDOUBLE:
      MPIR_BSwap_N_inplace( b, sizeof(long double), N );
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
}

int MPIR_Mem_XDR_Free( xdr_ctx )
XDR *xdr_ctx;
{
xdr_destroy( xdr_ctx );
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
   -1 if error
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
	if (!rval) return -1;
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

int MPIR_Mem_XDR_Decode(d, s, t, N, size, act_bytes, xdr_ctx ) 
unsigned char *d, *s;    /* dest and source */
xdrproc_t     t;         /* type */
int           N, size;   /* count and element size */
int           act_bytes; /* Number of bytes in message */
XDR           *xdr_ctx;
{ 
    int rval = 1;
    int total; 
    int i;
    
    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return -1;
	}
    total = 0;
    for (i=0; i<N; i++) {
	rval = t( xdr_ctx, d );
	/* Break if at end of buffer or error */
	if (!rval) break;
	total += size;
	d    += size;
	}
    /* ? how to return error ? */
    return total;
}

/* Special byte version */
int MPIR_Mem_XDR_ByteDecode(d, s, N, act_bytes, xdr_ctx ) 
unsigned char *d, *s;    /* dest and source */
int           N;         /* count */
int           act_bytes; /* Number of bytes in message */
XDR           *xdr_ctx;
{ 
    int rval = 1;
    int total; 
    int xdr_len;
    
    if (!xdr_ctx) {
	fprintf( stderr, "NULL XDR CONTEXT!\n" );
	return -1;
	}

    total = xdr_getpos( xdr_ctx );
    xdr_len = N;
    /* printf( "xdr_len in decode is %d with a size of %d\n", xdr_len, N ); */
    rval = xdr_opaque( xdr_ctx, (char *)d, xdr_len );
    /* printf( "after decode, xdr_len = %d\n", xdr_len ); */
    total += xdr_len;
    if (!rval) return -total;
    return total;
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
      len = MPIR_Mem_XDR_Encode(d, s, xdr_char, N, sizeof(char), xdr_ctx);
      break;
      case MPIR_UCHAR:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_char, N, sizeof(unsigned char), xdr_ctx);
      break;
      case MPIR_BYTE:
      case MPIR_PACKED:
      len = MPIR_Mem_XDR_ByteEncode(d, s, N, xdr_ctx);
      break;
      case MPIR_SHORT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_short, N, sizeof(short), xdr_ctx);
      break;
      case MPIR_USHORT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_short, N, sizeof(unsigned short), xdr_ctx);
      break;
      case MPIR_INT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_int, N, sizeof(int), xdr_ctx);
      break;
      case MPIR_UINT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_int, N, sizeof(unsigned int), xdr_ctx);
      break;
      case MPIR_LONG:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_long, N, sizeof(long), xdr_ctx);
      break;
      case MPIR_ULONG:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_u_long, N, sizeof(unsigned long), xdr_ctx);
      break;
      case MPIR_FLOAT:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_float, N, sizeof(float), xdr_ctx);
      break;
      case MPIR_DOUBLE:
      len = MPIR_Mem_XDR_Encode(d, s, xdr_double, N, sizeof(double), xdr_ctx);
      break;
      case MPIR_LONGDOUBLE:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
          "Unfortuantely, XDR does not support the long double type. Sorry.");
      len = MPIR_Mem_XDR_Encode(d, s, xdr_char, N, sizeof(char));
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
 */
int MPIR_Type_XDR_decode(s, N, t, elm_size, d, ctx )
unsigned char *d, *s;
MPI_Datatype  t;
int           N, elm_size;
void          *ctx;
{
  int act_len, act_size;
  XDR *xdr_ctx = (XDR *)ctx;
  /* printf( "Decoding %d items of type %d\n", N, t->dte_type ); */
  if (N == 0 || t->size == 0) {
      return 0;
      }

  /* The assumption in unpack is that the user does not exceed the limits of
     the input buffer */
  act_size = 12 * (N + 1);
  switch (t->dte_type) {
      case MPIR_CHAR:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_char, N, sizeof(char), 
				    act_size, xdr_ctx );
      break;
      case MPIR_UCHAR:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_u_char, N, 
				    sizeof(unsigned char), act_size, xdr_ctx  );    
      break;
      case MPIR_BYTE:
      case MPIR_PACKED:
      act_len = MPIR_Mem_XDR_ByteDecode(d, s, N, act_size, xdr_ctx  );
      break;
      case MPIR_SHORT:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_short, N, sizeof(short), act_size, xdr_ctx  );
      break;
      case MPIR_USHORT:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_u_short, N, 
				    sizeof(unsigned short), act_size, xdr_ctx  );
      break;
      case MPIR_INT:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_int, N, sizeof(int), act_size, xdr_ctx  );
      break;
      case MPIR_UINT:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_u_int, N, sizeof(unsigned int), 
				    act_size, xdr_ctx  );
      break;
      case MPIR_LONG:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_long, N, sizeof(long), act_size, xdr_ctx  );
      break;
      case MPIR_ULONG:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_u_long, N, 
				    sizeof(unsigned long), act_size, xdr_ctx  );
      break;
      case MPIR_FLOAT:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_float, N, sizeof(float), act_size, xdr_ctx  );
      break;
      case MPIR_DOUBLE:
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_double, N, sizeof(double), 
				    act_size, xdr_ctx  );
      break;
      case MPIR_LONGDOUBLE:
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
          "Unfortuantely, XDR does not support the long double type. Sorry.");
      act_len = MPIR_Mem_XDR_Decode(d, s, xdr_char, N, sizeof(char), act_size, xdr_ctx  );
      break;
      default:
	  act_len = 0;
      MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
		 "Tried to decode unsupported type");
      break;
      }
  if (act_len < 0) {
      act_len = - act_len;
      MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INTERN,
		 "Error converting data sent with XDR" );
      }
return act_len;
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
      len = MPIR_Type_XDR_encode(dbuf, sbuf, type, count, &xdr_ctx);
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
