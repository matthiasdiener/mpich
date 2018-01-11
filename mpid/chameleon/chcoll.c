/*
 *  $Id: chcoll.c,v 1.6 1995/08/11 00:23:42 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chcoll.c,v 1.6 1995/08/11 00:23:42 gropp Exp $";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it.  

    ** UNTESTED **
 */

#include "mpid.h"

/* 
   These routines provide hooks for the ADI - collective routines.

   The initial sample of these will provide access to operations that
   rely on shared memory; we expect a version for at least MPI_COMM_WORLD
   on the TMC CM5 to be made available.

   The next pass will also provide support for operations on contiguous,
   basic datatypes (for example, broadcast a block of memory).
 */


#ifdef MPID_HAS_SHARED_MEM
void *MPID_CH_Init_barrier();
double MPID_CH_Barrier_sum_double();

/* This uses the interface in shops2.c, which uses p4 access 
   to shared memory.  Alternate interfaces can replace shops2.c or this file */
int MPID_CH_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
newcomm->ADIBarrier = MPID_CH_Init_barrier( newcomm->local_group->np, 1 );
/* Once the processes are created, this may need a broadcast; should be
   part of INIT_BARRIER */
newcomm->ADIReduce  = newcomm->ADIBarrier;
newcomm->ADIScan    = 0;
newcomm->ADIBcast   = newcomm->ADIBarrier;
newcomm->ADICollect = 0;
MPID_CH_Setup_barrier( newcomm->ADIBarrier, newcomm->local_group->local_rank );

return MPI_SUCCESS;
}    
int MPID_CH_Comm_free( comm )
MPI_Comm comm;
{
if (comm->ADIBarrier)
    MPID_CH_Free_barrier( comm->ADIBarrier, 
			  comm->local_group->local_rank == 0 );

return MPI_SUCCESS;
}

void MPID_CH_Barrier( comm ) 
MPI_Comm comm;
{
MPID_CH_Wait_barrier( comm->ADIBarrier );
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
*recvbuf = MPID_CH_Barrier_sum_double( comm->ADIReduce, sendbuf );
}

#elif defined(MPID_COLL_WORLD)
/* Some systems provide special support for collective operations on the
   'world' group.  This provides access to them */

int MPID_CH_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
if (newcomm->comm->local_group->np == PInumtids) {
    newcomm->ADIBarrier = (void *)newcomm;
    newcomm->ADIReduce  = (void *)newcomm;
    newcomm->ADIScan    = (void *)0;
    newcomm->ADIBcast   = (void *)0;
    newcomm->ADICollect = (void *)0;
    }
else {
    newcomm->ADIBarrier = 0;
    newcomm->ADIReduce  = 0;
    newcomm->ADIScan    = 0;
    newcomm->ADIBcast   = 0;
    newcomm->ADICollect = 0;
    }
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
PIgsync(PSAllProcs);
}

void MPID_CH_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
int d;
*recvbuf = *sendbuf;
PIgisum(recvbuf,1,&d,PSAllProcs);
}

void MPID_CH_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
double d;
*recvbuf = *sendbuf;
PIgdsum(recvbuf,1,&d,PSAllProcs);
}

#else

int MPID_CH_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
newcomm->ADIBarrier = 0;
newcomm->ADIReduce  = 0;
newcomm->ADIScan    = 0;
newcomm->ADIBcast   = 0;
newcomm->ADICollect = 0;

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
