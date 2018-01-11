/* initialize.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_initialized_ PMPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ pmpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ pmpi_initialized
#else
#define mpi_initialized_ pmpi_initialized_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_initialized_ MPI_INITIALIZED
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_initialized_ mpi_initialized__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_initialized_ mpi_initialized
#endif
#endif
/* Prototype to suppress warnings about missing prototypes */
void mpi_initialized_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));
void mpi_initialized_( flag, __ierr )
MPI_Fint *flag;
MPI_Fint *__ierr;
{
    int lflag;
    *__ierr = MPI_Initialized(&lflag);
    *flag = MPIR_TO_FLOG(lflag);
}
