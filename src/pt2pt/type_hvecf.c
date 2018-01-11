/* type_hvec.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_hvector_ PMPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector
#else
#define mpi_type_hvector_ pmpi_type_hvector_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_hvector_ MPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_hvector_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                   MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_type_hvector_( count, blocklen, stride, old_type, newtype, __ierr )
MPI_Fint *count;
MPI_Fint *blocklen;
MPI_Fint *stride;
MPI_Fint *old_type;
MPI_Fint *newtype;
MPI_Fint *__ierr;
{
    MPI_Aint     c_stride = (MPI_Aint)*stride;
    MPI_Datatype ldatatype;

    *__ierr = MPI_Type_hvector((int)*count, (int)*blocklen, c_stride,
                               MPI_Type_f2c(*old_type),
                               &ldatatype);
    *newtype = MPI_Type_c2f(ldatatype);
}
