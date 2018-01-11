/* attr_delval.c */
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
#define mpi_attr_delete_ PMPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ pmpi_attr_delete
#else
#define mpi_attr_delete_ pmpi_attr_delete_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_delete_ MPI_ATTR_DELETE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_delete_ mpi_attr_delete
#endif
#endif

 void mpi_attr_delete_ ( comm, keyval, __ierr )
MPI_Comm comm;
int*keyval;
int *__ierr;
{
*__ierr = MPI_Attr_delete(
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),*keyval);
}
