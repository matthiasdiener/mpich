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
void mpi_attr_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint *, MPI_Fint * ));

void mpi_attr_get_ ( comm, keyval, attr_value, found, __ierr )
MPI_Fint *comm;
MPI_Fint *keyval;
MPI_Fint *attr_value;
MPI_Fint *found;
MPI_Fint *__ierr;
{
    void *vval;
    int  l_found;

    *__ierr = MPI_Attr_get( MPI_Comm_f2c(*comm), (int)*keyval, &vval,
                            &l_found);

    /* Convert attribute value to integer.  This code handles the case
       where sizeof(int) < sizeof(void *), and the value was stored as a
       void * 
     */
    if ((int)*__ierr || l_found == 0)
	*attr_value = 0;
    else {
	MPI_Aint lvval = (MPI_Aint)vval;
	*attr_value = (int)lvval;
    }

    *found = MPIR_TO_FLOG(l_found);
    return;
}
