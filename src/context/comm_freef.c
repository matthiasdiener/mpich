/* comm_free.c */
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
#define mpi_comm_free_ PMPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ pmpi_comm_free
#else
#define mpi_comm_free_ pmpi_comm_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_free_ MPI_COMM_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_free_ mpi_comm_free
#endif
#endif

 void mpi_comm_free_ ( comm, __ierr )
MPI_Comm *comm;
int *__ierr;
{
MPI_Comm lcomm = (MPI_Comm)MPIR_ToPointer( *(int*)comm );
*__ierr = MPI_Comm_free( &lcomm );
*(int*)comm = 0;
}

/* 
   We actually need to remove the pointer from the mapping if the ref
   count is zero... 
 */
