/* testsome.c */
/* CUSTOM Fortran interface file */
#include "mpiimpl.h"
#include "mpisys.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (int)(a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_testsome_ PMPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testsome_ pmpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testsome_ pmpi_testsome
#else
#define mpi_testsome_ pmpi_testsome_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_testsome_ MPI_TESTSOME
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_testsome_ mpi_testsome__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_testsome_ mpi_testsome
#endif
#endif

 void mpi_testsome_( incount, array_of_requests, outcount, array_of_indices, 
    array_of_statuses, __ierr )
int*incount, *outcount, array_of_indices[];
MPI_Request array_of_requests[];
MPI_Status  array_of_statuses[];
int *__ierr;
{
int i;
#ifdef POINTER_64_BITS
MPI_Request *r = (MPI_Request*)MALLOC(sizeof(MPI_Request)* *incount);
if (!r) {
    *__ierr = MPIR_ERROR(MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
					  "Out of space in MPI_TESTSOME" );
    return;
    }
for (i=0; i<*incount; i++) {
    r[i] = MPIR_ToPointer( *((int *)(array_of_requests)+i) );
    }
*__ierr = MPI_Testsome(*incount,r,outcount,array_of_indices,array_of_statuses);
/* By checking for r[a[i]] = 0, we handle persistant requests */
for (i=0; i<*outcount; i++) {
    if (array_of_indices[i] >= 0) {
	if (r[array_of_indices[i]] == 0) {
	    MPIR_RmPointer( *((int *)(array_of_requests) + 
			      array_of_indices[i]) );
	    *((int *)(array_of_requests)+array_of_indices[i]) = 0;
	    }
	}
    }
FREE( r );

#else
*__ierr = MPI_Testsome(*incount,array_of_requests,outcount,
		       array_of_indices,array_of_statuses);
#endif
for (i=0; i<*outcount; i++) {
    if (array_of_indices[i] >= 0)
	array_of_indices[i] = array_of_indices[i] + 1;
    }
}
