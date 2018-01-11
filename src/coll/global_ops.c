/*
 *  $Id: global_ops.c,v 1.31 1995/07/25 02:45:08 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: global_ops.c,v 1.31 1995/07/25 02:45:08 gropp Exp $";
#endif /* lint */


/* 

Global Compute Operations 

 */

#include "mpiimpl.h"
#include "coll.h"

typedef struct { 
  float re;
  float im; 
} s_complex;

typedef struct { 
  double re;
  double im; 
} d_complex;


void MPIR_MAXF( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned int *a = (unsigned int *)inoutvec; 
    unsigned int *b = (unsigned int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MAX(a[i],b[i]);
    break;
  }
#endif
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_MAX" );
    break;
  }
}


void MPIR_MINF ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; 
    unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_MIN(a[i],b[i]);
    break;
  }
#endif
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_MIN" );
    break;
  }
}

#ifndef MPIR_SUM
#define MPIR_LSUM(a,b) ((a)+(b))
#endif
void MPIR_SUM ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LSUM(a[i],b[i]);
    break;
  }
#endif
  case MPIR_COMPLEX: {
    s_complex *a = (s_complex *)inoutvec; s_complex *b = (s_complex *)invec;
    for ( i=0; i<len; i++ ) {
      a[i].re = MPIR_LSUM(a[i].re ,b[i].re);
      a[i].im = MPIR_LSUM(a[i].im ,b[i].im);
    }
    break;
  }
  case MPIR_DOUBLE_COMPLEX: {
    d_complex *a = (d_complex *)inoutvec; d_complex *b = (d_complex *)invec;
    for ( i=0; i<len; i++ ) {
      a[i].re = MPIR_LSUM(a[i].re ,b[i].re);
      a[i].im = MPIR_LSUM(a[i].im ,b[i].im);
    }
    break;
  }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_SUM" );
    break;
  }
}



#define MPIR_LPROD(a,b) ((a)*(b))
void MPIR_PROD ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LPROD(a[i],b[i]);
    break;
  }
#endif
  case MPIR_COMPLEX: {
    s_complex *a = (s_complex *)inoutvec; s_complex *b = (s_complex *)invec;
    for ( i=0; i<len; i++ ) {
	  s_complex c;
	  c.re = a[i].re; c.im = a[i].im;
      a[i].re = c.re*b[i].re - c.im*b[i].im;
      a[i].im = c.im*b[i].re + c.re*b[i].im;
    }
    break;
  }
  case MPIR_DOUBLE_COMPLEX: {
    d_complex *a = (d_complex *)inoutvec; d_complex *b = (d_complex *)invec;
    for ( i=0; i<len; i++ ) {
      d_complex c;
	  c.re = a[i].re; c.im = a[i].im;
      a[i].re = c.re*b[i].re - c.im*b[i].im;
      a[i].im = c.im*b[i].re + c.re*b[i].im;
    }
    break;
  }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_PROD" );
    break;
  }
}



#ifndef MPIR_LLAND
#define MPIR_LLAND(a,b) ((a)&&(b))
#endif
void MPIR_LAND ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLAND(a[i],b[i]);
    break;
  }
#endif
  case MPIR_LOGICAL: {
      /* Assume that C int == Fortran int for now */
      MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
      MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
      for (i=0; i<len; i++) 
	  a[i] = MPIR_TO_FLOG(MPIR_LLAND(MPIR_FROM_FLOG(a[i]),
					 MPIR_FROM_FLOG(b[i])));
      break;
      }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_LAND" );
    break;
  }
}



#ifndef MPIR_LBAND
#define MPIR_LBAND(a,b) ((a)&(b))
#endif
void MPIR_BAND ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_LOGICAL: {
    MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
    MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  case MPIR_BYTE: {
    unsigned char *a = (unsigned char *)inoutvec; 
    unsigned char *b = (unsigned char *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBAND(a[i],b[i]);
    break;
  }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_BAND" );
    break;
  }
}



#ifndef MPIR_LLOR
#define MPIR_LLOR(a,b) ((a)||(b))
#endif
void MPIR_LOR ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLOR(a[i],b[i]);
    break;
  }
#endif
  case MPIR_LOGICAL: {
      MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
      MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
      for (i=0; i<len; i++) 
	  a[i] = MPIR_TO_FLOG(MPIR_LLOR(MPIR_FROM_FLOG(a[i]),
					MPIR_FROM_FLOG(b[i])));
      break;
      }

  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_LOG" );
    break;
  }
}


#ifndef MPIR_LBOR
#define MPIR_LBOR(a,b) ((a)|(b))
#endif
void MPIR_BOR ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_LOGICAL: {
    MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
    MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  case MPIR_BYTE: {
    unsigned char *a = (unsigned char *)inoutvec; 
    unsigned char *b = (unsigned char *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBOR(a[i],b[i]);
    break;
  }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_BOR" );
    break;
  }
}



#ifndef MPIR_LLXOR
#define MPIR_LLXOR(a,b) (((a)&&(!b))||((!a)&&(b)))
#endif
void MPIR_LXOR ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_FLOAT: {
    float *a = (float *)inoutvec; float *b = (float *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
  case MPIR_DOUBLE: {
    double *a = (double *)inoutvec; double *b = (double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
#if defined(HAVE_LONG_DOUBLE)
  case MPIR_LONGDOUBLE: {
    long double *a = (long double *)inoutvec; 
    long double *b = (long double *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LLXOR(a[i],b[i]);
    break;
  }
#endif
  case MPIR_LOGICAL: {
      /* Assume that C int == Fortran int for now */
      MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
      MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
      for (i=0; i<len; i++) 
	  a[i] = MPIR_TO_FLOG(MPIR_LLXOR(MPIR_FROM_FLOG(a[i]),
					 MPIR_FROM_FLOG(b[i])));
      break;
      }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_LXOR" );
    break;
  }
}



#ifndef MPIR_LBXOR
#define MPIR_LBXOR(a,b) ((a)^(b))
#endif
void MPIR_BXOR ( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  switch ((*type)->dte_type) {
  case MPIR_LOGICAL: {
    MPIR_FORT_INT_T *a = (MPIR_FORT_INT_T *)inoutvec; 
    MPIR_FORT_INT_T *b = (MPIR_FORT_INT_T *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
      }
  case MPIR_INT: {
    int *a = (int *)inoutvec; int *b = (int *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_UINT: {
    unsigned *a = (unsigned *)inoutvec; 
    unsigned *b = (unsigned *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_LONG: {
    long *a = (long *)inoutvec; long *b = (long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_ULONG: {
    unsigned long *a = (unsigned long *)inoutvec; 
    unsigned long *b = (unsigned long *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_SHORT: {
    short *a = (short *)inoutvec; short *b = (short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_USHORT: {
    unsigned short *a = (unsigned short *)inoutvec; 
    unsigned short *b = (unsigned short *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  case MPIR_BYTE: {
    unsigned char *a = (unsigned char *)inoutvec; 
    unsigned char *b = (unsigned char *)invec;
    for ( i=0; i<len; i++ )
      a[i] = MPIR_LBXOR(a[i],b[i]);
    break;
  }
  default:
    MPIR_ERROR(MPI_COMM_WORLD,MPI_ERR_OP|MPIR_ERR_NOT_DEFINED,
	       "Error in MPI_BXOR" );
    break;
  }
}



/* MINLOC and MAXLOC structures */
typedef struct {
  int  value;
  int  loc;
} MPIR_2int_loctype;

typedef struct {
  float  value;
  int    loc;
} MPIR_floatint_loctype;

typedef struct {
  long  value;
  int    loc;
} MPIR_longint_loctype;

typedef struct {
  short  value;
  int    loc;
} MPIR_shortint_loctype;

typedef struct {
  double  value;
  int     loc;
} MPIR_doubleint_loctype;

#if defined(HAVE_LONG_DOUBLE)
typedef struct {
  long double   value;
  int           loc;
} MPIR_longdoubleint_loctype;
#endif


void MPIR_MAXLOC( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  if ((*type)->dte_type == MPIR_STRUCT) {
    /* Perform the operation based on the type of the first type in */
    /* struct */
    switch ((*type)->old_types[0]->dte_type) {
    case MPIR_INT: {
      MPIR_2int_loctype *a = (MPIR_2int_loctype *)inoutvec;
      MPIR_2int_loctype *b = (MPIR_2int_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value < b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_FLOAT: {
      MPIR_floatint_loctype *a = (MPIR_floatint_loctype *)inoutvec;
      MPIR_floatint_loctype *b = (MPIR_floatint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value < b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_LONG: {
      MPIR_longint_loctype *a = (MPIR_longint_loctype *)inoutvec;
      MPIR_longint_loctype *b = (MPIR_longint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value < b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_SHORT: {
      MPIR_shortint_loctype *as = (MPIR_shortint_loctype *)inoutvec;
      MPIR_shortint_loctype *bs = (MPIR_shortint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (as[i].value == bs[i].value)
          as[i].loc = MPIR_MIN(as[i].loc,bs[i].loc);
        else if (as[i].value < bs[i].value) {
          as[i].value = bs[i].value;
          as[i].loc   = bs[i].loc;
        }
      }
      break;
    }
    case MPIR_DOUBLE: {
      MPIR_doubleint_loctype *a = (MPIR_doubleint_loctype *)inoutvec;
      MPIR_doubleint_loctype *b = (MPIR_doubleint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value < b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }

#if defined(HAVE_LONG_DOUBLE)
    case MPIR_LONGDOUBLE: {
      MPIR_longdoubleint_loctype *a = (MPIR_longdoubleint_loctype *)inoutvec;
      MPIR_longdoubleint_loctype *b = (MPIR_longdoubleint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value < b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
#endif
    default:
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MAXLOC operation not supported on type");
    }
  }

  /* Some types are defined as contiguous with 2 elements */
  else if ((*type)->dte_type == MPIR_CONTIG && ((*type)->count == 2)) {
      
    MPI_Datatype dtype = (*type)->old_type;

    /* Set the actual length */
    len = len * (*type)->count;

    /* Perform the operation */
    switch (dtype->dte_type) {
    case MPIR_INT: {
      int *a = (int *)inoutvec; int *b = (int *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] < b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_LONG: {
      long *a = (long *)inoutvec; long *b = (long *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] < b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_SHORT: {
      short *a = (short *)inoutvec; short *b = (short *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] < b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_FLOAT: {
      float *a = (float *)inoutvec; float *b = (float *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] < b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_DOUBLE: {
      double *a = (double *)inoutvec; double *b = (double *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] < b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    default: 
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MAXLOC operation not supported on type");
      break;
    }
  }
  else {
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MAXLOC operation not supported on type");
      }
}


void MPIR_MINLOC( invec, inoutvec, Len, type )
void *invec, *inoutvec;
int  *Len;
MPI_Datatype *type;
{
  int i, len = *Len;

  if ((*type)->dte_type == MPIR_STRUCT) {
    /* Perform the operation based on the type of the first type in */
    /* struct */
    switch ((*type)->old_types[0]->dte_type) {
    case MPIR_INT: {
      MPIR_2int_loctype *a = (MPIR_2int_loctype *)inoutvec;
      MPIR_2int_loctype *b = (MPIR_2int_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_FLOAT: {
      MPIR_floatint_loctype *a = (MPIR_floatint_loctype *)inoutvec;
      MPIR_floatint_loctype *b = (MPIR_floatint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_LONG: {
      MPIR_longint_loctype *a = (MPIR_longint_loctype *)inoutvec;
      MPIR_longint_loctype *b = (MPIR_longint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_SHORT: {
      MPIR_shortint_loctype *a = (MPIR_shortint_loctype *)inoutvec;
      MPIR_shortint_loctype *b = (MPIR_shortint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
    case MPIR_DOUBLE: {
      MPIR_doubleint_loctype *a = (MPIR_doubleint_loctype *)inoutvec;
      MPIR_doubleint_loctype *b = (MPIR_doubleint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }

#if defined(HAVE_LONG_DOUBLE)
    case MPIR_LONGDOUBLE: {
      MPIR_longdoubleint_loctype *a = (MPIR_longdoubleint_loctype *)inoutvec;
      MPIR_longdoubleint_loctype *b = (MPIR_longdoubleint_loctype *)invec;
      for (i=0; i<len; i++) {
        if (a[i].value == b[i].value)
          a[i].loc = MPIR_MIN(a[i].loc,b[i].loc);
        else if (a[i].value > b[i].value) {
          a[i].value = b[i].value;
          a[i].loc   = b[i].loc;
        }
      }
      break;
    }
#endif
    default:
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MINLOC operation not supported on type");
    }
  }
  else if ((*type)->dte_type == MPIR_CONTIG && ((*type)->count == 2)) {

    MPI_Datatype dtype = (*type)->old_type;

    /* Set the actual length */
    len = len * (*type)->count;

    /* Perform the operation */
    switch (dtype->dte_type) {
    case MPIR_INT: {
      int *a = (int *)inoutvec; int *b = (int *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] > b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_LONG: {
      long *a = (long *)inoutvec; long *b = (long *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] > b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_SHORT: {
      short *a = (short *)inoutvec; short *b = (short *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] > b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_FLOAT: {
      float *a = (float *)inoutvec; float *b = (float *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] > b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    case MPIR_DOUBLE: {
      double *a = (double *)inoutvec; double *b = (double *)invec;
      for ( i=0; i<len; i+=2 ) {
        if (a[i] == b[i])
          a[i+1] = MPIR_MIN(a[i+1],b[i+1]);
        else if (a[i] > b[i]) {
          a[i]   = b[i];
          a[i+1] = b[i+1];
        }
      }
      break;
    }
    default: 
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MINLOC operation not supported on type");
      break;
    }
  }
  else {
      MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_OP|MPIR_ERR_NOT_DEFINED, 
                 "MINLOC operation not supported on type");
      }
}


