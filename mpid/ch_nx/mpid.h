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

#include "packets.h"
#endif

#include "mpid_bind.h"
#endif
