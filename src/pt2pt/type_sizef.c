/* type_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_size_ PMPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ pmpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ pmpi_type_size
#else
#define mpi_type_size_ pmpi_type_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_size_ MPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ mpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ mpi_type_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_size_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_size_ ( datatype, size, __ierr )
MPI_Fint *datatype;
MPI_Fint *size;
MPI_Fint *__ierr;
{
    /* MPI_Aint c_size;*/
    int c_size;
    *__ierr = MPI_Type_size(MPI_Type_f2c(*datatype), &c_size);
    /* Should check for truncation */
    *size = (MPI_Fint)c_size;
}
