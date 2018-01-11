/*
 *  $Id: dmpi.h,v 1.6 1994/05/10 22:34:50 doss Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* mpir version of device interface */

#include "mpir.h"

#ifndef _DMPI_INCLUDE
#define _DMPI_INCLUDE

#define  DMPI_mark_send_completed(DMPI_send_handle) \
          (DMPI_send_handle)->completed = MPIR_YES;
#define  DMPI_mark_recv_completed(DMPI_recv_handle) \
          (DMPI_recv_handle)->completed = MPIR_YES;

#define DMPI_mpid_recv_handle_from_rhandle( a, b ) b = &((a)->dev_rhandle)

#define DMPI_search_unexpected_queue(src,tag,ctxt_id,found,flag,unex) \
if (MPIR_unexpected_recvs.first) {\
MPIR_search_unexpected_queue( src, tag, ctxt_id, found, flag, unex );}\
else *(found) = 0;
 
/* Defined but do nothing */
#define DMPI_check_mpi(blocking)
#endif


