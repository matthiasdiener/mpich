/* pcontrol.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_pcontrol_ PMPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ pmpi_pcontrol
#else
#define mpi_pcontrol_ pmpi_pcontrol_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_pcontrol_ MPI_PCONTROL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_pcontrol_ mpi_pcontrol
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_pcontrol_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_pcontrol_( level, __ierr )
MPI_Fint *level;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Pcontrol((int)*level);
}
