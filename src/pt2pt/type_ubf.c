/* type_ub.c */
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
#define mpi_type_ub_ PMPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ pmpi_type_ub
#else
#define mpi_type_ub_ pmpi_type_ub_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_ub_ MPI_TYPE_UB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_ub_ mpi_type_ub
#endif
#endif

 void mpi_type_ub_ ( datatype, displacement, __ierr )
MPI_Datatype  datatype;
MPI_Aint      *displacement;
int *__ierr;
{
*__ierr = MPI_Type_ub(
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),displacement);
}
