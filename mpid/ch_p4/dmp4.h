#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_HAS_HETERO


/*
 *  $Id: dmch.h,v 1.31 1995/01/03 19:41:22 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* 
   These are the Chameleon-specific macros.

   See the readme file for details
 */


#ifndef _DMCH_INCLUDED
#define _DMCH_INCLUDED

#define MPIDPATCHLEVEL 1.3
#define MPIDTRANSPORT "p4"

#ifdef PI_NO_MSG_SEMANTICS
/* If there isn't enough buffering, or messages can't be received 
   in any order, then NO_MSG_SEMANTICS is defined.  In this case, 
   we try harder to drain messages at the receiving end to reduce the
   possibilities of deadlock.  
 */
#if !defined(MPID_NO_LIMITED_BUFFERS) && !defined(MPID_LIMITED_BUFFERS)
#define MPID_LIMITED_BUFFERS
#endif
#if !defined(MPID_NO_TINY_BUFFERS) && !defined(MPID_TINY_BUFFERS)
#define MPID_TINY_BUFFERS
#endif
#endif

/* This indicates that the ADI defines the debug routines
   (MPID_SetSendDebugFlag, MPID_SetRecvDebugFlag, MPID_SetSpaceDebugFlag,
   and MPID_SetMsgDebugFlag).
   Note that the MPID_SetSpaceDebugFlag may work only for the Chameleon
   device
 */
#define MPID_HAS_DEBUG

/* 
   When we compile the device, we want to include all of the device code,
   but when we compile user code, we don't want to require that they load 
   the defintions in either tools.h or comm/comm.h.  
 */

#ifdef MPID_DEVICE_CODE
#include <stdio.h>

#include "p4.h"
extern int __NUMNODES,__MYPROCID,__P4LEN,__P4TYPE,__P4FROM,__P4GLOBALTYPE;typedef long ASYNCRecvId_t;typedef long ASYNCSendId_t;
/* #MEIKO DECLARATION# */
#else
/* 
   These are used to provide a simple type for the rest of the MPI (API)
   code that needs to have this information when handling the device 
   structures
 */
/* #CMMD DECLARATION# */
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

/* These are used for items that are pointers on homogeneous systems and
   either not used or integers on heterogeneous systems 

   MPID_NOT_HETERO may be used to force a non-heterogeneous system
 */
#if !defined(MPID_NOT_HETERO) && \
 (defined(p4) || defined(pvm) || defined(pvm3) || defined(MPID_HAS_HETERO))
#ifndef MPID_HAS_HETERO
#define MPID_HAS_HETERO
#endif
#endif
#if defined(MPID_NOT_HETERO) && defined(MPID_HAS_HETERO)
#undef MPID_HAS_HETERO
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

   if dmpi_xxx_handle->completed, then message is completed and the only 
   remaining "clean up" is to deallocate the ADI handle.

   Otherwise, if dev_shandle->sid or dev_rhandle->rid, then there is a 
   non-blocking operation involving that handle that needs to be completed 
   (if a non-blocking operation is not used, then the sid or rid fields
   MUST be set to NULL).  

   In the case of a synchronous message, the completed field may not be set
   until the message is acknowledged.

 */
typedef struct {
    int           is_non_blocking;
    int sid;              /* Id of non-blocking send, if used.
				       0 if no non-blocking send used, 
				       or if non-blocking send has 
				       completed */
    void          *pkt;             /* Some systems will require use of
				       non-blocking sends; in these systems,
				       the packets need to be allocated
				       and managed */
        /* The following describes the buffer to be sent */
    void          *start;
    int           bytes_as_contig;
    } MPID_SHANDLE;

typedef struct {
    int           is_non_blocking;
    int rid;              /* Id of non-blocking recv, if used.
				       0 if no non-blocking recv used.
				       Used only if MPID_USE_RNDV set. */
    MPID_Aint     send_id;          /* Used for rendevous send; this is
				       needed when the incoming message
				       is unexpected. Used for the
				       sync_id for messages not received
				       in rendevous mode (not yet 
				       implemented) */
    char          *temp;            /* Holds body of unexpected message */
    int           mode;             /* mode bits and sequence number; needed
				       for unexpected messages */
    int           from;             /* Absolute process number that sent
				       message; used only for SYNC ack */

        /* The following describes the buffer to be received */
    void          *start;
    int           bytes_as_contig;
    } MPID_RHANDLE;

#define MPID_CAN_SEND_CONTIG

/* 
   Since allocation is done by placing the device structure directly into
   the MPIR_?HANDLE, we don't need to allocate space.  We do, however, take
   this opportunity to initialize it...

   The ...reuse... versions are for persistant handles (e.g., MPI_Send_init)
 */
#define MPID_Alloc_send_handle( ctx, a )
#define MPID_Alloc_recv_handle( ctx, a ) {(a)->temp  = 0;}
#define MPID_Free_send_handle( ctx, a )  
#define MPID_Free_recv_handle( ctx, a )  if ((a)->temp  ) {FREE((a)->temp);}
#define MPID_Reuse_send_handle( ctx, a ) 
#define MPID_Reuse_recv_handle( ctx, a ) {(a)->temp  = 0;}
#define MPID_Set_send_is_nonblocking( ctx, a, v ) (a)->is_non_blocking = v
#define MPID_Set_recv_is_nonblocking( ctx, a, v ) (a)->is_non_blocking = v

/* Contact with the device layer is made here.  These call the
   routines to actually process a message 

   We use different names to enable the use of a multi-protocol system
   (planned for future support)
 */
#define MPID_Post_send(ctx,dmpi_send_handle) \
    MPID_P4_post_send(dmpi_send_handle) 
#define MPID_Post_send_ready(ctx,dmpi_send_handle) \
    MPID_P4_post_send(dmpi_send_handle) 
#define MPID_Post_send_sync(ctx,dmpi_send_handle) \
    MPID_P4_post_send_sync(dmpi_send_handle) 
#define MPID_Complete_send(ctx,dmpi_send_handle) \
    MPID_P4_complete_send(dmpi_send_handle) 
#define MPID_Blocking_send(ctx, dmpi_send_handle) \
    MPID_P4_Blocking_send(dmpi_send_handle)
#define MPID_Blocking_send_ready(ctx, dmpi_send_handle) \
    MPID_P4_Blocking_send(dmpi_send_handle)
#define MPID_Test_send( ctx, dmpi_send_handle ) (dmpi_send_handle)->completed

#define MPID_Post_recv(ctx,dmpi_recv_handle, is_available ) \
    MPID_P4_post_recv(dmpi_recv_handle, is_available ) 
#define MPID_Blocking_recv(ctx,dmpi_recv_handle ) \
    MPID_P4_blocking_recv(dmpi_recv_handle) 
#define MPID_Complete_recv(ctx,dmpi_recv_handle) \
    MPID_P4_complete_recv(dmpi_recv_handle) 
#define MPID_Test_recv( ctx, dmpi_recv_handle ) (dmpi_recv_handle)->completed

#define MPID_Check_device( ctx,blocking ) \
    MPID_P4_check_device( blocking )

#define MPID_Iprobe( ctx, tag, source, context_id, flag, status ) \
    MPID_P4_Iprobe( tag, source, context_id, flag, status ) 
#define MPID_Probe( ctx, tag, source, context_id, status ) \
    MPID_P4_Probe( tag, source, context_id, status ) 

#define MPID_NODE_NAME( ctx, name, len ) \
    MPID_P4_Node_name( name, len )
#define MPID_Version_name( ctx, name ) MPID_P4_Version_name( name )
#define MPID_WTIME(ctx)         MPID_P4_Wtime()
#define MPID_WTICK(ctx)         MPID_P4_Wtick()
#define MPID_INIT(argc,argv) MPID_P4_Init( argc, argv )
#define MPID_END(ctx)           MPID_P4_End()

#define MPID_ABORT( ctx, errorcode ) MPID_P4_Abort( errorcode );

#define MPID_CANCEL( ctx, r ) MPID_P4_Cancel( r )

#define MPID_Myrank( ctx, rank ) MPID_P4_Myrank( rank )
#define MPID_Mysize( ctx, size ) MPID_P4_Mysize( size )

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
#define MPID_Comm_init(ctx,comm,newcomm) MPID_P4_Comm_init(comm)
#define MPID_Comm_free(ctx,comm) MPID_P4_Comm_free(comm)
#define MPID_Barrier(ctx,comm) MPID_P4_Barrier(comm)
#define MPID_Reduce_sum_int(ctx,send,recv,comm) \
      MPID_P4_Reduce_sum_int(send,recv,comm)
#define MPID_Reduce_sum_double(ctx,send,recv,comm) \
      MPID_P4_Reduce_sum_double(send,recv,comm)
/* Others as they are determined */
#else
#define MPID_Comm_init(ctx,comm,newcomm) MPI_SUCCESS
#define MPID_Comm_free(ctx,comm) MPI_SUCCESS
#endif

/* This device prefers that the data be prepacked (at least for now) */
#define MPID_PACK_IN_ADVANCE
#define MPID_RETURN_PACKED


#endif

#ifdef MPID_DEVICE_CODE
/* Device-only information */
#include "../../include/dmpiatom.h"

/* Some systems have special, customized memcpy routines (for example,
   using the floating point registers and 8 byte load/stores).  This macro
   gives us an easy way to make use of them, if we can find them */
#ifndef MEMCPY
#define MEMCPY(d,s,n) memcpy(d,s,n)
#endif

extern FILE *MPID_DEBUG_FILE;
/* End of code included only when building the ADI routines */
#endif

/* For heterogeneous support --- NOT YET FULLY IMPLEMENTED 
   (fully means that there is some code that is not yet used).
   This provides information on how data should be communicated to 
   a processor.  The approach is to only convert data when
   the formats are different on the source and destination, and then to
   use byte-swapping code rather than xdr on the SENDER where possible.
   In a heterogeneous environment, the receiver need only check for 
   a sender that had to use MPID_H_XDR; otherwise, the received data is
   already in the correct format.

   None of this code is executed or included in a homogeneous environment
 */
#ifndef MPID_H_INC
#define MPID_H_INC
typedef enum { MPID_H_NONE = 0, 
		   MPID_H_LSB, MPID_H_MSB, MPID_H_XDR } MPID_H_TYPE;
/* 
   The MPID_INFO structure is acquired from each node and used to determine
   the format for data that is sent 
 */
typedef struct {
    MPID_H_TYPE byte_order;
    } MPID_INFO;
extern MPID_INFO *MPID_procinfo;
extern MPID_H_TYPE MPID_byte_order;

extern void (*MPID_ErrorHandler)();
extern void MPID_DefaultErrorHandler();

#ifdef MPID_HAS_HETERO
extern int MPID_IS_HETERO;
#define MPID_Dest_byte_order(dest) MPID_P4_Dest_byte_order(dest)
#endif

#endif

