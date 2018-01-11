/* cart_map.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_map_ PMPI_CART_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_map_ pmpi_cart_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_map_ pmpi_cart_map
#else
#define mpi_cart_map_ pmpi_cart_map_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_map_ MPI_CART_MAP
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_map_ mpi_cart_map__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_map_ mpi_cart_map
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_map_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                               MPI_Fint *, MPI_Fint *, MPI_Fint * ));

void mpi_cart_map_ ( comm_old, ndims, dims, periods, newrank, __ierr )
MPI_Fint *comm_old;
MPI_Fint *ndims;
MPI_Fint *dims;
MPI_Fint *periods;
MPI_Fint *newrank;
MPI_Fint *__ierr;
{
    int lperiods[20], i;
    int ldims[20];
    int lnewrank;

    if ((int)*ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_old_ptr;
	comm_old_ptr = MPIR_GET_COMM_PTR(MPI_Comm_f2c(*comm_old));
	*__ierr = MPIR_ERROR( comm_old_ptr, MPI_ERR_LIMIT, 
			      "Too many dimensions" );
	return;
	}
    for (i=0; i<(int)*ndims; i++) {
	lperiods[i] = MPIR_FROM_FLOG(periods[i]);
	ldims[i] = (int)dims[i];
    }
    *__ierr = MPI_Cart_map( MPI_Comm_f2c(*comm_old), (int)*ndims, ldims,
			   lperiods, &lnewrank);
    *newrank = (MPI_Fint)lnewrank;
}
