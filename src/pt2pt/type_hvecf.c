/* type_hvec.c */
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
#define mpi_type_hvector_ PMPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ pmpi_type_hvector
#else
#define mpi_type_hvector_ pmpi_type_hvector_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_hvector_ MPI_TYPE_HVECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_hvector_ mpi_type_hvector
#endif
#endif

 void mpi_type_hvector_( count, blocklen, stride, old_type, newtype, __ierr )
int*count;
int*blocklen;
MPI_Aint*stride;
MPI_Datatype old_type;
MPI_Datatype *newtype;
int *__ierr;
{
MPI_Datatype lnewtype = 0;
*__ierr = MPI_Type_hvector(*count,*blocklen,*stride,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(old_type) ),&lnewtype);
*(int*)newtype = MPIR_FromPointer(lnewtype);
}
