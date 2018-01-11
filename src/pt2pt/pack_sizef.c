/* pack_size.c */
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
#define mpi_pack_size_ PMPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ pmpi_pack_size
#else
#define mpi_pack_size_ pmpi_pack_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pack_size_ MPI_PACK_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_size_ mpi_pack_size
#endif
#endif

 void mpi_pack_size_ ( incount, datatype, comm, size, __ierr )
int*incount;
MPI_Datatype  datatype;
MPI_Comm      comm;
int          *size;
int *__ierr;
{
*__ierr = MPI_Pack_size(*incount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),size);
}
