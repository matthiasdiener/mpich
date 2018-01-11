/* type_ind.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_indexed_ PMPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed
#else
#define mpi_type_indexed_ pmpi_type_indexed_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_indexed_ MPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_indexed_ ANSI_ARGS(( int *, int [], int [], MPI_Datatype *, 
				   MPI_Datatype *, int * ));

void mpi_type_indexed_( count, blocklens, indices, old_type, newtype, __ierr )
int*count;
int        blocklens[];
int        indices[];
MPI_Datatype  *old_type;
MPI_Datatype *newtype;
int *__ierr;
{
    *__ierr = MPI_Type_indexed(*count,blocklens,indices,*old_type,newtype );
}
