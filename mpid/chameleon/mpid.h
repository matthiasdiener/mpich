/*
 *  $Id: mpid.h,v 1.5 1995/06/30 17:35:10 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _MPID_H_INCLUDED
#define _MPID_H_INCLUDED

#include "mpiimpl.h"

/* #include "dmch.h" */

#ifdef MPID_DEVICE_CODE
/* Any thing that is specific to the device version */
/* #include "mpi_bc.h" */
/* dmpi.h includes mpir.h which includes mpid.h to pick up the device 
   definitions.  This undef/define pair keeps mpir from including mpid! */
/* #undef MPID_DEVICE_CODE */
/* #include "dmpi.h" */
/* #define MPID_DEVICE_CODE */

#include "packets.h"
/* #include "mpid_bind.h" */
#endif

#include "mpid_bind.h"

#endif
