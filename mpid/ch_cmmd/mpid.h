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

/* Use CMMD_Send_block instead of CMMD_Send_noblock */
#define MPID_USE_SEND_BLOCK
#define MPID_SendControlBlock( pkt, size, channel ) \
    { MPID_TRACE_CODE("BSendControl",channel);\
      CMMD_send_block(channel,MPID_PT2PT_TAG,(char*)(pkt),size);\
      MPID_TRACE_CODE("ESendControl",channel);}

#include "packets.h"
#endif

#include "mpid_bind.h"
#endif
