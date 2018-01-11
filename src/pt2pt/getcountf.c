/* getcount.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_get_count_ PMPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ pmpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ pmpi_get_count
#else
#define mpi_get_count_ pmpi_get_count_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_count_ MPI_GET_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_count_ mpi_get_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_count_ mpi_get_count
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_get_count_ ANSI_ARGS(( MPI_Status *, MPI_Datatype*, int *, int * ));

void mpi_get_count_( status, datatype, count, __ierr )
MPI_Status   *status;
MPI_Datatype *datatype;
int          *count;
int *__ierr;
{
    *__ierr = MPI_Get_count(status, *datatype, count);
}
