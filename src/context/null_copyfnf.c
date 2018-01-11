/* null_copy_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_ADI2
#include "mpifort.h"
#endif

#undef MPI_NULL_COPY_FN

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_null_copy_fn_ PMPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_copy_fn_ pmpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_copy_fn_ pmpi_null_copy_fn
#else
#define mpi_null_copy_fn_ pmpi_null_copy_fn_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_null_copy_fn_ MPI_NULL_COPY_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_copy_fn_ mpi_null_copy_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_copy_fn_ mpi_null_copy_fn
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_null_copy_fn_ ANSI_ARGS(( MPI_Comm, int *, void *, void *, void *,
				   int * ));

void mpi_null_copy_fn_ ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Comm  comm;
int       *keyval;
void      *extra_state;
void      *attr_in;
void      *attr_out;
int       *flag;
{
    /* Note the we actually need to fix the comm argument, except that the
       null function doesn't use it */
    MPIR_null_copy_fn(comm,*keyval,extra_state,attr_in,attr_out,flag);
    *flag = MPIR_TO_FLOG(*flag);
}
