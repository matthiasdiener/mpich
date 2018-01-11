/* type_size.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_size_ PMPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ pmpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ pmpi_type_size
#else
#define mpi_type_size_ pmpi_type_size_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_size_ MPI_TYPE_SIZE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_size_ mpi_type_size__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_size_ mpi_type_size
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_size_ ANSI_ARGS(( MPI_Datatype, int *, int * ));

void mpi_type_size_ ( datatype, size, __ierr )
MPI_Datatype  datatype;
int           *size;
int           *__ierr;
{
    /* MPI_Aint c_size;*/
    int c_size;
    *__ierr = MPI_Type_size(
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ), &c_size);
    /* Should check for truncation */
    *size = (int)c_size;
}
