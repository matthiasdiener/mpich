/* opcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_op_create_ PMPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_create_ pmpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_create_ pmpi_op_create
#else
#define mpi_op_create_ pmpi_op_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_op_create_ MPI_OP_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_op_create_ mpi_op_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_create_ mpi_op_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
void mpi_op_create_ ANSI_ARGS(( MPI_User_function **, MPI_Fint *, MPI_Fint *, 
				MPI_Fint * ));
#else
void mpi_op_create_ ANSI_ARGS(( MPI_User_function *, MPI_Fint *, MPI_Fint *,
                                MPI_Fint * ));
#endif

void mpi_op_create_( function, commute, op, __ierr )
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
MPI_User_function **function;
#else
MPI_User_function *function;
#endif
MPI_Fint          *commute;
MPI_Fint          *op;
MPI_Fint          *__ierr;
{

    MPI_Op l_op;

#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPI_Op_create(*function,MPIR_FROM_FLOG((int)*commute),
                            &l_op);
#elif defined(_TWO_WORD_FCD)
    int tmp = *commute;
    *__ierr = MPI_Op_create(*function,MPIR_FROM_FLOG(tmp),&l_op);

#else
    *__ierr = MPI_Op_create(function,MPIR_FROM_FLOG((int)*commute),
                            &l_op);
#endif
    *op = MPI_Op_c2f(l_op);
}
