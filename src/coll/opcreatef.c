/* opcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#ifndef INT_LT_POINTER
#define MPIR_ToPointer(a) a
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

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
#define mpi_reduce_ mpi_reduce__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_op_create_ mpi_op_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
#ifdef  FORTRAN_SPECIAL_FUNCTION_PTR
void mpi_op_create_ ANSI_ARGS(( MPI_User_function **, int *, MPI_Op *, 
				int * ));
#else
void mpi_op_create_ ANSI_ARGS(( MPI_User_function *, int *, MPI_Op *, int * ));
#endif

void mpi_op_create_( function, commute, op, __ierr )
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
MPI_User_function **function;
#else
MPI_User_function *function;
#endif
int               *commute;
MPI_Op            *op;
int               *__ierr;
{
    MPI_Op lop;
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPI_Op_create(*function,MPIR_FROM_FLOG(*commute),&lop);
#else
    *__ierr = MPI_Op_create(function,MPIR_FROM_FLOG(*commute),&lop);
#endif
    *(int*)op = MPIR_FromPointer( lop );
}
