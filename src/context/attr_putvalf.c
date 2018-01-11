/* attr_putval.c */
/* THIS IS A CUSTOM WRAPPER */

#include "mpiimpl.h"

#ifndef POINTER_64_BITS
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_attr_put_ PMPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put
#else
#define mpi_attr_put_ pmpi_attr_put_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_put_ MPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_attr_put_ ANSI_ARGS(( MPI_Comm, int *, int *, int * ));

void mpi_attr_put_ ( comm, keyval, attr_value, __ierr )
MPI_Comm comm;
int *keyval;
int *attr_value;
int *__ierr;
{
    *__ierr = MPI_Attr_put(
	(MPI_Comm)MPIR_ToPointer( *((int*)comm)),
	*keyval,(void *)(MPI_Aint)(*attr_value));
}
