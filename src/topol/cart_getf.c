/* cart_get.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_get_ PMPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ pmpi_cart_get
#else
#define mpi_cart_get_ pmpi_cart_get_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_get_ MPI_CART_GET
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_get_ mpi_cart_get
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_get_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_cart_get_ ( comm, maxdims, dims, periods, coords, __ierr )
MPI_Fint *comm;
MPI_Fint *maxdims;
MPI_Fint *dims; 
MPI_Fint *periods;
MPI_Fint *coords;
MPI_Fint *__ierr;
{
    struct MPIR_COMMUNICATOR *comm_ptr;
    int lperiods[20], i;
    int ldims[20];
    int lcoords[20];

    if ((int)*maxdims > 20) {
	comm_ptr = MPIR_GET_COMM_PTR(MPI_Comm_f2c(*comm));
	*__ierr = MPIR_ERROR( comm_ptr, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
    *__ierr = MPI_Cart_get( MPI_Comm_f2c(*comm), (int)*maxdims, ldims,
                            lperiods, lcoords);
    for (i=0; i<(int)*maxdims; i++) {
	   dims[i] = (MPI_Fint)ldims[i];
	   periods[i] = MPIR_TO_FLOG(lperiods[i]);
	   coords[i] = (MPI_Fint)lcoords[i];
    }
}



