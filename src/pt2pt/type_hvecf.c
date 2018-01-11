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
void mpi_type_hvector_ ANSI_ARGS(( int *, int *, int *, MPI_Datatype *,
				   MPI_Datatype *, int * ));

void mpi_type_hvector_( count, blocklen, stride, old_type, newtype, __ierr )
int          *count;
int          *blocklen;
int          *stride;
MPI_Datatype *old_type;
MPI_Datatype *newtype;
int          *__ierr;
{
    MPI_Aint     c_stride = (MPI_Aint)*stride;

    *__ierr = MPI_Type_hvector(*count,*blocklen,c_stride,*old_type,newtype);
}
