/* type_extent.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_extent_ PMPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ pmpi_type_extent
#else
#define mpi_type_extent_ pmpi_type_extent_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_extent_ MPI_TYPE_EXTENT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_extent_ mpi_type_extent
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_extent_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_extent_( datatype, extent, __ierr )
MPI_Fint *datatype;
MPI_Fint *extent;
MPI_Fint *__ierr;
{
    MPI_Aint c_extent;
    *__ierr = MPI_Type_extent(MPI_Type_f2c(*datatype), &c_extent);
    /* Really should check for truncation, ala mpi_address_ */
    *extent = (MPI_Fint)c_extent;
}
