/* bufattach.c */
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
#define mpi_buffer_attach_ PMPI_BUFFER_ATTACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_attach_ pmpi_buffer_attach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_attach_ pmpi_buffer_attach
#else
#define mpi_buffer_attach_ pmpi_buffer_attach_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_buffer_attach_ MPI_BUFFER_ATTACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_attach_ mpi_buffer_attach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_attach_ mpi_buffer_attach
#endif
#endif

 void mpi_buffer_attach_( buffer, size, __ierr )
void *buffer;
int*size;
int *__ierr;
{
*__ierr = MPI_Buffer_attach(buffer,*size);
}
