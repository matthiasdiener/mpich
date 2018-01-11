/*
 *  $Id: mpid.h,v 1.2 1993/11/12 11:54:52 doss Exp $
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
#define MPID_PKT_SEND_ALLOC(type,pkt) pkt = (type *)MPID_SPP_GetSendPkt()

#define MPID_PKT_GET_NEEDS_ACK
#define MPID_PKT_DYNAMIC_RECV
#define MPID_PKT_RECV_FREE(pkt) {if (pkt) MPID_SPP_FreeRecvPkt( pkt );}
#define MPID_PKT_RECV_CLR(pkt)  pkt = 0

#include "p2p.h"
#include "packets.h"
extern MPID_PKT_T *MPID_SPP_GetSendPkt();

#endif

/* Timers are usually consistent on SMPs */
#define MPID_Wtime_is_global() 1

#ifdef MPI_cspp
#include <sys/cnx_ail.h>
#ifdef MPID_WTIME
#undef MPID_WTIME
#endif
#define MPID_WTIME(ctx) toc_read() * 0.000001
#endif
#include "mpid_bind.h"
#endif
