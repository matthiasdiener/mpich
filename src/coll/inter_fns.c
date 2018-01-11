/*
 *  $Id: inter_fns.c,v 1.1.1.1 1997/09/17 20:42:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "coll.h"

/*
 * Provide the collective ops structure for inter communicators.
 * Doing it this way (as a different set of functions) removes a test
 * from each collective call, and gets us back at least the cost of the
 * additional indirections and function call we have to provide the 
 * abstraction !
 *
 * Written by James Cownie (Meiko) 31 May 1995
 */

/* Forward declarations */
static int inter_Barrier ANSI_ARGS((struct MPIR_COMMUNICATOR *));
static int inter_Bcast ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, int, struct MPIR_COMMUNICATOR *));
static int inter_Gather ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, void*, 
				   int, struct MPIR_DATATYPE *, int, struct MPIR_COMMUNICATOR *));
static int inter_Gatherv ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, void*, int *, 
				    int *, struct MPIR_DATATYPE *, int, struct MPIR_COMMUNICATOR *)); 
static int inter_Scatter ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, void*, int, 
				    struct MPIR_DATATYPE *, int, struct MPIR_COMMUNICATOR *));
static int inter_Scatterv ANSI_ARGS((void*, int *, int *, struct MPIR_DATATYPE *, 
				     void*, int, struct MPIR_DATATYPE *, int, struct MPIR_COMMUNICATOR *));
static int inter_Allgather ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, void*, int, 
				      struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *));
static int inter_Allgatherv ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, void*, int *,
				       int *, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *));
static int inter_Alltoall ANSI_ARGS((void*, int, struct MPIR_DATATYPE *, 
				     void*, int, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *));
static int inter_Alltoallv ANSI_ARGS((void*, int *, int *, 
				      struct MPIR_DATATYPE *, void*, int *, 
				      int *, struct MPIR_DATATYPE *, struct MPIR_COMMUNICATOR *));
static int inter_Reduce ANSI_ARGS((void*, void*, int, 
				   struct MPIR_DATATYPE *, MPI_Op, int, struct MPIR_COMMUNICATOR *));
static int inter_Allreduce ANSI_ARGS((void*, void*, int, 
				      struct MPIR_DATATYPE *, MPI_Op, struct MPIR_COMMUNICATOR *));
static int inter_Reduce_scatter ANSI_ARGS((void*, void*, int *, 
					   struct MPIR_DATATYPE *, MPI_Op, struct MPIR_COMMUNICATOR *));
static int inter_Scan ANSI_ARGS((void*, void*, int, struct MPIR_DATATYPE *, 
				 MPI_Op, struct MPIR_COMMUNICATOR * ));

static struct _MPIR_COLLOPS inter_collops = {
    inter_Barrier,
    inter_Bcast,
    inter_Gather, 
    inter_Gatherv, 
    inter_Scatter,
    inter_Scatterv,
    inter_Allgather,
    inter_Allgatherv,
    inter_Alltoall,
    inter_Alltoallv,
    inter_Reduce,
    inter_Allreduce,
    inter_Reduce_scatter,
    inter_Scan,
    1                              /* Giving it a refcount of 1 ensures it
				    * won't ever get freed.
				    */
};

MPIR_COLLOPS MPIR_inter_collops = &inter_collops;

/* Now the functions, each one simply raises an error */
static int inter_Barrier( comm )
struct MPIR_COMMUNICATOR * comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_BARRIER");
}

static int inter_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
struct MPIR_DATATYPE *      datatype;
int               root;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_BCAST");
}

static int inter_Gather ( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, 
		 root, comm )
void             *sendbuf;
int               sendcnt;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcount;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_GATHER");
}

static int inter_Gatherv ( sendbuf, sendcnt,  sendtype, 
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
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_GATHERV");
}

static int inter_Scatter ( sendbuf, sendcnt, sendtype, 
		  recvbuf, recvcnt, recvtype, 
		  root, comm )
void             *sendbuf;
int               sendcnt;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcnt;
struct MPIR_DATATYPE *      recvtype;
int               root;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_SCATTER");
}

static int inter_Scatterv ( sendbuf, sendcnts, displs, sendtype, 
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
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_SCATTERV");
}

static int inter_Allgather ( sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcount;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_ALLGATHER");
}

static int inter_Allgatherv ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcounts, displs,   recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int              *recvcounts;
int              *displs;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_ALLGATHERV");
}

static int inter_Alltoall( sendbuf, sendcount, sendtype, 
                  recvbuf, recvcnt, recvtype, comm )
void             *sendbuf;
int               sendcount;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int               recvcnt;
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_GATHERV");
}

static int inter_Alltoallv ( sendbuf, sendcnts, sdispls, sendtype, 
                    recvbuf, recvcnts, rdispls, recvtype, comm )
void             *sendbuf;
int              *sendcnts;
int              *sdispls;
struct MPIR_DATATYPE *      sendtype;
void             *recvbuf;
int              *recvcnts;
int              *rdispls; 
struct MPIR_DATATYPE *      recvtype;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_ALLTOALLV");
}

static int inter_Reduce ( sendbuf, recvbuf, count, datatype, op, root, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
int               root;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_REDUCE");
}

static int inter_Allreduce ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_ALLREDUCE");
}

static int inter_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_REDUCE_SCATTER");
}

static int inter_Scan ( sendbuf, recvbuf, count, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int               count;
struct MPIR_DATATYPE *      datatype;
MPI_Op            op;
struct MPIR_COMMUNICATOR *          comm;
{
    return MPIR_ERROR(comm, MPI_ERR_COMM_INTER,
		      "MPI_SCAN");
}

