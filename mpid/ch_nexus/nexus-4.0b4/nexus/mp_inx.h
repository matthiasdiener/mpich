/*
 * mp_inx.h
 *
 * Intel Paragon INX protocol configuration for pr_mp.c
 *
 * Author: Tal Lancaster
 * 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/mp_inx.h,v 1.32 1996/10/07 04:40:01 tuecke Exp $"
 */

#define NEXUS_MP_PROTO_INX
#define NEXUS_PROTO_TYPE_MP NEXUS_PROTO_TYPE_INX
#define NEXUS_PROTO_NAME_MP "inx"
#define MP_PROTOCOL_INFO _nx_pr_inx_info

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

#ifndef BUILD_LITE
#define MP_PROTO_IS_THREAD_SAFE
#endif


/*
 * mp_destination_t
 *
 * This encapsulates the information about a node to which we
 * may send messages.
 */
typedef struct _mp_destination_t
{
    long node;
    long ptype;
} mp_destination_t;

/* Desribe contents of mp_destination_t for nexus_mi_proto_t manipulation */
#define MI_INT_COUNT 2
#define MI_STR_COUNT 1

typedef int mp_send_message_id_t;

#define INX_MSG_TYPE		0
#define INX_MSG_ACK_TYPE	1


/*
 * MP_DEFAULT_STORAGE_SIZE
 *
 * This is the minimum buffer size that can be used on sends and receives.
 */
#define MP_MIN_STORAGE_SIZE 16


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
#define MP_INIT_NODE_INFO(My_node, N_nodes) \
{ \
    (My_node).node = mynode(); \
    (My_node).ptype = myptype(); \
    N_nodes = numnodes(); \
}


/*
 * Stuff needed for message receives
 */
static long	inx_outstanding_msgid;
static char *	inx_outstanding_buffer = (char *) NULL;
static nexus_bool_t	inx_outstanding_is_default_storage = NEXUS_FALSE;
static int	inx_buffer_size;
static int	inx_pending_big_messages = 0;
static nexus_bool_t	inx_receive_posted = NEXUS_FALSE;

/*
 * MP_POST_RECEIVE(Func)
 *
 * Post the next receive if one has not already been posted.
 */
#define MP_POST_RECEIVE(Func) \
{ \
    int save_errno; \
\
    if (!inx_receive_posted) \
    { \
        if (!inx_outstanding_buffer) \
        { \
            if (inx_buffer_size > mp_default_storage_size) \
	    { \
	        NexusMalloc(Func, inx_outstanding_buffer, char *, \
			    inx_buffer_size); \
	        inx_outstanding_is_default_storage = NEXUS_FALSE; \
	    } \
	    else \
	    { \
	        GetMpDefaultStorage(Func, inx_outstanding_buffer); \
	        inx_outstanding_is_default_storage = NEXUS_TRUE; \
	    } \
        } \
        if ((inx_outstanding_msgid = _irecv(INX_MSG_TYPE, \
					    inx_outstanding_buffer, \
					    inx_buffer_size)) == -1) \
        { \
	    save_errno = errno; \
            nexus_fatal("MP_POST_RECEIVE(): _irecv() failed, errno=%d, %s\n", save_errno, _nx_md_system_error_string(save_errno)); \
        } \
        inx_receive_posted = NEXUS_TRUE; \
    } \
}


#ifdef NEXUS_USE_ACK_PROTOCOL
#define INX_ACK_GRAB_NODE() \
    int __other_node = *(((int *) inx_outstanding_buffer) + 2); \
    int __other_ptype = *(((int *) inx_outstanding_buffer) + 3)
#define INX_ACK_BIG_MESSAGE() \
{ \
    if (_nx_use_ack_protocol) \
    { \
	int __the_message[1]; \
	csend(INX_MSG_TYPE, \
	      (char *) __the_message, \
	      0, \
	      __other_node, \
	      __other_ptype); \
    } \
}
#ifdef DONT_INCLUDE
#define INX_ACK_BIG_MESSAGE() \
{ \
    if (_nx_use_ack_protocol) \
    { \
	int __the_message[1]; \
	csend(INX_MSG_ACK_TYPE, \
	      (char *) __the_message, \
	      0, \
	      __other_node, \
	      __other_ptype); \
    } \
}
#endif
#else
#define INX_ACK_GRAB_NODE()
#define INX_ACK_BIG_MESSAGE()
#endif

#define INX_HANDLE_BIG_MESSAGE(Func) \
{ \
    int __size = *(((int *) inx_outstanding_buffer) + 1); \
    INX_ACK_GRAB_NODE(); \
    inx_pending_big_messages++; \
    if (__size > inx_buffer_size) \
    { \
	if (inx_outstanding_is_default_storage) \
	{ \
	    FreeMpDefaultStorage(inx_outstanding_buffer); \
	} \
	else \
	{ \
	    NexusFree(inx_outstanding_buffer); \
	} \
	inx_outstanding_buffer = (char *) NULL; \
	inx_buffer_size = __size; \
    } \
    MP_POST_RECEIVE(Func); \
    INX_ACK_BIG_MESSAGE(); \
}

#define INX_HANDLE_MESSAGE(Func, Buf, Size) \
{ \
    GetMpBuffer(Func, Buf); \
    InitMpRecvBuffer(Buf); \
    (Buf)->size = (Size); \
    (Buf)->storage = inx_outstanding_buffer; \
    (Buf)->is_default_storage = inx_outstanding_is_default_storage; \
    (Buf)->current = (Buf)->storage + MP_EXTRA_HEADER_BYTES; \
    inx_outstanding_buffer = (char *) NULL; \
    if((Size) > mp_default_storage_size) \
    { \
	if(--inx_pending_big_messages == 0) \
	{ \
	    inx_buffer_size = mp_default_storage_size; \
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
    inx_buffer_size = mp_default_storage_size; \
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
    int save_errno; \
    MP_POST_RECEIVE(Func); \
    while(!__done) \
    { \
	if((__rc = _msgdone(inx_outstanding_msgid)) == 1) \
	{ \
	    int __size; \
	    inx_receive_posted = NEXUS_FALSE; \
	    __size = *((int *) inx_outstanding_buffer); \
	    if(__size == -1) \
	    { \
		INX_HANDLE_BIG_MESSAGE(Func); \
	    } \
	    else \
	    { \
		MP_START_CRITICAL_PATH_TIMER(); \
		INX_HANDLE_MESSAGE(Func, Buf, __size); \
		Status = NEXUS_TRUE; \
		__done = NEXUS_TRUE; \
	    } \
	} \
	else if(__rc == 0) \
	{ \
	    Status = NEXUS_FALSE; \
	    __done = NEXUS_TRUE; \
	} \
	else \
	{ \
	    save_errno = errno; \
	    nexus_fatal("MP_PROBE_AND_RECEIVE(): _msgdone() failed, errno=%d, %s.\n", save_errno, _nx_md_system_error_string(save_errno)); \
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
    int save_errno; \
    MP_POST_RECEIVE(Func); \
    while(!__done) \
    { \
	if(_msgwait(inx_outstanding_msgid) == 0) \
	{ \
	    int __size; \
	    if (Need_to_relock) \
	    { \
	        mp_enter(); \
	    } \
	    inx_receive_posted = NEXUS_FALSE; \
	    __size = *((int *) inx_outstanding_buffer); \
	    if(__size == -1) \
	    { \
		INX_HANDLE_BIG_MESSAGE(Func); \
	        if (Need_to_relock) \
	        { \
		    mp_exit(); \
	        } \
	    } \
	    else \
	    { \
		MP_START_CRITICAL_PATH_TIMER(); \
		INX_HANDLE_MESSAGE(Func, Buf, __size); \
		__done = NEXUS_TRUE; \
	        if (Need_to_relock) \
	        { \
		    Need_to_relock = NEXUS_FALSE; \
	        } \
	    } \
	} \
	else \
	{ \
	    save_errno = errno; \
	    nexus_fatal("MP_BLOCKING_RECEIVE(): _msgwait() failed, errno=%d, %s.\n", save_errno, _nx_md_system_error_string(save_errno)); \
	} \
    } \
}


#ifdef NEXUS_USE_ACK_PROTOCOL
#define INX_BIG_MESSAGE_SIZE 4
#define INX_BIG_MESSAGE_INFO() \
    __notify[2] = my_node.node; \
    __notify[3] = my_node.ptype
#define INX_WAIT_FOR_ACK(Buf) \
{ \
    if (_nx_use_ack_protocol) \
    { \
        MP_POST_RECEIVE(mp_send_remote_service_request); \
	_msgwait(inx_outstanding_msgid); \
	inx_receive_posted = NEXUS_FALSE; \
        MP_POST_RECEIVE(mp_send_remote_service_request); \
    } \
}
#ifdef DONT_INCLUDE
#define INX_WAIT_FOR_ACK(Buf) \
{ \
    if (_nx_use_ack_protocol) \
    { \
	int __the_message[1]; \
	crecv(INX_MSG_ACK_TYPE, (char *) __the_message, 0); \
    } \
}
#endif
#else
#define INX_BIG_MESSAGE_SIZE 2
#define INX_BIG_MESSAGE_INFO()
#define INX_WAIT_FOR_ACK(Buf)
#endif

/*
 * MP_SEND(mp_buffer_t *Buf)
 *
 * Send a message using the information in Buf.
 *
 * If the message size if >MP_DEFAULT_BUFFER_SIZE, first send a
 * small message with notification of the impending big message.
 */
#define MP_SEND(Buf) \
{ \
    int __err; \
    int save_errno; \
    if ((Buf)->size > mp_default_storage_size) \
    { \
	int __notify[INX_BIG_MESSAGE_SIZE]; \
	__notify[0] = -1; \
	__notify[1] = (Buf)->size; \
	INX_BIG_MESSAGE_INFO(); \
	__err = _csend(INX_MSG_TYPE, \
		       (char *) __notify, \
		       sizeof(__notify), \
		       (Buf)->u.send.proto->destination.node, \
		       (Buf)->u.send.proto->destination.ptype); \
	if (__err == -1) \
	{ \
	    save_errno = errno; \
	    nexus_fatal("MP_SEND(): first _csend() error, errno=%d, %s\n", save_errno, _nx_md_system_error_string(save_errno)); \
	} \
	INX_WAIT_FOR_ACK(Buf); \
    } \
    *((int *)(Buf)->storage) = (Buf)->size; \
    __err = _csend(INX_MSG_TYPE, \
		   (Buf)->storage, \
		   (Buf)->size, \
		   (Buf)->u.send.proto->destination.node, \
		   (Buf)->u.send.proto->destination.ptype); \
    if (__err == -1) \
    { \
	save_errno = errno; \
        nexus_fatal("MP_SEND(): second _csend() error, errno=%d, %s\n", save_errno, _nx_md_system_error_string(save_errno)); \
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
    Status = NEXUS_TRUE; \
}


/*
 * MP_COMPARE_DESTINATIONS(mp_destination_t D1, mp_destination_t D2,
 *			   nexus_bool_t Result)
 *
 * Set Result==NEXUS_TRUE if mp_destination_t's D1 and D2 are the same.
 */
#define MP_COMPARE_DESTINATIONS(D1, D2, Result) \
    if (   ((D1).node == (D2).node) \
	&& ((D1).ptype == (D2).ptype) ) \
        Result = NEXUS_TRUE; \
    else \
        Result = NEXUS_FALSE; 



/*
 * MP_COPY_DESTINATION(mp_destination_t To, mp_destination_t From)
 *
 * Copy mp_destination_t 'From' to 'To'.
 */
#define MP_COPY_DESTINATION(To, From) \
{ \
    (To).node = (From).node; \
    (To).ptype = (From).ptype; \
}


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
 
 * What about collisions? will need to look at ptype -- tal 7/15/94
 */
#define MP_HASH_DESTINATION(Dest, Hash) \
    (Hash) = (((Dest).node + (Dest).ptype) % PROTO_TABLE_SIZE)


/*
 * MP_INIT_DESTINATION(mp_destination_t Dest)
 *
 * Initialize the destination.
 */
#define MP_INIT_DESTINATION(Dest) \
{ \
    (Dest).node =  -1; \
    (Dest).ptype=  -1; \
}


/*
 * MP_WAKEUP_HANDLER()
 *
 * Send a message to myself to as to wakeup my blocked handler thread.
 */
#define MP_WAKEUP_HANDLER() \
{ \
    int __msg[2]; \
    int save_errno; \
    __msg[0] = sizeof(__msg); \
    __msg[1] = CLOSE_NORMAL_FLAG; \
    if(_csend (0, (char *) __msg, sizeof(__msg), \
		mynode(), myptype()) == -1) \
    { \
	save_errno = errno; \
        nexus_fatal("MP_WAKEUP_HANDLER(): _csend() error, errno=%d, %s\n", save_errno, _nx_md_system_error_string(save_errno)); \
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
    /* Kill everything */ \
    kill (0, SIGKILL); \
}


/*
 * MP_GET_MY_MI_PROTO_SIZE(int Size)
 *
 * Fillin Size with the number of bytes I need to store my_node
 * into the mi_proto byte array.
 */
#define MP_GET_MY_MI_PROTO_SIZE(Size) (Size) = (sizeof(long) * 2)


/*
 * MP_GET_MY_MI_PROTO(nexus_byte_t *Array)
 *
 * Fillin my_node into the mi_proto byte array
 */
#define MP_GET_MY_MI_PROTO(Array) \
{ \
    memcpy((Array), &(my_node.node), sizeof(long)); \
    memcpy((Array) + sizeof(long), &(my_node.ptype), sizeof(long)); \
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
    memcpy(&((Dest).node), (Array), sizeof(long)); \
    memcpy(&((Dest).ptype), (Array) + sizeof(long), sizeof(long)); \
}


/* 
 * MP_BROADCAST_COMMAND(int Command)
 *
 * Send the command out to everybody.
 *
 * NOTE: This is hacked!!! 7/25/94 tal
 * Will need some way of broadcasting to all known ptypes.
 */
#define MP_BROADCAST_COMMAND(Command) \
{ \
    int __msg[2]; \
    int save_errno; \
    __msg[0] = sizeof(__msg); \
    __msg[1] = (Command); \
    if (_csend (INX_MSG_TYPE, (char *) __msg, sizeof(__msg), -1, 0) == -1) \
    { \
	save_errno = errno; \
	nexus_fatal("MP_BROADCAST_COMMAND(): _csend() error, errno=%d, %s\n", save_errno, _nx_md_system_error_string(save_errno)); \
    } \
}	

#if 0
static void clean_quit (int s)
{
    /* May want to have a test to see which interrrupt was used in
    case this function is shared. */
    /*
    nexus_debug_printf(3, ("Exiting due to interrupt (signal %d)\n", s));
    */
    _nx_nodelock_cleanup();
    _nx_md_exit(0);
}
#endif

extern void _nx_clean_quit (int);

/*
 * PARAGON_SHUTDOWN
 * Macro to terminate all nodes.  This should only be called in 
 * mp_shutdown().
 */
#define PARAGON_SHUTDOWN \
{ \
	nexus_debug_printf (2, ("shutdown_others node %d.\n", mynode())); \
	if (nexus_master_node()) { \
		int mask; \
		mask = sigmask (SIGUSR1); \
		sigblock (mask); \
		(*signal(SIGUSR1, _nx_clean_quit)) (SIGUSR1); \
		kill (0, SIGUSR1); \
	} \
}
