/* type_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_free_ PMPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ pmpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ pmpi_type_free
#else
#define mpi_type_free_ pmpi_type_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_free_ MPI_TYPE_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_free_ mpi_type_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_free_ mpi_type_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_free_ ANSI_ARGS(( MPI_Datatype *, int * ));

void mpi_type_free_ ( datatype, __ierr )
MPI_Datatype *datatype;
int *__ierr;
{
    *__ierr = MPI_Type_free(datatype);
}
