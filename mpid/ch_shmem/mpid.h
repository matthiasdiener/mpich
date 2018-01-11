/*
 *  $Id: mpid.h,v 1.3 1995/11/08 12:51:17 raja Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef MPID_INCL
#define MPID_INCL

#include "mpiimpl.h"

#ifdef MPID_DEVICE_CODE
/* Any thing that is specific to the device version */

/* Assert shared memory for the collective operations */
#define MPID_HAS_SHARED_MEM

/* Options for packets */
#define MPID_PKT_INCLUDE_LINK
#define MPID_PKT_INCLUDE_LEN
#define MPID_PKT_INCLUDE_SRC

#define MPID_USE_GET

#define MPID_PKT_DYNAMIC_SEND
#define MPID_PKT_SEND_ALLOC(type,pkt,nblk) \
			pkt = (type *)MPID_SHMEM_GetSendPkt(nblk)

#define MPID_PKT_GET_NEEDS_ACK
#define MPID_PKT_DYNAMIC_RECV
#define MPID_PKT_RECV_FREE(pkt) {if (pkt) MPID_SHMEM_FreeRecvPkt( pkt );}
#define MPID_PKT_RECV_CLR(pkt)  pkt = 0

#include "p2p.h"
#include "packets.h"
extern MPID_PKT_T *MPID_SHMEM_GetSendPkt();
extern void MPID_SHMEM_FreeSetup();

#endif

/* Timers are usually consistent on SMPs */
#define MPID_Wtime_is_global() 1

#ifdef MPID_WTIME
#undef MPID_WTIME
#endif
#if defined(MPI_cspp) && defined(__USE_LONG_LONG)
/* 
 * Special code for the Convex timer.  Other systems could do similar things.
 */
#include <sys/cnx_ail.h>
#define MPID_WTIME(ctx) (toc_read() * ((double) 0.000001))
#else
extern double p2p_wtime();
#define MPID_WTIME(ctx) p2p_wtime()
#endif

#if defined(MPI_cspp)
#ifdef MPID_USE_ADI_COLLECTIVE
#define MPID_Comm_init(ctx,comm,newcomm) MPID_SHMEM_Comm_init(comm,newcomm)
#define MPID_Comm_free(ctx,comm) MPID_SHMEM_Comm_free(comm)
#define MPID_FN_Barrier MPID_SHMEM_Barrier
#define MPID_Barrier(ctx,comm) MPID_SHMEM_Barrier(comm)
#define MPID_FN_Reduce_scatter MPID_SHMEM_Reduce_scatter
#define MPID_Reduce_scatter(sendbuf,recvbuf,recvcnts,datatype,op,comm) \
	MPID_SHMEM_Reduce_scatter(sendbuf,recvbuf,recvcnts,datatype,op,comm)
#define MPID_FN_Reduce_scatterv MPID_SHMEM_Reduce_scatterv
#define MPID_Reduce_scatterv(sendbuf,sendcnts,displs,datatype,recvbuf,\
				recvcnt,op,comm) \
	MPID_SHMEM_Reduce_scatterv(sendbuf,sendcnts,displs,datatype,\
				recvbuf,recvcnt,op,comm)
#define MPID_FN_Allgather MPID_SHMEM_Allgather
#define MPID_Allgather(sf,st,se,rf,rs,re,comm) \
	MPID_SHMEM_Allgather(sf,st,se,rf,rs,re,comm)
#define MPID_FN_Allgatherv MPID_SHMEM_Allgatherv
#define MPID_Allgatherv(sf,st,se,rf,rs,ds,re,comm) \
	MPID_SHMEM_Allgatherv(sf,st,se,rf,rs,ds,re,comm)
#define MPID_FN_Reduce MPID_SHMEM_Reduce
#undef MPID_Reduce
#define MPID_Reduce(sendbuf,recvbuf,count,datatype,op,root,comm) \
	MPID_SHMEM_Reduce(sendbuf,recvbuf,count,datatype,op,root,comm)
#define MPID_FN_Allreduce MPID_SHMEM_Allreduce
#define MPID_Allreduce(sendbuf,recvbuf,count,datatype,op,comm) \
	MPID_SHMEM_Allreduce(sendbuf,recvbuf,count,datatype,op,comm)
#define MPID_FN_Bcast MPID_SHMEM_Bcast
#define MPID_Bcast(sendbuf,count,datatype,root,comm) \
	MPID_SHMEM_Bcast(sendbuf,count,datatype,root,comm)
#define MPID_Sum_double(a,b,n) MPID_SHMEM_Sum_double(a,b,n)

int MPID_SHMEM_Barrier();
int MPID_SHMEM_Reduce_scatter();
int MPID_SHMEM_Reduce_scatterv();
int MPID_SHMEM_Allgatherv();
int MPID_SHMEM_Allgather();
int MPID_SHMEM_Allreduce();
int MPID_SHMEM_Reduce();
int MPID_SHMEM_Bcast();
#endif
#endif

#include "mpid_bind.h"

#endif
