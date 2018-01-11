/* attr_getval.c */
/* THIS IS A CUSTOM WRAPPER */

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
#define mpi_attr_get_ PMPI_ATTR_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_get_ pmpi_attr_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_get_ pmpi_attr_get
#else
#define mpi_attr_get_ pmpi_attr_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_get_ MPI_ATTR_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_get_ mpi_attr_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_get_ mpi_attr_get
#endif
#endif

 void mpi_attr_get_ ( comm, keyval, attr_value, found, __ierr )
MPI_Comm comm;
int *keyval;
int *attr_value;
int *found;
int *__ierr;
{
*__ierr = MPI_Attr_get(
	(MPI_Comm)MPIR_ToPointer( *((int*)comm)),
	 *keyval,(void **)attr_value,found);
	*found = MPIR_TO_FLOG(*found);
}
