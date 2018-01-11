/*
 *  $Id: keyval_create.c,v 1.7 1994/09/29 21:51:22 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"
/*@

MPI_Keyval_create - Generates a new attribute key

Input Parameters:
. copy_fn - Copy callback function for keyval 
. delete_fn - Delete callback function for keyval 
. extra_state - Extra state for callback functions 

Output Parameter:
. keyval - key value for future access (integer) 

Notes:
Key values are global (available for any and all communicators).

There are subtle differences between C and Fortran that require that the
copy_fn be written in the same language that MPI_Keyval_create is called from.
This should not be a problem for most users; only programers using both 
Fortran and C in the same program need to be sure that they follow this rule.
@*/
int MPI_Keyval_create ( copy_fn, delete_fn, keyval, extra_state )
MPI_Copy_function   *copy_fn;
MPI_Delete_function *delete_fn;
int                 *keyval;
void                *extra_state;
{
return MPIR_Keyval_create( copy_fn, delete_fn, keyval, extra_state, 0 );
}

