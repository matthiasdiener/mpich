/* opfree.c */
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

 void mpi_op_free_( op, __ierr )
MPI_Op  *op;
int *__ierr;
{
MPI_Op lop = (MPI_Op) MPIR_ToPointer( *(int*)op );
*__ierr = MPI_Op_free(&lop);
if (!lop) {
    MPIR_RmPointer( *(int*)op );
    }
*(int*)op = 0;
}
