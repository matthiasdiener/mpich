/*
 *  $Id: mpid.h,v 1.7 1996/01/03 19:07:39 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _MPID_H_INCLUDED
#define _MPID_H_INCLUDED

#include "mpiimpl.h"

#ifdef MPID_DEVICE_CODE
/* Any thing that is specific to the device version */
#include "packets.h"
#endif

#include "mpid_bind.h"
/* 
   Define MPID_END_NEEDS_BARRIER for finalize to call MPI_BARRIER
   before MPID_END
  */
#endif
