


#define PI_NO_MSG_SEMANTICS
/*
 *  $Id: chcoll.c,v 1.4 1995/01/03 19:40:12 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
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
void *MPID_EUI_Init_barrier();
double MPID_EUI_Barrier_sum_double();

/* This uses the interface in shops2.c, which uses p4 access 
   to shared memory.  Alternate interfaces can replace shops2.c or this file */
int MPID_EUI_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
newcomm->ADIBarrier = MPID_EUI_Init_barrier( newcomm->local_group->np, 1 );
newcomm->ADIReduce  = newcomm->ADIBarrier;
newcomm->ADIScan    = 0;
newcomm->ADIBcast   = newcomm->ADIBarrier;
newcomm->ADICollect = 0;
MPID_EUI_Setup_barrier( newcomm->ADIBarrier, newcomm->local_group->local_rank );

return MPI_SUCCESS;
}    
int MPID_EUI_Comm_free( comm )
MPI_Comm comm;
{
if (comm->ADIBarrier)
    MPID_EUI_Free_barrier( comm->ADIBarrier, 
			  comm->local_group->local_rank == 0 );

return MPI_SUCCESS;
}

void MPID_EUI_Barrier( comm ) 
MPI_Comm comm;
{
MPID_EUI_Wait_barrier( comm->ADIBarrier );
}

void MPID_EUI_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

void MPID_EUI_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
*recvbuf = MPID_EUI_Barrier_sum_double( comm->ADIReduce, sendbuf );
}

#elif defined(MPID_COLL_WORLD)
/* Some systems provide special support for collective operations on the
   'world' group.  This provides access to them */

int MPID_EUI_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
if (newcomm->comm->local_group->np == __NUMNODES) {
    newcomm->ADIBarrier = (void *)newcomm;
    newcomm->ADIReduce  = (void *)newcomm;
    newcomm->ADIScan    = (void *)newcomm;
    newcomm->ADIBcast   = (void *)newcomm;
    newcomm->ADICollect = (void *)newcomm;
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
int MPID_EUI_Comm_free( comm )
MPI_Comm comm;
{
return MPI_SUCCESS;
}

void MPID_EUI_Barrier( comm ) 
MPI_Comm comm;
{
mp_sync(&__ALLGRP);
}

void MPID_EUI_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
int d;
{int _size = 1*sizeof(int);
       mp_combine(sendbuf,sendbuf,i_vadd,&_size,&__ALLGRP);};
*recvbuf = *sendbuf;
}

void MPID_EUI_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
double d;
{int _size=(1)*sizeof(double);
       mp_combine(sendbuf,sendbuf,&_size,d_vadd,&__ALLGRP);};
*recvbuf = *sendbuf;
}

#else

int MPID_EUI_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
newcomm->ADIBarrier = 0;
newcomm->ADIReduce  = 0;
newcomm->ADIScan    = 0;
newcomm->ADIBcast   = 0;
newcomm->ADICollect = 0;

return MPI_SUCCESS;
}    
int MPID_EUI_Comm_free( comm )
MPI_Comm comm;
{
return MPI_SUCCESS;
}

void MPID_EUI_Barrier( comm ) 
MPI_Comm comm;
{
}

void MPID_EUI_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

void MPID_EUI_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
}
#endif
