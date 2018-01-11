/*
 *  $Id: chcoll.c,v 1.2 1994/10/26 22:17:25 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char SCCSid[] = "%W% %G%";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#include "mpid.h"

/* 
   These routines provide hooks for the ADI - collective routines.

   The initial sample of these will provide access to operations that
   rely on shared memory; we expect a version for at least MPI_COMM_WORLD
   on the TMC CM5 to be made available.
 */


#ifdef MPID_HAS_SHARED_MEM
/* This is a generic version that uses p4 access to shared memory */
/* Include routines to initialize each ... */
/* ... */
int MPID_CH_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
comm->ADIBarrier = 0;
comm->ADIReduce  = 0;
comm->ADIScan    = 0;
comm->ADIBcast   = 0;
comm->ADICollect = 0;

return MPI_SUCCESS;
}    
int MPID_CH_Comm_free( comm )
MPI_Comm comm;
{
return MPI_SUCCESS;
}

void MPID_CH_Barrier( comm ) 
MPI_Comm comm;
{
}

void MPID_CH_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

void MPID_CH_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
}
#else

int MPID_CH_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
comm->ADIBarrier = 0;
comm->ADIReduce  = 0;
comm->ADIScan    = 0;
comm->ADIBcast   = 0;
comm->ADICollect = 0;

return MPI_SUCCESS;
}    
int MPID_CH_Comm_free( comm )
MPI_Comm comm;
{
return MPI_SUCCESS;
}

void MPID_CH_Barrier( comm ) 
MPI_Comm comm;
{
}

void MPID_CH_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

void MPID_CH_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
}
#endif
