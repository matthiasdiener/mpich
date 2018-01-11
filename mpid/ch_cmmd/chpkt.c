/*
 *  $Id$
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
#endif

#include "mpid.h"

/*
    This file contains some simple routines for managing packets.  
    These routines are not needed by the basic message-passing interface,
    but are needed 

    (a) if non-blocking operations are used to send the packets,
    (b) packets are moved directly through shared memory.

    The packets are managed as a linked list, with each processor having 
    a free and in use list.  

    Shared memory systems may also have an 'incoming' list, consisting of
    packets provided by other processors and that contain data.
 */

/*
   Allocate pre_alloc packets; if == 0, allocate a default number.
   Return the first packet.

   Note that if packet links are enabled, the pre_alloc packets will
   be linked together.
 */
MPID_PKT_T *MPID_CMMD_Pkt_init( pre_alloc )
int pre_alloc;
{
MPID_PKT_T *new;
int        i;

if (pre_alloc == 0) pre_alloc = 128;

new = (MPID_PKT_T *)MALLOC( pre_alloc * sizeof(MPID_PKT_T) );
if (!new) return 0;

#ifdef MPID_PKT_INCLUDE_LINK
for (i=0; i<pre_alloc-1; i++) 
    new[i].head.next = &new[i+1];
new[pre_alloc-1].head.next = 0;
#endif

return new;
}
