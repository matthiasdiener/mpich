/*
 *  $Id: mpid.h,v 1.1 1995/08/09 15:53:22 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef MPID_INCL
#define MPID_INCL

#include "mpiimpl.h"

#ifdef MPID_DEVICE_CODE
/* Any thing that is specific to the device version */
#include "packets.h"
#endif

/* Special features of timer for EUI (allows access to global clock) */
#define MPID_EUI_Wtime MPID_GTime
#ifdef MPID_WTIME
#undef MPID_WTIME
#endif
#define MPID_WTIME(ctx)  MPID_GTime()
#define MPID_Wtime_is_global MPID_Time_is_global
#include "mpid_bind.h"
extern double MPID_GTime ANSI_ARGS((void));
extern int MPID_Time_is_global ANSI_ARGS((void));
#endif

