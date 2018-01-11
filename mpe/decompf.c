/* decomp.c */
/* Fortran interface file */
#include <stdio.h>
#include "mpe.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpe_decomp1d_ PMPE_DECOMP1D
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_decomp1d_ pmpe_decomp1d__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_decomp1d_ pmpe_decomp1d
#else
#define mpe_decomp1d_ pmpe_decomp1d_
#endif
#else
#ifdef FORTRANCAPS
#define mpe_decomp1d_ MPE_DECOMP1D
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_decomp1d_ mpe_decomp1d__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_decomp1d_ mpe_decomp1d
#endif
#endif

void mpe_decomp1d_ ANSI_ARGS(( int*, int *, int *, int *, int *, int * ));

 void mpe_decomp1d_( n, size, rank, s, e, __ierr )
int*n,*size,*rank, *s, *e;
int *__ierr;
{
    *__ierr = MPE_Decomp1d(*n,*size,*rank,s,e);
}
