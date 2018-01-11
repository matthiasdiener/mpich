/* Routines to swap integral types of different byte sizes */

#include "mpiimpl.h"

#ifdef MPID_HAS_HETERO
#ifdef HAS_XDR
#include "rpc/rpc.h"
#endif

struct MPIR_Type_convert {
  void (*char_convert)();
  void (*uchar_convert)();
  void (*short_convert)();
  void (*ushort_convert)();
  void (*int_convert)();
  void (*uint_convert)();
  void (*long_convert)();
  void (*ulong_convert)();
  void (*float_convert)();
  void (*double_convert)();
  void (*long_double_convert)();
};


/* Macro to swap 2 bytes */
#define SWAP2(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); } 

/* Byte swap an array of length n of N byte integal elements */
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

void 
MPIR_BSwap_short_inplace(b, n)
unsigned char *b;
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(short); i += sizeof(short))
    for (j = 0; j < sizeof(short)/2; j++)
      SWAP2(b[i+j], b[i+sizeof(short)-j-1]);
}

void 
MPIR_BSwap_int_inplace(b, n)
unsigned char *b;
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(int); i += sizeof(int))
    for (j = 0; j < sizeof(int)/2; j++)
      SWAP2(b[i+j], b[i+sizeof(int)-j-1]);
  
}

void 
MPIR_BSwap_long_inplace(b, n)
unsigned char *b; 
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(long); i += sizeof(long))
    for (j = 0; j < sizeof(long)/2; j++)
      SWAP2(b[i+j], b[i+sizeof(long)-j-1]);
}

void 
MPIR_BSwap_float_inplace(b, n)
unsigned char *b;
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(float); i += sizeof(float)) {
    for (j = 0; j < sizeof(float)/2; j++)
      SWAP2(b[i+j], b[i + sizeof(float) - j - 1]);
  }
}

void 
MPIR_BSwap_double_inplace(b, n)
unsigned char *b; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(double); i += sizeof(double)) {
    for (j = 0; j < sizeof(double)/2; j++)
      SWAP2(b[i+j], b[i + sizeof(double) - j - 1]);
  }
}

#ifdef HAVE_LONG_DOUBLE
void 
MPIR_BSwap_long_double_inplace(b, n)
unsigned char *b; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(long double); i += sizeof(long double)) {
    for (j = 0; j < sizeof(long double)/2; j++)
      SWAP2(b[i+j], b[i + sizeof(long double) - j - 1]);
  }
}

#else
void 
MPIR_BSwap_long_double_inplace(b, n)
unsigned char *b; 
int n;
{
#ifdef __STDC__
  MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	     "The ANSI long double type is not supported by your 'ANSI' compiler.");
#else
  MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	     "The long double type is not supported by your compiler.");
#endif
}
#endif


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

/* A good compiler should unroll the inner loops. Letting the compiler do it
   gives us portability. */

void 
MPIR_BSwap_short_copy(d, s, n)
unsigned char *d, *s;
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(short); i += sizeof(short)) {
    for (j = 0; j < sizeof(short); j++)
      d[i+j] = s[i + sizeof(short) - j - 1];
  }
}

void 
MPIR_BSwap_int_copy(d, s, n)
unsigned char *d, *s;
int n;
{
  int i, j;
  for (i = 0; i < n * sizeof(int); i += sizeof(int)) {
    for (j = 0; j < sizeof(int); j++)
      d[i+j] = s[i + sizeof(int) - j - 1];
  }
  
}

void 
MPIR_BSwap_long_copy(d, s, n)
unsigned char *d, *s; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(long); i += sizeof(long)) {
    for (j = 0; j < sizeof(long); j++)
      d[i+j] = s[i + sizeof(long) - j - 1];
  }
}

void 
MPIR_BSwap_float_copy(d, s, n)
unsigned char *d, *s; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(float); i += sizeof(float)) {
    for (j = 0; j < sizeof(float); j++)
      d[i+j] = s[i + sizeof(float) - j - 1];
  }
}

void 
MPIR_BSwap_double_copy(d, s, n)
unsigned char *d, *s; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(double); i += sizeof(double)) {
    for (j = 0; j < sizeof(double); j++)
      d[i+j] = s[i + sizeof(double) - j - 1];
  }
}

#ifdef HAVE_LONG_DOUBLE

void 
MPIR_BSwap_long_double_copy(d, s, n)
unsigned char *d, *s; 
int n;
{
  int i, j;
  for (i = 0; i < n*sizeof(long double); i += sizeof(long double)) {
    for (j = 0; j < sizeof(long double); j++)
      d[i+j] = s[i + sizeof(long double) - j - 1];
  }
}

#else
void 
MPIR_BSwap_long_double_copy(d, s, n)
unsigned char *d, *s; 
int n;
{
#ifdef __STDC__
  MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	     "The ANSI long double type is not supported by your 'ANSI' compiler.");
#else
  MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	     "The long double type is not supported by your compiler.");
#endif
}
#endif

struct MPIR_Type_convert MPIR_byte_swap_copy = {
  NULL, 
  NULL, 
  MPIR_BSwap_short_copy,
  MPIR_BSwap_short_copy,
  MPIR_BSwap_int_copy,
  MPIR_BSwap_int_copy,
  MPIR_BSwap_long_copy,
  MPIR_BSwap_long_copy,
  MPIR_BSwap_float_copy,
  MPIR_BSwap_double_copy,
  MPIR_BSwap_long_double_copy};

struct MPIR_Type_convert MPIR_byte_swap_inplace = {
  NULL, 
  NULL, 
  MPIR_BSwap_short_inplace,
  MPIR_BSwap_short_inplace,
  MPIR_BSwap_int_inplace,
  MPIR_BSwap_int_inplace,
  MPIR_BSwap_long_inplace,
  MPIR_BSwap_long_inplace,
  MPIR_BSwap_float_inplace,
  MPIR_BSwap_double_inplace,
  MPIR_BSwap_long_double_inplace};

void MPIR_Type_swap_copy(d, s, t, N) 
unsigned char *d, *s;
MPI_Datatype t;
int N;
{
  if (t == MPI_CHAR) {
    if (MPIR_byte_swap_copy.char_convert != NULL)
      (*MPIR_byte_swap_copy.char_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_UNSIGNED_CHAR) {
    if (MPIR_byte_swap_copy.uchar_convert != NULL)
      (*MPIR_byte_swap_copy.uchar_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_SHORT) {
    if (MPIR_byte_swap_copy.short_convert != NULL)
      (*MPIR_byte_swap_copy.short_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_UNSIGNED_SHORT) {
    if (MPIR_byte_swap_copy.ushort_convert != NULL)
      (*MPIR_byte_swap_copy.ushort_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_INT) {
    if (MPIR_byte_swap_copy.int_convert != NULL)
      (*MPIR_byte_swap_copy.int_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_UNSIGNED) {
    if (MPIR_byte_swap_copy.uint_convert != NULL)
      (*MPIR_byte_swap_copy.uint_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_LONG) {
    if (MPIR_byte_swap_copy.long_convert != NULL)
      (*MPIR_byte_swap_copy.long_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_UNSIGNED_LONG) {
    if (MPIR_byte_swap_copy.ulong_convert != NULL)
      (*MPIR_byte_swap_copy.ulong_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_FLOAT) {
    if (MPIR_byte_swap_copy.float_convert != NULL)
      (*MPIR_byte_swap_copy.float_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_DOUBLE) {
    if (MPIR_byte_swap_copy.double_convert != NULL)
      (*MPIR_byte_swap_copy.double_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_LONG_DOUBLE) {
    if (MPIR_byte_swap_copy.long_double_convert != NULL)
      (*MPIR_byte_swap_copy.long_double_convert)(d, s, N);
    else
      memcpy(d, s, t->size*N);
  } else if (t == MPI_BYTE || t == MPI_PACKED) {
      memcpy(d, s, t->size*N);
  } else {
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to swap unsupported type"); 
    memcpy(d, s, t->size*N);
  }
}

void MPIR_Type_swap_inplace(b, t, N)
unsigned char *b;
MPI_Datatype t;
int N;
{
  if (t ==  MPI_CHAR) {
    if (MPIR_byte_swap_inplace.char_convert != NULL)
      (*MPIR_byte_swap_inplace.char_convert)(b, N);
  } else if (t == MPI_UNSIGNED_CHAR) {
    if (MPIR_byte_swap_inplace.uchar_convert != NULL)
      (*MPIR_byte_swap_inplace.uchar_convert)(b, N);
  } else if (t == MPI_SHORT) {
    if (MPIR_byte_swap_inplace.short_convert != NULL)
      (*MPIR_byte_swap_inplace.short_convert)(b, N);
  } else if (t == MPI_UNSIGNED_SHORT) {
    if (MPIR_byte_swap_inplace.ushort_convert != NULL)
      (*MPIR_byte_swap_inplace.ushort_convert)(b, N);
  } else if (t == MPI_INT) {
    if (MPIR_byte_swap_inplace.int_convert != NULL)
      (*MPIR_byte_swap_inplace.int_convert)(b, N);
  } else if (t == MPI_UNSIGNED) {
    if (MPIR_byte_swap_inplace.uint_convert != NULL)
      (*MPIR_byte_swap_inplace.uint_convert)(b, N);
  } else if (t == MPI_LONG) {
    if (MPIR_byte_swap_inplace.long_convert != NULL)
      (*MPIR_byte_swap_inplace.long_convert)(b, N);
  } else if (t == MPI_UNSIGNED_LONG) {
    if (MPIR_byte_swap_inplace.ulong_convert != NULL)
      (*MPIR_byte_swap_inplace.ulong_convert)(b, N);
  } else if (t == MPI_FLOAT) {
    if (MPIR_byte_swap_inplace.float_convert != NULL)
      (*MPIR_byte_swap_inplace.float_convert)(b, N);
  } else if (t == MPI_DOUBLE) {
    if (MPIR_byte_swap_inplace.double_convert != NULL)
      (*MPIR_byte_swap_inplace.double_convert)(b, N);
  } else if (t == MPI_LONG_DOUBLE) {
    if (MPIR_byte_swap_inplace.long_double_convert != NULL)
      (*MPIR_byte_swap_inplace.long_double_convert)(b, N);
  } else if (t == MPI_BYTE || t == MPI_PACKED) {
    /* This should be empty */
  } else
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to swap unsupported type"); 
}

#ifdef HAS_XDR
/* XXXX- Both of these routines need a way of returning the true size
   in bytes to the device, so it knows how much to send! */

int MPIR_Mem_XDR_Encode(d, s, t, N, elsize) 
unsigned char *d, *s; /* dest, source */
xdrproc_t t; /* type */
int N, elsize; /* count and element size */
{ 
    XDR xdrstruct; 
    int rval = 1;
    u_int c;
    int total; 
    
    if (elsize <= 4) /* Must be a multiple of 4... */
      xdrmem_create(&xdrstruct, d, N * 4, XDR_ENCODE); 
    else
      xdrmem_create(&xdrstruct, d, N * elsize, XDR_ENCODE); 

    total = 0;
    while (total < N && rval) {
      rval = xdr_array(&xdrstruct, &s, &c, N, elsize, t); 
      total += c;
    }
    xdr_destroy(&xdrstruct); 
    return !rval;
}

int MPIR_Mem_XDR_Decode(d, s, t, N, size) 
unsigned char *d, *s; /* dest and source */
xdrproc_t t; /* type */
int N, size; /* count and element size */
{ 
    XDR xdrstruct; 
    int rval = 1;
    u_int c;
    int total; 
    
    xdrmem_create(&xdrstruct, d, N * size, XDR_DECODE); 
    total = 0;
    while (total < N && rval) {
      rval = xdr_array(&xdrstruct, &s, &c, N, size, t); 
      total += c;
    }
    xdr_destroy(&xdrstruct); 
    return !rval;
}


void MPIR_Type_XDR_encode(d, s, t, N) 
unsigned char *d, *s;
MPI_Datatype t;
int N;
{
  if (t ==  MPI_CHAR) {
    MPIR_Mem_XDR_Encode(d, s, xdr_char, N, sizeof(char));
  } else if (t == MPI_UNSIGNED_CHAR) {
    MPIR_Mem_XDR_Encode(d, s, xdr_u_char, N, sizeof(unsigned char));    
  } else if (t == MPI_SHORT) {
    MPIR_Mem_XDR_Encode(d, s, xdr_short, N, sizeof(short));
  } else if (t == MPI_UNSIGNED_SHORT) {
    MPIR_Mem_XDR_Encode(d, s, xdr_u_short, N, sizeof(unsigned short));
  } else if (t == MPI_INT) {
    MPIR_Mem_XDR_Encode(d, s, xdr_int, N, sizeof(int));
  } else if (t == MPI_UNSIGNED) {
    MPIR_Mem_XDR_Encode(d, s, xdr_u_int, N, sizeof(unsigned int));
  } else if (t == MPI_LONG) {
    MPIR_Mem_XDR_Encode(d, s, xdr_long, N, sizeof(long));
  } else if (t == MPI_UNSIGNED_LONG) {
    MPIR_Mem_XDR_Encode(d, s, xdr_u_long, N, sizeof(unsigned long));
  } else if (t == MPI_FLOAT) {
    MPIR_Mem_XDR_Encode(d, s, xdr_float, N, sizeof(float));
  } else if (t == MPI_DOUBLE) {
    MPIR_Mem_XDR_Encode(d, s, xdr_double, N, sizeof(double));
  } else if (t == MPI_LONG_DOUBLE) {
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	       "Unfortuantely, XDR does not support the long double type. Sorry.");
    MPIR_Mem_XDR_Encode(d, s, xdr_char, N, sizeof(char));
  } else if (t == MPI_BYTE || t == MPI_PACKED) {
    MPIR_Mem_XDR_Encode(d, s, xdr_opaque, N, sizeof(char));
  } else
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to encode unsupported type");
}

void MPIR_Type_XDR_Decode(d, s, t, N) 
unsigned char *d, *s;
MPI_Datatype t;
int N;
{
  if (t ==  MPI_CHAR) {
    MPIR_Mem_XDR_Decode(d, s, xdr_char, N, sizeof(char));
  } else if (t == MPI_UNSIGNED_CHAR) {
    MPIR_Mem_XDR_Decode(d, s, xdr_u_char, N, sizeof(unsigned char));    
  } else if (t == MPI_SHORT) {
    MPIR_Mem_XDR_Decode(d, s, xdr_short, N, sizeof(short));
  } else if (t == MPI_UNSIGNED_SHORT) {
    MPIR_Mem_XDR_Decode(d, s, xdr_u_short, N, sizeof(unsigned short));
  } else if (t == MPI_INT) {
    MPIR_Mem_XDR_Decode(d, s, xdr_int, N, sizeof(int));
  } else if (t == MPI_UNSIGNED) {
    MPIR_Mem_XDR_Decode(d, s, xdr_u_int, N, sizeof(unsigned int));
  } else if (t == MPI_LONG) {
    MPIR_Mem_XDR_Decode(d, s, xdr_long, N, sizeof(long));
  } else if (t == MPI_UNSIGNED_LONG) {
    MPIR_Mem_XDR_Decode(d, s, xdr_u_long, N, sizeof(unsigned long));
  } else if (t == MPI_FLOAT) {
    MPIR_Mem_XDR_Decode(d, s, xdr_float, N, sizeof(float));
  } else if (t == MPI_DOUBLE) {
    MPIR_Mem_XDR_Decode(d, s, xdr_double, N, sizeof(double));
  } else if (t == MPI_LONG_DOUBLE) {
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_TYPE, 
	       "Unfortuantely, XDR does not support the long double type. Sorry.");
    MPIR_Mem_XDR_Decode(d, s, xdr_char, N, sizeof(char));
  } else if (t == MPI_BYTE || t == MPI_PACKED) {
    MPIR_Mem_XDR_Decode(d, s, xdr_opaque, N, sizeof(char));
  } else
    MPIR_ERROR(MPI_COMM_NULL, MPI_ERR_INTERN, 
	       "Tried to Decode unsupported type");
}

#endif
/* 
 * This should only be called if MPIR_Dest_needs_conversion == 1 or 2
 * of MPIR_Comm_needs_conversion == 2.
 * decode is set to the outgoing representation of the data.
 */

void
MPIR_Type_convert_copy(comm, dbuf, sbuf, type, count, dest, decode)
MPI_Comm comm;
void *dbuf, *sbuf;
MPI_Datatype type;
int count, dest, *decode;
{
  /* The encoding routines will take care of the copying */
  if ((MPID_Dest_byte_order(MPIR_tid) == MPID_H_XDR) ||
      (MPID_Dest_byte_order(dest) == MPID_H_XDR) ||
      (type == MPI_PACKED)) {
#ifdef HAS_XDR
    *decode = MPIR_MSGREP_XDR;
    MPIR_Type_XDR_encode(dbuf, sbuf, type, count);
#else
    MPIR_ERROR( comm, MPI_ERR_TYPE, 
"Conversion requires XDR which is not available" );
#endif
  } else {
    *decode = MPIR_MSGREP_RECEIVER;
    MPIR_Type_swap_copy(dbuf, sbuf, type, count);
  }

}

#endif /* MPID_HAS_HETERO */
