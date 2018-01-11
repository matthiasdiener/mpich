/*
 *  $Id: t3devent.h,v 1.1 1995/06/07 06:03:24 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DEVENT_INCLUDED
#define _T3DEVENT_INCLUDED

/***************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ***************************************************************************/
#define MPID_Check_device( ctx,blocking )  T3D_Check_device( blocking )
#define MPID_Cancel( ctx, r )              T3D_Cancel( r )

#endif
