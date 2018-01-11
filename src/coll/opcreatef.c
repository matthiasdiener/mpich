/* opcreate.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef INT_LT_POINTER
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
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

 void mpi_op_create_( function, commute, op, __ierr )
MPI_User_function *function;
int               *commute;
MPI_Op            *op;
int               *__ierr;
{
*__ierr = MPI_Op_create(function,MPIR_FROM_FLOG(*commute),op);
}
