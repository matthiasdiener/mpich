/* comm_split.c */
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
#define mpi_comm_split_ PMPI_COMM_SPLIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_split_ pmpi_comm_split__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_split_ pmpi_comm_split
#else
#define mpi_comm_split_ pmpi_comm_split_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_comm_split_ MPI_COMM_SPLIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_comm_split_ mpi_comm_split__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_comm_split_ mpi_comm_split
#endif
#endif

 void mpi_comm_split_ ( comm, color, key, comm_out, __ierr )
MPI_Comm  comm;
int*color,*key;
MPI_Comm *comm_out;
int *__ierr;
{
MPI_Comm lcomm;
*__ierr = MPI_Comm_split(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*color,*key,&lcomm);
*(int*)comm_out = MPIR_FromPointer(lcomm);
}
