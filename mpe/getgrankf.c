/* getgrank.c */
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
#define mpe_comm_global_rank_ PMPE_COMM_GLOBAL_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_comm_global_rank_ pmpe_comm_global_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_comm_global_rank_ pmpe_comm_global_rank
#else
#define mpe_comm_global_rank_ pmpe_comm_global_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpe_comm_global_rank_ MPE_COMM_GLOBAL_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpe_comm_global_rank_ mpe_comm_global_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpe_comm_global_rank_ mpe_comm_global_rank
#endif
#endif

 void  mpe_comm_global_rank_( comm, rank, grank, __ierr )
MPI_Comm comm;
int*rank, *grank;
int *__ierr;
{
MPE_Comm_global_rank(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*rank,grank);
}
