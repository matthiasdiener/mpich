/*
 *  $Id: t3dinit.h,v 1.2 1995/06/07 06:26:05 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3DINIT_INCLUDED
#define _T3DINIT_INCLUDED

/***************************************************************************
   Rename the device names to device-independent names 
      i.e.  T3D_Xxxxx_xxx  ->  MPID_Xxxxx_xxx
 ***************************************************************************/
#define MPID_Set_pkt_size( len ) 
#define MPID_Set_space_debug_flag( f )   T3D_Set_space_debug_flag( f )
#define MPID_NODE_NAME( ctx, name, len ) T3D_Node_name( name, len )
#define MPID_Version_name( ctx, name )   T3D_Version_name( name )
#define MPID_Wtime(ctx)                  T3D_Wtime()
#define MPID_Wtick(ctx)                  T3D_Wtick()
#define MPID_Init(argc,argv)             T3D_Init( argc, argv )
#define MPID_End(ctx)                    T3D_End()
#define MPID_Abort( ctx, errorcode )     T3D_Abort( errorcode );
#define MPID_Myrank( ctx, rank )         T3D_Myrank( rank )
#define MPID_Mysize( ctx, size )         T3D_Mysize( size )


/***************************************************************************
  Value for undefined variables (same as for API layer)
 ***************************************************************************/
#define T3D_UNDEFINED MPI_UNDEFINED


/***************************************************************************
  Memory allocation defines
 ***************************************************************************/
#define T3D_MALLOC malloc
#define T3D_CALLOC calloc
#define T3D_FREE   free

#define T3D_MY_PE   t3d_myid
#define T3D_NUM_PES t3d_num_pes
#define T3D_HOSTNAME_LEN 64


#define IS_8BYTE_ALIGNED(addr)                    ( ! ( (int)(addr) & 0x7) )
#define IS_4BYTE_ALIGNED(addr)                    ( ! ( (int)(addr) & 0x3) )

#define T3D_MEMCPY( target, source, bytes )  memcpy( target, source, bytes )

/***************************************************************************
  Global variables defined in t3dinit.c
 ***************************************************************************/
extern char       t3d_hostname[];
extern int        t3d_myid;
extern int        t3d_num_pes;
extern char      *t3d_heap_limit;
extern int        modified_stack;
extern char      *save_stack;

#endif
