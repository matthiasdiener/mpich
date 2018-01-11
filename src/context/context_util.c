/*
 *  $Id: context_util.c,v 1.17 1994/11/30 15:47:41 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

/* context_util.c - Utilities used by the context chapter functions */

#include "mpiimpl.h"

#define MPIR_MAX(a,b)  (((a)>(b))?(a):(b))

/* 

MPIR_Context_alloc - allocate some number of contexts over given communicator

 */
int MPIR_Context_alloc ( comm, num_contexts, context )
MPI_Comm      comm;
int           num_contexts;
MPIR_CONTEXT *context;
{
  static MPIR_CONTEXT high_context = MPIR_FIRST_FREE_CONTEXT;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* Allocate contexts for intra-comms */
  if (comm->comm_type == MPIR_INTRA) {

    /* Find the highest context */
    MPI_Allreduce(&high_context,context,1,MPIR_CONTEXT_TYPE,MPI_MAX,comm);
  }

  /* Allocate contexts for inter-comms */
  else {
    MPIR_CONTEXT rcontext;
    MPI_Status   status;
    MPI_Comm     inter = comm->comm_coll;
    MPI_Comm     intra = inter->comm_coll;
    int          rank;

    /* Find the highest context on the local group */
    MPI_Allreduce(&high_context,context,1,MPIR_CONTEXT_TYPE,MPI_MAX,intra);

    MPI_Comm_rank ( comm, &rank );
    if (rank == 0) {
      /* Leaders swap info */
      MPI_Sendrecv(   context, 1, MPIR_CONTEXT_TYPE, 0, 0, 
                    &rcontext, 1, MPIR_CONTEXT_TYPE, 0, 0, inter, &status);

      /* Update context to be the highest context */
      (*context) = MPIR_MAX((*context),rcontext);
    }

    /* Leader give context info to everyone else */
    MPI_Bcast (context, 1, MPIR_CONTEXT_TYPE, 0, intra); 
  }

  /* Reset the highest context */
  high_context = (*context) + num_contexts;

  /* Lock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return(MPI_SUCCESS); 
}

/*+

MPIR_Context_dealloc - deallocate previously allocated contexts 

+*/
/*ARGSUSED*/
int MPIR_Context_dealloc ( comm, num, context )
MPI_Comm     comm;
int          num;
MPIR_CONTEXT context;
{
  /* This does nothing currently */
  return (MPI_SUCCESS);
}
