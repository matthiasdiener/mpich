/* address.c */
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
#define mpi_address_ PMPI_ADDRESS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_address_ pmpi_address__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_address_ pmpi_address
#else
#define mpi_address_ pmpi_address_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_address_ MPI_ADDRESS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_address_ mpi_address__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_address_ mpi_address
#endif
#endif

 void mpi_address_( location, address, __ierr )
void     *location;
MPI_Aint *address;
int *__ierr;
{
*__ierr = MPI_Address(location,address);
}
