/*
 *  $Id: t3d.h,v 1.5 1995/07/24 05:12:10 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef _T3D_INCLUDED
#define _T3D_INCLUDED

/***************************************************************************
  Internally, we have consistant naming.  Externally, some other names
  are expected.
 ***************************************************************************/
#define MPID_INIT                MPID_Init
#define MPID_END                 MPID_End
#define MPID_CANCEL              MPID_Cancel
#define MPID_ABORT               MPID_Abort
#define MPID_SetPktSize          MPID_Set_pkt_size
#define MPID_SetSpaceDebugFlag   MPID_Set_space_debug_flag
#define MPID_SetSendDebugFlag    MPID_Set_send_debug_flag
#define MPID_WTIME               MPID_Wtime
#define MPID_WTICK               MPID_Wtick


/***************************************************************************
  API include files
 ***************************************************************************/
#include "mpi.h"
#include "mpir.h"
#include "dmpiatom.h"
#include "mpi_errno.h"

/***************************************************************************
  Various type definitions
 ***************************************************************************/
typedef enum { MPID_NOTBLOCKING = 0, MPID_BLOCKING } MPID_BLOCKING_TYPE;


/***************************************************************************
  Thread locking.  These are currently empty.
 ***************************************************************************/
#define MPID_THREAD_LOCK(ctx,comm)
#define MPID_THREAD_UNLOCK(ctx,comm)
#define MPID_THREAD_LOCK_INIT(ctx,comm)
#define MPID_THREAD_LOCK_FINISH(ctx,comm)

/* These four are for locking individual data-structures.  The data-structure
   should contain something like
   typedef struct {
      MPID_THREAD_DS_LOCK_DECLARE
      other stuff
      } foo;
   and then use
   foo *p;
   MPID_THREAD_DS_LOCK(p)
   MPID_THREAD_DS_UNLOCK(p)
 */
#define MPID_THREAD_DS_LOCK_DECLARE
#define MPID_THREAD_DS_LOCK_INIT(p)
#define MPID_THREAD_DS_LOCK(p)
#define MPID_THREAD_DS_UNLOCK(p)

#define MPID_Clr_completed(ctx, request)  ((request)->chandle.completer = 1) 
#define MPID_Ctx( request )                (request)->chandle.comm->ADIctx
#define MPID_Set_completed( ctx, request ) (request)->chandle.completer = 0 

#define MPID_Test_request( ctx, request ) \
    ( (request)->chandle.handle_type == MPIR_SEND ? \
        T3D_Test_send(&(request)->shandle) : \
        T3D_Test_recv(&(request)->rhandle))

/***************************************************************************
  MPID routines to access send/recv handles.

     Since allocation is done by placing the device structure directly into
     the MPIR_?HANDLE, we don't need to allocate space.  We do, however, take
     this opportunity to initialize it...

     The ..reuse.. versions are for persistant handles (e.g., MPI_Send_init)
 ***************************************************************************/
#define MPID_Alloc_send_handle( ctx, a )                                    \
{									    \
   (a)->body_sent                       = 0;                                \
   (a)->long_send_info.target_buffer    = (char *)0;			    \
   (a)->long_send_info.target_completer = (char *)0;			    \
   (a)->long_send_info.length           = -1;                               \
}

#define MPID_Alloc_recv_handle( ctx, a )                                    \
{  (a)->temp                = (char *)0; 				    \
   (a)->needs_copy          = 0;					    \
   (a)->sync_recv_completed = (char *)0;				    \
   (a)->mode                = -1;                                           \
}

#define MPID_Free_send_handle( ctx, a )	
#define MPID_Free_recv_handle( ctx, a )
#define MPID_Reuse_send_handle( ctx, a )                                    \
{                                                                           \
   (a)->body_sent                       = 0;                                \
   (a)->long_send_info.target_buffer    = (char *)0;			    \
   (a)->long_send_info.target_completer = (char *)0;			    \
   (a)->long_send_info.length           = -1;                               \
}

    
#define MPID_Reuse_recv_handle( ctx, a )                                    \
{  (a)->temp                = (char *)0; 				    \
   (a)->needs_copy          = 0;					    \
   (a)->sync_recv_completed = (char *)0;				    \
   (a)->mode                = -1;                                           \
}

#define MPID_Set_send_is_nonblocking( ctx, a, v ) (a)->is_non_blocking = v
#define MPID_Set_recv_is_nonblocking( ctx, a, v ) (a)->is_non_blocking = v


/* This device prefers that the data be prepacked (at least for now) */
#define MPID_PACK_IN_ADVANCE
#define MPID_RETURN_PACKED


/****************************************************************************
  True/False defines
 ***************************************************************************/
#ifndef T3D_TRUE
#define T3D_TRUE  1
#define T3D_FALSE 0
#endif

typedef struct {
  void *target_buffer;     /* location of remote buffer         */
  void *target_completer;  /* location of remote completer flag */
  int   completer_value;   /* completer flag value to send      */
  int   length;            /* how much to send                  */
} T3D_Long_Send_Info;

extern struct _MPIR_QEL;

/****************************************************************************
  Device send and receive handles
 ***************************************************************************/
typedef struct {
    int                 is_non_blocking;
    int                 mode;
    void               *start;
    int                 bytes_as_contig;
    int                 body_sent;
    T3D_Long_Send_Info  long_send_info;
    struct _MPIR_QEL   *p;
} MPID_SHANDLE;

typedef struct {
    int          is_non_blocking;
    void        *temp;
    int          mode;
    int          from;
    void        *start;
    int          bytes_as_contig;
    int          needs_copy;
    char        *sync_recv_completed;
    void        *dmpi_unexpected;
} MPID_RHANDLE;

/***************************************************************************
  ADI include files
 ***************************************************************************/
#include "t3doptions.h"
#include "t3ddebug.h"
#include "t3dinit.h"
#include "t3dcoll.h"
#include "t3devent.h"
#include "t3dprobe.h"
#include "t3drecv.h"
#include "t3dsend.h"
#include "t3dsync.h"

extern char *get_stack();

#endif
