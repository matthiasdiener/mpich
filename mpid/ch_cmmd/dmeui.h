


/*
 *  $Id: dmch.h,v 1.19 1994/06/27 15:32:12 gropp Exp gropp $
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

/* 
   Redefine the names of the async communication types (typedefed in 
   comm$(COMM).h) so that a Chameleon program can include commmpi.h which
   includes this file (through the mpi.h file)  

   I'm not sure that this does what I need, but it doesn't hurt.
 */
#define ASYNCRecvId_t MPIDCH_ASYNCRecvId_t
#define ASYNCSendId_t MPIDCH_ASYNCSendId_t

/* 
   When we compile the device, we want to include all of the device code,
   but when we compile user code, we don't want to require that they load 
   the defintions in either tools.h or comm/comm.h.  
 */


#ifdef MPID_DEVICE_CODE

extern int __NUMNODES,__MYPROCID,__ALLGRP,__EUILEN,__EUIFROM,__EUITYPE;typedef long ASYNCRecvId_t;typedef long ASYNCSendId_t;static double __EUI_DBL;
#else
typedef long ASYNCRecvId_t;
typedef long ASYNCSendId_t;
#endif

#include "mpi.h"
#include "dmpiatom.h"

/* 
   This packet size should be selected such that
   (s + r*(n+h)) + c*n \approx (s+r*n) + s+r*h
   where s = latency, r = time to send a byte, n = total message length, 
   h = header size, and c = time to copy a byte.  This condition reduces to
   c n \approx s
   For a typical system with
   s = 30us
   c = .03us/byte
   this gives
   n = s / c = 30 us / (.03us/byte) = 1000 bytes

   When the message does not fit into a single packet, ALL of the message
   should be placed in the "extension" packet (see below).  This removes 
   an extra copy from the code.
 */
#define MPID_PACKET_SIZE 1024


/*
  The implementation reserves some message tags.

  (An optimization is to allow the use of all but a few very large tags
  for messages in the initial communicator, thus eliminating a separate
  header.  Messages in a different communicator would be sent on a reserved
  set of tags.  An alternate is to use the Chameleon tags for communicator
  types, making the message-passing system handle the queueing of messages
  by communicator

  PT2PT_TAG is the tag for short messages and the headers of long messages
  PT2PT2_TAG(src) is the tag for longer messages (by source).  This permits
  the header messages to be freely received into preallocated buffers, and
  for long messages to be received directly into user-buffers.

  The mode field is overloaded for the synchronous case because we support
  NONBLOCKING, SYNCHRONOUS sends; thus, there can be a variety of outstanding
  synchronous sends at any time, and we have to match them up.

  We do this by making the mode field look like this:

  <syncreqnum><modetype>

  The mode field is required because, while there are different sends for each
  mode, there is only one kind of receive, and hence we need the mode field
  to decide what to do.  In fact, our only need is to handle MPI_SYNCHRONOUS
  sends.
 */
#define MPID_PT2PT_TAG 0
#define MPID_PT2PT2_TAG(src) (1+(src))

/* Whether an operation should block or not */
typedef enum { MPID_NOTBLOCKING = 0, MPID_BLOCKING } MPID_BLOCKING_TYPE;

/*
   The mode field contains an ID if the mode is SYNCHRONOUS 
 */
#define MPID_MODE_MASK 0x7
#define MPID_MODE_BITS   3
typedef struct {
    int len,			/* TOTAL length of message in BYTES */
        tag,			/* Message tag */
        context_id,		/* Internal communicator ID */
        mode,                   /* mode (standard, ready, synchronous,
				   sync_ack) */
        lrank;                  /* rank in sending context */
    char buffer[MPID_PACKET_SIZE];
				/* Minimum message size */
    } MPID_PACKET;
/* HeaderLen is just the length of the envelope of MPID_PACKET */
#define MPID_HEADER_LEN (sizeof(MPID_PACKET)-MPID_PACKET_SIZE)

#define MPID_HEADER_INTS 5    /* Number of ints in the header */
/*
   Another option would be for the device handle to contain the initial
   packet.  I have NOT do this so as to keep down the size of the device
   handle (since I want relatively large packets)

   This needs to be structured so that there is just enough here for mpir.h.
 */
typedef struct {
    MPIR_BOOL     done;             /* done is set when the message has
				       been sent and, if a SYNC mode,
				       the ack has been received */
    int           is_non_blocking;
    ASYNCSendId_t sid;              /* Id of non-blocking send, if used.
				       0 if no non-blocking send used */
        /* The following describes the buffer to be sent */
    void          *start;
    int           bytes_as_contig;
    MPIR_NODETYPE dataelement;      /* Will eventually hold datatype for
				       heterogeneous systems */
    } MPID_SHANDLE;

typedef struct {
    MPIR_BOOL     done;
    int           is_non_blocking;
    ASYNCRecvId_t rid;              /* Id of non-blocking recv, if used.
				       0 if no non-blocking recv used */
    char          *temp;            /* Holds body of unexpected message */
    int           mode;             /* mode bits and sequence number; needed
				       for unexpected messages */
    int           from;             /* Absolute process number that sent
				       message; used only for SYNC ack */

        /* The following describes the buffer to be received */
    void          *start;
    int           bytes_as_contig;
    MPIR_NODETYPE dataelement;      /* Will eventually hold datatype for
				       heterogeneous systems */

    } MPID_RHANDLE;

#define MPID_MIN(a,b) ((a) < (b) ? (a) : (b))

#define MPID_CAN_SEND_CONTIG

/* 
   Since allocation is done by placing the device structure directly into
   the MPIR_?HANDLE, we don't need to allocate space.  We do, however, take
   this opportunity to initialize it...

   The ...reuse... versions are for persistant handles (e.g., MPI_Send_init)
 */
#define MPID_alloc_send_handle( a ) {(a)->done = MPIR_NO;}
#define MPID_alloc_recv_handle( a ) {(a)->done = MPIR_NO;(a)->temp  = 0;}
#define MPID_free_send_handle( a )  
#define MPID_free_recv_handle( a )  if ((a)->temp  ) {FREE((a)->temp);}
#define MPID_reuse_send_handle( a ) {(a)->done = MPIR_NO;}
#define MPID_reuse_recv_handle( a ) {(a)->done = MPIR_NO;(a)->temp  = 0;}
#define MPID_set_send_is_nonblocking( a, v ) (a)->is_non_blocking = v
#define MPID_set_recv_is_nonblocking( a, v ) (a)->is_non_blocking = v

/* Contact with the device layer is made here.  These call the
   routines to actually process a message (see dmch.c) 

   We use different names to enable the use of a multi-protocol system
   (planned for future support)
 */
#define MPID_post_send(dmpi_send_handle) \
    MPID_EUI_post_send(dmpi_send_handle) 
#define MPID_post_recv(dmpi_recv_handle, is_available ) \
    MPID_EUI_post_recv(dmpi_recv_handle, is_available ) 
#define MPID_blocking_recv(dmpi_recv_handle ) \
    MPID_EUI_blocking_recv(dmpi_recv_handle) 
#define MPID_complete_recv(dmpi_recv_handle) \
    MPID_EUI_complete_recv(dmpi_recv_handle) 
#define MPID_complete_send(dmpi_send_handle,status) \
    MPID_EUI_complete_send(dmpi_send_handle,status) 
#define MPID_check_device( blocking ) \
    MPID_EUI_check_device( blocking )
#define MPID_iprobe( tag, source, context_id, flag, status ) \
    MPID_EUI_iprobe( tag, source, context_id, flag, status ) 
#define MPID_probe( tag, source, context_id, status ) \
    MPID_EUI_probe( tag, source, context_id, status ) 

#define MPID_NODE_NAME( name, len ) \
    MPID_EUI_Node_name( name, len )
#define MPID_WTIME()         MPID_EUI_Wtime()
#define MPID_WTICK()         MPID_EUI_Wtick()
#define MPID_INIT(argc,argv) MPID_EUI_init( argc, argv )
#define MPID_END()           MPID_EUI_End()

#define MPID_ABORT( errorcode ) MPID_EUI_Abort( errorcode );

#define MPID_CANCEL( r ) MPID_EUI_Cancel( r )

#define MPID_myrank( rank ) MPID_EUI_myrank( rank )
#define MPID_mysize( size ) MPID_EUI_mysize( size )

/* thread locking.  Single-thread devices will make these empty 
   declarations */
#define MPID_THREAD_LOCK(comm)
#define MPID_THREAD_UNLOCK(comm)

/* This device prefers that the data be prepacked (at least for now) */
#define MPID_PACK_IN_ADVANCE
#define MPID_RETURN_PACKED

/* Forward refs */
#ifdef __STDC__
extern void MPID_EUI_Abort( );
extern void MPID_EUI_myrank( int * ), MPID_EUI_mysize( int * ), 
            MPID_EUI_End(void);
/* 
extern int MPID_EUI_post_send( MPIR_SHANDLE * ), 
           MPID_EUI_blocking_recv( MPIR_SHANDLE *), 
           MPID_EUI_complete_recv( MPIR_SHANDLE *),
           MPID_EUI_complete_send( MPIR_SHANDLE *, MPI_Status *);
 */
/* MPID_EUI_check_device( blocking ), 
   MPID_EUI_iprobe( tag, source, context_id, flag, status ), 
   MPID_EUI_probe( tag, source, context_id, status ) 
 */
#else
extern void MPID_EUI_Abort();
extern void MPID_EUI_myrank(), MPID_EUI_mysize(), MPID_EUI_End();
#endif

#endif

#ifdef MPID_DEVICE_CODE
/* Device-only information */
#include "../../include/dmpiatom.h"

/* For heterogeneous support --- NOT YET FULLY IMPLEMENTED 
   (fully means that there is some code that is not yet used).
   This provides information on how data should be communicated to 
   a processor.  The approach is to only convert data when
   the formats are different on the source and destination, and then to
   use byte-swapping code rather than xdr on the SENDER where possible.
   In a heterogeneous environment, the receiver need only check for 
   a sender that had to use MPID_H_XDR; otherwise, the received data is
   already in the correct format.

   None of this code is executed in a homogeneous environment
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

#if defined(p4) || defined(pvm) || defined(pvm3)
#define MPID_HAS_HETERO
extern int MPID_IS_HETERO;
#endif
#endif

#else
/* Allow a Chameleon program to have these symbols */
/* #undef ASYNCRecvId_t  */
/* #undef ASYNCSendId_t */
#endif

