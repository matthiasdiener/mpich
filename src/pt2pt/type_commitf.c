/* type_commit.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_commit_ PMPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit
#else
#define mpi_type_commit_ pmpi_type_commit_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_commit_ MPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_commit_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));
void mpi_type_commit_ ( datatype, __ierr )
MPI_Fint *datatype;
MPI_Fint *__ierr;
{
    MPI_Datatype ldatatype = MPI_Type_f2c(*datatype);
    *__ierr = MPI_Type_commit( &ldatatype );
    *datatype = MPI_Type_c2f(ldatatype);    
}
