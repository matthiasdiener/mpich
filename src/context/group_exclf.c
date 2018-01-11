/* group_excl.c */
/* Custom Fortran interface file */
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
#define mpi_group_excl_ PMPI_GROUP_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_excl_ pmpi_group_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_excl_ pmpi_group_excl
#else
#define mpi_group_excl_ pmpi_group_excl_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_excl_ MPI_GROUP_EXCL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_excl_ mpi_group_excl__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_excl_ mpi_group_excl
#endif
#endif

 void mpi_group_excl_ ( group, n, ranks, newgroup, __ierr )
MPI_Group group, *newgroup;
int*n, *ranks;
int *__ierr;
{
MPI_Group lgroup;
*__ierr = MPI_Group_excl(
	(MPI_Group)MPIR_ToPointer( *(int*)(group) ),*n,ranks,&lgroup);
*(int*)newgroup = MPIR_FromPointer(lgroup);
}
