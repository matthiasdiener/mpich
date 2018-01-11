/* type_size.c */
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
#define mpi_type_size_ PMPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ pmpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ pmpi_type_size
#else
#define mpi_type_size_ pmpi_type_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_size_ MPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ mpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ mpi_type_size
#endif
#endif

 void mpi_type_size_ ( datatype, size, __ierr )
MPI_Datatype  datatype;
MPI_Aint     *size;
int *__ierr;
{
*__ierr = MPI_Type_size(
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),size);
}
