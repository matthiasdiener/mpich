/*
 *  $Id: t3dsend.h,v 1.5 1995/09/15 19:19:24 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DSEND_INCLUDED
#define _T3DSEND_INCLUDED


/***************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ***************************************************************************/
#define MPID_Set_send_debug_flag(ctx,f)   T3D_Set_send_debug_flag(f)
#define MPID_Post_send(ctx,shandle)       T3D_Post_send(shandle) 
#define MPID_Post_send_sync(ctx,shandle)  T3D_Post_send_sync(shandle) 
#define MPID_Post_send_ready(ctx,shandle) T3D_Post_send(shandle) 
#define MPID_Blocking_send(ctx,shandle)   T3D_Blocking_send(shandle)
#define MPID_Blocking_send_ready(ctx,shandle) \
        MPID_Blocking_send(ctx,shandle)
#define MPID_Complete_send(ctx,shandle)   T3D_Complete_send(shandle)
#define MPID_Test_send(ctx,shandle)       T3D_Test_send(shandle)

#define T3D_MSG_LEN_64( len ) ( ( (len) / 8 ) + ( ( (len) % 8 ) ? 1 : 0 ) )
#define T3D_MSG_LEN_32( len ) ( ( (len) / 4 ) + ( ( (len) % 4 ) ? 1 : 0 ) )

#define T3D_CHECK_TARGET( target, length )                                  \
{									    \
   int   tpl = (int)target + length;                                        \
   int   tml = (int)target - length;                                        \
									    \
   save_stack = get_stack();                                                \
   modified_stack = 0;                                                      \
/*   if (DebugFlag) { */         					    \
/*     T3D_Printf("Current stack = 0x%x\n",save_stack); */                  \
/*     T3D_Printf("Current heap  = 0x%x\n",t3d_heap_limit); */		    \
/*     T3D_Printf("Sending %d bytes to 0x%x\n",length,target); */	    \
/*   } */                                                                   \
   if ( (tml < (int)save_stack) && (tpl > (int)t3d_heap_limit ) )           \
     if ( abs( (int)save_stack - tml ) < abs( tpl - (int)t3d_heap_limit ) ){\
       shmem_stack( (void *)tml );                                          \
       modified_stack = 1;                                                  \
/*       if (DebugFlag) */                                                  \
/*         T3D_Printf("New stack = 0x%x\n",get_stack()); */                 \
     }                                                                      \
     else {                                                                 \
       malloc_brk( (char *)(tpl) );                                         \
       t3d_heap_limit = (char *)sbrk( 0 );				    \
/*       if (DebugFlag) */						    \
/*         T3D_Printf("New heap     = 0x%x\n",t3d_heap_limit); */    	    \
     }									    \
}

#define T3D_RESET_STACK                                                     \
{                                                                           \
   if ( modified_stack ) set_stack( save_stack );                           \
}

extern volatile T3D_Buf_Status *t3d_dest_bufs;

#endif
