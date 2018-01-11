/*
 * mp_mpl2.h
 *
 * IBM SP2 MPL protocol configuration for pr_mp.c
 * Post a receive ahead of time with a default buffer size, and
 * split large messages into a header and a body.
 * 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/mp_mpl2.h,v 1.19 1996/10/07 04:40:02 tuecke Exp $"
 */

#include "mpproto.h"

#define NEXUS_MP_PROTO_MPL
#define NEXUS_PROTO_TYPE_MP NEXUS_PROTO_TYPE_MPL
#define NEXUS_PROTO_NAME_MP "mpl"
#define MP_PROTOCOL_INFO _nx_pr_mpl_info

/*
 * MP_PROTO_IS_THREAD_SAFE
 *
 * It is advantageous if an MP module be thread safe, since this
 * allows you to simply create a handler thread which blocks on a
 * receive, and automatically wakes up and handles the message one
 * one arrives.
 *
 * In order for an MP module to be considered "thread safe", the
 * following must be true:
 *   1) One thread can be doing a blocking receive, while other threads
 *	are simultaneously doing sends.
 *   2) A thread can send a message to its own node, which will
 *	be received by the blocking receive in the handler thread.
 *
 * If these conditions are true, then #define MP_PROTO_IS_THREAD_SAFE
 */
/*
#define MP_PROTO_IS_THREAD_SAFE
*/


/*
 * mp_destination_t
 *
 * This encapsulates the information about a node to which we
 * may send messages.
 */
typedef int mp_destination_t;

/* Desribe contents of mp_destination_t for nexus_mi_proto_t manipulation */
#define MI_INT_COUNT 1
#define MI_STR_COUNT 0

typedef int mp_send_message_id_t;

#define MPL_MSG_TYPE 42


/*
 * MP_DEFAULT_STORAGE_SIZE
 *
 * This is the minimum buffer size that can be used on sends and receives.
 */
#define MP_MIN_STORAGE_SIZE 12


/*
 * MP_DEFAULT_STORAGE_SIZE
 *
 * This is the default buffer size that will be used on sends and
 * receives to avoid extraneous mallocs and locks.
 */
#ifdef NEXUS_USE_ACK_PROTOCOL
#define MP_DEFAULT_STORAGE_SIZE MP_MIN_STORAGE_SIZE
#else
#define MP_DEFAULT_STORAGE_SIZE 32736
#endif


/*
 * MP_EXTRA_HEADER_BYTES
 *
 * If the message passing module needs extra bytes at the beginning
 * of messages, then set this to the number of bytes needed.
 */
#define MP_EXTRA_HEADER_BYTES sizeof(int)


/*
 * MP_INIT_NODE_INFO(mp_destination_t Mynode, int N_nodes)
 *
 * Set Mynode to the mp_destionation_t info for my node.
 * Set N_nodes the the number of nodes.
 */
#ifdef NEXUS_USE_FORWARDER
#define MP_INIT_NODE_INFO(My_node, N_nodes) \
{ \
    mpc_environ(&(N_nodes), &(My_node)); \
    (N_nodes)--; \
}
#else  /* NEXUS_USE_FORWARDER */
#define MP_INIT_NODE_INFO(My_node, N_nodes) \
{ \
    mpc_environ(&(N_nodes), &(My_node)); \
}
#endif /* NEXUS_USE_FORWARDER */


/*
 * Stuff needed for message receives
 */
static int		mpl_outstanding_msgid;
static char *		mpl_outstanding_buffer = (char *) NULL;
static nexus_bool_t	mpl_outstanding_is_default_storage = NEXUS_FALSE;
static int		mpl_buffer_size;
static int		mpl_pending_big_messages = 0;
static nexus_bool_t	mpl_receive_posted = NEXUS_FALSE;


/*
 * MP_POST_RECEIVE(Func)
 *
 * Post the next receive if one has not already been posted.
 */
static int mpl_post_receive_source;
static int mpl_post_receive_type;
#define MP_POST_RECEIVE(Func) \
{ \
    if (!mpl_receive_posted) \
    { \
	mpl_post_receive_source = DONTCARE; \
	mpl_post_receive_type = MPL_MSG_TYPE; \
        if (!mpl_outstanding_buffer) \
        { \
            if (mpl_buffer_size > mp_default_storage_size) \
	    { \
	        NexusMalloc(Func, mpl_outstanding_buffer, char *, \
			    mpl_buffer_size); \
	        mpl_outstanding_is_default_storage = NEXUS_FALSE; \
	    } \
	    else \
	    { \
	        GetMpDefaultStorage(Func, mpl_outstanding_buffer); \
	        mpl_outstanding_is_default_storage = NEXUS_TRUE; \
	    } \
        } \
        if (mpc_recv(mpl_outstanding_buffer, \
		     mpl_buffer_size, \
		     &mpl_post_receive_source, \
		     &mpl_post_receive_type, \
		     &mpl_outstanding_msgid) != 0) \
        { \
	    nexus_fatal("MP_POST_RECEIVE(): mpc_recv() failed, mperrno=%d\n", mperrno); \
        } \
        mpl_receive_posted = NEXUS_TRUE; \
    } \
}


#ifdef NEXUS_USE_ACK_PROTOCOL
#define MPL_ACK_GRAB_NODE() \
    int __other_node = *(((int *) mpl_outstanding_buffer) + 2)
#define MPL_ACK_BIG_MESSAGE() \
{ \
    if (_nx_use_ack_protocol) \
    { \
	int __the_message[1]; \
	mpc_bsend((char *) __the_message /* can't use NULL here! -SJT */, \
		  0, \
		  __other_node, \
		  MPL_MSG_TYPE); \
    } \
}
#else
#define MPL_ACK_GRAB_NODE()
#define MPL_ACK_BIG_MESSAGE()
#endif

#define MPL_HANDLE_BIG_MESSAGE(Func) \
{ \
    int __size = *(((int *) mpl_outstanding_buffer) + 1); \
    MPL_ACK_GRAB_NODE(); \
    mpl_pending_big_messages++; \
    if (__size > mpl_buffer_size) \
    { \
	if (mpl_outstanding_is_default_storage) \
	{ \
	    FreeMpDefaultStorage(mpl_outstanding_buffer); \
	} \
	else \
	{ \
	    NexusFree(mpl_outstanding_buffer); \
	} \
	mpl_outstanding_buffer = (char *) NULL; \
	mpl_buffer_size = __size; \
    } \
    MP_POST_RECEIVE(Func); \
    MPL_ACK_BIG_MESSAGE(); \
}

#define MPL_HANDLE_MESSAGE(Func, Buf, Size) \
{ \
    GetMpBuffer(Func, Buf); \
    InitMpRecvBuffer(Buf); \
    (Buf)->size = (Size); \
    (Buf)->storage = mpl_outstanding_buffer; \
    (Buf)->is_default_storage = mpl_outstanding_is_default_storage; \
    (Buf)->current = (Buf)->storage + MP_EXTRA_HEADER_BYTES; \
    mpl_outstanding_buffer = (char *) NULL; \
    if((Size) > mp_default_storage_size) \
    { \
	if(--mpl_pending_big_messages == 0) \
	{ \
	    mpl_buffer_size = mp_default_storage_size; \
	} \
    } \
    /* MP_POST_RECEIVE(Func); Leaving this in will cause the next receive to be posted before this message is handled. */ \
}


/*
 * MP_RECEIVE_INIT(Func)
 *
 * Do any receive initialization.
 */
#define MP_RECEIVE_INIT(Func) \
{ \
    mpl_buffer_size = mp_default_storage_size; \
    MP_POST_RECEIVE(Func); \
}


/*
 * MP_PROBE_AND_RECEIVE(Func, mp_buffer_t *Buf, nexus_bool_t Status)
 *
 * See if there are any messages available, and if so then
 * receive one into Buf.
 *
 * Set Status to NEXUS_TRUE if a message is received, NEXUS_FALSE otherwise.
 */
#define MP_PROBE_AND_RECEIVE(Func, Buf, Status) \
{ \
    nexus_bool_t __done = NEXUS_FALSE; \
    int __rc; \
    MP_POST_RECEIVE(Func); \
    while(!__done) \
    { \
	if((__rc = mpc_status(mpl_outstanding_msgid)) >= 0) \
	{ \
	    int __size; \
	    mpl_receive_posted = NEXUS_FALSE; \
	    __size = *((int *) mpl_outstanding_buffer); \
	    if(__size == -1) \
	    { \
		MPL_HANDLE_BIG_MESSAGE(Func); \
	    } \
	    else \
	    { \
		MP_START_CRITICAL_PATH_TIMER(); \
		MPL_HANDLE_MESSAGE(Func, Buf, __size); \
		Status = NEXUS_TRUE; \
		__done = NEXUS_TRUE; \
	    } \
	} \
	else if(__rc == -1) \
	{ \
	    Status = NEXUS_FALSE; \
	    __done = NEXUS_TRUE; \
	} \
	else \
	{ \
	    nexus_fatal("MP_PROBE_AND_RECEIVE(): mpc_status() failed, mperrno=%d\n", mperrno); \
	} \
    } \
}


/*
 * MP_BLOCKING_RECEIVE(Func, mp_buffer_t *Buf, nexus_bool_t Need_to_relock)
 *
 * Blocking receive of one message into Buf.
 */
#define MP_BLOCKING_RECEIVE(Func, Buf, Need_to_relock) \
{ \
    nexus_bool_t __done = NEXUS_FALSE; \
    size_t __n_bytes; \
    MP_POST_RECEIVE(Func); \
    while(!__done) \
    { \
	if(mpc_wait(&mpl_outstanding_msgid, &__n_bytes) == 0) \
	{ \
	    int __size; \
	    if (Need_to_relock) \
	    { \
	        mp_enter(); \
	    } \
	    mpl_receive_posted = NEXUS_FALSE; \
	    __size = *((int *) mpl_outstanding_buffer); \
	    if(__size == -1) \
	    { \
		MPL_HANDLE_BIG_MESSAGE(Func); \
	        if (Need_to_relock) \
	        { \
		    mp_exit(); \
	        } \
	    } \
	    else \
	    { \
		MP_START_CRITICAL_PATH_TIMER(); \
		MPL_HANDLE_MESSAGE(Func, Buf, __size); \
		__done = NEXUS_TRUE; \
	        if (Need_to_relock) \
	        { \
		    Need_to_relock = NEXUS_FALSE; \
	        } \
	    } \
	} \
	else \
	{ \
	    nexus_fatal("MP_BLOCKING_RECEIVE(): mpc_wait() failed, mperrno=%d\n", mperrno); \
	} \
    } \
}


#ifdef NEXUS_USE_ACK_PROTOCOL
#define MPL_BIG_MESSAGE_SIZE 3
#define MPL_BIG_MESSAGE_INFO() __notify[2] = my_node
#define MPL_WAIT_FOR_ACK(Buf) \
{ \
    if (_nx_use_ack_protocol) \
    { \
	size_t __n_bytes; \
        MP_POST_RECEIVE(mp_send_remote_service_request); \
	mpc_wait(&mpl_outstanding_msgid, &__n_bytes); \
	mpl_receive_posted = NEXUS_FALSE; \
        MP_POST_RECEIVE(mp_send_remote_service_request); \
    } \
}
#else
#define MPL_BIG_MESSAGE_SIZE 2
#define MPL_BIG_MESSAGE_INFO()
#define MPL_WAIT_FOR_ACK(Buf)
#endif

/*
 * MP_SEND(mp_buffer_t *Buf)
 *
 * Send a message using the information in Buf.
 */
#define MP_SEND(Buf) \
{ \
    int __err; \
    if ((Buf)->size > mp_default_storage_size) \
    { \
	int __notify[MPL_BIG_MESSAGE_SIZE]; \
	__notify[0] = -1; \
	__notify[1] = (Buf)->size; \
	MPL_BIG_MESSAGE_INFO(); \
	__err = mpc_bsend((char *) __notify, \
			  sizeof(__notify), \
			  ((Buf)->u.send.proto->destination), \
			  MPL_MSG_TYPE); \
	if (__err != 0) \
	{ \
	    nexus_fatal("MP_SEND(): first mpc_send() error, mperrno=%d\n", mperrno); \
	} \
	MPL_WAIT_FOR_ACK(Buf); \
    } \
    *((int *)(Buf)->storage) = (Buf)->size; \
    __err = mpc_send((Buf)->storage, \
		     ((Buf)->size), \
		     ((Buf)->u.send.proto->destination), \
		     MPL_MSG_TYPE, \
		     &((Buf)->u.send.message_id)); \
    if (__err != 0) \
    { \
        nexus_fatal("MP_SEND(): second mpc_send() error, errno=%d\n", mperrno); \
    } \
}


/*
 * MP_SEND_STATUS(mp_buffer_t *Buf, nexus_bool_t Status)
 *
 * Set Status to NEXUS_TRUE if the send in MP_SEND(Buf) has completed,
 * otherwise set it to NEXUS_FALSE.  This is used when non-blocking
 * sends are used. If blocking sends are used, then always
 * return NEXUS_TRUE.
 */
#define MP_SEND_STATUS(Buf, Status) \
{ \
    if (mpc_status(((Buf)->u.send.message_id)) >= 0) \
	Status = NEXUS_TRUE; \
    else \
	Status = NEXUS_FALSE; \
}


#ifdef NEXUS_USE_FORWARDER
/*
 * MP_SEND_BYTES(int Dest_node, nexus_byte_t *Buf, int Size,
 *               mp_send_message_id_t Message_id)
 *
 * Send a message using the passed bytes.
 */
#define MP_SEND_BYTES(Dest_node, Buf, Size, Message_id) \
{ \
    int __err; \
    if ((Size) > mp_default_storage_size) \
    { \
	int __notify[MPL_BIG_MESSAGE_SIZE]; \
	__notify[0] = -1; \
	__notify[1] = (Size); \
	MPL_BIG_MESSAGE_INFO(); \
	__err = mpc_bsend((char *) __notify, \
			  sizeof(__notify), \
			  (Dest_node), \
			  MPL_MSG_TYPE); \
	if (__err != 0) \
	{ \
	    nexus_fatal("MP_SEND(): first mpc_send() error, mperrno=%d\n", mperrno); \
	} \
	MPL_WAIT_FOR_ACK(NULL); \
    } \
    __err = mpc_send((Buf), \
		     (Size), \
		     (Dest_node), \
		     MPL_MSG_TYPE, \
		     &(Message_id)); \
    if (__err != 0) \
    { \
        nexus_fatal("MP_SEND(): second mpc_send() error, errno=%d\n", mperrno); \
    } \
}


/*
 * MP_SEND_BYTES_STATUS(mp_send_message_id_t Message_id, nexus_bool_t Status)
 *
 * Set Status to NEXUS_TRUE if the send in MP_SEND(Buf) has completed,
 * otherwise set it to NEXUS_FALSE.  This is used when non-blocking
 * sends are used. If blocking sends are used, then always
 * return NEXUS_TRUE.
 */
#define MP_SEND_BYTES_STATUS(Message_id, Status) \
{ \
    if (mpc_status((Message_id)) >= 0) \
	Status = NEXUS_TRUE; \
    else \
	Status = NEXUS_FALSE; \
}
#endif /* NEXUS_USE_FORWARDER */


/*
 * MP_COMPARE_DESTINATIONS(mp_destination_t D1, mp_destination_t D2,
 *			  nexus_bool_t Result)
 *
 * Set Result==NEXUS_TRUE if mp_destination_t's D1 and D2 are the same.
 */
#define MP_COMPARE_DESTINATIONS(D1, D2, Result) \
    Result = (((D1) == (D2)) ? NEXUS_TRUE : NEXUS_FALSE);


/*
 * MP_COPY_DESTINATION(mp_destination_t To, mp_destination_t From)
 *
 * Copy mp_destination_t 'From' to 'To'.
 */
#define MP_COPY_DESTINATION(To, From) \
    (To) = (From)


/*
 * MP_FREE_DESTINATION(mp_destination_t Dest)
 *
 * Free up any memory in 'Dest' which was malloced.
 */
#define MP_FREE_DESTINATION(Dest)


/*
 * MP_HASH_DESTINATION(mp_destination_t Dest, int Hash)
 *
 * Hash the destination information into a value
 * between 0 and PROTO_TABLE_SIZE.
 */
#define MP_HASH_DESTINATION(Dest, Hash) \
    (Hash) = ((Dest) % PROTO_TABLE_SIZE)


/*
 * MP_INIT_DESTINATION(mp_destination_t Dest)
 *
 * Initialize the destination.
 */
#define MP_INIT_DESTINATION(Dest)


/*
 * MP_WAKEUP_HANDLER()
 *
 * Send a message to myself to as to wakeup my blocked handler thread.
 */
#define MP_WAKEUP_HANDLER() \
{ \
    int __msg[2]; \
    __msg[0] = sizeof(__msg); \
    __msg[1] = CLOSE_NORMAL_FLAG; \
    if(mpc_bsend((char *) __msg, sizeof(__msg), my_node, MPL_MSG_TYPE) == -1) \
    { \
        nexus_fatal("MP_WAKEUP_HANDLER(): mpc_bsend() error, mperrno=%d\n", mperrno); \
    } \
}


/*
 * MP_NODE_EXIT()
 *
 * If a terminating process needs to do something to map itself out
 * of the set of processes, this macro should do it.
 */
#define MP_NODE_EXIT()


/*
 * MP_ABORT()
 *
 * Special statements to abort all processes.
 */
#define MP_ABORT() \
{ \
    mpc_stopall(1); \
}


/*
 * MP_GET_MY_MI_PROTO_SIZE(int Size)
 *
 * Fillin Size with the number of bytes I need to store my_node
 * into the mi_proto byte array.
 */
#define MP_GET_MY_MI_PROTO_SIZE(Size) (Size) = sizeof(int)


/*
 * MP_GET_MY_MI_PROTO(nexus_byte_t *Array)
 *
 * Fillin my_node into the mi_proto byte array
 */
#define MP_GET_MY_MI_PROTO(Array) \
{ \
    memcpy((Array), &my_node, sizeof(int)); \
}


/* 
 * MP_CONSTRUCT_FROM_MI_PROTO(mp_destination_t Dest,
 *			      nexus_mi_proto_t *Mi_proto,
 *			      nexus_byte_t *   Array)
 *
 * Copy the needed elements from Array to Dest.
 */
#define MP_CONSTRUCT_FROM_MI_PROTO(Dest, Mi_proto, Array) \
{ \
    memcpy(&(Dest), (Array), sizeof(int)); \
}


/* 
 * MP_BROADCAST_COMMAND(int Command)
 *
 * Send the command out to everybody.
 */
#define MP_BROADCAST_COMMAND(Command) \
{ \
    int i; \
    nexus_bool_t send_done; \
    int message_id; \
    int __msg[2]; \
    __msg[0] = sizeof(__msg); \
    __msg[1] = (Command); \
    for (i = 0; i < n_nodes; i++) \
    { \
	if (i != my_node) \
	{ \
	    mpc_send((char *) __msg, \
		     sizeof(__msg), \
		     i, \
		     MPL_MSG_TYPE, \
		     &message_id); \
	    if (mpc_status(message_id) >= 0) \
		send_done = NEXUS_TRUE; \
	    else \
		send_done = NEXUS_FALSE; \
	    while (!send_done) \
	    { \
		if (!using_handler_thread) \
		{ \
		    receive_messages(NON_BLOCKING, ENQUEUE_MESSAGES); \
		} \
		if (mpc_status(message_id) >= 0) \
		    send_done = NEXUS_TRUE; \
		else \
		    send_done = NEXUS_FALSE; \
	    } \
	} \
    } \
}
