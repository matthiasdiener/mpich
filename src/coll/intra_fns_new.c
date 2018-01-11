/*
 *  $Id: intra_fns_new.c,v 1.21 2003/02/10 21:32:37 thakur Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpimem.h"
/* pt2pt for MPIR_Type_get_limits */
#include "mpipt2pt.h"
#include "coll.h"
#include "mpiops.h"

/* threshold to switch between long and short vector algorithms */
#define MPIR_BCAST_SHORT_MSG 12288
#define MPIR_BCAST_MIN_PROCS 8
#define MPIR_ALLTOALL_SHORT_MSG 128
#define MPIR_ALLTOALL_MEDIUM_MSG 262144
#define MPIR_ALLGATHERV_MEDIUM_MSG 262144

#define MPIR_REDUCE_SCATTER_SHORT_MSG 512
/* On the NCSA cluster, this value was right for 8, 16, and 32
   processes. For 64 processes it was 1500 bytes; for 128 processes it
   was 15 KB; and for 256 processes it was 50 KB. Need to come up with a
   formula that takes into account the number of proesses */


#define MPIR_ERR_OP_NOT_DEFINED MPIR_ERRCLASS_TO_CODE(MPI_ERR_OP,MPIR_ERR_NOT_DEFINED)

/* These should *always* use the PMPI versions of the functions to ensure that
   only any user code to catch an MPI function only uses the PMPI functions.

   Here is a partial solution.  In the weak symbol case, we simply change all 
   of the routines to their PMPI versions. 
 */
#ifdef HAVE_WEAK_SYMBOLS
/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"
#endif

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
static int intra_Barrier (struct MPIR_COMMUNICATOR *comm );
static int intra_Bcast (void* buffer, int count, 
			struct MPIR_DATATYPE * datatype, int root, 
			struct MPIR_COMMUNICATOR *comm );
static int intra_Gather (void*, int, 
			 struct MPIR_DATATYPE *, void*, 
			 int, struct MPIR_DATATYPE *, 
			 int, struct MPIR_COMMUNICATOR *); 
static int intra_Gatherv (void*, int, 
			  struct MPIR_DATATYPE *, 
			  void*, int *, 
			  int *, struct MPIR_DATATYPE *, 
			  int, struct MPIR_COMMUNICATOR *); 
static int intra_Scatter (void* sendbuf, int sendcount, 
			  struct MPIR_DATATYPE * sendtype, 
			  void* recvbuf, int recvcount, 
			  struct MPIR_DATATYPE * recvtype, 
			  int root, struct MPIR_COMMUNICATOR *comm);
static int intra_Scatterv (void*, int *, 
			   int *, struct MPIR_DATATYPE *, 
			   void*, int, 
			   struct MPIR_DATATYPE *, int, 
			   struct MPIR_COMMUNICATOR *);
static int intra_Allgather (void*, int, 
			    struct MPIR_DATATYPE *, 
			    void*, int, 
			    struct MPIR_DATATYPE *, 
			    struct MPIR_COMMUNICATOR *comm);
static int intra_Allgatherv (void*, int, struct MPIR_DATATYPE *, 
			     void*, int *, int *, 
			     struct MPIR_DATATYPE *, 
			     struct MPIR_COMMUNICATOR *);
static int intra_Alltoall (void*, int, struct MPIR_DATATYPE *, 
			   void*, int, struct MPIR_DATATYPE *, 
			   struct MPIR_COMMUNICATOR *);
static int intra_Alltoallv (void*, int *, int *, 
			    struct MPIR_DATATYPE *, void*, int *, 
			    int *, struct MPIR_DATATYPE *, 
			    struct MPIR_COMMUNICATOR *);
static int intra_Reduce (void*, void*, int, struct MPIR_DATATYPE *, 
			 MPI_Op, int, struct MPIR_COMMUNICATOR *);
static int intra_Allreduce (void*, void*, int, 
			    struct MPIR_DATATYPE *, MPI_Op, 
			    struct MPIR_COMMUNICATOR *);
static int intra_Reduce_scatter (void*, void*, int *, 
				 struct MPIR_DATATYPE *, MPI_Op, 
				 struct MPIR_COMMUNICATOR *);
#ifdef MPIR_USE_BASIC_COLL
static int intra_Scan (void* sendbuf, void* recvbuf, int count, 
		       struct MPIR_DATATYPE * datatype, 
		       MPI_Op op, struct MPIR_COMMUNICATOR *comm );
#endif

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
#ifdef MPID_Gather
    MPID_FN_Gather,
#else
    intra_Gather,
#endif
#ifdef MPID_Gatherv
    MPID_FN_Gatherv,
#else
    intra_Gatherv,
#endif
#ifdef MPID_Scatter
    MPID_FN_Scatter,
#else
    intra_Scatter,
#endif
#ifdef MPID_Scatterv
    MPID_FN_Scatterv,
#else
    intra_Scatterv,
#endif
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
#ifdef MPID_Alltoall
    MPID_FN_Alltoall,
#else
    intra_Alltoall,
#endif
#ifdef MPID_Alltoallv
    MPID_FN_Alltoallv,
#else
    intra_Alltoallv,
#endif
    0, /* Fix me! a dummy for alltoallw */
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
#ifdef MPIR_USE_BASIC_COLL
    intra_Scan,
#else
    /* This is in intra_scan.c and is now the default */
#ifdef MPID_Scan
    MPID_FN_Scan,
#else
    MPIR_intra_Scan,
#endif
#endif
    1                              /* Giving it a refcount of 1 ensures it
                                    * won't ever get freed.
                                    *                                     */
};

MPIR_COLLOPS MPIR_intra_collops = &intra_collops;

/* Now the functions */
static int intra_Barrier ( struct MPIR_COMMUNICATOR *comm )
{
  int        rank, size, N2_prev, surfeit;
  int        d, dst, src;
  MPI_Status status;

  /* Intialize communicator size */
  (void) MPIR_Comm_size ( comm, &size );

#if defined(MPID_Barrier)  &&  !defined(TOPOLOGY_INTRA_FNS_H)
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


/* This is the default implementation of broadcast. The algorithm is:
   
   Algorithm: MPI_Bcast

   For short messages, we use a minimum spanning tree (MST) algorithm. 

   Cost = lgp.alpha + n.lgp.beta

   For long messages, we do a scatter followed by an allgather. 
   We first scatter the buffer using an MST algorithm. This costs
   lgp.alpha + n.((p-1)/p).beta
   If the datatype is contiguous and the communicator is homogeneous,
   we treat the data as bytes and divide (scatter) it among processes
   by using ceiling division. For the noncontiguous or heterogeneous
   cases, we first pack the data into a temporary buffer by using
   MPI_Pack, scatter it as bytes, and unpack it after the allgather.

   For the allgather, we use a recursive doubling algorithm. This
   takes lgp steps. In each step pairs of processes exchange all the
   data they have (we take care of non-power-of-two situations). This
   costs approximately lgp.alpha + n.((p-1)/p).beta. (Approximately
   because it may be slightly more in the non-power-of-two case, but
   it's still a logarithmic algorithm.) Therefore, for long messages

   Total Cost = 2.lgp.alpha + 2.n.((p-1)/p).beta

   Note that this algorithm has twice the latency as the MST algorithm
   we use for short messages, but requires lower bandwidth: 2.n.beta
   versus n.lgp.beta. Therefore, for long messages and when lgp > 2,
   this algorithm will perform better.

   Possible improvements: 
   For clusters of SMPs, we may want to do something differently to
   take advantage of shared memory on each node.

   End Algorithm: MPI_Bcast
*/

static int intra_Bcast ( 
	void *buffer, 
	int count, 
	struct MPIR_DATATYPE *datatype, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
  MPI_Status status;
  int        rank, size, src, dst;
  int        relative_rank, mask, tmp_buf_size;
  int        mpi_errno = MPI_SUCCESS;
  int scatter_size, nbytes, curr_size, recv_size, send_size;
  int type_size, j, k, i, tmp_mask, is_contig, is_homogeneous;
  int relative_dst, dst_tree_root, my_tree_root, send_offset;
  int recv_offset, tree_root, nprocs_completed, offset, position;
  void *tmp_buf;
  static char myname[] = "MPI_BCAST";

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* Is root within the comm and more than 1 processes involved? */
  MPIR_Comm_size ( comm, &size );
#ifndef MPIR_NO_ERROR_CHECKING
  if (root >= size) 
      mpi_errno = MPIR_Err_setmsg( MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, 
				   myname, (char *)0, (char *)0, root, size );
  else if (root < 0) 
      /* This catches the use of MPI_ROOT in an intracomm broadcast */
      mpi_errno = MPIR_Err_setmsg( MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
				   (char *)0, (char *)0, root );
  if (mpi_errno)
      return MPIR_ERROR(comm, mpi_errno, myname );
#endif
  
  /* If there is only one process */
  if (size == 1)
	return (mpi_errno);

  MPIR_Datatype_iscontig(datatype->self, &is_contig);

  is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
  is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
  is_homogeneous = 0;   /* Globus */
#endif

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  if (is_contig && is_homogeneous) {
      /* contiguous and homogeneous */
      MPI_Type_size(datatype->self, &type_size);
      nbytes = type_size * count;
  }
  else {
      MPI_Pack_size(1, datatype->self, comm->self, &tmp_buf_size);
      /* calculate the value of nbytes, the size in packed
         representation of the buffer to be broadcasted. We can't
         simply multiply tmp_buf_size by count because tmp_buf_size
         is an upper bound on the amount of memory required. (For
         example, for a single integer, MPICH returns pack_size=12.)
         Therefore, we actually pack some data into tmp_buf, see by
         how much 'position' is incremented, and multiply that by count. */
      MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                 MPI_ERR_EXHAUSTED, "MPI_BCAST"); 
      position = 0;
      MPI_Pack(buffer, 1, datatype->self, tmp_buf, tmp_buf_size,
               &position, comm->self);
      nbytes = position * count;
      FREE(tmp_buf);
  }

  relative_rank = (rank >= root) ? rank - root : rank - root + size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  if ((nbytes < MPIR_BCAST_SHORT_MSG) && (size <= MPIR_BCAST_MIN_PROCS)) {

      /* Use short message algorithm, namely, minimum spanning tree */

  /* Algorithm:
     This uses a fairly basic recursive subdivision algorithm.
     The root sends to the process size/2 away; the receiver becomes
     a root for a subtree and applies the same process. 

     So that the new root can easily identify the size of its
     subtree, the (subtree) roots are all powers of two (relative to the root)
     If m = the first power of 2 such that 2^m >= the size of the
     communicator, then the subtree at root at 2^(m-k) has size 2^k
     (with special handling for subtrees that aren't a power of two in size).
     
     Optimizations:
     
     The original code attempted to switch to a linear broadcast when
     the subtree size became too small.  As a further variation, the subtree
     broadcast sent data to the center of the block, rather than to one end.
     However, the original code did not properly compute the communications,
     resulting in extraneous (though harmless) communication.    

     For very small messages, using a linear algorithm (process 0 sends to
     process 1, who sends to 2, etc.) can be better, since no one process
     takes more than 1 send/recv time, and successive bcasts using the same
     root can overlap.  

     Another important technique for long messages is pipelining---sending
     the messages in blocks so that the message can be pipelined through
     the network without waiting for the subtree roots to receive the entire
     message before forwarding it to other processors.  This is hard to
     do if the datatype/count are not the same on each processor (note that
     this is allowed - only the signatures must match).  Of course, this can
     be accomplished at the byte transfer level, but it is awkward 
     from the MPI point-to-point routines.

     Nonblocking operations can be used to achieve some "horizontal"
     pipelining (on some systems) by allowing multiple send/receives
     to begin on the same processor.
  */


  /* Do subdivision.  There are two phases:
     1. Wait for arrival of data.  Because of the power of two nature
        of the subtree roots, the source of this message is alwyas the
        process whose relative rank has the least significant 1 bit CLEARED.
        That is, process 4 (100) receives from process 0, process 7 (111) 
        from process 6 (110), etc.   
     2. Forward to my subtree

     Note that the process that is the tree root is handled automatically
     by this code, since it has no bits set.
     
   */

      mask = 0x1;
      while (mask < size) {
          if (relative_rank & mask) {
              src = rank - mask; 
              if (src < 0) src += size;
              mpi_errno = MPI_Recv(buffer,count,datatype->self,src,
                                   MPIR_BCAST_TAG,comm->self,&status);
              if (mpi_errno) return mpi_errno;
              break;
          }
          mask <<= 1;
      }

  /* This process is responsible for all processes that have bits set from
     the LSB upto (but not including) mask.  Because of the "not including",
     we start by shifting mask back down one.

     We can easily change to a different algorithm at any power of two
     by changing the test (mask > 1) to (mask > block_size) 

     One such version would use non-blocking operations for the last 2-4
     steps (this also bounds the number of MPI_Requests that would
     be needed).
   */

      mask >>= 1;
      while (mask > 0) {
          if (relative_rank + mask < size) {
              dst = rank + mask;
              if (dst >= size) dst -= size;
              mpi_errno = MPI_Send (buffer,count,datatype->self,dst,
                                    MPIR_BCAST_TAG,comm->self);
              if (mpi_errno) return mpi_errno;
          }
          mask >>= 1;
      }

  }

  else { 

/* use long message algorithm: MST scatter followed by an allgather */

/* Scatter algorithm divides the buffer into nprocs pieces and
   scatters them among the processes. Root gets the first piece,
   root+1 gets the second piece, and so forth. Uses the same minimum
   spanning tree (MST) algorithm as above. Ceiling division
   is used to compute the size of each piece. This means some
   processes may not get any data. For example if bufsize = 97 and
   nprocs = 16, ranks 15 and 16 will get 0 data. On each process, the
   scattered data is stored at the same offset in the buffer as it is
   on the root process. */ 

      if (is_contig && is_homogeneous) {
          /* contiguous and homogeneous. no need to pack. */
          tmp_buf = buffer;
      }
      else {
          /* noncontiguous or heterogeneous. pack into temporary buffer. */

          MPIR_ALLOC(tmp_buf, (void *)MALLOC(nbytes), comm,
                     MPI_ERR_EXHAUSTED, "MPI_BCAST"); 

          if (rank == root) {
              position = 0;
              MPI_Pack(buffer, count, datatype->self, tmp_buf, nbytes,
                       &position, comm->self);
          }
      }

      scatter_size = (nbytes + size - 1)/size; /* ceiling division */
      curr_size = (rank == root) ? nbytes : 0; /* root starts with all the
                                                      data */

      mask = 0x1;
      while (mask < size) {
          if (relative_rank & mask) {
              src = rank - mask; 
              if (src < 0) src += size;
              recv_size = nbytes - relative_rank*scatter_size;
              /* recv_size is larger than what might actually be sent by the
                 sender. We don't need compute the exact value because MPI
                 allows you to post a larger recv.*/ 
              if (recv_size <= 0) 
                  curr_size = 0; /* this process doesn't receive any data
                                    because of uneven division */
              else {
                  mpi_errno = MPI_Recv((void *)((char *)tmp_buf +
                                                relative_rank*scatter_size),
                                       recv_size, MPI_BYTE, src,
                                       MPIR_BCAST_TAG, comm->self, &status);
                  if (mpi_errno) return mpi_errno;

                  /* query actual size of data received */
                  MPI_Get_count(&status, MPI_BYTE, &curr_size);
              }
              break;
          }
          mask <<= 1;
      }

      /* This process is responsible for all processes that have bits
         set from the LSB upto (but not including) mask.  Because of
         the "not including", we start by shifting mask back down
         one. */

      mask >>= 1;
      while (mask > 0) {
          if (relative_rank + mask < size) {
              
              send_size = curr_size - scatter_size * mask; 
              /* mask is also the size of this process's subtree */

              if (send_size > 0) {
                  dst = rank + mask;
                  if (dst >= size) dst -= size;
                  mpi_errno = MPI_Send (((char *)tmp_buf +
                                         scatter_size*(relative_rank+mask)),
                                        send_size, MPI_BYTE, dst,
                                        MPIR_BCAST_TAG, comm->self);
                  if (mpi_errno) return mpi_errno;
                  curr_size -= send_size;
              }
          }
          mask >>= 1;
      }

      /* Scatter complete. Now do an allgather using recursive
         doubling. The basic recursive doubling algorithm works for
         power-of-two number of processes. We modify it for
         non-powers-of-two.  */ 

      mask = 0x1;
      i = 0;
      while (mask < size) {
          relative_dst = relative_rank ^ mask;

          dst = (relative_dst + root) % size; 

          /* find offset into send and recv buffers.
             zero out the least significant "i" bits of relative_rank and
             relative_dst to find root of src and dst
             subtrees. Use ranks of roots as index to send from
             and recv into  buffer */ 

          dst_tree_root = relative_dst >> i;
          dst_tree_root <<= i;
          
          my_tree_root = relative_rank >> i;
          my_tree_root <<= i;

          send_offset = my_tree_root * scatter_size;
          recv_offset = dst_tree_root * scatter_size;

          if (relative_dst < size) {
              mpi_errno = MPI_Sendrecv((void *)((char *)tmp_buf + send_offset),
                           curr_size, MPI_BYTE, dst, MPIR_BCAST_TAG, 
                           (void *)((char *)tmp_buf + recv_offset),
                           scatter_size*mask, MPI_BYTE, dst,
                           MPIR_BCAST_TAG, comm->self, &status);
              if (mpi_errno) return mpi_errno; 
              MPI_Get_count(&status, MPI_BYTE, &recv_size);
              curr_size += recv_size;
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             data that they would normally have received from those
             processes. That is, the haves in this subtree must send to
             the havenots. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;

              offset = scatter_size * (my_tree_root + mask);
              tmp_mask = mask >> 1;

              while (tmp_mask) {
                  relative_dst = relative_rank ^ tmp_mask;
                  dst = (relative_dst + root) % size; 
                  
                  tree_root = relative_rank >> k;
                  tree_root <<= k;

                  /* send only if this proc has data and destination
                     doesn't have data. */

                  /* if (rank == 3) { 
                      printf("rank %d, dst %d, root %d, nprocs_completed %d\n", relative_rank, relative_dst, tree_root, nprocs_completed);
                      fflush(stdout);
                      }*/

                  if ((relative_dst > relative_rank) && 
                      (relative_rank < tree_root + nprocs_completed)
                      && (relative_dst >= tree_root + nprocs_completed)) {

                      /* printf("Rank %d, send to %d, offset %d, size %d\n", rank, dst, offset, recv_size);
                         fflush(stdout); */
                      mpi_errno = MPI_Send(((char *)tmp_buf + offset),
                                           recv_size, MPI_BYTE, dst, 
                                           MPIR_BCAST_TAG, comm->self); 
                      /* recv_size was set in the previous
                         receive. that's the amount of data to be
                         sent now. */
                      if (mpi_errno) return mpi_errno; 
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((relative_dst < relative_rank) && 
                           (relative_dst < tree_root + nprocs_completed) &&
                           (relative_rank >= tree_root + nprocs_completed)) {
                      /* printf("Rank %d waiting to recv from rank %d\n",
                         relative_rank, dst); */
                      mpi_errno = MPI_Recv(((char *)tmp_buf + offset),
                               scatter_size*nprocs_completed, MPI_BYTE,
                               dst, MPIR_BCAST_TAG, comm->self, &status);
                      /* nprocs_completed is also equal to the no. of processes
                         whose data we don't have */
                      if (mpi_errno) return mpi_errno; 
                      MPI_Get_count(&status, MPI_BYTE, &recv_size);
                      curr_size += recv_size;
                      /* printf("Rank %d, recv from %d, offset %d, size %d\n", rank, dst, offset, recv_size);
                         fflush(stdout);*/
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }

          mask <<= 1;
          i++;
      }

      if (!is_contig || !is_homogeneous) {
          if (rank != root) {
              position = 0;
              MPI_Unpack(tmp_buf, nbytes, &position, buffer, count,
                         datatype->self, comm->self);
          }
          FREE(tmp_buf);
      }
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


/* This is the default implementation of gather. The algorithm is:
   
   Algorithm: MPI_Gather

   We use a minimum spanning tree (MST) algorithm for both short and
   long messages. At nodes other than leaf nodes we need to allocate
   a temporary buffer to store the incoming message. If the root is
   not rank 0, we receive data in a temporary buffer on the root and
   then reorder it into the right order. In the heterogeneous case
   we first pack the buffers by using MPI_Pack and then do the gather. 

   Cost = lgp.alpha + n.((p-1)/p).beta

   where n is the total size of the data gathered at the root.

   Possible improvements: 

   End Algorithm: MPI_Gather
*/

static int intra_Gather ( 
	void *sendbuf, 
	int sendcnt, 
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int recvcount, 
	struct MPIR_DATATYPE *recvtype, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  int curr_cnt=0, relative_rank, nbytes, recv_size, is_homogeneous;
  int mask, sendtype_size, src, dst, position, tmp_buf_size;
  void *tmp_buf=NULL;
  MPI_Status status;
  MPI_Aint   extent;            /* Datatype extent */
  static char myname[] = "MPI_GATHER";

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0, (char *)0, root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char *)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif

  is_homogeneous = 1;
#ifdef MPID_HAS_HETERO  
  is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
  is_homogeneous = 0;   /* Globus */
#endif

  /* Get my rank and switch communicators to the hidden collective */
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

/* Use MST algorithm. */

  relative_rank = (rank >= root) ? rank - root : rank - root + size;

  if (rank == root) 
      MPI_Type_extent ( recvtype->self, &extent );

  if (is_homogeneous) {
      /* communicator is homogeneous. no need to pack buffer. */

      MPI_Type_size(sendtype->self, &sendtype_size);
      nbytes = sendtype_size * sendcnt;

      if (rank == root) {
          if (root != 0) {
              /* allocate temporary buffer to receive data because it
                 will not be in the right order. We will need to
                 reorder it into the recv_buf. */
              MPIR_ALLOC(tmp_buf, (void *)MALLOC(nbytes*size),
                         comm, MPI_ERR_EXHAUSTED, "MPI_GATHER" );

              /* copy root's sendbuf into tmpbuf just so that it is
                 easier to unpack everything later into the recv_buf */
              mpi_errno = MPI_Sendrecv ( sendbuf, sendcnt, sendtype->self,
                                         rank, MPIR_GATHER_TAG, 
                                         tmp_buf, nbytes, MPI_BYTE,
                                         rank, MPIR_GATHER_TAG,
                                         comm->self, &status );
              if (mpi_errno) return mpi_errno;
              curr_cnt = nbytes;
          }
          else {
              /* root is 0. no tmp_buf needed at root. */
              /* copy root's sendbuf into recvbuf */
              mpi_errno = MPI_Sendrecv ( sendbuf, sendcnt, sendtype->self,
                                         rank, MPIR_GATHER_TAG, recvbuf,
                                         recvcount, recvtype->self, rank,
                                         MPIR_GATHER_TAG, comm->self,
                                         &status );
              if (mpi_errno) return mpi_errno;
              curr_cnt = recvcount;
          }          
      }
      else if (!(relative_rank % 2)) {
          /* allocate temporary buffer for nodes other than leaf
             nodes. max size needed is (nbytes*size)/2. */
          MPIR_ALLOC(tmp_buf, (void *)MALLOC((nbytes*size)/2),
                     comm, MPI_ERR_EXHAUSTED, "MPI_GATHER" );

          /* copy from sendbuf into tmp_buf */
          mpi_errno = MPI_Sendrecv ( sendbuf, sendcnt, sendtype->self,
                                     rank, MPIR_GATHER_TAG, tmp_buf,
                                     nbytes, MPI_BYTE, rank,
                                     MPIR_GATHER_TAG, comm->self,
                                     &status );
          if (mpi_errno) return mpi_errno;
          curr_cnt = nbytes;
      }

      mask = 0x1;
      while (mask < size) {
          if ((mask & relative_rank) == 0) {
              src = relative_rank | mask;
              if (src < size) {
                  src = (src + root) % size;
                  if ((rank == root) && (root == 0)) {
                      /* root is 0. Receive directly into recvbuf */
                      mpi_errno = MPI_Recv(((char *)recvbuf + 
                                            src*recvcount*extent), 
                                           recvcount*mask, recvtype->self, src,
                                           MPIR_GATHER_TAG, comm->self, 
                                           &status);
                      if (mpi_errno) return mpi_errno;
                  }
                  else {
                      /* intermediate nodes or nonzero root. store in
                         tmp_buf */
                      mpi_errno = MPI_Recv(((char *)tmp_buf + curr_cnt), 
                                           mask*nbytes, MPI_BYTE, src,
                                           MPIR_GATHER_TAG, comm->self, 
                                           &status);
                      if (mpi_errno) return mpi_errno;
                      /* the recv size is larger than what may be sent in
                         some cases. query amount of data actually received */
                      MPI_Get_count(&status, MPI_BYTE, &recv_size);
                      curr_cnt += recv_size;
                  }
              }
          }
          else {
              dst = relative_rank ^ mask;
              dst = (dst + root) % size;
              if (relative_rank % 2) {
                  /* leaf nodes send directly from sendbuf */
                  mpi_errno = MPI_Send(sendbuf, sendcnt,
                                       sendtype->self, dst,
                                       MPIR_GATHER_TAG, comm->self); 
                  if (mpi_errno) return mpi_errno;
              }
              else {
                  mpi_errno = MPI_Send(tmp_buf, curr_cnt, MPI_BYTE, dst,
                                       MPIR_GATHER_TAG, comm->self); 
                  if (mpi_errno) return mpi_errno;
              }
              break;
          }
          mask <<= 1;
      }

      if ((rank == root) && (root != 0)) {
          /* reorder and copy from tmp_buf into recvbuf */
          position = 0;
          MPI_Unpack(tmp_buf, nbytes*size, &position,
                     ((char *) recvbuf + extent*recvcount*rank),
                     recvcount*(size-rank), recvtype->self, comm->self); 
          MPI_Unpack(tmp_buf, nbytes*size, &position, recvbuf,
                     recvcount*rank, recvtype->self, comm->self); 
          FREE(tmp_buf);
      }
      else if (relative_rank && !(relative_rank % 2))
          FREE(tmp_buf);
  }

  else { /* communicator is heterogeneous. pack data into tmp_buf. */
      if (rank == root)
          MPI_Pack_size(recvcount*size, recvtype->self, comm->self,
                        &tmp_buf_size); 
      else
          MPI_Pack_size(sendcnt*(size/2), sendtype->self,
                        comm->self, &tmp_buf_size);  

      MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                 MPI_ERR_EXHAUSTED, "MPI_GATHER"); 
      position = 0;
      MPI_Pack(sendbuf, sendcnt, sendtype->self, tmp_buf,
               tmp_buf_size, &position, comm->self);
      nbytes = position;

      curr_cnt = nbytes;

      mask = 0x1;
      while (mask < size) {
          if ((mask & relative_rank) == 0) {
              src = relative_rank | mask;
              if (src < size) {
                  src = (src + root) % size;
                  mpi_errno = MPI_Recv(((char *)tmp_buf + curr_cnt), 
                                       mask*nbytes, MPI_BYTE, src,
                                       MPIR_GATHER_TAG, comm->self, 
                                       &status);
                  if (mpi_errno) return mpi_errno;
                  /* the recv size is larger than what may be sent in
                     some cases. query amount of data actually received */
                  MPI_Get_count(&status, MPI_BYTE, &recv_size);
                  curr_cnt += recv_size;
              }
          }
          else {
              dst = relative_rank ^ mask;
              dst = (dst + root) % size;
              mpi_errno = MPI_Send(tmp_buf, curr_cnt, MPI_BYTE, dst,
                                   MPIR_GATHER_TAG, comm->self); 
              if (mpi_errno) return mpi_errno;
              break;
          }
          mask <<= 1;
      }

      if (rank == root) {
          /* reorder and copy from tmp_buf into recvbuf */
          position = 0;
          MPI_Unpack(tmp_buf, tmp_buf_size, &position,
                     ((char *) recvbuf + extent*recvcount*rank),
                     recvcount*(size-rank), recvtype->self, comm->self); 
          if (root != 0)
              MPI_Unpack(tmp_buf, tmp_buf_size, &position, recvbuf,
                         recvcount*rank, recvtype->self, comm->self); 
      }

      FREE(tmp_buf);
  }


#ifdef LINEAR 
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

#endif

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


/* This is the default implementation of gatherv. The algorithm is:
   
   Algorithm: MPI_Gatherv

   Since the array of recvcounts is valid only on the root, we cannot
   do a tree algorithm without first communicating the recvcounts to
   other processes. Therefore, we simply use a linear algorithm for the
   gather, which takes (p-1) steps versus lgp steps for the tree
   algorithm. The bandwidth requirement is the same for both algorithms.

   Cost = (p-1).alpha + n.((p-1)/p).beta

   Possible improvements: 

   End Algorithm: MPI_Gatherv
*/

static int intra_Gatherv ( 
	void *sendbuf, 
	int sendcnt,  
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int *recvcnts, 
	int *displs, 
	struct MPIR_DATATYPE *recvtype, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_GATHERV";

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char*)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif

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


/* This is the default implementation of scatter. The algorithm is:
   
   Algorithm: MPI_Scatter

   We use a minimum spanning tree (MST) algorithm for both short and
   long messages. At nodes other than leaf nodes we need to allocate
   a temporary buffer to store the incoming message. If the root is
   not rank 0, we reorder the sendbuf in order of relative ranks by 
   copying it into a temporary buffer, so that all the sends from the
   root are contiguous and in the right order. In the heterogeneous
   case, we first pack the buffer by using MPI_Pack and then do the
   scatter. 

   Cost = lgp.alpha + n.((p-1)/p).beta

   where n is the total size of the data to be scattered from the root.

   Possible improvements: 

   End Algorithm: MPI_Scatter
*/

static int intra_Scatter ( 
	void *sendbuf, 
	int sendcnt, 
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int recvcnt, 
	struct MPIR_DATATYPE *recvtype, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
  MPI_Status status;
  MPI_Aint   extent;
  int        rank, size, is_homogeneous;
  int curr_cnt, relative_rank, nbytes, send_subtree_cnt;
  int mask, recvtype_size, src, dst, position, tmp_buf_size;
  void *tmp_buf=NULL;
  int        mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_SCATTER";

  /* Get size and rank */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Check for invalid arguments */
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char *)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif
 
  is_homogeneous = 1;
#ifdef MPID_HAS_HETERO  
  is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
  is_homogeneous = 0;   /* Globus */
#endif

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

/* Use MST algorithm */

  if (rank == root) 
      MPI_Type_extent ( sendtype->self, &extent );

  relative_rank = (rank >= root) ? rank - root : rank - root + size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  if (is_homogeneous) {
      /* communicator is homogeneous */

      MPI_Type_size(recvtype->self, &recvtype_size);
      nbytes = recvtype_size * recvcnt;

      curr_cnt = 0;

      /* all even nodes other than root need a temporary buffer to
         receive data of max size (nbytes*size)/2 */
      if (relative_rank && !(relative_rank % 2))
          MPIR_ALLOC(tmp_buf, (void *)MALLOC((nbytes*size)/2),
                     comm, MPI_ERR_EXHAUSTED, "MPI_SCATTER" );

      /* if the root is not rank 0, we reorder the sendbuf in order of
         relative ranks and copy it into a temporary buffer, so that
         all the sends from the root are contiguous and in the right
         order. */

      if (rank == root) {
          if (root != 0) {
              MPIR_ALLOC(tmp_buf, (void *)MALLOC(nbytes*size),
                         comm, MPI_ERR_EXHAUSTED, "MPI_SCATTER" );
              position = 0;
              MPI_Pack(((char *) sendbuf + extent*sendcnt*rank),
                       sendcnt*(size-rank), sendtype->self, tmp_buf,
                       nbytes*size, &position, comm->self); 
              MPI_Pack(sendbuf, sendcnt*rank, sendtype->self, tmp_buf,
                       nbytes*size, &position, comm->self); 
              curr_cnt = nbytes*size;
          } 
          else 
              curr_cnt = sendcnt*size;
      }

      /* root has all the data; others have zero so far */

      mask = 0x1;
      while (mask < size) {
          if (relative_rank & mask) {
              src = rank - mask; 
              if (src < 0) src += size;

              /* The leaf nodes receive directly into recvbuf because
                 they don't have to forward data to anyone. Others
                 receive data into a temporary buffer. */
              if (relative_rank % 2) {
                  mpi_errno = MPI_Recv(recvbuf, recvcnt, recvtype->self,
                                       src, MPIR_SCATTER_TAG, comm->self, 
                                       &status);
                  if (mpi_errno) return mpi_errno;
              }
              else {
                  mpi_errno = MPI_Recv(tmp_buf, mask * recvcnt *
                                       recvtype_size, MPI_BYTE, src,
                                       MPIR_SCATTER_TAG, comm->self, 
                                       &status);
                  if (mpi_errno) return mpi_errno;

                  /* the recv size is larger than what may be sent in
                     some cases. query amount of data actually received */
                  MPI_Get_count(&status, MPI_BYTE, &curr_cnt);
              }
              break;
          }
          mask <<= 1;
      }

      /* This process is responsible for all processes that have bits
         set from the LSB upto (but not including) mask.  Because of
         the "not including", we start by shifting mask back down
         one. */

      mask >>= 1;
      while (mask > 0) {
          if (relative_rank + mask < size) {
              dst = rank + mask;
              if (dst >= size) dst -= size;

              if ((rank == root) && (root == 0)) {
                  send_subtree_cnt = curr_cnt - sendcnt * mask; 
                  /* mask is also the size of this process's subtree */
                  mpi_errno = MPI_Send (((char *)sendbuf + 
                                         extent * sendcnt * mask),
                                        send_subtree_cnt,
                                        sendtype->self, dst, 
                                        MPIR_SCATTER_TAG,
                                        comm->self);
              }
              else {
                  /* non-zero root and others */
                  send_subtree_cnt = curr_cnt - nbytes*mask; 
                  /* mask is also the size of this process's subtree */
                  mpi_errno = MPI_Send (((char *)tmp_buf + nbytes*mask),
                                        send_subtree_cnt,
                                        MPI_BYTE, dst,
                                        MPIR_SCATTER_TAG,
                                        comm->self);
              }
              if (mpi_errno) return mpi_errno;
              curr_cnt -= send_subtree_cnt;
          }
          mask >>= 1;
      }

      if ((rank == root) && (root == 0)) {
          /* put root's data in the right place */
          mpi_errno = MPI_Sendrecv ( sendbuf,   
                                     sendcnt,sendtype->self, rank,
                                     MPIR_SCATTER_TAG, recvbuf,
                                     recvcnt, recvtype->self, rank,
                                     MPIR_SCATTER_TAG, comm->self, &status );
          if (mpi_errno) return mpi_errno;
      }
      else if (!(relative_rank % 2)) {
          /* for non-zero root and others, copy from tmp_buf into recvbuf */
          mpi_errno = MPI_Sendrecv ( tmp_buf,
                                     recvcnt*recvtype_size, MPI_BYTE, rank,
                                     MPIR_SCATTER_TAG, recvbuf,
                                     recvcnt, recvtype->self, rank,
                                     MPIR_SCATTER_TAG, comm->self, &status );
          if (mpi_errno) return mpi_errno;
          FREE(tmp_buf);
      }
  }

  else { /* communicator is heterogeneous */

      if (rank == root) {
          MPI_Pack_size(sendcnt*size, sendtype->self, comm->self,
                        &tmp_buf_size); 
          MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                     MPI_ERR_EXHAUSTED, "MPI_SCATTER"); 

          /* calculate the value of nbytes, the number of bytes in packed
             representation that each process receives. We can't
             accurately calculate that from tmp_buf_size because
             MPI_Pack_size returns an upper bound on the amount of memory
             required. (For example, for a single integer, MPICH returns
             pack_size=12.) Therefore, we actually pack some data into
             tmp_buf and see by how much 'position' is incremented. */

          position = 0;
          MPI_Pack(sendbuf, 1, sendtype->self, tmp_buf, tmp_buf_size,
                   &position, comm->self);
          nbytes = position*sendcnt;

          curr_cnt = nbytes*size;

          position = 0;
          if (root == 0)
              MPI_Pack(sendbuf, sendcnt*size, sendtype->self, tmp_buf,
                       tmp_buf_size, &position, comm->self);
          else {
              /* reorder and pack into tmp_buf such that tmp_buf
                 begins with root's data */
              MPI_Pack(((char *) sendbuf + extent*sendcnt*rank),
                       sendcnt*(size-rank), sendtype->self, tmp_buf,
                       tmp_buf_size, &position, comm->self); 
              MPI_Pack(sendbuf, sendcnt*rank, sendtype->self, tmp_buf,
                       tmp_buf_size, &position, comm->self); 
          }
      }
      else {
          MPI_Pack_size(recvcnt*(size/2), recvtype->self, comm->self,
                        &tmp_buf_size); 
          MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                     MPI_ERR_EXHAUSTED, "MPI_SCATTER");

          /* calculate nbytes */
          position = 0;
          MPI_Pack(recvbuf, 1, recvtype->self, tmp_buf, tmp_buf_size,
                   &position, comm->self);
          nbytes = position*recvcnt;

          curr_cnt = 0;
      }

      mask = 0x1;
      while (mask < size) {
          if (relative_rank & mask) {
              src = rank - mask; 
              if (src < 0) src += size;

              mpi_errno = MPI_Recv(tmp_buf, mask*nbytes, MPI_BYTE, src,
                                   MPIR_SCATTER_TAG, comm->self, &status);
              if (mpi_errno) return mpi_errno;
              /* the recv size is larger than what may be sent in
                 some cases. query amount of data actually received */
              MPI_Get_count(&status, MPI_BYTE, &curr_cnt);
              break;
          }
          mask <<= 1;
      }

      /* This process is responsible for all processes that have bits
         set from the LSB upto (but not including) mask.  Because of
         the "not including", we start by shifting mask back down
         one. */

      mask >>= 1;
      while (mask > 0) {
          if (relative_rank + mask < size) {
              dst = rank + mask;
              if (dst >= size) dst -= size;

              send_subtree_cnt = curr_cnt - nbytes * mask; 
              /* mask is also the size of this process's subtree */
              mpi_errno = MPI_Send (((char *)tmp_buf + nbytes*mask),
                                    send_subtree_cnt, MPI_BYTE, dst,
                                    MPIR_SCATTER_TAG, comm->self);
              if (mpi_errno) return mpi_errno;
              curr_cnt -= send_subtree_cnt;
          }
          mask >>= 1;
      }

      /* copy local data into recvbuf */
      position = 0;
      MPI_Unpack(tmp_buf, tmp_buf_size, &position, recvbuf, recvcnt,
                 recvtype->self, comm->self);
      FREE(tmp_buf);
  }


#ifdef LINEAR
      /* use linear algorithm */

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

#endif

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (mpi_errno);
}


/* This is the default implementation of scatterv. The algorithm is:
   
   Algorithm: MPI_Scatterv

   Since the array of sendcounts is valid only on the root, we cannot
   do a tree algorithm without first communicating the sendcounts to
   other processes. Therefore, we simply use a linear algorithm for the
   scatter, which takes (p-1) steps versus lgp steps for the tree
   algorithm. The bandwidth requirement is the same for both algorithms.

   Cost = (p-1).alpha + n.((p-1)/p).beta

   Possible improvements: 

   End Algorithm: MPI_Scatterv
*/

static int intra_Scatterv ( 
	void *sendbuf, 
	int *sendcnts, 
	int *displs, 
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int recvcnt,  
	struct MPIR_DATATYPE *recvtype, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
{
  MPI_Status status;
  int        rank, size;
  int        mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_SCATTERV";

  /* Get size and rank */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  /* Check for invalid arguments */
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char *)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif

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

/* 
   General comments on Allxxx operations
   
   It is hard (though not impossible) to avoid having each at least one process
   doing a send to every other process.  In that case, the order of the
   operations becomes important.
   For example, in the alltoall case, you do NOT want all processes to send 
   to process 1, then all to send to process 2, etc.  In addition, you
   don't want the messages to compete for bandwidth in the network (remember,
   most networks don't provide INDEPENDENT paths between every pair of nodes).
   In that case, the topology of the underlying network becomes important.
   This can further control the choice of ordering for the sends/receives.
   Unfortunately, there is no interface to find this information (one was
   considered by the MPI-1 Forum but not adopted).  Vendor-specific 
   implementations of these routines can take advantage of such information.
 */


/* This is the default implementation of allgather. The algorithm is:
   
   Algorithm: MPI_Allgather

   For both short and long messages, we use a recursive doubling
   algorithm, which takes lgp steps. In each step, pairs of processes
   exchange all the data they have. We take care of non-power-of-two
   situations. 

   Cost = lgp.alpha + n.((p-1)/p).beta

   where n is total size of data gathered on each process.
   (The cost may be slightly more in the non-power-of-two case, but
   it's still a logarithmic algorithm.) 

   It is interesting to note that this algorithm for MPI_Allgather has
   the same cost as the MST algorithm for MPI_Gather!

   Possible improvements: 

   End Algorithm: MPI_Allgather
*/

static int intra_Allgather ( 
	void *sendbuf, 
	int sendcount, 
	struct MPIR_DATATYPE *sendtype,
	void *recvbuf, 
	int recvcount, 
	struct MPIR_DATATYPE *recvtype, 
	struct MPIR_COMMUNICATOR *comm )
{
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  MPI_Aint   recv_extent;
  int        j, i, is_homogeneous, tmp_buf_size;
  int curr_cnt, mask, dst, dst_tree_root, my_tree_root, 
      send_offset, recv_offset, last_recv_cnt, nprocs_completed, k,
      offset, tmp_mask, tree_root, position, nbytes;
  void *tmp_buf;
#ifdef OLD
  int left, right, jnext;
#endif

  /* Get the size of the communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  MPI_Type_extent ( recvtype->self, &recv_extent );

  is_homogeneous = 1;
#ifdef MPID_HAS_HETERO  
  is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
  is_homogeneous = 0;   /* Globus */
#endif

  if (sendcount == 0) return MPI_SUCCESS;

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;
 
  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  /* use recursive doubling algorithm */

  if (is_homogeneous) {
      /* homogeneous. no need to pack into tmp_buf on each node. copy
         local data into recvbuf */ 
      mpi_errno = MPI_Sendrecv ( sendbuf, sendcount, sendtype->self,
                                 rank, MPIR_ALLGATHER_TAG, 
                                 ((char *)recvbuf +
                                  rank*recvcount*recv_extent), 
                                 recvcount, recvtype->self,
                                 rank, MPIR_ALLGATHER_TAG,
                                 comm->self, &status );
      if (mpi_errno) return mpi_errno;
      curr_cnt = recvcount;

      mask = 0x1;
      i = 0;
      while (mask < size) {
          dst = rank ^ mask;

          /* find offset into send and recv buffers. zero out 
             the least significant "i" bits of rank and dst to 
             find root of src and dst subtrees. Use ranks of 
             roots as index to send from and recv into buffer */ 
          
          dst_tree_root = dst >> i;
          dst_tree_root <<= i;
      
          my_tree_root = rank >> i;
          my_tree_root <<= i;
      
          send_offset = my_tree_root * recvcount * recv_extent;
          recv_offset = dst_tree_root * recvcount * recv_extent;

          if (dst < size) {
              mpi_errno = MPI_Sendrecv(((char *)recvbuf + send_offset),
                           curr_cnt, recvtype->self, dst, MPIR_ALLGATHER_TAG, 
                           ((char *)recvbuf + recv_offset),
                           recvcount*mask, recvtype->self, dst,
                           MPIR_ALLGATHER_TAG, comm->self, &status);
              if (mpi_errno) return mpi_errno;
          
              MPI_Get_count(&status, recvtype->self, &last_recv_cnt);
              curr_cnt += last_recv_cnt;
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             data that they would normally have received from those
             processes. That is, the haves in this subtree must send to
             the havenots. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;
          
              offset = recvcount * (my_tree_root + mask) * recv_extent;
              tmp_mask = mask >> 1;

              while (tmp_mask) {
                  dst = rank ^ tmp_mask;
                  
                  tree_root = rank >> k;
                  tree_root <<= k;
              
                  /* send only if this proc has data and destination
                     doesn't have data. at any step, multiple processes
                     can send if they have the data */
                  if ((dst > rank) && 
                      (rank < tree_root + nprocs_completed)
                      && (dst >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Send(((char *)recvbuf + offset),
                                           last_recv_cnt,
                                           recvtype->self, dst,
                                           MPIR_ALLGATHER_TAG, comm->self); 
                      /* last_recv_cnt was set in the previous
                         receive. that's the amount of data to be
                         sent now. */
                      if (mpi_errno) return mpi_errno;
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((dst < rank) && 
                           (dst < tree_root + nprocs_completed) &&
                           (rank >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Recv(((char *)recvbuf + offset),
                               recvcount*nprocs_completed, recvtype->self,
                               dst, MPIR_ALLGATHER_TAG, comm->self, &status);
                      /* nprocs_completed is also equal to the no. of processes
                         whose data we don't have */
                      if (mpi_errno) return mpi_errno;
                      MPI_Get_count(&status, recvtype->self, &last_recv_cnt);
                      curr_cnt += last_recv_cnt;
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }
      
          mask <<= 1;
          i++;
      }
  }

  else {
      /* heterogeneous. need to use temp. buffer. */
      MPI_Pack_size(recvcount*size, recvtype->self, comm->self, &tmp_buf_size);
      MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                 MPI_ERR_EXHAUSTED, "MPI_ALLGATHER");

      /* calculate the value of nbytes, the number of bytes in packed
         representation that each process contributes. We can't simply divide
         tmp_buf_size by comm_size because tmp_buf_size is an upper
         bound on the amount of memory required. (For example, for
         a single integer, MPICH returns pack_size=12.) Therefore, we
         actually pack some data into tmp_buf and see by how much
         'position' is incremented. */

      position = 0;
      MPI_Pack(recvbuf, 1, recvtype->self, tmp_buf, tmp_buf_size,
               &position, comm->self);
      nbytes = position*recvcount;

      /* pack local data into right location in tmp_buf */
      position = rank * nbytes;
      MPI_Pack(sendbuf, sendcount, sendtype->self, tmp_buf, tmp_buf_size,
               &position, comm->self);
      curr_cnt = nbytes;

      mask = 0x1;
      i = 0;
      while (mask < size) {
          dst = rank ^ mask;

          /* find offset into send and recv buffers. zero out 
             the least significant "i" bits of rank and dst to 
             find root of src and dst subtrees. Use ranks of 
             roots as index to send from and recv into buffer. */ 
          
          dst_tree_root = dst >> i;
          dst_tree_root <<= i;
      
          my_tree_root = rank >> i;
          my_tree_root <<= i;
      
          send_offset = my_tree_root * nbytes;
          recv_offset = dst_tree_root * nbytes;

          if (dst < size) {
              mpi_errno = MPI_Sendrecv(((char *)tmp_buf + send_offset),
                           curr_cnt, MPI_BYTE, dst, MPIR_ALLGATHER_TAG, 
                           ((char *)tmp_buf + recv_offset),
                           nbytes*mask, MPI_BYTE, dst,
                           MPIR_ALLGATHER_TAG, comm->self, &status);
              if (mpi_errno) return mpi_errno;
          
              MPI_Get_count(&status, MPI_BYTE, &last_recv_cnt);
              curr_cnt += last_recv_cnt;
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             data that they would normally have received from those
             processes. That is, the haves in this subtree must send to
             the havenots. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;
          
              offset = nbytes * (my_tree_root + mask);
              tmp_mask = mask >> 1;

              while (tmp_mask) {
                  dst = rank ^ tmp_mask;
                  
                  tree_root = rank >> k;
                  tree_root <<= k;
              
                  /* send only if this proc has data and destination
                     doesn't have data. at any step, multiple processes
                     can send if they have the data */
                  if ((dst > rank) && 
                      (rank < tree_root + nprocs_completed)
                      && (dst >= tree_root + nprocs_completed)) {
                  
                      mpi_errno = MPI_Send(((char *)tmp_buf + offset),
                                           last_recv_cnt, MPI_BYTE,
                                           dst, MPIR_ALLGATHER_TAG,
                                           comm->self);  
                      /* last_recv_cnt was set in the previous
                         receive. that's the amount of data to be
                         sent now. */
                      if (mpi_errno) return mpi_errno;
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((dst < rank) && 
                           (dst < tree_root + nprocs_completed) &&
                           (rank >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Recv(((char *)tmp_buf + offset),
                               nbytes*nprocs_completed, MPI_BYTE,
                               dst, MPIR_ALLGATHER_TAG, comm->self, &status);
                      /* nprocs_completed is also equal to the no. of processes
                         whose data we don't have */
                      if (mpi_errno) return mpi_errno;
                      MPI_Get_count(&status, MPI_BYTE, &last_recv_cnt);
                      curr_cnt += last_recv_cnt;
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }
          mask <<= 1;
          i++;
      }

      position = 0;
      MPI_Unpack(tmp_buf, tmp_buf_size, &position, recvbuf, recvcount*size,
                 recvtype->self, comm->self);

      FREE(tmp_buf);
  }


#ifdef OLD

  /* Do a gather for each process in the communicator
     This is the "circular" algorithm for allgather - each process sends to
     its right and receives from its left.  This is faster than simply
     doing size Gathers.
   */

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
#endif

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}



/* This is the default implementation of allgatherv. The algorithm is:
   
   Algorithm: MPI_Allgatherv

   For short and medium-size messages (up to 256KB), we use a
   recursive doubling algorithm, which takes lgp steps. In each step,
   pairs of processes exchange all the data they have. We take care of
   non-power-of-two situations. 

   Cost = lgp.alpha + n.((p-1)/p).beta

   where n is total size of data gathered on each process.
   (The cost may be slightly more in the non-power-of-two case, but
   it's still a logarithmic algorithm.) 

   Because of displacements in the recvbuf, the data received may be
   noncontiguously located. In the recursive doubling algorithm, this
   requires us to use a temp buffer, collect all the data
   contiguously, and then copy it into the recvbuf. 

   For long messages, to avoid the overhead of copying data, we
   instead use the ring algorithm where each process sends its own
   contribution around the ring of processes. This takes p-1 steps
   but has the same bandwidth requirement as recursive doubling.

   Cost = (p-1).alpha + n.((p-1)/p).beta

   Possible improvements: 

   End Algorithm: MPI_Allgatherv
*/

static int intra_Allgatherv ( 
	void *sendbuf, 
	int sendcount,  
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int *recvcounts, 
	int *displs,   
	struct MPIR_DATATYPE *recvtype, 
	struct MPIR_COMMUNICATOR *comm )
{
  int        size, rank, j, i, tmp_buf_size;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  MPI_Aint   recv_extent, lb;
  int curr_cnt, mask, dst, dst_tree_root, my_tree_root, is_homogeneous,
      send_offset, recv_offset, last_recv_cnt, nprocs_completed, k,
      offset, tmp_mask, tree_root, position, nbytes, total_count;
  void *tmp_buf;
  int left, right, jnext;

  /* Get the size of the communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );

  MPI_Type_extent ( recvtype->self, &recv_extent );

  total_count = 0;
  for (i=0; i<size; i++)
      total_count += recvcounts[i];

  if (total_count == 0) return MPI_SUCCESS;

  /* Switch communicators to the hidden collective */
  comm = comm->comm_coll;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx,comm);

  if (total_count > MPIR_ALLGATHERV_MEDIUM_MSG) {
        /* long message. use ring algorithm. */

      /* First, load the "local" version in the recvbuf. */
      mpi_errno = 
          MPI_Sendrecv( sendbuf, sendcount, sendtype->self, rank, 
                        MPIR_ALLGATHERV_TAG,
                        (void *)((char *)recvbuf + displs[rank]*recv_extent),
                        recvcounts[rank], recvtype->self, rank, 
                        MPIR_ALLGATHERV_TAG, 
                        comm->self, &status );
      if (mpi_errno)  return mpi_errno;

      left  = (size + rank - 1) % size;
      right = (rank + 1) % size;
      
      j     = rank;
      jnext = left;
      for (i=1; i<size; i++) {
          mpi_errno = 
              MPI_Sendrecv( (void *)((char *)recvbuf+displs[j]*recv_extent),
                            recvcounts[j], recvtype->self, right,
                            MPIR_ALLGATHERV_TAG, 
                            (void *)((char *)recvbuf +
                                     displs[jnext]*recv_extent), 
                            recvcounts[jnext], recvtype->self, left, 
                            MPIR_ALLGATHERV_TAG, comm->self,
                            &status );
          if (mpi_errno) break;
          j	    = jnext;
          jnext = (size + jnext - 1) % size;
      }
  }

  else { /* Short or medium size message. Use recursive doubling algorithm */

      is_homogeneous = 1;
#ifdef MPID_HAS_HETERO  
      is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
      is_homogeneous = 0;   /* Globus */
#endif

      if (is_homogeneous) {
          /* need to receive contiguously into tmp_buf because
             displs could make the recvbuf noncontiguous */

          MPIR_ALLOC(tmp_buf, (void *)MALLOC(total_count*recv_extent), comm,
                     MPI_ERR_EXHAUSTED, "MPI_ALLGATHERV");
          /* adjust for potential negative lower bound in datatype */
          MPI_Type_lb( recvtype->self, &lb );
          tmp_buf = (void *)((char*)tmp_buf - lb);

          /* copy local data into right location in tmp_buf */ 
          position = 0;
          for (i=0; i<rank; i++) position += recvcounts[i];
          mpi_errno = MPI_Sendrecv(sendbuf, sendcount,
                                   sendtype->self, rank, 
                                   MPIR_ALLGATHERV_TAG, 
                                   ((char *)tmp_buf + position*
                                    recv_extent), 
                                   recvcounts[rank], recvtype->self,
                                   rank, MPIR_ALLGATHERV_TAG,
                                   comm->self, &status);
          if (mpi_errno) return mpi_errno;

          curr_cnt = recvcounts[rank];
          
          mask = 0x1;
          i = 0;
          while (mask < size) {
              dst = rank ^ mask;

              /* find offset into send and recv buffers. zero out 
                 the least significant "i" bits of rank and dst to 
                 find root of src and dst subtrees. Use ranks of 
                 roots as index to send from and recv into buffer */ 
          
              dst_tree_root = dst >> i;
              dst_tree_root <<= i;
              
              my_tree_root = rank >> i;
              my_tree_root <<= i;
              
              send_offset = 0;
              for (j=0; j<my_tree_root; j++)
                  send_offset += recvcounts[j];
              send_offset *= recv_extent;
                
              recv_offset = 0;
              for (j=0; j<dst_tree_root; j++)
                  recv_offset += recvcounts[j];
              recv_offset *= recv_extent;
              
              if (dst < size) {
                  mpi_errno = MPI_Sendrecv(((char *)tmp_buf + send_offset),
                                           curr_cnt, recvtype->self,
                                           dst, MPIR_ALLGATHERV_TAG,  
                                           ((char *)tmp_buf + recv_offset),
                                           total_count, recvtype->self, dst,
                                           MPIR_ALLGATHERV_TAG,
                                           comm->self, &status); 
                  /* for convenience, recv is posted for a bigger amount
                     than will be sent */ 
                  if (mpi_errno) return mpi_errno;
          
                  MPI_Get_count(&status, recvtype->self, &last_recv_cnt);
                  curr_cnt += last_recv_cnt;
              }

              /* if some processes in this process's subtree in this step
                 did not have any destination process to communicate with
                 because of non-power-of-two, we need to send them the
                 data that they would normally have received from those
                 processes. That is, the haves in this subtree must send to
                 the havenots. We use a logarithmic recursive-halfing algorithm
                 for this. */

              if (dst_tree_root + mask > size) {
                  nprocs_completed = size - my_tree_root - mask;
                  /* nprocs_completed is the number of processes in this
                     subtree that have all the data. Send data to others
                     in a tree fashion. First find root of current tree
                     that is being divided into two. k is the number of
                     least-significant bits in this process's rank that
                     must be zeroed out to find the rank of the root */ 
                  j = mask;
                  k = 0;
                  while (j) {
                      j >>= 1;
                      k++;
                  }
                  k--;
          
                  offset = 0;
                  for (j=0; j<(my_tree_root+mask); j++)
                      offset += recvcounts[j];
                  offset *= recv_extent;

                  tmp_mask = mask >> 1;
                  
                  while (tmp_mask) {
                      dst = rank ^ tmp_mask;
                      
                      tree_root = rank >> k;
                      tree_root <<= k;
                      
                      /* send only if this proc has data and destination
                         doesn't have data. at any step, multiple processes
                         can send if they have the data */
                      if ((dst > rank) && 
                          (rank < tree_root + nprocs_completed)
                          && (dst >= tree_root + nprocs_completed)) {
                          mpi_errno = MPI_Send(((char *)tmp_buf + offset),
                                               last_recv_cnt,
                                               recvtype->self, dst,
                                               MPIR_ALLGATHERV_TAG, comm->self); 
                          /* last_recv_cnt was set in the previous
                             receive. that's the amount of data to be
                             sent now. */
                          if (mpi_errno) return mpi_errno;
                      }
                      /* recv only if this proc. doesn't have data and sender
                         has data */
                      else if ((dst < rank) && 
                               (dst < tree_root + nprocs_completed) &&
                               (rank >= tree_root + nprocs_completed)) {
                          mpi_errno = MPI_Recv(((char *)tmp_buf + offset),
                                               total_count, recvtype->self,
                                               dst, MPIR_ALLGATHERV_TAG,
                                               comm->self, &status);  
                          if (mpi_errno) return mpi_errno;
                          /* for convenience, recv is posted for a
                             bigger amount than will be sent */ 

                          MPI_Get_count(&status, recvtype->self, &last_recv_cnt);
                          curr_cnt += last_recv_cnt;
                      }
                      tmp_mask >>= 1;
                      k--;
                  }
              }
              
              mask <<= 1;
              i++;
          }

          /* copy data from tmp_buf to recvbuf */
          position = 0;
          for (j=0; j<size; j++) {
              MPI_Sendrecv(((char *)tmp_buf + position*recv_extent),
                           recvcounts[j], recvtype->self, rank,
                           MPIR_ALLGATHERV_TAG, 
                           ((char *)recvbuf + displs[j]*recv_extent),
                           recvcounts[j], recvtype->self, rank,
                           MPIR_ALLGATHERV_TAG, comm->self, &status);
              position += recvcounts[j];
          }

          FREE((char*)tmp_buf + lb);
      }

      else {
          /* heterogeneous. need to use temp. buffer. */
          MPI_Pack_size(total_count, recvtype->self, comm->self, &tmp_buf_size);
          MPIR_ALLOC(tmp_buf, (void *)MALLOC(tmp_buf_size), comm,
                     MPI_ERR_EXHAUSTED, "MPI_ALLGATHERV");
          
          /* calculate the value of nbytes, the number of bytes in packed
             representation corresponding to a single recvtype. Since
             MPI_Pack_size returns only an upper bound on 
             the size, to get the real size we actually pack some data
             into tmp_buf and see by how much 'position' is incremented. */
          
          position = 0;
          MPI_Pack(recvbuf, 1, recvtype->self, tmp_buf, tmp_buf_size,
                   &position, comm->self);
          nbytes = position;
          
          /* pack local data into right location in tmp_buf */
          position = 0;
          for (i=0; i<rank; i++)
              position += recvcounts[i];
          position *= nbytes;
          MPI_Pack(sendbuf, sendcount, sendtype->self, tmp_buf, tmp_buf_size,
                   &position, comm->self);
          curr_cnt = recvcounts[rank]*nbytes;
          
          mask = 0x1;
          i = 0;
          while (mask < size) {
              dst = rank ^ mask;
              
              /* find offset into send and recv buffers. zero out 
                 the least significant "i" bits of rank and dst to 
                 find root of src and dst subtrees. Use ranks of 
                 roots as index to send from and recv into buffer. */ 
              
              dst_tree_root = dst >> i;
              dst_tree_root <<= i;
              
              my_tree_root = rank >> i;
              my_tree_root <<= i;
              
              send_offset = 0;
              for (j=0; j<my_tree_root; j++)
                  send_offset += recvcounts[j];
              send_offset *= nbytes;
              
              recv_offset = 0;
              for (j=0; j<dst_tree_root; j++)
                  recv_offset += recvcounts[j];
              recv_offset *= nbytes;
              
              if (dst < size) {
                  mpi_errno = MPI_Sendrecv(((char *)tmp_buf + send_offset),
                                           curr_cnt, MPI_BYTE, dst,
                                           MPIR_ALLGATHERV_TAG,  
                                           ((char *)tmp_buf + recv_offset),
                                           nbytes*total_count, MPI_BYTE, dst,
                                           MPIR_ALLGATHERV_TAG,
                                           comm->self, &status); 
                  /* for convenience, recv is posted for a bigger amount
                     than will be sent */ 
                  if (mpi_errno) return mpi_errno;
                  
                  MPI_Get_count(&status, MPI_BYTE, &last_recv_cnt);
                  curr_cnt += last_recv_cnt;
              }
              
              /* if some processes in this process's subtree in this step
                 did not have any destination process to communicate with
                 because of non-power-of-two, we need to send them the
                 data that they would normally have received from those
                 processes. That is, the haves in this subtree must send to
                 the havenots. We use a logarithmic recursive-halfing algorithm
                 for this. */
              
              if (dst_tree_root + mask > size) {
                  nprocs_completed = size - my_tree_root - mask;
                  /* nprocs_completed is the number of processes in this
                     subtree that have all the data. Send data to others
                     in a tree fashion. First find root of current tree
                     that is being divided into two. k is the number of
                     least-significant bits in this process's rank that
                     must be zeroed out to find the rank of the root */ 
                  j = mask;
                  k = 0;
                  while (j) {
                      j >>= 1;
                      k++;
                  }
                  k--;
                  
                  offset = 0;
                  for (j=0; j<(my_tree_root+mask); j++)
                      offset += recvcounts[j];
                  offset *= nbytes;
                  tmp_mask = mask >> 1;
                  
                  while (tmp_mask) {
                      dst = rank ^ tmp_mask;
                      
                      tree_root = rank >> k;
                      tree_root <<= k;
                      
                      /* send only if this proc has data and destination
                         doesn't have data. at any step, multiple processes
                         can send if they have the data */
                      if ((dst > rank) && 
                          (rank < tree_root + nprocs_completed)
                          && (dst >= tree_root + nprocs_completed)) {
                          
                          mpi_errno = MPI_Send(((char *)tmp_buf + offset),
                                               last_recv_cnt, MPI_BYTE,
                                               dst, MPIR_ALLGATHERV_TAG,
                                               comm->self);  
                          /* last_recv_cnt was set in the previous
                             receive. that's the amount of data to be
                             sent now. */
                          if (mpi_errno) return mpi_errno;
                      }
                      /* recv only if this proc. doesn't have data and sender
                         has data */
                      else if ((dst < rank) && 
                               (dst < tree_root + nprocs_completed) &&
                               (rank >= tree_root + nprocs_completed)) {
                          mpi_errno = MPI_Recv(((char *)tmp_buf + offset),
                                               nbytes*total_count, MPI_BYTE,
                                               dst,
                                               MPIR_ALLGATHERV_TAG,
                                               comm->self, &status); 
                          /* for convenience, recv is posted for a
                             bigger amount than will be sent */ 
                          if (mpi_errno) return mpi_errno;
                          MPI_Get_count(&status, MPI_BYTE, &last_recv_cnt);
                          curr_cnt += last_recv_cnt;
                      }
                      tmp_mask >>= 1;
                      k--;
                  }
              }
              mask <<= 1;
              i++;
          }
          
          position = 0;
          for (j=0; j<size; j++)
              MPI_Unpack(tmp_buf, tmp_buf_size, &position, 
                         ((char *)recvbuf + displs[j]*recv_extent),
                         recvcounts[j], recvtype->self, comm->self);
          
          FREE(tmp_buf);
      }
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


/* This is the default implementation of alltoall. The algorithm is:
   
   Algorithm: MPI_Alltoall

   We use four algorithms for alltoall. For very short messages, we use
   a recursive doubling algorithm that takes lgp steps. At each step
   pairs of processes exchange all the data they have (received) so
   far. A lot more data is communicated than each process needs, but
   for very short messages (typically 256 bytes or less), this
   algorithm is still better because of the lower latency.

   Cost = lgp.alpha + n.p.beta

   where n is the total amount of a data that a process sends to all
   other processes

   For medium size messages (typically 256 bytes -- 256 Kbytes), we
   use an algorithm that posts all irecvs and isends and then does a
   waitall. We scatter the order of sources and destinations among the
   processes, so that all processes don't try to send/recv to/from the
   same process at the same time.

   For long messages, we use a pairwise exchange algorithm, which
   takes p-1 steps. For a power-of-two number of processes, we
   calculate the pairs by using an exclusive-or algorithm:
           for (i=1; i<comm_size; i++)
               dest = rank ^ i;
   This algorithm doesn't work if the number of processes is not a power of
   two. For a non-power-of-two number of processes, we create pairs by
   having each process receive from (rank-i) and send to (rank+i) at
   step i.

   Cost = (p-1).alpha + n.beta

   where n is the total amount of data a process needs to send to all
   other processes.

   Possible improvements: 

   End Algorithm: MPI_Alltoall
*/


static int intra_Alltoall( 
	void *sendbuf, 
	int sendcount, 
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int recvcnt, 
	struct MPIR_DATATYPE *recvtype, 
	struct MPIR_COMMUNICATOR *comm )
{
  int          size, i, j, k, p, pof2;
  MPI_Aint     sendtype_extent, recvtype_extent, sendbuf_extent, lb;
  int          mpi_errno = MPI_SUCCESS;
  static char myname[] = "MPI_ALLTOALL";
  MPI_Status status;
  int src, dst, rank, nbytes, curr_cnt, dst_tree_root, my_tree_root;
  int last_recv_cnt, mask, tmp_mask, tree_root, nprocs_completed;
  int sendtype_size;
  void *tmp_buf;
  MPI_Status  *starray;
  MPI_Request *reqarray;

  /* Get size and switch to collective communicator */
  MPIR_Comm_size ( comm, &size );
  MPIR_Comm_rank ( comm, &rank );
  comm = comm->comm_coll;
  
  /* Get extent of send and recv types */
  MPI_Type_extent ( sendtype->self, &sendtype_extent );
  MPI_Type_extent ( recvtype->self, &recvtype_extent );

  MPI_Type_size(sendtype->self, &sendtype_size);
  nbytes = sendtype_size * sendcount * size;

  /* Lock for collective operation */
  MPID_THREAD_LOCK(comm->ADIctx, comm);

  if (nbytes <= MPIR_ALLTOALL_SHORT_MSG) {
      /* Short message. Use recursive doubling. Each process sends all
       its data at each step along with all data it received in
       previous steps. */

      /* need to allocate temporary buffer of size
         sendbuf_extent*comm_size */

      sendbuf_extent = sendcount * size * sendtype_extent; 
      MPIR_ALLOC(tmp_buf, (void *) MALLOC(sendbuf_extent*size),
                 comm, MPI_ERR_EXHAUSTED, myname);

      /* adjust for potential negative lower bound in datatype */
      MPI_Type_lb( sendtype->self, &lb );
      tmp_buf = (void *)((char*)tmp_buf - lb);

      /* copy local sendbuf into tmp_buf at location indexed by rank */
      curr_cnt = sendcount*size;
      mpi_errno = MPI_Sendrecv (sendbuf, curr_cnt, sendtype->self,
                                rank, MPIR_ALLTOALL_TAG, 
                                ((char *)tmp_buf + rank*sendbuf_extent),
                                curr_cnt, sendtype->self, rank,
                                MPIR_ALLTOALL_TAG, comm->self, &status); 
      if (mpi_errno) return mpi_errno;

      mask = 0x1;
      i = 0;
      while (mask < size) {
          dst = rank ^ mask;

          dst_tree_root = dst >> i;
          dst_tree_root <<= i;
      
          my_tree_root = rank >> i;
          my_tree_root <<= i;

          if (dst < size) {
              mpi_errno = MPI_Sendrecv(((char *)tmp_buf +
                                        my_tree_root*sendbuf_extent),
                                       curr_cnt, sendtype->self,
                                       dst, MPIR_ALLTOALL_TAG, 
                                       ((char *)tmp_buf +
                                        dst_tree_root*sendbuf_extent),
                                       sendcount*size*mask,
                                       sendtype->self, dst, 
                                       MPIR_ALLTOALL_TAG, comm->self,
                                       &status);
              if (mpi_errno) return mpi_errno;

              /* in case of non-power-of-two nodes, less data may be
                 received than specified */
              MPI_Get_count(&status, sendtype->self, &last_recv_cnt);
              curr_cnt += last_recv_cnt;
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             result. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;

              tmp_mask = mask >> 1;
              while (tmp_mask) {
                  dst = rank ^ tmp_mask;
                  
                  tree_root = rank >> k;
                  tree_root <<= k;
              
                  /* send only if this proc has data and destination
                     doesn't have data. at any step, multiple processes
                     can send if they have the data */
                  if ((dst > rank) && 
                      (rank < tree_root + nprocs_completed)
                      && (dst >= tree_root + nprocs_completed)) {
                      /* send the data received in this step above */
                      mpi_errno = MPI_Send(((char *)tmp_buf +
                                            dst_tree_root*sendbuf_extent),
                                           last_recv_cnt, sendtype->self,
                                           dst, MPIR_ALLTOALL_TAG,
                                           comm->self);  
                      if (mpi_errno) return mpi_errno;
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((dst < rank) && 
                           (dst < tree_root + nprocs_completed) &&
                           (rank >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Recv(((char *)tmp_buf +
                                            dst_tree_root*sendbuf_extent),
                                           sendcount*size*mask, sendtype->self,
                                           dst, MPIR_ALLTOALL_TAG,
                                           comm->self, &status); 
                      if (mpi_errno) return mpi_errno;
                      MPI_Get_count(&status, sendtype->self, &last_recv_cnt);
                      curr_cnt += last_recv_cnt;
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }

          mask <<= 1;
          i++;
      }

      /* now copy everyone's contribution from tmp_buf to recvbuf */
      for (p=0; p<size; p++) {
          mpi_errno = MPI_Sendrecv (((char *)tmp_buf +
                                     (p*size+rank)*sendcount*sendtype_extent),
                                    sendcount, sendtype->self, rank,
                                    MPIR_ALLTOALL_TAG, 
                                    ((char*)recvbuf +
                                     p*recvcnt*recvtype_extent), 
                                    recvcnt, recvtype->self, rank,
                                    MPIR_ALLTOALL_TAG, comm->self,
                                    &status); 
          if (mpi_errno) return mpi_errno;
      }

      FREE((char *)tmp_buf+lb); 
  }

  else if ((nbytes > MPIR_ALLTOALL_SHORT_MSG) &&
           (nbytes < MPIR_ALLTOALL_MEDIUM_MSG)) {

      /* 1st, get some storage from the heap to hold handles, etc. */
      MPIR_ALLOC(starray,(MPI_Status *)MALLOC(2*size*sizeof(MPI_Status)),
                 comm, MPI_ERR_EXHAUSTED, myname );

      MPIR_ALLOC(reqarray, (MPI_Request *)MALLOC(2*size*sizeof(MPI_Request)),
                 comm, MPI_ERR_EXHAUSTED, myname );

      /* do the communication -- post all sends and receives: */
      for ( i=0; i<size; i++ ) { 
          dst = (rank+i) % size;
          mpi_errno = MPI_Irecv((char *)recvbuf +
                                dst*recvcnt*recvtype_extent,  
                                recvcnt,
                                recvtype->self,
                                dst,
                                MPIR_ALLTOALL_TAG,
                                comm->self,
                                &reqarray[i]);
          if (mpi_errno) return mpi_errno;
      }

      for ( i=0; i<size; i++ ) { 
          dst = (rank+i) % size;
          mpi_errno = MPI_Isend((char *)sendbuf +
                                dst*sendcount*sendtype_extent, 
                                sendcount,
                                sendtype->self,
                                dst,
                                MPIR_ALLTOALL_TAG,
                                comm->self,
                                &reqarray[i+size]);
          if (mpi_errno) return mpi_errno;
      }

      /* ... then wait for *all* of them to finish: */
      mpi_errno = MPI_Waitall(2*size,reqarray,starray);
      if (mpi_errno == MPI_ERR_IN_STATUS) {
          for (j=0; j<2*size; j++) {
              if (starray[j].MPI_ERROR != MPI_SUCCESS) 
                  mpi_errno = starray[j].MPI_ERROR;
          }
      }
  
      FREE(starray);
      FREE(reqarray);
  }

  else {
      /* Long message. Use pairwise exchange. If comm_size is a
         power-of-two, use exclusive-or to create pairs. Else send
         to rank+i, receive from rank-i. */

      /* Is comm_size a power-of-two? */
      i = 1;
      while (i < size)
          i *= 2;
      if (i == size)
          pof2 = 1;
      else 
          pof2 = 0;

      /* The i=0 case takes care of moving local data into recvbuf */
      for (i=0; i<size; i++) {
          if (pof2 == 1) {
              /* use exclusive-or algorithm */
              src = dst = rank ^ i;
          }
          else {
              src = (rank - i + size) % size;
              dst = (rank + i) % size;
          }
          src = (rank - i + size) % size;
          dst = (rank + i) % size;
          mpi_errno = MPI_Sendrecv(((char *)sendbuf +
                                    dst*sendcount*sendtype_extent), 
                                   sendcount, sendtype->self, dst,
                                   MPIR_ALLTOALL_TAG, 
                                   ((char *)recvbuf +
                                    src*recvcnt*recvtype_extent),
                                   recvcnt, recvtype->self, src,
                                   MPIR_ALLTOALL_TAG, comm->self, &status);
          if (mpi_errno) return mpi_errno;
      }
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}



/* This is the default implementation of alltoallv. The algorithm is:
   
   Algorithm: MPI_Alltoallv

   Since each process sends/receives different amounts of data to
   every other process, we don't know the total message size for all
   processes without additional communication. Therefore we simply use
   the "middle of the road" isend/irecv algorithm that works
   reasonably well in all cases.

   We post all irecvs and isends and then do a waitall. We scatter the
   order of sources and destinations among the processes, so that all
   processes don't try to send/recv to/from the same process at the
   same time.

   Possible improvements: 

   End Algorithm: MPI_Alltoallv
*/


static int intra_Alltoallv ( 
	void *sendbuf, 
	int *sendcnts, 
	int *sdispls, 
	struct MPIR_DATATYPE *sendtype, 
	void *recvbuf, 
	int *recvcnts, 
	int *rdispls, 
	struct MPIR_DATATYPE *recvtype, 
	struct MPIR_COMMUNICATOR *comm )
{
  int        size, i, j, rcnt, dest, rank;
  MPI_Aint   send_extent, recv_extent;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status *starray;
  MPI_Request *reqarray;
  
  /* Get size and switch to collective communicator */
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
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALLV" );

  MPIR_ALLOC(reqarray,(MPI_Request *)MALLOC(2*size*sizeof(MPI_Request)),
	     comm, MPI_ERR_EXHAUSTED, "MPI_ALLTOALLV" );

  /* do the communication -- post *all* sends and receives: */
  rcnt = 0;
  for ( i=0; i<size; i++ ) { 
      dest = (rank+i) % size;
      if (( mpi_errno=MPI_Irecv(
	                  (void *)((char *)recvbuf+rdispls[dest]*recv_extent), 
                           recvcnts[dest], 
                           recvtype->self,
                           dest,
                           MPIR_ALLTOALLV_TAG,
                           comm->self,
                           &reqarray[i]))
          )
          break;
      rcnt++;
  }

  if (!mpi_errno) {
      for ( i=0; i<size; i++ ) { 
          dest = (rank+i) % size;
          if (( mpi_errno=MPI_Isend(
                     (void *)((char *)sendbuf+sdispls[dest]*send_extent), 
                     sendcnts[dest], 
                     sendtype->self,
                     dest,
                     MPIR_ALLTOALLV_TAG,
                     comm->self,
                     &reqarray[i+size]))
           )
              break;
          rcnt++;
      }
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
      if (mpi_errno == MPI_ERR_IN_STATUS) {
	  for (j=0; j<2*size; j++) {
	      if (starray[j].MPI_ERROR != MPI_SUCCESS) 
		  mpi_errno = starray[j].MPI_ERROR;
	  }
      }
  }
  
  /* clean up */
  FREE(reqarray);
  FREE(starray);

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);

  return (mpi_errno);
}


/* This is the default implementation of reduce. The algorithm is:
   
   Algorithm: MPI_Reduce

   We use a minimum spanning tree (MST) algorithm for both short and
   long messages. 

   Cost = lgp.alpha + n.lgp.beta + n.lgp.gamma

   Possible improvements: 

   End Algorithm: MPI_Reduce
*/

static int intra_Reduce ( 
	void *sendbuf, 
	void *recvbuf, 
	int count, 
	struct MPIR_DATATYPE *datatype, 
	MPI_Op op, 
	int root, 
	struct MPIR_COMMUNICATOR *comm )
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

  /* Is root within the communicator? */
  MPIR_Comm_size ( comm, &size );
#ifndef MPIR_NO_ERROR_CHECKING
    if ( root >= size )
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG,
				    myname,(char *)0,(char *)0,root,size);
    if (root < 0) 
	mpi_errno = MPIR_Err_setmsg(MPI_ERR_ROOT,MPIR_ERR_DEFAULT,myname,
				    (char *)0,(char *)0,root);
    if (mpi_errno)
	return MPIR_ERROR(comm, mpi_errno, myname );
#endif

  /* See the overview in Collection Operations for why this is ok */
  if (count == 0) return MPI_SUCCESS;

  /* If the operation is predefined, we could check that the datatype's
     type signature is compatible with the operation.  
   */
#if defined(MPID_Reduce)  &&  !defined(TOPOLOGY_INTRA_FNS_H)
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
	     "MPI_REDUCE" );
  buffer = (void *)((char*)buffer - lb);

  /* If I'm not the root, then my recvbuf may not be valid, therefore
     I have to allocate a temporary one */
  if (rank != root) {
      MPIR_ALLOC(recvbuf,(void *)MALLOC(m_extent * count),
		 comm, MPI_ERR_EXHAUSTED, "MPI_REDUCE" );
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
      /* PRINTF( "Error in performing MPI_Op in reduce\n" ); */
      mpi_errno = MPIR_Op_errno;
  }

  /* Unlock for collective operation */
  MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  
  return (mpi_errno);
}



/* This is the default implementation of allreduce. The algorithm is:
   
   Algorithm: MPI_Allreduce

   For the homogeneous case, we use a recursive doubling algorithm
   (similar to the one in MPI_Allgather) for both short and long messages.

   Cost = lgp.alpha + n.lgp.beta + n.lgp.gamma

   For the heterogeneous case, we call MPI_Reduce followed by MPI_Bcast
   in order to meet the requirement that all processes must have the
   same result.

   Possible improvements: 

   End Algorithm: MPI_Allreduce
*/

static int intra_Allreduce ( 
	void *sendbuf, 
	void *recvbuf, 
	int count, 
	struct MPIR_DATATYPE *datatype, 
	MPI_Op op, 
	struct MPIR_COMMUNICATOR *comm )
{
  int rc, is_homogeneous;
  int        size, rank;
  int        mpi_errno = MPI_SUCCESS;
  MPI_Status status;
  int mask, dst, dst_tree_root, my_tree_root, nprocs_completed, k, i,
      j, tmp_mask, tree_root; 
  MPI_Aint extent, lb;
  void *tmp_buf;
  MPI_User_function *uop;
  struct MPIR_OP *op_ptr;
  static char myname[] = "MPI_ALLREDUCE";

  is_homogeneous = 1;
#ifdef MPID_HAS_HETERO  
  is_homogeneous = (comm->msgform == MPID_MSG_OK) ? 1 : 0;
#endif
#ifdef MPID_DOES_DATACONV
  is_homogeneous = 0;   /* Globus */
#endif

  if (count == 0) return MPI_SUCCESS;

  if (!is_homogeneous) {
      /* heterogeneous. To get the same result on all processes, we
         do a reduce to 0 and then broadcast. */
      mpi_errno = MPI_Reduce ( sendbuf, recvbuf, count, datatype->self, op, 0, 
                               comm->self );
      if (mpi_errno == MPIR_ERR_OP_NOT_DEFINED || mpi_errno == MPI_SUCCESS) {
          rc = MPI_Bcast  ( recvbuf, count, datatype->self, 0, comm->self );
          if (rc) mpi_errno = rc;
      }
  }
  else {
      /* homogeneous. Use recursive doubling algorithm similar to the
         one used in all_gather */

      MPIR_Comm_size(comm, &size);
      MPIR_Comm_rank(comm, &rank);

      /* Switch communicators to the hidden collective */
      comm = comm->comm_coll;
 
      op_ptr = MPIR_GET_OP_PTR(op);
      MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
      uop  = op_ptr->op;

      /* need to allocate temporary buffer to store incoming data*/
      MPI_Type_extent(datatype->self, &extent);
      MPIR_ALLOC(tmp_buf,(void *)MALLOC(count*extent), comm,
                 MPI_ERR_EXHAUSTED, myname);
      /* adjust for potential negative lower bound in datatype */
      MPI_Type_lb( datatype->self, &lb );
      tmp_buf = (void *)((char*)tmp_buf - lb);

      /* Lock for collective operation */
      MPID_THREAD_LOCK(comm->ADIctx,comm);

      /* copy local data into recvbuf */
      mpi_errno = MPI_Sendrecv ( sendbuf, count, datatype->self,
                                 rank, MPIR_ALLREDUCE_TAG, 
                                 recvbuf, count, datatype->self,
                                 rank, MPIR_ALLREDUCE_TAG,
                                 comm->self, &status );
      if (mpi_errno) return mpi_errno;

      mask = 0x1;
      i = 0;
      while (mask < size) {
          dst = rank ^ mask;

          dst_tree_root = dst >> i;
          dst_tree_root <<= i;
      
          my_tree_root = rank >> i;
          my_tree_root <<= i;
      
          if (dst < size) {
              /* Send most current data, which is in recvbuf. Recv
                 into tmp_buf */ 
              mpi_errno = MPI_Sendrecv(recvbuf, count, datatype->self,
                                       dst, MPIR_ALLREDUCE_TAG, tmp_buf,
                                       count, datatype->self, dst,
                                       MPIR_ALLREDUCE_TAG, comm->self,
                                       &status); 
              if (mpi_errno) return mpi_errno;
 
              /* tmp_buf contains data received in this step.
                 recvbuf contains data accumulated so far */
      
              if ((op_ptr->commute) || (dst_tree_root < my_tree_root))
                  (*uop)(tmp_buf, recvbuf, &count, &datatype->self);
              else {
                  (*uop)(recvbuf, tmp_buf, &count, &datatype->self);
                  /* copy result back into recvbuf */
                  mpi_errno = MPI_Sendrecv(tmp_buf, count, datatype->self,
                                           rank, MPIR_ALLREDUCE_TAG, recvbuf,
                                           count, datatype->self, rank,
                                           MPIR_ALLREDUCE_TAG, comm->self,
                                           &status); 
                  if (mpi_errno) return mpi_errno;
              }
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             result. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;

              tmp_mask = mask >> 1;
              while (tmp_mask) {
                  dst = rank ^ tmp_mask;
                  
                  tree_root = rank >> k;
                  tree_root <<= k;
              
                  /* send only if this proc has data and destination
                     doesn't have data. at any step, multiple processes
                     can send if they have the data */
                  if ((dst > rank) && 
                      (rank < tree_root + nprocs_completed)
                      && (dst >= tree_root + nprocs_completed)) {
                      /* send the current result */
                      mpi_errno = MPI_Send(recvbuf, count, datatype->self,
                                           dst, MPIR_ALLREDUCE_TAG,
                                           comm->self);  
                      if (mpi_errno) return mpi_errno;
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((dst < rank) && 
                           (dst < tree_root + nprocs_completed) &&
                           (rank >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Recv(recvbuf, count, datatype->self,
                                           dst, MPIR_ALLREDUCE_TAG, comm->self,
                                           &status); 
                      if (mpi_errno) return mpi_errno;
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }
          mask <<= 1;
          i++;
      }

      FREE((char *)tmp_buf+lb); 

      /* Unlock for collective operation */
      MPID_THREAD_UNLOCK(comm->ADIctx,comm);
  }

  return (mpi_errno);
}


/* This is the default implementation of reduce_scatter. The algorithm is:
   
   Algorithm: MPI_Reduce_scatter

   For long messages, we use a pairwise exchange algorithm similar to
   the one used in MPI_Alltoall. At step i, each process sends n/p
   amount of data to (rank+i) and receives n/p amount of data from 
   (rank-i).

   Cost = (p-1).alpha + n.((p-1)/p).beta + n.((p-1)/p).gamma

   For short messages, we use a recursive doubling algorithm, which
   takes lgp steps. At step 1, processes exchange (n-n/p) amount of
   data; at step 2, (n-2n/p) amount of data; at step 3, (n-4n/p)
   amount of data, and so forth.

   Cost = lgp.alpha + n.(lgp-(p-1)/p).beta + n.(lgp-(p-1)/p).gamma

   Possible improvements: 

   End Algorithm: MPI_Reduce_scatter
*/


static int intra_Reduce_scatter ( 
	void *sendbuf, 
	void *recvbuf, 
	int *recvcnts, 
	struct MPIR_DATATYPE *datatype, 
	MPI_Op op, 
	struct MPIR_COMMUNICATOR *comm )
{
  int   rank, size, i;
  MPI_Aint extent, lb; 
  int  *displs;
  void *tmp_recvbuf, *tmp_results;
  int   mpi_errno = MPI_SUCCESS;
  int type_size, dis[2], blklens[2], total_count, nbytes, src, dst;
  int mask, dst_tree_root, my_tree_root, j, k;
  MPI_Datatype sendtype, recvtype;
  int nprocs_completed, tmp_mask, tree_root, received;
  MPI_User_function *uop;
  struct MPIR_OP *op_ptr;
  MPI_Status status;
  static char myname[] = "MPI_REDUCE_SCATTER";
#ifdef OLD
  int rc;
  void *buffer;
  MPI_Aint m_extent, ub;
#endif

  MPI_Type_size(datatype->self, &type_size);
  MPI_Type_extent(datatype->self, &extent);
  MPI_Type_lb( datatype->self, &lb );

  op_ptr = MPIR_GET_OP_PTR(op);
  MPIR_TEST_MPI_OP(op,op_ptr,comm,myname);
  uop  = op_ptr->op;

  MPIR_Comm_size(comm, &size);
  MPIR_Comm_rank(comm, &rank);
  comm = comm->comm_coll;

  MPIR_ALLOC(displs,(int *)MALLOC(size*sizeof(int)),comm, MPI_ERR_EXHAUSTED, 
             myname);
  total_count = 0;
  for (i=0; i<size; i++) {
      displs[i] = total_count;
      total_count += recvcnts[i];
  }

  nbytes = total_count * type_size;

  MPID_THREAD_LOCK(comm->ADIctx,comm);

  if (nbytes > MPIR_REDUCE_SCATTER_SHORT_MSG) {
      /* for long messages, use (p-1) pairwise exchanges */ 

      /* copy local data into recvbuf */
      mpi_errno = MPI_Sendrecv (((char *)sendbuf+displs[rank]*extent),
                                recvcnts[rank], datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, recvbuf,
                                recvcnts[rank], datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, comm->self,
                                &status); 
      if (mpi_errno) return mpi_errno;
 
      /* allocate temporary buffer to store incoming data */
      MPIR_ALLOC(tmp_recvbuf,(int *)MALLOC(extent*recvcnts[rank]),comm,
                 MPI_ERR_EXHAUSTED, myname);
      /* adjust for potential negative lower bound in datatype */
      tmp_recvbuf = (void *)((char*)tmp_recvbuf - lb);

      for (i=1; i<size; i++) {
          src = (rank - i + size) % size;
          dst = (rank + i) % size;

          /* send the data that dst needs. recv data that this process
             needs from src into tmp_recvbuf */
          mpi_errno = MPI_Sendrecv(((char *)sendbuf+displs[dst]*extent), 
                                   recvcnts[dst], datatype->self, dst,
                                   MPIR_REDUCE_SCATTER_TAG, tmp_recvbuf,
                                   recvcnts[rank], datatype->self, src,
                                   MPIR_REDUCE_SCATTER_TAG, comm->self,
                                   &status);  
          if (mpi_errno) return mpi_errno;

          if ((op_ptr->commute) || (src < rank))
              (*uop)(tmp_recvbuf, recvbuf, &recvcnts[rank], &datatype->self); 
          else {
              (*uop)(recvbuf, tmp_recvbuf, &recvcnts[rank], &datatype->self); 
              /* copy result back into recvbuf */
              mpi_errno = MPI_Sendrecv (tmp_recvbuf,
                                        recvcnts[rank], datatype->self, rank,
                                        MPIR_REDUCE_SCATTER_TAG, recvbuf,
                                        recvcnts[rank], datatype->self, rank,
                                        MPIR_REDUCE_SCATTER_TAG, comm->self,
                                        &status); 
              if (mpi_errno) return mpi_errno;
          }
      }

      FREE((char *)tmp_recvbuf+lb); 
  }

  else {
      /* for short messages, use recursive doubling. */

      /* need to allocate temporary buffer to receive incoming data*/
      MPIR_ALLOC(tmp_recvbuf,(void *)MALLOC(extent*total_count), comm,
                 MPI_ERR_EXHAUSTED, myname);
      /* adjust for potential negative lower bound in datatype */
      tmp_recvbuf = (void *)((char*)tmp_recvbuf - lb);

      /* need to allocate another temporary buffer to accumulate
         results */
      MPIR_ALLOC(tmp_results,(void *)MALLOC(extent*total_count), comm,
                 MPI_ERR_EXHAUSTED, myname);
      /* adjust for potential negative lower bound in datatype */
      tmp_results = (void *)((char*)tmp_results - lb);

      /* copy sendbuf into tmp_results */
      mpi_errno = MPI_Sendrecv (sendbuf, total_count, datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, tmp_results,
                                total_count, datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, comm->self,
                                &status); 
      if (mpi_errno) return mpi_errno;

      mask = 0x1;
      i = 0;
      while (mask < size) {
          dst = rank ^ mask;

          dst_tree_root = dst >> i;
          dst_tree_root <<= i;
      
          my_tree_root = rank >> i;
          my_tree_root <<= i;

          /* At step 1, processes exchange (n-n/p) amount of
             data; at step 2, (n-2n/p) amount of data; at step 3, (n-4n/p)
             amount of data, and so forth. We use derived datatypes for this.

             At each step, a process does not need to send data
             indexed from my_tree_root to
             my_tree_root+mask-1. Similarly, a process won't receive
             data indexed from dst_tree_root to dst_tree_root+mask-1. */

          /* calculate sendtype */
          blklens[0] = blklens[1] = 0;
          for (j=0; j<my_tree_root; j++)
              blklens[0] += recvcnts[j];
          for (j=my_tree_root+mask; j<size; j++)
              blklens[1] += recvcnts[j];

          dis[0] = 0;
          dis[1] = blklens[0];
          for (j=my_tree_root; (j<my_tree_root+mask) && (j<size); j++)
              dis[1] += recvcnts[j];

          MPI_Type_indexed(2, blklens, dis, datatype->self, &sendtype);
          MPI_Type_commit(&sendtype);

          /* calculate recvtype */
          blklens[0] = blklens[1] = 0;
          for (j=0; j<dst_tree_root; j++)
              blklens[0] += recvcnts[j];
          for (j=dst_tree_root+mask; j<size; j++)
              blklens[1] += recvcnts[j];

          dis[0] = 0;
          dis[1] = blklens[0];
          for (j=dst_tree_root; (j<dst_tree_root+mask) && (j<size); j++)
              dis[1] += recvcnts[j];

          MPI_Type_indexed(2, blklens, dis, datatype->self, &recvtype);
          MPI_Type_commit(&recvtype);

          received = 0;

          if (dst < size) {
              /* tmp_results contains data to be sent in each step. Data is
                 received in tmp_recvbuf and then accumulated into
                 tmp_results. the accumulation is done later below. */ 
      
              mpi_errno = MPI_Sendrecv(tmp_results, 1, sendtype, dst,
                                       MPIR_REDUCE_SCATTER_TAG, tmp_recvbuf,
                                       1, recvtype, dst,
                                       MPIR_REDUCE_SCATTER_TAG, comm->self,
                                       &status); 
              received = 1;
              if (mpi_errno) return mpi_errno;
          }

          /* if some processes in this process's subtree in this step
             did not have any destination process to communicate with
             because of non-power-of-two, we need to send them the
             result. We use a logarithmic recursive-halfing algorithm
             for this. */

          if (dst_tree_root + mask > size) {
              nprocs_completed = size - my_tree_root - mask;
              /* nprocs_completed is the number of processes in this
                 subtree that have all the data. Send data to others
                 in a tree fashion. First find root of current tree
                 that is being divided into two. k is the number of
                 least-significant bits in this process's rank that
                 must be zeroed out to find the rank of the root */ 
              j = mask;
              k = 0;
              while (j) {
                  j >>= 1;
                  k++;
              }
              k--;

              tmp_mask = mask >> 1;
              while (tmp_mask) {
                  dst = rank ^ tmp_mask;
                  
                  tree_root = rank >> k;
                  tree_root <<= k;
              
                  /* send only if this proc has data and destination
                     doesn't have data. at any step, multiple processes
                     can send if they have the data */
                  if ((dst > rank) && 
                      (rank < tree_root + nprocs_completed)
                      && (dst >= tree_root + nprocs_completed)) {
                      /* send the current result */
                      mpi_errno = MPI_Send(tmp_recvbuf, 1, recvtype,
                                           dst, MPIR_REDUCE_SCATTER_TAG,
                                           comm->self);  
                      if (mpi_errno) return mpi_errno;
                  }
                  /* recv only if this proc. doesn't have data and sender
                     has data */
                  else if ((dst < rank) && 
                           (dst < tree_root + nprocs_completed) &&
                           (rank >= tree_root + nprocs_completed)) {
                      mpi_errno = MPI_Recv(tmp_recvbuf, 1, recvtype, dst,
                                           MPIR_REDUCE_SCATTER_TAG,
                                           comm->self, &status); 
                      received = 1;
                      if (mpi_errno) return mpi_errno;
                  }
                  tmp_mask >>= 1;
                  k--;
              }
          }

          /* The following reduction is done here instead of after 
             the MPI_Sendrecv or MPI_Recv above. This is
             because to do it above, in the noncommutative 
             case, we would need an extra temp buffer so as not to
             overwrite temp_recvbuf, because temp_recvbuf may have
             to be communicated to other processes in the
             non-power-of-two case. To avoid that extra allocation,
             we do the reduce here. */
          if (received) {
              if ((op_ptr->commute) || (dst_tree_root < my_tree_root)) {
                  (*uop)(tmp_recvbuf, tmp_results, &blklens[0],
                         &datatype->self); 
                  (*uop)(((char *)tmp_recvbuf + dis[1]*extent),
                         ((char *)tmp_results + dis[1]*extent),
                         &blklens[1], &datatype->self); 
              }
              else {
                  (*uop)(tmp_results, tmp_recvbuf, &blklens[0],
                         &datatype->self); 
                  (*uop)(((char *)tmp_results + dis[1]*extent),
                         ((char *)tmp_recvbuf + dis[1]*extent),
                         &blklens[1], &datatype->self); 
                  /* copy result back into tmp_results */
                  mpi_errno = MPI_Sendrecv(tmp_recvbuf, 1, recvtype, rank,
                                           MPIR_REDUCE_SCATTER_TAG, 
                                           tmp_results, 1, recvtype, rank,
                                           MPIR_REDUCE_SCATTER_TAG, 
                                           comm->self, &status);
                  if (mpi_errno) return mpi_errno;
              }
          }

          MPI_Type_free(&sendtype);
          MPI_Type_free(&recvtype);

          mask <<= 1;
          i++;
      }

      /* now copy final results from tmp_results to recvbuf */
      mpi_errno = MPI_Sendrecv (((char *)tmp_results+displs[rank]*extent),
                                recvcnts[rank], datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, recvbuf,
                                recvcnts[rank], datatype->self, rank,
                                MPIR_REDUCE_SCATTER_TAG, comm->self,
                                &status); 
      if (mpi_errno) return mpi_errno;

      FREE((char *)tmp_recvbuf+lb); 
      FREE((char *)tmp_results+lb); 
  }

  FREE(displs);

  MPID_THREAD_UNLOCK(comm->ADIctx,comm);


#ifdef OLD

  /* Determine the "count" of items to reduce and set the displacements*/
  MPIR_Type_get_limits( datatype, &lb, &ub );
  m_extent = ub - lb;
  /* MPI_Type_extent (datatype, &extent); */
  MPIR_Comm_size   (comm, &size);
  MPIR_Comm_rank   (comm, &rank);

  /* Allocate the displacements and initialize them */
  MPIR_ALLOC(displs,(int *)MALLOC(size*sizeof(int)),comm, MPI_ERR_EXHAUSTED, 
			 myname);

  total_count = 0;
  for (i=0;i<size;i++) {
    displs[i] = total_count;
    total_count += recvcnts[i];
    if (recvcnts[i] < 0) {
	FREE( displs );
	mpi_errno = MPIR_Err_setmsg( MPI_ERR_COUNT, MPIR_ERR_COUNT_ARRAY_NEG,
				     myname, (char *)0, (char *)0,
				     i, recvcnts[i] );
	return mpi_errno;
    }
  }

  /* Allocate a temporary buffer */
  if (total_count == 0) {
      FREE( displs );
      return MPI_SUCCESS;
      }

  MPIR_ALLOC(buffer,(void *)MALLOC(m_extent*total_count), comm, MPI_ERR_EXHAUSTED, 
			 myname);
  buffer = (void *)((char*)buffer - lb);

  /* Reduce to 0, then scatter */
  mpi_errno = MPI_Reduce   ( sendbuf, buffer, total_count, datatype->self, op, 0, 
			     comm->self);
  if (mpi_errno == MPI_SUCCESS || mpi_errno == MPIR_ERR_OP_NOT_DEFINED) {
      rc = MPI_Scatterv ( buffer, recvcnts, displs, datatype->self,
			  recvbuf, recvcnts[rank], datatype->self, 0,
			  comm->self );
      if (rc) mpi_errno = rc;
  }
  /* Free the temporary buffers */
  FREE((char *)buffer+lb); FREE(displs);
#endif

  return (mpi_errno);
}


/* New scan is defined in intra_scan.c */
#ifdef MPIR_USE_BASIC_COLL
static int intra_Scan ( 
	void *sendbuf, 
	void *recvbuf, 
	int count, 
	struct MPIR_DATATYPE *datatype, 
	MPI_Op op, 
	struct MPIR_COMMUNICATOR *comm )
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
		     comm, MPI_ERR_EXHAUSTED, "MPI_SCAN" );
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
#endif
