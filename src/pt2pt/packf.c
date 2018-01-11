/* pack.c */
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
#define mpi_pack_ PMPI_PACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_ pmpi_pack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_ pmpi_pack
#else
#define mpi_pack_ pmpi_pack_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pack_ MPI_PACK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pack_ mpi_pack__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pack_ mpi_pack
#endif
#endif

 void mpi_pack_ ( inbuf, incount, type, outbuf, outcount, position, comm, __ierr )
void         *inbuf;
int*incount;
MPI_Datatype  type;
void         *outbuf;
int*outcount;
int          *position;
MPI_Comm      comm;
int *__ierr;
{
*__ierr = MPI_Pack(inbuf,*incount,
	(MPI_Datatype)MPIR_ToPointer( *(int*)(type) ),outbuf,*outcount,position,
	(MPI_Comm)MPIR_ToPointer( *(int*)(comm) ));
}
