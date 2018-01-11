/* waitsome.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_waitsome_ PMPI_WAITSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitsome_ pmpi_waitsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitsome_ pmpi_waitsome
#else
#define mpi_waitsome_ pmpi_waitsome_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_waitsome_ MPI_WAITSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_waitsome_ mpi_waitsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_waitsome_ mpi_waitsome
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_waitsome_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint *, 
                               MPI_Fint [], MPI_Fint [][MPI_STATUS_SIZE],
                               MPI_Fint * ));

void mpi_waitsome_( incount, array_of_requests, outcount, array_of_indices, 
    array_of_statuses, __ierr )
MPI_Fint *incount;
MPI_Fint *outcount;
MPI_Fint array_of_indices[];
MPI_Fint array_of_requests[];
MPI_Fint array_of_statuses[][MPI_STATUS_SIZE];
MPI_Fint *__ierr;
{
    int i,j,found;
    int loutcount;
    int *l_indices;
    int local_l_indices[MPIR_USE_LOCAL_ARRAY];
    MPI_Request *lrequest;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status * c_status;
    MPI_Status local_c_status[MPIR_USE_LOCAL_ARRAY];

    if ((int)*incount > 0) {
	if ((int)*incount > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request)* (int)*incount),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITSOME" );

	    MPIR_FALLOC(l_indices,(int*)MALLOC(sizeof(int)* (int)*incount),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITSOME" );

	    MPIR_FALLOC(c_status,(MPI_Status*)MALLOC(sizeof(MPI_Status)* (int)*incount),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		        "MPI_WAITSOME" );
	}
	else {
	    lrequest = local_lrequest;
	    l_indices = local_l_indices;
	    c_status = local_c_status;
	}

	for (i=0; i<(int)*incount; i++) 
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	
	*__ierr = MPI_Waitsome((int)*incount,lrequest,&loutcount,l_indices,
			       c_status);

/* By checking for lrequest[l_indices[i]] = 0, 
   we handle persistant requests */
        for (i=0; i<(int)*incount; i++) {
	    if ( i < loutcount) {
                if (l_indices[i] >= 0) {
#ifdef OLD_POINTER
		    if (lrequest[l_indices[i]] == 0) {
		        MPIR_RmPointer( (int)(lrequest[l_indices[i]]) ); 
		        array_of_requests[l_indices[i]] = 0;
		    }
                    else
#endif
		        array_of_requests[l_indices[i]] = 
			      MPI_Request_c2f( lrequest[l_indices[i]] );
		}
	    }
	    else {
		found = 0;
		j = 0;
		while ( (!found) && (j<loutcount) ) {
		    if (l_indices[j++] == i)
			found = 1;
		}
		if (!found)
	            array_of_requests[i] = MPI_Request_c2f( lrequest[i] );
	    }
	}
    }
    else 
	*__ierr = MPI_Waitsome( (int)*incount, (MPI_Request *)0, &loutcount,
			       l_indices, c_status );

    for (i=0; i<loutcount; i++) {
	MPI_Status_c2f( &c_status[i], &(array_of_statuses[i][0]) );
	if (l_indices[i] >= 0)
	    array_of_indices[i] = l_indices[i] + 1;

    }
    *outcount = (MPI_Fint)loutcount;
    if ((int)*incount > MPIR_USE_LOCAL_ARRAY) {
        FREE( l_indices );
        FREE( lrequest );
        FREE( c_status );
    }
}
