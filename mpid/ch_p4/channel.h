#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_HAS_HETERO


/*
   Sending and receiving packets

   Packets are sent and received on connections.  In order to simultaneously
   provide a good fit with conventional message-passing systems and with 
   other more direct systems (e.g., sockets), I've defined a set of
   connection macros that are here translated into Chameleon message-passing
   calls.  These are somewhat complicated by the need to provide access to
   non-blocking operations

   These are not yet fully integrated into the code.  

   This file is designed for use with vendor message-passing systems through
   the Chameleon definitions.  Other systems should REPLACE this file.
   See mpid/ch_tcp and mpid/ch_shmem for examples.  Note also that once
   NewDevice creates a device, the local mpid.h is not modified, so that
   changes to packets.h can be accomplised by #defines

   In addition, we provide a simple way to log the "channel" operations
   If MPID_TRACE_FILE is set, we write information on the operation (both
   start and end) to the given file.  In order to simplify the code, we
   use the macro MPID_TRACE_CODE(name,channel).  Other implementations
   of channel.h are encourage to provide the trace calls; note that as macros,
   they can be completely removed at compile time for more 
   performance-critical systems.

 */
/* Do we need stdio here, or has it already been included? */

#define MPID_MyWorldRank \
    __MYPROCID
#define MPID_WorldSize \
    __NUMNODES

#define MPID_RecvAnyControl( pkt, size, from ) \
    { MPID_TRACE_CODE("BRecvAny",-1);\
      {char *__p4lbuf=0;__P4LEN=size;__P4FROM= -1;__P4TYPE=MPID_PT2PT_TAG;p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(pkt,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);}; *(from) = __P4FROM;\
      MPID_TRACE_CODE("ERecvAny",*(from));}
#define MPID_RecvFromChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BRecvFrom",channel);\
      {char *__p4lbuf=0;__P4LEN=size;__P4FROM= -1;__P4TYPE=MPID_PT2PT2_TAG(channel);p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(buf,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};\
      MPID_TRACE_CODE("ERecvFrom",channel);}
#define MPID_ControlMsgAvail( ) \
    (__P4TYPE=MPID_PT2PT_TAG,__P4FROM= -1,p4_messages_available(&__P4TYPE,&__P4FROM))
#define MPID_SendControl( pkt, size, channel ) \
    { MPID_TRACE_CODE("BSendControl",channel);\
      p4_send(MPID_PT2PT_TAG,channel,(char *)(pkt),size);\
      MPID_TRACE_CODE("ESendControl",channel);}
#if defined(MPID_USE_SEND_BLOCK) && ! defined(MPID_SendControlBlock)
/* 
   SendControlBlock allows the send to wait until the message is 
   received (but does NOT require it).  This can simplify some buffer 
   handling.
 */
#define MPID_SendControlBlock( pkt, size, channel ) \
    { MPID_TRACE_CODE("BSendControl",channel);\
      p4_send(MPID_PT2PT_TAG,channel,(char *)(pkt),size);\
      MPID_TRACE_CODE("ESendControl",channel);}
#endif

/* If we did not  SendControlBlock, make it the same as SendControl */
#if !defined(MPID_SendControlBlock)
#define MPID_SendControlBlock(pkt,size,channel) \
      MPID_SendControl(pkt,size,channel)
#endif

/* Because a common operation is to send a control block, and decide whether
   to use SendControl or SendControlBlock based on whether the send is 
   non-blocking, we include a definition for it here: 
 */
#ifdef MPID_USE_SEND_BLOCK
#define MPID_SENDCONTROL(mpid_send_handle,pkt,len,dest) \
if (mpid_send_handle->is_non_blocking) \
    MPID_SendControl( pkt, len, dest );\
else \
    MPID_SendControlBlock( pkt, len, dest );
#else
#define MPID_SENDCONTROL(mpid_send_handle,pkt,len,dest) \
MPID_SendControl( pkt, len, dest )
#endif

#define MPID_SendChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BSend",channel);\
      p4_send(MPID_PT2PT2_TAG(__MYPROCID),channel,(char *)(buf),size);\
      MPID_TRACE_CODE("ESend",channel);}

/* 
   Non-blocking versions (NOT required, but if PI_NO_NRECV and PI_NO_NSEND
   are NOT defined, they must be provided)
 */
#define MPID_IRecvFromChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BIRecvFrom",channel);\
     id =0;\
     MPID_TRACE_CODE("EIRecvFrom",channel);}
#define MPID_WRecvFromChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BWRecvFrom",channel);\
     {char *__p4lbuf=0;__P4LEN=size;__P4FROM= -1;__P4TYPE=MPID_PT2PT2_TAG(channel);p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(buf,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};\
     MPID_TRACE_CODE("EWRecvFrom",channel);}
#define MPID_RecvStatus( id ) \
    1

/* Note that these use the tag based on the SOURCE, not the channel
   See MPID_SendChannel */
#define MPID_ISendChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BISend",channel);\
     {id =0;p4_sendx(MPID_PT2PT2_TAG(__MYPROCID),channel,(char *)(buf),size,P4NOX);};\
     MPID_TRACE_CODE("EISend",channel);}
#define MPID_WSendChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BWSend",channel);\
    ;\
    MPID_TRACE_CODE("EWSend",channel);}
/* Test the channel operation */
#define MPID_TSendChannel( id ) \
    1

/*
   We also need an abstraction for out-of-band operations.  These could
   use transient channels or some other operation.  This is essentially for
   performing remote memory operations without local intervention; the need
   to determine completion of the operation requires some sort of handle.
   Here are the ones that we've chosen. Rather than call them transient 
   channels, we  "transfers", which are split operations.  Both 
   receivers and senders may create a transfer.

   Note that the message-passing version of this uses the 'ready-receiver'
   version of the operations.
 */
#define MPID_CreateSendTransfer( buf, size, partner, id ) {*(id) = 0;}
#define MPID_CreateRecvTransfer( buf, size, partner, id ) \
       {*(id) = CurTag++;TagsInUse++;}

#ifndef PI_NO_NRECV
#define MPID_StartRecvTransfer( buf, size, partner, id, rid ) \
    {MPID_TRACE_CODE("BIRRRecv",id);\
     rid =0;\
     MPID_TRACE_CODE("EIRRRecv",id);}
#define MPID_EndRecvTransfer( buf, size, partner, id, rid ) \
    {MPID_TRACE_CODE("BIWRRecv",id);\
     {char *__p4lbuf=0;__P4LEN=size;__P4FROM= -1;__P4TYPE=MPID_PT2PT2_TAG(id);p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(buf,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};\
     MPID_TRACE_CODE("EIWRRecv",id);\
     if (--TagsInUse == 0) CurTag = 1024; else if (id == CurTag-1) CurTag--;}
#define MPID_TestRecvTransfer( rid ) \
    1
#else
/* Put the tag value into rid so that we can probe it ... */
#define MPID_StartRecvTransfer( buf, size, partner, id, rid ) \
    {MPID_TRACE_CODE("BIRRRecv",id);\
     rid = MPID_PT2PT2_TAG(id);\
     MPID_TRACE_CODE("EIRRRecv",id);}
#define MPID_EndRecvTransfer( buf, size, partner, id, rid ) \
    {MPID_TRACE_CODE("BIWRRecv",id);\
     {char *__p4lbuf=0;__P4LEN=size;__P4FROM= -1;__P4TYPE=MPID_PT2PT2_TAG(id);p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(buf,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};\
     MPID_TRACE_CODE("EIWRRecv",id);\
     if (--TagsInUse == 0) CurTag = 1024; else if (id == CurTag-1) CurTag--;}
#define MPID_TestRecvTransfer( rid ) \
    (__P4TYPE=rid ,__P4FROM= -1,p4_messages_available(&__P4TYPE,&__P4FROM))
#endif

#ifdef PI_NO_NSEND
#define MPID_StartSendTransfer( buf, size, partner, id, sid ) \
    {MPID_TRACE_CODE("BIRRSend",id);\
     p4_sendx(MPID_PT2PT2_TAG(id),partner,(char *)(buf),size,P4NOX );\
     sid = 1;\
     MPID_TRACE_CODE("EIRRSend",id);}
#define MPID_EndSendTransfer( buf, size, partner, id, sid ) \
    {MPID_TRACE_CODE("BWRRSend",id);\
    MPID_TRACE_CODE("EWRRSend",id);}
#define MPID_TestSendTransfer( sid ) \
    1
#else
#define MPID_StartSendTransfer( buf, size, partner, id, sid ) \
    {MPID_TRACE_CODE("BIRRSend",id);\
     {sid =0;p4_sendx(MPID_PT2PT2_TAG(id),partner,(char *)(buf),size,P4NOX);};\
     MPID_TRACE_CODE("EIRRSend",id);}
#define MPID_EndSendTransfer( buf, size, partner, id, sid ) \
    {MPID_TRACE_CODE("BWRRSend",id);\
    ;\
    MPID_TRACE_CODE("EWRRSend",id);}
#define MPID_TestSendTransfer( sid ) \
    1
#endif

