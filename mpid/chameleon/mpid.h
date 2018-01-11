/*
 *  $Id: mpid.h,v 1.6 1995/08/11 00:21:56 gropp Exp $
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

#endif
