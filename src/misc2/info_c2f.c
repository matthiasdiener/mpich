/* 
 *   $Id: info_c2f.c,v 1.5 1998/05/07 19:42:27 gropp Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"

/*@
    MPI_Info_c2f - Translates a C info handle to a Fortran info handle

Input Parameters:
. info - C info handle (integer)

Return Value:
Fortran info handle (handle)
@*/
MPI_Fint MPI_Info_c2f(MPI_Info info)
{
#ifndef INT_LT_POINTER
    return (MPI_Fint) info;
#else
    int i;

    if ((info <= (MPI_Info) 0) || (info->cookie != MPIR_INFO_COOKIE)) 
	return (MPI_Fint) 0;
    if (!MPIR_Infotable) {
	MPIR_Infotable_max = 1024;
	MPIR_Infotable = (MPI_Info *)
	    MALLOC(MPIR_Infotable_max*sizeof(MPI_Info)); 
        MPIR_Infotable_ptr = 0;  /* 0 can't be used though, because 
                                  MPI_INFO_NULL=0 */
	for (i=0; i<MPIR_Infotable_max; i++) MPIR_Infotable[i] = MPI_INFO_NULL;
    }
    if (MPIR_Infotable_ptr == MPIR_Infotable_max-1) {
	MPIR_Infotable = (MPI_Info *) realloc(MPIR_Infotable, 
                           (MPIR_Infotable_max+1024)*sizeof(MPI_Info));
	if (!MPIR_Infotable){
	    printf("realloc failed in MPI_Info_c2f\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}
	for (i=MPIR_Infotable_max; i<MPIR_Infotable_max+1024; i++) 
	    MPIR_Infotable[i] = MPI_INFO_NULL;
	MPIR_Infotable_max += 1024;
    }
    MPIR_Infotable_ptr++;
    MPIR_Infotable[MPIR_Infotable_ptr] = info;
    return (MPI_Fint) MPIR_Infotable_ptr;
#endif
}
