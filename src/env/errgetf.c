/* errget.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_errhandler_get_ PMPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ pmpi_errhandler_get
#else
#define mpi_errhandler_get_ pmpi_errhandler_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_get_ MPI_ERRHANDLER_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_get_ mpi_errhandler_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_errhandler_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                     MPI_Fint * ));

void mpi_errhandler_get_( comm, errhandler, __ierr )
MPI_Fint *comm;
MPI_Fint *errhandler;
MPI_Fint *__ierr;
{
    MPI_Errhandler l_errhandler;
    *__ierr = MPI_Errhandler_get( MPI_Comm_f2c(*comm), &l_errhandler );
    *errhandler = MPI_Errhandler_c2f(l_errhandler);
}
