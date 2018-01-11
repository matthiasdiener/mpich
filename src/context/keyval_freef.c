/* keyval_free.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_keyval_free_ PMPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ pmpi_keyval_free
#else
#define mpi_keyval_free_ pmpi_keyval_free_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_keyval_free_ MPI_KEYVAL_FREE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_free_ mpi_keyval_free
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_keyval_free_ ANSI_ARGS(( MPI_Fint *, MPI_Fint * ));

void mpi_keyval_free_ ( keyval, __ierr )
MPI_Fint *keyval;
MPI_Fint *__ierr;
{
    int l_keyval = (int)*keyval;
    *__ierr = MPI_Keyval_free(&l_keyval);
    *keyval = (MPI_Fint)l_keyval;
}
