/* type_contig.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_contiguous_ PMPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous
#else
#define mpi_type_contiguous_ pmpi_type_contiguous_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_contiguous_ MPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_contiguous_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *,
				      MPI_Fint * ));

void mpi_type_contiguous_( count, old_type, newtype, __ierr )
MPI_Fint *count;
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{
    MPI_Datatype  ldatatype;

    *__ierr = MPI_Type_contiguous((int)*count, MPI_Type_f2c(*old_type),
                                  &ldatatype);
    *newtype = MPI_Type_c2f(ldatatype);
}
