/*
 * Nexus
 * Authors:     Steven Tuecke
 *              Argonne National Laboratory
 *
 * pr_mp.c	- Protocol module for message passing systems, including:
 *			- MPL on the IBM SP1
 *      		- INX on the Intel Paragon
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_mp.c,v 1.34 1997/02/23 21:50:49 tuecke Exp $";

#include "internal.h"

#ifdef NEXUS_CRITICAL_PATH_TIMER
#include "perf/utp/UTP.h"
nexus_bool_t _nx_time_critical_path = NEXUS_FALSE;
nexus_bool_t _nx_critical_path_timer_started = NEXUS_FALSE;
int _nx_critical_path_start_timer = 0;
int _nx_critical_path_stop_timer = 0;
#endif

#ifdef NEXUS_USE_ACK_PROTOCOL
nexus_bool_t _nx_use_ack_protocol = NEXUS_FALSE;
#endif

/*
 * Hash table size for the proto table
 */
#define PROTO_TABLE_SIZE 1021

/*
 * Thread is handler?
 *
 * Thread specific storage is used to keep track if the current
 * thread is a handler thread or not.
 */
#ifdef BUILD_LITE
#define _nx_set_i_am_mp_handler_thread() /* nop */
#define _nx_i_am_mp_handler_thread(Result) *(Result) = NEXUS_TRUE
#else
static nexus_thread_key_t i_am_mp_handler_thread_key;
#define _nx_set_i_am_mp_handler_thread() \
    nexus_thread_setspecific(i_am_mp_handler_thread_key, (void *) 1)
#define _nx_i_am_mp_handler_thread(Result) \
    *(Result) = (nexus_bool_t)nexus_thread_getspecific(i_am_mp_handler_thread_key)
#endif /* BUILD_LITE */


/*
 * Only one thread is allowed to be in the mp code (and thus
 * mucking with data structures) at a time.
 */
static nexus_mutex_t		mp_mutex;
static nexus_cond_t		mp_cond;
static nexus_bool_t		mp_done;
static nexus_bool_t		handle_in_progress;
static nexus_bool_t		send_in_progress;
static int			sends_waiting;
static nexus_cond_t		sends_waiting_cond;
static nexus_bool_t		using_handler_thread;
static nexus_bool_t		handler_thread_done;
static nexus_mutex_t		handler_thread_done_mutex;
static nexus_cond_t		handler_thread_done_cond;

#define mp_enter() nexus_mutex_lock(&mp_mutex);
#define mp_exit()  nexus_mutex_unlock(&mp_mutex);
#define mp_send_wait() \
{ \
    sends_waiting++; \
    nexus_cond_wait(&sends_waiting_cond, &mp_mutex); \
    sends_waiting--; \
}
#define mp_send_signal() \
{ \
    if (sends_waiting) \
    { \
	nexus_cond_signal(&sends_waiting_cond); \
    } \
}

#define mp_fatal mp_exit(); nexus_fatal


/*
 * The mi_proto for this protocol module carries a unique session
 * string with it.  This allows one MPP to distinguish itself
 * from another MPP of the same type.
 */
char *	_nx_session_string;
int	_nx_session_string_length;
    

/*
 * Other useful defines
 */
#define HANDLE_MESSAGES			1
#define ENQUEUE_MESSAGES		2

#define BLOCKING			NEXUS_TRUE
#define NON_BLOCKING			NEXUS_FALSE

#define CLOSE_NORMAL_FLAG		-1
#define CLOSE_ABNORMAL_FLAG		-2
#define CLOSE_SHUTDOWN_FLAG		-3


/*
 * Some forward typedef declarations...
 */
typedef struct _mp_buffer_t	mp_buffer_t;
typedef struct _mp_proto_t	mp_proto_t;


/*
 * A default buffer storage free list, to avoid malloc calls on small
 * sends and receives.
 *
 * Access to the free list must be locked by mp_enter() and mp_exit().
 */
typedef struct _mp_storage_t
{
    struct _mp_storage_t *	next;
    char			storage[1];
} mp_storage_t;

static int mp_default_storage_size;
static mp_storage_t *default_storage_free_list = (mp_storage_t *) NULL;

#define MallocMpDefaultStorage(Routine, Ptr) \
{ \
    mp_storage_t *__s; \
    NexusMalloc(Routine, __s, mp_storage_t *, \
		(sizeof(mp_storage_t)+mp_default_storage_size-1)); \
    (Ptr) = &(__s->storage[0]); \
}

#define GetMpDefaultStorage(Routine, Ptr) \
{ \
    if (default_storage_free_list) \
    { \
	Ptr = default_storage_free_list->storage; \
	default_storage_free_list = default_storage_free_list->next; \
    } \
    else \
    { \
	MallocMpDefaultStorage(Routine, Ptr); \
    } \
}

#define FreeMpDefaultStorage(Ptr) \
{ \
    mp_storage_t *__s; \
    __s = (mp_storage_t *) (((char *) (Ptr)) - sizeof(mp_storage_t *)); \
    __s->next = default_storage_free_list; \
    default_storage_free_list = __s; \
}


/*
 * A mp_buffer_t free list, to avoid malloc calls on the
 * main body of a message buffer.
 *
 * Access to the free list must be locked by mp_enter() and mp_exit().
 */
static mp_buffer_t *	buffer_free_list = (mp_buffer_t *) NULL;

#define SEND_BUFFER	1
#define RECV_BUFFER	2

#ifdef BUILD_DEBUG
#define MallocMpBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, mp_buffer_t *, \
		    sizeof(struct _mp_buffer_t)); \
	Buf->magic = NEXUS_BUFFER_MAGIC; \
    }
#else  /* BUILD_DEBUG */
#define MallocMpBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, mp_buffer_t *, \
		    sizeof(struct _mp_buffer_t)); \
    }
#endif /* BUILD_DEBUG */

#define GetMpBuffer(Routine, Buf) \
    if (buffer_free_list) \
    { \
	Buf = buffer_free_list; \
	buffer_free_list = buffer_free_list->next; \
    } \
    else \
    { \
	MallocMpBuffer(Routine, Buf); \
    }

#define FreeMpBuffer(Buf) \
{ \
    if ((Buf)->storage != (char *) NULL) \
    { \
        if ((Buf)->is_default_storage) \
	{ \
	    FreeMpDefaultStorage((Buf)->storage); \
	} \
	else \
	{ \
	    NexusFree((Buf)->storage); \
	} \
    } \
    (Buf)->next = buffer_free_list; \
    buffer_free_list = (Buf); \
}

#define InitMpRecvBuffer(Buf) \
{ \
    buf->funcs = &mp_buffer_funcs; \
    buf->sizeof_table = &mp_sizeof_table; \
    buf->buffer_type = RECV_BUFFER; \
    buf->handler_name = (char *) NULL; \
    buf->u.recv.stashed = NEXUS_FALSE; \
}


/*
 * This is a message queue containing all messages that have
 * arrived and are waiting to be handled.
 */
static mp_buffer_t *	message_queue_head;
static mp_buffer_t *	message_queue_tail;

#define EnqueueMessage(buf) \
{ \
    if (message_queue_head == (mp_buffer_t *) NULL) \
    { \
	message_queue_head = message_queue_tail = buf; \
    } \
    else \
    { \
	message_queue_tail->next = buf; \
	message_queue_tail = buf; \
    } \
    buf->next = (mp_buffer_t *) NULL; \
}
#define DequeueMessage(buf) \
{ \
    /* Assume that message_queue_head != NULL */ \
    buf = message_queue_head; \
    message_queue_head = message_queue_head->next; \
}
#define MessagesEnqueued() (message_queue_head != (mp_buffer_t *) NULL)

#ifdef NEXUS_CRITICAL_PATH_TIMER
#define MP_START_CRITICAL_PATH_TIMER() \
{ \
    if (_nx_time_critical_path && handle_or_enqueue == HANDLE_MESSAGES) \
    { \
	_nx_critical_path_timer_started = NEXUS_TRUE; \
	UTP_start_timer(_nx_critical_path_start_timer); \
    } \
}
#else
#define MP_START_CRITICAL_PATH_TIMER()
#endif

/*********************************************************************
 * 		Include protocol configuration
 *********************************************************************/
#include NEXUS_MP_INCLUDE


/*
 * mp_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * mp specific information to that structure.
 */
struct _mp_buffer_t
{
#ifdef BUILD_DEBUG
    int				magic;
#endif
    nexus_buffer_funcs_t *	funcs;
    nexus_sizeof_table_t *	sizeof_table;
    mp_buffer_t *		next;
    int				buffer_type; /* SEND_BUFFER || RECV_BUFFER */

    /* Pointers to message storage */
    char *			storage;
    char *			current;
    int				size;
    nexus_bool_t		is_default_storage;
    
    /* Keep next 5 ints together, so one put_int/get_int can do them all */
    int				handler_id;
    int				liba_size;
    int				handler_name_length;
#ifdef BUILD_PROFILE
    int				source_node_id;
    int				source_context_id;
    int				dest_node_id;
    int				dest_context_id;
#endif	    

    char *			handler_name;
    
    union
    {
	struct /* send */
	{
	    mp_proto_t *		proto;
	    mp_send_message_id_t	message_id;
	    int				n_elements;
	    int				header_size;
	    nexus_byte_t *		liba;
	    nexus_startpoint_t * 	sp;
	} send;
	struct /* recv */
	{
	    unsigned long		context;
	    unsigned long		address;
	    nexus_handler_type_t	handler_type;
	    nexus_handler_func_t	handler_func;
	    nexus_bool_t		stashed;
	} recv;
    } u;
};


static mp_destination_t	my_node;
static int		n_nodes;


/*
 * mp_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * mp specific information to that structure.
 */
struct _mp_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_MP */
    nexus_proto_funcs_t *funcs;
    mp_destination_t	destination;
    int			reference_count;
};



/*
 * Protocol table stuff.
 *
 * The protocol table is hashed on the destination. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple mp_proto_t
 * objects to the same context.  Multiple global pointers to the same
 * context share a mp_proto_t.
 */
typedef struct _proto_table_entry_t
{
    mp_proto_t *proto;
    struct _proto_table_entry_t *next;
} proto_table_entry_t;

struct _proto_table_entry_t	proto_table[PROTO_TABLE_SIZE];

static void			proto_table_init(void);
static void			proto_table_insert(mp_proto_t *proto);
static mp_proto_t *		proto_table_lookup(mp_destination_t *dest);


/*
 * Various forward declarations of procedures
 */
static nexus_proto_type_t	mp_proto_type(void);
static void		mp_init(int *argc, char ***argv);
static void		mp_shutdown(nexus_bool_t shutdown_others);
static void		mp_abort(void);
static nexus_bool_t	mp_poll(void);
static void		mp_blocking_poll(void);

static int		mp_send_rsr(nexus_buffer_t *buffer,
				    nexus_startpoint_t *startpoint,
				    int handler_id,
				    nexus_bool_t destroy_buffer,
				nexus_bool_t called_from_non_threaded_handler);
static void             mp_increment_reference_count(nexus_proto_t *nproto);
static void		mp_decrement_reference_count(nexus_proto_t *nproto);
static void		mp_get_my_mi_proto(nexus_byte_t **array,
					   int *size);
static nexus_bool_t	mp_construct_from_mi_proto(nexus_proto_t **proto,
						   nexus_mi_proto_t *mi_proto,
						   nexus_byte_t *array,
						   int size);
static int		mp_direct_info_size(void);

static mp_proto_t *	construct_proto(mp_destination_t destination);
static void		free_proto(mp_proto_t *proto);

static nexus_bool_t	receive_messages(int blocking, int handle_or_enqueue);
static void		handle_enqueued_messages(void);
static void		broadcast_command(int command);

static void		mp_set_buffer_size(nexus_buffer_t *buffer,
					    int size, int n_elements);
static int		mp_check_buffer_size(nexus_buffer_t *buffer,
					      int slack, int increment);
static void		mp_free_buffer(nexus_buffer_t *buffer);
static void		mp_stash_buffer(nexus_buffer_t *buffer,
				       nexus_stashed_buffer_t *stashed_buffer);
static void		mp_free_stashed_buffer(
				       nexus_stashed_buffer_t *stashed_buffer);


static nexus_proto_funcs_t mp_proto_funcs =
{
    mp_proto_type,
    mp_init,
    mp_shutdown,
    mp_abort,
    mp_poll,
    mp_blocking_poll,
    mp_increment_reference_count,
    mp_decrement_reference_count,
    mp_get_my_mi_proto,
    mp_construct_from_mi_proto,
    NULL /* mp_test_proto */,
    mp_send_rsr,
    mp_direct_info_size,
    NULL /* mp_direct_get */,
};


/*
 * _nx_pr_*_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *MP_PROTOCOL_INFO(void)
{
    return((void *) (&mp_proto_funcs));
} /* _nx_pr_*_info() */


/*
 * mp_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t mp_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_MP);
} /* mp_proto_type() */


#ifdef MP_PROTO_IS_THREAD_SAFE    
/*
 * mp_handler_thread()
 *
 * In the multi-threaded version, this is the entry point
 * for the handler thread.
 */
static void *mp_handler_thread(void *arg)
{
    _nx_set_i_am_mp_handler_thread();
    
    mp_enter();
    receive_messages(BLOCKING, HANDLE_MESSAGES);
    mp_exit();

    nexus_mutex_lock(&handler_thread_done_mutex);
    handler_thread_done = NEXUS_TRUE;
    nexus_cond_signal(&handler_thread_done_cond);
    nexus_mutex_unlock(&handler_thread_done_mutex);
    
    return ((void *) NULL);
} /* mp_handler_thread() */
#endif /* MP_PROTO_IS_THREAD_SAFE */


/*
 * mp_init()
 *
 * Initialize the MP protocol.
 */
static void mp_init(int *argc, char ***argv)
{
    int arg_num;
    char *size_string;

    /* Get the default storage size */
    if ((arg_num = nexus_find_argument(argc, argv, "mp_buf", 2)) >= 0)
    {
	mp_default_storage_size = MAX(atoi((*argv)[arg_num + 1]),
				      MP_MIN_STORAGE_SIZE);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	mp_default_storage_size = MP_DEFAULT_STORAGE_SIZE;
    }

    MP_INIT_NODE_INFO(my_node, n_nodes);
    
#ifndef BUILD_LITE
    nexus_thread_key_create(&i_am_mp_handler_thread_key, NULL);
#endif
    message_queue_head = message_queue_tail = (mp_buffer_t *) NULL;
    proto_table_init();
    nexus_mutex_init(&mp_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&mp_cond, (nexus_condattr_t *) NULL);
    mp_done = NEXUS_FALSE;
    handle_in_progress = NEXUS_FALSE;
    send_in_progress = NEXUS_FALSE;
    sends_waiting = 0;
    nexus_cond_init(&sends_waiting_cond, (nexus_condattr_t *) NULL);

    MP_RECEIVE_INIT(mp_init());
    
#ifdef MP_PROTO_IS_THREAD_SAFE    
    if (nexus_preemptive_threads())
    {
	nexus_thread_t thread;

	using_handler_thread = NEXUS_TRUE;

	/* NULL out the poll and blocking_poll entries in my function table */
	mp_proto_funcs.poll = NULL;
	mp_proto_funcs.blocking_poll = NULL;
	
	/* Create the handler thread */
	handler_thread_done = NEXUS_FALSE;
	nexus_mutex_init(&handler_thread_done_mutex,
			 (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&handler_thread_done_cond,
			(nexus_condattr_t *) NULL);
	nexus_thread_create(&thread,
			    (nexus_thread_attr_t *) NULL,
			    mp_handler_thread,
			    (void *) NULL);
    }
    else
#endif /* MP_PROTO_IS_THREAD_SAFE */
    {
	using_handler_thread = NEXUS_FALSE;
    }
} /* mp_init() */


/*
 * mp_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 */
static void mp_shutdown(nexus_bool_t shutdown_others)
{
    int i;
    mp_proto_t *proto;
    nexus_bool_t i_am_mp_handler_thread;

    mp_enter();
    mp_done = NEXUS_TRUE;

    if (using_handler_thread)
    {
	_nx_i_am_mp_handler_thread(&i_am_mp_handler_thread);
	if (!i_am_mp_handler_thread)
	{
	    /*
	     * If this is not the mp handler thread, then we need
	     * to get the handler thread to shutdown.
	     *
	     * Since there other thread may be sitting in a blocking
	     * receive, we need to send a message to myself
	     * to wake up the handler thread.  Otherwise
	     * the handler will not notice the mp_done flag is set.
	     */

	    /* Wakeup the blocked handler thread*/
	    MP_WAKEUP_HANDLER();

	    /* Wait for the handler thread to shutdown */
	    mp_exit();
	    nexus_mutex_lock(&handler_thread_done_mutex);
	    while (!handler_thread_done)
	    {
		nexus_cond_wait(&handler_thread_done_cond,
				&handler_thread_done_mutex);
	    }
	    nexus_mutex_unlock(&handler_thread_done_mutex);
	    mp_enter();
	}
	nexus_mutex_destroy(&handler_thread_done_mutex);
	nexus_cond_destroy(&handler_thread_done_cond);
	using_handler_thread = NEXUS_FALSE;
    }

    /*
     * If I'm supposed to shutdown other nodes, then
     * broadcast a shutdown command to all other nodes.
     */
    if (shutdown_others)
    {
#ifdef TARGET_ARCH_PARAGON
		PARAGON_SHUTDOWN
#else
		broadcast_command(CLOSE_SHUTDOWN_FLAG);
#endif
    }

    /*
     * Call system routine to remove me from the system
     */
    MP_NODE_EXIT();
    
    mp_exit();

} /* mp_shutdown() */


/*
 * mp_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void mp_abort(void)
{
    MP_ABORT();
} /* mp_abort() */


/*
 * mp_poll()
 *
 * In a version of the mp protocol module that does not
 * use a handler thread (preemptive thread module & thread safe blocking
 * receives), this routine should check to see if there are
 * any messages to receive, and if so then receive them and invoke them.
 */
static nexus_bool_t mp_poll(void)
{
    nexus_bool_t message_handled = NEXUS_FALSE;

    /*
     * This should not be called if a separate handler thread is in use.
     */
    NexusAssert2((!using_handler_thread),
		 ("mp_poll(): Internal error: Should never be called when using a handler thread\n") );
    
    nexus_debug_printf(5, ("mp_poll(): entering\n"));
    
    mp_enter();
    if (!handle_in_progress)
    {
	message_handled = receive_messages(NON_BLOCKING, HANDLE_MESSAGES);
    }
    mp_exit();

#ifndef BUILD_LITE
    /*
     * Only yield the processor if there was a message handled.
     * That handler may have enabled another thread for execution.
     */
    if (message_handled)
    {
	nexus_thread_yield();
    }
#endif
    
    nexus_debug_printf(5, ("mp_poll(): exiting\n"));

    return (message_handled);
} /* mp_poll() */


/*
 * mp_blocking_poll()
 *
 * In a version of the mp protocol module that does not
 * use a handler thread, this routine should do a blocking
 * message receive and invoke the handler for the message.
 * This is only called if this is the only protocol module
 * requiring polling, and all other threads are suspended.
 *
 * It is ok for this to return without actually handling
 * a message.
 *
 * Note: There is no need to call nexus_thread_yield() from this
 * routine like there is in mp_poll(), because this routine will only
 * be used from an idle or handler thread which will do a subsequent
 * nexus_thread_yield() anyway.
 */
static void mp_blocking_poll(void)
{
    /*
     * This should not be called if a separate handler thread is in use.
     */
    NexusAssert2((!using_handler_thread),
		 ("mp_blocking_poll(): Internal error: Should never be called when using a handler thread\n") );
    
    nexus_debug_printf(5, ("mp_blocking_poll(): entering\n"));
    mp_enter();
    if (!handle_in_progress)
    {
	receive_messages(BLOCKING, HANDLE_MESSAGES);
    }
    mp_exit();
    nexus_debug_printf(5, ("mp_blocking_poll(): exiting\n"));
    
} /* mp_blocking_poll() */


/*
 * mp_send_rsr()
 *
 * Generate a remote service request message to the node and context
 * saved in the 'nexus_buffer'.
 */
static int mp_send_rsr(nexus_buffer_t *buffer,
		       nexus_startpoint_t *startpoint,
		       int handler_id,
		       nexus_bool_t destroy_buffer,
		       nexus_bool_t called_from_non_threaded_handler)
{
    nexus_bool_t send_done;
    mp_proto_t *proto;
    int i_am_mp_handler_thread = -1;

    NexusBufferMagicCheck(mp_send_remote_service_request, buffer);
    NEXUS_INTERROGATE(startpoint, _NX_STARTPOINT_T, "mp_send_rsr");
    nexus_debug_printf(2,("mp_send_rsr(): invoked with buffer:%x\n",buffer));

#define THRESHOLD 1500 /* totally arbitrary and should be changed */
    total_direct_puts = 0;
    for (direct_segment = (*buffer)->direct_segments;
	 direct_segment;
	 direct_segment = direct_segment->next)
    {
        for (i = 0; i < direct_segment->size; i++)
	{
	    if (direct_segment->storage[i].size < THRESHOLD)
	    {
	        direct_segment->storage[i].action
		    = NEXUS_DIRECT_INFO_ACTION_INLINE;
	    }
	    else
	    {
	        direct_segment->storage[i].action
		    = NEXUS_DIRECT_INFO_ACTION_CUSTOM;
	    }
	}
	total_direct_puts += direct_segment->size;
    }

    _nx_buffer_coalesce(*buffer,
			&r_buffer,
			startpoint,
			handler_id,
			total_direct_puts,
			called_from_non_threaded_handler,
			NEXUS_TRUE,
			destroy_buffer);
    
    mp_enter();
    
    proto = (mp_proto_t *) startpoint->mi_proto->proto;
    NexusAssert2((proto->type == NEXUS_PROTO_TYPE_MP),
		 ("mp_send_rsr(): Internal error: proto_type is not NEXUS_PROTO_TYPE_EUI\n"));

#ifdef BUILD_PROFILE
    _nx_pablo_log_remote_service_request_send(buf->dest_node_id,
					      buf->dest_context_id,
					      buf->handler_name,
					      buf->handler_id,
					      (buf->size
					       - buf->u.send.header_size) );
#endif
    
    /*
     * Make sure there is only one thread trying to send at a time.
     * Suspend any others that try.
     */
    while (send_in_progress)
    {
	mp_send_wait();
    }
    send_in_progress = NEXUS_TRUE;

#ifdef NEXUS_CRITICAL_PATH_TIMER
    if (_nx_critical_path_timer_started)
    {
	UTP_stop_timer(_nx_critical_path_stop_timer);
	_nx_critical_path_timer_started = NEXUS_FALSE;
    }
#endif

    MP_SEND(buf);

    MP_SEND_STATUS(buf, send_done);
    while (!send_done)
    {
	if (using_handler_thread)
	{
	    if (i_am_mp_handler_thread < 0)
	    {
		_nx_i_am_mp_handler_thread(&i_am_mp_handler_thread);
		NexusAssert2((i_am_mp_handler_thread >= 0),
			     ("mp_send_remote_service_request(): Internal error: _nx_i_am_mp_handler_thread() failed\n"));
	    }
	    if (i_am_mp_handler_thread)
	    {
		/*
		 * This send must be from a handler.
		 * So just receive messages and enqueue them.
		 * We'll handle them later when the handler that
		 * called this send returns.
		 */
		receive_messages(NON_BLOCKING, ENQUEUE_MESSAGES);
	    }
#ifndef BUILD_LITE
	    /*
	     * Else if I am not the handler thread, then yield.
	     * The handler thread will wake up automatically to
	     * handle messages if something arrives, but we must
	     * release the mp_mutex to allow it to do its thing.
	     * The yield will also let other threads get useful work done.
	     *
	     * Note: Since a handler thread is only used with
	     * preemptive threads, yielding here should not cause
	     * a problem with starving this thread.
	     */
	    mp_exit();
	    nexus_thread_yield();
	    mp_enter();
#endif
	}
	else
	{
	    /*
	     * Messages enqueued here will get handled below by nexus_poll.
	     */
	    receive_messages(NON_BLOCKING, ENQUEUE_MESSAGES);
	}
	
	MP_SEND_STATUS(buf, send_done);
    }

    FreeMpBuffer(buf);
    send_in_progress = NEXUS_FALSE;
    mp_send_signal();	/* Wake up any threads waiting to send */
    mp_exit();

    nexus_poll();

    return(0);
    
} /* mp_send_rsr() */


#ifdef NEXUS_USE_FORWARDER
/*
 * _nx_mp_send_bytes()
 *
 * Send raw bytes to the specified nodes.
 */
void _nx_mp_send_bytes(int dest_node,
			      nexus_byte_t *buf,
			      int size)
{
    mp_send_message_id_t message_id;
    nexus_bool_t send_done;
    int i_am_mp_handler_thread = -1;

    nexus_debug_printf(2, ("_nx_mp_send_bytes(): to=%d, size=%d\n", dest_node, size));

    mp_enter();
    
    /*
     * Make sure there is only one thread trying to send at a time.
     * Suspend any others that try.
     */
    while (send_in_progress)
    {
	mp_send_wait();
    }
    send_in_progress = NEXUS_TRUE;

#ifdef NEXUS_CRITICAL_PATH_TIMER
    if (_nx_critical_path_timer_started)
    {
	UTP_stop_timer(_nx_critical_path_stop_timer);
	_nx_critical_path_timer_started = NEXUS_FALSE;
    }
#endif

    MP_SEND_BYTES(dest_node, buf, size, message_id);

    MP_SEND_BYTES_STATUS(message_id, send_done);
    while (!send_done)
    {
	if (using_handler_thread)
	{
	    if (i_am_mp_handler_thread < 0)
	    {
		_nx_i_am_mp_handler_thread(&i_am_mp_handler_thread);
		NexusAssert2((i_am_mp_handler_thread >= 0),
			     ("_nx_mp_send_bytes(): Internal error: _nx_i_am_mp_handler_thread() failed\n"));
	    }
	    if (i_am_mp_handler_thread)
	    {
		/*
		 * This send must be from a handler.
		 * So just receive messages and enqueue them.
		 * We'll handle them later when the handler that
		 * called this send returns.
		 */
		receive_messages(NON_BLOCKING, ENQUEUE_MESSAGES);
	    }
#ifndef BUILD_LITE
	    /*
	     * Else if I am not the handler thread, then yield.
	     * The handler thread will wake up automatically to
	     * handle messages if something arrives, but we must
	     * release the mp_mutex to allow it to do its thing.
	     * The yield will also let other threads get useful work done.
	     *
	     * Note: Since a handler thread is only used with
	     * preemptive threads, yielding here should not cause
	     * a problem with starving this thread.
	     */
	    mp_exit();
	    nexus_thread_yield();
	    mp_enter();
#endif
	}
	else
	{
	    /*
	     * Messages enqueued here will get handled below by nexus_poll.
	     */
	    receive_messages(NON_BLOCKING, ENQUEUE_MESSAGES);
	}
	
	MP_SEND_BYTES_STATUS(message_id, send_done);
    }

    send_in_progress = NEXUS_FALSE;
    mp_send_signal();	/* Wake up any threads waiting to send */
    mp_exit();

} /* _nx_mp_send_bytes() */
#endif /* NEXUS_USE_FORWARDER */


/*
 * mp_increment_reference_count()
 *
 * Increase the reference count on the associated proto and copy the
 * pointer to the nexus_proto_t
 *
 */
static void mp_increment_reference_count(nexus_proto_t *nproto)
{
    mp_proto_t *proto = (mp_proto_t *) nproto;
    mp_enter();
    proto->reference_count++;
    mp_exit();
} /* mp_increment_reference_count() */


/*
 * mp_decrement_reference_count()
 *
 * Decrement the reference count for this proto.  If it goes to 0
 * then close the fd used by this proto.
 */
static void mp_decrement_reference_count(nexus_proto_t *nproto)
{
    mp_proto_t *proto = (mp_proto_t *) nproto;
    mp_enter();
    proto->reference_count--;
    NexusAssert2((proto->reference_count >= 0),
		 ("mp_decrement_reference_count(): Internal error: Reference count < 0\n"));
    mp_exit();
} /* mp_decrement_reference_count() */


/*
 * mp_get_my_mi_proto()
 *
 * Return the machine independent mp protocol information
 * for this protocol.
 */
static void mp_get_my_mi_proto(nexus_byte_t **array,
			       int *size)
{
    int my_size;
    MP_GET_MY_MI_PROTO_SIZE(my_size);
    *size = (_nx_session_string_length + 1 + my_size); \
    NexusMalloc(mp_get_my_mi_proto(), \
		*array, \
		nexus_byte_t *, \
		*size); \
    memcpy(*array, _nx_session_string, _nx_session_string_length + 1);
    MP_GET_MY_MI_PROTO(((*array) + _nx_session_string_length + 1));
} /* mp_get_my_mi_proto() */


/*
 * mp_construct_from_mi_proto()
 *
 * From the passed machine independent protocol list ('mi_proto'), plus
 * the mp specific entry from that list ('proto_array' and 'size'),
 * see if I can use the information to create a nexus_proto_t object
 * that can be used to connect to the node:
 *	- If I cannot use this protocol to attach to the node, then
 *		return NEXUS_FALSE.  (This option is useful if two nodes
 *		both speak a particular protocol, but they cannot
 *		talk to each other via that protocol.  For example,
 *		on two MPP, the nodes within a single MPP can
 *		talk to each other via the native messaging protocol,
 *		but cannot talk to the nodes on the other MPP
 *		using that native protocol.)
 *	- If this mp protocol points to myself, then set
 *		*proto=NULL, and return NEXUS_TRUE.
 *	- Otherwise, construct a mp protocol object for this mi_proto
 *		and put it in *proto.  Then return NEXUS_TRUE.
 */
static nexus_bool_t mp_construct_from_mi_proto(nexus_proto_t **proto,
					       nexus_mi_proto_t *mi_proto,
					       nexus_byte_t *array,
					       int size)
{
    mp_destination_t destination;
    nexus_bool_t result;

    /*
     * Compare the session string from the array with mine.
     */
    if (strcmp((char *) array, _nx_session_string) != 0)
    {
	return(NEXUS_FALSE);
    }

    /*
     * Extract the mp_destination_t from the array
     */
    MP_CONSTRUCT_FROM_MI_PROTO(destination,
			       mi_proto,
			       (array + _nx_session_string_length + 1));
    
    /*
     * Test to see if this mi_proto points to myself.
     * If it does, then return *proto=NULL.
     */
    MP_COMPARE_DESTINATIONS(destination, my_node, result);
    if (result)
    {
	*proto = (nexus_proto_t *) NULL;
    }
    else
    {
	mp_enter();
	*proto = (nexus_proto_t *) construct_proto(destination);
	mp_exit();
    }
    return (NEXUS_TRUE);
} /* mp_construct_from_mi_proto() */


/*
 * mp_direct_info_size()
 */
static int mp_direct_info_size(void)
{
    /* TODO: This needs to be filled in */
    return(0);
} /* mp_direct_info_size() */


/*
 * construct_proto()
 *
 * Construct a mp_proto_t for the given destination. Look up in the
 * proto table to see if one already exists. If it does, bump its reference
 * count and return that one. Otherwise create one, insert into the
 * table with a reference count of 1 and return it.
 */
static mp_proto_t *construct_proto(mp_destination_t destination)
{
    mp_proto_t *proto;

    proto = proto_table_lookup(&destination);
    nexus_debug_printf(3,
		       ("construct_proto(): Table lookup returns proto=%x\n",
			proto));
    if (proto == (mp_proto_t *) NULL)
    {
	NexusMalloc(construct_proto(), proto, mp_proto_t *,
		    sizeof(mp_proto_t));

	proto->type = NEXUS_PROTO_TYPE_MP;
	proto->funcs = &mp_proto_funcs;
	proto->reference_count = 1;

	MP_COPY_DESTINATION(proto->destination, destination);
	
	proto_table_insert(proto);
    }
    else
    {
	proto->reference_count++;
    }
	
    return (proto);
} /* construct_proto() */


/*
 * free_proto()
 *
 * Free the passed 'proto'.
 */
static void free_proto(mp_proto_t *proto)
{
    MP_FREE_DESTINATION(proto->destination);

    NexusFree(proto);
} /* free_proto() */


/*
 * receive_messages();
 *
 * Receive all pending messages to this node.
 *
 * If blocking==BLOCKING, use a blocking receive.
 * If blocking==NON_BLOCKING, use a non-blocking receive.  Do not
 * return until the non-blocking receive fails.
 *
 * If handle_or_enqueue==HANDLE_MESSAGES, then invoke the
 * message handler immediately upon receipt of a message.
 * If handle_or_enqueue==ENQUEUE_MESSAGES, then enqueue
 * any messages that are received onto the message_queue, to
 * be handled later.
 *
 * Note: If using_handler_thread==NEXUS_TRUE,
 *	 this routine will never be called by anyone except the
 *	 handler thread.  (Though it may be called recursively by the
 *	 the handler thread, if a send_rsr is done from within a handler.)
 *
 * Return: NEXUS_TRUE if a message is handled, otherwise NEXUS_FALSE
 */
static nexus_bool_t receive_messages(int blocking, int handle_or_enqueue)
{
    nexus_bool_t done = NEXUS_FALSE;
    nexus_bool_t message_received;
    nexus_bool_t message_handled = NEXUS_FALSE;
    mp_buffer_t *buf;
    char handler_name[NEXUS_MAX_HANDLER_NAME_LENGTH];
    nexus_byte_t liba_storage[NEXUS_DEFAULT_LIBA_SIZE];
    nexus_byte_t *liba;
    nexus_bool_t need_to_relock;
    nexus_bool_t try_probe_and_receive;
    unsigned char liba_flag;
    unsigned long endpoint_id;
    nexus_gp_endpoint_t *endpoint;

    nexus_debug_printf(3, ("receive_messages(): entering\n"));
    
    /*
     * Handle any enqueued messages if need be.
     */
    if (handle_or_enqueue == HANDLE_MESSAGES && MessagesEnqueued())
    {
	handle_enqueued_messages();
	message_handled = NEXUS_TRUE;
    }

    while (!done)
    {
	if (blocking)
	{
	    if (using_handler_thread)
	    {
		/*
		 * This is the handler thread.  Since it is thread safe,
		 * we should do an mp_exit() so that other threads can
		 * do sends while this thread is blocked in the receive.
		 */
		need_to_relock = NEXUS_TRUE;
		mp_exit();
	    }
	    else
	    {
		need_to_relock = NEXUS_FALSE;
	    }
	    try_probe_and_receive = NEXUS_FALSE;
	    MP_BLOCKING_RECEIVE(receive_messages(),
				buf,
				need_to_relock);
	    if (need_to_relock)
	    {
		mp_enter();
	    }
	    if (!using_handler_thread)
	    {
		if (try_probe_and_receive)
		{
		    /*
		     * Switch to non-blocking after the first message so
		     * that this thread won't just indefinitely spin in
		     * a blocking receive loop.
		     */
		    blocking = NON_BLOCKING;
		}
		else
		{
		    /*
		     * Just exit receive_messages() after handling
		     * this message.
		     * This is a good option to take when
		     * the non-blocking check for messages is expensive.
		     */
		    done = NEXUS_TRUE;
		}
	    }
	    message_received = NEXUS_TRUE;
	}
	else
	{
	    MP_PROBE_AND_RECEIVE(receive_messages(),
				 buf,
				 message_received);
	}
	
	if (mp_done)
	    return;
	
	if (message_received)
	{
	    /*
	     * We received a message into 'buf'.
	     */
	    
	    /* get handler_id */
	    GET_INT(buf, &(buf->handler_id), 1);

	    if (buf->handler_id >= 0)
	    {
		/* get liba_size, handler_name_length, node_id, context_id */
		GET_INT(buf, &(buf->liba_size), (MP_MSG_HEADER_INTS-1));
		
		/* get liba */
		if (buf->liba_size <= NEXUS_DEFAULT_LIBA_SIZE)
		{
		    liba = liba_storage;
		}
		else
		{
		    NexusMalloc(receive_messages(),
				liba,
				nexus_byte_t *,
				buf->liba_size);
		}
		GET_BYTE(buf, liba, buf->liba_size);
		
		/* get handler_name */
		GET_CHAR(buf, handler_name, buf->handler_name_length);
		handler_name[buf->handler_name_length] = '\0';

		/* unpack the liba */
	    /* UnpackLiba(liba, buf->u.recv.context, buf->u.recv.address); */
		UnpackLiba(liba, liba_flag, endpoint_id, buf->u.recv.address);
		if (endpoint = ENDPOINT_FROM_ENDPOINT_ID(liba_flag, 
				    endpoint_id, NEXUS_FALSE))
		    buf->u.recv.context = endpoint->context;
		else
		{
		    nexus_fatal("receive_messages(): extracted NULL endpoint from LIBA ... liba_flag %d endpoint_id %x\n", liba_flag, endpoint_id);
		} /* endif */
		if (liba != liba_storage)
		{
		    NexusFree(liba);
		}

		if (buf->u.recv.handler_type == NEXUS_HANDLER_TYPE_THREADED)
		{
		    buf->u.recv.stashed = NEXUS_TRUE;
		}

		/* Enqueue or handle the message */
		if (handle_or_enqueue == HANDLE_MESSAGES)
		{
		    handle_in_progress = NEXUS_TRUE;
		    mp_exit();
		    _nx_handle_message(buf->handler_id,
				       buf->endpoint,
#ifdef BUILD_PROFILE
				       buf->source_node_id,
				       buf->source_context_id,
				       (buf->size
					- MP_MSG_HEADER_SIZE
					- buf->liba_size
					- buf->handler_name_length),
#endif
				       (void *) buf);
		    mp_enter();
		    handle_in_progress = NEXUS_FALSE;
		    message_handled = NEXUS_TRUE;
		    
		    /*
		     * Handle any enqueued messages.
		     * It is possible that a message could have
		     * been enqueued during this last message handle.
		     */
		    if (MessagesEnqueued())
		    {
			handle_enqueued_messages();
		    }
		}
		else
		{
		    EnqueueMessage(buf);
		}

		MP_POST_RECEIVE(receive_messages());
	    }
	    else if (buf->handler_id == CLOSE_SHUTDOWN_FLAG)
	    {
#ifdef NEXUS_CRITICAL_PATH_TIMER
		UTP_stop_timer(_nx_critical_path_stop_timer);
		_nx_critical_path_timer_started = NEXUS_FALSE;
#endif

		mp_exit();

		/*
		 * TODO: This should use nexus_exit(), but I don't
		 * want to deal with it for now.
		 * nexus_exit(0, NEXUS_TRUE);
		 */
		_nx_exit_transient_process(0);
	    }
	    else
	    {
		nexus_fatal("receive_messages(): Got unknown control message\n");
	    }
	}
	else
	{
	    /*
	     * The (non-blocking) receive didn't receive anything,
	     */
	    done = NEXUS_TRUE;
	}
    }

    nexus_debug_printf(3,
		       ("receive_messages(): returning message_handled=%d\n",
			message_handled) );

    return (message_handled);
    
} /* receive_messages() */


/*
 * handle_enqueued_messages()
 *
 * Handle all messages that are enqueued.
 */
static void handle_enqueued_messages(void)
{
    mp_buffer_t *buf;

    handle_in_progress = NEXUS_TRUE;
    while (MessagesEnqueued())
    {
	DequeueMessage(buf);
	mp_exit();
	_nx_handle_message(buf->handler_id,
			   buf->endpoint,
#ifdef BUILD_PROFILE
			   buf->source_node_id,
			   buf->source_context_id,
			   (buf->size - MP_MSG_HEADER_SIZE
			    - buf->handler_name_length),
#endif
			   (void *) buf);
	mp_enter();
    }
    handle_in_progress = NEXUS_FALSE;

} /* handle_enqueued_messages() */


/*
 * broadcast_command()
 *
 * Send the 'command' to all other nodes.
 *
 * TODO: This needs to be generalized.
 */
static void broadcast_command(int command)
{
    MP_BROADCAST_COMMAND(command);
} /* broadcast_command() */


/*
 * proto_table_init()
 *
 * Initialize the protocol table.
 */
static void proto_table_init(void)
{
    int i;

    for (i = 0; i < PROTO_TABLE_SIZE; i++)
    {
	proto_table[i].proto = (mp_proto_t *) NULL;
	proto_table[i].next = (proto_table_entry_t *) NULL;
    }
} /* proto_table_init() */


/*
 * proto_table_insert()
 *
 * Insert the given proto into the table, hashing on its destination.
 *
 * We assume that the entry is not present in the table.
 */
static void proto_table_insert(mp_proto_t *proto)
{
    int bucket;
    proto_table_entry_t *new_ent;

    MP_HASH_DESTINATION(proto->destination, bucket);

    if (proto_table[bucket].proto == (mp_proto_t *) NULL)
    {
	/* Drop it into the preallocated table entry */
	proto_table[bucket].proto = proto;
    }
    else
    {
	/*
	 * Need to allocate a new proto_table_entry_t and add it
	 * to the bucket
	 */
	NexusMalloc(proto_table_insert(),
		    new_ent,
		    proto_table_entry_t *,
		    sizeof(struct _proto_table_entry_t));

	new_ent->proto = proto;
	new_ent->next = proto_table[bucket].next;

	proto_table[bucket].next = new_ent;
    }

} /* proto_table_insert() */


/*
 * proto_table_lookup()
 *
 * Look up and return the mp_proto_t for the given destination.
 * Return NULL if none exists.
 */
static mp_proto_t *proto_table_lookup(mp_destination_t *dest)
{
    proto_table_entry_t *ent;
    int bucket;
    nexus_bool_t result;

    MP_HASH_DESTINATION(*dest, bucket);

    for (ent = &(proto_table[bucket]);
	 ent != (proto_table_entry_t *) NULL;
	 ent = ent->next)
    {
	if (ent->proto != (mp_proto_t *) NULL)
	{
	    MP_COMPARE_DESTINATIONS(*dest, ent->proto->destination, result);
	    if (result)
	    {
		return (ent->proto);
	    }
	}
    }
    
    return ((mp_proto_t *) NULL);
} /* proto_table_lookup() */



/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

/*
 * mp_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void mp_set_buffer_size(nexus_buffer_t *buffer,
			       int size, int n_elements)
{
    mp_buffer_t *mp_buffer;
    char *storage;
    int total_size;

    NexusBufferMagicCheck(mp_check_buffer_size, buffer);

    mp_buffer = (mp_buffer_t *) *buffer;
    
    NexusAssert2((mp_buffer->buffer_type == SEND_BUFFER),
		 ("mp_set_buffer_size(): Internal error: Expected a send buffer\n"));

    if (size > 0 )
    {
	total_size = size + mp_buffer->u.send.header_size;
	if (total_size < mp_default_storage_size)
	{
	    total_size = mp_default_storage_size;
	    /*
	     * Just use the default storage that was preallocated
	     * in mp_init_remote_service_request().
	     */
	}
	else
	{
	    /*
	     * Need to free the default storage that was preallocated
	     * in mp_init_remove_service_request().
	     */
	    mp_enter();
	    FreeMpDefaultStorage(mp_buffer->storage);
	    mp_exit();
	    mp_buffer->is_default_storage = NEXUS_FALSE;

	    /*
	     * Now allocate buffer storage of the right size.
	     */
	    NexusMalloc(mp_set_buffer_size(),
			mp_buffer->storage,
			char *,
			total_size);
	}

	mp_buffer->size = total_size;
	mp_buffer->u.send.n_elements = (n_elements < 0 ? n_elements :
					n_elements + MP_MSG_HEADER_N_ELEMENTS);
	mp_buffer->current = mp_buffer->storage + MP_EXTRA_HEADER_BYTES;
	PutHeaderIntoBuffer(mp_buffer);
    }

} /* mp_set_buffer_size() */

/*
 * mp_check_buffer_size()
 *
 * Check that that passed message 'buffer' has at least 'slack'
 * bytes remaining; if not, increase size by 'increment' bytes
 * until there are enough bytes remaining.
 *
 * If no resizing is necessary, leave 'buffer' unchanged and
 * return NEXUS_TRUE.
 * If resizing is successful, modify 'buffer' to a new, larger
 * buffer and return NEXUS_TRUE.
 * Otherwise, if 'increment' is 0 and 'slack' bytes are not
 * available in teh buffer, then leave 'buffer' unchanged and
 * return NEXUS_FALSE.
 */
static int mp_check_buffer_size(nexus_buffer_t *buffer,
				int slack,
				int increment)
{
    mp_buffer_t *mp_buffer;
    int used;
    int needed;

    NexusBufferMagicCheck(mp_check_buffer_size, buffer);

    mp_buffer = (mp_buffer_t *) *buffer;
    
    NexusAssert2((mp_buffer->buffer_type == SEND_BUFFER),
		 ("mp_check_buffer_size(): Internal error: Expected a send buffer\n"));
    
    used = mp_buffer->current - mp_buffer->storage;
    needed = used + slack;

    if (mp_buffer->size == 0)
    {
	mp_set_buffer_size(buffer, slack, -1);
    }
    else if (needed > mp_buffer->size)
    {
	char *new_storage;
	int new_size;

	if (increment <= 0)
	{
	    return(NEXUS_FALSE);
	}

	new_size = mp_buffer->size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

	NexusMalloc(mp_check_buffer_size(),
		    new_storage,
		    char *,
		    new_size);
	
	memcpy(new_storage, mp_buffer->storage, used);

	if (mp_buffer->is_default_storage)
	{
	    mp_enter();
	    FreeMpDefaultStorage(mp_buffer->storage);
	    mp_exit();
	    mp_buffer->is_default_storage = NEXUS_FALSE;
	}
	else
	{
	    NexusFree(mp_buffer->storage);
	}

	mp_buffer->size = new_size;
	mp_buffer->storage = new_storage;
	mp_buffer->current = mp_buffer->storage + used;
    }
    
    return(NEXUS_TRUE);
} /* mp_check_buffer_size() */


/*
 * mp_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since mp_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void mp_free_buffer(nexus_buffer_t *buffer)
{
    mp_buffer_t *mp_buffer;
    
    NexusAssert2((buffer),
		 ("mp_free_buffer(): Passed a NULL nexus_buffer_t *\n") );
    
    /* If the buffer was stashed, *buffer will have been set to NULL */
    if (!(*buffer))
    {
	return;
    }

    NexusBufferMagicCheck(mp_free_buffer, buffer);

    mp_buffer = (mp_buffer_t *) *buffer;
    
    NexusAssert2((mp_buffer->buffer_type == RECV_BUFFER),
		 ("mp_free_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!mp_buffer->u.recv.stashed),
		 ("mp_free_buffer(): Expected a non-stashed buffer\n"));

    if (mp_buffer->handler_name)
    {
	NexusFree(mp_buffer->handler_name);
    }
    
    mp_enter();
    FreeMpBuffer(mp_buffer);
    mp_exit();

    *buffer = (nexus_buffer_t) NULL;
    
} /* mp_free_buffer() */


/*
 * mp_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void mp_stash_buffer(nexus_buffer_t *buffer,
			     nexus_stashed_buffer_t *stashed_buffer)
{
    mp_buffer_t *mp_buffer;
    
    NexusBufferMagicCheck(mp_stash_buffer, buffer);

    mp_buffer = (mp_buffer_t *) *buffer;
    NexusAssert2((mp_buffer->buffer_type == RECV_BUFFER),
		 ("mp_stash_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!mp_buffer->u.recv.stashed),
		 ("mp_stash_buffer(): Expected an un-stashed buffer\n"));
    
    mp_buffer->u.recv.stashed = NEXUS_TRUE;
		  
    *stashed_buffer = (nexus_stashed_buffer_t) *buffer;
    
    *buffer = (nexus_buffer_t) NULL;
} /* mp_stash_buffer() */


/*
 * mp_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by mp_stash_buffer().
 */
static void mp_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
    mp_buffer_t *mp_buffer;
    
    NexusAssert2((stashed_buffer),
		 ("mp_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n") );
    NexusBufferMagicCheck(mp_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
    
    mp_buffer = (mp_buffer_t *) *stashed_buffer;
    
    NexusAssert2((mp_buffer->buffer_type == RECV_BUFFER),
		 ("mp_free_stashed_buffer(): Expected a receive buffer\n"));
    NexusAssert2((mp_buffer->u.recv.stashed),
		 ("mp_free_stashed_buffer(): Expected a stashed buffer\n"));

    mp_enter();
    FreeMpBuffer(mp_buffer);
    mp_exit();

    *stashed_buffer = (nexus_stashed_buffer_t) NULL;
    
} /* mp_free_stashed_buffer() */



/*
 * mp_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int mp_sizeof_float(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->float_size * count);
}

static int mp_sizeof_double(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->double_size * count);
}

static int mp_sizeof_short(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->short_size * count);
}

static int mp_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->u_short_size * count);
}

static int mp_sizeof_int(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->int_size * count);
}

static int mp_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->u_int_size * count);
}

static int mp_sizeof_long(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->long_size * count);
}

static int mp_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->u_long_size * count);
}

static int mp_sizeof_char(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->char_size * count);
}

static int mp_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->u_char_size * count);
}

static int mp_sizeof_byte(nexus_buffer_t *buffer, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buffer;
    return(mp_buf->sizeof_table->byte_size * count);
}




/*
 * mp_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */
static void mp_put_float(nexus_buffer_t *buf, float *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_FLOAT(mp_buf, data, count);
}

static void mp_put_double(nexus_buffer_t *buf, double *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_DOUBLE(mp_buf, data, count);
}

static void mp_put_short(nexus_buffer_t *buf, short *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_SHORT(mp_buf, data, count);
}

static void mp_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_U_SHORT(mp_buf, data, count);
}

static void mp_put_int(nexus_buffer_t *buf, int *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_INT(mp_buf, data, count);
}

static void mp_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_U_INT(mp_buf, data, count);
}

static void mp_put_long(nexus_buffer_t *buf, long *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_LONG(mp_buf, data, count);
}

static void mp_put_u_long(nexus_buffer_t *buf, unsigned long *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_U_LONG(mp_buf, data, count);
}

static void mp_put_char(nexus_buffer_t *buf, char *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_CHAR(mp_buf, data, count);
}

static void mp_put_u_char(nexus_buffer_t *buf, unsigned char *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_U_CHAR(mp_buf, data, count);
}

static void mp_put_byte(nexus_buffer_t *buf, unsigned char *data, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    PUT_BYTE(mp_buf, data, count);
}



/*
 * mp_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */
static void mp_get_float(nexus_buffer_t *buf, float *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_FLOAT(mp_buf, dest, count);
}

static void mp_get_double(nexus_buffer_t *buf, double *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_DOUBLE(mp_buf, dest, count);
}

static void mp_get_short(nexus_buffer_t *buf, short *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_SHORT(mp_buf, dest, count);
}

static void mp_get_u_short(nexus_buffer_t *buf, unsigned short *dest,
			    int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_U_SHORT(mp_buf, dest, count);
}

static void mp_get_int(nexus_buffer_t *buf, int *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_INT(mp_buf, dest, count);
}

static void mp_get_u_int(nexus_buffer_t *buf, unsigned int *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_U_INT(mp_buf, dest, count);
}

static void mp_get_long(nexus_buffer_t *buf, long *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_LONG(mp_buf, dest, count);
}

static void mp_get_u_long(nexus_buffer_t *buf, unsigned long *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_U_LONG(mp_buf, dest, count);
}

static void mp_get_char(nexus_buffer_t *buf, char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_CHAR(mp_buf, dest, count);
}

static void mp_get_u_char(nexus_buffer_t *buf, unsigned char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_U_CHAR(mp_buf, dest, count);
}

static void mp_get_byte(nexus_buffer_t *buf, unsigned char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_BYTE(mp_buf, dest, count);
}



/*
 * mp_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */
static void mp_get_stashed_float(nexus_stashed_buffer_t *buf,
				 float *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_FLOAT(mp_buf, dest, count);
}

static void mp_get_stashed_double(nexus_stashed_buffer_t *buf,
				  double *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_DOUBLE(mp_buf, dest, count);
}

static void mp_get_stashed_short(nexus_stashed_buffer_t *buf,
				 short *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_SHORT(mp_buf, dest, count);
}

static void mp_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				   unsigned short *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_U_SHORT(mp_buf, dest, count);
}

static void mp_get_stashed_int(nexus_stashed_buffer_t *buf,
			       int *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_INT(mp_buf, dest, count);
}

static void mp_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				 unsigned int *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_U_INT(mp_buf, dest, count);
}

static void mp_get_stashed_long(nexus_stashed_buffer_t *buf,
				long *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_LONG(mp_buf, dest, count);
}

static void mp_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				  unsigned long *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_U_LONG(mp_buf, dest, count);
}

static void mp_get_stashed_char(nexus_stashed_buffer_t *buf,
				char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_CHAR(mp_buf, dest, count);
}

static void mp_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				  unsigned char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_U_CHAR(mp_buf, dest, count);
}

static void mp_get_stashed_byte(nexus_stashed_buffer_t *buf,
				unsigned char *dest, int count)
{
    mp_buffer_t *mp_buf = (mp_buffer_t *) *buf;
    GET_STASHED_BYTE(mp_buf, dest, count);
}

