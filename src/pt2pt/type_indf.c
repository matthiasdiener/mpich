/* type_ind.c */
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
#define mpi_type_indexed_ PMPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ pmpi_type_indexed
#else
#define mpi_type_indexed_ pmpi_type_indexed_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_indexed_ MPI_TYPE_INDEXED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_indexed_ mpi_type_indexed
#endif
#endif

 void mpi_type_indexed_( count, blocklens, indices, old_type, newtype, __ierr )
int*count;
int        blocklens[];
int        indices[];
MPI_Datatype  old_type;
MPI_Datatype *newtype;
int *__ierr;
{
MPI_Datatype lnewtype;
*__ierr = MPI_Type_indexed(*count,blocklens,indices,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(old_type) ),&lnewtype);
*(int*)newtype = MPIR_FromPointer(lnewtype);
}
