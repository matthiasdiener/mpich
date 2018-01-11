/*
 *  $Id: finalized.c,v 1.3 1998/03/13 22:38:32 gropp Exp $
 *
 *  (C) 1997 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"

/*@
   MPI_Finalized - Indicates whether 'MPI_Finalize' has been called.

Output Parameter:
. flag - Flag is true if 'MPI_Finalize' has been called and false otherwise. 

.N fortran
@*/
int MPI_Finalized( flag )
int  *flag;
{
/* 
   MPI_Init sets MPIR_Has_been_initialized to 1, MPI_Finalize sets to 2.
 */
    *flag = MPIR_Has_been_initialized >= 2;
    return MPI_SUCCESS;
}
