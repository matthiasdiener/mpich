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
   of channel.h are encourage to provide the trace calls; note that as macros,
   they can be completely removed at compile time for more 
   performance-critical systems.

 */
/* Do we need stdio here, or has it already been included? */

#if defined(MPI_hpux)
#define MPID_MAX_NODES 16
#define MPID_MAX_PROCS_PER_NODE 8
#define MPID_MAX_PROCS MPID_MAX_NODES*MPID_MAX_PROCS_PER_NODE
#define MPID_MAX_SPP 16777216
#define MPID_SPP_MAX_PKTS_PER_NODE 128
#else
#define MPID_MAX_PROCS 32
#define MPID_MAX_SPP 4194304
#define MPID_SPP_MAX_PKTS 128
#endif
#if HAS_VOLATILE || defined(__STDC__)
#define VOLATILE volatile
#else
#define VOLATILE
#endif

typedef struct {
    int          size;           /* Size of barrier */
    VOLATILE int phase;          /* Used to indicate the phase of this 
				    barrier; only process 0 can change */
    VOLATILE int cnt1, cnt2;     /* Used to perform counts */
    } MPID_SHMEM_Barrier_t;

typedef struct {
    p2p_lock_t globlock;                   /* protects mods to globid */
    VOLATILE int        globid;           /* Used to get my id in the world */
    MPID_SHMEM_Barrier_t barrier;         /* Used for barriers */
    } MPID_SPP_globmem;	

typedef struct {
    p2p_lock_t *availlockPtr[MPID_MAX_PROCS];    /* locks on avail list */
    p2p_lock_t *incominglockPtr[MPID_MAX_PROCS]; /* locks on 
							 incoming list */
    VOLATILE MPID_PKT_T **availPtr[MPID_MAX_PROCS];     /* Availlist for each */
    VOLATILE MPID_PKT_T **incomingPtr[MPID_MAX_PROCS];  /* Incoming messages */
    VOLATILE MPID_PKT_T **incomingtailPtr[MPID_MAX_PROCS];   /* Tails of 
							      incoming msgs */
    } MPID_SPP_ptrmem;	

typedef struct {
    /* locks may need to be aligned, so keep at front (p2p_shmalloc provides
       16-byte alignment for each allocated block) */
    p2p_lock_t availlock[MPID_MAX_PROCS_PER_NODE];    /* locks on avail list */
    p2p_lock_t incominglock[MPID_MAX_PROCS_PER_NODE]; /* locks on 
							 incoming list */
    VOLATILE MPID_PKT_T *avail[MPID_MAX_PROCS_PER_NODE];     /* Availlist for each */
    VOLATILE MPID_PKT_T *incoming[MPID_MAX_PROCS_PER_NODE];  /* Incoming messages */
    VOLATILE MPID_PKT_T *incomingtail[MPID_MAX_PROCS_PER_NODE];   /* Tails of 
							      incoming msgs */
    VOLATILE MPID_PKT_T pool[MPID_SPP_MAX_PKTS_PER_NODE];    /* Preallocated pkts */
    } MPID_SPP_pktmem;	

extern MPID_SPP_globmem *MPID_shmem;
extern MPID_SPP_ptrmem MPID_shmemptr;
extern MPID_SPP_pktmem *MPID_pktmem;
extern int                 MPID_myid;
extern int                 MPID_numids;
extern VOLATILE MPID_PKT_T *MPID_local, **MPID_incoming;

#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_MyWorldRank \
    MPID_myid
#define MPID_WorldSize \
    MPID_numids

#define MPID_RecvAnyControl( pkt, size, from ) \
    { MPID_TRACE_CODE("BRecvAny",-1);\
      MPID_SPP_ReadControl( pkt, size, from );\
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
      MPID_SPP_SendControl( pkt, size, channel );\
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
