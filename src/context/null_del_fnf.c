/* null_del_fn.c */
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

#undef MPI_NULL_DELETE_FN

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_null_delete_fn_ PMPI_NULL_DELETE_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_delete_fn_ pmpi_null_delete_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_delete_fn_ pmpi_null_delete_fn
#else
#define mpi_null_delete_fn_ pmpi_null_delete_fn_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_null_delete_fn_ MPI_NULL_DELETE_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_null_delete_fn_ mpi_null_delete_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_null_delete_fn_ mpi_null_delete_fn
#endif
#endif

void mpi_null_delete_fn_ ( comm, keyval, attr, extra_state )
MPI_Comm  *comm;
int       *keyval;
void      *attr;
void      *extra_state;
{
MPIR_null_delete_fn(comm,keyval,attr,extra_state);
}
