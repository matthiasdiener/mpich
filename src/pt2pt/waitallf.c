/* waitall.c */
/* Custom Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_waitall_ PMPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ pmpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ pmpi_waitall
#else
#define mpi_waitall_ pmpi_waitall_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_waitall_ MPI_WAITALL
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitall_ mpi_waitall__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitall_ mpi_waitall
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_waitall_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [],
			      MPI_Fint [][MPI_STATUS_SIZE], 
			      MPI_Fint *));

void mpi_waitall_(count, array_of_requests, array_of_statuses, __ierr )
MPI_Fint *count;
MPI_Fint array_of_requests[];
MPI_Fint array_of_statuses[][MPI_STATUS_SIZE];
MPI_Fint *__ierr;
{
    int i;
    MPI_Request *lrequest;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status *c_status;
    MPI_Status local_c_status[MPIR_USE_LOCAL_ARRAY];

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request) * 
                        (int)*count), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITALL" );

	    MPIR_FALLOC(c_status,(MPI_Status*)MALLOC(sizeof(MPI_Status) * 
                        (int)*count), MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITALL" );
	}
	else {
	    lrequest = local_lrequest;
	    c_status = local_c_status;
	}

	for (i=0; i<(int)*count; i++) {
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	}

	*__ierr = MPI_Waitall((int)*count,lrequest,c_status);
	/* By checking for lrequest[i] = 0, we handle persistant requests */
	for (i=0; i<(int)*count; i++) {
#ifdef OLD_POINTER
	    if (lrequest[i] == MPI_REQUEST_NULL) {
		MPIR_RmPointer( (int)(lrequest[i]) );
		array_of_requests[i] = 0;
	    }
	    else
#endif
	        array_of_requests[i] = MPI_Request_c2f( lrequest[i] );
	}
    }
    else 
	*__ierr = MPI_Waitall((int)*count,(MPI_Request *)0, c_status );

    for (i=0; i<(int)*count; i++) 
	MPI_Status_c2f(&(c_status[i]), &(array_of_statuses[i][0]) );
    
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
        FREE( lrequest );
        FREE( c_status );
    }
}






