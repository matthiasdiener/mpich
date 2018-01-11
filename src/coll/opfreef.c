/* opfree.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_op_free_ PMPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_free_ pmpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_free_ pmpi_op_free
#else
#define mpi_op_free_ pmpi_op_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_op_free_ MPI_OP_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_free_ mpi_op_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_free_ mpi_op_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_op_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_op_free_( op, __ierr )
MPI_Fint *op;
MPI_Fint *__ierr;
{
    MPI_Op l_op = MPI_Op_f2c(*op);
    *__ierr = MPI_Op_free(&l_op);
}
