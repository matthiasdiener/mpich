/* unpack.c */
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
#define mpi_unpack_ PMPI_UNPACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_unpack_ pmpi_unpack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_unpack_ pmpi_unpack
#else
#define mpi_unpack_ pmpi_unpack_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_unpack_ MPI_UNPACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_unpack_ mpi_unpack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_unpack_ mpi_unpack
#endif
#endif

 void mpi_unpack_ ( inbuf, insize, position, outbuf, outcount, type, comm, __ierr )
void         *inbuf;
int*insize;
int          *position;
void         *outbuf;
int*outcount;
MPI_Datatype  type;
MPI_Comm      comm;
int *__ierr;
{
*__ierr = MPI_Unpack(inbuf,*insize,position,outbuf,*outcount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
