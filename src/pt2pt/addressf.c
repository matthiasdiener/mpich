/* address.c */
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

/*
   This code is a little subtle.  By making all addresses relative 
   to MPIR_F_MPI+ BOTTOM, we can all ways add a computed address to the Fortran
   MPI_BOTTOM to get the correct address.  In addition, this can fix 
   problems on systems where Fortran integers are too short for addresses,
   since often, addresses will be within 2 GB of each other, and making them
   relative to MPIR_F_MPI_BOTTOM makes the relative addresses fit into
   a Fortran integer.

   (Note that ALL addresses in MPI are relative; an absolute address is
   just one that is relative to MPI_BOTTOM.)
 */
void mpi_address_( location, address, __ierr )
void     *location;
int      *address;
int      *__ierr;
{
MPI_Aint a;
*__ierr = MPI_Address( location, &a );
*address = (int)( a - (MPI_Aint)MPIR_F_MPI_BOTTOM);
}
