/* address.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

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

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS  3

void mpi_address_( void *unknown, ...)
{
void *location;
int*address;
int *__ierr;
int buflen;
va_list ap;
MPI_Aint a;

va_start(ap, unknown);
location = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
address         = va_arg(ap, int *);
__ierr          = va_arg(ap, int *);

*__ierr = MPI_Address(location,&a);
*address = (int)( a - (MPI_Aint)MPIR_F_MPI_BOTTOM);
}

#else

void mpi_address_( location, address, __ierr )
void     *location;
int      *address;
int      *__ierr;
{
_fcd temp;
MPI_Aint a;
if (_isfcd(location)) {
	temp = _fcdtocp(location);
	location = (void *) temp;
}
*__ierr = MPI_Address( location, &a );
*address = (int)( a - (MPI_Aint)MPIR_F_MPI_BOTTOM);
}
#endif
#else

/* Prototype to suppress warnings about missing prototypes */
void mpi_address_ ANSI_ARGS(( void *, int *, int * ));

void mpi_address_( location, address, __ierr )
void     *location;
int      *address;
int      *__ierr;
{
    MPI_Aint a, b;
    *__ierr = MPI_Address( location, &a );
    if (*__ierr != MPI_SUCCESS) return;

    b = a - (MPI_Aint)MPIR_F_MPI_BOTTOM;
    *address = (int)( b );
    if (((MPI_Aint)*address) - b != 0) {
	*__ierr = MPIR_ERROR( MPI_COMM_WORLD,     
			      MPI_ERR_ARG | MPIR_ERR_FORTRAN_ADDRESS_RANGE, 
			      "Error in MPI_ADDRESS" );
    }
}
#endif
