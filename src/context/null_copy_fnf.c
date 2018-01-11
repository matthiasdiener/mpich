/* null_copy_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
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

void mpi_null_copy_fn_ ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Comm  *comm;
int       *keyval;
void      *extra_state;
void      *attr_in;
void     **attr_out;
int       *flag;
{
MPIR_null_copy_fn(comm,keyval,extra_state,attr_in,attr_out,flag);
*flag = MPIR_TO_FLOG(*flag);
}
