/* startall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_startall_ PMPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ pmpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ pmpi_startall
#else
#define mpi_startall_ pmpi_startall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_startall_ MPI_STARTALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_startall_ mpi_startall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_startall_ mpi_startall
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_startall_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint * ));

void mpi_startall_( count, array_of_requests, __ierr )
MPI_Fint *count;
MPI_Fint array_of_requests[];
MPI_Fint *__ierr;

{ 
   MPI_Request *lrequest;
   MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
   int i;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request) * (int)*count),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_STARTALL" );
	}
	else {
	    lrequest = local_lrequest;
	}
	for (i=0; i<(int)*count; i++) {
            lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}
	*__ierr = MPI_Startall((int)*count,lrequest);
    }
    else 
	*__ierr = MPI_Startall((int)*count,(MPI_Request *)0);

    for (i=0; i<(int)*count; i++) {
        array_of_requests[i] = MPI_Request_c2f( lrequest[i]);
    }
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	FREE( lrequest );
    }
}


