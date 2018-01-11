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
void mpi_type_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_type_free_ ( datatype, __ierr )
MPI_Fint *datatype;
MPI_Fint *__ierr;
{
    MPI_Datatype ldatatype = MPI_Type_f2c(*datatype);
    *__ierr = MPI_Type_free(&ldatatype);
    *datatype = MPI_Type_c2f(ldatatype);
}
