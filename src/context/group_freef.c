/* group_free.c */
/* Fortran interface file */
#include "mpiimpl.h"

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

 void mpi_group_free_ ( group, __ierr )
MPI_Group *group;
int *__ierr;
{
*__ierr = MPI_Group_free(group);
}
