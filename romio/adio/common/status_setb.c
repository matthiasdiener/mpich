/* 
 *   $Id: status_setb.c,v 1.1 2000/02/09 21:30:08 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#ifdef MPICH

#include "mpi.h"

int MPIR_Status_set_bytes(MPI_Status *status, MPI_Datatype datatype, 
			  int nbytes)
{
    status->count = nbytes;
    return MPI_SUCCESS;
}

#endif
