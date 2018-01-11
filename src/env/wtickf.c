/* wtick.c */
/* Custom Fortran interface file*/
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_wtick_ PMPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtick_ pmpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtick_ pmpi_wtick
#else
#define mpi_wtick_ pmpi_wtick_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_wtick_ MPI_WTICK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_wtick_ mpi_wtick__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_wtick_ mpi_wtick
#endif
#endif

double  mpi_wtick_()
{
return MPI_Wtick();
}
