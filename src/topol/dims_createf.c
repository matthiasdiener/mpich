/* dims_create.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_dims_create_ PMPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ pmpi_dims_create
#else
#define mpi_dims_create_ pmpi_dims_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_dims_create_ MPI_DIMS_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_dims_create_ mpi_dims_create
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_dims_create_ ANSI_ARGS(( MPI_Fint *, MPI_Fint *, MPI_Fint *, 
                                  MPI_Fint * ));

void mpi_dims_create_(nnodes, ndims, dims, __ierr )
MPI_Fint *nnodes;
MPI_Fint *ndims;
MPI_Fint *dims;
MPI_Fint *__ierr;
{

    if (sizeof(MPI_Fint) == sizeof(int))
        *__ierr = MPI_Dims_create(*nnodes,*ndims,dims);
    else {
        int *ldims;
        int i;

	MPIR_FALLOC(ldims,(int*)MALLOC(sizeof(int)* (int)*ndims),
		    MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		    "MPI_Dims_create");

        for (i=0; i<(int)*ndims; i++)
	    ldims[i] = (int)dims[i];

        *__ierr = MPI_Dims_create((int)*nnodes, (int)*ndims, ldims);

        for (i=0; i<(int)*ndims; i++)
	    dims[i] = (MPI_Fint)ldims[i];
	    
        FREE( ldims );
    }

}
