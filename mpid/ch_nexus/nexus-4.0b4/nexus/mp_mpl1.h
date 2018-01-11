/*
 * mp_mpl1.h
 *
 * IBM SP2 MPL protocol configuration for pr_mp.c
 * Use mpc_probe to check for messages.
 * 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/mp_mpl1.h,v 1.13 1996/10/07 04:40:02 tuecke Exp $"
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
#define MP_DEFAULT_STORAGE_SIZE 32736


/*
 * MP_EXTRA_HEADER_BYTES
 *
 * If the message passing module needs extra bytes at the beginning
 * of messages, then set this to the number of bytes needed.
 */
#define MP_EXTRA_HEADER_BYTES 0


/*
 * MP_INIT_NODE_INFO(mp_destination_t Mynode, int N_nodes)
 *
 * Set Mynode to the mp_destionation_t info for my node.
 * Set N_nodes the the number of nodes.
 */
#define MP_INIT_NODE_INFO(My_node, N_nodes) \
{ \
    mpc_environ(&(N_nodes), &(My_node)); \
}


/*
 * MP_SEND(mp_buffer_t *Buf)
 *
 * Send a message using the information in Buf.
 */
#define MP_SEND(Buf) \
{ \
    int __type = MPL_MSG_TYPE; \
    mpc_send((Buf)->storage, \
	     ((Buf)->size), \
	     ((Buf)->u.send.proto->destination), \
	     __type, \
	     &((Buf)->u.send.message_id)); \
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


/*
 * MP_RECEIVE_INIT(Func)
 *
 * Do any receive initialization.
 */
#define MP_RECEIVE_INIT(Func)


/*
 * MP_POST_RECEIVE(Func)
 *
 * Post the next receive if one has not already been posted.
 */
#define MP_POST_RECEIVE(Func)


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
    int __source = DONTCARE; \
    int __type = MPL_MSG_TYPE; \
    int __n_bytes; \
    size_t __n_bytes_out; \
    mpc_probe(&__source, &__type, &__n_bytes); \
    if (__n_bytes >= 0) \
    { \
	GetMpBuffer(Func, Buf); \
	InitMpRecvBuffer(Buf); \
	(Buf)->size = __n_bytes; \
	NexusMalloc(Func, (Buf)->storage, char *, __n_bytes); \
	if (mpc_brecv((Buf)->storage, \
		      (size_t) __n_bytes, \
		      &__source, \
		      &__type, \
		      &__n_bytes_out) == 0) \
	{ \
	    (Buf)->current = (Buf)->storage; \
	    Status = NEXUS_TRUE; \
	} \
	else \
	{ \
	    nexus_fatal("MP_PROBE_AND_RECEIVE(): mp_probe() succeeded but mp_recv() failed\n"); \
	} \
    } \
    else \
    { \
	Status = NEXUS_FALSE; \
    } \
}


/*
 * MP_BLOCKING_RECEIVE(Func, mp_buffer_t *Buf, nexus_bool_t Need_to_relock)
 *
 * Blocking receive of one message into Buf.
 */
#define MP_BLOCKING_RECEIVE(Func, Buf, Need_to_relock) \
{ \
    int __source; \
    int __type; \
    int __n_bytes = -1; \
    size_t __n_bytes_out; \
    while (__n_bytes < 0) \
    { \
	__source = DONTCARE; \
	__type = MPL_MSG_TYPE; \
	mpc_probe(&__source, &__type, &__n_bytes); \
    } \
    if (Need_to_relock) \
    { \
        mp_enter(); \
	Need_to_relock = NEXUS_FALSE; \
    } \
    GetMpBuffer(Func, Buf); \
    InitMpRecvBuffer(Buf); \
    (Buf)->size = __n_bytes; \
    NexusMalloc(Func, (Buf)->storage, char *, __n_bytes); \
    if (mpc_brecv((Buf)->storage, \
		  (size_t) __n_bytes, \
		  &__source, \
		  &__type, \
		  &__n_bytes_out) == 0) \
    { \
        (Buf)->current = (Buf)->storage; \
    } \
    else \
    { \
        nexus_fatal("MP_PROBE_AND_RECEIVE(): mp_probe() succeeded but mp_recv() failed\n"); \
    } \
}


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
#define MP_WAKEUP_HANDLER()


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
