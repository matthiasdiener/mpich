/* null_del_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
void mpi_null_delete_fn_ ANSI_ARGS(( MPI_Fint, MPI_Fint *, void *, 
                                     void * ));

void mpi_null_delete_fn_ ( comm, keyval, attr, extra_state )
MPI_Fint  comm;
MPI_Fint  *keyval;
void      *attr;
void      *extra_state;
{
    MPIR_null_delete_fn(MPI_Comm_f2c(comm), (int)*keyval, attr,
                        extra_state);
}
