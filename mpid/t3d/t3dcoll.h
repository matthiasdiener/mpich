/*
 *  $Id: t3dcoll.h,v 1.1 1995/06/07 06:03:09 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DCOLL_INCLUDED
#define _T3DCOLL_INCLUDED

/*
 * ADI-collective support
 *
 * 
 * Description
 * -----------
 *
 * The MPID_USE_ADI_COLLECTIVE symbol is used to determine if the
 * ADI should provide support for collective operations.  If the
 * ADI does support collective operations, it can support them in
 * three different ways.  See the comments in t3dcoll.c for more 
 * information.
 *
 * 
 * The "T3D" collective funcions are not called directly.  Instead 
 * macros are defined for each of the operations:
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
 *
 * Below is the "ifdef" structure for ADI-collective support
 *
 *     #ifdef MPID_USE_ADI_COLLECTIVE
 *         #ifdef MPID_HAS_SHARED_MEM
 *             The ADI has shared memory on all nodes and supports
 *             collective operations.
 *         #elif defined(MPID_COLL_WORLD)
 *             The ADI supports collective operations on communicators
 *             which have the same number of processes as MPI_COMM_WORLD.
 *         #else
 *             The ADI supports collective operations in some unspecified
 *             manner.
 *         #endif
 *     #else 
 *         The ADI does not support collective operations.
 *     #endif
 */


/***************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ***************************************************************************/
#ifdef MPID_USE_ADI_COLLECTIVE
#define MPID_Comm_init(ctx,comm,newcomm) T3D_Comm_init(comm)
#define MPID_Comm_free(ctx,comm)         T3D_Comm_free(comm)
#define MPID_Barrier(ctx,comm)           T3D_Barrier(comm)
#define MPID_Reduce_sum_int(ctx,send,recv,comm)    \
                                  T3D_Reduce_sum_int(send,recv,comm)
#define MPID_Reduce_sum_double(ctx,send,recv,comm) \
                                  T3D_Reduce_sum_double(send,recv,comm)
#else
#define MPID_Comm_init(ctx,comm,newcomm) MPI_SUCCESS
#define MPID_Comm_free(ctx,comm)         MPI_SUCCESS
#endif

#endif
