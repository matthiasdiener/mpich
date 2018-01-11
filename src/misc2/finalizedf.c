/* finalized.c */
/* Custom Fortran interface file */

#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_finalized_ PMPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalized_ pmpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalized_ pmpi_finalized
#else
#define mpi_finalized_ pmpi_finalized_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_finalized_ MPI_FINALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_finalized_ mpi_finalized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_finalized_ mpi_finalized
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_finalized_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

/* Definitions of Fortran Wrapper routines */
void mpi_finalized_( flag, __ierr )
MPI_Fint  *flag;
MPI_Fint *__ierr;
{
    int lflag;
    *__ierr = MPI_Finalized(&lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
