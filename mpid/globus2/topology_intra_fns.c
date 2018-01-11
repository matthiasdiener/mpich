
#include "mpid.h"
#include "mpiimpl.h"
#include "mem.h"
#include "coll.h"
#include "mpiops.h"


/*********************/
/* public prototypes */
/*********************/

#include "topology_intra_fns.h"


/*************************************************************************/
/* the library routines should call the profiling                        */
/* versions of the functions (PMPI_...) to ensure that only any user     */
/* code to catch an MPI function only uses the PMPI functions.  Here is  */
/* a partial solution.  In the weak symbol case, we simply change all of */
/* the routines to their PMPI versions.                                  */
/*************************************************************************/

/* Include mapping from MPI->PMPI */
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
/* Insert the prototypes for the PMPI routines */
#undef __MPI_BINDINGS
#include "binding.h"


/**********************/
/* private prototypes */
/**********************/


/* handle the cluster ID table */
static void update_cluster_id_table (int, struct MPIR_COMMUNICATOR *);
static int is_a_master(int, int *, int);

/* various utility functions */
static int previous_power_of_2(int, int*);
static int proc_to_index(int, int **, int);
static void copy_buf(void *, int, void *, int, int);
static void pack_dependencies(void *, int, MPI_Datatype, int, int, MPI_Comm,
                              void *, int, int *, int *, int **, int, int,
                              int);
static void unpack_dependencies(void *, int, int *, void *, int, int, int *,
                                int **, int, MPI_Datatype, MPI_Comm, int, int,
                                int);

/* make the sets of procs involved in the communication at different levels */
static void make_set(int, struct MPIR_COMMUNICATOR *, single_set_t *);
static void involve_sets(struct MPIR_COMMUNICATOR *, multiple_set_t *);

/* the sub-routines for the Barrier */
static int hypercube_barrier(single_set_t, MPI_Request *, int *, MPI_Comm);
static int flat_tree_barrier(single_set_t, MPI_Request *, int *, MPI_Comm);
static int wait_barrier(MPI_Request *, int);

/* the sub-routines for the BroadCast */
static int flat_tree_bcast(void *, int, MPI_Datatype, MPI_Comm, single_set_t,
                           MPI_Request *, int *);
static int binomial_bcast(void *, int, MPI_Datatype, MPI_Comm, single_set_t,
                          MPI_Request *, int *);

/* the sub-routines for the Gather */
static int flat_tree_gather(single_set_t, void *, int, int, int, MPI_Datatype,
                            MPI_Comm, int *, int **, void *, int, int, int,
                            MPI_Datatype, int);
static int binomial_gather(single_set_t, void *, int, int, int, MPI_Datatype,
                           MPI_Comm, int *, int **, void *, int, int, int,
                           MPI_Datatype, int);

/* the sub-routines for the Scatter */
static int flat_tree_scatter(single_set_t, void *, int, int, int, MPI_Datatype,
                             MPI_Comm, int *, int **, void *, int, int, int,
                             MPI_Datatype, int);
static int binomial_scatter(single_set_t, void *, int, int, int, MPI_Datatype,
                            MPI_Comm, int *, int **, void *, int, int, int,
                            MPI_Datatype, int);

/* the sub-routines for the Reduce */
static int flat_tree_reduce(single_set_t, void *, int, int, int, MPI_Datatype,
                            MPI_Comm, MPI_User_function *, void *);
static int hypercube_reduce(single_set_t, void *, int, MPI_Datatype, MPI_Comm,
                            int, int, MPI_User_function *, void *);


/********************/
/* public functions */
/********************/

/******** MPID_FN_Barrier *********************************************/

#ifdef MPID_Barrier
int
MPID_FN_Barrier (struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, lvl;
   multiple_set_t set_info;
   MPI_Request *req;
   int n_req, n_set;


   /* Intialize communicator size */
   (void) MPIR_Comm_size (comm, &size);

   /* If there's only one member, this is trivial */
   if ( size == 1 )
      return MPI_SUCCESS;

   /* NICK: Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   involve_sets(comm, &set_info);
   n_set = set_info.num;

   for (n_req = 0, lvl = 0; lvl < n_set; lvl++)
      n_req += set_info.sets[lvl].size;
   /* n_req == 0 is impossible here */
   req = (MPI_Request *) globus_libc_malloc (2 * n_req * sizeof(MPI_Request));
   if ( !req )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "MPID_FN_Barrier() - failed malloc");

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

   /* Note: for high-latency networks (WAN TCP), a flat tree is more
    * efficient (faster) than a hypercube algo below a certain number
    * of processes (depending on the ratio between the overhead to send
    * or recv a msg and the latency */

   /* this code could be used to implement a "fuzzy barrier" */
   /* start of ENTER_FUZZY_BARRIER */
   /* enter the barrier (tell my neighbors I reached the barrier) */
   for (n_req = 0, lvl = 0; lvl < n_set; lvl++)
   {
      single_set_t set = set_info.sets[lvl];

      if ( set.level == 0 )   /* WAN TCP */
         mpi_errno = flat_tree_barrier(set, req, &n_req, comm->self);
      else
         mpi_errno = hypercube_barrier(set, req, &n_req, comm->self);
      if ( mpi_errno ) goto clean_exit;
   }
   /* end of ENTER_FUZZY_BARRIER */

   /* start of EXIT_FUZZY_BARRIER */
   /* wait for my neighbors to enter the barrier */
   mpi_errno = wait_barrier(req, n_req);
   if ( mpi_errno ) goto clean_exit;

   /* exit the barrier (tell my neighbors they can go on working) */
   for (n_req = 0, lvl = 0; lvl < n_set; lvl++)
   {
      mpi_errno = hypercube_barrier(set_info.sets[lvl], req, &n_req,
                                    comm->self);
      if ( mpi_errno ) goto clean_exit;
   }   /* endfor */

   /* exit the barrier (waiting again) */
   mpi_errno = wait_barrier(req, n_req);
   /* end of EXIT_FUZZY_BARRIER */

clean_exit:
   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   g_free(req);
   /* do not free "set_info.sets[...].set": they are pre-allocated pointers */
   /* do not free "set_info.sets": it is a pre-allocated pointer */

   return (mpi_errno);
}   /* MPID_FN_Barrier */
#endif   /* MPID_Barrier */

/******** MPID_FN_Bcast ***********************************************/

#ifdef MPID_Bcast
int
MPID_FN_Bcast (void *buffer, int count, struct MPIR_DATATYPE *datatype,
               int root, struct MPIR_COMMUNICATOR *comm)
{
   multiple_set_t set_info;
   int set, size, n_req, n_set;
   int mpi_errno = MPI_SUCCESS;
   static char myname[] = "MPI_BCAST";
   MPI_Request *req;
   MPI_Status *status;

   /* Is root within the comm and more than 1 processes involved? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if (root >= size) 
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, 
                                   myname, (char *)0, (char *)0, root, size);
   else if (root < 0)
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char*)0, root);
   if (mpi_errno)
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* If there is only one process or nothing to broadcast... */
   if ( size == 1  ||  count == 0 )
      return (mpi_errno);

   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   /* Algorithm:
    * This uses the cluster ids to guess to which processes to send
    * the message first (starting with the processes you can touch
    * with the slowest protocol, i.e.: WAN TCP).  At each level
    * corresponding to a given protocol, a root process broadcasts
    * the message to the masters of the other clusters at the same
    * level using the binomial tree algorithm. */

   /* Note: for high-latency networks (WAN TCP), a flat tree algo is better */

   /* first we 'rename' the clusters at each level so that the root
    * process has only zeros as cluster ids (at each level). */
   update_cluster_id_table(root, comm);
   involve_sets(comm, &set_info);
   n_set = set_info.num;

   for (n_req = 0, set = 0; set < n_set; set++)
      n_req += set_info.sets[set].size - 1;
   /* n_req == 0 is impossible here, because count != 0 at this point:
    * see test above. */
   req = (MPI_Request *) globus_libc_malloc (n_req * sizeof(MPI_Request));
   if ( !req )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "MPID_FN_Bcast() - failed malloc");

   /* Lock for collective operation */
   MPID_THREAD_LOCK (comm->ADIctx,comm);

   for (n_req = 0, set = 0; set < n_set; set++)
   {
      single_set_t current_set = set_info.sets[set];

      if ( current_set.level == 0 )   /* WAN-TCP: flat-tree algorithm */
         mpi_errno = flat_tree_bcast(buffer, count, datatype->self, comm->self,
                                     current_set, req, &n_req);
      else   /* binomial tree algorithm */
         mpi_errno = binomial_bcast(buffer, count, datatype->self, comm->self,
                                    current_set, req, &n_req);
      if (mpi_errno) break;
   }   /* endfor */

   if ( n_req )
   {
      status = (MPI_Status *) globus_libc_malloc (n_req * sizeof(MPI_Status));
      if ( !status )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "MPID_FN_Bcast() - failed malloc");
      mpi_errno = MPI_Waitall(n_req, req, status);
      g_free(status);
   }

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   g_free(req);
   /* do not free "set_info.sets[...].set": they are pre-allocated pointers */
   /* do not free "set_info.sets": it is a pre-allocated pointer */

   return (mpi_errno);
}   /* MPID_FN_Bcast */
#endif   /* MPID_Bcast */

/******** MPID_FN_Gather **********************************************/

#ifdef MPID_Gather
int
MPID_FN_Gather (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
                void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype,
                int root, struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int stride, size, rank, set, buf_elts, init_lvl;
   static char myname[] = "MPI_GATHER";
   multiple_set_t set_info;
   MPI_Aint lb, ub, extent;
   MPI_Status status;
   void *tmp_buf;

   /* Is root within the communicator? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if ( root >= size )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, myname,
                                   (char *)0, (char *)0, root, size);
   else if ( root < 0 )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char *)0, root);
   if ( mpi_errno )
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* Get my rank and switch communicators to the hidden collective */
   (void) MPIR_Comm_rank (comm, &rank);
   comm = comm->comm_coll;

   /* first we 'rename' the clusters at each level so that the root
    * process has only zeros as cluster ids (at each level). */
   update_cluster_id_table(root, comm);
   involve_sets(comm, &set_info);

   /* p: # of procs; o: overhead to send/recv; l: latency
    * flat tree: time = o*p + l
    * binomial tree: time = (l+2*o) * ceil(log p)
    * If l>>o: for large values of p, binomial tree is more efficient;
    *          for small values of p, flat tree is faster;
    * If l~=o: idem, but the threshold for 'p' is lower. */

   /* depending on the msg size (sendcnt * extent * size), it might be
    * more efficient:
    *  - to take advantage of the protocol levels in case of small msg
    *    size (because there are memory copies),
    *  - or to ignore the protocol levels in case of large msg size
    *    (when the memory copies would take more time than latencies). */

   /* MPI standard: recvbuf, recvcnt, recvtype are not significant for
    * procs != root.  Allocate a temporary buffer to store the data I have
    * to relay. */
   if ( rank != root )
   {
      recvcnt = sendcnt;
      recvtype = sendtype;
   }
   MPIR_Type_get_limits(recvtype, &lb, &ub);
   extent = ub - lb;
   stride = extent * recvcnt;
   init_lvl = set_info.sets[0].level;
   buf_elts = recvcnt * comm->Topology_ClusterSizes[init_lvl];
   /* allocate a buffer for the data I recv before bouncing it */
   if ( rank != root )
   {
      recvbuf = (void *) globus_libc_malloc (extent * buf_elts);
      if ( !recvbuf )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "MPID_FN_Gather() - failed malloc");
      recvbuf = (void *) ((char *) recvbuf - lb);
   }   /* endif */

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

   /* MPI_Sendrecv should be optimized in case source == destination */
   mpi_errno = MPI_Sendrecv(sendbuf, sendcnt, sendtype->self, rank,
                            MPIR_GATHER_TAG, (char*) recvbuf + stride
                    * proc_to_index(rank, comm->Topology_ColorTable, init_lvl),
                            recvcnt, recvtype->self, rank, MPIR_GATHER_TAG,
                            comm->self, &status);
   if ( mpi_errno ) goto clean_exit;

   /* allocate a temporary buffer to recv the data, before copying it into
    * recvbuf */
   tmp_buf = (void *) globus_libc_malloc (extent * buf_elts);
   if ( !tmp_buf )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "MPID_FN_Gather() - failed malloc");
   tmp_buf = (void *) ((char *) tmp_buf - lb);

   for (set = set_info.num-1; set >= 0; set--)
   {
      single_set_t current_set = set_info.sets[set];

      if ( current_set.level == 0 )   /* WAN TCP: flat tree */
         mpi_errno = flat_tree_gather(current_set, tmp_buf, buf_elts * extent,
                                      init_lvl, sendcnt, sendtype->self,
                                      comm->self, comm->Topology_Depths,
                                      comm->Topology_ColorTable, recvbuf,
                                      stride, size, lb, recvtype->self,
                                      recvcnt);
      else   /* binomial tree algorithm */
         mpi_errno = binomial_gather(current_set, tmp_buf, buf_elts * extent,
                                     init_lvl, sendcnt, sendtype->self,
                                     comm->self, comm->Topology_Depths,
                                     comm->Topology_ColorTable, recvbuf,
                                     stride, size, lb, recvtype->self,
                                     recvcnt);
      if ( mpi_errno ) break;
   }   /* endfor */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   g_free(tmp_buf);
clean_exit:
   if ( rank != root )
      g_free(recvbuf);

   return (mpi_errno);
}   /* MPID_FN_Gather */
#endif   /* MPID_Gather */

/******** MPID_FN_Gatherv *********************************************/

#ifdef MPID_Gatherv
int
MPID_FN_Gatherv (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
                 void *recvbuf, int *recvcnts, int *displs,
                 struct MPIR_DATATYPE *recvtype, int root,
                 struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;
   static char myname[] = "MPI_GATHERV";

   /* Is root within the communicator? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if ( root >= size )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, myname,
                                   (char *)0, (char *)0, root, size);
   else if ( root < 0 )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char *)0, root);
   if ( mpi_errno )
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* Get my rank and switch communicators to the hidden collective */
   (void) MPIR_Comm_rank (comm, &rank);
   comm = comm->comm_coll;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

/* do something here! */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Gatherv */
#endif   /* MPID_Gatherv */

/******** MPID_FN_Scatter *********************************************/

#ifdef MPID_Scatter
int
MPID_FN_Scatter (void *sendbuf, int sendcnt, struct MPIR_DATATYPE *sendtype,
                 void *recvbuf, int recvcnt, struct MPIR_DATATYPE *recvtype,
                 int root, struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int stride, size, rank, buf_elts, init_lvl, set, n_set;
   static char myname[] = "MPI_SCATTER";
   multiple_set_t set_info;
   MPI_Aint extent, lb, ub;
   MPI_Status status;
   void *tmp_buf;

   /* Is root within the communicator? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if ( root >= size )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, myname,
                                   (char *)0, (char *)0, root, size);
   else if ( root < 0 )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char *)0, root);
   if ( mpi_errno )
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* Get my rank and switch communicators to the hidden collective */
   (void) MPIR_Comm_rank (comm, &rank);
   comm = comm->comm_coll;

   /* rename the clusters and create the communication sets of processes */
   update_cluster_id_table(root, comm);
   involve_sets(comm, &set_info);
   n_set = set_info.num;

   /* MPI Standard: 'sendbuf', 'sendcnt', 'sendtype' are significant only
    * at root */
   /* If I'm NOT root, I might recv some data to relay to other processes:
    * allocate memory to store that data */
   if ( rank != root )
   {
      sendtype = recvtype;
      sendcnt = recvcnt;
   }
   MPIR_Type_get_limits(sendtype, &lb, &ub);
   extent = ub - lb;
   stride = sendcnt * extent;
   init_lvl = set_info.sets[0].level;
   buf_elts = sendcnt * comm->Topology_ClusterSizes[init_lvl];
   if ( rank != root )
   {
      sendbuf = (void *) globus_libc_malloc (extent * buf_elts);
      if ( !sendbuf )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "MPID_FN_Scatter() - failed malloc");
      sendbuf = (void *) ((char *) sendbuf - lb);
   }
   /* temporary buffer to store the chunks of data to send */
   tmp_buf = (void *) globus_libc_malloc (extent * buf_elts);
   if ( !tmp_buf )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "MPID_FN_Scatter() - failed malloc");

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

   for (set = 0; set < n_set; set++)
   {
      single_set_t current_set = set_info.sets[set];

      if ( current_set.level == 0 )   /* WAN-TCP ==> flat-tree algorithm */
         mpi_errno = flat_tree_scatter(current_set, tmp_buf, buf_elts * extent,
                                       init_lvl, recvcnt, recvtype->self,
                                       comm->self, comm->Topology_Depths,
                                       comm->Topology_ColorTable, sendbuf,
                                       stride, size, lb, sendtype->self,
                                       sendcnt);
      else   /* low latency ==> binomial-tree algorithm */
         mpi_errno = binomial_scatter(current_set, tmp_buf, buf_elts * extent,
                                      init_lvl, recvcnt, recvtype->self,
                                      comm->self, comm->Topology_Depths,
                                      comm->Topology_ColorTable, sendbuf,
                                      stride, size, lb, sendtype->self,
                                      sendcnt);
      if ( mpi_errno ) break;
   }   /* endfor */

   /* MPI_Sendrecv should be optimized in case source == destination */
   mpi_errno = MPI_Sendrecv((char *) sendbuf + stride
                    * proc_to_index(rank, comm->Topology_ColorTable, init_lvl),
                            sendcnt, sendtype->self, rank, MPIR_SCATTER_TAG,
                            recvbuf, recvcnt, recvtype->self, rank,
                            MPIR_SCATTER_TAG, comm->self, &status);

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   if ( rank != root )
      g_free(sendbuf);
   g_free(tmp_buf);
   return (mpi_errno);
}   /* MPID_FN_Scatter */
#endif   /* MPID_Scatter */

/******** MPID_FN_Scatterv ********************************************/

#ifdef MPID_Scatterv
int
MPID_FN_Scatterv (void *sendbuf, int *sendcnts, int *displs,
                  struct MPIR_DATATYPE *sendtype, void *recvbuf,
                  int recvcnt, struct MPIR_DATATYPE *recvtype, int root,
                  struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;
   static char myname[] = "MPI_SCATTERV";

   /* Is root within the communicator? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if ( root >= size )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, myname,
                                   (char *)0, (char *)0, root, size);
   else if ( root < 0 )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char *)0, root);
   if ( mpi_errno )
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* Get my rank and switch communicators to the hidden collective */
   (void) MPIR_Comm_rank (comm, &rank);
   comm = comm->comm_coll;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

/* do something here! */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Scatterv */
#endif   /* MPID_Scatterv */

/******** MPID_FN_Allgather *******************************************/

#ifdef MPID_Allgather
int
MPID_FN_Allgather (void *sendbuf, int sendcnt,
                   struct MPIR_DATATYPE *sendtype, void *recvbuf,
                   int recvcnt, struct MPIR_DATATYPE *recvtype,
                   struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

/* do something here! */

   return (mpi_errno);
}   /* MPID_FN_Allgather */
#endif   /* MPID_Allgather */

/******** MPID_FN_Allgatherv ******************************************/

#ifdef MPID_Allgatherv
int
MPID_FN_Allgatherv (void *sendbuf, int sendcnt,
                    struct MPIR_DATATYPE *sendtype, void *recvbuf,
                    int *recvcnts, int *displs,
                    struct MPIR_DATATYPE *recvtype,
                    struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

/* do something here! */

   return (mpi_errno);
}   /* MPID_FN_Allgatherv */
#endif   /* MPID_Allgatherv */

/******** MPID_FN_Alltoall ********************************************/

#ifdef MPID_Alltoall
int
MPID_FN_Alltoall (void *sendbuf, int sendcnt,
                  struct MPIR_DATATYPE *sendtype, void *recvbuf,
                  int recvcnt, struct MPIR_DATATYPE *recvtype,
                  struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx, comm);

/* do something here! */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Alltoall */
#endif   /* MPID_Alltoall */

/******** MPID_FN_Alltoallv *******************************************/

#ifdef MPID_Alltoallv
int
MPID_FN_Alltoallv (void *sendbuf, int *sendcnts, int *sdispls,
                   struct MPIR_DATATYPE *sendtype, void *recvbuf,
                   int *recvcnts, int *rdispls,
                   struct MPIR_DATATYPE *recvtype,
                   struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx, comm);

/* do something here! */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Alltoallv */
#endif   /* MPID_Alltoallv */

/******** MPID_FN_Reduce **********************************************/

#ifdef MPID_Reduce
int
MPID_FN_Reduce (void *sendbuf, void *recvbuf, int count,
                struct MPIR_DATATYPE *datatype, MPI_Op op, int root,
                struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank, stride;
   struct MPIR_OP *op_ptr;
   MPI_User_function *uop;
   static char myname[] = "MPI_REDUCE";
   MPI_Aint lb, ub, extent;

   /* Is root within the communicator? */
   (void) MPIR_Comm_size (comm, &size);

#ifndef MPIR_NO_ERROR_CHECKING
   if ( root >= size )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_ROOT_TOOBIG, myname,
                                   (char *)0, (char *)0, root, size);
   else if ( root < 0 )
      mpi_errno = MPIR_Err_setmsg (MPI_ERR_ROOT, MPIR_ERR_DEFAULT, myname,
                                   (char *)0, (char *)0, root);
   if ( mpi_errno )
      return MPIR_ERROR(comm, mpi_errno, myname);
#endif

   /* See the overview in Collection Operations for why this is ok */
   if ( count == 0 )
      return mpi_errno;

   /* Get my rank and switch communicators to the hidden collective */
   (void) MPIR_Comm_rank (comm, &rank);
   comm = comm->comm_coll;
   op_ptr = MPIR_GET_OP_PTR(op);
   MPIR_TEST_MPI_OP(op, op_ptr, comm, myname);
   uop = op_ptr->op;
   MPIR_Type_get_limits(datatype, &lb, &ub);
   extent = ub - lb;
   stride = extent * count;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

   /* MPI complete reference (Snir, Otto, Dongarra...), p177: advice to
    * implementors not respected.  The same result might NOT be obtained
    * whenever the function is applied on the same arguments appearing in
    * the same order: to take advantage of the physical location of the
    * processors, the results might vary with the root process (depending
    * on the architectures) */

   /* if the operation is not commutative, then it might be faster to
    * MPI_Gather to the root proc and let it compute everything.  That
    * also depends on the message size (size * count * extent).  If the
    * message size is very very large, it might be better ignore the
    * protocol levels and resort to an "hypercube algorithm":  for 8 procs:
    *  - phase 1: 1 sends {1} to 0 and 0 computes {0} * {1}
    *             5 sends {5} to 4 and 4 computes {4} * {5}
    *             3 sends {3} to 2 and 2 computes {2} * {3}
    *             7 sends {7} to 6 and 6 computes {6} * {7}
    *  - phase 2: 2 sends {2*3} to 0 and 0 computes {0*1} * {2*3}
    *             6 sends {6*7} to 4 and 4 computes {4*5} * {6*7}
    *  - phase 3: 4 sends {4*5*6*7} to 0 and 0 computes {0*1*2*3} * {4*5*6*7}.
    * But that could incur several WAN-TCP latencies in sequence...  That
    * decision must be made comparing the computation time (msg size) and
    * the latency. */
   if ( !op_ptr->commute )
   {
      void *tmp_buf;
      int i;

      if ( rank == root )
      {
         tmp_buf = (void *) globus_libc_malloc (size * stride);
         if ( !tmp_buf )
            MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                       "MPID_FN_Reduce() - failed malloc");
         tmp_buf = (void *) ((char *) tmp_buf - lb);
      }   /* endif */
      mpi_errno = MPI_Gather(sendbuf, count, datatype->self, tmp_buf, count,
                             datatype->self, root, comm->self);
      if ( mpi_errno == MPI_SUCCESS  &&  rank == root )
      {
         /* MPI Standard: the operation is always assumed to be associative */
         /* copy the last data element into recvbuf */
         copy_buf((char *) tmp_buf + lb, size - 1,
                  (char *) recvbuf + lb, 0, stride);
         for (i = size-2; i >= 0; i--)
            (*uop)((char *) tmp_buf + i * stride, recvbuf, &count,
                   &(datatype->self));
      }   /* endif */
      if ( rank == root )
         g_free(tmp_buf);
   }
   else   /* commutative operation (and always assumed associative!) */
   {
      multiple_set_t set_info;
      int set;
      void *tmp_buf;

      /* rename the clusters to have only zeros in root's column */
      update_cluster_id_table(root, comm);

      /* find the sets of procs among which I will send/recv msgs */
      involve_sets(comm, &set_info);

      /* if I'm not the global root proc then I need to allocate a temporary
       * buffer to hold the intermediate value in the computation. */
      if ( rank != root )
      {
         recvbuf = (void *) globus_libc_malloc (stride);
         if ( !recvbuf )
            MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                       "MPID_FN_Reduce() - failed malloc");
         recvbuf = (void *) ((char *) recvbuf - lb);
      }   /* endif */
      /* copy my element into the recv buffer */
      copy_buf((char *)sendbuf + lb, 0, (char *) recvbuf + lb, 0,
               stride);

      /* allocate a temporary recv buffer */
      tmp_buf = (void *) globus_libc_malloc (stride);
      if ( !tmp_buf )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "MPID_FN_Reduce() - failed malloc");
      tmp_buf = (void *) ((char *) tmp_buf - lb);

      for (set = set_info.num-1; set >= 0; set--)
      {
         single_set_t current_set = set_info.sets[set];

         /* for high latencies (WAN-TCP) it's more efficient to Gather
          * the data to the local root process and let it compute. */
         /* for very very large msg sizes or small latencies, use a
          * binomial-tree algorithm. */

         /* recving messages and computing a local intermediate value */
         /* sending combined messages */
         if ( current_set.level == 0 )   /* WAN-TCP ==> flat-tree algorithm */
            mpi_errno = flat_tree_reduce(current_set, recvbuf, extent, lb,
                                         count, datatype->self, comm->self,
                                         uop, tmp_buf);
         else   /* binomial-tree algo */
            mpi_errno = hypercube_reduce(current_set, recvbuf, count,
                                         datatype->self, comm->self, extent, lb,
                                         uop, tmp_buf);
         if ( mpi_errno ) break;
      }   /* endfor */

      g_free(tmp_buf);
      if ( rank != root )
         g_free(recvbuf);
   }   /* endif: end of commutative case */

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Reduce */
#endif   /* MPID_Reduce */

/******** MPID_FN_Allreduce *******************************************/

#ifdef MPID_Allreduce
int
MPID_FN_Allreduce (void *sendbuf, void *recvbuf, int count,
                   struct MPIR_DATATYPE *datatype, MPI_Op op,
                   struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;

   return (mpi_errno);
}   /* MPID_FN_Allreduce */
#endif   /* MPID_Allreduce */

/******** MPID_FN_Reduce_scatter **************************************/

#ifdef MPID_Reduce_scatter
int
MPID_FN_Reduce_scatter (void *sendbuf, void *recvbuf, int *recvcnts,
                        struct MPIR_DATATYPE *datatype, MPI_Op op,
                        struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;
   static char myname[] = "MPI_REDUCE_SCATTER";

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   return (mpi_errno);
}   /* MPID_FN_Reduce_scatter */
#endif   /* MPID_Reduce_scatter */

/******** MPID_FN_Scan ************************************************/

#ifdef MPID_Scan
int
MPID_FN_Scan (void *sendbuf, void *recvbuf, int count,
              struct MPIR_DATATYPE *datatype, MPI_Op op,
              struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int size, rank;

   /* See the overview in Collection Operations for why this is ok */
   if (count == 0) return MPI_SUCCESS;

   /* Get my rank and the size of the communicator */
   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   /* Switch communicators to the hidden collective */
   comm = comm->comm_coll;

   /* Lock for collective operation */
   MPID_THREAD_LOCK(comm->ADIctx,comm);

   /* Unlock for collective operation */
   MPID_THREAD_UNLOCK(comm->ADIctx,comm);

   return (mpi_errno);
}   /* MPID_FN_Scan */
#endif   /* MPID_Scan */


/*********************/
/* private functions */
/*********************/

/******** flat_tree_barrier *******************************************/

static int
flat_tree_barrier(single_set_t set_info, MPI_Request *req, int *num,
                  MPI_Comm comm)
{
   int set_size, rank_idx, i;
   int mpi_errno = MPI_SUCCESS;
   int *set;

   rank_idx = set_info.my_rank_index;
   set_size = set_info.size;
   set = set_info.set;

   for (i = 0; i < set_size; i++)
      if ( i != rank_idx )
      {
         mpi_errno = MPI_Isend((void *)0, 0, MPI_INT, set[i], MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
         mpi_errno = MPI_Irecv((void *)0, 0, MPI_INT, set[i], MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
      }   /* endif */
   return mpi_errno;
}   /* flat_tree_barrier */


/******** hypercube_barrier *******************************************/

static int
hypercube_barrier(single_set_t set_info, MPI_Request *req, int *num,
                  MPI_Comm comm)
{
   int N2_prev, set_size, rank_idx, surfeit;
   int mpi_errno = MPI_SUCCESS;
   int *set;

   rank_idx = set_info.my_rank_index;
   set_size = set_info.size;
   set = set_info.set;
   N2_prev = previous_power_of_2(set_size, NULL);
   surfeit = set_size - N2_prev;

   /* Perform a combine-like operation (hypercube algorithm) */
   /* enter the barrier (notify my neighbors I reached the barrier) */

   if ( rank_idx < N2_prev )   /* embedded N2_prev power-of-two processes */
   {
      int d;

      if ( rank_idx < surfeit )
      {
         int dst = set[N2_prev + rank_idx];

         mpi_errno = MPI_Isend((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
         mpi_errno = MPI_Irecv((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
      }   /* endif */

      for (d = 1; d < N2_prev; d <<= 1)
      {
         int dst = set[rank_idx ^ d];

         mpi_errno = MPI_Isend((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
         mpi_errno = MPI_Irecv((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG,
                               comm, req + (*num)++);
         if ( mpi_errno ) return mpi_errno;
      }   /* endfor */
   }
   else   /* surfeit */
   {
      int dst = set[rank_idx - N2_prev];

      mpi_errno = MPI_Isend((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG, comm,
                            req + (*num)++);
      if ( mpi_errno ) return mpi_errno;
      mpi_errno = MPI_Irecv((void *)0, 0, MPI_INT, dst, MPIR_BARRIER_TAG, comm,
                            req + (*num)++);
      if ( mpi_errno ) return mpi_errno;
   }   /* endif */

   return mpi_errno;
}   /* hypercube_barrier */


/******** wait_barrier ************************************************/

static int
wait_barrier (MPI_Request *req, int n_req)
{
   MPI_Status *statuses;
   int mpi_errno = MPI_SUCCESS;

   if ( !n_req ) return mpi_errno;
   statuses = (MPI_Status *) globus_libc_malloc (n_req * sizeof(MPI_Status));
   if ( !statuses )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "wait_barrier() - failed malloc");

   mpi_errno = MPI_Waitall(n_req, req, statuses);
   /* set mpi_errno in function of statuses if mpi_errno != MPI_SUCCESS */

   g_free(statuses);

   return mpi_errno;
}   /* wait_barrier */


/******** binomial_bcast **********************************************/

static int
binomial_bcast(void *buffer, int count, MPI_Datatype datatype, MPI_Comm comm,
               single_set_t set, MPI_Request *req, int *n_req)
{
   int set_size, rank_idx, root_idx, mask, relative_rnk_idx;
   int mpi_errno = MPI_SUCCESS;

   rank_idx = set.my_rank_index;
   root_idx = set.root_index;
   set_size = set.size;
   relative_rnk_idx = (rank_idx - root_idx + set_size) % set_size;

   mask = 0x1;
   while ( mask < set_size )
   {
      if ( relative_rnk_idx & mask )
      {
         MPI_Status st;
         int sub_src_index = (rank_idx - mask + set_size) % set_size;

         mpi_errno = MPI_Recv(buffer, count, datatype, set.set[sub_src_index],
                              MPIR_BCAST_TAG, comm, &st);
         if (mpi_errno) return mpi_errno;
         break;
      }   /* endif */
      mask <<= 1;
   }   /* end while */

   /* using the binomial tree algorithm, I may have to relay the message */
   mask >>= 1;
   while (mask > 0)
   {
      if (relative_rnk_idx + mask < set_size)
      {
         int dst_index = (rank_idx + mask) % set_size;

         mpi_errno = MPI_Isend(buffer, count, datatype, set.set[dst_index],
                               MPIR_BCAST_TAG, comm, req + (*n_req)++);
         if ( mpi_errno ) return (mpi_errno);
      }
      mask >>= 1;
   }   /* end while */

   return (mpi_errno);
}   /* binomial_bcast */


/******** flat_tree_bcast *********************************************/

static int
flat_tree_bcast(void *buffer, int count, MPI_Datatype datatype, MPI_Comm comm,
                single_set_t set, MPI_Request *req, int *n_req)
{
   int i, set_size, rank_idx, root_idx;
   int mpi_errno = MPI_SUCCESS;

   root_idx = set.root_index;
   rank_idx = set.my_rank_index;
   set_size = set.size;

   if ( root_idx == rank_idx )   /* I'm root, i send */
   {
      for (i = 0; i < set_size; i++)
         if ( i != rank_idx )
         {
            mpi_errno = MPI_Isend(buffer, count, datatype, set.set[i],
                                  MPIR_BCAST_TAG, comm, req + (*n_req)++);
            if (mpi_errno) return (mpi_errno);
         }   /* endif */
   }
   else   /* I'm not the root proc in this set, I recv */
   {
      MPI_Status status;

      mpi_errno = MPI_Recv(buffer, count, datatype, set.set[root_idx],
                           MPIR_BCAST_TAG, comm, &status);
   }   /* endif */

   return (mpi_errno);
}   /* flat_tree_bcast */


/******** flat_tree_gather ********************************************/

static int
flat_tree_gather(single_set_t set, void *tmp_buf, int buf_elts, int init_lvl,
                 int sendcnt, MPI_Datatype sendtype, MPI_Comm comm,
                 int *Depths, int **ColorTable, void *my_buf, int stride,
                 int world_size, int lb, MPI_Datatype recvtype, int recvcnt)
{
   int mpi_errno = MPI_SUCCESS;
   int rank_idx, root_idx;

   rank_idx = set.my_rank_index;
   root_idx = set.root_index;

   if ( root_idx == rank_idx )   /* I'm the root of this set: I recv */
   {
      int i, set_size;

      set_size = set.size;

      for (i = 0; i < set_size; i++)   /* I recv lots */
         if ( i != root_idx )
         {
            int src, pos;
            MPI_Status status;

            src = set.set[i];
            mpi_errno = MPI_Recv(tmp_buf, buf_elts, MPI_PACKED, src,
                                 MPIR_GATHER_TAG, comm, &status);
            if ( mpi_errno ) return mpi_errno;

            /* copy the elements recv'd in tmp_buf to their correct position
             * in my_buf */
            pos = 0;
            unpack_dependencies(tmp_buf, buf_elts, &pos, my_buf, src, init_lvl,
                                Depths, ColorTable, set.level, recvtype, comm,
                                world_size, stride, recvcnt);
         }   /* endif */
   }
   else   /* I'm not the root process: I send my buffer to the local root */
   {
      int pos = 0;

      pack_dependencies(my_buf, sendcnt, sendtype, set.set[rank_idx], init_lvl,
                        comm, tmp_buf, buf_elts, &pos, Depths, ColorTable,
                        set.level, world_size, stride);
      mpi_errno = MPI_Send(tmp_buf, pos, MPI_PACKED, set.set[root_idx],
                           MPIR_GATHER_TAG, comm);
   }

   return mpi_errno;
}   /* flat_tree_gather */

/******** binomial_gather *********************************************/

static int
binomial_gather(single_set_t set, void *tmp_buf, int buf_elts, int init_lvl,
                int sendcnt, MPI_Datatype sendtype, MPI_Comm comm,
                int *Depths, int **ColorTable, void *my_buf, int stride,
                int world_size, int lb, MPI_Datatype recvtype, int recvcnt)
{
   int mpi_errno = MPI_SUCCESS;
   int rank_idx, root_idx, set_size, set_lvl, relative_rnk_idx, mask;

   rank_idx = set.my_rank_index;
   root_idx = set.root_index;
   set_size = set.size;
   set_lvl = set.level;

   relative_rnk_idx = (rank_idx - root_idx + set_size) % set_size;
   mask = 0x1;
   /* receive some chunks of data and copy them into my own buffer */
   while ( mask < set_size )
   {
      int rel_src_idx;

      if ( relative_rnk_idx & mask ) break;
      rel_src_idx = relative_rnk_idx + mask;
      if ( rel_src_idx < set_size )
      {
         MPI_Status status;
         int i, source, pos;

         source = set.set[(rel_src_idx + root_idx) % set_size];
         mpi_errno = MPI_Recv(tmp_buf, buf_elts, MPI_PACKED, source,
                              MPIR_GATHER_TAG, comm, &status);
         if ( mpi_errno ) return mpi_errno;
         /* copy the recv'd elements from tmp_buf to my_buf */
         pos = 0;
         for (i = 0; i < mask; i++)
         {
            int src;

            src = rel_src_idx + i;
            if ( src >= set_size ) break;
            src = set.set[(src + root_idx) % set_size];
            unpack_dependencies(tmp_buf, buf_elts, &pos, my_buf, src, init_lvl,
                                Depths, ColorTable, set_lvl, recvtype, comm,
                                world_size, stride, recvcnt);
         }   /* endfor */
      }   /* endif */
      mask <<= 1;
   }   /* end while */

   /* send all the data elements I collected to my local root */
   if ( rank_idx != root_idx )
   {
      int i, pos = 0;
      int dst = (rank_idx - mask + set_size) % set_size;

      for (i = 0; i < mask; i++)
      {
         int prc = relative_rnk_idx + i;

         if ( prc >= set_size ) break;
         prc = set.set[(prc + root_idx) % set_size];
         pack_dependencies(my_buf, sendcnt, sendtype, prc, init_lvl, comm,
                           tmp_buf, buf_elts, &pos, Depths, ColorTable,
                           set_lvl, world_size, stride);
      }   /* endfor */
      mpi_errno = MPI_Send(tmp_buf, pos, MPI_PACKED, set.set[dst],
                           MPIR_GATHER_TAG, comm);
   }   /* endif */

   return mpi_errno;
}   /* binomial_gather */

/******** flat_tree_scatter *******************************************/

static int
flat_tree_scatter(single_set_t set, void *tmp_buf, int buf_elts, int init_lvl,
                  int recvcnt, MPI_Datatype recvtype, MPI_Comm comm,
                  int *Depths, int **ColorTable, void *my_buf, int stride,
                  int world_size, int lb, MPI_Datatype sendtype, int sendcnt)
{
   int mpi_errno = MPI_SUCCESS;
   int set_lvl, rank_idx, root_idx;

   rank_idx = set.my_rank_index;
   root_idx = set.root_index;
   set_lvl = set.level;

   if ( root_idx == rank_idx ) /* I send chunks to the processes of this set */
   {
      int i, set_size;

      set_size = set.size;

      for (i = 0; i < set_size; i++)
         if ( i != rank_idx )
         {
            int dst, pos;

            dst = set.set[i];
            /* copy the right chunks of my_buf into tmp_buf */
            pos = 0;
            pack_dependencies(my_buf, sendcnt, sendtype, dst, init_lvl, comm,
                              tmp_buf, buf_elts, &pos, Depths, ColorTable,
                              set_lvl, world_size, stride);

            /* send a part of my data */
            mpi_errno = MPI_Send(tmp_buf, pos, MPI_PACKED, dst,
                                 MPIR_SCATTER_TAG, comm);
            if ( mpi_errno ) return mpi_errno;
         }   /* endif */
   }
   else   /* I recv from the root of this set of processes */
   {
      MPI_Status status;
      int pos = 0;

      mpi_errno = MPI_Recv(tmp_buf, buf_elts, MPI_PACKED, set.set[root_idx],
                           MPIR_SCATTER_TAG, comm, &status);
      if ( mpi_errno ) return mpi_errno;
      unpack_dependencies(tmp_buf, buf_elts, &pos, my_buf, set.set[rank_idx],
                          init_lvl, Depths, ColorTable, set_lvl, recvtype,
                          comm, world_size, stride, recvcnt);
   }   /* endif */

   return mpi_errno;
}   /* flat_tree_scatter */

/******** binomial_scatter ********************************************/

static int
binomial_scatter(single_set_t set, void *tmp_buf, int buf_elts, int init_lvl,
                 int recvcnt, MPI_Datatype recvtype, MPI_Comm comm,
                 int *Depths, int **ColorTable, void *my_buf, int stride,
                 int world_size, int lb, MPI_Datatype sendtype, int sendcnt)
{
   int mpi_errno = MPI_SUCCESS;
   int mask, set_lvl, set_size, relative_rnk_idx, rank_idx, root_idx;
   MPI_Status status;

   rank_idx = set.my_rank_index;
   root_idx = set.root_index;
   set_size = set.size;
   set_lvl = set.level;
   relative_rnk_idx = (rank_idx - root_idx + set_size) % set_size;
   mask = 0x1;

   /* find the guy which is going to send me the data */
   while ( mask < set_size )
   {
      if ( relative_rnk_idx & mask ) break;
      mask <<= 1;
   }

   /* receive some chunks of data from my local root */
   if ( rank_idx != root_idx )
   {
      int i, source, pos = 0;

      source = set.set[(rank_idx - mask + set_size) % set_size];
      mpi_errno = MPI_Recv(tmp_buf, buf_elts, MPI_PACKED, source,
                           MPIR_SCATTER_TAG, comm, &status);
      if ( mpi_errno ) return mpi_errno;
      for (i = 0; i < mask; i++)
      {
         int prc = relative_rnk_idx + i;

         if ( prc >= set_size ) break;
         prc = set.set[(prc + root_idx) % set_size];
         unpack_dependencies(tmp_buf, buf_elts, &pos, my_buf, prc, init_lvl,
                             Depths, ColorTable, set_lvl, recvtype, comm,
                             world_size, stride, recvcnt);
      }
   }   /* endif */

   mask >>= 1;
   /* dispatch my data elements to my friends (binomial-tree algorithm) */
   while ( mask > 0 )
   {
      int dst = relative_rnk_idx + mask;

      if ( dst < set_size )
      {
          int i, pos = 0;

         /* copy the right data elements from my_buf into tmp_buf */
         for (i = 0; i < mask; i++)
         {
            int sub_dest;

            sub_dest = dst + i;
            if (sub_dest >= set_size ) break;
            sub_dest = set.set[(sub_dest + root_idx) % set_size];
            pack_dependencies(my_buf, sendcnt, sendtype, sub_dest, init_lvl,
                              comm, tmp_buf, buf_elts, &pos, Depths,
                              ColorTable, set_lvl, world_size, stride);
         }   /* endfor */

         dst = set.set[(dst + root_idx) % set_size];
         /* send the tmp_buf */
         mpi_errno = MPI_Send(tmp_buf, pos, MPI_PACKED, dst, MPIR_SCATTER_TAG,
                              comm);
         if ( mpi_errno ) return mpi_errno;
      }
      mask >>= 1;
   }   /* end while */

   return mpi_errno;
}   /* binomial_scatter */

/******** flat_tree_reduce ********************************************/

static int
flat_tree_reduce(single_set_t set, void *my_buf, int extent, int lb, int count,
                 MPI_Datatype datatype, MPI_Comm comm, MPI_User_function *uop,
                 void *tmp_buf)
{
   /* here the operation is assumed to be commutative and associative */

   int mpi_errno = MPI_SUCCESS;
   int root_idx, rank_idx;

   root_idx = set.root_index;
   rank_idx = set.my_rank_index;

   /* I recv and compute only if I'm the root of this set */
   if ( root_idx == rank_idx )
   {
      int i, set_size;

      set_size = set.size;
      if ( set_size < 2 ) return mpi_errno;

      /* receive all the elements and compute */
      for (i = 0; i < set_size; i++)
         if ( i != root_idx )
         {
            MPI_Status status;

            mpi_errno = MPI_Recv((char *) tmp_buf, count, datatype, set.set[i],
                                 MPIR_REDUCE_TAG, comm, &status);
            if ( mpi_errno ) return mpi_errno;
            /* compute */
            (*uop)(tmp_buf, my_buf, &count, &datatype);
         }   /* endif */
   }
   else   /* I'm not root: I send my buffer */
      mpi_errno = MPI_Send(my_buf, count, datatype, set.set[root_idx],
                           MPIR_REDUCE_TAG, comm);

   return mpi_errno;
}   /* flat_tree_reduce */


/******** hypercube_reduce ********************************************/

static int
hypercube_reduce(single_set_t set, void *my_buf, int count,
                 MPI_Datatype datatype, MPI_Comm comm, int extent, int lb,
                 MPI_User_function *uop, void *tmp_buf)
{
   /* here the operation is assumed to be commutative and associative */

   int mpi_errno = MPI_SUCCESS;
   int relative_rnk_idx, mask, root_idx, rank_idx, set_size;

   root_idx = set.root_index;
   rank_idx = set.my_rank_index;
   set_size = set.size;

   relative_rnk_idx = (rank_idx - root_idx + set_size) % set_size;
   mask = 0x1;
   while ( mask < set_size )
   {
      if ( mask & relative_rnk_idx )   /* send my (intermediate) result */
      {
         int dst = set.set[(rank_idx - mask + set_size) % set_size];

         mpi_errno = MPI_Send(my_buf, count, datatype, dst, MPIR_REDUCE_TAG,
                              comm);
         break;
      }
      else   /* recv and compute */
      {
         int source;

         source = relative_rnk_idx | mask;
         if ( source < set_size )
         {
            MPI_Status status;

            source = set.set[(source + root_idx) % set_size];
            mpi_errno = MPI_Recv(tmp_buf, count, datatype, source,
                                 MPIR_REDUCE_TAG, comm, &status);
            if ( mpi_errno ) break;
            /* compute */
            (*uop)(tmp_buf, my_buf, &count, &datatype);
         }   /* endif */
      }
      mask <<= 1;
   }   /* end while */

   return mpi_errno;
}   /* hypercube_reduce */


/******** update_cluster_id_table *************************************/

static void
update_cluster_id_table (int root, struct MPIR_COMMUNICATOR *comm)
{
   /* 'rename' the clusters at each level so that the root */
   /* process has only zeros as cluster ids (at each level) */

   int lvl, root_depth, size;
   int *root_column, *root_colors;
   int *Depths = comm->Topology_Depths;
   int **ClusterIds = comm->Topology_ClusterIds;
   int **ColorTable = comm->Topology_ColorTable;
   int *ClusterSizes = comm->Topology_ClusterSizes;

   (void) MPIR_Comm_size (comm, &size);
   root_depth = Depths[root];
   root_column = ClusterIds[root];
   root_colors = ColorTable[root];

   for (lvl = 0; lvl < root_depth; lvl++)
   {
      int shift = root_column[lvl];

      if ( shift )
      {
         /* at the current level, the root process has a non-zero
          * cluster id: we shift (rotate) the cids at this level
          * for all the processes which can communicate directly
          * with the root process at this level (ie: all the procs
          * in the same cluster as the root). */

         int n_cid, proc;

         /* find the number of cids that need to be rotated at */
         /* this level */
         for (n_cid = 0, proc = 0; proc < size; proc++)
            if ( Depths[proc] > lvl
                 && ClusterIds[proc][lvl] > n_cid
                 && root_colors[lvl] == ColorTable[proc][lvl] )
               n_cid = ClusterIds[proc][lvl];

         shift = ++n_cid - shift;

         for (proc = 0; proc < size; proc++)
            if ( Depths[proc] > lvl
                 &&  root_colors[lvl] == ColorTable[proc][lvl] )
               ClusterIds[proc][lvl] = (ClusterIds[proc][lvl] + shift) % n_cid;
      }   /* endif */
   }   /* endfor */

   return;
}   /* update_cluster_id_table */

/******** is_a_master *************************************************/

static int
is_a_master (int level, int *column, int depth)
{
   /* A process is a master process at the given level iff its cluster */
   /* ids are 0 from the given level + 1 up to its depth: a master proc
    * is the representative of a cluster at level n+1 in a cluster at
    * level n. */

   int lvl;

   for (lvl = level+1; lvl < depth; lvl++)
      if ( column[lvl] != 0 )
         return GLOBUS_FALSE;

   return GLOBUS_TRUE;
}   /* is_a_master */

/******** involve_sets ************************************************/

static void
involve_sets(struct MPIR_COMMUNICATOR *comm, multiple_set_t *set_info)
{
   /* create the sets of processes in which I will be involved for
    * communication */

   int first_lvl, lvl, my_depth, rank;
   single_set_t *set_array;
   int *my_column;

   (void) MPIR_Comm_rank (comm, &rank);
   my_depth = comm->Topology_Depths[rank];
   /* pre-allocated memory */
   set_info->sets = set_array = comm->Topology_InfoSets;
   my_column = comm->Topology_ClusterIds[rank];

   /* from which level will I be involved in a communication */
   for (first_lvl = 0, lvl = my_depth-1; lvl >= 0; lvl--)
      if ( my_column[lvl] != 0 )
      {
         first_lvl = lvl;
         break;
      }   /* endif */
   set_info->num = my_depth - first_lvl;

   for (lvl = first_lvl; lvl < my_depth; lvl++, set_array++)
      make_set(lvl, comm, set_array);

   return;
}   /* involve_sets */

/******** make_set ****************************************************/

static void
make_set (int lvl, struct MPIR_COMMUNICATOR *comm, single_set_t *set_info)
{
   /* create a single set of procs at a given level */

   int proc, index, rank, size;
   int *my_colors;
   int **ClusterIds = comm->Topology_ClusterIds;
   int *Depths = comm->Topology_Depths;
   int **ColorTable = comm->Topology_ColorTable;

   (void) MPIR_Comm_size (comm, &size);
   (void) MPIR_Comm_rank (comm, &rank);
   set_info->level = lvl;
   my_colors = ColorTable[rank];

   /* pre-allocated memory for the sets of processes */
   set_info->set = comm->Topology_ClusterSets[lvl];

   /* a set is made of all the master processes at the given level, ie: all
    * the root processes at level+1 */
   set_info->my_rank_index = set_info->root_index = -1;
   for (proc = 0, index = 0; proc < size; proc++)
   {
      int current_depth = Depths[proc];
      int *current_column = ClusterIds[proc];

      if ( lvl < current_depth
           && my_colors[lvl] == ColorTable[proc][lvl]
           && is_a_master(lvl, current_column, current_depth) )
      {
         /* know the index of my process in the set */
         if ( proc == rank )
            set_info->my_rank_index = index;

         /* know the index of the root in this set */
         if ( current_column[lvl] == 0 )
            set_info->root_index = index;

         set_info->set[index++] = proc;
      }   /* endif */
   }   /* endfor */
   set_info->size = index;

   return;
}   /* make_set */

/******** previous_power_of_2 *****************************************/

static int
previous_power_of_2 (int n, int *p)
{
   int n2_prev = 1;
   int e;

   /* the return value is the power of 2 less than or equal to 'n' */
   /* 'p' is set so that 2^p = return value */
   e = 0;
   while (n > 1)
   {
      n >>= 1;
      n2_prev <<= 1;
      e++;
   }   /* end while */

   if (p != NULL) *p = e;
   return n2_prev;
}   /* previous_power_of_2 */

/******** copy_buf ****************************************************/

static void
copy_buf(void *from_buf, int from, void *to_buf, int to, int stride)
{
   memcpy((char *)to_buf + to*stride, (char *)from_buf + from*stride, stride);
   return;
}   /* copy_buf */


/******** proc_to_index ***********************************************/

static int
proc_to_index(int rank, int **ColorTable, int level)
{
   /* convert my absolute process number (in a communicator) into my index
    * (= position) in my cluster at the given level */

   int indx, i;
   int my_color = ColorTable[rank][level];

   for (indx = 0, i = 0; i < rank; i++)
      if ( ColorTable[i][level] == my_color )
         indx++;
   return indx;
}   /* proc_to_index */

/******** pack_dependencies *******************************************/

static void
pack_dependencies(void *from_buf, int count, MPI_Datatype datatype,
                  int from_rank, int from_lvl, MPI_Comm comm, void *to_buf,
                  int to_sz, int *to_position, int *Depths, int **ColorTable,
                  int dep_lvl, int size, int stride)
{
   /* pack the data elements corresponding to process 'rank' (in
    * 'from_buf') at level 'dep_lvl', copying them into 'to_buf'
    * from position 'to_position'.  Also update 'to_position' for the
    * next data to pack. */

   int next_lvl = dep_lvl + 1;

   if ( Depths[from_rank] > next_lvl )
   {
      int prc, my_color;

      my_color = ColorTable[from_rank][next_lvl];

      for (prc = 0; prc < size; prc++)
      {
         /* the process 'prc' belongs to the "dependencies" of process
          * 'from_rank' iff they both have the same color at level 'dep_lvl' */
         if (Depths[prc] > next_lvl && ColorTable[prc][next_lvl] == my_color)
            MPI_Pack((char *) from_buf + stride
                                    * proc_to_index(prc, ColorTable, from_lvl),
                     count, datatype, to_buf, to_sz, to_position, comm);
      }   /* endfor */
   }
   else
      MPI_Pack((char *) from_buf + stride
                              * proc_to_index(from_rank, ColorTable, from_lvl),
               count, datatype, to_buf, to_sz, to_position, comm);

   return;
}   /* pack_dependencies */

/******** unpack_dependencies *****************************************/

static void
unpack_dependencies(void *from_buf, int from_sz, int *from_position,
                    void *to_buf, int to_rank, int to_lvl, int *Depths,
                    int **ColorTable, int dep_lvl, MPI_Datatype datatype,
                    MPI_Comm comm, int size, int stride, int count)
{
   /* unpack the data elements from the buffer 'from_buf' from position
    * 'from_position' copying them into 'to_buf' to the positions
    * corresponding to the dependencies of process 'to_rank' at level
    * 'dep_lvl'. Also update 'from_position' for the next data to unpack */

   int next_lvl = dep_lvl + 1;

   if ( Depths[to_rank] > next_lvl )
   {
      int prc, my_color;

      my_color = ColorTable[to_rank][next_lvl];

      for (prc = 0; prc < size; prc++)
      {
         /* the process 'prc' belongs to the "dependencies" of process
          * 'to_rank' iff they both have the same color at level 'dep_lvl' */
         if (Depths[prc] > next_lvl && ColorTable[prc][next_lvl] == my_color)
            MPI_Unpack(from_buf, from_sz, from_position, (char *) to_buf
                             + stride * proc_to_index(prc, ColorTable, to_lvl),
                       count, datatype, comm);
      }   /* endfor */
   }
   else
      MPI_Unpack(from_buf, from_sz, from_position, (char *) to_buf + stride
                                  * proc_to_index(to_rank, ColorTable, to_lvl),
                 count, datatype, comm);

   return;
}   /* unpack_dependencies */

