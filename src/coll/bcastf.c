/* bcast.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef _CRAY
#include <fortran.h>
#include <stdarg.h>
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_bcast_ PMPI_BCAST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bcast_ pmpi_bcast__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bcast_ pmpi_bcast
#else
#define mpi_bcast_ pmpi_bcast_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_bcast_ MPI_BCAST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_bcast_ mpi_bcast__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_bcast_ mpi_bcast
#endif
#endif

#ifdef _CRAY
#ifdef _TWO_WORD_FCD
#define NUMPARAMS 6

 void mpi_bcast_ (void *unknown, ...)
{
void             *buffer;
int		*count;
MPI_Datatype    *datatype;
int		*root;
MPI_Comm        *  comm;
int 		*__ierr;
int             buflen;
va_list         ap;

va_start(ap, unknown);
buffer = unknown;
if (_numargs() == NUMPARAMS+1) {
        buflen = va_arg(ap, int) /8;          /* This is in bits. */
}
count =     	va_arg (ap, int *);
datatype =      va_arg(ap, MPI_Datatype *);
root =		va_arg(ap, int *);
comm =          va_arg(ap, MPI_Comm *);
__ierr =        va_arg(ap, int *);

*__ierr = MPI_Bcast(MPIR_F_PTR(buffer),*count,*datatype,*root,*comm );
}

#else

 void mpi_bcast_ ( buffer, count, datatype, root, comm, __ierr )
void             *buffer;
int*count;
MPI_Datatype     * datatype;
int*root;
MPI_Comm         * comm;
int *__ierr;
{
_fcd            temp;
if (_isfcd(buffer)) {
        temp = _fcdtocp(buffer);
        buffer = (void *)temp;
}

*__ierr = MPI_Bcast(MPIR_F_PTR(buffer),*count,*datatype,*root,*comm);
}

#endif
#else
/* Prototype to suppress warnings about missing prototypes */

void mpi_bcast_ ANSI_ARGS(( void *, MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                            MPI_Fint *, MPI_Fint * ));

void mpi_bcast_ ( buffer, count, datatype, root, comm, __ierr )
void     *buffer;
MPI_Fint *count;
MPI_Fint *datatype;
MPI_Fint *root;
MPI_Fint *comm;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Bcast(MPIR_F_PTR(buffer), (int)*count, 
                        MPI_Type_f2c(*datatype), (int)*root,
                        MPI_Comm_f2c(*comm));
}
#endif
