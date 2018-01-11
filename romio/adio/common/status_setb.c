/* 
 *   $Id: status_setb.c,v 1.3 2001/03/06 00:02:41 rross Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifdef MPICH

#include "mpi.h"

void MPID_Status_set_bytes(MPI_Status *status, int nbytes);
int MPIR_Status_set_bytes(MPI_Status *status, MPI_Datatype datatype, 
			  int nbytes);

int MPIR_Status_set_bytes(MPI_Status *status, MPI_Datatype datatype, 
			  int nbytes)
{
    MPID_Status_set_bytes(status, nbytes);
    return MPI_SUCCESS;
}

#endif
