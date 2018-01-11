/* type_lb.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_type_lb_ PMPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ pmpi_type_lb
#else
#define mpi_type_lb_ pmpi_type_lb_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_type_lb_ MPI_TYPE_LB
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_type_lb_ mpi_type_lb
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_type_lb_ ANSI_ARGS(( MPI_Datatype, int *, int * ));
void mpi_type_lb_ ( datatype, displacement, __ierr )
MPI_Datatype  datatype;
int           *displacement;
int           *__ierr;
{
    MPI_Aint   c_displacement;
    *__ierr = MPI_Type_lb(
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ), &c_displacement);
    /* Should check for truncation */
    *displacement = (int)c_displacement;
}
