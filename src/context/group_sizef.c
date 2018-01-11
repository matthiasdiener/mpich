/* group_size.c */
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
#define mpi_group_size_ PMPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ pmpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ pmpi_group_size
#else
#define mpi_group_size_ pmpi_group_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_group_size_ MPI_GROUP_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_group_size_ mpi_group_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_group_size_ mpi_group_size
#endif
#endif

 void mpi_group_size_ ( group, size, __ierr )
MPI_Group group;
int *size;
int *__ierr;
{
*__ierr = MPI_Group_size(
	(MPI_Group)MPIR_ToPointer( *(int*)(group) ),size);
}
