/* cart_rank.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_rank_ PMPI_CART_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_rank_ pmpi_cart_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_rank_ pmpi_cart_rank
#else
#define mpi_cart_rank_ pmpi_cart_rank_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_rank_ MPI_CART_RANK
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_rank_ mpi_cart_rank__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_rank_ mpi_cart_rank
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_rank_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                MPI_Fint * ));
void mpi_cart_rank_ ( comm, coords, rank, __ierr )
MPI_Fint *comm;
MPI_Fint *coords;
MPI_Fint *rank;
MPI_Fint *__ierr;
{
    MPI_Comm l_comm;
    int lcoords[20];
    int lrank;
    int ndims;
    int i;

    l_comm = MPI_Comm_f2c(*comm);
    if (l_comm == MPI_COMM_NULL) {
	struct MPIR_COMMUNICATOR *comm_ptr;
	comm_ptr = MPIR_GET_COMM_PTR(MPI_Comm_f2c(*comm));
	*__ierr = MPIR_ERROR( comm_ptr, MPI_ERR_COMM, "Null Communicator");
	return;
	}
	
    MPI_Cartdim_get( MPI_Comm_f2c(*comm), &ndims);
  
    if (ndims > 20) {
	struct MPIR_COMMUNICATOR *comm_ptr;
	comm_ptr = MPIR_GET_COMM_PTR(MPI_Comm_f2c(*comm));
	*__ierr = MPIR_ERROR( comm_ptr, MPI_ERR_LIMIT, "Too many dimensions" );
	return;
	}
 
    for (i=0; i<ndims; i++)
	lcoords[i] = (int)coords[i];
    *__ierr = MPI_Cart_rank( MPI_Comm_f2c(*comm), lcoords, &lrank);
    *rank = (MPI_Fint)lrank;

}

