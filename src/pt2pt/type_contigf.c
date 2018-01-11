/* type_contig.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_contiguous_ PMPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ pmpi_type_contiguous
#else
#define mpi_type_contiguous_ pmpi_type_contiguous_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_contiguous_ MPI_TYPE_CONTIGUOUS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_contiguous_ mpi_type_contiguous
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_contiguous_ ANSI_ARGS(( int *, MPI_Datatype, MPI_Datatype *,
				      int * ));

void mpi_type_contiguous_( count, old_type, newtype, __ierr )
int*count;
MPI_Datatype old_type;
MPI_Datatype *newtype;
int *__ierr;
{
    MPI_Datatype lnewtype = 0;
    *__ierr = MPI_Type_contiguous(*count,
			     (MPI_Datatype)MPIR_ToPointer( *(int*)(old_type) ),
				  &lnewtype);
    *(int*)newtype = MPIR_FromPointer(lnewtype);
}
