/* mpe_seq.c */
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
#define mpe_seq_begin_ PMPE_SEQ_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_seq_begin_ pmpe_seq_begin__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_seq_begin_ pmpe_seq_begin
#else
#define mpe_seq_begin_ pmpe_seq_begin_
#endif
#else
#ifdef FORTRANCAPS
#define mpe_seq_begin_ MPE_SEQ_BEGIN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_seq_begin_ mpe_seq_begin__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_seq_begin_ mpe_seq_begin
#endif
#endif

 void  mpe_seq_begin_( comm, ng, __ierr )
MPI_Comm comm;
int*ng;
int *__ierr;
{
MPE_Seq_begin(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*ng);
}
#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpe_seq_end_ PMPE_SEQ_END
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_seq_end_ pmpe_seq_end__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_seq_end_ pmpe_seq_end
#else
#define mpe_seq_end_ pmpe_seq_end_
#endif
#else
#ifdef FORTRANCAPS
#define mpe_seq_end_ MPE_SEQ_END
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_seq_end_ mpe_seq_end__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_seq_end_ mpe_seq_end
#endif
#endif

 void  mpe_seq_end_( comm, ng, __ierr )
MPI_Comm comm;
int*ng;
int *__ierr;
{
MPE_Seq_end(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*ng);
}
