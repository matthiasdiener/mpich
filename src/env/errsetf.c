/* errset.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_errhandler_set_ PMPI_ERRHANDLER_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_set_ pmpi_errhandler_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_set_ pmpi_errhandler_set
#else
#define mpi_errhandler_set_ pmpi_errhandler_set_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_errhandler_set_ MPI_ERRHANDLER_SET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_errhandler_set_ mpi_errhandler_set__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_errhandler_set_ mpi_errhandler_set
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_errhandler_set_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, 
                                     MPI_Fint * ));

void mpi_errhandler_set_( comm, errhandler, __ierr )
MPI_Fint *comm;
MPI_Fint *errhandler;
MPI_Fint *__ierr;
{
    MPI_Errhandler l_errhandler = MPI_Errhandler_f2c(*errhandler);
    *__ierr = MPI_Errhandler_set(MPI_Comm_f2c(*comm), l_errhandler );
}
