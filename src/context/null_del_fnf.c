/* null_del_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
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

/* Prototype to suppress warnings about missing prototypes */
void mpi_null_delete_fn_ ANSI_ARGS(( MPI_Comm, int *, void *, void * ));

void mpi_null_delete_fn_ ( comm, keyval, attr, extra_state )
MPI_Comm  comm;
int       *keyval;
void      *attr;
void      *extra_state;
{
    MPIR_null_delete_fn(comm,*keyval,attr,extra_state);
}
