/* dup_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#undef MPI_DUP_FN

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_dup_fn_ PMPI_DUP_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dup_fn_ pmpi_dup_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dup_fn_ pmpi_dup_fn
#else
#define mpi_dup_fn_ pmpi_dup_fn_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_dup_fn_ MPI_DUP_FN
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dup_fn_ mpi_dup_fn__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dup_fn_ mpi_dup_fn
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_dup_fn_ ANSI_ARGS(( MPI_Comm, int *, void *, void **, void **, 
			     int * ));

/* Fortran functions aren't quite the same */
void mpi_dup_fn_ ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Comm  comm;
int       *keyval;
void      *extra_state;
void     **attr_in;
void     **attr_out;
int       *flag;
{
    MPIR_dup_fn(comm,*keyval,extra_state,*attr_in,attr_out,flag);
    *flag = MPIR_TO_FLOG(*flag);
}
