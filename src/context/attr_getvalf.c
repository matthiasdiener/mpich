/* attr_getval.c */
/* THIS IS A CUSTOM WRAPPER */

#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_attr_get_ PMPI_ATTR_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_get_ pmpi_attr_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_get_ pmpi_attr_get
#else
#define mpi_attr_get_ pmpi_attr_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_attr_get_ MPI_ATTR_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_attr_get_ mpi_attr_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_attr_get_ mpi_attr_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_attr_get_ ANSI_ARGS(( MPI_Comm *, int *, int *, int *, int * ));

void mpi_attr_get_ ( comm, keyval, attr_value, found, __ierr )
MPI_Comm *comm;
int *keyval;
int *attr_value;
int *found;
int *__ierr;
{
    void *vval;

    *__ierr = MPI_Attr_get( *comm, *keyval,&vval,found);

    /* Convert attribute value to integer.  This code handles the case
       where sizeof(int) < sizeof(void *), and the value was stored as a
       void * 
     */
    if (*__ierr || *found == 0)
	*attr_value = 0;
    else {
	MPI_Aint lvval = (MPI_Aint)vval;
	*attr_value = (int)lvval;
    }

    *found = MPIR_TO_FLOG(*found);
    return;
}
