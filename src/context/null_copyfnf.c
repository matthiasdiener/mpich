/* null_copy_fn.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

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
void mpi_null_copy_fn_ ANSI_ARGS(( MPI_Fint, MPI_Fint *, void *, void *, 
				   void *, MPI_Fint * ));

void mpi_null_copy_fn_ ( comm, keyval, extra_state, attr_in, attr_out, flag )
MPI_Fint  comm;
MPI_Fint  *keyval;
void      *extra_state;
void      *attr_in;
void      *attr_out;
MPI_Fint  *flag;
{
    int l_flag;
    /* Note the we actually need to fix the comm argument, except that the
       null function doesn't use it */
    MPIR_null_copy_fn(MPI_Comm_f2c(comm),(int)*keyval,extra_state,attr_in,
                      attr_out,&l_flag);
    *flag = MPIR_TO_FLOG(l_flag);
}
