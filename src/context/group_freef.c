/* group_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_group_free_ PMPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ pmpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ pmpi_group_free
#else
#define mpi_group_free_ pmpi_group_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_free_ MPI_GROUP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_free_ mpi_group_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_free_ mpi_group_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_group_free_ ANSI_ARGS(( MPI_Group *, int * ));

void mpi_group_free_ ( group, __ierr )
MPI_Group *group;
int *__ierr;
{
    MPI_Group lgroup = (MPI_Group)MPIR_ToPointer( *(int*)group );
    *__ierr = MPI_Group_free(&lgroup);
    if (!lgroup) {
	MPIR_RmPointer( *(int*)(group) );
	*(int*)group = 0;
    }
}
