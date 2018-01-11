/*
 *  $Id: intra_fns.c,v 1.7 1997/02/18 23:06:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "mpimem.h"
/* pt2pt for MPIR_Type_get_limits */
#include "mpipt2pt.h"
#else
#include "mpisys.h"
#endif
#include "coll.h"
#include "mpiops.h"

/*
 * Provide the collective ops structure for intra communicators.
 * Reworked from the existing code by James Cownie (Meiko) 31 May 1995
 *
 * We put all of the functions in this one file, since this allows 
 * them to be static, avoiding name space pollution, and
 * we're going to need them all anyway. 
 *
 * These functions assume that the communicator is valid; routines that
 * call these should confirm that
 */

/* Forward declarations */
static int intra_Barrier ANSI_ARGS((struct MPIR_COMMUNICATOR *comm ));
static int intra_Bcast ANSI_ARGS((void* buffer, int count, 
				  struct MPIR_DATATYPE * datatype, int root, 
				  struct MPIR_COMMUNICATOR *comm ));
static int intra_Gather ANSI_ARGS((void*, int, 
				   struct MPIR_DATATYPE *, void*, 
				   int, struct MPIR_DATATYPE *, 
				   int, struct MPIR_COMMUNICATOR *)); 
static int intra_Gatherv ANSI_ARGS((void*, int, 
				    struct MPIR_DATATYPE *, 
				    void*, int *, 
				    int *, struct MPIR_DATATYPE *, 
				    int, struct MPIR_COMMUNICATOR *)); 
static int intra_Scatter ANSI_ARGS((void* sendbuf, int sendcount, 
				    struct MPIR_DATATYPE * sendtype, 
				    void* recvbuf, int recvcount, 
				    struct MPIR_DATATYPE * recvtype, 
				    int root, struct MPIR_COMMUNICATOR *comm));
static int intra_Scatterv ANSI_ARGS((void*, int *, 
				     int *, struct MPIR_DATATYPE *, 
				     void*, int, 
				     struct MPIR_DATATYPE *, int, 
				     struct MPIR_COMMUNICATOR *));
static int intra_Allgather ANSI_ARGS((void*, int, 
				      struct MPIR_DATATYPE *, 
				      void*, int, 
				      struct MPIR_DATATYPE *, 
				      struct MPIR_COMMUNICATOR *comm));
static int intra_Allgatherv ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, 
				       void*, int *, int *, 
				       struct MPIR_DATATYPE *, 
				       struct MPIR_COMMUNICATOR *));
static int intra_Alltoall ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, 
				     void*, int, struct MPIR_DATATYPE *, 
				     struct MPIR_COMMUNICATOR *));
static int intra_Alltoallv ANSI_ARGS((void*, int *, int *, 
				      struct MPIR_DATATYPE *, void*, int *, 
				      int *, struct MPIR_DATATYPE *, 
				      struct MPIR_COMMUNICATOR *));
static int intra_Reduce ANSI_ARGS((void*, void*, int, struct MPIR_DATATYPE *, 
				   MPI_Op, 
				   int, struct MPIR_COMMUNICATOR *));
static int intra_Allreduce ANSI_ARGS((void*, void*, int, 
				      struct MPIR_DATATYPE *, MPI_Op, 
				      struct MPIR_COMMUNICATOR *));
static int intra_Reduce_scatter ANSI_ARGS((void*, void*, int *, 
					   struct MPIR_DATATYPE *, MPI_Op, 
					   struct MPIR_COMMUNICATOR *));
static int intra_Scan ANSI_ARGS((void* sendbuf, void* recvbuf, int count, 
				 struct MPIR_DATATYPE * datatype, 
				 MPI_Op op, struct MPIR_COMMUNICATOR *comm ));

/* I don't really want to to this this way, but for now... */
static struct _MPIR_COLLOPS intra_collops =  {
#ifdef MPID_Barrier
    MPID_FN_Barrier,
#else
    intra_Barrier,
#endif
#ifdef MPID_Bcast
    MPID_FN_Bcast,
#else
    intra_Bcast,
#endif
    intra_Gather, 
    intra_Gatherv, 
    intra_Scatter,
    intra_Scatterv,
#ifdef MPID_Allgather
    MPID_FN_Allgather,
#else
    intra_Allgather,
#endif
#ifdef MPID_Allgatherv
    MPID_FN_Allgatherv,
#else
    intra_Allgatherv,
#endif
    intra_Alltoall,
    intra_Alltoallv,
#ifdef MPID_Reduce
    MPID_FN_Reduce,
#else
    intra_Reduce,
#endif
#ifdef MPID_Allreduce
    MPID_FN_Allreduce,
#else
    intra_Allreduce,
#endif
#ifdef MPID_Reduce_scatter
    MPID_FN_Reduce_scatter,
#else
    intra_Reduce_scatter,
#endif
#ifdef FOO
#ifdef MPID_Reduce_scatterv
    MPID_FN_Reduce_scatterv,
#else
    intra_Reduce_scatterv,
#endif
#endif
    intra_Scan,
    1                              /* Giving it a refcount of 1 ensures it
				    * won't ever get freed.
				    */
};

MPIR_COLLOPS MPIR_intra_collops = &intra_collops;

/* Now the functions */
static int intra_Barrier ( comm )
struct MPIR_COMMUNICATOR *comm;
{
  int        rank, size, N2_prev, surfeit;
  int        d, dst, src;
  MPI_Status status;

  /* Intialize communicator size */
  (void) MPIR_Comm_size ( comm, &size );

#ifdef MPID_Barrier
  if (comm->ADIBarrier) {
      MPID_Barrier( comm->ADIctx, comm );
      return MPI_SUCCESS;
      }
#endif
  /* If there's only one member, this is trivial */
  if ( size > 1 ) {

    /* Initialize collective communicator */
    comm = comm->comm_coll;
    (void) MPIR_Comm_rank ( comm, &rank );
    (void) MPIR_Comm_N2_prev ( comm, &N2_prev );
    surfeit = size - N2_prev;

    /* Lock for collective operation */
    MPID_THREAD_LOCK(comm->ADIctx,comm);

    /* Perform a combine-like operation */
    if ( rank < N2_prev ) {
      if( rank < surfeit ) {

        /* get the fanin letter from the upper "half" process: */
        dst = N2_prev + rank;

        MPI_Recv((void *)0,0,MPI_INT,dst,MPIR_BARRIER_TAG, comm->self, &status);
      }

      /* combine on embedded N2_prev power-of-two processes */
      for (d = 1; d < N2_prev; d <<= 1) {
        dst = (rank ^ d);

        MPI_Sendrecv( (void *)0,0,MPI_INT,dst, MPIR_BARRIER_TAG,
                     (void *)0,0,MPI_INT,dst, MPIR_BARRIER_TAG, 
                     comm->self, &status);
      }

      /* fanout data to nodes above N2_prev... */
      if ( rank < surfeit ) {
        dst = N2_prev + rank;
        MPI_Send( (void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG, comm->self);
      }
    } 
    else {
      /* fanin data to power of 2 subset */
      src = rank - N2_prev;
      MPI_Sendrecv( (void *)0, 0, MPI_INT, src, MPIR_BARRIER_TAG,
                   (void *)0, 0, MPI_INT, src, MPIR_BARRIER_TAG, 
                   comm->self, &status);
    }

    /* Unlock for collective operation */
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  } 
  return(MPI_SUCCESS); 
}

static int intra_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
struct MPIR_DATATYPE *      datatype;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  MPI_Status status;
  int        rank, size, src, dst;
  int        n, N, surfeit, N2_prev, N2_next, N_rank;
  int        participants, my_block, my_offset;
  int        mpi_errno = MPI_SUCCESS;
  int        bsize;
  int        int_n;

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* Is root within the comm and more than 1 processes involved? */
  MPIR_Comm_size ( comm, &size );
  if ( (root >= size)  && (MPIR_ERROR_PUSH_ARG(&root),mpi_errno = MPI_ERR_ROOT) )
    return MPIR_ERROR( comm, mpi_errno, "MPI_BCAST" );
  
  /* If there is only one process */
  if (size == 1)
	return (mpi_errno);

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  /* Determine number of blocks and previous power of 2 for number of blocks */
  bsize = (size+MPIR_BCAST_BLOCK_SIZE-1)/MPIR_BCAST_BLOCK_SIZE;
  MPIR_Powers_of_2 ( bsize, &N2_next, &N2_prev );
  participants = MPIR_MIN(size, N2_prev * MPIR_BCAST_BLOCK_SIZE);
  N = N2_prev;

  /* Shift ranks, so root is at virtual node 0 */
  if ( rank < root )
    n = size - root + rank;
  else
    n = rank - root;

  /* How many "extra" nodes do we have? */
  surfeit = size - participants;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If I'm in a participating block */
  if ( n < participants ) {

    /* Which block am I in? and where in the block am I */
    my_block  = n / MPIR_BCAST_BLOCK_SIZE;
    my_offset = n % MPIR_BCAST_BLOCK_SIZE;

    /* If I'm the first node in a participating block, perform a bcast */
    /* using an inter-block binary algorithm and then use a linear */
    /* algorithm to bcast within the block */
    if ( my_offset == 0 ) {
      int num_in_block, i;
	  MPI_Request      hd[MPIR_BCAST_BLOCK_SIZE];
	  MPI_Status       statuses[MPIR_BCAST_BLOCK_SIZE];

      /* Set number of nodes in my block */
      if ((surfeit == 0) && ((my_block+1) == N2_prev) )
		num_in_block = participants - n;
      else
		num_in_block = MPIR_BCAST_BLOCK_SIZE;

	  int_n = n;

      /* While there's someone to send to or someone to receive from ... */
      /* The MPI_Send to the subtree should be non-blocking in order to
	 allow several to happen simultaneously (and not to be
	 blocked by a slow destination early in the delivery) 

	 In addition, the sends should allow pipelining for large messages,
	 so that the maximum bandwidth of the network can be used.
	 */
      while ( N > 1 ) {
		
		/* Determine the real rank of first node of middle block */
		N    >>= 1;
		N_rank = N * MPIR_BCAST_BLOCK_SIZE;
		
		/* If I'm the "root" of some processes, then send */
		if ( int_n == 0 ) {
		  dst = (rank + N_rank) % size;
		  mpi_errno = MPI_Send (buffer,count,datatype->self,dst,
					MPIR_BCAST_TAG,comm->self);
		  if (mpi_errno) return mpi_errno;
		}
		/* If a root is sending me a message, then recv and become a 
		   "root" */
		else if ( int_n == N_rank ) {
		  src = (rank - N_rank + size) % size;
		  mpi_errno = MPI_Recv(buffer,count,datatype->self,src,
				       MPIR_BCAST_TAG,comm->self,&status);
		  if (mpi_errno) return mpi_errno;
		  int_n   = 0;
		}
		/* I now have a new root to receive from */
		else if ( int_n > N_rank ) {
		  int_n -= N_rank;
		}
		/* else if ( n < N_rank ), do nothing, my root 
		   stayed the same */
	  }
	  
      /* Now linearly broadcast to the other members of my block */
      if (num_in_block > 1) {
	  for ( i = 1; i < num_in_block; i++ ) {
	      dst = (rank + i) % size;
	      mpi_errno = MPI_Isend(buffer,count,datatype->self,dst,
				    MPIR_BCAST_TAG,comm->self, &hd[i]);
	      if (mpi_errno) return mpi_errno;
	      }
	  mpi_errno = MPI_Waitall(num_in_block-1, &hd[1], &statuses[1]);
	  if (mpi_errno) return mpi_errno;
	  }
    }
    /* else I'm in a participating block, wait for a message from the first */
    /* node in my block */
    else {
      src = (rank + size - my_offset) % size;
      mpi_errno = MPI_Recv(buffer,count,datatype->self,src,
			   MPIR_BCAST_TAG,comm->self,&status);
      if (mpi_errno) return mpi_errno;
    }
	
    /* Now send to any "extra" nodes */
    if ( n < surfeit ) {
      dst = ( rank + participants ) % size;
      mpi_errno = MPI_Send( buffer, count, datatype->self, dst, 
			   MPIR_BCAST_TAG, comm->self );
      if (mpi_errno) return mpi_errno;
    }
  }
  /* else I'm not in a participating block, I'm an "extra" node */
  else {
    src = ( root + n - participants ) % size;
    mpi_errno = MPI_Recv ( buffer, count, datatype->self, src, 
			  MPIR_BCAST_TAG, comm->self, &status );
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

static int intra_Gather ( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, 
		 root, comm )
void             *sendbuf;
int               sendcnt;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcount;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Aint   extent;            /* Datatype extent */

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
  if ( (root >= size || root < 0) && 
       (MPIR_ERROR_PUSH_ARG(&root),mpi_errno = MPI_ERR_ROOT) )
    return MPIR_ERROR( comm, mpi_errno, "MPI_GATHER" );

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If rank == root, then I recv lots, otherwise I send */
  /* This should use the same mechanism used in reduce; the intermediate nodes
     will need to allocate space. 
   */
  if ( rank == root ) {
    int         i;
    MPI_Request req;
    MPI_Status  status;

    /* This should really be COPYSELF.... , with the for look skipping
       root. */
    mpi_errno = MPI_Isend(sendbuf, sendcnt, sendtype->self, root, 
			  MPIR_GATHER_TAG, comm->self, &req);
    if (mpi_errno) return mpi_errno;
    MPI_Type_extent(recvtype->self, &extent);
    for ( i=0; i<size; i++ ) {
	mpi_errno = MPI_Recv( (void *)(((char*)recvbuf)+i*extent*recvcount), 
			     recvcount, recvtype->self, i, 
			     MPIR_GATHER_TAG, comm->self, &status);
	if (mpi_errno) return mpi_errno;
    }
    mpi_errno = MPI_Wait(&req, &status);
  }
  else 
      mpi_errno = MPI_Send(sendbuf, sendcnt, sendtype->self, root, 
			   MPIR_GATHER_TAG, comm->self);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

static int intra_Gatherv ( sendbuf, sendcnt,  sendtype, 
                  recvbuf, recvcnts, displs, recvtype, 
                  root, comm )
void             *sendbuf;
int               sendcnt;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *displs;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
  if ( (root >= size) || (root < 0) ) {
      MPIR_ERROR_PUSH_ARG(&root);
      return MPIR_ERROR( comm, MPI_ERR_ROOT, "MPI_GATHERV" );
  }

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If rank == root, then I recv lots, otherwise I send */
  if ( rank == root ) {
      MPI_Aint       extent;
      int            i;
	MPI_Request req;
	MPI_Status       status;

    mpi_errno = MPI_Isend(sendbuf, sendcnt, sendtype->self, root, 
			  MPIR_GATHERV_TAG, comm->self, &req);
      if (mpi_errno) return mpi_errno;
    MPI_Type_extent(recvtype->self, &extent);
    for ( i=0; i<size; i++ ) {
	mpi_errno = MPI_Recv( (void *)((char *)recvbuf+displs[i]*extent), 
			     recvcnts[i], recvtype->self, i,
			     MPIR_GATHERV_TAG, comm->self, &status );
	if (mpi_errno) return mpi_errno;
    }
      mpi_errno = MPI_Wait(&req, &status);
  }
  else 
      mpi_errno = MPI_Send( sendbuf, sendcnt, sendtype->self, root, 
			   MPIR_GATHERV_TAG, comm->self );

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

static int intra_Scatter ( sendbuf, sendcnt, sendtype, 
		  recvbuf, recvcnt, recvtype, 
		  root, comm )
void             *sendbuf;
int               sendcnt;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcnt;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  MPI_Status status;
  MPI_Aint   extent;
  int        rank, size, i;
  int        mpi_errno = MPI_SUCCESS;
  

  /* Get size and rank */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Check for invalid arguments */
  if ( ( (root            <  0)           && (mpi_errno = MPI_ERR_ROOT) )   || 
       ( (root            >= size)        && (mpi_errno = MPI_ERR_ROOT) )) {
      MPIR_ERROR_PUSH_ARG(&root);
      return MPIR_ERROR( comm, mpi_errno, "MPI_SCATTER" ); 
  }
 
  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

  /* Get the size of the send type */
  MPI_Type_extent ( sendtype->self, &extent );

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If I'm the root, send messages to the rest of 'em */
  if ( rank == root ) {
    for ( i=0; i<root; i++ ) {
	mpi_errno = MPI_Send( (void *)((char *)sendbuf+i*sendcnt*extent), 
			     sendcnt, sendtype->self, i, MPIR_SCATTER_TAG, comm->self);
	if (mpi_errno) return mpi_errno;
	}

    mpi_errno = MPI_Sendrecv ( (void *)((char *)sendbuf+rank*sendcnt*extent),
			      sendcnt,sendtype->self, rank, MPIR_SCATTER_TAG,
			       recvbuf, recvcnt, recvtype->self, rank, 
			       MPIR_SCATTER_TAG, 
			      comm->self, &status );
    if (mpi_errno) return mpi_errno;

    for ( i=root+1; i<size; i++ ) {
	mpi_errno = MPI_Send( (void *)((char *)sendbuf+i*sendcnt*extent), 
			      sendcnt, sendtype->self, i, 
			      MPIR_SCATTER_TAG, comm->self);
	if (mpi_errno) return mpi_errno;
	}
  }
  else 
      mpi_errno = MPI_Recv(recvbuf,recvcnt,recvtype->self,root,
			   MPIR_SCATTER_TAG,comm->self,&status);
  
  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (mpi_errno);
}

static int intra_Scatterv ( sendbuf, sendcnts, displs, sendtype, 
                   recvbuf, recvcnt,  recvtype, 
                   root, comm )
void             *sendbuf;
int              *sendcnts;
int              *displs;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcnt;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  MPI_Status status;
  int        rank, size;
  int        mpi_errno = MPI_SUCCESS;

  /* Get size and rank */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Check for invalid arguments */
  if ( ( (root            <  0)           && (mpi_errno = MPI_ERR_ROOT) )  || 
       ( (root            >= size)        && (mpi_errno = MPI_ERR_ROOT) )) {
      MPIR_ERROR_PUSH_ARG(&root);
      return MPIR_ERROR( comm, mpi_errno, "MPI_SCATTERV" );
  }

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* If I'm the root, then scatter */
  if ( rank == root ) {
    MPI_Aint extent;
    int      i;

    MPI_Type_extent(sendtype->self, &extent);
    /* We could use Isend here, but since the receivers need to execute
       a simple Recv, it may not make much difference in performance, 
       and using the blocking version is simpler */
    for ( i=0; i<root; i++ ) {
      mpi_errno = MPI_Send( (void *)((char *)sendbuf+displs[i]*extent), 
			   sendcnts[i], sendtype->self, i, MPIR_SCATTERV_TAG, comm->self);
      if (mpi_errno) return mpi_errno;
      }
    mpi_errno = MPI_Sendrecv((void *)((char *)sendbuf+displs[rank]*extent), 
		 sendcnts[rank], 
                 sendtype->self, rank, MPIR_SCATTERV_TAG, 
			     recvbuf, recvcnt, recvtype->self, 
                 rank, MPIR_SCATTERV_TAG, comm->self, &status);
    if (mpi_errno) return mpi_errno;

    for ( i=root+1; i<size; i++ ) {
      mpi_errno = MPI_Send( (void *)((char *)sendbuf+displs[i]*extent), 
			   sendcnts[i], sendtype->self, i, 
			    MPIR_SCATTERV_TAG, comm->self);
      if (mpi_errno) return mpi_errno;
      }
  }
  else
      mpi_errno = MPI_Recv(recvbuf,recvcnt,recvtype->self,root,
			   MPIR_SCATTERV_TAG,comm->self,&status);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


static int intra_Allgather ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcount;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *         comm;
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  MPI_Aint   recv_extent;
  int        j, jnext, i, right, left;

  /* Get the size of the communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Do a gather for each process in the communicator
     This is the "circular" algorithm for allgather - each process sends to
     its right and receives from its left.  This is faster than simply
     doing size Gathers.
   */

  MPI_Type_extent ( recvtype->self, &recv_extent );

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;
 
  /* First, load the "local" version in the recvbuf. */
  mpi_errno = 
      MPI_Sendrecv( sendbuf, sendcount, sendtype->self, rank, 
		    MPIR_ALLGATHER_TAG,
                    (void *)((char *)recvbuf + rank*recvcount*recv_extent),
		    recvcount, recvtype->self, rank, MPIR_ALLGATHER_TAG, 
		    comm->self,
		    &status );
  if (mpi_errno) return mpi_errno;

  /* 
     Now, send left to right.  This fills in the receive area in 
     reverse order.
   */
  left  = (size + rank - 1) % size;
  right = (rank + 1) % size;
  
  j     = rank;
  jnext = left;
  for (i=1; i<size; i++) {
      mpi_errno = 
	  MPI_Sendrecv( (void *)((char *)recvbuf+j*recvcount*recv_extent),
		    recvcount, recvtype->self, right, MPIR_ALLGATHER_TAG,
                    (void *)((char *)recvbuf + jnext*recvcount*recv_extent),
		    recvcount, recvtype->self, left, 
			MPIR_ALLGATHER_TAG, comm->self,
		    &status );
      if (mpi_errno) break;
      j	    = jnext;
      jnext = (size + jnext - 1) % size;
      }
  return (mpi_errno);
}


static int intra_Allgatherv ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcounts, displs,   recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int              *recvcounts;
int              *displs;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *         comm;
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  MPI_Aint   recv_extent;
  int        j, jnext, i, right, left;

  /* Get the size of the communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

  /* Do a gather for each process in the communicator
     This is the "circular" algorithm for allgatherv - each process sends to
     its right and receives from its left.  This is faster than simply
     doing size Gathervs.
   */

  MPI_Type_extent ( recvtype->self, &recv_extent );

  /* First, load the "local" version in the recvbuf. */
  mpi_errno = 
      MPI_Sendrecv( sendbuf, sendcount, sendtype->self, rank, 
		    MPIR_ALLGATHERV_TAG,
                    (void *)((char *)recvbuf + displs[rank]*recv_extent),
		    recvcounts[rank], recvtype->self, rank, 
		    MPIR_ALLGATHERV_TAG, 
		    comm->self, &status );
  if (mpi_errno) {
      return mpi_errno;
  }

  left  = (size + rank - 1) % size;
  right = (rank + 1) % size;
  
  j     = rank;
  jnext = left;
  for (i=1; i<size; i++) {
      mpi_errno = 
	  MPI_Sendrecv( (void *)((char *)recvbuf+displs[j]*recv_extent),
		    recvcounts[j], recvtype->self, right, MPIR_ALLGATHERV_TAG,
                    (void *)((char *)recvbuf + displs[jnext]*recv_extent),
		    recvcounts[jnext], recvtype->self, left, 
		       MPIR_ALLGATHERV_TAG, comm->self,
		    &status );
      if (mpi_errno) break;
      j	    = jnext;
      jnext = (size + jnext - 1) % size;
      }
  return (mpi_errno);
}

static int intra_Alltoall( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcnt;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *         comm;
{
  int          size, rank, i;
  MPI_Aint     send_extent, recv_extent;
  int          mpi_errno = MPI_SUCCESS;
  MPI_Status  *starray;
  MPI_Request *reqarray;

  /* Get size and rank and switch to collective communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  /* Get extent of send and recv types */
  MPI_Type_extent ( sendtype->self, &send_extent );
  MPI_Type_extent ( recvtype->self, &recv_extent );

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx, comm);

/* 
 */
  /* 1st, get some storage from the heap to hold handles, etc. */
  MPIR_ALLOC(starray,(MPI_Status *)MALLOC(2*size*sizeof(MPI_Status)),
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALL" );

  MPIR_ALLOC(reqarray, (MPI_Request *)MALLOC(2*size*sizeof(MPI_Request)),
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALL" );

  /* do the communication -- post *all* sends and receives: */
  for ( i=0; i<size; i++ ) { 
      /* We'd like to avoid sending and receiving to ourselves; 
	 however, this is complicated by the presence of different
	 sendtype and recvtypes. */
      if ( (mpi_errno=MPI_Irecv(
	  (void *)((char *)recvbuf + i*recvcnt*recv_extent),
                           recvcnt,
                           recvtype->self,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm->self,
                           &reqarray[2*i+1]))
          )
          break;
      if ((mpi_errno=MPI_Isend(
	  (void *)((char *)sendbuf+i*sendcount*send_extent),
                           sendcount,
                           sendtype->self,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm->self,
                           &reqarray[2*i]))
          )
          break;
  }
  
  if (mpi_errno) return mpi_errno;

  /* ... then wait for *all* of them to finish: */
  mpi_errno = MPI_Waitall(2*size,reqarray,starray);
  
  /* clean up */
  FREE(starray);
  FREE(reqarray);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

static int intra_Alltoallv ( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm )
void             *sendbuf;
int              *sendcnts;
int              *sdispls;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *rdispls; 
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *         comm;
{
  int        size, rank, i, j, rcnt;
  MPI_Aint   send_extent, recv_extent;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status  *starray;
  MPI_Request *reqarray;
  
  /* Get size and rank and switch to collective communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Get extent of send and recv types */
  MPI_Type_extent(sendtype->self, &send_extent);
  MPI_Type_extent(recvtype->self, &recv_extent);

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* 1st, get some storage from the heap to hold handles, etc. */
  MPIR_ALLOC(starray,(MPI_Status *)MALLOC(2*size*sizeof(MPI_Status)),
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALL" );

  MPIR_ALLOC(reqarray,(MPI_Request *)MALLOC(2*size*sizeof(MPI_Request)),
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALL" );

  /* do the communication -- post *all* sends and receives: */
  rcnt = 0;
  for ( i=0; i<size; i++ ) { 
      reqarray[2*i] = MPI_REQUEST_NULL;
      if (( mpi_errno=MPI_Irecv(
	                  (void *)((char *)recvbuf+rdispls[i]*recv_extent), 
                           recvcnts[i], 
                           recvtype->self,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm->self,
                           &reqarray[2*i+1]))
          )
          break;
      rcnt++;
      if (( mpi_errno=MPI_Isend(
	                   (void *)((char *)sendbuf+sdispls[i]*send_extent), 
                           sendcnts[i], 
                           sendtype->self,
                           i,
                           MPIR_ALLTOALL_TAG,
                           comm->self,
                           &reqarray[2*i]))
          )
          break;
      rcnt++;
  }
  
  /* ... then wait for *all* of them to finish: */
  if (mpi_errno) {
      /* We should really cancel all of the active requests */
      for (j=0; j<rcnt; j++) {
	  MPI_Cancel( &reqarray[j] );
      }
  }
  else {
      mpi_errno = MPI_Waitall(2*size,reqarray,starray);
  }
  
  /* clean up */
  FREE(reqarray);
  FREE(starray);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}

static int intra_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *datatype;
MPI_Op            op;
int               root;
struct MPIR_COMMUNICATOR *         comm;
{
  MPI_Status status;
  int        size, rank;
  int        mask, relrank, source, lroot;
  int        mpi_errno = MPI_SUCCESS;
  MPI_User_function *uop;
  MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
  void       *buffer;
  struct MPIR_OP *op_ptr;
  static char myname[] = "MPI_REDUCE";
  MPIR_ERROR_DECL;
  mpi_comm_err_ret = 0;

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
  if ( ((root >= size) || (root < 0)) ) {
      MPIR_ERROR_PUSH_ARG(&root);
      return MPIR_ERROR(comm, MPI_ERR_ROOT, myname );
  }

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* If the operation is predefined, we could check that the datatype's
     type signature is compatible with the operation.  
   */
#ifdef MPID_Reduce
  /* Eventually, this could apply the MPID_Reduce routine in a loop for
     counts > 1 */
  if (comm->ADIReduce && count == 1) {
      /* Call a routine to sort through the datatypes and operations ...
	 This allows us to provide partial support (e.g., only SUM_DOUBLE)
       */
      if (MPIR_ADIReduce( comm->ADIctx, comm, sendbuf, recvbuf, count, 
                      datatype->self, op, root ) == MPI_SUCCESS)
	  return MPI_SUCCESS;
      }
#endif

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  op_ptr = MPIR_GET_OP_PTR(op);
  MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
  uop  = op_ptr->op;


  /* Here's the algorithm.  Relative to the root, look at the bit pattern in 
     my rank.  Starting from the right (lsb), if the bit is 1, send to 
     the node with that bit zero and exit; if the bit is 0, receive from the
     node with that bit set and combine (as long as that node is within the
     group)

     Note that by receiving with source selection, we guarentee that we get
     the same bits with the same input.  If we allowed the parent to receive 
     the children in any order, then timing differences could cause different
     results (roundoff error, over/underflows in some cases, etc).

     Because of the way these are ordered, if root is 0, then this is correct
     for both commutative and non-commutitive operations.  If root is not
     0, then for non-commutitive, we use a root of zero and then send
     the result to the root.  To see this, note that the ordering is
     mask = 1: (ab)(cd)(ef)(gh)            (odds send to evens)
     mask = 2: ((ab)(cd))((ef)(gh))        (3,6 send to 0,4)
     mask = 4: (((ab)(cd))((ef)(gh)))      (4 sends to 0)

     Comments on buffering.  
     If the datatype is not contiguous, we still need to pass contiguous 
     data to the user routine.  
     In this case, we should make a copy of the data in some format, 
     and send/operate on that.

     In general, we can't use MPI_PACK, because the alignment of that
     is rather vague, and the data may not be re-usable.  What we actually
     need is a "squeeze" operation that removes the skips.
   */
  /* Make a temporary buffer */
  MPIR_Type_get_limits( datatype, &lb, &ub );
  m_extent = ub - lb;
  /* MPI_Type_extent ( datatype, &extent ); */
  MPIR_ALLOC(buffer,(void *)MALLOC(m_extent * count),comm, MPI_ERR_EXHAUSTED, 
	     "Out of space in MPI_REDUCE" );
  buffer = (void *)((char*)buffer - lb);

  /* If I'm not the root, then my recvbuf may not be valid, therefore
     I have to allocate a temporary one */
  if (rank != root) {
      MPIR_ALLOC(recvbuf,(void *)MALLOC(m_extent * count),
		 comm, MPI_ERR_EXHAUSTED, 
                        "Out of space in MPI_REDUCE" );
      recvbuf = (void *)((char*)recvbuf - lb);
  }

  /* This code isn't correct if the source is a more complex datatype */
  memcpy( recvbuf, sendbuf, m_extent*count );
  mask    = 0x1;
  if (op_ptr->commute) lroot   = root;
  else                 lroot   = 0;
  relrank = (rank - lroot + size) % size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);
  
  MPIR_Op_errno = MPI_SUCCESS;
  while (/*(mask & relrank) == 0 && */mask < size) {
	/* Receive */
	if ((mask & relrank) == 0) {
	    source = (relrank | mask);
	    if (source < size) {
		source = (source + lroot) % size;
		mpi_errno = MPI_Recv (buffer, count, datatype->self, source, 
				      MPIR_REDUCE_TAG, comm->self, &status);
		if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname ); 
		/* The sender is above us, so the received buffer must be
		   the second argument (in the noncommutitive case). */
		/* error pop/push allows errors found by predefined routines
		   to be visible.  We need a better way to do this */
		/* MPIR_ERROR_POP(comm); */
		if (op_ptr->commute)
		    (*uop)(buffer, recvbuf, &count, &datatype->self);
		else {
		    (*uop)(recvbuf, buffer, &count, &datatype->self);
		    /* short term hack to keep recvbuf up-to-date */
		    memcpy( recvbuf, buffer, m_extent*count );
		    }
		/* MPIR_ERROR_PUSH(comm); */
		}
	    }
	else {
	    /* I've received all that I'm going to.  Send my result to 
	       my parent */
	    source = ((relrank & (~ mask)) + lroot) % size;
	    mpi_errno  = MPI_Send( recvbuf, count, datatype->self, 
				  source, 
				  MPIR_REDUCE_TAG, 
				  comm->self );
	    if (mpi_errno) return MPIR_ERROR( comm, mpi_errno, myname );
	    break;
	    }
	mask <<= 1;
	}
  FREE( (char *)buffer + lb );
  if (!op_ptr->commute && root != 0) {
      if (rank == 0) {
	  mpi_errno  = MPI_Send( recvbuf, count, datatype->self, root, 
				MPIR_REDUCE_TAG, comm->self );
	  }
      else if (rank == root) {
	  mpi_errno = MPI_Recv ( recvbuf, count, datatype->self, 0, /*size-1, */
				MPIR_REDUCE_TAG, comm->self, &status);
	  }
      }

  /* Free the temporarily allocated recvbuf */
  if (rank != root)
    FREE( (char *)recvbuf + lb );

  /* If the predefined operation detected an error, report it here */
  /* Note that only the root gets this result, so this can cause
     programs to hang, particularly if this is used to implement 
     MPI_Allreduce.  Use care with this.
   */
  if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno) {
      /* printf( "Error in performing MPI_Op in reduce\n" ); */
      mpi_errno = MPIR_Op_errno;
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (mpi_errno);
}

static int intra_Allreduce ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *         comm;
{
  int mpi_errno, rc;

  /* Reduce to 0, then bcast */
  mpi_errno = MPI_Reduce ( sendbuf, recvbuf, count, datatype->self, op, 0, 
			   comm->self );
  if (mpi_errno == MPI_ERR_OP_NOT_DEFINED || mpi_errno == MPI_SUCCESS) {
      rc = MPI_Bcast  ( recvbuf, count, datatype->self, 0, comm->self );
      if (rc) mpi_errno = rc;
  }

  return (mpi_errno);
}

static int intra_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *         comm;
{
  int   rank, size, i, count=0;
  MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
  int  *displs;
  void *buffer;
  int   mpi_errno = MPI_SUCCESS, rc;

  /* Determine the "count" of items to reduce and set the displacements*/
  MPIR_Type_get_limits( datatype, &lb, &ub );
  m_extent = ub - lb;
  /* MPI_Type_extent (datatype, &extent); */
  MPIR_Comm_size   (comm, &size);
  MPIR_Comm_rank   (comm, &rank);

  /* Allocate the displacements and initialize them */
  MPIR_ALLOC(displs,(int *)MALLOC(size*sizeof(int)),comm, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_REDUCE_SCATTER" );
  for (i=0;i<size;i++) {
    displs[i] = count;
    count += recvcnts[i];
    if (recvcnts[i] < 0) {
	FREE( displs );
	MPIR_ERROR_PUSH_ARG(&i);
	MPIR_ERROR_PUSH_ARG(&recvcnts[i]);
	return MPI_ERR_COUNT_ARRAY_NEG;
    }
  }

  /* Allocate a temporary buffer */
  if (count == 0) {
      FREE( displs );
      return MPI_SUCCESS;
      }

  MPIR_ALLOC(buffer,(void *)MALLOC(m_extent*count), comm, MPI_ERR_EXHAUSTED, 
			 "Out of space in MPI_REDUCE_SCATTER" );
  buffer = (void *)((char*)buffer - lb);

  /* Reduce to 0, then scatter */
  mpi_errno = MPI_Reduce   ( sendbuf, buffer, count, datatype->self, op, 0, 
			     comm->self);
  if (mpi_errno == MPI_SUCCESS || mpi_errno == MPI_ERR_OP_NOT_DEFINED) {
      rc = MPI_Scatterv ( buffer, recvcnts, displs, datatype->self,
			  recvbuf, recvcnts[rank], datatype->self, 0,
			  comm->self );
      if (rc) mpi_errno = rc;
  }
  /* Free the temporary buffers */
  FREE((char *)buffer+lb); FREE(displs);
  return (mpi_errno);
}

static int intra_Scan ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *         comm;
{
  MPI_Status status;
  int        rank, size;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Aint   lb, ub, m_extent;  /* Extent in memory */
  MPI_User_function   *uop;
  struct MPIR_OP *op_ptr;
  MPIR_ERROR_DECL;
  mpi_comm_err_ret = 0;

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* Get my rank & size and switch communicators to the hidden collective */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );
  MPIR_Type_get_limits( datatype, &lb, &ub );
  m_extent = ub - lb;
  comm	   = comm->comm_coll;
  op_ptr = MPIR_GET_OP_PTR(op);
  MPIR_TEST_MPI_OP(op,op_ptr,comm,"MPI_SCAN");
  uop	   = op_ptr->op;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* 
     This is an O(size) algorithm.  A modification of the algorithm in 
     reduce.c can be used to make this O(log(size)) 
   */
  /* commutative case requires no extra buffering */
  MPIR_Op_errno = MPI_SUCCESS;
  if (op_ptr->commute) {
      /* Do the scan operation */
      if (rank > 0) {
          mpi_errno = MPI_Recv(recvbuf,count,datatype->self,rank-1,
			       MPIR_SCAN_TAG,comm->self,&status);
	  if (mpi_errno) return mpi_errno;
	  /* See reduce for why pop/push */
	  MPIR_ERROR_POP(comm);
          (*uop)(sendbuf, recvbuf, &count, &datatype->self); 
	  MPIR_ERROR_PUSH(comm);
      }
      else {
	  MPIR_COPYSELF( sendbuf, count, datatype->self, recvbuf, 
			 MPIR_SCAN_TAG, rank, comm->self );
	  if (mpi_errno) return mpi_errno;
      }
  }
  /* non-commutative case requires extra buffering */
  else {
      /* Do the scan operation */
      if (rank > 0) {
          void *tmpbuf;
          MPIR_ALLOC(tmpbuf,(void *)MALLOC(m_extent * count),
		     comm, MPI_ERR_EXHAUSTED, "Out of space in MPI_SCAN" );
	  tmpbuf = (void *)((char*)tmpbuf-lb);
	  MPIR_COPYSELF( sendbuf, count, datatype->self, recvbuf, 
			 MPIR_SCAN_TAG, rank, comm->self );
	  if (mpi_errno) return mpi_errno;
          mpi_errno = MPI_Recv(tmpbuf,count,datatype->self,rank-1,
			       MPIR_SCAN_TAG,comm->self,&status);
	  if (mpi_errno) return mpi_errno;
          (*uop)(tmpbuf, recvbuf, &count, &datatype->self); 
          FREE((char*)tmpbuf+lb);
      }
      else {
	  MPIR_COPYSELF( sendbuf, count, datatype->self, recvbuf, 
			 MPIR_SCAN_TAG, rank, comm->self );
	  if (mpi_errno) return mpi_errno;
	  }
  }

  /* send the letter to destination */
  if (rank < (size-1)) 
      mpi_errno = MPI_Send(recvbuf,count,datatype->self,rank+1,MPIR_SCAN_TAG,
			   comm->self);

  /* If the predefined operation detected an error, report it here */
  if (mpi_errno == MPI_SUCCESS && MPIR_Op_errno)
      mpi_errno = MPIR_Op_errno;

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return(mpi_errno);
}
