/* group_incl.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_incl_ PMPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ pmpi_group_incl
#else
#define mpi_group_incl_ pmpi_group_incl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_incl_ MPI_GROUP_INCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_incl_ mpi_group_incl
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_incl_ ANSI_ARGS(( MPI_Group, int *, int *, MPI_Group *, 
				 int * ));

void mpi_group_incl_ ( group, n, ranks, group_out, __ierr )
MPI_Group group, *group_out;
int*n, *ranks;
int *__ierr;
{
    MPI_Group lgroup;
    *__ierr = MPI_Group_incl(
	(MPI_Group)MPIR_ToPointer( *(int*)(group) ),*n,ranks, &lgroup);
    *(int*)group_out = MPIR_FromPointer(lgroup);
}
