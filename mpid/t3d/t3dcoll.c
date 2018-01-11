/*
 *  $Id: t3dcoll.c,v 1.1 1995/06/07 06:03:06 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */
#ifndef lint
static char vcid[] = "$Id: t3dcoll.c,v 1.1 1995/06/07 06:03:06 bright Exp $";
#endif

#include "mpid.h"

/* 
 *   ADI-collective support
 *
 *
 * Interface Description
 * ---------------------
 *
 * These routines provide hooks for the ADI - collective routines.  They
 * are included if the "MPID_USE_ADI_COLLECTIVE" symbol is defined.
 * On some systems, there is special hardware for some collective operations.
 * On others, there are special techniques that can be applied to 
 * significantly speed up some collective operations.
 * 
 * It is not necessary to support ANY collective operations within the ADI.
 * However, the ADI does provide a way for the ADI to perform some 
 * operations.  In addition, these operations may be provided for just
 * MPI_COMM_WORLD or for any communicator.
 * 
 * ADI-collective routines can be provided in one of three ways:
 *
 *   1)  MPID_HAS_SHARED_MEM is defined.  Collective operations
 *       use shared memory.
 *   2)  MPID_COLL_WORLD is defined.  ADI Collective operations over
 *       the entire set of processes.
 *   3)  None of the above.  The ADI supports collective operations
 *       in some unspecified manner.
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *   int T3D_Comm_init ( MPI_Comm comm, MPI_Comm newcomm )
 *      Initializes "newcomm" for ADI collective operations.
 *
 *   int T3D_Comm_free ( MPI_Comm comm )
 *      Frees a communicators ADI collective resources.
 *
 *   void T3D_Barrier ( MPI_Comm comm )
 *      Performs a barrier operation on "comm".
 *
 *   void T3D_Reduce_sum_int ( int *sbuf, int *rbuf,   MPI_Comm comm )
 *   void T3D_Reduce_sum_double( double *s, double *r, MPI_Comm comm )
 *      Performs a reduce (?ND? allreduce?) operation on one element
 *      over "comm".  These are the ones that are currently provided,
 *      however more of these will be provided for each datatype and
 *      operation.
 *
 * Support for operations on contiguous, basic datatypes (for example,
 * broadcast a block of memory) may also be included at a later date.
 *
 * The ADI will eventuall provide the following classes of
 * collective operations:
 *
 *   1) MPID_Barrier - Barrier
 *
 *   2) MPID_Reduce_op_type - Reduction (op == sum, prod, etc, 
 *      type = double, int, etc).  Define MPID_Reduce if ANY are defined
 *
 *   3) MPID_Scan_op_type - Scan reduction.  Define MPID_Scan if ANY are 
 *      defined.
 *
 *   4) MPID_Bcast - Broadcast (1 to all)
 *
 *   5) MPID_AlltoAll - All to all
 *
 *
 *   
 * Implementation Notes
 * --------------------
 *   
 * Since these may require the creation of special data structures,
 * the ADI routine MPID_Comm_init must be called before these may be
 * used.  MPID_Comm_free must be called when a communicator is
 * destroyed to ensure that any resources are returned.
 *
 * The "MPI_Comm" structure currently has the following fields that
 * can be used for ADI-supported collective operations. These are
 * ignored if the ADI doesn't support the operation; otherwise, they
 * are null if the operation is not supported on this communicator and
 * non-null otherwise.
 *
 *      void          *ADIBarrier;
 *      void          *ADIReduce;
 *      void          *ADIScan;
 *      void          *ADIBcast;
 *      void          *ADICollect;
 *
 *
 * The "T3D" versions are not called directly.  Instead macros
 * are defined for each of these operations:
 *
 *      #define MPID_Barrier  T3D_Barrier
 *
 * Since all of the "routines" are actually macros, this allows the
 * API code to contain code like:
 *
 *     #ifdef MPID_Barrier
 *        if (comm->AdiBarrier) 
 *          MPID_Barrier( comm->AdiCtx, comm );
 *        else 
 *     #endif
 *        {
 *          ... code using pt-2-pt
 *        }
 *
 * Not yet decided: are the reduction and scan reduced to a single root 
 * or to all. 
 */


/* If the ADI supports collective operations and they are used */
#ifdef MPID_USE_ADI_COLLECTIVE


#ifdef MPID_HAS_SHARED_MEM
/* Versions of ADI-collective code which use shared memory */


/***************************************************************************
   T3D_Comm_init  

   Description: 
      This routine is responsible for initializing any data areas
      needed to perform any ADI-supported collective operations (such
      as MPID_Barrier).  In addition, this routine is assumed to be
      collective over the "comm" communicator.  If "comm" is NULL,
      MPI_COMM_WORLD must be used and all processes must call this
      routine. 
 ***************************************************************************/
int T3D_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
/*  Initialize communicator structure.  Initialize to NULL if not used.
    newcomm->ADIBarrier = ...
    newcomm->ADIReduce  = ...
    newcomm->ADIScan    = ...
    newcomm->ADIBcast   = ...
    newcomm->ADICollect = ...
*/
    return (MPI_SUCCESS);
}    


/***************************************************************************
   T3D_Comm_free
 ***************************************************************************/
int T3D_Comm_free( comm )
MPI_Comm comm;
{
/*  Free resources associated with ADI-collective operations
    if (comm->ADIBarrier)
        -- free resource --
    if (comm->ADIReduce)
        -- free resource --
    etc.
*/
    return (MPI_SUCCESS);
}

/***************************************************************************
   T3D_Barrier
 ***************************************************************************/
void T3D_Barrier( comm ) 
MPI_Comm comm;
{
    /* Perform ADI barrier operation */
}

/***************************************************************************
   T3D_Reduce_sum_int
 ***************************************************************************/
void T3D_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

/***************************************************************************
   T3D_Reduce_sum_double
 ***************************************************************************/
void T3D_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
}



#elif defined(MPID_COLL_WORLD)
/* Some systems provide special support for collective operations on
   the 'world' group.  This provides access to them. */

/***************************************************************************
   T3D_Comm_init  

   Description: 
      This routine is responsible for initializing any data areas
      needed to perform any ADI-supported collective operations (such
      as MPID_Barrier).  In addition, this routine is assumed to be
      collective over the "comm" communicator.  If "comm" is NULL,
      MPI_COMM_WORLD must be used and all processes must call this
      routine. 
 ***************************************************************************/
int T3D_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
/*  Initialize collective-ADI fields of MPI_Comm structure if the
    communicator is the same size as the world.

    if (newcomm->comm->local_group->np == world size) {
p        newcomm->ADIBarrier = ...
        newcomm->ADIReduce  = ...
        newcomm->ADIScan    = ...
        newcomm->ADIBcast   = ...
        newcomm->ADICollect = ...
    }
    else {
        newcomm->ADIBarrier = (void *)0;
        newcomm->ADIReduce  = (void *)0;
        newcomm->ADIScan    = (void *)0;
        newcomm->ADIBcast   = (void *)0;
        newcomm->ADICollect = (void *)0;
    }
*/
    return (MPI_SUCCESS);
}

/***************************************************************************
   T3D_Comm_free
 ***************************************************************************/
int T3D_Comm_free( comm )
MPI_Comm comm;
{
/*  Free resources associated with ADI-collective operations
    if (comm->ADIBarrier)
        -- free resource --
    if (comm->ADIReduce)
        -- free resource --
    etc.
*/
    return MPI_SUCCESS;
}

/***************************************************************************
   T3D_Barrier
 ***************************************************************************/
void T3D_Barrier( comm ) 
MPI_Comm comm;
{
    /* If this is called, perform barrier over all processes */
}

/***************************************************************************
   T3D_Reduce_sum_int
 ***************************************************************************/
void T3D_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
    /* If this is called, reduce over all processes */
}

/***************************************************************************
   T3D_Reduce_sum_double
 ***************************************************************************/
void T3D_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
    /* If this is called, reduce over all processes */
}


#else
/* The ADI provides support for collective operations in some unspecified
   way */

/***************************************************************************
   T3D_Comm_init  

   Description: 
      This routine is responsible for initializing any data areas
      needed to perform any ADI-supported collective operations (such
      as MPID_Barrier).  In addition, this routine is assumed to be
      collective over the "comm" communicator.  If "comm" is NULL,
      MPI_COMM_WORLD must be used and all processes must call this
      routine. 
 ***************************************************************************/
int T3D_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
/*
    newcomm->ADIBarrier = 0;
    newcomm->ADIReduce  = 0;
    newcomm->ADIScan    = 0;
    newcomm->ADIBcast   = 0;
    newcomm->ADICollect = 0;
*/
    return MPI_SUCCESS;
}    

/***************************************************************************
   T3D_Comm_free
 ***************************************************************************/
int T3D_Comm_free( comm )
MPI_Comm comm;
{
/*  Free resources associated with ADI-collective operations
    if (comm->ADIBarrier)
        -- free resource --
    if (comm->ADIReduce)
        -- free resource --
    etc.
*/
    return (MPI_SUCCESS);
}

/***************************************************************************
   T3D_Barrier
 ***************************************************************************/
void T3D_Barrier( comm ) 
MPI_Comm comm;
{
}

/***************************************************************************
   T3D_Reduce_sum_int
 ***************************************************************************/
void T3D_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

/***************************************************************************
   T3D_Reduce_sum_double
 ***************************************************************************/
void T3D_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
}
#endif
#endif
