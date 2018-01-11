/*
   Sending and receiving packets

   Packets are sent and received on connections.  In order to simultaneously
   provide a good fit with conventional message-passing systems and with 
   other more direct systems (e.g., sockets), I've defined a set of
   connection macros that are here translated into Chameleon message-passing
   calls.  These are somewhat complicated by the need to provide access to
   non-blocking operations

   These are not yet fully integrated into the code.  

   This file is designed for use with the portable shared memory code from
   p2p.

   In addition, we provide a simple way to log the "channel" operations
   If MPID_TRACE_FILE is set, we write information on the operation (both
   start and end) to the given file.  In order to simplify the code, we
   use the macro MPID_TRACE_CODE(name,channel).  Other implementations
   of channel.h are encouraged to provide the trace calls; note that as macros,
   they can be completely removed at compile time for more 
   performance-critical systems.

 */
/* Do we need stdio here, or has it already been included? */

#define MPID_MAX_PROCS 32
#define MPID_MAX_SHMEM 4194304
#define MPID_SHMEM_MAX_PKTS (4*MPID_MAX_PROCS)
#if HAS_VOLATILE || defined(__STDC__)
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/*
   Notes on the shared data.

   Some of the data may be pointers to shared memory where the POINTERS
   should reside in local memory (for increased efficiency).

   In particularly, the structure MPID_SHMEM_globmem is allocated out of
   shared memory and contains various thinks like the locks.  However, we
   don't want to find the ADDRESS of a lock by looking through some 
   shared memory.  
   Note also that we want all changable data to be on separate cache lines.

   Thus, we want to replace 
     VOLATILE MPID_PKT_T *(a[MPID_MAX_PROCS]);
   by
     VOLATILE MPID_PKT_PTR_T (a[MPID_MAX_PROCS])
   where
      typedef union { MPID_PTK_T *pkt ; PAD } MPID_PKT_PTR_T ;
   where the PAD is char pad[sizeof-cachline].

   In addition, we want to put data that is guarded together into the
   same cache-line.  However, we may not want to put the locks on the same
   cache-line, since other processes may be attempting to acquire the
   locks.

   Note that there are two structures.  A Queue, for messages (required
   message ordering), and a Stack, for available message packets.

 */

#ifndef MPID_CACHE_LINE_SIZE
#define MPID_CACHE_LINE_SIZE 128
#define MPID_CACHE_LINE_LOG_SIZE 7
#endif
typedef struct {
    VOLATILE MPID_PKT_T *head;
    VOLATILE MPID_PKT_T *tail;
    char                pad[MPID_CACHE_LINE_SIZE - 2 * sizeof(MPID_PKT_T *)];
    } MPID_SHMEM_Queue;

typedef struct {
    VOLATILE MPID_PKT_T *head;
    char                pad[MPID_CACHE_LINE_SIZE - 1 * sizeof(MPID_PKT_T *)];
    } MPID_SHMEM_Stack;

typedef struct {
    /* locks may need to be aligned, so keep at front (p2p_shmalloc provides
       16-byte alignment for each allocated block) */
    p2p_lock_t availlock[MPID_MAX_PROCS];    /* locks on avail list */
    p2p_lock_t incominglock[MPID_MAX_PROCS]; /* locks on incoming list */
    p2p_lock_t globlock;
    MPID_SHMEM_Queue    incoming[MPID_MAX_PROCS];     /* Incoming messages */
    MPID_SHMEM_Stack    avail[MPID_MAX_PROCS];        /* Avail pkts */

    VOLATILE MPID_PKT_T pool[MPID_SHMEM_MAX_PKTS];    /* Preallocated pkts */

    /* We put globid last because it may otherwise upset the cache alignment
       of the arrays */
    VOLATILE int        globid;           /* Used to get my id in the world */
    } MPID_SHMEM_globmem;	

extern MPID_SHMEM_globmem *MPID_shmem;
extern int                 MPID_myid;
extern int                 MPID_numids;
extern MPID_PKT_T          *MPID_local;       /* Local pointer to arrived
						 packets; it is only
						 accessed by the owner */
extern VOLATILE MPID_PKT_T **MPID_incoming;   /* pointer to my incoming 
						 queue HEAD */

#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_MyWorldRank \
    MPID_myid
#define MPID_WorldSize \
    MPID_numids

#define MPID_RecvAnyControl( pkt, size, from ) \
    { MPID_TRACE_CODE("BRecvAny",-1);\
      MPID_SHMEM_ReadControl( pkt, size, from );\
      MPID_TRACE_CODE_PKT("ERecvAny",*(from),\
			  MPID_PKT_RECV_GET(*(pkt),head.mode));}
#define MPID_RecvFromChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BRecvFrom",channel);\
      fprintf( stderr, "message too big and truncated!\n" );\
      MPID_TRACE_CODE("ERecvFrom",channel);}
#define MPID_ControlMsgAvail( ) \
    (MPID_local || *MPID_incoming)
#define MPID_SendControl( pkt, size, channel ) \
    { MPID_TRACE_CODE_PKT("BSendControl",channel,\
			  MPID_PKT_SEND_GET((MPID_PKT_T*)(pkt),head.mode));\
      MPID_SHMEM_SendControl( pkt, size, channel );\
      MPID_TRACE_CODE("ESendControl",channel);}
#define MPID_SendChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BSend",channel);\
	fprintf( stderr, "message too big (%d) and truncated!\n", size  );\
      MPID_TRACE_CODE("ESend",channel);}
#define MPID_SendControlBlock(pkt,size,channel) \
      MPID_SendControl(pkt,size,channel)
#define MPID_SENDCONTROL(mpid_send_handle,pkt,len,dest) \
MPID_SendControl( pkt, len, dest );

/* 
   Non-blocking versions (NOT required, but if PI_NO_NRECV and PI_NO_NSEND
   are NOT defined, they must be provided)
 */
#define MPID_IRecvFromChannel( buf, size, channel, id ) 
#define MPID_WRecvFromChannel( buf, size, channel, id ) 
#define MPID_RecvStatus( id ) 

/* Note that these use the tag based on the SOURCE, not the channel
   See MPID_SendChannel */
#define MPID_ISendChannel( buf, size, channel, id ) 
#define MPID_WSendChannel( buf, size, channel, id ) 

/*
   We also need an abstraction for out-of-band operations.  These could
   use transient channels or some other operation.  This is essentially for
   performing remote memory operations without local intervention; the need
   to determine completion of the operation requires some sort of handle.
 */
/* And here they are. Rather than call them transient channels, we define
   "transfers", which are split operations.  Both receivers and senders
   may create a transfer.

   Note that the message-passing version of this uses the 'ready-receiver'
   version of the operations.
 */
#define MPID_CreateSendTransfer( buf, size, partner, id ) 
#define MPID_CreateRecvTransfer( buf, size, partner, id ) 

#define MPID_StartRecvTransfer( buf, size, partner, id, rid ) 
#define MPID_EndRecvTransfer( buf, size, partner, id, rid ) 
#define MPID_TestRecvTransfer( rid ) 

#define MPID_StartSendTransfer( buf, size, partner, id, sid ) 
#define MPID_EndSendTransfer( buf, size, partner, id, sid ) 
#define MPID_TestSendTransfer( sid ) 

/* Miscellaneous definitions */
#define MALLOC(a) malloc((unsigned)(a))
#define FREE(a)   free(a)
#define SYexitall(a,b) p2p_error(a,b)
