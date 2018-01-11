/*
 *  $Id: t3doptions.h,v 1.1 1995/06/07 06:03:45 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DOPTIONS_INCLUDED
#define _T3DOPTIONS_INCLUDED

/****************************************************************************
  This device currently does not support collective operations directly.
 ***************************************************************************/
#undef MPID_USE_ADI_COLLECTIVE

/****************************************************************************
  This device does not currently support heterogeneous machines.
 ***************************************************************************/
#undef MPID_HAS_HETERO

#endif
