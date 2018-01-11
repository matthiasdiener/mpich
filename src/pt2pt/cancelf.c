/* cancel.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cancel_ PMPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ pmpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ pmpi_cancel
#else
#define mpi_cancel_ pmpi_cancel_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cancel_ MPI_CANCEL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cancel_ mpi_cancel__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cancel_ mpi_cancel
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cancel_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_cancel_( request, __ierr )
MPI_Fint *request;
MPI_Fint *__ierr;
{
    MPI_Request lrequest;

    lrequest = MPI_Request_f2c(*request);  
    *__ierr = MPI_Cancel(&lrequest); 
}
