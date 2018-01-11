/* testany.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpimem.h"
#include "mpifort.h"

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_testany_ PMPI_TESTANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testany_ pmpi_testany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testany_ pmpi_testany
#else
#define mpi_testany_ pmpi_testany_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_testany_ MPI_TESTANY
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testany_ mpi_testany__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testany_ mpi_testany
#endif
#endif

/* Prototype to suppress warnings about missing prototypes */
void mpi_testany_ ANSI_ARGS(( MPI_Fint *, MPI_Fint [], MPI_Fint *, 
			      MPI_Fint *, MPI_Fint *, MPI_Fint * ));
void mpi_testany_( count, array_of_requests, index, flag, status, __ierr )
MPI_Fint *count;
MPI_Fint array_of_requests[];
MPI_Fint *index; 
MPI_Fint *flag;
MPI_Fint *status;
MPI_Fint *__ierr;
{
    int lindex;
    int lflag;
    MPI_Request *lrequest;
    MPI_Request local_lrequest[MPIR_USE_LOCAL_ARRAY];
    MPI_Status c_status;
    int i;

    if ((int)*count > 0) {
	if ((int)*count > MPIR_USE_LOCAL_ARRAY) {
	    MPIR_FALLOC(lrequest,(MPI_Request*)MALLOC(sizeof(MPI_Request)* (int)*count),
		        MPIR_COMM_WORLD, MPI_ERR_EXHAUSTED,
		        "MPI_TESTANY");
	}
	else 
	    lrequest = local_lrequest;
	
	for (i=0; i<(int)*count; i++) 
	    lrequest[i] = MPI_Request_f2c( array_of_requests[i] );
	
    }
    else
	lrequest = 0;

    *__ierr = MPI_Testany((int)*count,lrequest,&lindex,&lflag,&c_status);
    if (lindex != -1) {
        if (lflag && !*__ierr) {
#ifdef OLD_POINTER
	    /* By checking for r[i] = 0, we handle persistant requests */
	    if (lrequest[lindex] == MPI_REQUEST_NULL) {
	        MPIR_RmPointer( (int)(lrequest[lindex]) );
	        array_of_requests[lindex] = 0;
	    }
#endif
	    array_of_requests[lindex] = MPI_Request_c2f(lrequest[lindex]);
        }
     }
    if ((int)*count > MPIR_USE_LOCAL_ARRAY) 
	FREE( lrequest );
    
    *flag = MPIR_TO_FLOG(lflag);
    /* See the description of waitany in the standard; the Fortran index ranges
       are from 1, not zero */
    *index = (MPI_Fint)lindex;
    if ((int)*index >= 0) *index = *index + 1;
    MPI_Status_c2f(&c_status, status);
}



