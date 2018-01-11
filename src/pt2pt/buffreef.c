/* buffree.c */
/* Custom Fortran interface file */

/* Note that the calling args are different in Fortran and C */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_buffer_detach_ PMPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_detach_ pmpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_detach_ pmpi_buffer_detach
#else
#define mpi_buffer_detach_ pmpi_buffer_detach_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_buffer_detach_ MPI_BUFFER_DETACH
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_buffer_detach_ mpi_buffer_detach__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_buffer_detach_ mpi_buffer_detach
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_buffer_detach_ ANSI_ARGS(( void **, int *, int * ));

void mpi_buffer_detach_( buffer, size, __ierr )
void **buffer;
int  *size;
int *__ierr;
{
void *tmp = (void *)buffer;
*__ierr = MPI_Buffer_detach(&tmp,size);
}
