/* cart_coords.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_cart_coords_ PMPI_CART_COORDS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_coords_ pmpi_cart_coords__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_coords_ pmpi_cart_coords
#else
#define mpi_cart_coords_ pmpi_cart_coords_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_cart_coords_ MPI_CART_COORDS
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_cart_coords_ mpi_cart_coords__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_cart_coords_ mpi_cart_coords
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_cart_coords_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint *, MPI_Fint * ));

void mpi_cart_coords_ ( comm, rank, maxdims, coords, __ierr )
MPI_Fint *comm;
MPI_Fint *rank;
MPI_Fint *maxdims;
MPI_Fint *coords;
MPI_Fint *__ierr;
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Cart_coords( MPI_Comm_f2c(*comm),*rank,
                                   *maxdims, coords);
    else {
        int *lcoords;
        int i;

	MPIR_FALLOC(lcoords,(int*)MALLOC(sizeof(int)* (int)*maxdims),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Cart_coords");

        *__ierr = MPI_Cart_coords( MPI_Comm_f2c(*comm), (int)*rank,
                               (int)*maxdims, lcoords);

        for (i=0; i<(int)*maxdims; i++)
	    coords[i] = (MPI_Fint)(lcoords[i]);

	FREE( lcoords );
    }
}
