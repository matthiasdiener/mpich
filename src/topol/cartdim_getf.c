/* cartdim_get.c */
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
#define mpi_cartdim_get_ PMPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ pmpi_cartdim_get
#else
#define mpi_cartdim_get_ pmpi_cartdim_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cartdim_get_ MPI_CARTDIM_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cartdim_get_ mpi_cartdim_get
#endif
#endif

 void mpi_cartdim_get_ ( comm, ndims, __ierr )
MPI_Comm  comm;
int      *ndims;
int *__ierr;
{
*__ierr = MPI_Cartdim_get(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),ndims);
}
