/* type_count.c */
/* Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_count_ PMPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_count_ pmpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_count_ pmpi_type_count
#else
#define mpi_type_count_ pmpi_type_count_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_count_ MPI_TYPE_COUNT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_count_ mpi_type_count__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_count_ mpi_type_count
#endif
#endif

 void mpi_type_count_ ( datatype, count, __ierr )
MPI_Datatype  datatype;
int          *count;
int *__ierr;
{
*__ierr = MPI_Type_count( *(int*)(datatype) ),count);
}
