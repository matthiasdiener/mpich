/* keyval_create.c */
/* CUSTOM WRAPPER */
#include "mpiimpl.h"

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

/* Prototype to suppress warnings about missing prototypes */
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
void mpi_keyval_create_ ANSI_ARGS(( MPI_Copy_function **, 
				    MPI_Delete_function **, MPI_Fint *, 
                                    void *, MPI_Fint * ));
#else
void mpi_keyval_create_ ANSI_ARGS(( MPI_Copy_function *, 
				    MPI_Delete_function *, MPI_Fint *, 
                                    void *, MPI_Fint * ));
#endif

void mpi_keyval_create_ ( copy_fn, delete_fn, keyval, extra_state, __ierr )
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
MPI_Copy_function   **copy_fn;
MPI_Delete_function **delete_fn;
#else
MPI_Copy_function   *copy_fn;
MPI_Delete_function *delete_fn;
#endif
MPI_Fint            *keyval;
void                *extra_state;
MPI_Fint            *__ierr;
{
    int l_keyval = 0;
#ifdef FORTRAN_SPECIAL_FUNCTION_PTR
    *__ierr = MPIR_Keyval_create( *copy_fn, *delete_fn, &l_keyval, 
				  extra_state, 1 );
#else
    *__ierr = MPIR_Keyval_create( copy_fn, delete_fn, &l_keyval, 
                                  extra_state, 1 );
#endif
    *keyval = (MPI_Fint)l_keyval;
}
