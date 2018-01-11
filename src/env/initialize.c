/*
 *  $Id: initialize.c,v 1.6 1995/12/21 21:57:37 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */


#ifndef lint
static char vcid[] = "$Id: initialize.c,v 1.6 1995/12/21 21:57:37 gropp Exp $";
#endif /* lint */
#include "mpiimpl.h"

int MPIR_Has_been_initialized = 0;

/*@
   MPI_Initialized - Indicates whether 'MPI_Init' has been called.

Output Parameter:
. flag - Flag is true if 'MPI_Init' has been called and false otherwise. 

.N fortran
@*/
int MPI_Initialized( flag )
int  *flag;
{
/* MPI_Init sets MPIR_Has_been_initialized to 1, MPI_Finalize sets to 2.
   Currently, there is no way to determine, if MPI_Finalize has been called,
   other than trapping references to free'd memory.  Perhaps we should set
   MPI_COMM_WORLD to 0 after freeing it?
 */
*flag = MPIR_Has_been_initialized > 0;
return MPI_SUCCESS;
}
