/* type_vec.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_vector_ PMPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector
#else
#define mpi_type_vector_ pmpi_type_vector_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_vector_ MPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_vector_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_vector_( count, blocklen, stride, old_type, newtype, __ierr )
MPI_Fint *count;
MPI_Fint *blocklen;
MPI_Fint *stride;
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{
    MPI_Datatype l_datatype;

    *__ierr = MPI_Type_vector((int)*count, (int)*blocklen, (int)*stride,
                              MPI_Type_f2c(*old_type), 
                              &l_datatype);
    *newtype = MPI_Type_c2f(l_datatype);
}
