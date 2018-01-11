/* type_commit.c */
/* Fortran interface file */
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
#define mpi_type_commit_ PMPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ pmpi_type_commit
#else
#define mpi_type_commit_ pmpi_type_commit_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_commit_ MPI_TYPE_COMMIT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_commit_ mpi_type_commit
#endif
#endif

 void mpi_type_commit_ ( datatype, __ierr )
MPI_Datatype *datatype;
int *__ierr;
{
*__ierr = MPI_Type_commit(datatype);
}
