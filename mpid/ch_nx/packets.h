






/* 
   This file defines the packet/message format that WILL be used in
   the next generation ADI.  
 */

#ifndef MPID_PKT_DEF
#define MPID_PKT_DEF
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

/*
  The implementation reserves some message tags.

  (An optimization is to allow the use of all but a few very large tags
  for messages in the initial communicator, thus eliminating a separate
  header.  Messages in a different communicator would be sent on a reserved
  set of tags.  An alternate is to use the Chameleon tags for communicator
  types, making the message-passing system handle the queueing of messages
  by communicator.  Note that if tags are used to separate communicators,
  message-passing systems that have stream semantics, like EUI-H, will
  fail to operate correctly.  Another approach is to make the tags a 
  combination of tag, context, and source.)

  PT2PT_TAG is the tag for short messages and the headers of long messages
  PT2PT2_TAG(source) is the tag for longer messages (by source).  This permits
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

/*
   The mode field contains an ID if the mode is SYNCHRONOUS 

   This is a very simple, open, single packet structure.  Additional
   performance, particularly for short messages, can be obtained by:
       mode is two bits; if sync or sync_ack, next nn bits are id
       lrank is 8 or 14 bits (depending on size of system; the reason
       for 14 is made clear below).
       len is provided only if a "large message" bit is set; otherwise,
       the length is computed from what is actually delivered.
       context_id is 16 bits
   With these changes, the short, non-synchronous send packet 
   is only 31 (tag) + 1 (short) + 2 (mode) + 16 context_id + 14 (lrank) =
   64 bits.  The packet defined here is 160 bits.

   Similar games can be played for other design points.  Note that the 
   lrank could be determined by looking up the absolute rank in the 
   matching context_id; this approach may be cost effective if many small
   messages are sent on a slow system.
 */

#define MPID_MODE_MASK  0x1f
#define MPID_MODE_BITS    5

#define MPID_MODE_XDR   0x4
#define MPIR_MODE_IS_SYNC(mpid) ((mpid)->mode & (int)MPIR_MODE_SYNCHRONOUS)
#define MPIR_MODE_SYNC_ID(mpid) ((mpid)->mode >> MPID_MODE_BITS)

#ifdef FOO
/* 
   Here are definitions from mpi_bc.h ...
typedef enum { 
    MPIR_MODE_STANDARD = 0, 
    MPIR_MODE_READY = 1, 
    MPIR_MODE_SYNCHRONOUS = 2, 
    MPIR_MODE_RECV = 3,
    MPIR_MODE_SYNC_ACK = 4
 */


/* Mode bits are:
   0x1 - Ready
   0x2 - Synchronous
   0x4 - XDR (two-ended transmission protocol)
   0x8 - Long message (not yet used)
   0x10 - Sync Ack

   THESE MUST BE THE SAME AS THE API USES! (see include/mpi_bc.h)
   Note that the API just uses the first two modes; XDR is indicated
   in a separate word.  Also node that the the API stores the mode and the
   sync_id in a single word; see MPIR_MODE_xxxx below.
 */
#define MPID_MODE_READY 0x1
#define MPID_MODE_SYNC  0x2
#define MPID_MODE_LONG  0x8
#define MPID_MODE_SYNC_ACK 0x10

/* 
   The compressed packet format trades a slightly smaller packet for 
   more complicated code to access the entries.  In addition, it 
   does not send the id of the synchronous message unless the message type
   is indeed synchronous.
 */
#ifdef USE_COMPRESSED_PACKET
/* 
   Long messages have no local data; we let them include the sync_id 
   as the last field (which need not be sent!)
 */
typedef struct {
    int len;                    /* TOTAL length of message in BYTES */
    int sync_id;                /* Id of a synchronous message */
    } MPID_PACKET_LONG;
typedef struct {
    int sync_id;                /* Id of a synchronous message */
	char buffer[MPID_PACKET_SIZE];
				/* Maximum message size for short messages */
    } MPID_PACKET_SYNC;
typedef struct {
    unsigned mode:5;            /* Mode */
    unsigned context_id:16;     /* Context_id */
    unsigned lrank:11;          /* Local rank in sending context */
    int tag;                    /* Tag is a full 32 bits */
    union {
	char buffer[MPID_PACKET_SIZE];
				/* Maximum message size for short messages */
	MPID_PACKET_SYNC s_short;  /* Short, synchronous message */
	MPID_PACKET_LONG s_long;   /* Long message, may be synchronous */
	};
    } MPID_PACKET;

/* Header_Len is just the length of the envelope of MPID_PACKET */
#define MPID_HEADER_LEN (sizeof(MPID_PACKET)-MPID_PACKET_SYNC)

#define MPID_HEADER_INTS (MPID_HEADER_LEN/sizeof(int))  
                           /* Number of ints in the header */
#define MPID_SYNC_GET_ID(pkt)    ((pkt)->sync_id)
#define MPID_SYNC_SET_ID(pkt,id) (pkt)->sync_id = id
#define MPID_MODE_GET_SYNC_WITH_ID(pkt) \
    (((pkt)->sync_id << MPID_MODE_BITS) | (pkt)->mode )
#define MPID_MODE_IS_SYNC(pkt)   ((pkt)->mode & MPID_MODE_SYNC)
#define MPID_MODE_IS_READY(pkt)  ((pkt)->mode & MPID_MODE_READY)
#define MPID_MODE_IS_SYNCACK(pkt) \
    ((pkt)->mode == MPID_MODE_SYNC_ACK)
#define MPID_MODE_HAS_XDR(pkt)   ((pkt)->mode & MPID_MODE_XDR)

#define MPID_MSG_LEN(pkt) \
    ((pkt)->mode & MPID_MODE_LONG ? (ptk)->len : infocount() - MPID_HEADER_LEN)
#define MPID_MSG_SET_LEN(pkt,s) \
    ((pkt)->mode & MPID_MODE_LONG ? (ptk)->len = s : 0)
#else
typedef struct {
    int len,			/* TOTAL length of message in BYTES */
        tag,			/* Message tag */
        context_id,		/* Internal communicator ID */
        mode,                   /* mode (standard, ready, synchronous,
				   sync_ack) */
        lrank;                  /* rank in sending context */
    char buffer[MPID_PACKET_SIZE];
				/* Maximum message size for short messages */
    } MPID_PACKET;

/* HeaderLen is just the length of the envelope of MPID_PACKET */
#define MPID_HEADER_LEN (sizeof(MPID_PACKET)-MPID_PACKET_SIZE)

/* Number of ints in the header */
#define MPID_HEADER_INTS 5    
#define MPID_SYNC_GET_ID(pkt)    ((pkt)->mode >> MPID_MODE_BITS)
#define MPID_SYNC_SET_ID(pkt,id) ((pkt)->mode |= ((id) << MPID_MODE_BITS))
#define MPID_MODE_GET_SYNC_WITH_ID(pkt) (pkt)->mode
#define MPID_MODE_IS_SYNC(pkt)   \
         (((pkt)->mode & MPID_MODE_MASK) == MPIR_MODE_SYNCHRONOUS)
#define MPID_MODE_IS_READY(pkt)  ((pkt)->mode & MPID_MODE_READY)
#define MPID_MODE_IS_SYNCACK(pkt) \
    (((pkt)->mode & MPID_MODE_MASK) == MPID_MODE_SYNC_ACK)
#define MPID_MODE_HAS_XDR(pkt)   ((pkt)->mode & MPID_MODE_XDR)

#define MPID_MSG_LEN(pkt) (pkt)->len
#define MPID_MSG_SET_LEN(pkt,s) (pkt)->len = s
#endif

#endif

#define MPID_MIN(a,b) ((a) < (b) ? (a) : (b))



/***************************************************************************
   What follows is the next-generation packet format.  We may try to 
   incrementally port to it...
 ***************************************************************************/


/* Here are all of the packet types.  We use the first bit to indicate
   short or long; by using small integers, we can use a single 
   select statement to jump to the correct code (here's hoping the 
   compiler generates good code for that!).
   In the cases where the packed is a control packet (neither long nor short),
   the first bit does NOT mean long/short.
 */
typedef enum { MPID_PKT_SHORT=0, MPID_PKT_LONG=1, MPID_PKT_SHORT_SYNC=2,
	       MPID_PKT_LONG_SYNC=3, MPID_PKT_SHORT_READY=4, 
               MPID_PKT_LONG_READY=5, 
               MPID_PKT_REQUEST_SEND=6,
	       MPID_PKT_REQUEST_SEND_READY=7,
               MPID_PKT_OK_TO_SEND=8, MPID_PKT_SYNC_ACK=9, 
	       MPID_PKT_READY_ERROR=10, 
	       MPID_PKT_COMPLETE_SEND=11, MPID_PKT_COMPLETE_RECV=12 } 
    MPID_Pkt_t;

/* Comments on packets */
/* The design here is fairly general, allowing both eager and reluctant 
   (rendevous) protocols for each of the MPI modes.  Some simplification can
   be achieved if, for example, the MPI Synchronous mode is only implemented
   with the reluctant protocol.

   Eager:
   In the eager protocol, the message data is sent without first asking the
   destination node (for example, to check on buffer space).  In order to 
   achieve better use of the underlying message-passing systems, there are
   two major categories: long and short.  In a short message, the message
   data and message envelope (context, tag, lrank) are sent together.
   In a long message, the data is sent separately from the envelope (avoiding
   any data copies).  If the MPI mode is Synchronous or Ready, then 
   some additional information is included.
   In the synchronous case, it is necessary for the receiver to tell the
   sender when the receive has started (but see Rendevous below); this
   is done with the MPID_PKT_SYNC_ACK.
   In the ready case, if the matching receive is NOT available, then an
   error message can be sent back to the sender with MPID_PKT_READY_ERROR.

   In the long message case, the SENDER may use monblocking send operation.
   On some systems, it may be necessary to ask the sender to execute an 
   MPI_Wait-like operation on the nonblocking send in order to get a 
   matching receive to complete.  On these systems, the macro
   MPID_WSEND_BEFORE_WRECV is defined, and the MPI_Wait on the
   send is requested with the MPID_PKT_COMPLETE_SEND packet.  (The systems
   that require this tend to use rendevous or reluctant protocols for long 
   messages).

   Rendevous:
   In the Rendevous protocol, a LONG send is not begun until the receiver
   oks it.  The request is made with MPID_PKT_REQUEST_SEND, and 
   is acknowledged with MPID_PKT_OK_TO_SEND.  Note that a message-passing
   implementation may make use of a "ready-receiver" send once the 
   OK_TO_SEND has been received.  Also note that the rendevous protocol
   handles standard and synchronous sends identically; no separate SYNC_ACK
   packet is required.  

   Depending on the system, it may or may not be advantageous to compress
   the size of the data in the packet.  If the macro MPID_PKT_COMPRESSED
   is defined, then the envelope data is encoded into 2 ints.
 */
   
#ifdef MPID_HAS_HETERO
#define MPID_PKT_XDR_DECL int has_xdr;
#else
#define MPID_PKT_XDR_DECL 
#endif

#ifdef MPID_PKT_COMPRESSED
#define MPID_PKT_MODE  unsigned mode:4;
#define MPID_PKT_BASIC \
    unsigned mode:4;             /* Contains MPID_Pkt_t */             \
    unsigned context_id:16;      /* Context_id */                      \
    unsigned lrank:12;           /* Local rank in sending context */   \
    int      tag;                /* tag is full sizeof(int) */         \
    int      len; \
    MPID_PKT_XDR_DECL
#else
#define MPID_PKT_MODE  int mode;
#define MPID_PKT_BASIC \
    MPID_Pkt_t  mode;       \
    int         context_id; \
    int         lrank;      \
    int         tag;  \
    int         len; \
    MPID_PKT_XDR_DECL
#endif

#ifndef MPID_PKT_MAX_DATA_SIZE
#if !defined(MPID_HAS_HETERO)
/* Probably an MPP with 100 us latency */
#define MPID_PKT_MAX_DATA_SIZE 1024
/* 
   Warning: It must be possible for a blocking send to deliver this much
   data and return unless nonblocking sends are used for control packets
   (currently not available).  This is important on IBM MPL and TMC CMMD.
 */
#else
/* Probably a workstation with 1000 us latency */
#define MPID_PKT_MAX_DATA_SIZE 16384
#endif
#endif

#ifdef MPID_PKT_VAR_SIZE
extern int MPID_PKT_DATA_SIZE;
#else
#define MPID_PKT_DATA_SIZE MPID_PKT_MAX_DATA_SIZE
#endif

#define MPID_PKT_IS_MSG(mode) ((mode) <= MPID_PKT_REQUEST_SEND_READY)
/* 
   One unanswered question is whether it is better to send the length of
   a short message in the short packet types, or to compute it from the
   message-length provided by the underlying message-passing system.
   Currently, I'm planning to send it.  Note that for short messages, I 
   only need another 2 bytes to hold the length (1 byte if I restrict
   short messages to 255 bytes).  The tradeoff here is addition computation
   at sender and receiver versus reduced data-load on the connection between
   sender and receiver.
 */
typedef struct {
    MPID_PKT_BASIC
    } MPID_PKT_HEAD_T;
typedef struct { 
    MPID_PKT_BASIC
    char     buffer[MPID_PKT_MAX_DATA_SIZE];
    } MPID_PKT_SHORT_T;
typedef struct {
    MPID_PKT_BASIC
    } MPID_PKT_LONG_T;
typedef struct { 
    MPID_PKT_BASIC
    char     buffer[MPID_PKT_MAX_DATA_SIZE];
    } MPID_PKT_SHORT_READY_T;
typedef struct {
    MPID_PKT_BASIC
    } MPID_PKT_LONG_READY_T;

typedef struct {
    MPID_PKT_BASIC
    MPID_Aint sync_id;
    char      buffer[MPID_PKT_MAX_DATA_SIZE];
    } MPID_PKT_SHORT_SYNC_T;
typedef struct {
    MPID_PKT_BASIC
    MPID_Aint sync_id;
    } MPID_PKT_LONG_SYNC_T;

typedef struct {
    MPID_PKT_MODE
    MPID_Aint sync_id;
    } MPID_PKT_SYNC_ACK_T;

typedef struct {
    MPID_PKT_MODE
    MPID_Aint send_id;
    } MPID_PKT_COMPLETE_SEND_T;
typedef struct {
    MPID_PKT_MODE
    MPID_Aint recv_id;
    } MPID_PKT_COMPLETE_RECV_T;
typedef struct {
    MPID_PKT_BASIC
    MPID_Aint send_id;
    } MPID_PKT_REQUEST_SEND_T;
typedef struct {
    MPID_PKT_BASIC
    MPID_Aint send_id;
    } MPID_PKT_REQUEST_SEND_READY_T;
typedef struct {
    MPID_PKT_MODE
    MPID_Aint send_id;
    int use_tag;
    } MPID_PKT_OK_TO_SEND_T;
typedef struct {
    MPID_PKT_BASIC
    } MPID_PKT_READY_ERROR_T;

typedef union {
    MPID_PKT_HEAD_T          head;
    MPID_PKT_SHORT_T         short_pkt;
    MPID_PKT_SHORT_SYNC_T    short_sync_pkt;
    MPID_PKT_SHORT_READY_T   short_ready_pkt;
#ifdef MPID_USE_RNDV
    MPID_PKT_REQUEST_SEND_T  request_pkt;
    MPID_PKT_REQUEST_SEND_T  request_ready_pkt;
    MPID_PKT_OK_TO_SEND_T    sendok_pkt;
#else
    MPID_PKT_LONG_T          long_pkt;
    MPID_PKT_LONG_SYNC_T     long_sync_pkt;
    MPID_PKT_LONG_READY_T    long_ready_pkt;
#endif
    MPID_PKT_SYNC_ACK_T      sync_ack_pkt;
    MPID_PKT_COMPLETE_SEND_T send_pkt;
    MPID_PKT_COMPLETE_RECV_T recv_pkt;
    MPID_PKT_READY_ERROR_T   error_pkt;
    } MPID_PKT_T;

#define MPID_PKT_HAS_XDR(pkt) (pkt)->head.has_xdr

/* Managing packets

   In a perfect world, there would always be a place for an incoming packet
   to be received.  In systems that work at the hardware level, this is
   often managed by having a separate pool for each possible source, and
   having each pair of processors keep track of how much space is being used.
   In the implementations of the ADI on top of existing message-passing 
   systems, we usually allow the underlying message-passing system to 
   manage flow control.  

   The message-passing equivalent of having an available buffer is to pre-post
   a non-blocking receive into which an incoming message can be placed.  The
   pros of this are that unnecessary data movement (from internal to ADI's 
   buffers) can be avoided, and that systems with interrupt-driven 
   receives (e.g., Intel, TMC, IBM) can repsond on an interrupt basis
   to incoming packets.

   The con-side to this is that doing an Irecv/Wait pair can be more
   expensive than a (blocking) Recv, and that interrupts can be expensive.
   Since we intend to do both a native shared-memory and active-message 
   version, and since there probably isn't a correct answer to question of
   which approach is best, we provide for both based on whether the macro
   MPID_PKT_PRE_POST is defined.

   An additional option is provided to allow the message packets to be
   preallocated.   This may be appropriate for p4, for example, where
   preallocating the message packets may eliminate a memory copy.

   Note also that the send packets need to be managed on some systems where
   blocking sends should not be used to dispatch the control information 
   for non-blocking operations.

   The operations are
   MPID_PKT_ALLOC() - Allocate the packets.  Either a single packet is
   used (non-pre-posted) or multiple packets (possibly two, to use double  
   buffering, for starters).

   MPID_PKT_INIT() - Initialize the packets .  Called during init portion.

   MPID_PKT_FREE() - Fress the allocated packets.  Note: if the packets
   are allocated on the calling-routines stack, this does nothing.
   
   MPID_PKT_CHECK() - Basically a check to see if a packet is available
 
   MPID_PKT_WAIT() - Waits for a packet to be available

   MPID_PKT_POST() - Post a non-blocking receive for the next packet.
   May be a nop in the case that packets are held by the underlying
   message-passing system

   MPID_PKT_POST_AND_WAIT() - Post and wait for a packet to be 
   available.  A blocking receive in both cases

 */
#if defined(MPID_PKT_PRE_POST)
/* Single buffer for now. Note that this alloc must EITHER be in the
   same routine as all of the calls OR in the same file .
   An alternative is to make these POINTERS to global values
   declared elsewhere 

   Note that because this file is fed into m4 to generate the native versions
   for non-Chameleon systems, it is REQUIRED that the Chameleon calls
   be on different lines from the #define's.  
 */
#define MPID_PKT_GALLOC \
    static MPID_PKT_T     pkt; \
    static int  pktid;
#define MPID_PKT_LALLOC 
#define MPID_PKT_INIT() MPID_PKT_POST()
#define MPID_PKT_CHECK()  \
    msgdone(&pktid)
#define MPID_PKT_WAIT() \
    msgwait(&pktid)
#define MPID_PKT_POST() \
    &pktid=_irecv(MPID_PT2PT_TAG,&pkt,sizeof(MPID_PKT_T))
#define MPID_PKT_POST_AND_WAIT() \
    _crecv(MPID_PT2PT_TAG,&pkt,sizeof(MPID_PKT_T))
#define MPID_PKT_FREE()
#define MPID_PKT (pkt)

#elif defined(MPID_PKT_PREALLOC)
/* Preallocate the buffer, but use blocking operations to access it.
   This is appropriate for p4 
   This is not ready yet since many pieces of code expect pkt to be
   statically and locally declared...
 */
#define MPID_PKT_GALLOC \
    static MPID_PKT_T     *pkt = 0; 
#define MPID_PKT_LALLOC 
#define MPID_PKT_INIT() \
    pkt = (MPID_PKT_T *)malloc(sizeof(MPID_PKT_T));
#define MPID_PKT_CHECK()  \
    iprobe(MPID_PT2PT_TAG)
#define MPID_PKT_WAIT() MPID_PKT_POST_AND_WAIT()
#define MPID_PKT_POST() 
#define MPID_PKT_POST_AND_WAIT() \
    _crecv(MPID_PT2PT_TAG,pkt,sizeof(MPID_PKT_T))
#define MPID_PKT_FREE() \
    free(pkt)
#define MPID_PKT (*pkt)

#else
/* Just use blocking send/recieve operations */
#define MPID_PKT_LALLOC MPID_PKT_T pkt;
#define MPID_PKT_GALLOC 
#define MPID_PKT_INIT()
#define MPID_PKT_CHECK()  \
    iprobe(MPID_PT2PT_TAG)
#define MPID_PKT_WAIT() MPID_PKT_POST_AND_WAIT()
#define MPID_PKT_POST() 
#define MPID_PKT_POST_AND_WAIT() \
    _crecv(MPID_PT2PT_TAG,&pkt,sizeof(MPID_PKT_T))
#define MPID_PKT_FREE()
#define MPID_PKT (pkt)
#endif

#endif
