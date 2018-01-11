/*
 *  $Id: errfree.c,v 1.12 1997/01/07 01:46:11 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "sbcnst2.h"
#define MPIR_SBfree MPID_SBfree
#else
#include "mpisys.h"
#endif

/*@
  MPI_Errhandler_free - Frees an MPI-style errorhandler

Input Parameter:
. errhandler - MPI error handler (handle).  Set to 'MPI_ERRHANDLER_NULL' on 
exit.

.N fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_ARG
@*/
int MPI_Errhandler_free( errhandler )
MPI_Errhandler *errhandler;
{
    struct MPIR_Errhandler *old;
    static char myname[] = "MPI_ERRHANDLER_FREE";

    TR_PUSH(myname);

    old = MPIR_GET_ERRHANDLER_PTR(*errhandler);
    MPIR_TEST_MPI_ERRHANDLER(*errhandler,old,MPIR_COMM_WORLD,myname);

    MPIR_REF_DECR(old);
    if (old->ref_count <= 0) {
	MPIR_CLR_COOKIE(old);
	MPIR_SBfree ( MPIR_errhandlers, old );
	MPIR_RmPointer( *errhandler );
	}

    *errhandler = MPI_ERRHANDLER_NULL;
    TR_POP;
    return MPI_SUCCESS;
}
