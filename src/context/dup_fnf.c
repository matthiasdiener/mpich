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
void mpi_dup_fn_ ANSI_ARGS(( MPI_Fint, MPI_Fint *, void *, void **, void **, 
			     MPI_Fint * ));

/* Fortran functions aren't quite the same */
void mpi_dup_fn_ ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Fint comm;
MPI_Fint *keyval;
void     *extra_state;
void     **attr_in;
void     **attr_out;
MPI_Fint *flag;
{
    int l_flag;

    MPIR_dup_fn(MPI_Comm_f2c(comm), (int)*keyval, extra_state, *attr_in,
                attr_out, &l_flag);
    *flag = MPIR_TO_FLOG(l_flag);
}
