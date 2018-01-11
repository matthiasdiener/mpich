/*
 *  $Id: dm.h,v  Exp $
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

#define MPIDPATCHLEVEL "0.1"
#define MPIDTRANSPORT "Native Paragon (Non chamelon)"

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


typedef struct {
    long 	  id;             /* Paragon NX message id */
    	  /* The following describes the buffer to be sent */
    void          *start;
    int           bytes_as_contig;
} MPID_HANDLE;

typedef struct {
    long          id;             /* Paragon NX message id */
          /* The following describes the buffer to be sent */
    void          *start;
    int           bytes_as_contig;
    long	recvid;
    long	rtq_id;
}MPID_SHANDLE;
typedef struct {
    long 	  id;             /* Paragon NX message id */
    	  /* The following describes the buffer to be sent */
    char          *start;
    int           bytes_as_contig;
    char sync_buf[16];		/* to recv SEND_SYNC id on < 16 byte buffer */
    char 	  *save_start;  /* real address of < 8 byte buffer */
} MPID_RHANDLE;

/* Properties of the device which impact higher level code */
#define MPID_CAN_SEND_CONTIG

/* 
   Since allocation is done by placing the device structure directly into
   the MPIR_?HANDLE, we don't need to allocate space.  We do, however, take
   this opportunity to initialize it...

   The ...reuse... versions are for persistent handles (e.g., MPI_Send_init)
 */
#define MPID_Alloc_send_handle( ctx, a ) ((a)->id = -1)
#define MPID_Alloc_recv_handle( ctx, a ) ((a)->id = -1, (a)->save_start = 0)
#define MPID_Free_send_handle( ctx, a )  
#define MPID_Free_recv_handle( ctx, a )  
#define MPID_Reuse_send_handle( ctx, a ) ((a)->id = -1)
#define MPID_Reuse_recv_handle( ctx, a ) ((a)->id = -1)
#define MPID_Set_send_is_nonblocking( ctx, a, v ) 
#define MPID_Set_recv_is_nonblocking( ctx, a, v ) 

/* Contact with the device layer is made here.  These call the
   routines to actually process a message 

   We use different names to enable the use of a multi-protocol system
   (planned for future support)
 */

/* Ready mode operations are treated the same as normal mode ones.
 */
#define MPID_Post_send(ctx,dmpi_send_handle) \
    MPID_NX_Post_send(dmpi_send_handle) 
#define MPID_Post_send_ready(ctx,dmpi_send_handle) \
    MPID_NX_Post_send(dmpi_send_handle) 
#define MPID_Post_send_sync(ctx,dmpi_send_handle) \
    MPID_NX_Post_send_sync(dmpi_send_handle) 
#define MPID_Complete_send(ctx,dmpi_send_handle) \
    MPID_NX_Complete_send(dmpi_send_handle) 
#define MPID_Blocking_send(ctx, dmpi_send_handle) \
    MPID_NX_Blocking_send(dmpi_send_handle)
#define MPID_Blocking_send_ready(ctx, dmpi_send_handle) \
    MPID_NX_Blocking_send(dmpi_send_handle)
#define MPID_Test_send( ctx, dmpi_send_handle ) \
    MPID_NX_Test_send(dmpi_send_handle)


#define MPID_Post_recv(ctx,dmpi_recv_handle ) \
    MPID_NX_Post_recv(dmpi_recv_handle ) 
#define MPID_Blocking_recv(ctx, dmpi_recv_handle ) \
    MPID_NX_Blocking_recv(dmpi_recv_handle) 
#define MPID_Complete_recv(ctx,dmpi_recv_handle) \
    MPID_NX_Complete_recv(dmpi_recv_handle) 
#define MPID_Test_recv( ctx, dmpi_recv_handle ) \
    MPID_NX_Test_recv(dmpi_recv_handle)

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
        MPID_NX_Test_send(&(request)->shandle) :  \
        MPID_NX_Test_recv(&(request)->rhandle))

#define MPID_Clr_completed( ctx, request ) 
#define MPID_Set_completed( ctx, request ) 
#define MPID_Check_device(  ctx, blocking ) 

#define MPID_Iprobe( ctx, tag, source, context_id, flag, status ) \
    MPID_NX_Iprobe(tag, source, context_id, flag, status ) 
#define MPID_Probe( ctx, tag, source, context_id, status ) \
    MPID_NX_Probe(tag, source, context_id, status ) 

#define MPID_NODE_NAME( ctx, name, len ) MPID_NX_Node_name( name, len )
#define MPID_Version_name( ctx, name )   MPID_NX_Version_name( name )
#define MPID_WTIME(ctx)                  MPID_NX_Wtime()
#define MPID_WTICK(ctx)                  MPID_NX_Wtick()
#define MPID_INIT(argc,argv)             MPID_NX_Init( (argc), (argv) )
#define MPID_END(ctx)                    MPID_NX_End()

#define MPID_ABORT( ctx, errorcode )     MPID_NX_Abort( errorcode );

#define MPID_CANCEL( ctx, r )            MPID_NX_Cancel( r )

#define MPID_Myrank( ctx, rank )         MPID_NX_Myrank( rank )
#define MPID_Mysize( ctx, size )         MPID_NX_Mysize( size )

/* Various functions which make little sense */
#define MPID_SetPktSize(size) (printf("MPID_SetPktSize(%d) ignored\n",size))

/* thread locking.  Single-thread devices will make these empty 
   declarations */
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

/* 
   Context and Communicator operations
 */

#define MPID_Comm_init(ctx,comm,newcomm) MPID_NX_Comm_init(comm,newcomm)
#define MPID_Comm_free(ctx,comm) MPID_NX_Comm_free(comm)

/* This device prefers that the data be prepacked (at least for now) */
#define MPID_PACK_IN_ADVANCE
#define MPID_RETURN_PACKED

/* This device has a limited number of contexts, so tell the rest of 
 * the system that, so that context allocation can be done sparingly.
 */
#define MPID_MAX_CONTEXT 32767

#define MPID_TAG_UB 0x3fffffff
#ifdef PTS_13504_FIXED
#define MPID_NODE_MASK 0x00004000
#else
#define MPID_NODE_MASK 0
#endif

#ifdef MPID_DEVICE_CODE
/* Device-only information */
#include "../../include/dmpiatom.h"

#define MPID_TAG_MASK MPID_TAG_UB


/* End of code included only when building the ADI routines */

#endif /* MPID_DEVICE_CODE */

extern void (*MPID_ErrorHandler)();
extern void MPID_DefaultErrorHandler();

#endif




