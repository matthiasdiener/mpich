/*
 *  $Id: t3dprobe.h,v 1.1 1995/06/07 06:03:52 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DPROBE_INCLUDED
#define _T3DPROBE_INCLUDED

/******************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ******************************************************************************/
#define MPID_Iprobe( ctx, tag, source, context_id, flag, status ) \
        T3D_Iprobe( tag, source, context_id, flag, status ) 
#define MPID_Probe( ctx, tag, source, context_id, status ) \
        T3D_Probe( tag, source, context_id, status ) 

#endif
