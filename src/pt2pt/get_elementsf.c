/* get_elements.c */
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
#define mpi_get_elements_ PMPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ pmpi_get_elements
#else
#define mpi_get_elements_ pmpi_get_elements_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_get_elements_ MPI_GET_ELEMENTS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_get_elements_ mpi_get_elements
#endif
#endif

 void mpi_get_elements_ ( status, datatype, elements, __ierr )
MPI_Status    *status;
MPI_Datatype  datatype;
int          *elements;
int *__ierr;
{
*__ierr = MPI_Get_elements(status,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(datatype) ),elements);
}
