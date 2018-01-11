/* group_tranks.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_translate_ranks_ PMPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks
#else
#define mpi_group_translate_ranks_ pmpi_group_translate_ranks_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_translate_ranks_ MPI_GROUP_TRANSLATE_RANKS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_translate_ranks_ mpi_group_translate_ranks
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_translate_ranks_ ANSI_ARGS(( MPI_Group, int *, int *, MPI_Group,
					    int *, int * ));
void mpi_group_translate_ranks_ ( group_a, n, ranks_a, group_b, ranks_b,
				  __ierr )
MPI_Group group_a;
int*n;
int       *ranks_a;
MPI_Group  group_b;
int       *ranks_b;
int *__ierr;
{
    *__ierr = MPI_Group_translate_ranks(
	(MPI_Group)MPIR_ToPointer( *(int*)(group_a) ),*n,ranks_a,
	(MPI_Group)MPIR_ToPointer( *(int*)(group_b) ),ranks_b);
}
