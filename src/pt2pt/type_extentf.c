/* type_extent.c */
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
#define mpi_type_extent_ PMPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent
#else
#define mpi_type_extent_ pmpi_type_extent_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_extent_ MPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent
#endif
#endif

 void mpi_type_extent_( datatype, extent, __ierr )
MPI_Datatype  datatype;
MPI_Aint     *extent;
int *__ierr;
{
*__ierr = MPI_Type_extent(
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),extent);
}
