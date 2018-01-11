


#define PI_NO_MSG_SEMANTICS
/*
   Sending and receiving packets

   Packets are sent and received on connections.  In order to simultaneously
   provide a good fit with conventional message-passing systems and with 
   other more direct systems (e.g., sockets), I've defined a set of
   connection macros that are here translated into Chameleon message-passing
   calls.  These are somewhat complicated by the need to provide access to
   non-blocking operations

   These are not yet fully integrated into the code.  

   Also note that the connection definitions are wrapped within
   a MPID_MSG_PASSING; this allows other systems to provide an
   alternative implementation at the connection level.

   In addition, we provide a simple way to log the "channel" operations
   If MPID_TRACE_FILE is set, we write information on the operation (both
   start and end) to the given file.  In order to simplify the code, we
   use the macro MPID_TRACE_CODE(name,channel)

 */
/* Do we need stdio here, or has it already been included? */

#define MPID_MSG_PASSING
#if defined(MPID_MSG_PASSING)
#define MPID_MyWorldRank \
    __MYPROCID
#define MPID_WorldSize \
    __NUMNODES

#define MPID_RecvAnyControl( pkt, size, from ) \
    { MPID_TRACE_CODE("BRecvAny",-1);\
      {__EUIFROM=-1;__EUITYPE=MPID_PT2PT_TAG;mpc_brecv(pkt,size,&__EUIFROM,&__EUITYPE,&__EUILEN);}; *(from) = __EUIFROM;\
      MPID_TRACE_CODE("ERecvAny",*(from));}
#define MPID_RecvFromChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BRecvFrom",channel);\
      {__EUIFROM=-1;__EUITYPE=MPID_PT2PT2_TAG(channel);mpc_brecv(buf,size,&__EUIFROM,&__EUITYPE,&__EUILEN);};\
      MPID_TRACE_CODE("ERecvFrom",channel);}
#define MPID_ControlMsgAvail( ) \
    (__EUIFROM=-1,__EUITYPE=MPID_PT2PT_TAG,mp_probe(&__EUIFROM,&__EUITYPE,&__EUILEN),(__EUILEN>=0))
#define MPID_SendControl( pkt, size, channel ) \
    { MPID_TRACE_CODE("BSendControl",channel);\
      mpc_bsend(pkt,size,channel,MPID_PT2PT_TAG);\
      MPID_TRACE_CODE("ESendControl",channel);}
#define MPID_SendChannel( buf, size, channel ) \
    { MPID_TRACE_CODE("BSend",channel);\
      mpc_bsend(buf,size,channel,MPID_PT2PT2_TAG(__MYPROCID));\
      MPID_TRACE_CODE("ESend",channel);}

/* 
   Non-blocking versions (NOT required, but if PI_NO_NRECV and PI_NO_NSEND
   are NOT defined, they must be provided)
 */
#define MPID_IRecvFromChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BIRecvFrom",channel);\
     {__EUIFROM=-1;__EUITYPE=MPID_PT2PT2_TAG(channel);mpc_recv(buf,size,&__EUIFROM,&__EUITYPE,&(id ));};\
     MPID_TRACE_CODE("EIRecvFrom",channel);}
#define MPID_WRecvFromChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BWRecvFrom",channel);\
     mp_wait(&(id ),&__EUILEN);;\
     MPID_TRACE_CODE("EWRecvFrom",channel);}
#define MPID_RecvStatus( id ) \
    (mp_status(&((id) )) > -1 )

/* Note that these use the tag based on the SOURCE, not the channel
   See MPID_SendChannel */
#define MPID_ISendChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BISend",channel);\
     mpc_send(buf,size,channel,MPID_PT2PT2_TAG(__MYPROCID),&(id ));\
     MPID_TRACE_CODE("EISend",channel);}
#define MPID_WSendChannel( buf, size, channel, id ) \
    {MPID_TRACE_CODE("BWSend",channel);\
    {int d; mp_wait(&(id ),&d);};\
    MPID_TRACE_CODE("EWSend",channel);}

/* These are the RR versions (channel is ready and buffer is prepared). 
   This kind of channel COULD use remote put/get by using the remote address
   as the 'channel' id */
#define MPID_IRRSendChannel( buf, size, channel, partner, id ) \
    {MPID_TRACE_CODE("BIRRSend",channel);\
     mpc_send(buf,size,partner,MPID_PT2PT2_TAG(channel),&(id ));\
     MPID_TRACE_CODE("EIRRSend",channel);}
#define MPID_WRRSendChannel( buf, size, channel, partner, id ) \
    {MPID_TRACE_CODE("BWRRSend",channel);\
    {int d; mp_wait(&(),&d);};\
    MPID_TRACE_CODE("EWRRSend",channel);}

/*
   The syntax of this WILL change
 */
#define MPID_NewChannel( partner, channel ) *(channel) = CurTag++
/*
   We also need an abstraction for out-of-band operations.  These could
   use transient channels or some other operation.  This is essentially for
   performing remote memory operations without local intervention; the need
   to determine completion of the operation requires some sort of handle.
 */
/* ... not designed yet ... */
#endif
