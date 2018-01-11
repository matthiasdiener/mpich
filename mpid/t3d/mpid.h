/*
 *  $Id: mpid.h,v 1.1 1995/06/07 06:02:46 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#include "t3d.h"

#ifdef MPID_DEVICE_CODE
/* Any thing that is specific to the device version */
#include "mpi_bc.h"
/* dmpi.h includes mpir.h which includes mpid.h to pick up the device 
   definitions.  This undef/define pair keeps mpir from including mpid! */
#undef MPID_DEVICE_CODE
#include "dmpi.h"
#define MPID_DEVICE_CODE

#include "mpid_bind.h"

#endif
