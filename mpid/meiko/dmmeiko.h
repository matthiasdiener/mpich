/*
 *  $Id: dmmeiko.h,v 1.5 1995/05/26 14:21:02 jim Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* 
   These are specific to the native Meiko device

   See the readme file for details
 */


#ifndef _DMCH_INCLUDED
#define _DMCH_INCLUDED

#define MPIDPATCHLEVEL "1.6"
#define MPIDTRANSPORT "Native MEIKO (single tport)"

/* This indicates that the ADI defines the debug routines
   (MPID_SetSendDebugFlag, MPID_SetRecvDebugFlag, MPID_SetSpaceDebugFlag,
   and MPID_SetMsgDebugFlag).
   Note that the MPID_SetSpaceDebugFlag may work only for the Chameleon
   device
 */
/* #define MPID_HAS_DEBUG */
#undef MPID_HAS_DEBUG

/* 
   When we compile the device, we want to include all of the device code,
   but when we compile user code, we don't want to require that they load 
   the defintions in either tools.h or comm/comm.h.  
 */

#ifdef MPID_DEVICE_CODE
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#else
/* 
   These are used to provide a simple type for the rest of the MPI (API)
   code that needs to have this information when handling the device 
   structures
 */
#endif

#include "mpi.h"
#include "dmpiatom.h"

/* Undefine MPID_DEBUG_ALL to remove the debugging code from the device 
   In order to READ the device code without the debugging statements,
   see the script rmdebug.
 */
#ifndef MPID_DEBUG_NONE
#define MPID_DEBUG_ALL
#endif

/* Define this as null to eliminate keeping statistics on use */
#ifndef MPID_STAT_NONE
#define MPID_KEEP_STAT(a) a
#else
#define MPID_KEEP_STAT(a)
#endif

#if defined(MPID_NOT_HETERO) && !defined(MPID_HAS_HETERO)
#define MPID_Aint void *
#else
#define MPID_Aint long
#endif

/* Whether an operation should block or not */
typedef enum { MPID_NOTBLOCKING = 0, MPID_BLOCKING } MPID_BLOCKING_TYPE;


/*
   Another option would be for the device handle to contain the initial
   packet.  I have NOT do this so as to keep down the size of the device
   handle (since I want relatively large packets).  It also helps hide
   the precise form of the packet from the upper layers.

   This needs to be structured so that there is just enough here for mpir.h.

   At one time, there was a "done" field in the device handle, along
   with a "completed" field in the API/MPI level.  I've removed the
   "done" field; this code now relies on the "completed" field in the
   dmpi_handle.  This does reduce the software separation, but it
   turns out that the ADI needs much of the upper-level's handle. 
   Rather than making both opaque to each other, it would be better
   to  a common data-structure that they could both access, and
   then private parts for and ADI and API.

   The Device handle also used to contain a "NODETYPE" element for
   handling abstract data items.  This should be taken from the 
   API handle instead.

   In a shared-memory version, we may want the device handles to contain
   enough information for the transfer to be handled remotedly (remote
   addresses and sizes).

   Completion status:
   Currently, the completion status of a message (both send and receive) is
   determined as follows:

   if dmpi_xxx_handle->completer == NULL, then message is completed and the only 
   remaining "clean up" is to deallocate the ADI handle.
   (if not zero, it is the  of the routine to call to perform the 
   completion).

   Otherwise, if dev_shandle->sid or dev_rhandle->rid, then there is a 
   non-blocking operation involving that handle that needs to be completed 
   (if a non-blocking operation is not used, then the sid or rid fields
   MUST be set to NULL).  

   In the case of a synchronous message, the completed field may not be set
   until the message is acknowledged.

 */
#ifndef MPID_DEVICE_CODE
typedef void ELAN_EVENT;   /* Only the device should need this definition ! */
#endif

/* We only need one type of handle */
typedef struct {
    ELAN_EVENT    *id;             /* An event to check for completion,
				    * or NULL if the operation has already completed
				    */
    /* The following describes the buffer to be sent */
    void          *start;
    int           bytes_as_contig;
} MPID_HANDLE;

#define MPID_RHANDLE MPID_HANDLE
#define MPID_SHANDLE MPID_HANDLE

/* Properties of the device which impact higher level code */
#define MPID_CAN_SEND_CONTIG

/* If this is defined the source argument passed to the device on
 * receive and probe calls is converted into the COMM_WORLD index, 
 * rather than being left relative to the active communicator.
 */
#define MPID_NEEDS_WORLD_SRC_INDICES

/* 
   Since allocation is done by placing the device structure directly into
   the MPIR_?HANDLE, we don't need to allocate space.  We do, however, take
   this opportunity to initialize it...

   The ...reuse... versions are for persistent handles (e.g., MPI_Send_init)
 */
#define MPID_Alloc_send_handle( ctx, a ) ((a)->id = NULL)
#define MPID_Alloc_recv_handle( ctx, a ) ((a)->id = NULL)
#define MPID_Free_send_handle( ctx, a )  
#define MPID_Free_recv_handle( ctx, a )  
#define MPID_Reuse_send_handle( ctx, a ) ((a)->id = NULL)
#define MPID_Reuse_recv_handle( ctx, a ) ((a)->id = NULL)
#define MPID_Set_send_is_nonblocking( ctx, a, v ) 
#define MPID_Set_recv_is_nonblocking( ctx, a, v ) 

/* Contact with the device layer is made here.  These call the
   routines to actually process a message 

   We use different names to enable the use of a multi-protocol system
   (planned for future support)
 */
#ifndef EW_TPORT_TXSYNC
/* This is rather tacky, but avoids having to include all of the
 * ew includes in the layer above the device
 */
#define EW_TPORT_TXSYNC 1   
#endif

/* Ready mode operations are treated the same as normal mode ones.
 */
#define MPID_Post_send(ctx,dmpi_send_handle) \
    MPID_MEIKO_post_send(dmpi_send_handle, 0) 
#define MPID_Post_send_ready(ctx,dmpi_send_handle) \
    MPID_MEIKO_post_send(dmpi_send_handle, 0) 
#define MPID_Post_send_sync(ctx,dmpi_send_handle) \
    MPID_MEIKO_post_send(dmpi_send_handle, EW_TPORT_TXSYNC) 
#define MPID_Complete_send(ctx,dmpi_send_handle) \
    MPID_MEIKO_complete_send(dmpi_send_handle) 
#define MPID_Blocking_send(ctx, dmpi_send_handle) \
    MPID_MEIKO_Blocking_send(dmpi_send_handle)
#define MPID_Blocking_send_ready(ctx, dmpi_send_handle) \
    MPID_MEIKO_Blocking_send(dmpi_send_handle)
#define MPID_Test_send( ctx, dmpi_send_handle ) \
    MPID_MEIKO_Test_send(dmpi_send_handle)


#define MPID_Post_recv(ctx,dmpi_recv_handle ) \
    MPID_MEIKO_post_recv(dmpi_recv_handle ) 
#define MPID_Blocking_recv(ctx, dmpi_recv_handle ) \
    MPID_MEIKO_blocking_recv(dmpi_recv_handle) 
#define MPID_Complete_recv(ctx,dmpi_recv_handle) \
    MPID_MEIKO_complete_recv(dmpi_recv_handle) 
#define MPID_Test_recv( ctx, dmpi_recv_handle ) \
    MPID_MEIKO_Test_recv(dmpi_recv_handle)

/* This should never be required, since the ADIctx is not used.
 * Therefore expand to something which will likely cause an error
 * if it is ever left in the source.
 */
#define MPID_Ctx( request ) void

/* This is a generic test for completion.  Note that it takes a request.
 * It returns true for completed, false if not. If the request is complete,
 * it finishes up all the device processing for the request.
 *
 * We now ignore the completer field completely, since we don't need it.
 */
#define MPID_Test_request( ctx, request ) \
    ( (request)->chandle.handle_type == MPIR_SEND ? \
        MPID_MEIKO_Test_send(&(request)->shandle) : \
        MPID_MEIKO_Test_recv(&(request)->rhandle))

#define MPID_Clr_completed( ctx, request ) 
#define MPID_Set_completed( ctx, request ) 
#define MPID_Check_device(  ctx, blocking ) 

#define MPID_Iprobe( ctx, tag, source, context_id, flag, status ) \
    MPID_MEIKO_Iprobe(tag, source, context_id, flag, status ) 
#define MPID_Probe( ctx, tag, source, context_id, status ) \
    MPID_MEIKO_Probe(tag, source, context_id, status ) 

#define MPID_NODE_NAME( ctx, name, len ) MPID_MEIKO_Node_name( name, len )
#define MPID_Version_name( ctx, name )   MPID_MEIKO_Version_name( name )
#define MPID_WTIME(ctx)                  MPID_MEIKO_Wtime()
#define MPID_WTICK(ctx)                  MPID_MEIKO_Wtick()
#define MPID_INIT(argc,argv)             MPID_MEIKO_Init( (argc), (argv) )
#define MPID_END(ctx)                    MPID_MEIKO_End()

#define MPID_ABORT( ctx, errorcode )     MPID_MEIKO_Abort( errorcode );

#define MPID_CANCEL( ctx, r )            MPID_MEIKO_Cancel( r )

#define MPID_Myrank( ctx, rank )         MPID_MEIKO_Myrank( rank )
#define MPID_Mysize( ctx, size )         MPID_MEIKO_Mysize( size )

/* Various functions which make little sense */
#define MPID_SetPktSize(size) (printf("MPID_SetPktSize(%d) ignored\n",size))

/* thread locking.  Single-thread devices will make these empty 
   declarations */
#define MPID_THREAD_LOCK(ctx,comm)
#define MPID_THREAD_UNLOCK(ctx,comm)
#define MPID_THREAD_LOCK_INIT(ctx,comm)
#define MPID_THREAD_LOCK_FINISH(ctx,comm)

/* 
   Context and Communicator operations
 */

/*
   Enable collective operations.
   Note: enabling these definitions with the default code (which does nothing)
   won't hurt in terms of correctness but also won't help (The default code
   just says "Sorry, I can't help") when the operations are being initialized.
   
   Code needs to be inserted into Comm_init and Comm_free to make these
   provide any benefit.  This is still under development.
 */
#ifdef MPID_USE_ADI_COLLECTIVE
#define MPID_Comm_init(ctx,comm,newcomm) MPID_MEIKO_Comm_init(comm,newcomm)
#define MPID_Comm_free(ctx,comm) MPID_MEIKO_Comm_free(comm)
#define MPID_Barrier(ctx,comm) MPID_MEIKO_Barrier(comm)
/* See mpich/src/coll/reduceutil.c and reduce.c; defining MPID_Reduce
   make reduceutil use MPID_Reduce_xxx_xxx */
#define MPID_Reduce
#define MPID_Reduce_sum_int(ctx,send,recv,comm) \
      MPID_MEIKO_Reduce_sum_int(send,recv,comm)
#define MPID_Reduce_sum_double(ctx,send,recv,comm) \
      MPID_MEIKO_Reduce_sum_double(send,recv,comm)
/* Others as they are determined */
#else
#define MPID_Comm_init(ctx,comm,newcomm) MPI_SUCCESS
#define MPID_Comm_free(ctx,comm) MPI_SUCCESS
#endif

/* This device prefers that the data be prepacked (at least for now) */
#define MPID_PACK_IN_ADVANCE
#define MPID_RETURN_PACKED

/* This device has a limited number of contexts, so tell the rest of 
 * the system that, so that context allocation can be done sparingly.
 */
#define MPID_MAX_CONTEXT 255

#define MPID_TAG_UB 0x0000ffff

#ifdef MPID_DEVICE_CODE
/* Device-only information */
#include "../../include/dmpiatom.h"

/*
 * To get the source of a message to come out right we have to send it 
 * as well. 
 */

#define MPID_CTX_MASK 0xff000000
#define MPID_CTX_SHFT 24

#define MPID_SRC_MASK 0x00ff0000
#define MPID_SRC_SHFT 16

#define MPID_TAG_MASK MPID_TAG_UB


/* End of code included only when building the ADI routines */

#endif /* MPID_DEVICE_CODE */

extern void (*MPID_ErrorHandler)();
extern void MPID_DefaultErrorHandler();

#endif




