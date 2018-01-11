/* attr_putval.c */
/* THIS IS A CUSTOM WRAPPER */

#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_attr_put_ PMPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ pmpi_attr_put
#else
#define mpi_attr_put_ pmpi_attr_put_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_put_ MPI_ATTR_PUT
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_put_ mpi_attr_put
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_attr_put_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint * ));

void mpi_attr_put_ ( comm, keyval, attr_value, __ierr )
MPI_Fint *comm;
MPI_Fint *keyval;
MPI_Fint *attr_value;
MPI_Fint *__ierr;
{
    *__ierr = MPI_Attr_put( MPI_Comm_f2c(*comm), (int)*keyval,
                            (void *)(MPI_Aint)((int)*attr_value));
}
