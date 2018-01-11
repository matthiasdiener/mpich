/* keyval_create.c */
/* CUSTOM WRAPPER */
#include "mpiimpl.h"

#ifdef POINTER_64_BITS
extern void *MPIR_ToPointer();
extern int MPIR_FromPointer();
extern void MPIR_RmPointer();
#else
#define MPIR_ToPointer(a) (a)
#define MPIR_FromPointer(a) (a)
#define MPIR_RmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mpi_keyval_create_ PMPI_KEYVAL_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_create_ pmpi_keyval_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_create_ pmpi_keyval_create
#else
#define mpi_keyval_create_ pmpi_keyval_create_
#endif
#else
#ifdef FORTRANCAPS
#define mpi_keyval_create_ MPI_KEYVAL_CREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define mpi_keyval_create_ mpi_keyval_create__
#elif !defined(FORTRANUNDERSCORE)
#define mpi_keyval_create_ mpi_keyval_create
#endif
#endif

void mpi_keyval_create_ ( copy_fn, delete_fn, keyval, extra_state, __ierr )
MPI_Copy_function   *copy_fn;
MPI_Delete_function *delete_fn;
int                 *keyval;
void                *extra_state;
int *__ierr;
{
*__ierr = MPIR_Keyval_create( copy_fn, delete_fn, keyval, extra_state, 1 );
}
