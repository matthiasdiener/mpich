/* type_vec.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_vector_ PMPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ pmpi_type_vector
#else
#define mpi_type_vector_ pmpi_type_vector_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_vector_ MPI_TYPE_VECTOR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_vector_ mpi_type_vector
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_vector_ ANSI_ARGS(( int *, int *, int *, MPI_Datatype,
				  MPI_Datatype *, int * ));

void mpi_type_vector_( count, blocklen, stride, old_type, newtype, __ierr )
int*count;
int*blocklen;
int*stride;
MPI_Datatype old_type;
MPI_Datatype *newtype;
int *__ierr;
{
    MPI_Datatype lnewtype = 0;
    *__ierr = MPI_Type_vector(*count,*blocklen,*stride,
			    (MPI_Datatype)MPIR_ToPointer( *(int*)(old_type) ),
			      &lnewtype);
    *(int*)newtype = MPIR_FromPointer(lnewtype);
}
