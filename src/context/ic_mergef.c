/* ic_merge.c */
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
#define mpi_intercomm_merge_ PMPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ pmpi_intercomm_merge
#else
#define mpi_intercomm_merge_ pmpi_intercomm_merge_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_intercomm_merge_ MPI_INTERCOMM_MERGE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_intercomm_merge_ mpi_intercomm_merge
#endif
#endif

 void mpi_intercomm_merge_ ( comm, high, comm_out, __ierr )
MPI_Comm  comm;
int*high;
MPI_Comm *comm_out;
int *__ierr;
{
MPI_Comm lcomm;
*__ierr = MPI_Intercomm_merge(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*high, &lcomm );
*(int*)comm_out = MPIR_FromPointer(lcomm);
}
