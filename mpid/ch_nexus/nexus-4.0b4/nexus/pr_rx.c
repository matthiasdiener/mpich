/*
 * Nexus
 * Authors:     Stephen Schwab
 *              The Aerospace Corporation
 *
 * pr_rx.c	- Reliable Acknowledgement Retransmission protocol Module
 *                (used to build ATM, UDP and MN_UDP protocols)
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_rx.c,v 1.25 1996/10/07 04:40:10 tuecke Exp $";


#include "internal.h"

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#ifdef TARGET_ARCH_AIX
#include <sys/select.h>
#include <dce/cma_ux.h> /* get the thread-safe version of select */
#define HAVE_THREAD_SAFE_SELECT
#endif

#define USE_TRACEPOINT
/*
*/
/*
*/
#ifdef USE_TRACEPOINT
#include "tracepoint.h"
#else
#define tracepoint(str)
#endif /* USE_TRACEPOINT */

static nexus_mutex_t		rx_mutex;

#define rx_enter()     nexus_mutex_lock(&rx_mutex); 
#define rx_exit()	nexus_mutex_unlock(&rx_mutex); 

#define rx_fatal 	rx_exit(); nexus_fatal

#ifdef TARGET_ARCH_AXP
#define NX_NTOHL(m) m
#define NX_HTONL(m) m
#else
#define NX_NTOHL(m) ntohl(m)
#define NX_HTONL(m) htonl(m)
#endif

struct _rx_adaptor_funcs_t
{
    int type;
    int	(*client)(char *hostname, int socket);
    int (*server)(int fd);
    int (*send)(int fd, const char *buf, int len);
    int (*recv)(int fd, char *buf, int len);
    int (*close)(int fd);
};

typedef struct _rx_adaptor_funcs_t	rx_adaptor_funcs_t;

/*
 * This is a function indirection table used to tie the rx_proto
 * layer to the underlying network interface library.
 */
#define RX_FORE_ATM	101
#define RX_UDP	 	102
#define RX_MN_UDP	103

static int	default_adaptor_type;

#include NEXUS_RX_INCLUDE

/*
 * TIMESTAMP_ECHO 
 *
 * Use an internal protocol timestamping method.
 */
/*
#define TIMESTAMP_ECHO
*/
#undef	TIMESTAMP_ECHO
/*
*/

#ifdef NEXUS_SINGLE_THREADED
#define RX_PROTO_SINGLE_THREADED
#endif


#ifdef NEXUS_SPOOF_COPY
nexus_bool_t _nx_spoof_copy = NEXUS_FALSE;
#endif

#ifndef SOMAXCONN /* for Linux */
#define SOMAXCONN 5
#endif

static int xdrrc;

/*
 * Thread is handler?
 *
 * Thread specific storage is used to keep track if the current
 * thread is a handler thread or not.
 */
static nexus_thread_key_t i_am_rx_handler_thread_key;
#define _nx_set_i_am_rx_handler_thread() \
    nexus_thread_setspecific(i_am_rx_handler_thread_key, (void *) 1)
#define _nx_i_am_rx_handler_thread(Result) \
    *(Result) = (nexus_bool_t)nexus_thread_getspecific(i_am_rx_handler_thread_key)


/*
 * Only one thread is allowed to be in the rx code (and thus
 * mucking with data structures) at a time.
 */

static nexus_cond_t		rx_cond;
static nexus_bool_t		rx_done;
/*static nexus_bool_t		rx_in_use;*/
static nexus_bool_t		handle_in_progress;
static nexus_bool_t		using_rx_handler_thread;
static nexus_bool_t		handler_thread_done;
/*static int			pipe_to_self[2];*/ /* 0=read, 1=write */
static nexus_mutex_t		handler_thread_done_mutex;
static nexus_cond_t		handler_thread_done_cond;
#if 0
static nexus_mutex_t		shutdown_completed_mutex;
static nexus_cond_t		shutdown_completed_cond;
#endif 0
static nexus_bool_t		shutdown_in_progress;
#ifdef TIMESTAMP_ECHO
static unsigned int 		select_tick_clock;
static int 			last_timestamp_rsr_buffers = 100;
#endif /* TIMESTAMP_ECHO */
static int			n_nacks;	/* nacks in */
static int			n_acks;		/* acks in */
static int			n_timeouts;	
static int			n_retransmits;
static int			n_rsrs;		/* sent */
static int			n_rsr_buffers;	/* datagrams sent */
static int			n_rtt;
static double			ave_rtt_ticks;
static double			ave_rtt_usec;


#if !defined( RX_CLOSE_DEBUG_LEVEL )
#define RX_CLOSE_DEBUG_LEVEL 1
#endif
    
/* If connect() fails with ECONNREFUSED, try again this many times quickly */
#define CONNECT_BACKOFF_FAST_RETRY	5

/* After connect() fails CONNECT_BACKOFF_FAST_RETRY times, sleep for
 * this many microseconds between connect attempts */
#define CONNECT_BACKOFF_USEC		100000

#ifndef HAVE_THREAD_SAFE_SELECT
/* When a select fails in the handler thread (and the handler
 * thread cannot call a blocking select), then sleep this long
 * between attempts.
 */
#define POLLING_SELECT_USLEEP_TIME	100000
#endif



/* Allow the select() operation to execute this many times before
 * giving up on an ACK and retransmitting a lost datagram */
#define RX_ROUND_TRIP_TIMEOUT_TICKS	200

/* Save at least this many file descriptors for the user to use */
#define MIN_SAVE_FDS		3

/* Save this many files descriptors for the user by default */
#define DEFAULT_SAVE_FDS	10

/*
 * Other useful defines
 */
#define INCOMING_RECV_CLOSED           -1
#define INCOMING_RECV_DISPATCH		0
#define INCOMING_RECV_HEADER		1
#define INCOMING_RECV_HANDLER		2
#define INCOMING_RECV_BODY		3
#define INCOMING_RECV_DISCARD		4
#define INCOMING_RECV_FRAG		5
#define INCOMING_RECV_ENDFRAG		6

#define BLOCKING			NEXUS_TRUE
#define NONBLOCKING			NEXUS_FALSE

#define HANDLE_MESSAGES			1
#define ENQUEUE_MESSAGES		2

#define POLL_ONE_SELECT			1
#define POLL_UNTIL_SELECT_FAILS		2
#define POLL_BLOCKING_UNTIL_RECEIVE	3
#define POLL_FROM_HANDLER_THREAD	4
	
#define RSR_OPEN_TYPE			0
#define RSR_OPENFRAG_TYPE		1
#define RSR_FRAG_TYPE			2
#define RSR_ENDFRAG_TYPE		3
#define CLOSE_NORMAL_TYPE		4
#define CLOSE_ABNORMAL_TYPE		5
#define CLOSE_SHUTDOWN_TYPE		6
#define	ACK_TYPE			7
#define NACK_TYPE			8
#define REQ_TO_CLOSE_NORMAL_TYPE	9  /* For incomings to request a */
#define REQ_TO_CLOSE_SHUTDOWN_TYPE	10 /* close or shutdown. 	 */

#ifdef TIMESTAMP_ECHO
#define TIMESTAMP_ECHO_REQUEST_TYPE	11
#define TIMESTAMP_ECHO_RESPONSE_TYPE	12
#endif /* TIMESTAMP_ECHO */


/*
 * ROUND_MALLOC
 *
 * When a message buffer is allocated, the size of the header information
 * at the head of the buffer is rounded up to ROUND_MALLOC bytes.  Further,
 * the user's part of that buffer is kept on this boundary as well.
 * This is done so that the user's portion of the buffer can be kept
 * word aligned, which could potentially speed copy operations.
 *
 * If ROUND_MALLOC==0, then the header size will not be rounded up.
 */
#if defined(TARGET_ARCH_CRAYC90) || defined(TARGET_ARCH_AXP)
#define ROUND_MALLOC	8
#else
#define ROUND_MALLOC	4
#endif


/*
 * RX_MSG_HEADER_INTS
 * RX_MSG_HEADER_SIZE
 *
 * When sending a message, we send a header which contains
 * unsigned longs.
 *
 *   0) Message size
 *   1) Type Field (replaces single byte flag)
 *   2) Sequence Number (used for tracking ACKs)
 *   3) Message is in XDR format
 *   4) Context in the gp
 *   5) Address in the gp
 *   6) Handler id
 *   7) Length of the handler name string
 *   8) node_id in the gp    (#ifdef BUILD_PROFILE)
 *   9) context_id in the gp (#ifdef BUILD_PROFILE)
 *
 * Cray note: To have the Cray interoperate with other machines, we
 * need to pack the unsigned longs into 4 bytes instead of 8.
 * But in a Cray only version it might be more efficient to
 * just send 8 byte unsigned longs.  Aside: Crays don't have RX support
 * yet!
 *
 * RX_MSG_HEADER_INTS is the number of ints in the header.
 * RX_MSG_HEADER_SIZE is the size of the header in bytes.
 */
#ifdef BUILD_PROFILE
#define RX_MSG_HEADER_INTS		10
#else
#define RX_MSG_HEADER_INTS		8
#endif

/*
 * A close message looks like this.  It must match the
 * first three fields of all messages.
 *
 * 0) Message size
 * 2) Type Field (replaces single byte flag)
 * 3) Sequence Number  (used for tracking ACKs)
 */
#define CLOSE_MSG_INTS			3

/*
 * RX_DISPATCH_SIZE is the smallest chunk of data to read
 * out of a message.  It is used to decide what to do next.
 */
#define RX_DISPATCH_INTS		CLOSE_MSG_INTS

#ifdef TIMESTAMP_ECHO
#define TIMESTAMP_MSG_INTS		3
#endif /* TIMESTAMP_ECHO */

#if defined(TARGET_ARCH_CRAYC90) || defined(TARGET_ARCH_AXP)
#define RX_MSG_HEADER_SIZE		(RX_MSG_HEADER_INTS * 8)
#define CLOSE_MESSAGE_SIZE		(CLOSE_MSG_INTS * 8)
#define RX_DISPATCH_SIZE		(RX_DISPATCH_INTS * 8)

#ifdef TIMESTAMP_ECHO
#define TIMSTAMP_MSG_SIZE		(TIMESTAMP_MSG_INTS * 8)
#endif /* TIMESTAMP_ECHO */

#else  /* TARGET_ARCH_CRAYC90 */
#define RX_MSG_HEADER_SIZE		(RX_MSG_HEADER_INTS * 4)
#define CLOSE_MESSAGE_SIZE		(CLOSE_MSG_INTS * 4)
#define RX_DISPATCH_SIZE		(RX_DISPATCH_INTS * 4)

#ifdef TIMESTAMP_ECHO
#define TIMESTAMP_MSG_SIZE		(TIMESTAMP_MSG_INTS * 4)
#endif /* TIMESTAMP_ECHO */

#endif /* TARGET_ARCH_CRAYC90 */

/*
 * Some forward typedef declarations...
 */
typedef struct _rx_buffer_t	rx_buffer_t;
typedef struct _rx_proto_t	rx_proto_t;
typedef struct _rx_incoming_t	rx_incoming_t;
typedef struct _rx_stats_t	rx_stats_t;

/*
 * rx_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * rx specific information to that structure.
 */
struct _rx_buffer_t
{
#ifdef BUILD_DEBUG
    int				magic;
#endif
    nexus_buffer_funcs_t *	funcs;
    rx_adaptor_funcs_t *	adaptor_funcs;
    rx_buffer_t *		next;
    int				buffer_type; /* SEND_BUFFER || RECV_BUFFER */
    char *			storage;
    XDR				xdrs;
    int				use_xdr;
    int				msg_size;
    int				sequence_number;
    union
    {
	struct /* send */
	{
	    nexus_global_pointer_t *	gp;
	    int				handler_id;
	    char *			handler_name;
	    unsigned long		handler_name_length;
	    unsigned long		header_size;
	    unsigned long		size;
	    int				n_elements;
	    char *			header_pointer;
	    char *			name_pointer;
	    char *			base_pointer;
	    char *			current_pointer;
	} send;
	struct /* recv */
	{
	    int				handler_id;
	    char *			handler_name;
	    unsigned long		handler_name_length;
	    nexus_handler_type_t	handler_type;
	    nexus_handler_func_t	handler_func;
	    unsigned long		context;
	    unsigned long		address;
#ifdef BUILD_PROFILE
	    int				node_id;
	    int				context_id;
#endif /* BUILD_PROFILE */    
	    nexus_bool_t		stashed;
	    unsigned long		size;
	    char *			current_pointer;
	} recv;
    } u;
};

/*
 * An rx_buffer_t free list, to avoid malloc calls on the
 * main body of a message buffer.
 */
static rx_buffer_t *buffer_free_list = (rx_buffer_t *) NULL;

static nexus_mutex_t	buffer_free_list_mutex;
#define lock_buffer_free_list()   nexus_mutex_lock(&buffer_free_list_mutex)
#define unlock_buffer_free_list() nexus_mutex_unlock(&buffer_free_list_mutex)

#define SEND_BUFFER	1
#define RECV_BUFFER	2

#ifdef BUILD_DEBUG
#define MallocRxBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, rx_buffer_t *, \
		    sizeof(struct _rx_buffer_t)); \
	Buf->magic = NEXUS_BUFFER_MAGIC; \
    }
#else  /* BUILD_DEBUG */
#define MallocRxBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, rx_buffer_t *, \
		    sizeof(struct _rx_buffer_t)); \
    }
#endif /* BUILD_DEBUG */

#define GetRxBuffer(Routine, Buf ) \
    lock_buffer_free_list(); \
    if (buffer_free_list) \
    { \
	Buf = buffer_free_list; \
	buffer_free_list = buffer_free_list->next; \
	unlock_buffer_free_list(); \
    } \
    else \
    { \
	unlock_buffer_free_list(); \
	MallocRxBuffer(Routine, Buf); \
    } \

#define FreeRxBuffer(Buf) \
{ \
    if ((Buf)->storage != (char *) NULL) \
	NexusFree((Buf)->storage); \
    lock_buffer_free_list(); \
    (Buf)->next = buffer_free_list; \
    buffer_free_list = (Buf); \
    unlock_buffer_free_list(); \
}

#define SaveRxBuffer(Proto, Buf, Force_timeout_reset) \
    (Buf)->next = NULL; \
    if ((Proto)->ack_list_head == NULL) \
    { \
	(Proto)->ack_list_head = (Buf); \
	(Proto)->ack_list_tail = (Buf); \
	(Proto)->timeout_list_next = timeout_list; \
	timeout_list = (Proto); \
	(Proto)->timeout_value = (Proto)->timeout_reset; \
    } \
    else \
    { \
	if (Force_timeout_reset) \
		(Proto)->timeout_value = (Proto)->timeout_reset; \
	(Proto)->ack_list_tail->next = (Buf); \
	(Proto)->ack_list_tail = (Buf); \
    } 

#define SaveRxBufferWithIncoming(Incoming, Buf) \
{ \
    if ((Incoming)->ack_buf == NULL) \
    { \
	(Buf)->next = NULL ; \
	(Incoming)->ack_buf = (Buf); \
	(Incoming)->timeout_list_next = incoming_timeout_list; \
	incoming_timeout_list = (Incoming); \
	(Incoming)->timeout_value = (Incoming)->timeout_reset; \
    } \
    else \
    { \
	nexus_fatal("SaveRxBufferWithIncoming(): multiple saves\n"); \
    } \
}

#ifdef TARGET_ARCH_HPUX
#define HP_HACK_CAST (int)
#else
#define HP_HACK_CAST
#endif
#define SetBufferPointers(Buf) \
{ \
    (Buf)->u.send.base_pointer \
	= ((Buf)->storage + (Buf)->u.send.header_size); \
    (Buf)->u.send.name_pointer \
	= ((Buf)->u.send.base_pointer \
	   - HP_HACK_CAST (Buf)->u.send.handler_name_length); \
    (Buf)->u.send.header_pointer \
	= ((Buf)->u.send.name_pointer - RX_MSG_HEADER_SIZE); \
}

#ifdef TIMESTAMP_ECHO
struct timestamp 
{
    struct timeval 	i0;
    unsigned int 	ticks0;
};
#endif /* TIMESTAMP_ECHO */

struct _rx_suspended_rsr_t
{
    rx_proto_t *	proto;
    rx_buffer_t *	buf;
    rx_buffer_t *	buf2;
    int			write_len;		
    int			n_left;		/* bytes remaining to send */
    int			next_frag_delta;
    char *		next_frag_ptr;  /* position of next fragment in buf*/ 
    char *		send_ptr;
    int			force_timeout_reset;
    int			rsr_type;
};

typedef struct _rx_suspended_rsr_t rx_suspended_rsr_t;


/*
 * rx_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * rx specific information to that structure.
 */
struct _rx_proto_t
{
    nexus_proto_type_t	type;		/* NEXUS_PROTO_TYPE_RX */
    nexus_proto_funcs_t *funcs;
    rx_adaptor_funcs_t *adaptor_funcs;	/* FORE API, UDP or MN_UDP adaptor */
    char *		host;
    int			port;
    int			fd;
    int			reference_count;
    nexus_bool_t	being_closed;
    nexus_bool_t	send_in_progress;
    int			sequence_number;
    rx_buffer_t *	ack_list_head;
    rx_buffer_t *	ack_list_tail;
    rx_proto_t *	timeout_list_next;
    int			timeout_reset;
    int			timeout_value;
#ifdef TIMESTAMP_ECHO
    struct timestamp *	timestamp;
#endif /* TIMESTAMP_ECHO */
    int			sends_waiting;
    nexus_cond_t	sends_waiting_cond;
    rx_suspended_rsr_t  *suspended;
    int			last_retransmit_sequence_number;
};


/*
 * rx_incoming_t
 *
 * One of these is allocated for each incoming file descriptor.
 * It stores partial incoming messages until they are complete
 * and can be handled or queued.
 *
 * The 'type' field should be in the same location as the 'type'
 * field in a rx_proto_data_t, so that rx_proto_data_t's and
 * rx_incoming_t's can be distinguished from each other.
 *
 * The 'header_buf' will be used to temporarily store the header
 * information, and the handler name.
 */
#define INCOMING_TYPE NEXUS_PROTO_TYPE_INTERNAL1

#if NEXUS_MAX_HANDLER_NAME_LENGTH > RX_MSG_HEADER_SIZE
#define HEADER_BUF_LENGTH NEXUS_MAX_HANDLER_NAME_LENGTH
#else
#define HEADER_BUF_LENGTH RX_MSG_HEADER_SIZE
#endif

struct _rx_incoming_t
{
    nexus_proto_type_t	type;	/* INCOMING_TYPE */
    int			fd;
    rx_adaptor_funcs_t *adaptor_funcs;
    int			sequence_number;
    rx_buffer_t *	ack_buf;
    nexus_bool_t	supress_nacks;
    nexus_bool_t	being_closed;
    rx_incoming_t *	timeout_list_next;
    int			timeout_reset;
    int			timeout_value;
    rx_incoming_t *	next_free;

#ifdef BUILD_DEBUG
    int			connected_pid;
#endif /* BUILD_DEBUG */

    int			recv_state;	
    rx_buffer_t *	recv_buf;	/* Buffer we're receiving into */
    char *		recv_buf_current;
    int			recv_n_left;
    char		header_buf[HEADER_BUF_LENGTH];
};

static rx_incoming_t *incoming_free_list = (rx_incoming_t *) NULL;

#define ResetIncoming(Incoming) \
{ \
    (Incoming)->recv_state = INCOMING_RECV_DISPATCH; \
    (Incoming)->recv_buf = (rx_buffer_t *) NULL; \
    (Incoming)->recv_buf_current = &(Incoming->header_buf[0]); \
    (Incoming)->recv_n_left = RX_DISPATCH_SIZE; \
}

/*
 * This is a message queue containing all messages that have
 * arrived and are waiting to be handled.
 */
static rx_buffer_t *	message_queue_head;
static rx_buffer_t *	message_queue_tail;

#define EnqueueMessage(buf) \
{ \
    if (message_queue_head == (rx_buffer_t *) NULL) \
    { \
	message_queue_head = message_queue_tail = buf; \
    } \
    else \
    { \
	message_queue_tail->next = buf; \
	message_queue_tail = buf; \
    } \
    buf->next = (rx_buffer_t *) NULL; \
}

#define DequeueMessage(buf) \
{ \
    /* Assume that message_queue_head != NULL */ \
    buf = message_queue_head; \
    message_queue_head = message_queue_head->next; \
}

#define MessagesEnqueued() (message_queue_head != (rx_buffer_t *) NULL)

#define TestAndHandleMessages() \
    if (handle_or_enqueue == HANDLE_MESSAGES && MessagesEnqueued()) \
    { \
	handle_enqueued_messages(); \
    }

struct _rx_stats_t
{
    int n_nacks;
    int n_acks;
    int n_timeouts;
    int n_retransmits;
    int n_rsrs;
    int n_rsr_buffers;
    int	n_rtt;
    double ave_rtt_ticks;
    double ave_rtt_usec;
};

/*
 * List of rx_proto_t's with outstanding
 * buffers on their ack_list.
 */

static rx_proto_t *timeout_list = NULL;
static rx_incoming_t *incoming_timeout_list = NULL;

/*
 * My local RX connection information
 */
static int	rx_local_port;
static int	rx_local_fd;
static int	rx_local_pid;
static char	rx_local_host[MAXHOSTNAMELEN];


/*
 * Stuff for keeping track of file descriptors.
 *   fd_tablesize:	Maximum number of fds that system supports
 *   max_fds_to_use:	Maximum number of fds that we will open at once
 *   n_fds_in_use:	Number of fds that we are currently using
 *   fd_to_proto:	Array of pointers to rx_proto_data_t's or
 *				rx_incoming_t's.  This is used
 *				to map from an fd to the appropriate
 *				proto or incoming data structure.
 *   current_fd_set:	The current fd_set that should be used for select()
 *   next_fd_to_close:  If we run out of fd, start looking for an fd
 *				to close starting at this fd.
 *   modified_fd_table:	Set to NEXUS_TRUE whenever the fd table
 *				changes.  This signals the
 *				handler thread which is blocked in
 *				a select that the fd_table is
 *				different than when it was called,
 *				and therefore should not be trusted.
 */
static int fd_tablesize;
static int max_fds_to_use;
static int n_fds_in_use;
static void **fd_to_proto;
static fd_set current_fd_set;
static fd_set current_write_fd_set;
static int next_fd_to_close;

#ifdef HAVE_THREAD_SAFE_SELECT
static nexus_bool_t modified_fd_table;
#endif

#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_HPUX)
#define NUM_FDS sysconf(_SC_OPEN_MAX)
#else
/* This works for BSD machines */
#define NUM_FDS getdtablesize()
#endif



/* XXX need to work out a way of closing rx fd's */
/*
 * When rx_destroy_proto() decrements proto->reference_count to 0, it
 * tries to close proto->fd by sending a close message down the fd
 * and then closing the fd.  But its possible that it cannot close
 * the proto because the socket buffer was full.
 * In this case, it bumps n_deferred_closes
 * so that we can try to close again later in make_room_for_new_fd().
 *
 * Do a deferred closed proto is one with:
 *     (proto->reference_count == 0) && (proto->fd >= 0)
 */
static int n_deferred_closes = 0;


/*
 * Protocol table stuff.
 *
 * The protocol table is hashed on host/port. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple rx_proto_t
 * objects to the same context.  Multiple global pointers to the same
 * context share a rx_proto_t.
 */
typedef struct _proto_table_entry_t
{
    rx_proto_t *proto;
    struct _proto_table_entry_t *next;
} proto_table_entry_t;

#define PROTO_TABLE_SIZE 149

struct _proto_table_entry_t	proto_table[PROTO_TABLE_SIZE];

static void			proto_table_init(void);
static int			proto_table_hash(char *host, int port);
static void			proto_table_insert(rx_proto_t *proto);
static void 			proto_table_delete(rx_proto_t *proto);
static rx_proto_t *		proto_table_lookup(char *host, int port);

#ifdef BUILD_DEBUG
static char tmpbuf1[1024];
static char tmpbuf2[1024];
#endif


/*
 * Various forward declarations of procedures
 */
static nexus_proto_type_t rx_proto_type(void);
static void		rx_init(int *argc, char ***argv);
static void		rx_shutdown(nexus_bool_t shutdown_others);
static void		rx_abort(void);
static nexus_bool_t	rx_poll(void);
static void		rx_blocking_poll(void);

static void		rx_init_remote_service_request(
					    nexus_buffer_t *buffer,
					    nexus_global_pointer_t *gp,
					    char *handler_name,
					    int handler_id);
static int		rx_send_remote_service_request(
						nexus_buffer_t *buffer);
static void		suspend_remote_service_request();
static void		continue_remote_service_request(rx_proto_t *proto);
static int		rx_send_urgent_remote_service_request(
						nexus_buffer_t *buffer);
static void		rx_destroy_proto(nexus_proto_t *nexus_proto);
static void             rx_copy_proto(nexus_proto_t *proto_in,
				       nexus_proto_t **proto_out);
static nexus_mi_proto_t* rx_get_my_mi_proto(void);
static nexus_bool_t	rx_construct_from_mi_proto(nexus_proto_t **proto,
						  nexus_mi_proto_t *mi_proto);
static void		rx_get_creator_proto_params(char *buf,
						     int buf_size,
						     char *node_name,
						     int node_number);
static void		rx_construct_creator_proto(nexus_proto_t **proto);
static int		rx_compare_protos(nexus_proto_t *proto1,
					   nexus_proto_t *proto2);


static rx_proto_t *	construct_proto(char *host, int port, int type );
static void		open_proto(rx_proto_t *proto);
static void		close_proto(rx_proto_t *proto);

static rx_incoming_t *	construct_incoming(int fd, 
					   rx_adaptor_funcs_t *adaptor_funcs);
static void		close_incoming(rx_incoming_t *incoming);

#ifdef DONT_INCLUDE
/*static nexus_bool_t	close_incoming_with_message(rx_incoming_t *incoming,
						    nexus_bool_t read_messages);*/
#endif

static nexus_bool_t	close_proto_with_message(rx_proto_t *proto);

#ifdef DONT_INCLUDE
/*static void		check_proto_for_close(rx_proto_t *proto);*/
#endif

static void		add_proto_with_fd(void *proto, int fd);
static void		remove_proto_with_fd(void *proto, int fd);
static void		make_room_for_new_fd(void);

static nexus_bool_t	select_and_read(int poll_type, int handle_or_enqueue);
static void		read_incoming(rx_incoming_t *incoming,
				      int handle_or_enqueue);
static void		retransmit_buffers(rx_buffer_t *buf, int fd,
					  int next_sequence_number,
					  int *last_transmit_sequence_number);

#ifdef TIMESTAMP_ECHO
static void		timestamp_send(int fd, char *buf, int msg_size, 
				       char *caller);
static void		timestamp_echo( rx_proto_t *proto );
static void		timestamp_reply( int fd );
static void		timestamp_compute_round_trip_time(rx_proto_t *proto);
void timestamp_external_echo( nexus_global_pointer_t *gp);
#endif /* TIMESTAMP_ECHO */
static void 		cleanup_list(rx_proto_t *proto, int sequence_number);
static void		read_proto(rx_proto_t *proto);
static void 		generate_ack(rx_incoming_t *incoming, 
				     int sequence_number, 
				     int ack_type);
static void		handle_enqueued_messages(void);

static void             md_unix_get_host_by_address(struct sockaddr_in from, 
						    struct hostent *hp);
static void		accept_new_connection(int fd);
/*static void		config_socket(int s);*/
static void		set_nonblocking(int fd);
static int		do_connect(char *host, int port, 
				   rx_adaptor_funcs_t *funcs);
static int		setup_listener(int *port);


static rx_adaptor_funcs_t *rx_type_to_funcs( int type );
#ifdef BUILD_DEBUG
static void		print_fd_set(char *fmt_string, fd_set *set);
#endif /* BUILD_DEBUG */

static void		rx_set_buffer_size(nexus_buffer_t *buffer,
					    int size, int n_elements);
static int		rx_check_buffer_size(nexus_buffer_t *buffer,
					      int slack, int increment);
static void		rx_free_buffer(nexus_buffer_t *buffer);
static void		rx_stash_buffer(nexus_buffer_t *buffer,
				       nexus_stashed_buffer_t *stashed_buffer);
static void		rx_free_stashed_buffer(
				       nexus_stashed_buffer_t *stashed_buffer);


/* PRINTING_OFF */
static int	rx_sizeof_float	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_double	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_short	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_u_short	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_int		(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_u_int	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_long		(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_u_long	(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_char		(nexus_buffer_t *buffer, int count);
static int	rx_sizeof_u_char	(nexus_buffer_t *buffer, int count);


static void	rx_put_float	(nexus_buffer_t *buffer,
				 float *data, int count);
static void	rx_put_double	(nexus_buffer_t *buffer,
				 double *data, int count);
static void	rx_put_short	(nexus_buffer_t *buffer,
				 short *data, int count);
static void	rx_put_u_short	(nexus_buffer_t *buffer,
				 unsigned short *data, int count);
static void	rx_put_int	(nexus_buffer_t *buffer,
				 int *data, int count);
static void	rx_put_u_int	(nexus_buffer_t *buffer,
				 unsigned int *data, int count);
static void	rx_put_long	(nexus_buffer_t *buffer,
				 long *data, int count);
static void	rx_put_u_long	(nexus_buffer_t *buffer,
				 unsigned long *data, int count);
static void	rx_put_char	(nexus_buffer_t *buffer,
				 char *data, int count);
static void	rx_put_u_char	(nexus_buffer_t *buffer,
				 unsigned char *data, int count);

static void	rx_get_float	(nexus_buffer_t *buffer,
				 float *dest, int count);
static void	rx_get_double	(nexus_buffer_t *buffer,
				 double *dest, int count);
static void	rx_get_short	(nexus_buffer_t *buffer,
				 short *dest, int count);
static void	rx_get_u_short	(nexus_buffer_t *buffer,
				 unsigned short *dest, int count);
static void	rx_get_int	(nexus_buffer_t *buffer,
				 int *dest, int count);
static void	rx_get_u_int	(nexus_buffer_t *buffer,
				 unsigned int *dest, int count);
static void	rx_get_long	(nexus_buffer_t *buffer,
				 long *dest, int count);
static void	rx_get_u_long	(nexus_buffer_t *buffer,
				 unsigned long *dest, int count);
static void	rx_get_char	(nexus_buffer_t *buffer,
				 char *dest, int count);
static void	rx_get_u_char	(nexus_buffer_t *buffer,
				 unsigned char *dest, int count);

static void	rx_get_stashed_float	(nexus_stashed_buffer_t *buffer,
					 float *dest, int count);
static void	rx_get_stashed_double	(nexus_stashed_buffer_t *buffer,
					 double *dest, int count);
static void	rx_get_stashed_short	(nexus_stashed_buffer_t *buffer,
					 short *dest, int count);
static void	rx_get_stashed_u_short	(nexus_stashed_buffer_t *buffer,
					 unsigned short *dest, int count);
static void	rx_get_stashed_int	(nexus_stashed_buffer_t *buffer,
					 int *dest, int count);
static void	rx_get_stashed_u_int	(nexus_stashed_buffer_t *buffer,
					 unsigned int *dest, int count);
static void	rx_get_stashed_long	(nexus_stashed_buffer_t *buffer,
					 long *dest, int count);
static void	rx_get_stashed_u_long	(nexus_stashed_buffer_t *buffer,
					 unsigned long *dest, int count);
static void	rx_get_stashed_char	(nexus_stashed_buffer_t *buffer,
					 char *dest, int count);
static void	rx_get_stashed_u_char	(nexus_stashed_buffer_t *buffer,
					 unsigned char *dest, int count);

static void		rx_xdr_set_buffer_size(nexus_buffer_t *buffer,
					    int size, int n_elements);
static int		rx_xdr_check_buffer_size(nexus_buffer_t *buffer,
					      int slack, int increment);
static void		rx_xdr_free_buffer(nexus_buffer_t *buffer);
static void		rx_xdr_stash_buffer(nexus_buffer_t *buffer,
				       nexus_stashed_buffer_t *stashed_buffer);
static void		rx_xdr_free_stashed_buffer(
				       nexus_stashed_buffer_t *stashed_buffer);

static int	rx_xdr_sizeof_float	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_double	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_short	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_u_short	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_int	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_u_int	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_long	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_u_long	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_char	(nexus_buffer_t *buffer, int count);
static int	rx_xdr_sizeof_u_char	(nexus_buffer_t *buffer, int count);


static void	rx_xdr_put_float	(nexus_buffer_t *buffer,
				 float *data, int count);
static void	rx_xdr_put_double	(nexus_buffer_t *buffer,
				 double *data, int count);
static void	rx_xdr_put_short	(nexus_buffer_t *buffer,
				 short *data, int count);
static void	rx_xdr_put_u_short	(nexus_buffer_t *buffer,
				 unsigned short *data, int count);
static void	rx_xdr_put_int		(nexus_buffer_t *buffer,
				 int *data, int count);
static void	rx_xdr_put_u_int	(nexus_buffer_t *buffer,
				 unsigned int *data, int count);
static void	rx_xdr_put_long	(nexus_buffer_t *buffer,
				 long *data, int count);
static void	rx_xdr_put_u_long	(nexus_buffer_t *buffer,
				 unsigned long *data, int count);
static void	rx_xdr_put_char	(nexus_buffer_t *buffer,
				 char *data, int count);
static void	rx_xdr_put_u_char	(nexus_buffer_t *buffer,
				 unsigned char *data, int count);

static void	rx_xdr_get_float	(nexus_buffer_t *buffer,
				 float *dest, int count);
static void	rx_xdr_get_double	(nexus_buffer_t *buffer,
				 double *dest, int count);
static void	rx_xdr_get_short	(nexus_buffer_t *buffer,
				 short *dest, int count);
static void	rx_xdr_get_u_short	(nexus_buffer_t *buffer,
				 unsigned short *dest, int count);
static void	rx_xdr_get_int		(nexus_buffer_t *buffer,
				 int *dest, int count);
static void	rx_xdr_get_u_int	(nexus_buffer_t *buffer,
				 unsigned int *dest, int count);
static void	rx_xdr_get_long	(nexus_buffer_t *buffer,
				 long *dest, int count);
static void	rx_xdr_get_u_long	(nexus_buffer_t *buffer,
				 unsigned long *dest, int count);
static void	rx_xdr_get_char	(nexus_buffer_t *buffer,
				 char *dest, int count);
static void	rx_xdr_get_u_char	(nexus_buffer_t *buffer,
				 unsigned char *dest, int count);

static void	rx_xdr_get_stashed_float	(nexus_stashed_buffer_t *buffer,
					 float *dest, int count);
static void	rx_xdr_get_stashed_double	(nexus_stashed_buffer_t *buffer,
					 double *dest, int count);
static void	rx_xdr_get_stashed_short	(nexus_stashed_buffer_t *buffer,
					 short *dest, int count);
static void	rx_xdr_get_stashed_u_short	(nexus_stashed_buffer_t *buffer,
					 unsigned short *dest, int count);
static void	rx_xdr_get_stashed_int		(nexus_stashed_buffer_t *buffer,
					 int *dest, int count);
static void	rx_xdr_get_stashed_u_int	(nexus_stashed_buffer_t *buffer,
					 unsigned int *dest, int count);
static void	rx_xdr_get_stashed_long	(nexus_stashed_buffer_t *buffer,
					 long *dest, int count);
static void	rx_xdr_get_stashed_u_long	(nexus_stashed_buffer_t *buffer,
					 unsigned long *dest, int count);
static void	rx_xdr_get_stashed_char	(nexus_stashed_buffer_t *buffer,
					 char *dest, int count);
static void	rx_xdr_get_stashed_u_char	(nexus_stashed_buffer_t *buffer,
					 unsigned char *dest, int count);
/* PRINTING_ON */

static nexus_proto_funcs_t rx_proto_funcs =
{
    rx_proto_type,
    rx_init,
    rx_shutdown,
    rx_abort,
    rx_poll,
    rx_blocking_poll,
    rx_init_remote_service_request,
    rx_destroy_proto,
    rx_copy_proto,
    rx_get_my_mi_proto,
    rx_construct_from_mi_proto,
    rx_get_creator_proto_params,
    rx_construct_creator_proto,
    rx_compare_protos,
    NULL /* rx_test_proto */,
};

static nexus_buffer_funcs_t rx_buffer_funcs =
{
    rx_set_buffer_size,
    rx_check_buffer_size,
    rx_send_remote_service_request,
    rx_send_urgent_remote_service_request,
    rx_free_buffer,
    rx_stash_buffer,
    rx_free_stashed_buffer,
/* PRINTING_OFF */
    rx_sizeof_float,
    rx_sizeof_double,
    rx_sizeof_short,
    rx_sizeof_u_short,
    rx_sizeof_int,
    rx_sizeof_u_int,
    rx_sizeof_long,
    rx_sizeof_u_long,
    rx_sizeof_char,
    rx_sizeof_u_char,
    rx_put_float,
    rx_put_double,
    rx_put_short,
    rx_put_u_short,
    rx_put_int,
    rx_put_u_int,
    rx_put_long,
    rx_put_u_long,
    rx_put_char,
    rx_put_u_char,
    rx_get_float,
    rx_get_double,
    rx_get_short,
    rx_get_u_short,
    rx_get_int,
    rx_get_u_int,
    rx_get_long,
    rx_get_u_long,
    rx_get_char,
    rx_get_u_char,
    rx_get_stashed_float,
    rx_get_stashed_double,
    rx_get_stashed_short,
    rx_get_stashed_u_short,
    rx_get_stashed_int,
    rx_get_stashed_u_int,
    rx_get_stashed_long,
    rx_get_stashed_u_long,
    rx_get_stashed_char,
    rx_get_stashed_u_char,
/* PRINTING_ON */
};

static nexus_buffer_funcs_t rx_xdr_buffer_funcs =
{
    rx_xdr_set_buffer_size,
    rx_xdr_check_buffer_size,
    rx_send_remote_service_request,
    rx_send_urgent_remote_service_request,
    rx_xdr_free_buffer,
    rx_xdr_stash_buffer,
    rx_xdr_free_stashed_buffer,
/* PRINTING_OFF */
    rx_xdr_sizeof_float,
    rx_xdr_sizeof_double,
    rx_xdr_sizeof_short,
    rx_xdr_sizeof_u_short,
    rx_xdr_sizeof_int,
    rx_xdr_sizeof_u_int,
    rx_xdr_sizeof_long,
    rx_xdr_sizeof_u_long,
    rx_xdr_sizeof_char,
    rx_xdr_sizeof_u_char,
    rx_xdr_put_float,
    rx_xdr_put_double,
    rx_xdr_put_short,
    rx_xdr_put_u_short,
    rx_xdr_put_int,
    rx_xdr_put_u_int,
    rx_xdr_put_long,
    rx_xdr_put_u_long,
    rx_xdr_put_char,
    rx_xdr_put_u_char,
    rx_xdr_get_float,
    rx_xdr_get_double,
    rx_xdr_get_short,
    rx_xdr_get_u_short,
    rx_xdr_get_int,
    rx_xdr_get_u_int,
    rx_xdr_get_long,
    rx_xdr_get_u_long,
    rx_xdr_get_char,
    rx_xdr_get_u_char,
    rx_xdr_get_stashed_float,
    rx_xdr_get_stashed_double,
    rx_xdr_get_stashed_short,
    rx_xdr_get_stashed_u_short,
    rx_xdr_get_stashed_int,
    rx_xdr_get_stashed_u_int,
    rx_xdr_get_stashed_long,
    rx_xdr_get_stashed_u_long,
    rx_xdr_get_stashed_char,
    rx_xdr_get_stashed_u_char,
/* PRINTING_ON */
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
void *RX_PROTOCOL_INFO(void)
{
    return((void *) (&rx_proto_funcs));
} /* _nx_pr_*_info() */


/*
 * rx_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t rx_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_RX);
} /* rx_proto_type() */


#ifdef HAVE_THREAD_SAFE_SELECT
/*
 * rx_handler_thread()
 *
 * In the multi-threaded version, this is the entry point
 * for the handler thread.
 */
static void *rx_handler_thread(void *arg)
{
    nexus_bool_t message_handled;

    _nx_set_i_am_rx_handler_thread();
    
    rx_enter();
    message_handled = select_and_read(POLL_FROM_HANDLER_THREAD, 
				      HANDLE_MESSAGES);
    rx_exit();

    nexus_mutex_lock(&handler_thread_done_mutex);
    handler_thread_done = NEXUS_TRUE;
    nexus_cond_signal(&handler_thread_done_cond);
    nexus_mutex_unlock(&handler_thread_done_mutex);
    
    return ((void *) NULL);
} /* rx_handler_thread() */
#endif /* HAVE_THREAD_SAFE_SELECT */


/*
 * rx_init()
 *
 * Initialize the RX protocol.
 */
static void rx_init(int *argc, char ***argv)
{
    int i;
    int save_fds;
    int arg_num;

    /* suppress warnings */
    int nexus_force_lint = _nexus_force_resolution();
    char *rcsid_lint = rcsid;

    nexus_force_lint = nexus_force_lint;
    rcsid_lint = rcsid_lint;

#ifdef NEXUS_RX_PROTO_ATM
    fore_atm_defeat_warnings();

    arg_atm_device = "/dev/fa0"; /* XXX hack -- fix with local query */
#endif /* NEXUS_RX_PROTO_ATM */

    default_adaptor_type = rx_default_adaptor_type();
    
    /*
     * Set up the listener for this process. The nexus_proto_t returned
     * refers to this tcp port.
     */
    rx_local_fd = setup_listener(&rx_local_port);
    if (rx_local_fd < 0)
	nexus_fatal("rx_init(): setup_listener failed\n");

    _nx_md_gethostname(rx_local_host, MAXHOSTNAMELEN);
    rx_local_pid = _nx_md_getpid();

    /*
     * Set up file descriptor tables
     */
    fd_tablesize = NUM_FDS;
    if ((arg_num = nexus_find_argument(argc, argv, "save_fds", 2)) >= 0)
    {
	save_fds = atoi((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	save_fds = DEFAULT_SAVE_FDS;
    }
    save_fds = NEXUS_MIN(NEXUS_MAX(save_fds, MIN_SAVE_FDS), fd_tablesize);
    max_fds_to_use = fd_tablesize - save_fds - 1; /* -1 for bootstrap_fd */
    n_fds_in_use = 0;
    NexusMalloc(rx_init(), fd_to_proto, void **,
		sizeof(void *) * fd_tablesize);
    for (i = 0; i < fd_tablesize; i++)
	fd_to_proto[i] = (void *) NULL;
    FD_ZERO(&current_fd_set);
    FD_ZERO(&current_write_fd_set);
    FD_SET(rx_local_fd, &current_fd_set);

    next_fd_to_close = 0;
    
    nexus_debug_printf(3, ("RX listening on %d\n", rx_local_port));

    nexus_thread_key_create(&i_am_rx_handler_thread_key, NULL);
    message_queue_head = message_queue_tail = (rx_buffer_t *) NULL;
    proto_table_init();
    nexus_mutex_init(&rx_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&rx_cond, (nexus_condattr_t *) NULL);
    rx_done = NEXUS_FALSE;
    handle_in_progress = NEXUS_FALSE;
    shutdown_in_progress = NEXUS_FALSE;
    nexus_mutex_init(&buffer_free_list_mutex, (nexus_mutexattr_t *) NULL);

    rx_adaptor_init();

    /* Initialize rx_stats */
    n_nacks = 0;
    n_acks = 0;
    n_timeouts = 0;
    n_retransmits = 0;
    n_rsrs = 0;
    n_rsr_buffers = 0;
    n_rtt = 0;
    ave_rtt_ticks = 0.0;
    ave_rtt_usec = 0.0;
    
#ifdef HAVE_THREAD_SAFE_SELECT
    if (nexus_preemptive_threads())
    {
        nexus_thread_t thread;
	using_rx_handler_thread = NEXUS_TRUE;

#if DONT_INCLUDE
	/*
	 * Create a pipe to myself, so that I can wake up the handler
	 * thread that is blocked on a select
	 */
	if (pipe(pipe_to_self) != 0)
	{
	    nexus_fatal("rx_init(): pipe() failed with errno=%d\n", errno);
	}
	n_fds_in_use += 2;
	FD_SET(pipe_to_self[0], &current_fd_set);
#endif

	/* NULL out the poll and blocking_poll entries in my function table */
	rx_proto_funcs.poll = NULL;
	rx_proto_funcs.blocking_poll = NULL;
      
	/* Create the handler thread */
	handler_thread_done = NEXUS_FALSE;
	nexus_mutex_init(&handler_thread_done_mutex, 
			 (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&handler_thread_done_cond, 
			(nexus_condattr_t *) NULL);
	nexus_thread_create(&thread,
			    (nexus_thread_attr_t *) NULL,
			    rx_handler_thread, 
			    (void *) NULL);
    }
    else
#endif
    {
	using_rx_handler_thread = NEXUS_FALSE;
    }
} /* rx_init() */


/*
 * rx_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 *
 * XXX Fix comment to describe RX
 *
 * Shutdown the rx protocol module by closing all of the
 * proto's that have open fd's.  This will result in an orderly
 * shutdown of fd's, so that other nodes will not detect an
 * abnormal eof on the other ends of these fd's.
 *
 * It is ok if this procedure takes a little while to complete -- for
 * example to wait for the remote processes to send ACKs back to
 * the read_proto() routine indicating that the close messages have
 * been received.
 *
 * An orderly closing of a connection is done as follows:
 *   1) Send the CLOSE_NORMAL_TYPE message down the fd,
 *      signaling a close due to a normal termination, 
 *	if shutdown_others==NEXUS_FALSE.  Otherwise send the
 *	CLOSE_SHUTDOWN_TYPE message.
 *   2) Mark this as a deferred close, and wait for the ACK message.
 *   3) When the ACK for the CLOSE arrives, close the fd and its 
 *      associated proto.
 *   4) CLOSEs are resent if we don't get an ACK.
 */

static void rx_shutdown(nexus_bool_t shutdown_others)
{
    int i;
    char *close_buf;
    int fd;
    rx_adaptor_funcs_t *adaptor_funcs;
    int n_sent;
    rx_incoming_t *incoming;
    rx_proto_t *proto;
    rx_buffer_t *buf;
    nexus_bool_t is_proto;
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    int msg_size, sequence_number;
    int close_type, request_close_type;
    nexus_bool_t i_am_rx_handler_thread;
    nexus_bool_t message_handled = NEXUS_FALSE;
    int save_error;

    /*
     * Check to ensure that rx_shutdown only happens once.
     */
    if (shutdown_in_progress == NEXUS_TRUE) {
      return;
    }

    rx_enter();

    shutdown_in_progress = NEXUS_TRUE;

    rx_done = NEXUS_TRUE; /* XXX Should this come later */

    if (shutdown_others)
    {
	close_type = CLOSE_SHUTDOWN_TYPE;
	request_close_type = REQ_TO_CLOSE_SHUTDOWN_TYPE;
    }
    else
    {
	close_type = CLOSE_NORMAL_TYPE;
	request_close_type = REQ_TO_CLOSE_NORMAL_TYPE;
    }
    msg_size = CLOSE_MESSAGE_SIZE;

    for (i = 0; i < fd_tablesize; i++)
    {
#ifdef BUILD_DEBUG
	if(NexusDebug(5))
	{
	    nexus_printf("rx_shutdown(): fd:%d proto:%x\n",
			 i, fd_to_proto[i]);
	}
#endif
	if (fd_to_proto[i] != (rx_proto_t *) NULL)
	{
	    if (((rx_proto_t *)(fd_to_proto[i]))->type == INCOMING_TYPE)
	    {
#if 0
	        continue;  /*XXX don't even try to close incomings */
#else
		incoming = (rx_incoming_t *) (fd_to_proto[i]);
		if (incoming->being_closed)
		  continue;
		fd = incoming->fd;
		adaptor_funcs = incoming->adaptor_funcs;
		sequence_number = 0; /* incomings don't normally send */
		is_proto = NEXUS_FALSE;
#endif
	    }
	    else
	    {
		proto = (rx_proto_t *) (fd_to_proto[i]);
		/* check to see if this proto is already being closed.
		 * If so, don't do anything
		 */
		if (proto->being_closed)
		  continue;
		nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("rx_shutdown: proto(%x)->being_closed = TRUE\n",proto));
		proto->being_closed = NEXUS_TRUE;
		fd = proto->fd;
		adaptor_funcs = proto->adaptor_funcs;
		sequence_number = proto->sequence_number++;
		/*proto->reference_count = 0;*/ /* Will doing this cause an error? XXX */
		n_deferred_closes++;
		is_proto = NEXUS_TRUE;
	    }
#ifdef BUILD_DEBUG
	    if(NexusDebug(5))
	    {
	        nexus_printf("rx_shutdown(): checking fd:%d\n",fd);
	    }
#endif
	    NexusMalloc(rx_shutdown(), close_buf, char *,
			msg_size );
	    GetRxBuffer(rx_shutdown(), buf);
	    buf->adaptor_funcs = adaptor_funcs;
	    buf->storage = close_buf;
	    buf->u.send.header_pointer = close_buf;
	    tmp_longs[0] = NX_HTONL(msg_size);
	    tmp_longs[1] = NX_HTONL((is_proto ? close_type : request_close_type));
	    tmp_longs[2] = NX_HTONL(sequence_number);
	    memcpy( close_buf, tmp_longs, msg_size );
	    buf->sequence_number = sequence_number;
	    buf->msg_size = msg_size;

	    tracepoint("rx_send shutdown");
	    n_sent = adaptor_funcs->send(fd, close_buf, msg_size);
	    save_error = errno;
#ifdef BUILD_DEBUG
	    if(NexusDebug(5))
	    {
	        nexus_printf("rx_shutdown(); on shut down fd:%d rx_send returned: %d\n",fd,n_sent);
	    }
#endif
	    if (n_sent != msg_size)
	    {
	        /* Something weird happened on the fd  */
#ifdef BUILD_DEBUG
	        if(NexusDebug(3))
	        {
		    nexus_printf("rx_shutdown(): trouble shutting down fd:%d, received errno:%d\n",fd,save_error);
		}
#endif
	    }
	    /* Wait for ACKs */
	    if (is_proto) 
	    {
	        SaveRxBuffer(proto, buf, NEXUS_FALSE); 
	    }
	    else
	    {
	        SaveRxBufferWithIncoming(incoming, buf); 
	    }
	}
    }

    if (using_rx_handler_thread)
    {
	_nx_i_am_rx_handler_thread(&i_am_rx_handler_thread);
	if (!i_am_rx_handler_thread)
	{
#if 0 /* XXX decide if this is right for pr_dx before using it */
	    /*
	     * Close the pipe_to_self to wakeup the handler thread
	     * from the select, and wait for the handler thread
	     * to shutdown.
	     */
	    close_flag = '\0';
	    while (   (write(pipe_to_self[1], &close_flag, 1) == -1)
		   && (errno == EINTR) )
		;
	    while (   (close(pipe_to_self[1]) == -1)
		   && (errno == EINTR) )
		;
#endif
	    /*
	     * XXX
	     * Wait for the handler thread to shutdown.
	     * select() uses timeouts on the block so that it can
	     * periodically check rx_done.
	     */
	    rx_exit();
	    nexus_mutex_lock(&handler_thread_done_mutex);
	    while (!handler_thread_done)
	    {
		nexus_cond_wait(&handler_thread_done_cond,
				&handler_thread_done_mutex);
	    }
	    nexus_mutex_unlock(&handler_thread_done_mutex);
	    rx_enter();
	}
	nexus_mutex_destroy(&handler_thread_done_mutex);
	nexus_cond_destroy(&handler_thread_done_cond);
    }
#if !defined(HAVE_THREAD_SAFE_SELECT)
    else
#endif	/* HAVE_THREAD_SAFE_SELECT */
    {
      /*
       * The global handler thread has already been shutdown,
       * so keep polling the rx protocol until all the fds are
       * closed.
       */

      while (n_fds_in_use > 0)
      {
	message_handled = select_and_read(POLL_UNTIL_SELECT_FAILS,
					  HANDLE_MESSAGES);
      }
    }
    close(rx_local_fd);
    rx_exit();
} /* rx_shutdown() */


/*
 * rx_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 *
 * XXX This isn't much different from rx_shutdown().  Do we need it?
 *
 * Shutdown the rx protocol module by closing all off of the
 * proto's that have open fd's.  This will hopefully result
 * in a reasonably  orderly shutdown of fd's, so that
 * other nodes do not spew error messages when they detect
 * closed sockets.
 *
 * However, this procedure should not wait to do things cleanly,
 * like rx_shutdown().  If it cannot close a fd cleanly (first
 * write a CLOSE_ABNORMAL_FLAG to it), then it should close the
 * fd anyway and get on with things.  The highest priority is
 * to just get shut down.
 *
 * The closing of a connection is done as follows:
 *   1) Send the CLOSE_ABNORMAL_FLAG message down the fd (1 byte,
 *      signaling a close due to a normal termination) down the fd.
 *	If the write would block, give up and go on to next step.
 *   2) Then close the fd.
 */
static void rx_abort(void)
{
    int i;
    char *close_buf;
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    int close_type, msg_size, sequence_number;
    rx_adaptor_funcs_t *adaptor_funcs;

    msg_size = CLOSE_MESSAGE_SIZE;
    close_type = CLOSE_ABNORMAL_TYPE;
    
    sequence_number = -1;

    NexusMalloc(rx_abort(), close_buf, char *, msg_size );
    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(close_type);
    tmp_longs[2] = NX_HTONL(sequence_number);
    memcpy( close_buf, tmp_longs, msg_size );

    for (i = 0; i < fd_tablesize; i++)
    {
	if (fd_to_proto[i] != (rx_proto_t *) NULL)
	{
	  if (((rx_proto_t *)(fd_to_proto[i]))->type == INCOMING_TYPE)
	  {
	    adaptor_funcs = ((rx_incoming_t *)(fd_to_proto[i]))->adaptor_funcs;
	  }
	  else
	  {
	    adaptor_funcs = ((rx_proto_t *)(fd_to_proto[i]))->adaptor_funcs;
	  }  

	  tracepoint("rx_send abort");
	  adaptor_funcs->send(i, close_buf, msg_size);
	  adaptor_funcs->close(i);
	}
    }

} /* rx_abort() */


/*
 * rx_poll()
 *
 * In a version of the rx protocol module that is NOT threaded,
 * all handlers are invoked from this routine.  So check all of the
 * incoming file descriptors for messages.  If there are any
 * messages, receive them and invoke the handler.
 *
 * If threads are supported, then rx_poll() can be a no-op, since
 * one or more threads can be created which just sit watching
 * file descriptors and invoking handlers as stuff comes in
 * those file descriptors.
 * 
 * XXX incorporate Tuecke's new handler stuff per email message
 */
static nexus_bool_t rx_poll(void)
{
    nexus_bool_t message_handled = NEXUS_FALSE;
    
    /*
     * This should not be called if a separate handler thread is in use.
     */
    NexusAssert2((!using_rx_handler_thread),
		 ("rx_poll(): Internal error: Should never be called when using a handler thread\n") );
    
    nexus_debug_printf(6, ("rx_poll(): entering\n"));

    rx_enter();
    if (!handle_in_progress)
    {
      message_handled = select_and_read(POLL_UNTIL_SELECT_FAILS, 
					HANDLE_MESSAGES);
    }
    rx_exit();

    /*
     * Only yield the processor if there was a message handled.
     * That handler may have enabled another thread for execution.
     */
    if (message_handled)
    {
	nexus_thread_yield();
    }
    
    nexus_debug_printf(6, ("rx_poll(): exiting\n"));

    return (message_handled);
} /* rx_poll() */

/*
 * rx_blocking_poll()
 *
 * XXX rx_proto uses the number of times through the select() loop
 * with 0 timeout as a sort of clock.  When enough ticks have gone
 * by, it knows to retransmit unack'ed data buffers.  So really, 
 * this routine can only block if the retransmission clock is done
 * some other way.  Signals can be used, or itimers, but I'm not sure
 * what will turn out to be better.
 *
 * In a version of the rx protocol module that does not
 * use a handler thread, this routine should do a blocking
 * select and invoke the handler for any received messages.
 * This is only called if this is the only protocol module
 * requiring polling, and all other threads are suspended.
 *
 * It is ok for this to return without actually handling
 * a message.
 *
 * Note: There is no need to call nexus_thread_yield() from this
 * routine like there is in rx_poll(), because this routine will only
 * be used from an idle or handler thread which will do a subsequent
 * nexus_thread_yield() anyway.
 */
static void rx_blocking_poll(void)
{
    nexus_bool_t message_handled = NEXUS_FALSE;

    /*
     * This should not be called if a separate handler thread is in use.
     */
    NexusAssert2((!using_rx_handler_thread),
		 ("rx_blocking_poll(): Internal error: Should never be called when using a handler thread\n") );
    
    nexus_debug_printf(3, ("rx_blocking_poll(): entering\n"));
    rx_enter();
    if (!handle_in_progress)
    {
#if 0
      /* XXX I really don't like the POLL_BLOCKING_UNTIL_RECEIVE
       * because it seems too easy to deadlock.
       * For now, I'll just poll
       */
      message_handled = select_and_read(POLL_UNTIL_SELECT_FAILS, 
					HANDLE_MESSAGES);
#else
      message_handled = select_and_read(POLL_BLOCKING_UNTIL_RECEIVE,
			HANDLE_MESSAGES);
#endif
    }
    rx_exit();
    nexus_debug_printf(3, ("rx_blocking_poll(): exiting\n"));
    
} /* rx_blocking_poll() */

/*
 * rx_init_remote_service_request()
 *
 * Initiate a remote service request to the node and context specified
 * in the gp, to the handler specified by handler_id and handler_name.
 *
 * Note: The things pointed to by 'gp' and 'handler_name' should
 * NOT change between this init_rsr call and the subsequent send_rsr.
 * Only the pointers are stashed away, and not the full data.
 *
 * Return: Fill in 'buffer' with a nexus_buffer_t.
 */
static void rx_init_remote_service_request(nexus_buffer_t *buffer,
					    nexus_global_pointer_t *gp,
					    char *handler_name,
					    int handler_id)
{
    rx_buffer_t *rx_buffer;
    int header_size;
#if ROUND_MALLOC > 1
    int header_mod;
#endif    

    NexusAssert2((gp->proto->type == NEXUS_PROTO_TYPE_RX),
		 ("rx_init_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_RX(%d)\n",NEXUS_PROTO_TYPE_RX));

    GetRxBuffer(rx_init_remote_service_request(), rx_buffer);
#ifndef NEXUS_FORCE_XDR
    if (   (gp->class_type == SYS_CLASS_TYPE)
	&& (gp->class_type != CLASS_TYPE_UNKNOWN32)
	&& (gp->class_type != CLASS_TYPE_UNKNOWN64) )
    {
        rx_buffer->funcs = &rx_buffer_funcs;
	rx_buffer->use_xdr = NEXUS_FALSE;
    } else
#endif
    {

	rx_buffer->funcs = &rx_xdr_buffer_funcs;
	rx_buffer->use_xdr = NEXUS_TRUE;
	/* 
	 * set ops to NULL to show that this xdr stream hasn't been
	 * used and therefore shouldn't be destroyed later
	 */
	rx_buffer->xdrs.x_ops = NULL;
    }
    rx_buffer->adaptor_funcs = ((rx_proto_t *)gp->proto)->adaptor_funcs;
    rx_buffer->next = (rx_buffer_t *) NULL;
    rx_buffer->buffer_type = SEND_BUFFER;
    rx_buffer->storage = (char *) NULL;
    rx_buffer->sequence_number = -1;
    rx_buffer->u.send.gp = gp;
    rx_buffer->u.send.handler_id = handler_id;
    rx_buffer->u.send.handler_name = handler_name;
    rx_buffer->u.send.handler_name_length = strlen(handler_name);
    rx_buffer->u.send.size = 0;
    rx_buffer->u.send.n_elements = -1;
    rx_buffer->u.send.base_pointer = (char *) NULL;
    rx_buffer->u.send.current_pointer = (char *) NULL;
    rx_buffer->u.send.header_pointer = (char *) NULL;

    NexusAssert2((rx_buffer->u.send.handler_name_length < NEXUS_MAX_HANDLER_NAME_LENGTH),
		 ("rx_init_remote_service_request(): Handler name exceeds maximum length (%d): %s\n", NEXUS_MAX_HANDLER_NAME_LENGTH, handler_name));
    
    header_size = RX_MSG_HEADER_SIZE + rx_buffer->u.send.handler_name_length;
#if ROUND_MALLOC > 1
    header_mod = header_size % ROUND_MALLOC;
    if (header_mod != 0)
    {
	header_size += (ROUND_MALLOC - header_mod);
    }
#endif    
    rx_buffer->u.send.header_size = header_size;

    *buffer = (nexus_buffer_t) rx_buffer;
} /* rx_init_remote_service_request() */


#ifdef BUILD_DEBUG
void print_buffer_for_debugging(char *routine_name, int total_size, 
				int fd, int n_sent, 
				char *send_ptr )
{
    int save_error = errno;
    int i;
    char *buf = tmpbuf1, *tbuf = tmpbuf2;

    nexus_printf("%s: Write attempt of %d bytes on fd=%d returns %d\n",
		 routine_name,
		 total_size,
		 fd,
		 n_sent);
    if (n_sent > 0)
    {
	buf[0] = '\0';
	for (i = 0; i < 20 && i < n_sent; i++)
	{
	     sprintf(tbuf, "%u ", (unsigned char) send_ptr[i]);
	     strcat(buf, tbuf);
	}

	nexus_printf("Beginning of message: %s\n", buf);
    }
    errno = save_error;
} /* print_buffer_for_debugging */
#endif

int allocate_buffer_and_return_size( rx_buffer_t *buf )
{
    int msg_size;

    if (buf->storage == (char *) NULL)
    {
	/*
	 * This is a zero length message. Neither rx_set_buffer_size() nor
	 * rx_check_buffer_size() was called for this buffer.  So use a dummy
	 * buffer for the header information.
	 */
	NexusMalloc( rx_send_remote_service_request(),
		     buf->storage,
		     char *,
		     buf->u.send.header_size );
	SetBufferPointers( buf );
        msg_size = 0;
    }
    else
    {
	if ( buf->use_xdr ) 
	{
	    msg_size = xdr_getpos( &(buf->xdrs) );
	}
	else
	{
	    msg_size = buf->u.send.current_pointer - buf->u.send.base_pointer;
	}
    }

    /*
     * RX proto uses msg_size for complete size of datagram, including 
     * everything.  If a message buffer won't fit in a single datagram
     * then it is fragmented into multiple buffers and sent to the
     * receiver, which reassembles and handles the message.
     * 
     * Question: The mtu isn't known until open_proto() 
     * opens a connection.  Should we assume 4K mtu's are o.k., or do 
     * something more dynamic?
     */

    msg_size += RX_MSG_HEADER_SIZE + buf->u.send.handler_name_length;    
 
    return msg_size;
} /* allocate_buffer_and_return_size() */


void suspend_remote_service_request(rx_proto_t *proto, 
				    rx_buffer_t *buf, 
				    char *send_ptr, 
				    int write_len,
				    int rsr_type)
{
  rx_suspended_rsr_t *s;

  tracepoint("suspend_remote_service_request");

      NexusMalloc(suspend_remote_service_request(),
		s,
		rx_suspended_rsr_t *,
		sizeof(rx_suspended_rsr_t));
  s->proto = proto;
  s->buf = buf;
  s->send_ptr = send_ptr;
  s->rsr_type = rsr_type;
  s->write_len = write_len;
  s->next_frag_delta = 0;
  s->next_frag_ptr = (char *)NULL;
  s->force_timeout_reset = 0;
  s->n_left = -1;
  s->buf2 = NULL;

  FD_SET( proto->fd, &current_write_fd_set );
  proto->suspended = s;
} /* suspend_remote_service_request() */


void suspend_remote_service_request_frag(rx_proto_t *proto, 
				    rx_buffer_t *buf, 
				    char *send_ptr, 
				    int write_len,
				    int rsr_type,
				    int n_left,
				    char *next_frag_ptr,
				    int next_frag_delta,
				    int force_timeout_reset,
					 rx_buffer_t *buf2)
{
  rx_suspended_rsr_t *s;
  
  tracepoint("suspend_remote_service_request_frag");

      NexusMalloc(suspend_remote_service_request(),
		s,
		rx_suspended_rsr_t *,
		sizeof(rx_suspended_rsr_t));
  s->proto = proto;
  s->buf = buf;
  s->send_ptr = send_ptr;
  s->rsr_type = rsr_type;
  s->write_len = write_len;
  s->next_frag_delta = next_frag_delta;
  s->next_frag_ptr = next_frag_ptr;
  s->force_timeout_reset = force_timeout_reset;
  s->n_left = n_left;
  s->buf2 = buf2;

  FD_SET( proto->fd, &current_write_fd_set );
  proto->suspended = s;
} /* suspend_remote_service_request_frag() */


/*
 * rx_send_remote_service_request()
 *
 * Generate a remote service request message to the node and context
 * saved in the 'nexus_buffer'.
 */
static int rx_send_remote_service_request(nexus_buffer_t *nexus_buffer)
{
    rx_buffer_t *buf,*buf2;
    rx_proto_t *proto;
    unsigned long tmp_longs[RX_MSG_HEADER_INTS];
    int msg_size, total_size, n_left, n_sent, write_len;
    char *send_ptr;
    char *next_frag_ptr;
    int next_frag_delta;
    int fd;
    int force_timeout_reset;

    int sequence_number;
    int rsr_type;
    int save_error;
#ifdef BUILD_PROFILE
    int node_id, context_id;
#endif    

    n_rsrs++;
    NexusBufferMagicCheck(rx_send_remote_service_request, nexus_buffer);

    buf = (rx_buffer_t *) *nexus_buffer;
    NexusAssert2((buf->buffer_type == SEND_BUFFER),
		 ("rx_send_remote_service_request(): Internal error: Expected a send buffer\n"));
    
    proto = (rx_proto_t *) buf->u.send.gp->proto;
    NexusAssert2((proto->type == NEXUS_PROTO_TYPE_RX),
		 ("rx_send_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_RX(%d)\n",NEXUS_PROTO_TYPE_RX));

#ifdef BUILD_DEBUG
    if(NexusDebug(2))
    {
      nexus_printf("rx_send_remote_service_request(): invoked with buffer:%x\n",
		   nexus_buffer );
    }
#endif

    msg_size = allocate_buffer_and_return_size( buf );

    if ( msg_size > ATM_MAX_TRANSMISSION_UNIT )
    {
      rsr_type = RSR_OPENFRAG_TYPE;
    }
    else 
    {
      rsr_type = RSR_OPEN_TYPE;
    }

    /*
     * Setup message header:
     *
     * Cray note: 8 bytes are used for unsigned longs instead of 4.
     * So the following code will likely have to change for the Cray to
     * interoperate with other (32 bit) machines.
     *
     * After the header comes the handler name string, _not_ NULL terminated.
     */

    /*
     * The sequence number is reset to zero whenever a proto is
     * opened, and incremented once for each new message or message
     * fragment.  When the header is first set up, a dummy sequence
     * number is copied into place.  This is filled in below, after
     * the proto is opened, and once this thread has performed
     * rx_enter(). 
     */
#define SEQUENCE_NUMBER_POSITION 2
    sequence_number = -1;

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(rsr_type);
    tmp_longs[2] = NX_HTONL(sequence_number); /* place holder */
    tmp_longs[3] = NX_HTONL(buf->use_xdr);
    tmp_longs[4] = NX_HTONL(buf->u.send.gp->context);
    tmp_longs[5] = NX_HTONL(buf->u.send.gp->address);
    tmp_longs[6] = NX_HTONL((unsigned long) (buf->u.send.handler_id));
    tmp_longs[7] = NX_HTONL((unsigned long) (buf->u.send.handler_name_length));
#ifdef BUILD_PROFILE
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    tmp_longs[8] = NX_HTONL((long) node_id);
    tmp_longs[9] = NX_HTONL((long) context_id);
#endif    
    memcpy(buf->u.send.header_pointer, tmp_longs,
	   RX_MSG_HEADER_SIZE );
    memcpy(buf->u.send.name_pointer, buf->u.send.handler_name,
	   buf->u.send.handler_name_length);
    
    total_size = msg_size;

#ifdef BUILD_PROFILE
    _nx_pablo_log_remote_service_request_send(buf->u.send.gp->node_id,
					      buf->u.send.gp->context_id,
					      buf->u.send.handler_name,
					      buf->u.send.handler_id,
					      msg_size);
#endif
	
    rx_enter();
    
    while (proto->send_in_progress)
    {
	/*
	 * There is already another thread sending on this proto.
	 * That thread probably would have blocked on a write,
	 * so it yielded in the hope that some else could do
	 * useful work in the mean time.  But since this thread's work
	 * is on the same proto, it cannot do anything.
	 *
	 */
	proto->sends_waiting++;
	nexus_cond_wait(&(proto->sends_waiting_cond), &rx_mutex);
	proto->sends_waiting--;
    }
    
    proto->send_in_progress = NEXUS_TRUE;

    /*
     * Open up the proto if it is not already open.
     */
    if (proto->fd < 0)
    {
        open_proto(proto);
    }

    /*
     * Fill-in place holder for sequence number
     */
    sequence_number = proto->sequence_number;
    proto->sequence_number++;
    tmp_longs[SEQUENCE_NUMBER_POSITION] = NX_HTONL(sequence_number);
    memcpy(buf->u.send.header_pointer + (SEQUENCE_NUMBER_POSITION * sizeof(long)),
	   tmp_longs + SEQUENCE_NUMBER_POSITION,
	   sizeof(unsigned long));
    buf->sequence_number = sequence_number;
    buf->msg_size = msg_size;

    fd = proto->fd;
    NexusAssert2((fd >= 0),
		 ("rx_send_remote_service_request(): Internal error: Failed to open proto\n"));

    send_ptr = buf->u.send.header_pointer;

    /*
     * If the message fits in a single datagram, just send it.
     * Otherwise fragment it, and reassemble it on the other end.
     */

    if (rsr_type == RSR_OPEN_TYPE)
    {
        tracepoint("rx_send single rsr");
        n_rsr_buffers++;

	n_sent = proto->adaptor_funcs->send(fd, send_ptr, total_size);
#ifdef NEXUS_RX_PROTO_MN_UDP
	save_error = mn_errno;
#else
	save_error = errno;
#endif
        tracepoint("rx_send single rsr done");
	
	/*
	 * n_sent: is > 0 on success -- number of bytes written
	 *         is < 0 on error -- need to check errno
	 *         XXX is 0 (SysV) or (-1 && errno==EWOULDBLOCK) (BSD)
	 *             if the write would block without writing anything
	 */
	
#ifdef BUILD_DEBUG
	if (NexusDebug(3))
	{
	    print_buffer_for_debugging("rx_send_remote_service_request()",
				       total_size, fd, n_sent, send_ptr);
	}
#endif
	if (n_sent == total_size) 
        {
	    /* Save the buffer until the ack arrives */
	    SaveRxBuffer( proto, buf, NEXUS_FALSE );
	}
#ifdef NEXUS_RX_PROTO_MN_UDP
	else if ((mn_errno == EWOULDBLOCK) || (mn_errno == EIO))
	{
	    suspend_remote_service_request(proto,buf,send_ptr,total_size,rsr_type);
	    rx_exit();
	    return;
	}
#endif
	else /* n_sent < 0  or n_sent != total_size */
	{
#ifdef NEXUS_RX_PROTO_MN_UDP
/* JGG 
 * was this done correctly?  I'm assuming that mn_errno was set in the
 * send, and set save_errno.  Is it possible for both errno and mn_errno
 * to be set from the same call?
 *
 * Not sure -- schwab.
 *
 * JGG */
	    if ((save_error == ENOBUFS) || (save_error == EIO))
	    {
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EINTR: %s\n",
			  _nx_md_system_error_string());
	    }
	    else if (save_error == EINTR)
#else
    	    if (save_error == EINTR)
#endif
	    {
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EINTER: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	    else if (save_error == EBADF)
	    {
		/*
		 * The fd was closed unexpectedly.
		 * So call check_proto_for_close() to see what's going on.
		 * The choices are:
		 *   1) The process at the other end died (or
		 *      erroneously closed the fd).  In this case
		 *      check_proto_for_close() will fatal error out.
		 *   2) It closed the fd, so as to make room
		 *      to open another fd.  In this case we need
		 *      to reopen the fd and restart the write
		 *	from the beginning of the buffer.
		 */
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EBADF: %s\n",
			  _nx_md_system_error_string(save_error));
#if 0
#ifdef BUILD_DEBUG
		if(NexusDebug(2))
		    nexus_printf("rx_send_remote_service_request(): checking for proto close\n");
#endif
		check_proto_for_close(proto);
		NexusAssert2((proto->fd < 0),
			     ("rx_send_remote_service_request(): Internal error: expected fd<0\n"));
		open_proto(proto);
		fd = proto->fd;
		send_ptr = buf->u.send.header_pointer;
#endif 
	    }
	    else
	    {
		rx_fatal("rx_send_remote_service_request(): Write failed: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	}
    }
    else /* rsr_type == RSR_OPENFRAG_TYPE */
    {
      /*
       * Send the message in fragments.  The structure of a message is
       * as follows
       *
       * 1st Fragment -- RSR_OPENFRAG_TYPE -- msg_size == size of
       * entire message. -- contains header and handler info, plus
       * start of rx_buffer.
       *		
       * 2nd-(n-1)st -- RSR_FRAG_TYPE -- only has the 3 RX_DISPATCH
       * fields, (size,type,seq) and size is ignored.  The incoming
       * receiver will extract the rest of the payload as a portion of
       * the message and append it onto the previous fragment.
       *
       * Nth (last) -- RSR_ENDFRAG_TYPE -- only has the 3 RX_DISPATCH
       * fields, (size,type,seq) and size identifies total size of
       * datagram, including the dispatch header.  The receiver will
       * copy the last portion of the message into the receive buffer
       * and call _nx_handle_message().
       *
       * As an optimization, RSR_FRAG_TYPES are **NOT** ACK'ed, but my
       * be NACK'ed.  When the RSR_ENDFRAG_TYPE is processed, it's ACK
       * will cancel all other fragments. 
       */

      /* 
       * code sketch:
       *
       * while (!done)
       *  rx_send frag, SaveRxBuffer(),
       *  allocate new RX buffer for next fragment
       *  copy next piece into RX buffer
       *  if reached end of data, set type to RSR_ENDFRAG_TYPE,
       *  else set type to RSR_FRAG_TYPE
       *  bump sequence_number
       * end_while
       */

      /*
       * Do some contortions on the pointers and counters so we can
       * just run the first fragment off using the normal setup,
       * and then process the other ones on-the-fly.
       *
       * XXX Fix the stupidity of not "protecting" the original
       * rx_buffer from being free'd before all the fragments
       * have been created.
       */

      n_left = total_size;
      write_len = ATM_MAX_TRANSMISSION_UNIT;
      next_frag_ptr = send_ptr;
      next_frag_delta = ATM_MAX_TRANSMISSION_UNIT;
      buf->msg_size = ATM_MAX_TRANSMISSION_UNIT;
      buf2 = buf;
      force_timeout_reset = (proto->ack_list_head == NULL);

      /* make buf2 a real buffer that is a copy of the initial fragment */
      /* allocate new RX buffer for next fragment  */
      GetRxBuffer(rx_send_remote_service_request(), buf2);
      buf2->adaptor_funcs = buf->adaptor_funcs;
      NexusMalloc(rx_send_remote_service_request(), buf2->storage, char *,
		  ATM_MAX_TRANSMISSION_UNIT );
      buf2->u.send.header_pointer = buf2->storage;
      send_ptr = buf2->storage;

      memcpy( buf2->storage, next_frag_ptr, next_frag_delta );
      buf2->sequence_number = sequence_number;
      buf2->msg_size = write_len;

      while ( NEXUS_TRUE )
      {
	tracepoint("rx_send multifrag rsr");
	n_rsr_buffers++;
	n_sent = proto->adaptor_funcs->send(fd, send_ptr, write_len);
#ifdef NEXUS_RX_PROTO_MN_UDP
	save_error = mn_errno;
#else
	save_error = errno;
#endif

	/*
	 * n_sent: is > 0 on success -- number of bytes written
	 *         is < 0 on error -- need to check errno
	 *         XXX is 0 (SysV) or (-1 && errno==EWOULDBLOCK) (BSD)
	 *             if the write would block without writing anything
	 */
	
#ifdef BUILD_DEBUG
	if (NexusDebug(3))
	{
	    print_buffer_for_debugging("rx_send_remote_service_request()",
				       write_len, fd, n_sent, send_ptr);
	}
#endif
	if (n_sent == write_len) 
	{
	  SaveRxBuffer( proto, buf2, force_timeout_reset ); 
	}
#ifdef NEXUS_RX_PROTO_MN_UDP
	else if ((mn_errno == EWOULDBLOCK) || (mn_errno == EIO))
	{
	      suspend_remote_service_request_frag(proto,buf,send_ptr,
						  write_len,rsr_type,
					     n_left,
					     next_frag_ptr,next_frag_delta,
					     force_timeout_reset,buf2);
	      rx_exit();
	      return;
	}
#endif
	else /* n_sent < 0  or n_sent != total_size */
	{
    	    if (save_error == EINTR)
	    {
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EINTR: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	    else if (save_error == EBADF)
	    {
		/*
		 * The fd was closed unexpectedly.
		 * So call check_proto_for_close() to see what's going on.
		 * The choices are:
		 *   1) The process at the other end died (or
		 *      erroneously closed the fd).  In this case
		 *      check_proto_for_close() will fatal error out.
		 *   2) It closed the fd, so as to make room
		 *      to open another fd.  In this case we need
		 *      to reopen the fd and restart the write
		 *	from the beginning of the buffer.
		 */
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EBADF: %s\n",
			  _nx_md_system_error_string(save_error));
#if 0
#ifdef BUILD_DEBUG
		if(NexusDebug(2))
		    nexus_printf("rx_send_remote_service_request(): checking for proto close\n");
#endif
		check_proto_for_close(proto);
		NexusAssert2((proto->fd < 0),
			     ("rx_send_remote_service_request(): Internal error: expected fd<0\n"));
		open_proto(proto);
		fd = proto->fd;
		send_ptr = buf->u.send.header_pointer;
#endif 0
	    }
	    else
	    {
		rx_fatal("rx_send_remote_service_request(): Write failed: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	}
	/* check if anything is left */
        n_left -= next_frag_delta;
	if (n_left == 0)
	  break;
	next_frag_ptr += next_frag_delta;

        /* allocate new RX buffer for next fragment  */
        GetRxBuffer(rx_send_remote_service_request(), buf2);
	buf2->adaptor_funcs = buf->adaptor_funcs;
	NexusMalloc(rx_send_remote_service_request(), buf2->storage, char *,
			ATM_MAX_TRANSMISSION_UNIT );
	buf2->u.send.header_pointer = buf2->storage;
	send_ptr = buf2->storage;

        /* copy next piece into RX buffer */
	if (n_left > ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE)
	{ 
	  write_len = ATM_MAX_TRANSMISSION_UNIT;
	  next_frag_delta = ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE;
	  rsr_type = RSR_FRAG_TYPE;
	}
	else
	{
	  write_len = n_left + RX_DISPATCH_SIZE;
	  next_frag_delta = n_left;
	  rsr_type = RSR_ENDFRAG_TYPE;
	}

	memcpy ( (buf2->storage) + RX_DISPATCH_SIZE , next_frag_ptr, 
		next_frag_delta );

        /* bump sequence_number */
	sequence_number = proto->sequence_number;
	proto->sequence_number++;
        tmp_longs[0] = NX_HTONL(write_len);
	tmp_longs[1] = NX_HTONL(rsr_type);
	tmp_longs[2] = NX_HTONL(sequence_number);
	memcpy( buf2->storage, tmp_longs, RX_DISPATCH_SIZE );
	buf2->sequence_number = sequence_number;
	buf2->msg_size = write_len;
      }
      
      /* O.K. to free buf now */
      FreeRxBuffer( buf );
    } 

    proto->send_in_progress = NEXUS_FALSE;
    if (proto->sends_waiting > 0)
    {
      nexus_cond_signal(&(proto->sends_waiting_cond));
    }

    rx_exit();

    /*XXX should this be here, or where the FreeRxBuffer() happens? */
    if(buf->use_xdr && buf->xdrs.x_ops) xdr_destroy(&(buf->xdrs)); 

    
/* XXX Never recurse here */
/*    nexus_poll();*/
    return( 0 );

} /* rx_send_remote_service_request() */


/*
 * continue_remote_service_request()
 *
 * The fd for this proto is now writeable.  So continue
 * sending datagrams where the sending left off.
 *
 * reset the current_write_fd_set bit for this fd if the rsr 
 * is completed, and signal any waiters
 */
void continue_remote_service_request(rx_proto_t *proto)
{
    int n_sent;
    int sequence_number;
    unsigned long tmp_longs[RX_MSG_HEADER_INTS];
    rx_suspended_rsr_t *s = proto->suspended;
    int save_error;

    tracepoint("continue_remote_service_request");

    if ( s->rsr_type == RSR_OPENFRAG_TYPE ) 
    {
        n_rsr_buffers++;
	n_sent = proto->adaptor_funcs->send(proto->fd, 
					    s->send_ptr, 
					    s->write_len);
	
#ifdef NEXUS_RX_PROTO_MN_UDP
	save_error = mn_errno;
#else
	save_error = errno;
#endif
	if (n_sent == s->write_len) 
        {
	    /* Save the buffer until the ack arrives */
	    SaveRxBuffer( proto, s->buf, NEXUS_FALSE );
	    proto->suspended = NULL;
	    FD_CLR(proto->fd, &current_write_fd_set);
	    NexusFree( s );
	    proto->send_in_progress = NEXUS_FALSE;
	    if (proto->sends_waiting > 0)
	    {
		nexus_cond_signal(&(proto->sends_waiting_cond));
	    }
	}
#ifdef NEXUS_RX_PROTO_MN_UDP
	else if ((mn_errno == EWOULDBLOCK) || (mn_errno == EIO))
	{
	    return;
	}
#endif
	else /* n_sent < 0  or n_sent != total_size */
	{
    	    if (save_error == EINTR)
	    {
		rx_fatal("continue_remote_service_request(): Don't know what to do about an EINTR: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	    else if (save_error == EBADF)
	    {
		/*
		 * The fd was closed unexpectedly.
		 * So call check_proto_for_close() to see what's going on.
		 * The choices are:
		 *   1) The process at the other end died (or
		 *      erroneously closed the fd).  In this case
		 *      check_proto_for_close() will fatal error out.
		 *   2) It closed the fd, so as to make room
		 *      to open another fd.  In this case we need
		 *      to reopen the fd and restart the write
		 *	from the beginning of the buffer.
		 */
		rx_fatal("continue_remote_service_request(): Don't know what to do about an EBADF: %s\n",
			  _nx_md_system_error_string(save_error));
#if 0
		check_proto_for_close(proto);
		NexusAssert2((proto->fd < 0),
			     ("rx_send_remote_service_request(): Internal error: expected fd<0\n"));
		open_proto(proto);
		fd = proto->fd;
#endif
		/*send_ptr = buf->u.send.header_pointer;*/
	    }
	    else
	    {
		rx_fatal("rx_send_remote_service_request(): Write failed: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	}
  }
  else  /* multifrag rsr */
  {
    while (NEXUS_TRUE) 
    {
	n_rsr_buffers++;

	n_sent = proto->adaptor_funcs->send(proto->fd, s->send_ptr, s->write_len);
#ifdef NEXUS_RX_PROTO_MN_UDP
	save_error = mn_errno;
#else
	save_error = errno;
#endif
	if (n_sent == s->write_len) 
	{
	  /* Save the buffer until the ack arrives */
	  SaveRxBuffer( proto, s->buf2, s->force_timeout_reset ); 
	}
#ifdef NEXUS_RX_PROTO_MN_UDP
	else if ((mn_errno == EWOULDBLOCK) || (mn_errno == EIO))
	{
	  return;
	}
#endif
	else /* n_sent < 0  or n_sent != total_size */
	{
    	    if (save_error == EINTR)
	    {
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EINTR: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	    else if (save_error == EBADF)
	    {
		/*
		 * The fd was closed unexpectedly.
		 * So call check_proto_for_close() to see what's going on.
		 * The choices are:
		 *   1) The process at the other end died (or
		 *      erroneously closed the fd).  In this case
		 *      check_proto_for_close() will fatal error out.
		 *   2) It closed the fd, so as to make room
		 *      to open another fd.  In this case we need
		 *      to reopen the fd and restart the write
		 *	from the beginning of the buffer.
		 */
		rx_fatal("rx_send_remote_service_request(): Don't know what to do about an EBADF: %s\n",
			  _nx_md_system_error_string(save_error));
#if 0
#ifdef BUILD_DEBUG
		if(NexusDebug(2))
		    nexus_printf("rx_send_remote_service_request(): checking for proto close\n");
#endif
		check_proto_for_close(proto);
		NexusAssert2((proto->fd < 0),
			     ("rx_send_remote_service_request(): Internal error: expected fd<0\n"));
		open_proto(proto);
		fd = proto->fd;
		send_ptr = buf->u.send.header_pointer;
#endif 0
	    }
	    else
	    {
		rx_fatal("rx_send_remote_service_request(): Write failed: %s\n",
			  _nx_md_system_error_string(save_error));
	    }
	}
	/* update pointers for next fragment */
        s->n_left -= s->next_frag_delta;
	if (s->n_left == 0)
	  break;
	s->next_frag_ptr += s->next_frag_delta;

        /* allocate new RX buffer for next fragment  */
        GetRxBuffer(continue_remote_service_request(), s->buf2);
	s->buf2->adaptor_funcs = s->buf->adaptor_funcs;
	NexusMalloc(continue_remote_service_request(), s->buf2->storage, char *,
			ATM_MAX_TRANSMISSION_UNIT );
	s->buf2->u.send.header_pointer = s->buf2->storage;
	s->send_ptr = s->buf2->storage;

        /* copy next piece into RX buffer */
	if (s->n_left > ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE)
	{ 
	  s->write_len = ATM_MAX_TRANSMISSION_UNIT;
	  s->next_frag_delta = ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE;
	  s->rsr_type = RSR_FRAG_TYPE;
	}
	else
	{
	  s->write_len = s->n_left + RX_DISPATCH_SIZE;
	  s->next_frag_delta = s->n_left;
	  s->rsr_type = RSR_ENDFRAG_TYPE;
	}

	memcpy ( (s->buf2->storage) + RX_DISPATCH_SIZE , s->next_frag_ptr, 
		s->next_frag_delta );

        /* bump sequence_number */
	sequence_number = proto->sequence_number;
	proto->sequence_number++;
        tmp_longs[0] = NX_HTONL(s->write_len);
	tmp_longs[1] = NX_HTONL(s->rsr_type);
	tmp_longs[2] = NX_HTONL(sequence_number);
	memcpy( s->buf2->storage, tmp_longs, RX_DISPATCH_SIZE );
	s->buf2->sequence_number = sequence_number;
	s->buf2->msg_size = s->write_len;

    }
    proto->suspended = NULL;
    FD_CLR(proto->fd, &current_write_fd_set);
    NexusFree( s );
    proto->send_in_progress = NEXUS_FALSE;
    if (proto->sends_waiting > 0)
    {
      nexus_cond_signal(&(proto->sends_waiting_cond));
    }
  }
} /* continue_remote_service_request() */


/*
 * rx_send_urgent_remote_service_request()
 */
static int rx_send_urgent_remote_service_request(nexus_buffer_t *buffer)
{
    return(rx_send_remote_service_request(buffer));
} /* rx_send_urgent_remote_service_request() */


/*
 * rx_destroy_proto()
 *
 * Decrement the reference count for this proto.  If it goes to 0
 * then send a CLOSE_NORMAL message and bump the n_defered_closes count.
 * read_proto() will handle ACKs for this proto, and when the CLOSE_NORMAL
 * message has been ACK'ed, it will close the fd used by this proto.
 *
 * The being_closed field will be set TRUE to prevent an rx_shutdown()
 * operation from trying to start a close sequence on the proto while
 * it is being closed.
 */
static void rx_destroy_proto(nexus_proto_t *nexus_proto)
{
    rx_proto_t *proto = (rx_proto_t *) nexus_proto;

    rx_enter();
    
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
        nexus_printf("rx_destroy(): proto=%x %s/%d fd=%d refcount=%d\n",
		     proto,
		     proto->host, proto->port,
		     proto->fd,
		     proto->reference_count);
    }
#endif

    if (proto->being_closed)
    {
      rx_exit();
      return;
    }

    proto->reference_count--;
    
    NexusAssert2((proto->reference_count >= 0),
		 ("rx_destroy(): Internal error: Reference count < 0\n"));
	       
    if (proto->reference_count == 0 && proto->fd >= 0)
    {
	if (!close_proto_with_message(proto))
	{
	    /*
	     * Under RX we always need to wait for the ACK.
	     *
	     * So we need to defer the close.  This is done by
	     * incrementing n_deferred_closes.  At some later point,
	     * we can quickly check for the presence of deferred
	     * closes by checking this variable, and if there are
	     * then search the fd_to_proto table for protos with
	     * a reference_count of 0 and an open fd.
	     * 
	     * XXX Make sure close_proto_with_message sends the close_messages.
	     * The receipt of an ACK will cause the close.
	     */
	    n_deferred_closes++;
	}
    }

    rx_exit();
    
} /* rx_destroy_proto() */

/*
 * rx_copy_proto()
 *
 * Increase the reference count on the associated proto and copy the
 * pointer to the nexus_proto_t
 *
 */
static void rx_copy_proto(nexus_proto_t *proto_in,
			   nexus_proto_t **proto_out)
{
    rx_proto_t *proto = (rx_proto_t *) proto_in;

    rx_enter();
    proto->reference_count++;
    *proto_out = proto_in;
    rx_exit();
} /* rx_copy_proto() */

/*
 * rx_get_my_mi_proto()
 *
 * Return the machine independent rx protocol information
 * for this protocol.
 *
 * The rx proto requires two integers (the tcp port, the adaptor type) and one
 * string (the hostname) to encode its information.
 */
static nexus_mi_proto_t *rx_get_my_mi_proto(void)
{
    nexus_mi_proto_t *mi_proto;
    int host_length;

    host_length = strlen(rx_local_host);
    NexusMalloc(rx_get_my_mi_proto(), mi_proto, nexus_mi_proto_t *,
		(sizeof(nexus_mi_proto_t)	/* base */
		 + 2 *sizeof(int)		/* space for ints */
		 + sizeof(int)			/* space for string_lengths */
		 + sizeof(char *)		/* space for string pointers */
		 + host_length + 1		/* space for strings */
		 ));
    mi_proto->proto_type = NEXUS_PROTO_TYPE_RX;
    mi_proto->n_ints = 2;
    mi_proto->n_strings = 1;
    mi_proto->ints = (int *) (((char *) mi_proto)
			      + sizeof(nexus_mi_proto_t));
    mi_proto->ints[0] = rx_local_port;
    mi_proto->ints[1] = default_adaptor_type;
    mi_proto->string_lengths = (int *) (((char *) mi_proto)
					+ sizeof(nexus_mi_proto_t)
					+ 2 * sizeof(int));
    mi_proto->string_lengths[0] = host_length;
    mi_proto->strings = (char **) (((char *) mi_proto)
				   + sizeof(nexus_mi_proto_t)
				   + 2 * sizeof(int)
				   + sizeof(int));
    mi_proto->strings[0] = (char *) (((char *) mi_proto)
				     + sizeof(nexus_mi_proto_t)
				     + 2 * sizeof(int)
				     + sizeof(int)
				     + sizeof(char *));
    strcpy(mi_proto->strings[0], rx_local_host);

    return (mi_proto);
} /* rx_get_my_mi_proto() */


/*
 * rx_construct_from_mi_proto()
 *
 * The passed machine independent protocol, 'mi_proto', should
 * be an rx protocol that I can use to connect to that node:
 *  - If it is not a rx protocol, then fatal out.
 *  - If it is a rx protocol:
 *	- If I cannot use this protocol to attach to the node, then
 *		return NEXUS_FALSE.  (This option is useful if two nodes
 *		both speak a particular protocol, but they cannot
 *		talk to each other via that protocol.  For example,
 *		on two MPP, the nodes within a single MPP can
 *		talk to each other via the native messaging protocol,
 *		but cannot talk to the nodes on the other MPP
 *		using that native protocol.)
 *	- If this rx protocol points to myself, then set
 *		*proto=_nx_local_proto, and return NEXUS_TRUE.
 *	- Otherwise, construct a rx protocol object for this mi_proto
 *		and put it in *proto.  Then return NEXUS_TRUE.  
 */
static nexus_bool_t rx_construct_from_mi_proto(nexus_proto_t **proto,
					  nexus_mi_proto_t *mi_proto)
{
    char *host;
    int port;
    int type;
    
    NexusAssert2((mi_proto->proto_type == NEXUS_PROTO_TYPE_RX),
		 ("rx_construct_from_mi_proto(): Internal error: Was given a non-rx(%d) mi_proto\n",NEXUS_PROTO_TYPE_RX));

    /*
     * Test to see if this proto points to myself.
     * If it does, then return the _nx_local_proto.
     */
    host = mi_proto->strings[0];
    port = mi_proto->ints[0];
    type = mi_proto->ints[1];
    if (   (port == rx_local_port)
	&& (strcmp(host, rx_local_host) == 0) )
    {
	*proto = _nx_local_proto;
    }
    else
    {
	rx_enter();
	*proto = (nexus_proto_t *) construct_proto(host, port, type);
	rx_exit();
    }
    return (NEXUS_TRUE);
} /* rx_construct_from_mi_proto() */


/*
 * rx_get_creator_proto_params()
 *
 * Encapsulate enough protocol information into a retained command
 * line argument so that the construct_creator_proto() routine
 * can reconstruct a proto to connect back to me.
 */
static void rx_get_creator_proto_params(char *buf,
					 int buf_size,
					 char *node_name,
					 int node_number)
{
    char tmp_buf[MAXHOSTNAMELEN + 64];
    
    nexus_stdio_lock();
    sprintf(tmp_buf, "-nf_nx_rx %s:%d:%d",
            rx_local_host,
	    rx_local_pid,
	    rx_local_port);
    nexus_stdio_unlock();
    if ( strlen(tmp_buf) > (size_t)buf_size)
    {
	nexus_fatal("rx_get_creator_proto_params(): Internal error: Buffer not big enough for arguments\n");
    }
    strcpy(buf, tmp_buf);
} /* rx_get_creator_proto_params() */


/*
 * rx_construct_creator_proto()
 *
 * Use the stored command line arguments to construct a proto
 * to my creator.
 */
static void rx_construct_creator_proto(nexus_proto_t **proto)
{
    char *creator_string;
    char *next;
    /*char host[MAXHOSTNAMELEN];*/
    char *host;
    int pid;
    int port;
    int rc;
    int arg_num;
    int *argc;
    char ***argv;

    nexus_get_retained_arguments(&argc, &argv);
    
    /* Get the -nf_nx_rx argument and parse it */
    if ((arg_num = nexus_find_argument(argc, argv, "nf_nx_rx", 2)) >= 0)
    {
	creator_string = (*argv)[arg_num + 1];
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	nexus_fatal("rx_construct_creator_proto(): Expected a -nf_nx_rx argument\n");
    }
    _nx_get_next_value(creator_string, ':', &next, &host);
    if (!next)
    {
	nexus_fatal("rx_construct_creator_proto(): Invalid -nf_nx_rx argument\n");
    }
    nexus_stdio_lock();
    rc = sscanf(next, "%d:%d", &pid, &port);
    nexus_stdio_unlock();
    if (rc != 2)
    {
	nexus_fatal("rx_construct_creator_proto(): Invalid -nf_nx_rx argument\n");
    }

    rx_enter();
    *proto = (nexus_proto_t *) construct_proto(host, port, default_adaptor_type);
    rx_exit();
} /* rx_construct_creator_proto() */


/*
 * rx_compare_protos()
 */
static int rx_compare_protos(nexus_proto_t *proto1,
			      nexus_proto_t *proto2)
{
    rx_proto_t *p1 = (rx_proto_t *) proto1;
    rx_proto_t *p2 = (rx_proto_t *) proto2;
    
    if (   (p1->port == p2->port)
	&& (strcmp(p1->host, p2->host) == 0) )
    {
	return (NEXUS_TRUE);
    }
    else
    {
	return (NEXUS_FALSE);
    }
} /* rx_compare_protos() */


/*
 * construct_proto()
 *
 * Construct a rx_proto_t for the given host and port. Look up in the
 * proto table to see if one already exists, and is open.  If so, bump its 
 * reference
 * count and return that one. Otherwise create one, insert into the
 * table with a reference count of 1 and return it.
 */
static rx_proto_t *construct_proto(char *host, int port, int type)
{
    rx_proto_t *proto;
    int fd = -1;

    proto = proto_table_lookup(host, port);
#ifdef BUILD_DEBUG
    if (NexusDebug(3))
	nexus_printf("construct_proto(): Table lookup returns proto=%x\n",
		     proto);
#endif
    if (proto == (rx_proto_t *) NULL)
    {
	NexusMalloc(construct_proto(), proto, rx_proto_t *,
		    sizeof(rx_proto_t));

	proto->type = NEXUS_PROTO_TYPE_RX;
	proto->funcs = &rx_proto_funcs;
	proto->adaptor_funcs = rx_type_to_funcs( type );
	proto->host = _nx_copy_string(host);
	proto->port = port;
	proto->fd = fd;
	proto->reference_count = 1;
	proto->being_closed = NEXUS_FALSE;
	proto->send_in_progress = NEXUS_FALSE;
	proto->sends_waiting = 0;
	proto->suspended = (rx_suspended_rsr_t *) NULL;
	nexus_cond_init(&(proto->sends_waiting_cond),
			(nexus_condattr_t *) NULL);
	proto->sequence_number = 0;
	proto->ack_list_head = proto->ack_list_tail = NULL;
	proto->timeout_list_next = NULL;
	proto->timeout_reset = RX_ROUND_TRIP_TIMEOUT_TICKS;
	proto->timeout_value = 0;
	proto->last_retransmit_sequence_number = 0;

#ifdef TIMESTAMP_ECHO
	proto->timestamp = NULL;
#endif /* TIMESTAMP_ECHO */

	proto_table_insert(proto);
    }
    else 
    {
	proto->reference_count++;
    }
    return (proto);
} /* construct_proto() */

/*
 * open_proto()
 *
 * Open the connection for the passed proto.  If it is already
 * connected, then just return.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 */
static void open_proto( rx_proto_t *proto )
{
    char *interface, *temp, *fullname;

    if ( proto->fd >= 0 )
    {
#ifdef BUILD_DEBUG
	if ( NexusDebug(2) )
	    nexus_printf( "open_proto(): initial return fd=%d\n", proto->fd );
#endif
	return;
    }

    make_room_for_new_fd();

    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,
		       ("open_proto(): proto(%x) do_connect\n",proto) );

    interface = nexus_rdb_lookup(proto->host, RX_INTERFACE_NAME_KEY);
    if ( !interface )
    {
        interface = _nx_copy_string( proto->host );
    }
    else if ( (temp = strchr(interface, '*')) )
    {
        NexusMalloc(open_proto(), fullname, char *, 
		    ( strlen( interface ) + strlen( proto->host )));

	/* Copy in 3 pieces.  Before the '*', replacing the '*', and after the '*' */
	strncpy( fullname, interface, temp-interface );
	strncpy( fullname + (temp-interface), proto->host, strlen(proto->host));
	strcpy( fullname + (temp-interface) + strlen(proto->host), temp + 1 );
	NexusFree( interface );
	interface = fullname;
	fullname = temp = (char *) NULL;
    }
    proto->fd = do_connect( interface, proto->port, proto->adaptor_funcs );
    
#ifdef BUILD_DEBUG
    if ( NexusDebug(2) )
	nexus_printf( "open_proto(): do_connect(%s/%d) returns %d\n",
		      proto->host, proto->port, proto->fd );
#endif
    NexusAssert2( (proto->fd >= 0),
		  ("open_proto(): Failed to make connection\n") );

    add_proto_with_fd( proto, proto->fd );

    NexusFree( interface );

} /* open_proto() */


/*
 * close_proto()
 *
 * Close a proto: (end that got the close)
 * precondition: ACK sent in response to CLOSE message.
 *
 *   1) Remove the proto from the fd_to_proto table
 *   2) Close the fd
 *   3) Modify proto data structure
 */
#if DONT_INCLUDE
static void close_proto(rx_proto_t *proto)
{
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    char send_ptr[CLOSE_MESSAGE_SIZE];
    int msg_size, close_type, sequence_number;

    remove_proto_with_fd(proto, proto->fd);
    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("close_proto(%x): about to do an adaptor_funcs->close(%d)\n",proto,proto->fd));

/* Hack Hack Hack */
/* Try to send a message just before the adaptor_funcs->close() */
    msg_size = CLOSE_MESSAGE_SIZE;
    close_type = CLOSE_NORMAL_TYPE;
    sequence_number = -1;

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(close_type);
    tmp_longs[2] = NX_HTONL(sequence_number);
    memcpy( send_ptr, tmp_longs, msg_size );

    proto->adaptor_funcs->send(proto->fd, send_ptr, msg_size);
    proto->adaptor_funcs->close(proto->fd);
    proto->fd = -1;
} /* close_proto() */
#endif 

static void close_proto(rx_proto_t *proto)
{
    remove_proto_with_fd(proto, proto->fd);
    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("close_proto(%x): about to do an adaptor_funcs->close(%d)\n",proto,proto->fd));
    proto->adaptor_funcs->close(proto->fd);
    proto->fd = -1;
} /* close_proto() */

/*
 * construct_incoming()
 *
 * Construct a rx_incoming_t for the given file descriptor, 'fd'.
 */
static rx_incoming_t *construct_incoming(int fd,
					  rx_adaptor_funcs_t *adaptor_funcs)
{
    rx_incoming_t *incoming;

    if (incoming_free_list == (rx_incoming_t *) NULL)
    {
	NexusMalloc(construct_incoming(), incoming, rx_incoming_t *,
		    sizeof(rx_incoming_t));
    }
    else
    {
	incoming = incoming_free_list;
	incoming_free_list = incoming_free_list->next_free;
    }

    incoming->type = INCOMING_TYPE;
    incoming->fd = fd;
    incoming->adaptor_funcs = adaptor_funcs;
    incoming->sequence_number = 0;
    incoming->ack_buf = NULL;
    incoming->supress_nacks = NEXUS_FALSE;
    incoming->being_closed = NEXUS_FALSE;
    incoming->timeout_list_next = NULL;
    incoming->timeout_reset = RX_ROUND_TRIP_TIMEOUT_TICKS;
    incoming->timeout_value = 0;
    incoming->next_free = (rx_incoming_t *) NULL;

    ResetIncoming(incoming);
#ifdef BUILD_DEBUG
    if(NexusDebug(2)) {
      nexus_printf("construct_incoming(): state: INCOMING_RECV_HEADER size: %d\n",
		   RX_MSG_HEADER_SIZE );
    }
#endif

    return (incoming);
} /* construct_incoming() */


/*
 * close_incoming()
 *
 * XXX RTCN or RTCS
 *
 * Close an incoming connection: (end that got the close)
 * precondition: ACK sent in response to CLOSE message
 *
 *   1) Remove the rx_incoming_t from the fd_to_proto table
 *   2) Close the fd
 *   3) Put the rx_incoming_t back on the free list.
 */
static void close_incoming(rx_incoming_t *incoming)
{
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
      nexus_printf("close_incoming(); closing %x fd=%d\n",
		   incoming,
		   incoming->fd);
    }
#endif
    remove_proto_with_fd((void *) incoming, incoming->fd);
    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("close_incoming(%x): about to do an adaptor_funcs->close(%d)\n",incoming,incoming->fd));
    incoming->adaptor_funcs->close(incoming->fd);
    
    NexusAssert2((incoming->ack_buf == NULL),("close_incoming(): ack_buf is not NULL!\n"));
    incoming->fd = -2;
    incoming->being_closed = NEXUS_FALSE;

    incoming->recv_state = INCOMING_RECV_CLOSED;

#if 0
    incoming->next_free = incoming_free_list;
    incoming_free_list = incoming;
#endif 
} /* close_incoming() */


#ifdef DONT_INCLUDE
/*
 * close_incoming_with_message()
 *
 * Send a normal close message down the file descriptor for
 * the given incoming connection.  If that works, then close it.
 * XXX Never do the close, let other side close it first.
 *
 * If 'read_messages' is NEXUS_TRUE, then read any pending messages between
 * sending the close message and actually closing the fd.
 *
 * Check for a failed write due to a closed fd, and handle it
 * accordingly.  And check for any other errors.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 *
 * Return: NEXUS_TRUE on success (incoming was closed)
 *         NEXUS_FALSE if the write would have blocked (the socket buffer
 *		is full), and thus the incoming was not closed
 */
static nexus_bool_t close_incoming_with_message(rx_incoming_t *incoming,
						nexus_bool_t read_messages)
{
    int n_sent;
    nexus_bool_t rc;
    int fd;
    char *close_buf;
    int close_type;
    int msg_size;
    int sequence_number;
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    rx_buffer_t *buf;
    int save_error;

    msg_size = CLOSE_MESSAGE_SIZE;
    close_type = REQ_TO_CLOSE_NORMAL_TYPE;
    sequence_number = 0;

    NexusMalloc(close_incoming_with_message(), close_buf, char *, 
		msg_size );
    GetRxBuffer(close_incoming_with_message(), buf);
    buf->adaptor_funcs = incoming->adaptor_funcs;
    buf->storage = close_buf;
    buf->u.send.header_pointer = close_buf;
    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(close_type);
    tmp_longs[2] = NX_HTONL(sequence_number);
    memcpy( close_buf, tmp_longs, msg_size );
    buf->sequence_number = sequence_number;
    buf->msg_size = msg_size;

    fd = incoming->fd;
    
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
	nexus_printf("close_incoming_with_message(): sending on fd=%d \n",
		    fd);
    }
#endif

    tracepoint("rx_send close_incoming_with_message");
    n_sent = incoming->adaptor_funcs->send(fd, close_buf, msg_size );
    save_error = errno;

#ifdef BUILD_DEBUG
    if (NexusDebug(3))
    {
	nexus_printf("close_incoming_with_message(): n_sent=%d err=%s\n",
		    n_sent,
		    n_sent < 0 ? _nx_md_system_error_string(save_error) : "<none>");
    }
#endif
    
    if (n_sent == msg_size)
    {
      	SaveRxBufferWithIncoming( incoming, buf );
	if (read_messages)
	{
	    read_incoming(incoming, ENQUEUE_MESSAGES);
	}
	/* close_incoming(incoming); NOT HERE! */
	rc = NEXUS_TRUE;
    }
    else
    {
	if (save_error == EBADF)
	{
	    /*
	     * The fd for this incoming was closed unexpectedly.
	     *
	     * So call read_incoming() to see what's going on. The choices are:
	     *   1) The process at the other end died (or
	     *      erroneously closed the fd).  In this case
	     *      read_incoming() will fatal error out.
	     *   2) The process at the other end closed
	     *	the fd, so as to make room to
	     *      open another fd.  In this case, we
	     *      just go on our merry way, since the
	     *      other end beat us to closing the fd.
	     */

	    read_incoming(incoming, ENQUEUE_MESSAGES);

	    /*
	     * read_incoming() should either close the proto
	     * cleanly, or it should fatal out.
	     */
	    NexusAssert2((fd_to_proto[fd] == (void *) NULL),
			 ("close_uincoming_with_message(): Internal error: Expected incoming to be closed\n")); 

	    rc = NEXUS_TRUE;
	}
	else
	{
	    rx_fatal("close_incoming_with_message(): Write failed: %s\n",
		      _nx_md_system_error_string(save_error));
	}
    }
    return (rc);
} /* close_incoming_with_message() */
#endif /* DONT_INCLUDE */
				       
/*
 * close_proto_with_message()
 *
 * Construct and send a CLOSE_NORMAL message.
 * Save the message in an rx_buffer, and let it go through the
 * acknowledgement protocol like any other message.  The receipt of
 * the ACK for a close message means it is time to do an 
 * adaptor_funcs->close(fd).
 *
 * The other side (the incoming) used to attempt a read on the closed fd
 * and get the ECONNRESET error, but this doesn't seem to work under
 * Solaris2.3, so a method based on timing out and the incoming side
 * is used to close that side of the connection.
 *
 * Return: NEXUS_FALSE (the proto is never closed here!)
 */
static nexus_bool_t close_proto_with_message(rx_proto_t *proto)
{
    rx_buffer_t *rx_buffer;
    int n_sent;
    nexus_bool_t rc;
    int fd;
    char *send_ptr;
    int close_type;
    int msg_size;
    int sequence_number;
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    int save_error;

    if (proto->being_closed) 
    {
        NexusAssert2((proto->ack_list_head != NULL),
            ("close_proto_with_mesage(): proto() must have a non-empty acknowledgement list\n"));
	return NEXUS_FALSE;
    }
    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,
        ("close_proto_with_message(): proto(%x)->being_closed = TRUE\n",proto));
    proto->being_closed = NEXUS_TRUE;

    /* Use a real buffer, so it can live on the ack list */
    GetRxBuffer(close_proto_with_message(), rx_buffer);
#if 1
    rx_buffer->funcs = &rx_buffer_funcs;
#endif
    rx_buffer->adaptor_funcs = proto->adaptor_funcs;
    rx_buffer->next = (rx_buffer_t *) NULL;
    rx_buffer->buffer_type = SEND_BUFFER;
    rx_buffer->sequence_number = -1;
#if 1
    rx_buffer->u.send.gp = NULL;
    rx_buffer->u.send.handler_id = 0;
    rx_buffer->u.send.handler_name = NULL;
    rx_buffer->u.send.handler_name_length = 0;
    rx_buffer->u.send.size = 0;
    rx_buffer->u.send.n_elements = -1;
#endif 
    rx_buffer->u.send.current_pointer = (char *) NULL;
    rx_buffer->u.send.base_pointer = (char *) NULL;
    rx_buffer->u.send.name_pointer = (char *) NULL;
    rx_buffer->u.send.header_size = CLOSE_MESSAGE_SIZE;
    NexusMalloc(close_proto_with_message(),
		rx_buffer->storage,
		char *,
		rx_buffer->u.send.header_size);
    rx_buffer->u.send.header_pointer = rx_buffer->storage;
    /*SetBufferPointers(rx_buffer); XXX don't do this! XXX */

    msg_size = CLOSE_MESSAGE_SIZE;
    close_type = CLOSE_NORMAL_TYPE;
    sequence_number = proto->sequence_number;
    proto->sequence_number++;

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(close_type);
    tmp_longs[2] = NX_HTONL(sequence_number);
    memcpy( rx_buffer->u.send.header_pointer, tmp_longs, msg_size );

    rx_buffer->sequence_number = sequence_number;
    rx_buffer->msg_size = msg_size;

    fd = proto->fd;

    nexus_debug_printf(2,("close_proto_with_message(): sending on fd=%d \n",
			  fd));

    send_ptr = rx_buffer->u.send.header_pointer;

    tracepoint("rx_send close_proto_with_message");
    n_sent = proto->adaptor_funcs->send(fd, send_ptr, msg_size );
#ifdef NEXUS_RX_PROTO_MN_UDP
    save_error = mn_errno;
#else
    save_error = errno;
#endif

#ifdef BUILD_DEBUG
    if (NexusDebug(3))
    {
	nexus_printf("close_proto_with_message(): n_sent=%d err=%s\n",
		    n_sent,
		    n_sent < 0 ? _nx_md_system_error_string(save_error) : "<none>");
    }
#endif
    
    if (n_sent == msg_size)
    {
	SaveRxBuffer( proto, rx_buffer, NEXUS_FALSE );
	rc = NEXUS_FALSE;
    }
    else
    {
#if DONT_INCLUDE
	if (save_error == EBADF)
	{
	
	    /*
	     * The fd for this proto was closed unexpectedly.
	     *
	     * XXX Can't just read the flag.
	     * So read one byte from the proto.
	     * It should be a CLOSE_NORMAL_FLAG.  If so, then close
	     * the fd and go on our merry way, since the other
	     * end beat us to closing the fd.  If not, then fatal out.
	     * check_proto_for_close() will do this for us.
	     */
#ifdef BUILD_DEBUG
	    if(NexusDebug(3))
		nexus_printf("close_proto_with_message(): Checking for proto close\n");
#endif
	    check_proto_for_close(proto);

	    /*
	     * check_proto_for_close() should either close the proto
	     * cleanly, or it should fatal out.
	     */
	    NexusAssert2((fd_to_proto[fd] == (void *) NULL),
			 ("close_proto_with_message(): Internal error: Expected proto to be closed\n")); 
	    
	    rc = NEXUS_TRUE;
	}
	else
#endif
	{
	    rx_fatal("close_proto_with_message(): Write failed: %s\n",
		      _nx_md_system_error_string(save_error));
	}
    }
    return (rc);
} /* close_proto_with_message() */


#ifdef DONT_INCLUDE				       
/*
 * check_proto_for_close()
 *
 * XXX Change comment to reflect RX.
 * Do a non-blocking read from the passed proto.  The options are:
 *	1) If we get a close flag (single byte == CLOSE_NORMAL_FLAG),
 *		then close the proto.
 *	2) If we get EOF, then fatal out (the process on the other end
 *		probably died)
 *	3) If we get an error (including EWOULDBLOCK), then fatal out.
 *
 * Or in other words, this routine will either fatal out, or it will
 * return after closing the proto.
 */

static void check_proto_for_close(rx_proto_t *proto)
{
    char buf[CLOSE_MESSAGE_SIZE];
    unsigned long tmp_longs[CLOSE_MSG_INTS];
    int close_type;
    int msg_size;
    int sequence_number;
    int n_read;

    rx_fatal("check_proto_for_close(): should not be called.\n");

    n_read = proto->adaptor_funcs->recv(proto->fd, buf, CLOSE_MESSAGE_SIZE);

    /*
     * n_read: is > 0 if it successfully read some bytes
     *         is < 0 on error -- need to check errno
     *         is 0 on EOF
     */
    
#ifdef BUILD_DEBUG
    if(NexusDebug(5))
	nexus_printf("check_proto_for_close(): read returned:%d\n",n_read);
#endif
    if (n_read == CLOSE_MESSAGE_SIZE)
      {
	memcpy(tmp_longs, buf, CLOSE_MESSAGE_SIZE);
	msg_size = NX_NTOHL(tmp_longs[0]);
	close_type = NX_NTOHL(tmp_longs[1]);
	sequence_number = NX_NTOHL(tmp_longs[2]);
	
	if (close_type == CLOSE_NORMAL_TYPE)
	{
	    /*
	     * We got a normal close of a proto.  So close our proto.
	     */
	    close_proto(proto);
	}
	else if (close_type == CLOSE_ABNORMAL_TYPE)
	{
	    /*
	     * We got an abnormal close of a proto.  So silent fatal out.
	     */
	    close_proto(proto);
	    rx_exit();
	    nexus_silent_fatal();
	}
	else if (close_type == CLOSE_SHUTDOWN_TYPE)
	{
	    if (shutdown_in_progress == NEXUS_FALSE)
	    {
	        close_proto(proto);
		rx_exit();
		nexus_exit(0, NEXUS_TRUE);
	    }
	}
	else
	{
	    rx_fatal("check_proto_for_close(): Internal error: Read unexpected data from a proto\n");
	}
    }
    else 
    {
      rx_fatal("check_proto_for_close(): Read failed (errno=%d): %s\n",
		errno, _nx_md_system_error_string());
    }
} /* check_proto_for_close() */
#endif /* DONT_INCLUDE */

/*
 * add_proto_with_fd()
 */
static void add_proto_with_fd(void *proto, int fd)
{
    n_fds_in_use++;
    NexusAssert(n_fds_in_use <= max_fds_to_use);
    NexusAssert(fd_to_proto[fd] == (void *) NULL);
    NexusAssert(fd >= 0 && fd < fd_tablesize);
    fd_to_proto[fd] = proto;
    FD_SET(fd, &current_fd_set);
    
#ifdef HAVE_THREAD_SAFE_SELECT
    modified_fd_table = NEXUS_TRUE;
#endif
} /* add_proto_with_fd() */


/*
 * remove_proto_with_fd()
 */
static void remove_proto_with_fd(void *proto, int fd)
{
    n_fds_in_use--;
    NexusAssert(n_fds_in_use >= 0);
    NexusAssert(fd_to_proto[fd] != (void *) NULL);
    NexusAssert(fd >= 0 && fd < fd_tablesize);
    fd_to_proto[fd] = (void *) NULL;
    FD_CLR(fd, &current_fd_set);
    
#ifdef HAVE_THREAD_SAFE_SELECT
    modified_fd_table = NEXUS_TRUE;
#endif
} /* remove_proto_with_fd() */


/*
 * make_room_for_new_fd() 
 * 
 * XXX just NexusAssert that there are enough fd's to use...  for now
 *
 * This is called immediately before opening a file descriptor
 * for a proto or incoming.  It makes sure that there are
 * enough file descriptors available (after subtracting off
 * the number reserved by the user) to open a new one.  If so,
 * then it does nothing.  If not, then it closes a proto or
 * incoming to make room for the new fd.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 */
static void make_room_for_new_fd(void)
{
#if 0 /*XXX decide what to do here */   
    int i;
    int n_tries;
    rx_proto_t *proto;
    rx_incoming_t *incoming;
    int i_am_rx_handler_thread = -1;

    if (n_deferred_closes > 0)
    {
	/*
	 * XXX Some previous proto close attempts
	 * were deferred (because the close message couldn't
	 * be written to the fd).  Try these closes again...
	 */
	n_tries = n_deferred_closes;
	for (i = 0; i < fd_tablesize && n_tries > 0; i++)
	{
	    proto = (rx_proto_t *) fd_to_proto[i];
	    if (   (proto != (rx_proto_t *) NULL)
		&& (proto->type == NEXUS_PROTO_TYPE_RX)
		&& (proto->reference_count == 0) )
	    {
		if (close_proto_with_message(proto))
		{
		    n_deferred_closes--;
		}
		n_tries--;
	    }
	}
    }
#endif    

#if 0
    while (n_fds_in_use >= max_fds_to_use)
    {
	/*
	 * We've run out of file descriptors, so we must close one.
	 *
	 * First, look for a proto (outgoing connection) that is
	 * not in the middle of sending something.  Close one
	 * if one can be found.
	 *
	 * Second, look for an incoming connection that is
	 * between message receives.  Close one if one can be found.
	 * If one cannot be found, then do a select_and_read()
	 * pass and keep retrying to find an incoming connection
	 * to free.  This should not loop infinitely, since
	 * if an incoming is in the middle of a message receipt,
	 * the rest of the message should be coming shortly.  Once
	 * that message receipt has completed, that incoming is now
	 * a candidate for closure.
	 */

	/*
	 * First, check for any outgoing protos that are not in
	 * the middle of sending a message.
	 */
	i = next_fd_to_close;
	do
	{
	    proto = (rx_proto_t *) fd_to_proto[i];
	    if (   (proto != (rx_proto_t *) NULL)
		&& (proto->type == NEXUS_PROTO_TYPE_RX)
		&& (!proto->send_in_progress) )
	    {
		/*
		 * Found one, so try to close it.
		 *
		 * The following call may or may not actually close the
		 * incoming.  But if it does close it, then n_fds_in_use
		 * will be decremented, which is what this loop is using.
		 */
		close_proto_with_message(proto);
	    }
	    i = (i + 1) % fd_tablesize;
	} while (   (n_fds_in_use >= max_fds_to_use)
		 && (i != next_fd_to_close) );
	next_fd_to_close = i;

	if (n_fds_in_use < max_fds_to_use)
	    break;
	
	/*
	 * Second, check for any incoming connections that
	 * are not in the middle of a message receive.
	 */
	do
	{
	    incoming = (rx_incoming_t *) fd_to_proto[i];
	    if (   (incoming != (rx_incoming_t *) NULL)
		&& (incoming->type == INCOMING_TYPE)
		&& (incoming->recv_state == INCOMING_RECV_HEADER)
		&& (incoming->recv_n_left == RX_MSG_HEADER_SIZE) )
	    {
		/*
		 * Found one, so try to close it.
		 *
		 * The following call may or may not actually close the
		 * incoming.  But if it does close it, then n_fds_in_use
		 * will be decremented, which is what this loop is using.
		 */
		close_incoming_with_message(incoming, NEXUS_TRUE);
	    }
	    i = (i + 1) % fd_tablesize;
	} while (   (n_fds_in_use >= max_fds_to_use)
		 && (i != next_fd_to_close) );
	next_fd_to_close = i;

	if (n_fds_in_use >= max_fds_to_use)
	{
	    /*
	     * We've tried all protos and all incomings, but
	     * couldn't find any (enough) suitable candidates
	     * to free of some fds.
	     * So try to read some stuff and/or yield
	     * the thread before trying again.  This may allow
	     * a proto or incoming to become a suitable
	     * candidate for closing.
	     */

	    if (i_am_rx_handler_thread < 0)
	    {
		_nx_i_am_rx_handler_thread(&i_am_rx_handler_thread);
		NexusAssert2((i_am_rx_handler_thread >= 0),
			     ("make_room_for_new_fd(): Internal error: _nx_i_am_rx_handler_thread() failed\n"));
	    }
	    if (i_am_rx_handler_thread)
	    {
		/*
		 * This is the handler thread.
		 * So just read some message and enqueue them.
		 * We'll handle them later when this returns.
		 */
	      {
		nexus_bool_t message_handled;

		message_handled = select_and_read(POLL_UNTIL_SELECT_FAILS, 
						  ENQUEUE_MESSAGES);
	      }
	    }
	    /*
	     * Yield to handler thread to read some stuff,
	     * or to another thread with useful work to do.
	     */
	    rx_exit();
	    nexus_thread_yield();
	    rx_enter();
	}
    }
#endif 
    NexusAssert2((n_fds_in_use < max_fds_to_use),
		 ("make_room_for_new_fd(): Internal error: n_fds_in_use(%d) >= max_fds_to_use(%d)\n", n_fds_in_use, max_fds_to_use) );
    
} /* make_room_for_new_fd() */


/*
 * delete_incoming_from_list()
 */
static void delete_incoming_from_list(rx_incoming_t *incoming)
{
    rx_incoming_t *link;

    FreeRxBuffer( incoming->ack_buf );
    incoming->ack_buf = NULL;

    if (incoming_timeout_list == incoming)
    {
        incoming_timeout_list = incoming_timeout_list->timeout_list_next;
    }
    else
    {
      link = incoming_timeout_list;
      
      while (link->timeout_list_next != incoming )
      {
	  NexusAssert2((link->timeout_list_next != NULL),
		       ("read_incoming(): Internal error: incoming(%d) is not on timeout_list\n",incoming));
	  
	  link = link->timeout_list_next;
      }
      link->timeout_list_next =
	link->timeout_list_next->timeout_list_next;
    }

} /* delete_incoming_from_list */

/*
 * select_and_read();
 *
 * Do a select on all valid file descriptors, and read from all
 * that have stuff ready to read.  Once a complete message is
 * read, invoke the handler with the message.
 *
 * If poll_type==POLL_ONE_SELECT, then only do one select and read
 * pass on the file desciptors.
 *
 * If poll_type==POLL_UNTIL_SELECT_FAILS, then do not return until
 * select returns 0.  This means that we may do several select
 * and read passes on the file descritors.
 *
 * If poll_type==POLL_BLOCKING_UNTIL_RECEIVE, then using a blocking select.
 * This will only be called when there is not a tcp handler thread,
 * and when all threads have blocked.
 *
 * If poll_type==POLL_FROM_HANDLER_THREAD, then do not ever return.
 * This should be used when calling from a separate handler thread.
 *
 * If handle_or_enqueue==HANDLE_MESSAGES, then invoke the
 * message handler immediately upon receipt of a message.
 *
 * If handle_or_enqueue==ENQUEUE_MESSAGES, then enqueue
 * any messages that are received onto the message_queue, to
 * be handled later.
 *
 * Return: NEXUS_TRUE if a message is handled, otherwise NEXUS_FALSE
 */

static nexus_bool_t select_and_read(int poll_type, int handle_or_enqueue)
{
    fd_set select_fd_set;
    fd_set select_write_fd_set;
    int select_n_ready, n_checked;
    struct timeval timeout;
    nexus_bool_t use_timeout;
    int fd;
    nexus_bool_t done = NEXUS_FALSE;
    nexus_bool_t message_handled = NEXUS_FALSE;
    int select_error;

    /*
     * Handle any enqueued messages if need be.
     */
    TestAndHandleMessages();

    while (!done)
    {
	
#ifdef BUILD_DEBUG
	if (NexusDebug(5))
	{
	    print_fd_set("Selecting on fd_set=<%s>\n", &current_fd_set);
	}
#endif

	while (1)
	{
	    select_fd_set = current_fd_set;
	    select_write_fd_set = current_write_fd_set;
	    
	    if (   poll_type == POLL_ONE_SELECT
		|| poll_type == POLL_UNTIL_SELECT_FAILS)
	    {
	        use_timeout = NEXUS_TRUE;
		timeout.tv_sec = timeout.tv_usec = 0L;
	    }
	    else if (poll_type == POLL_BLOCKING_UNTIL_RECEIVE)
	    {
	      /*
	       * Only use a blocking poll if there is nothing on
	       * the timeout_lists and therefore no retransmissions
               * need to have their tick-clocks aged.
               */
	      if (   timeout_list == NULL 
		  && incoming_timeout_list == NULL)
	      {
/*	        use_timeout = NEXUS_FALSE;*/
		use_timeout = NEXUS_TRUE; /* Hack -- don't block in select XXX*/
		timeout.tv_sec = timeout.tv_usec = 0L;
	      }
	      else
	      {
		use_timeout = NEXUS_TRUE;
		timeout.tv_sec = timeout.tv_usec = 0L;
	      }
	    }
	    else if (poll_type == POLL_FROM_HANDLER_THREAD)
	    {
#ifdef HAVE_THREAD_SAFE_SELECT
		/*
		 * This is the handler thread, and select() is
		 * a thread safe operation.  So allow other code
		 * to execute in rx while in a blocking select.
		 * But use modified_fd_table as a flag
		 * to tell if the fd_table was modified
		 * while the select was blocked.
		 *
		 * Don't block indefinitely. We want to be able
		 * to check for shutdown periodically.
		 */
		modified_fd_table = NEXUS_FALSE;
#if 0 /* XXX figure out if this is right for RX! */
		use_timeout = NEXUS_FALSE;
#else
		use_timeout = NEXUS_TRUE;
		timeout.tv_sec = timeout.tv_usec = 0L;
#endif 
		rx_exit();
#endif /* HAVE_THREAD_SAFE_SELECT */
	    }

	    /*tracepoint("select");*/

#ifdef TIMESTAMP_ECHO
	    select_tick_clock++;
#endif /* TIMESTAMP_ECHO */
#ifdef NEXUS_RX_PROTO_MN_UDP
	    select_n_ready = mn_select(fd_tablesize,
				    NEXUS_FD_SET_CAST &select_fd_set,
				    NEXUS_FD_SET_CAST &select_write_fd_set,
				    NEXUS_FD_SET_CAST NULL,
				    (use_timeout ? &timeout : NULL) );
	    select_error = mn_errno;
#else  /* NEXUS_RX_PROTO_MN_UDP */
	    select_n_ready = select(fd_tablesize,
				    NEXUS_FD_SET_CAST &select_fd_set,
				    NEXUS_FD_SET_CAST &select_write_fd_set,
				    NEXUS_FD_SET_CAST NULL,
				    (use_timeout ? &timeout : NULL) );
	    select_error = errno;
#endif /* NEXUS_RX_PROTO_MN_UDP */


	    if (   poll_type == POLL_BLOCKING_UNTIL_RECEIVE
		&& select_n_ready > 0) /* XXX added to prevent automatic returns */
            {
	      poll_type = POLL_UNTIL_SELECT_FAILS;
	    }
#ifdef HAVE_THREAD_SAFE_SELECT
	    else if (poll_type == POLL_FROM_HANDLER_THREAD)
	    {
		/*
		 * This is the handler thread, with a threadsafe select().
		 * Check to make sure the fd_table wasn't modified
		 * while the select was blocked.
		 */
		rx_enter();
/*		if (rx_done && (timeout_list == NULL) && (incoming_timeout_list == NULL))*/
		if (rx_done && (n_fds_in_use == 0))
		  return(message_handled);
		if (modified_fd_table)
		    continue;
	    }
#endif /* HAVE_THREAD_SAFE_SELECT */

	    if (select_n_ready >= 0)
	    {
		break;
	    }
	    else if (select_n_ready < 0)
	    {
	      if ((select_error == EINTR))
	      {
		;
	      }
	      else if (select_error == EPROTO) 
	      {
		/* This is needed for TARGET_ARCH_SOLARIS and
		 * TARGET_ARCH_SUNOS41, at least. 
		 *
		 * This is incredibly gross.  It looks like select() can
		 * return EPROTO because one of the rx fds is in the
		 * EPROTO state.  
		 */
		  int test_fd;
		  int psuedo_ready = 0;

		  FD_CLR(rx_local_fd, &select_fd_set); /* never try this with the TCP_PORT */
		  for (test_fd = 0; test_fd < fd_tablesize; test_fd++)
		  {
		      if (FD_ISSET(test_fd, &select_fd_set))
		      {
			  nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,
					     ("select_and_read(): psuedo_ready fd(%d)\n",test_fd));
			  psuedo_ready++;
		      }
		  }

		  if (psuedo_ready > 0)
		  {
		    select_n_ready = psuedo_ready;
		    break;
		  }
		  else
		  {
		      rx_fatal("select_and_read(): returns EPROTO with nothing set.\n",
				_nx_md_system_error_string(select_error));
		  }
	      }
	      else
	      {
		rx_fatal("select_and_read(): select failed: %s\n",
			  _nx_md_system_error_string(select_error));
	      }
	    }
	}

#ifdef BUILD_DEBUG
	if (NexusDebug(5))
	{
	    nexus_printf("Select returns %d\n", select_n_ready);
	    print_fd_set("Select returned fd_set=<%s>\n", &select_fd_set);
	    print_fd_set("Select returned fd write set=<%s>\n",
			 &select_write_fd_set);
	}
#endif
	
	for (n_checked = 0, fd = 0;
	     n_checked < select_n_ready;
	     fd++)
	{
	    NexusAssert2(fd < fd_tablesize,
			 ("select_and_read(): Internal error: fd >= fd_tablesize\n"));
	    if (FD_ISSET(fd, &select_fd_set))
	    {
#ifdef BUILD_DEBUG
		if (NexusDebug(5))
		{
			nexus_printf("select_and_read(): selected fd=%d.\n",
				    fd);
		}
#endif
		n_checked++;
		
		if ((fd == rx_local_fd) && !rx_done)
		{
		    /* Somebody wants to connect to me */
		    accept_new_connection(fd);
		    TestAndHandleMessages();
		}
		else
		{
#ifdef BUILD_DEBUG		    
		    if (((rx_proto_t *)(fd_to_proto[fd]))
			 == (rx_proto_t *) NULL)
		    {
			rx_fatal("select_and_read(): Data is ready on fd %d, but there is no proto or incoming for it\n", fd);
		    }
#endif /* BUILD_DEBUG */			

		    if (((rx_proto_t *)(fd_to_proto[fd]))->type
			 == INCOMING_TYPE)
		    {
		      tracepoint("READ_INCOMING");
			read_incoming((rx_incoming_t *) (fd_to_proto[fd]),
				      handle_or_enqueue);
		    }
		    else
		    {
#ifdef BUILD_DEBUG
			if(NexusDebug(5))
			    nexus_printf("select_and_read(): Checking for proto close\n");
#endif
			/*
			 * see what's happening on this proto
			 */
			tracepoint("READ_PROTO");
			read_proto(fd_to_proto[fd]);
#if 0
			check_proto_for_close((rx_proto_t *)
					      (fd_to_proto[fd]));
#endif
		    }
		}
	    }

	    /* 
	     * The checks for fds in select_write_fd_set are independent of
	     * select_fd_set
	     */
	    if ( FD_ISSET( fd, &select_write_fd_set ) )
	    {
	        nexus_debug_printf( 1,("select_and_read(): write fd=%d.\n", fd) );

		n_checked++;
		if ( fd == rx_local_fd )
	        {
		    rx_fatal("select_and_read(): Write select on rx_local_fd\n");
		}
		else if ( ((rx_proto_t *)(fd_to_proto[fd]))->type == INCOMING_TYPE )
 	        {
		    rx_fatal( "select_and_read(): Write select on incoming(%d)\n", fd );
		}
	        else
	        {
		    tracepoint( "continue_rsr" );
		    continue_remote_service_request( fd_to_proto[fd] );
	        }
	    }
	}

	if (   (   (poll_type == POLL_ONE_SELECT) )
	    || (   (poll_type == POLL_UNTIL_SELECT_FAILS)
		&& (select_n_ready == 0) )
	    )
	{
	    done = NEXUS_TRUE;
	}
	/*
	 * For each pass through the select() call, mark a tick
	 * off the clock of the proto's and incomings on the timeout_list
	 * or incoming_timeout_list.
	 *
	 * If the timeout_value reaches 0, then retransmit all
	 * the buffers on that proto's (incoming's) ack_list (ack_buf),
	 * and reset the timer.
	 * 
	 */
	{
	    rx_proto_t *proto = timeout_list;
	    rx_incoming_t *incoming = incoming_timeout_list;
	    int dummy_retransmit_sequence_number;

	    while (proto != NULL) 
	    {
	        if (proto->timeout_value-- == 0) 
		{
		    n_timeouts++;
		    tracepoint("RETRANSMIT_BUFFERS(PROTO)");
		    retransmit_buffers(proto->ack_list_head, proto->fd, -1,
				     &proto->last_retransmit_sequence_number);
		    /*
		     * Experimental simple exponential backoff.
		     *
		     * If timeout_reset is less than limit, double it.
		     */
		    if (proto->timeout_reset < 25600)
		    { 
		    	proto->timeout_reset *= 2;
		    }
		    proto->timeout_value = proto->timeout_reset;
		}
#ifdef TIMESTAMP_ECHO
	        if (n_rsr_buffers/100 > last_timestamp_rsr_buffers/100) 
		{
		    last_timestamp_rsr_buffers = n_rsr_buffers;
		    timestamp_echo(proto);
	        }
#endif /* TIMESTAMP_ECHO */
		    proto = proto->timeout_list_next;
    	    }

	    while (incoming != NULL)
	    {
	        if (incoming->timeout_value-- == 0)
	        {
		    if (incoming->being_closed) /* LAST timer */
		    {
		       rx_incoming_t *dead_incoming = incoming;
		    
		       incoming = incoming->timeout_list_next;
		       delete_incoming_from_list( dead_incoming );
		       nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,
					  ("LAST timer on incoming(%x)\n",
					   dead_incoming));
		       close_incoming( dead_incoming );
		    }
		    else /* Normal RTC time-out */
		    {  
		        tracepoint("RETRANSMIT_BUFFERS(INCOMING)");
			retransmit_buffers(incoming->ack_buf, incoming->fd,
					   -1,
					   &dummy_retransmit_sequence_number);
			incoming->timeout_value = incoming->timeout_reset;
			incoming = incoming->timeout_list_next;
		    }
		}
		else 
		{ 
		    incoming = incoming->timeout_list_next;
		}
	    }
	}
    }

#ifdef BUILD_DEBUG
    if (NexusDebug(3))
	nexus_printf("select_and_read(): returning. message handled=%d\n", message_handled);
#endif
    /*tracepoint("select_and_read: returns");*/
    return (message_handled);
    
} /* select_and_read() */

/*
 * retransmit_buffers:
 *
 * Just blast away for now... bandwidth is free.
 */
static void retransmit_buffers( rx_buffer_t *buf, int fd, 
			        int next_sequence_number, 
			        int *last_retransmit_sequence_number )
{
    int n_sent;
    int save_error;

#if 0
    /* Debugging check */
    if ( buf != NULL )
    {
        if ( next_sequence_number != -1 )
	{
	    if ( NX_NTOHL(*(unsigned long *)(buf->u.send.header_pointer + 8)) 
		!= next_sequence_number )
	    {
	      nexus_printf("retransmit_buffers(): next_sequence_number = %d, buffer sequence number = %d\n", 
			   next_sequence_number, NX_NTOHL(*(unsigned long *)(buf->u.send.header_pointer + 8)));

	    }
	}
    }
#endif

    while ( buf != NULL ) 
    {
        n_retransmits++;
	tracepoint("rx_send retransmit_buffers");
        n_sent = buf->adaptor_funcs->send(fd, 
					  buf->u.send.header_pointer,    
					  buf->msg_size);
#ifdef NEXUS_RX_PROTO_MN_UDP
        save_error = mn_errno;
#else
	save_error = errno;
#endif

	/* record the buffer sequence_number */
	*last_retransmit_sequence_number = buf->sequence_number;


#ifdef BUILD_DEBUG
        if (NexusDebug(3))
        {
	    nexus_printf("retransmit_buffers(): n_sent=%d err=%s\n",
			 n_sent,
			 n_sent < 0 ? _nx_md_system_error_string(save_error) : "<none>");
	}
#endif
        if ( n_sent < 0 ) 
	{

#ifdef NEXUS_RX_PROTO_MN_UDP
	    if ( ( save_error == EWOULDBLOCK ) || ( save_error == EIO ) )
	    {
	      return;
	    }
#endif
	    rx_fatal("retransmit_buffers(): Write failed (errno=%d): %s\n",
		      save_error,
		      _nx_md_system_error_string(save_error));
	}
	else if ( n_sent < buf->msg_size ) 
	{
	    rx_fatal("retransmit_buffers(): Write incomplete -- %d of %d written\n",
		      n_sent, buf->msg_size);
	}
	buf = buf->next;
     }
} /* retransmit_buffers */

#ifdef TIMESTAMP_ECHO
/*
 * timestamp_send:
 *
 * Perform a send() and fatal out if there are errors.
 */
static void timestamp_send(int fd, char *buf, int msg_size, char *caller)
{
    int n;
    int save_error;

    tracepoint("rx_send timestamp_send");
    n = rx_send( fd, buf, msg_size ); /* XXX.  Must select correct send from
				       adaptor_funcs */
    save_error = errno;

    if (n < 0)
    {
        rx_exit();
	rx_fatal("%s: rx_send failed: %s\n", caller,
		  _nx_md_system_error_string(save_error));
    }
    else if (n < msg_size)
    {
        rx_exit();
	rx_fatal("%s: rx_send sent only %d bytes\n", caller, n);
    }

} /* timestamp_send() */


/*
 * timestamp_echo:
 *
 * Send a timestamp to the host connected to the specified global pointer.
 *
 * The response will be used to calibrate the tick-clocks used by select 
 * and the round trip time of an RX_PROTO datagram.
 *
 * This is a first cut.  The actual interface should be in a nexus handler 
 * defined in another file.
 *
 * TIMESTAMP_ECHO_REQUEST_TYPE
 * TIMESTAMP_ECHO_RESPONSE_TYPE
 * format: size type seq 
 *         
 * Upon receipt of a REQUEST, immediately send an identical RESPONSE.  
 * Upon receipt of a RESPONSE, compute the time interval and the number 
 * of ticks off the select clock, and assign a new timeout_value_reset.
 *
 * The SEQ numbers will be ignored for TIMESTAMP_ECHOs
 * because it should be possible (and useful) to reset
 * the timeout interval in the middle of sending a bunch
 * of stuff.  If SEQ numbers were used, then the TIMESTAMP_ECHOs
 * would be ignored until the messages ahead of it were delivered,
 * which might make it difficult to correct the network behavior.
 * For this reason, using nexus handlers() to measure network
 * traffic probably isn't useful here.
 *
 */
static void timestamp_echo( rx_proto_t *proto )
{
/*     nexus_global_pointer_t *gp */
/*    rx_proto_t *       proto = (rx_proto_t *) gp->proto;*/
    unsigned long 	tmp_longs[TIMESTAMP_MSG_INTS];
    char 		buf[TIMESTAMP_MSG_SIZE];
    int 		msg_size = TIMESTAMP_MSG_SIZE;
    struct timeval *    tv;

    if (proto->timestamp != NULL) return;	/* No multiple timestamps */

    NexusAssert2((proto->type == NEXUS_PROTO_TYPE_RX),
		 ("timestamp_echo(): Internal error: proto_type is not NEXUS_PROTO_TYPE_RX(%d)\n",NEXUS_PROTO_TYPE_RX));

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(TIMESTAMP_ECHO_REQUEST_TYPE);
    tmp_longs[2] = NX_HTONL(-1);

    /*
     * Allocate a struct timestamp and record the current time,
     * and the tick clock in it.
     */
    
    NexusMalloc(timestamp_echo(), proto->timestamp, struct timestamp *,
		sizeof(struct timestamp));
    tv = &proto->timestamp->i0;
    gettimeofday( tv, NULL );
    proto->timestamp->ticks0 = select_tick_clock;

    memcpy( buf, tmp_longs, msg_size );

    if (proto->fd < 0)
    {
      open_proto(proto);
    }

    timestamp_send( proto->fd, buf, msg_size, "timestamp_echo()");
 
} /* timestamp_echo */

void timestamp_external_echo( nexus_global_pointer_t *gp )
{
    rx_proto_t *       proto = (rx_proto_t *) gp->proto;

    rx_enter();
    timestamp_echo( proto );
    rx_exit();

}


/*
 * timestamp_reply:
 *
 * Send a timestamp echo reply back to the proto
 * that sent the request.
 *
 */
static void timestamp_reply(int fd)
{
    unsigned long 	tmp_longs[TIMESTAMP_MSG_INTS];
    char 		buf[TIMESTAMP_MSG_SIZE];
    int 		msg_size = TIMESTAMP_MSG_SIZE;

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(TIMESTAMP_ECHO_RESPONSE_TYPE);
    tmp_longs[2] = NX_HTONL(-1);

    memcpy( buf, tmp_longs, msg_size );
/*    rx_enter();*/

    timestamp_send( fd, buf, msg_size, "timestamp_reply()");
 
/*    rx_exit();*/
  
} /* timestamp_reply() */

static void timestamp_compute_round_trip_time(rx_proto_t *proto)
{
  struct timeval i1;
  unsigned int ticks1, round_trip_ticks;
  double usecs;

  gettimeofday( &i1, NULL );
  ticks1 = select_tick_clock;

  usecs = (i1.tv_sec - proto->timestamp->i0.tv_sec) * 1000000.0;
  usecs += (i1.tv_usec - proto->timestamp->i0.tv_usec);

  round_trip_ticks = ticks1 - proto->timestamp->ticks0;
  ave_rtt_ticks = ((ave_rtt_ticks * n_rtt) + round_trip_ticks) / (n_rtt + 1);
  ave_rtt_usec = ((ave_rtt_usec * n_rtt) + usecs) / (n_rtt + 1);
  n_rtt++;
  NexusFree(proto->timestamp);
  proto->timestamp = NULL;
/*  nexus_printf("compute_round_trip_time(): rtt=%d, secs=%d, usecs=%d\n",
	       round_trip_ticks, secs, usecs);*/
   
#if 0
  /*
   * At this point all that needs to be done is to make the
   * new timeout_reset = round_trip_ticks.  But, some safeguards
   * are needed to prevent a bogus RTT from messing things up.
   * So, the minimum and maximum are hardcoded.   A more elaborate
   * policy will eventually be added to exploit the time value.
   */

#define MIN_TICKS 50
#define MAX_TICKS 100

  if (round_trip_ticks < MIN_TICKS) round_trip_ticks = MIN_TICKS;
  if (round_trip_ticks > MAX_TICKS) round_trip_ticks = MAX_TICKS;
  proto->timeout_reset = round_trip_ticks;

  if (proto->timeout_value > round_trip_ticks)
  {
    proto->timeout_value = round_trip_ticks;
  }
#endif

} /* timestamp_compute_round_trip_time() */

#endif /* TIMESTAMP_ECHO */

/*
 * cleanup_list():
 *
 * remove buffers upto sequence number from ack list, 
 * and remove this proto from the
 * timeout list when the ack list is empty.
 */
static void cleanup_list(rx_proto_t *proto, int sequence_number)
{
  rx_buffer_t *buf, *dead_buf;
  nexus_bool_t	not_empty_to_start;

  /*
   * Free up everything on ack list upto but not
   * including sequence_number.
   */
  buf = proto->ack_list_head;
  not_empty_to_start = (buf != NULL) ? NEXUS_TRUE : NEXUS_FALSE;

  while ( buf != NULL && buf->sequence_number < sequence_number ) 
  {
    dead_buf = buf;
    buf = buf->next;
    FreeRxBuffer(dead_buf);
  }
  proto->ack_list_head = buf;

  /*
   * When there are no more buffers on the list to be ACKed,
   * then go ahead and remove the proto from the timeout_list.
   */
  if ( buf == NULL && not_empty_to_start )
  {
    rx_proto_t *link;
		  
    proto->ack_list_tail = NULL;

    /*
     * Remove this rx_proto_t from the timeout_list
     */
		   
    NexusAssert2((timeout_list != NULL),
		 ("cleanup_list(): Internal error: proto(%d) got an ACK/NACK, should be on NULL timeout_list\n",proto));

    if ( timeout_list == proto )
    {
      /* This proto is the first on the list */
      timeout_list = proto->timeout_list_next;
    }
    else 
    {
      /* Find it and remove it */
      link = timeout_list;
      
      while( link->timeout_list_next != proto )
      {
	NexusAssert2((link->timeout_list_next != NULL),
		     ("read_proto(): Internal error: proto(%d) is not on timeout_list\n",proto));
			
	link = link->timeout_list_next;
      }
      link->timeout_list_next = link->timeout_list_next->timeout_list_next;
		      
    }
  }

} /* cleanup_list() */

/*
 * read_proto():
 *
 * Process acks and nacks coming in on this proto, as well as close messages.
 */
static void read_proto(rx_proto_t *proto)
{
    int n_read;
    int recv_error;
#if 0
    int n_sent;
#endif
    unsigned long tmp_longs[RX_DISPATCH_INTS];
    int msg_size, msg_type, sequence_number;
    char tmp_buffer[RX_DISPATCH_SIZE];
#if 0
    rx_buffer_t *buf,*dead_buf;
    nexus_bool_t not_empty_to_start;
#endif

    do
    {
      NexusAssert2((proto->fd >= 0),("read_proto(): Invalid fd(%d)\n",proto->fd));
      NexusAssert2((tmp_buffer[0]==tmp_buffer[0]),("read_proto(): Invalid tmp_buffer\n"));

        /*tracepoint("RX_RECV: READ_PROTO");*/
	n_read = proto->adaptor_funcs->recv(proto->fd,
					    tmp_buffer,
					    RX_DISPATCH_SIZE);
#ifdef NEXUS_RX_PROTO_MN_UDP
        recv_error = mn_udp_recv_errno;
#else  /* NEXUS_RX_PROTO_MN_UDP */
        recv_error = errno; /* XXX fix for FORE API and UDP */
#endif /* NEXUS_RX_PROTO_MN_UDP */

	/*
	 * n_read: is > 0 if it successfully read some bytes
	 *         is < 0 on error -- need to check errno
	 *         is 0 on EOF
	 */
#ifdef BUILD_DEBUG
	if (NexusDebug(2))
	{
	    nexus_printf("read_proto(): Read on fd=%d returns %d\n",
			proto->fd,
			n_read);
	    if (n_read > 0)
	    {
		int i;

		tmpbuf1[0] = '\0';
		for (i = 0; i < 60 && i < n_read ; i++)
		{
		    sprintf(tmpbuf2, "%u ",
			    (unsigned char) tmp_buffer[i]);
		    strcat(tmpbuf1, tmpbuf2);
		}

		nexus_printf("read_proto(): Beginning of message: %s\n",
			    tmpbuf1);
	    }
	}
#endif /* BUILD_DEBUG */

	if (n_read == RX_DISPATCH_SIZE)
	{
	      /*
	       * Extract the dispatch fields
	       */

	      memcpy(tmp_longs, tmp_buffer, RX_DISPATCH_SIZE);
	      msg_size = NX_NTOHL(tmp_longs[0]);
	      msg_type = NX_NTOHL(tmp_longs[1]);
	      sequence_number = NX_NTOHL(tmp_longs[2]);

	      if ((msg_type == ACK_TYPE) || (msg_type == NACK_TYPE))
	      {
		if (msg_type == ACK_TYPE) {
		  n_acks++;
		  tracepoint("ACK_TYPE");

		  /* Mechanism to reduce timeout_reset */
		  if (( proto->timeout_reset >= 400) &&
		     ( proto->last_retransmit_sequence_number != 
		       sequence_number ))
		  {
		    proto->timeout_reset /= 2;
		  }
		} else {
		  n_nacks++;
		  tracepoint("NACK TYPE");
		}
		cleanup_list( proto, sequence_number );
		
		/*
		 * If we have ACK'ed everything on the ack_list, and 
		 * being_closed == TRUE, then we are now ready to close down.
		 * So just do it...
		 */
		if ( proto->ack_list_head == NULL &&
		    proto->being_closed == NEXUS_TRUE )
		{
		  close_proto(proto);
		  proto_table_delete(proto);
		  /* XXX This NexusFree() caused the context->master_gp to
		   * have a dangling reference to a free'd proto.
		   * Removing it solves that problem, but I don't know why
		   * the reference count went to zero in the first place.
		   */
		  /*NexusFree(proto);*/
		  n_deferred_closes--;
		  n_read = 0; /* exit loop */
		}
	      }
		
	      /*
	       * Retransmit starting with NACK'ed sequence_number
	       */
	      if (msg_type == NACK_TYPE) 
	      {
		retransmit_buffers(proto->ack_list_head, proto->fd, 
				   sequence_number, 
				   &proto->last_retransmit_sequence_number);
	      }
#if 0
	      /* 
	       * Check to issue the final shutdown signal...
	       */
	      if (rx_done 
		  && (timeout_list == NULL) 
		  && (incoming_timeout_list == NULL)) 
	      {
		nexus_mutex_lock(&shutdown_completed_mutex);
		nexus_cond_signal(&shutdown_completed_cond);
		nexus_mutex_unlock(&shutdown_completed_mutex);
	      }
#endif 0
#ifdef TIMESTAMP_ECHO
	      else if (msg_type == TIMESTAMP_ECHO_RESPONSE_TYPE)
	      {
		timestamp_compute_round_trip_time( proto );
	      }
#endif /* TIMESTAMP_ECHO */
	      else if (msg_type == REQ_TO_CLOSE_NORMAL_TYPE ) 
	      {
		/*tracepoint("REQ_TO_CLOSE_NORMAL");*/
		close_proto_with_message(proto); /* error causing? */
	      }
	      else if (msg_type == REQ_TO_CLOSE_SHUTDOWN_TYPE )
	      {
		  if (shutdown_in_progress == NEXUS_FALSE)
		  {
		      /*tracepoint("REQ_TO_CLOSE_SHUTDOWN");*/
		      close_proto_with_message(proto);
		      /*
		       * This will cause everything to shutdown, including
		       * rx_shutdown() in this protocol module. 
		       */
		      rx_exit();
		      nexus_exit(0, NEXUS_TRUE); 
		  }
	      }
	      else if (msg_type == CLOSE_ABNORMAL_TYPE )
	      {
		  close_proto(proto);
		  rx_exit();
		  nexus_silent_fatal();
	      }
	      else if (msg_type == CLOSE_NORMAL_TYPE )
	      {
		  rx_fatal("read_proto(): clueless\n");
 	      }
	      else if (msg_type == CLOSE_SHUTDOWN_TYPE )
	      {
		rx_fatal("read_proto(): clueless\n");
	      }
	}
	else if (n_read > 0)
	{
	    rx_fatal("read_proto(): got fewer bytes for fd %d than expected %d\n",
		      proto->fd,
		      n_read);
	}
	else if (n_read == 0) 
	{
	  /* This fatal's out in read_incoming */
	  ;
	  /*rx_fatal("read_proto(): clueless\n");*/
	}
	else /* n_read < 0 */
	{
	  if (recv_error == EWOULDBLOCK) 
	  {
	    ;
	  }
/*	  else 
	  {
	    ;
	  }*/
	  else
	  {
	    if (recv_error == EPROTO && proto->being_closed && 
		proto->ack_list_head == proto->ack_list_tail)
	    {
	      /*
	       * This is copied from the normal case above.
	       * This is a somewhat strange but valid path to closing.
	       */
	      cleanup_list( proto, proto->sequence_number + 1 );
	      close_proto( proto );
	      proto_table_delete( proto );
	      /* NexusFree( proto ); */
	      n_deferred_closes--;
	      n_read = 0; /* exit loop */
	    }
	    else if ((recv_error == EPROTO) && (proto->being_closed)) 
	    {
	      rx_fatal("read_proto(%x): Read failed during close sequence, ack_list_head = %x, _tail = %x\n", proto, proto->ack_list_head, proto->ack_list_tail);
	    }
	    else
	    {
	      rx_fatal("read_proto(): Read failed (errno=%d): %s\n",
		      recv_error, _nx_md_system_error_string(recv_error));
	    }
	  }
	}
    } while (n_read > 0);

} /* read_proto() */

/*
 * generate_ack()
 *
 * Send an acknowledgement message back to the other side of this
 * rx_incoming_t.  This acks all messages less than sequence_number,
 * and in the case of a NACK, explicitly asks for the message containing
 * sequence_number to be sent again.
 *
 */
static void generate_ack(rx_incoming_t *incoming, int sequence_number, 
			 int ack_type)
{
    int n_sent;
    int fd;
    char buf[RX_DISPATCH_SIZE];
    int msg_size = RX_DISPATCH_SIZE;
    unsigned long tmp_longs[RX_DISPATCH_INTS];
    int save_error;

    tmp_longs[0] = NX_HTONL(msg_size);
    tmp_longs[1] = NX_HTONL(ack_type);
    tmp_longs[2] = NX_HTONL(sequence_number);
    memcpy( buf, tmp_longs, msg_size );

    fd = incoming->fd;

#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
	nexus_printf("generate_ack( %s, %d ): sending on fd=%d \n",
		     ((ack_type == ACK_TYPE) ? "ACK" : "NACK"), 
		     sequence_number, 
		     fd);
    }
#endif

    if ( ack_type == ACK_TYPE )
    {
        tracepoint("GENERATE ACK");
    }
    else
    {
        tracepoint("GENERATE NACK");
    }

    n_sent = incoming->adaptor_funcs->send(fd, buf, msg_size );
#ifdef NEXUS_RX_PROTO_MN_UDP
    save_error = mn_errno;
#else
    save_error = errno;
#endif

#ifdef BUILD_DEBUG
    if (NexusDebug(3))
    {
	nexus_printf("generate_ack(): n_sent=%d err=%s\n",
		    n_sent,
		    n_sent < 0 ? _nx_md_system_error_string(save_error) : "<none>");
    }
#endif
    
    if (n_sent != msg_size)
    {
      rx_fatal("generate_ack(): clueless\n");
    }
} /* generate_ack() */

/*
 * incoming_wait_for_last
 *
 * Add a dummy buffer to the incoming incoming_timeout_list
 * that will act as a timer.  When this timer goes off,
 * call close_incoming() on the connection.
 *
 * If the LAST message is received, cancel the timer and
 * call close_incoming()
 *
 * If any traffic is received on the connection besides
 * the last message, reset the timer.
 */
static void incoming_wait_for_last(rx_incoming_t *incoming)
{
    rx_buffer_t *buf;
    char *dummy_buf;

    NexusMalloc(incoming_wait_for_last(), dummy_buf, char *, 
		CLOSE_MESSAGE_SIZE);
    GetRxBuffer(incoming_wait_for_last(), buf);
    buf->adaptor_funcs = incoming->adaptor_funcs;
    buf->storage = dummy_buf;
    buf->u.send.header_pointer = dummy_buf;
    /* normally would fill in buffer here */
    buf->sequence_number = 0;
    buf->msg_size = 0;
    
    /*
     * This starts the LAST timer up using this incoming/buffer pair.
     * What distinguises this as the LAST timer is that being->closed == TRUE.
     * XXX Is the timeout being used to small?  Perhaps it should be 
     * artificially increased...
     */
    SaveRxBufferWithIncoming(incoming, buf);
    /* 
     * This is really bad.  It breaks any encapsultion of the timeout_value
     * provided.  A better way to specify and control timeout_values needs
     * to be added.
     */
    incoming->timeout_value *= 10;
    incoming->timeout_reset *= 10;

} /* incoming_wait_for_last */

/*
 * read_incoming()
 *
 * Read as much of a complete message as we can for this incoming connection.
 * The various stages of reading a complete message are:
 *
 * XXX Change to describe RX
 *   1) Blocking rx_recv of RX_DISPATCH_SIZE header. (size,type,seq)
 *   2) Check sequence number.  Discard and nack if mismatch.
 *   3) Check for close message, or ack/nack.
 *        Continue if a normal rsr message.
 *   4) Blocking rx_recv of remainder of message in pieces.
 *        Go around the loop, reading the various pieces.
 *   5) Allocate a message buffer
 *   6) Extract the handler name.
 *   7) Extract the message body.
 *   8) Enqueue or handle complete message.
 *   9) Reset incoming for reading a new message.
 */

static void read_incoming(rx_incoming_t *incoming, int handle_or_enqueue)
{
    int n_read;
    unsigned long tmp_longs[RX_MSG_HEADER_INTS];
    int msg_size, msg_type, sequence_number;
#if 0
    char tmp_buf[ATM_MAX_TRANSMISSION_UNIT];
#endif
    char *recv_buf_current_frag_hack;
    int read_incoming_recv_error;

    do
    {

      if (incoming->recv_state == INCOMING_RECV_CLOSED)
	return;

      NexusAssert2((   (incoming->recv_state == INCOMING_RECV_DISPATCH)
		    || (incoming->recv_state == INCOMING_RECV_HEADER)
		    || (incoming->recv_state == INCOMING_RECV_HANDLER)
		    || (incoming->recv_state == INCOMING_RECV_BODY)
		    || (incoming->recv_state == INCOMING_RECV_DISCARD)
		    || (incoming->recv_state == INCOMING_RECV_ENDFRAG)
		    || (incoming->recv_state == INCOMING_RECV_FRAG)),
		   ("read_incoming(): Internal error: Invalid incoming->recv_state = %d\n", incoming->recv_state) );

      NexusAssert2((incoming->fd >= 0),("read_incoming(): invalid fd(%d)\n",
					incoming->fd));

/*#define rx_string(x) # x*/

     if (incoming->recv_state == INCOMING_RECV_DISPATCH)
     {
       tracepoint("state is INCOMING_RECV_DISPATCH");
     }
     else if (incoming->recv_state == INCOMING_RECV_HEADER)
     {
       tracepoint("state is INCOMING_RECV_HEADER");
     }
     else if (incoming->recv_state == INCOMING_RECV_HANDLER)
     {
       tracepoint("state is INCOMING_RECV_HANDLER");
     }
     else if (incoming->recv_state == INCOMING_RECV_BODY)
     {
       tracepoint("state is INCOMING_RECV_BODY");
     }
     else if (incoming->recv_state == INCOMING_RECV_DISCARD)
     {
       tracepoint("state is INCOMING_RECV_DISCARD");
     }
     else if (incoming->recv_state == INCOMING_RECV_ENDFRAG)
     {
       tracepoint("state is INCOMING_RECV_ENDFRAG");
     }
     else if (incoming->recv_state == INCOMING_RECV_FRAG)
     {
       tracepoint("state is INCOMING_RECV_FRAG");
     }

#if 0
      {
	/* This block of code reads and writes from the arguments
	 * to adaptor_funcs->recv().  It should help debug the case 
	 * where recv() returns bad address, errno=14
         */
	int range_i;
	char range_value;

	for (range_i = 0; range_i < incoming->recv_n_left; range_i++) {
	  range_value = *(incoming->recv_buf_current + range_i);
	  range_value = (char) 0;
	  *(incoming->recv_buf_current + range_i) = range_value;
	}
      }
#endif 
	n_read = incoming->adaptor_funcs->recv(incoming->fd,
					       incoming->recv_buf_current,
					       incoming->recv_n_left);
#ifdef NEXUS_RX_PROTO_MN_UDP
        read_incoming_recv_error = mn_udp_recv_errno;
#else  /* NEXUS_RX_PROTO_MN_UDP */
        read_incoming_recv_error = errno;
#endif /* NEXUS_RX_PROTO_MN_UDP */

	/*
	 * n_read: is > 0 if it successfully read some bytes
	 *         is < 0 on error -- need to check errno
	 *         is 0 on EOF
	 */

#ifdef BUILD_DEBUG
	if (NexusDebug(2))
	{
	    nexus_printf("read_incoming(): Read on fd=%d returns %d\n",
			incoming->fd,
			n_read);
	    if (n_read > 0)
	    {
		int i;

		tmpbuf1[0] = '\0';
		for (i = 0; i < 60 && i < n_read ; i++)
		{
		    sprintf(tmpbuf2, "%u ",
			    (unsigned char) incoming->recv_buf_current[i]);
		    strcat(tmpbuf1, tmpbuf2);
		}

		nexus_printf("read_incoming(): Beginning of message: %s\n",
			    tmpbuf1);
	    }
	}
#endif /* BUILD_DEBUG */
	
	if ( n_read == incoming->recv_n_left ) /* Got what we expected */
	{
	    if ( incoming->recv_state == INCOMING_RECV_DISCARD )
	    {
		NexusFree(incoming->recv_buf_current);
		if (incoming->recv_buf == NULL)
		{
		  ResetIncoming(incoming);
		}
		else
		{
		  /* partial reset, since we are processing fragments */
		  incoming->recv_buf_current = recv_buf_current_frag_hack;
		  recv_buf_current_frag_hack = NULL;
		  incoming->recv_state = INCOMING_RECV_DISPATCH;
		  incoming->recv_n_left = RX_DISPATCH_SIZE;
		}
		continue;
	    }
	    else if ( incoming->recv_state == INCOMING_RECV_DISPATCH )
	    {
	      /*
	       * Extract the dispatch fields
	       */

	      if (incoming->recv_buf == NULL)
	      {
		memcpy(tmp_longs, incoming->header_buf, RX_DISPATCH_SIZE);
	      }
	      else
	      {
		memcpy(tmp_longs, incoming->recv_buf_current, RX_DISPATCH_SIZE);
	      }
	      msg_size = NX_NTOHL(tmp_longs[0]);
	      msg_type = NX_NTOHL(tmp_longs[1]);
	      sequence_number = NX_NTOHL(tmp_longs[2]);

#ifdef TIMESTAMP_ECHO
	      /*
	       * Handle a timestamp_echo message.  This happens before
               * checking the sequence number because timestamp_echo
	       * doesn't generate ack or nack.
	       */

              if (msg_type == TIMESTAMP_ECHO_REQUEST_TYPE)
	      {
		timestamp_reply( incoming->fd );
	        continue;
	      }
#endif /* TIMESTAMP_ECHO */

	      /*
	       * Check for sequence number, and generate ack or nack
	       * Also, whenever a datagram is sucessfully received,
	       * allow NACKs to be sent once again.  If a NACK is 
	       * generated, then disable NACKSs.
	       */

	      if ( sequence_number == incoming->sequence_number ) 
	      {
		  incoming->supress_nacks = NEXUS_FALSE;
		  incoming->sequence_number++;
		  if (   ( msg_type != RSR_FRAG_TYPE )
		      && ( msg_type != RSR_OPENFRAG_TYPE ) )
		  {
		      generate_ack( incoming, incoming->sequence_number, ACK_TYPE );
#if 0
		      /* Hack to generate ACKS anyways */
		  }
		  else
		  {
		      generate_ack( incoming, incoming->sequence_number, ACK_TYPE );
#endif
		  }
	      }
	      else
	      {
#if 0
		  {
		      char *s;
		      /* XXX this is a core leak. */
		      s = (char *) malloc( 30 );
		      strcpy( s, "NACK " );
		      sprintf( s+5, "%d %d", sequence_number, 
			       incoming->sequence_number );
		      tracepoint( s );
		  }
#endif 
		  if ( !incoming->supress_nacks ) 
		  {
		      generate_ack( incoming, 
			  	    incoming->sequence_number, 
				    NACK_TYPE );
		      incoming->supress_nacks = NEXUS_TRUE;
		  }
		/* 
		 * discard rest of message 
		 */
		incoming->recv_state = INCOMING_RECV_DISCARD;
		/*incoming->recv_buf_current = tmp_buf; XXX we could overrun tmp_buf */
		if (incoming->recv_buf != NULL)
		{
		  /* Save the current pointer, and restore it after the
		   * INCOMING_RECV_DISCARD state has flushed the rest of
		   * this datagram.
		   */
		  recv_buf_current_frag_hack = incoming->recv_buf_current;
		  incoming->recv_buf_current = NULL;
		}
		NexusMalloc(read_incoming(),
			    incoming->recv_buf_current,
			    char *,
			    ATM_MAX_TRANSMISSION_UNIT);
		if ((msg_type != RSR_OPENFRAG_TYPE))
		{
		  incoming->recv_n_left = msg_size - RX_DISPATCH_SIZE;
	        }
		else
		{
		  /* to handle case where msg_size includes other fragments */
		  incoming->recv_n_left = ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE;
		}
		/*tracepoint("NACK'ed");*/
		continue;
	      }

	      /* Check for a close message */
	      if ((msg_type == CLOSE_NORMAL_TYPE) || 
		  (msg_type == CLOSE_ABNORMAL_TYPE) ||
		  (msg_type == CLOSE_SHUTDOWN_TYPE))
	      {
		/*tracepoint("CLOSE_TYPE");*/
		n_read = 0;	/* exit loop */
		if ((msg_type == CLOSE_NORMAL_TYPE) ||
		    (msg_type == CLOSE_SHUTDOWN_TYPE))
		{
		  if (incoming->ack_buf) 
		    /* && incoming->ack_buf->sequence_number == sequence_number 
		     * (Check to see that this close message cancels the RTC)
		     */
		  {
		    rx_incoming_t *link;

		    FreeRxBuffer( incoming->ack_buf );
		    incoming->ack_buf = NULL;

		    if (incoming_timeout_list == incoming)
		    {
		      incoming_timeout_list = incoming_timeout_list->timeout_list_next;
		    }
		    else
		    {
		      link = incoming_timeout_list;

		      while (link->timeout_list_next != incoming )
		      {
			NexusAssert2((link->timeout_list_next != NULL),
				   ("read_incoming(): Internal error: incoming(%d) is not on timeout_list\n",incoming));

			link = link->timeout_list_next;
		      }
		      link->timeout_list_next =
			link->timeout_list_next->timeout_list_next;
		    }
		  }
		}
		if (msg_type == CLOSE_NORMAL_TYPE)
		{
		    /*
		     * We got a close message on this fd due to either:
		     *   1) the node at the other end exited normally
		     *   2) the node at the other end closed this
		     *      socket in order to open another one
		     * So close the incoming _without_ first sending
		     * the close message down the fd.  (i.e., Just
		     * close down my end of the fd that the other
		     * end already closed.)
		     */
		    /*close_incoming(incoming); NOT HERE */
		  nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("read incoming: incoming(%x)->being_closed = TRUE\n",incoming));
		  incoming->being_closed = NEXUS_TRUE;
		  /* start timeout wait for LAST message */
		  incoming_wait_for_last(incoming);
		}
		else if (msg_type == CLOSE_ABNORMAL_TYPE)
		{
		    /*
		     * We got an abnormal close message on this fd.
		     * This means some other node fataled out,
		     * and sent this down the pipe just before dieing.
		     * So fatal out silently, since the node that
		     * generated this message will have complained
		     * already.  (No need for cascading fatal messages.)
		     */
		    close_incoming(incoming);
		    rx_exit();
		    nexus_silent_fatal();
		}
		else if (msg_type == CLOSE_SHUTDOWN_TYPE)
		{
		    /*
		     * We got a shutdown close message on this fd.
		     * This means some other node is shutting down
		     * and is telling us to do the same.
		     * So shutdown and propogate it to other nodes.
		     */
		    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("read incoming: incoming(%x)->being_closed = TRUE\n",incoming));
		    incoming->being_closed = NEXUS_TRUE;
		    /* start timeout wait for LAST message */
		    incoming_wait_for_last(incoming);
		    if (shutdown_in_progress == NEXUS_FALSE)
		    {
			rx_exit();
			nexus_exit(0, NEXUS_TRUE);
		    }
		}
	      }
	      else if ((msg_type == RSR_OPEN_TYPE) 
		    || (msg_type == RSR_OPENFRAG_TYPE))
	      {
		/*tracepoint("RSR_OPEN[FRAG]_TYPE");*/
		/* Setup to grab the rest of the header */
		incoming->recv_state = INCOMING_RECV_HEADER;
		incoming->recv_n_left = RX_MSG_HEADER_SIZE - RX_DISPATCH_SIZE;
		incoming->recv_buf_current += RX_DISPATCH_SIZE;
		continue;
	      }
	      else if (msg_type == RSR_FRAG_TYPE)
	      {
		/*tracepoint("RSR_FRAG_TYPE");*/
		incoming->recv_state = INCOMING_RECV_FRAG;
		incoming->recv_n_left = ATM_MAX_TRANSMISSION_UNIT - RX_DISPATCH_SIZE;
		continue;
	      }
	      else if (msg_type == RSR_ENDFRAG_TYPE)
	      {
		/*tracepoint("RSR_ENDFRAG_TYPE");*/
		incoming->recv_state = INCOMING_RECV_ENDFRAG;
		incoming->recv_n_left = msg_size - RX_DISPATCH_SIZE;
		continue;
	      }
	      else
	      {
		rx_fatal("read_incoming(): Internal error: Got an illegal RX message type: %d\n", (int) msg_type);
	      }
	    }
	    else if (incoming->recv_state == INCOMING_RECV_HEADER)
	    {
		    /* We have completed reading the header. */
		    rx_buffer_t *buf;

		    /* Get a buffer and initialize it */
		    GetRxBuffer(read_incoming(), buf);
#ifdef BUILD_DEBUG
		    if(NexusDebug(3))
		    {
		      nexus_printf("read_incoming(): Allocating buffer for incoming message:%x\n", buf);
		    }
#endif
		    buf->adaptor_funcs = incoming->adaptor_funcs;
		    buf->buffer_type = RECV_BUFFER;
		    buf->u.recv.stashed = NEXUS_FALSE;

		    /* Extract the rest of the message header information */
		    memcpy(tmp_longs + RX_DISPATCH_INTS, incoming->recv_buf_current,
			   RX_MSG_HEADER_SIZE - RX_DISPATCH_SIZE);

		    buf->u.recv.size = msg_size;
		    buf->use_xdr = NX_NTOHL(tmp_longs[3]);
		    buf->u.recv.context = NX_NTOHL(tmp_longs[4]);
		    buf->u.recv.address = NX_NTOHL(tmp_longs[5]);
		    buf->u.recv.handler_id = (int) NX_NTOHL(tmp_longs[6]);
		    buf->u.recv.handler_name_length = NX_NTOHL(tmp_longs[7]);
#ifdef BUILD_PROFILE		    
		    buf->u.recv.node_id = (int) NX_NTOHL(tmp_longs[8]);
		    buf->u.recv.context_id = (int) NX_NTOHL(tmp_longs[9]);
#endif		    
		    if(buf->use_xdr) buf->funcs = &rx_xdr_buffer_funcs;
		    else buf->funcs = &rx_buffer_funcs;
		    
		    /* Adjust size downward */
		    buf->u.recv.size -= ( RX_MSG_HEADER_SIZE + buf->u.recv.handler_name_length);
#ifdef BUILD_DEBUG
		    if (NexusDebug(3))
			nexus_printf("read_incoming(): got size=%d context=%x address=%x handler_id=%ld handler_name_length=%ld\n",
				     buf->u.recv.size,
				     buf->u.recv.context,
				     buf->u.recv.address,
				     (unsigned long) buf->u.recv.handler_id,
				     buf->u.recv.handler_name_length);
#endif
		    /*
		     * Allocate memory to hold the buffer.
		     */
		    if (buf->u.recv.size > 0)
		    {
			NexusMalloc(read_incoming(), buf->storage,
				    char *, buf->u.recv.size);
			buf->u.recv.current_pointer = buf->storage;
		    }
		    else
		    {
			buf->storage = (char *) NULL;
			buf->u.recv.current_pointer = (char *) NULL;
		    }
		    
		    /*
		     * Reset the incoming's recv_state
		     * to INCOMING_RECV_HANDLER
		     */
		    incoming->recv_state = INCOMING_RECV_HANDLER;
		    incoming->recv_buf = buf;
		    incoming->recv_buf_current = &(incoming->header_buf[0]);
		    incoming->recv_n_left = buf->u.recv.handler_name_length;
#ifdef BUILD_DEBUG
		    if(NexusDebug(3)) {
		      nexus_printf("read_incoming(): State now: INCOMING_RECV_HANDLER  expecting: %d bytes\n", incoming->recv_n_left );
		    }
#endif
	    }
	    else if (incoming->recv_state == INCOMING_RECV_HANDLER)
	    {
	            rx_buffer_t *buf = incoming->recv_buf;

		    /*
		     * NULL terminate the handler name,
		     * and lookup the handler function.
		     */
		    incoming->header_buf[buf->u.recv.handler_name_length]
			= '\0';
#ifdef BUILD_DEBUG
		    if(NexusDebug(3)) {
		      nexus_printf("read_incoming(): got call to handler:%s\n",
				   incoming->header_buf );
		    }
#endif
		    _nx_lookup_handler(buf->u.recv.context,
				       &(incoming->header_buf[0]),
				       buf->u.recv.handler_id,
				       &(buf->u.recv.handler_type),
				       &(buf->u.recv.handler_func) );
		    if (buf->u.recv.handler_type == NEXUS_HANDLER_TYPE_THREADED)
		    {
			buf->u.recv.stashed = NEXUS_TRUE;
		    }
		    if (buf->u.recv.handler_func == (nexus_handler_func_t)NULL)
		    {
			/*
			 * Only copy the handler name into the receive
			 * buffer if the handler doesn't exist, so that
			 * it can be passed to _nx_handle_message().
			 */
			buf->u.recv.handler_name
			    = _nx_copy_string(&(incoming->header_buf[0]));
		    }
		    else
		    {
			buf->u.recv.handler_name = (char *) NULL;
		    }
		    
		    if (buf->u.recv.size > 0)
		    {
			/*
			 * Reset the incoming's recv_state
			 * to INCOMING_RECV_BODY
			 */
			if (msg_type == RSR_OPEN_TYPE)
			{
			  incoming->recv_state = INCOMING_RECV_BODY;
			  incoming->recv_buf_current = buf->storage;
			  incoming->recv_n_left = buf->u.recv.size;
			}
			else /* msg_type == RSR_OPENFRAG_TYPE */
			{
			  incoming->recv_state = INCOMING_RECV_FRAG;
			  incoming->recv_buf_current = buf->storage;
			  incoming->recv_n_left = ATM_MAX_TRANSMISSION_UNIT
			    - RX_MSG_HEADER_SIZE 
			    - buf->u.recv.handler_name_length;
			}
#ifdef BUILD_DEBUG
			if(NexusDebug(3)) {
			  nexus_printf("read_incoming(): State now: INCOMING_RECV_BODY  expecting: %d bytes\n", incoming->recv_n_left );
			}
#endif
		    }
		    else
		    {
			/* Zero length message, so message is complete */
			
			/*
			 * Reset the incoming's recv_state
			 * to INCOMING_RECV_HEADER
			 */
			ResetIncoming(incoming);
			if( buf->use_xdr ) {
			    xdrmem_create(&(buf->xdrs),
					  buf->u.recv.current_pointer,
					  buf->u.recv.size, XDR_DECODE );
			}
			
			/* Enqueue or handle the message */
			if (handle_or_enqueue == HANDLE_MESSAGES)
			{
#ifdef RX_PROTO_SINGLE_THREADED
			    handle_in_progress = NEXUS_TRUE;
#endif
			    rx_exit();
			    _nx_handle_message(buf->u.recv.handler_name,
					       buf->u.recv.handler_id,
					       buf->u.recv.handler_type,
					       buf->u.recv.handler_func,
					       buf->u.recv.context,
					       buf->u.recv.address,
#ifdef BUILD_PROFILE
					       buf->u.recv.node_id,
					       buf->u.recv.context_id,
					       buf->u.recv.size,
#endif
					       (void *) buf);
			    rx_enter();
#ifdef RX_PROTO_SINGLE_THREADED
			    handle_in_progress = NEXUS_FALSE;
#endif
			}
			else
			{
			    EnqueueMessage(buf);
			}
 		    /* 
		     * We have received an entire message and
		     * either handled or enqueued it.
		     * So set n_read=0 to break out of the read loop.
		     * If there are any more messages coming in on 
		     * this fd, we will catch them on the next
		     * select() go-around.
		     */
		        n_read = 0;
			
		    }
	    }
	    else if (incoming->recv_state == INCOMING_RECV_FRAG)
	    {
	      /* We have received the next chunk of the body from
	       * a fragment.  Advance the recv pointers, and
	       * go to the INCOMING_RECV_DISPATCH state.
	       */
	      incoming->recv_buf_current += incoming->recv_n_left;
	      incoming->recv_state = INCOMING_RECV_DISPATCH;
	      incoming->recv_n_left = RX_DISPATCH_SIZE;
	    }
	    else if (incoming->recv_state == INCOMING_RECV_ENDFRAG)
	    {
	      rx_buffer_t *buf = incoming->recv_buf;
	      ResetIncoming(incoming);
		    /* Enqueue or handle the message */
		    if (handle_or_enqueue == HANDLE_MESSAGES)
		    {
#ifdef RX_PROTO_SINGLE_THREADED
			handle_in_progress = NEXUS_TRUE;
#endif
			rx_exit();
			_nx_handle_message(buf->u.recv.handler_name,
					   buf->u.recv.handler_id,
					   buf->u.recv.handler_type,
					   buf->u.recv.handler_func,
					   buf->u.recv.context,
					   buf->u.recv.address,
#ifdef BUILD_PROFILE
					   buf->u.recv.node_id,
					   buf->u.recv.context_id,
					   buf->u.recv.size,
#endif
					   (void *) buf);
			rx_enter();
#ifdef RX_PROTO_SINGLE_THREADED
			handle_in_progress = NEXUS_FALSE;
#endif
		    }
		    else
		    {
			EnqueueMessage(buf);
		    }
	    }
	    else /* (incoming->recv_state == INCOMING_RECV_BODY) */
	    {
	      
		    /* We have completed reading the body */
		    rx_buffer_t *buf = incoming->recv_buf;

		    /*
		     * Reset the incoming's recv_state
		     * to INCOMING_RECV_HEADER.
		     * This needs to be done before the
		     * call to _nx_handle_message(), so
		     * that everything is in a consistent
		     * state when we rx_exit() for
		     * the message handle.
		     */
		    ResetIncoming(incoming);

		    /* Enqueue or handle the message */
		    if (handle_or_enqueue == HANDLE_MESSAGES)
		    {
#ifdef RX_PROTO_SINGLE_THREADED
			handle_in_progress = NEXUS_TRUE;
#endif
			rx_exit();
			_nx_handle_message(buf->u.recv.handler_name,
					   buf->u.recv.handler_id,
					   buf->u.recv.handler_type,
					   buf->u.recv.handler_func,
					   buf->u.recv.context,
					   buf->u.recv.address,
#ifdef BUILD_PROFILE
					   buf->u.recv.node_id,
					   buf->u.recv.context_id,
					   buf->u.recv.size,
#endif
					   (void *) buf);
			rx_enter();
#ifdef RX_PROTO_SINGLE_THREADED
			handle_in_progress = NEXUS_FALSE;
#endif
		    }
		    else
		    {
			EnqueueMessage(buf);
		    }
	    }
	}
	else if (n_read > 0)
	{
	    rx_fatal("read_incoming(): in state %d, got fewer bytes (%d) for fd(%d) than expected (%d)\n",
		      incoming->recv_state,
		      n_read,
		      incoming->fd,
		      incoming->recv_n_left);
	}
	else if (n_read == 0)
	{
	    /*
	     * Got EOF on the read, so die...
	     *
	     * We should never get an EOF on read under normal
	     * circumstances.  We should always get (and handle) a
	     * close message (the leading byte of a message) before
	     * a fd is closed, so we can handle the closed fd
	     * before we ever get an EOF.  This allows us to
	     * detect abnormal termination of any process we
	     * are connected to.
	     */
	    rx_fatal("read_incoming(): got EOF for fd %d\n",
		      incoming->fd);
	}
	else /* n_read < 0 */
	{
/*	  NexusAssert2((errno == read_incoming_recv_error),("read_incoming(): n_read = %d, errno = %d, read_incoming_recv_error = %d\n", n_read, errno, read_incoming_recv_error));*/
	    /* Got an error on the read */
#ifdef BUILD_DEBUG
	    if(NexusDebug(5))
	    {
		nexus_printf("read_incoming(): got -1 on rx_recv. errno:%d\n",read_incoming_recv_error);
	    }
#endif
	    if (read_incoming_recv_error == EINTR)
	    {
		/*
		 * The read() got interrupted (for example, by a signal),
		 * so retry the read.
		 */
#ifdef BUILD_DEBUG
		if(NexusDebug(5))
		{
		    nexus_printf("read_incoming(): error:EINTR retrying\n");
		}
#endif
		continue;
	    }
	    else if (read_incoming_recv_error == EAGAIN)
	    {
		/*
		 *  Resource temporarily unavailable
		 */
#ifdef BUILD_DEBUG
		if(NexusDebug(5))
		{
		    nexus_printf("read_incoming(): error:EAGAIN retrying\n");
		}
#endif
		continue;
	    }
	    else if (read_incoming_recv_error == EWOULDBLOCK)
	    {
		/*
		 * The read() would have blocked without reading anything.
		 * So quit trying and return.
		 */
#ifdef BUILD_DEBUG
		if(NexusDebug(5))
		{
		    nexus_printf("read_incoming(): error:EWOULDBLOCK quitting\n");
		}
#endif
		break;
	    }
#ifdef	TARGET_ARCH_SOLARIS
	    else if (((read_incoming_recv_error == ECONNRESET) 
		      || (read_incoming_recv_error == EPROTO) 
		      || (read_incoming_recv_error == 0)
		      /* This is really gross! 
		       *  Perhaps should look at atm_errno*/
		     ) 
		     && incoming->being_closed)
#else	/* TARGET_ARCH_SOLARIS */
	    else if (((read_incoming_recv_error == ECONNRESET) 
		      || (read_incoming_recv_error == EPROTO)) 
		     && incoming->being_closed)
#endif	/* TARGET_ARCH_SOLARIS */
	    {
	        delete_incoming_from_list( incoming );
	        close_incoming( incoming );
	    }
#ifdef	NEXUS_RX_PROTO_UDP
	    else if ( read_incoming_recv_error == ECONNREFUSED 
		     && incoming->being_closed )
	    {
	        delete_incoming_from_list( incoming );
		close_incoming( incoming );
	    }
#endif  /* NEXUS_RX_PROTO_UDP */
	    else
	    {
		rx_fatal("read_incoming(%d): Read failed (errno=%d): %s\n",
			  incoming->fd, read_incoming_recv_error, 
			  _nx_md_system_error_string(read_incoming_recv_error));
	    }
	}

    } while (n_read > 0);
} /* read_incoming() */


/*
 * handle_enqueued_messages()
 *
 * Handle all messages that are enqueued.
 */
static void handle_enqueued_messages(void)
{
    rx_buffer_t *buf;
#ifdef RX_PROTO_SINGLE_THREADED
    handle_in_progress = NEXUS_TRUE;
#endif
    while (MessagesEnqueued())
    {
	DequeueMessage(buf);
	rx_exit();
	_nx_handle_message(buf->u.recv.handler_name,
			   buf->u.recv.handler_id,
			   buf->u.recv.handler_type,
			   buf->u.recv.handler_func,
			   buf->u.recv.context,
			   buf->u.recv.address,
#ifdef BUILD_PROFILE
			   buf->u.recv.node_id,
			   buf->u.recv.context_id,
			   buf->u.recv.size,
#endif
			   (void *) buf);
	rx_enter();
    }
#ifdef RX_PROTO_SINGLE_THREADED
    handle_in_progress = NEXUS_FALSE;
#endif

} /* handle_enqueued_messages() */


static void md_unix_get_host_by_address(struct sockaddr_in from, 
					struct hostent *hp)
{
#if defined(TARGET_ARCH_SOLARIS)
    struct hostent hp2;
    char hp_tsdbuffer[500];
    int hp_errnop;
#endif
#if defined(TARGET_ARCH_AIX)
    struct hostent hp2;
    struct hostent_data hp_data;
#endif
  
#if defined(TARGET_ARCH_AIX) || defined(TARGET_ARCH_SOLARIS)
#if defined(TARGET_ARCH_SOLARIS)
    hp = gethostbyaddr_r( (char *) &from.sin_addr, sizeof(from.sin_addr), from.sin_family,
			  &hp2, hp_tsdbuffer, 500, &hp_errnop );
#endif
#if defined(TARGET_ARCH_AIX)
    gethostbyaddr_r( ( char *) &from.sin_addr, sizeof(from.sin_addr),
		     from.sin_family, &hp2, &hp_data );
    hp = &hp2;
#endif
#else    
    hp = gethostbyaddr((char *) &from.sin_addr, sizeof(from.sin_addr),
		       from.sin_family);
#endif    

} /* md_unix_get_host_by_address() */


/*
 * accept_new_connection()
 *
 * Do an accept() on the passed 'fd', and then setup
 * a new rx_incoming_t for this connection.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 */
static void accept_new_connection(int fd)
{
    int new_fd;
    int rx_fd;
    struct sockaddr_in from;
    int len;
    rx_incoming_t *incoming;
    int pid, type, rc;
    rx_adaptor_funcs_t *adaptor_funcs;
    int save_error;
#ifdef BUILD_DEBUG
    struct hostent *hp;
#endif

    make_room_for_new_fd(); /* XXX need room for two fds? */
    
    len = sizeof( from );
    new_fd = accept(fd, (struct sockaddr *) &from, &len);
    save_error = errno;
    if ( new_fd < 0 )
    {
	rx_fatal("accept_new_connection(): accept failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    /* Read the pid of the connecting process */
    if ( (rc = read(new_fd, &pid, sizeof pid)) != sizeof pid  )
    {
	save_error = errno;
        rx_fatal("accept_new_connection(): read of pid returns %d, errno = %d\n",
		  rc, save_error);
    }
    pid = NX_NTOHL(pid);

    if ( (rc = read(new_fd, &type, sizeof type)) != sizeof type )
    {
	save_error = errno;
        rx_fatal("accept_new_connection(): read of type returns %d, errno = %d\n",
		  rc, save_error);
    }
    type = NX_NTOHL(type);

    adaptor_funcs = rx_type_to_funcs( type );

#ifdef BUILD_DEBUG
    md_unix_get_host_by_address( from, hp );
    if (NexusDebug(3))
    {
        nexus_notice("Got incoming from %s/%d on %d pid=%d\n",
		     hp->h_name, ntohs(from.sin_port), new_fd, pid);
	nexus_notice("Got incoming from %s/%d on %d pid=%d\n",
	       hp->h_name, ntohs(from.sin_port), new_fd, pid);
        nexus_notice("Bringing up RX connection...\n");
    }
#endif /* BUILD_DEBUG */

    /* This operation blocks for now.  XXX it needs a timeout */
    rx_fd = adaptor_funcs->server( new_fd );

    /* Get rid of the old tcp connection, it was just for bootstrapping */
    close( new_fd ); 

    set_nonblocking( rx_fd ); /* XXX can rx fd's be set non-blocking? */

    incoming = construct_incoming( rx_fd, adaptor_funcs );

#ifdef BUILD_DEBUG
    incoming->connected_pid = pid;
#endif
    
    add_proto_with_fd(incoming, rx_fd);
    nexus_debug_printf(RX_CLOSE_DEBUG_LEVEL,("accept_new_connection: incoming(%x) constructed\n",incoming));

} /* accept_new_connection() */


#ifdef DONT_INCLUDE
/*
 * config_socket()
 *
 * Configure the passed socket (a file descriptor) with:
 *	- no delay: Keep the TCP driver from delaying
 *			outgoing packets to coalescing
 *	- keepalive: Periodically send messages down the socket
 *			so as to be able to detect if the process
 *			on the other end of the socket has died
 *	- non-blocking: read() and write() operations will not block
 */
static void config_socket(int s)
{
    int one = 1;
    int save_error;

#ifndef FM_NO_TCP_NODELAY
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &one,
		   sizeof(one)) < 0)
    {
	save_error = errno;
	nexus_warning("config_socket(): setsockopt TCP_NODELAY failed: %s\n",
		     _nx_md_system_error_string(save_error));
    }
#endif
    
#ifdef SO_KEEPALIVE
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *) &one,
		   sizeof(one)) < 0)
    {
        save_error = errno;
	nexus_warning("config_socket(): setsockopt SO_KEEPALIVE failed: %s\n",
		      _nx_md_system_error_string(save_error));
    }
#endif

    set_nonblocking(s);
} /* config_socket() */
#endif /* DONT_INCLUDE */

#ifdef	NEXUS_RX_PROTO_MN_UDP
#define fcntl(a, b, c) mn_fcntl(a, b, c)
#endif /* NEXUS_RX_PROTO_MN_UDP */
/*
 * set_nonblocking()
 *
 * Set the passed file descriptor, fd, to non-blocking.  (Calls
 * to read() and write() on that fd will not block.)
 */
static void set_nonblocking(int fd)
{
    int flags;
    int save_error;
    
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
	save_error = errno;
	rx_fatal("set_nonblocking(): fcntl F_GETFL 1: %s\n",
		  _nx_md_system_error_string(save_error));
    }

#if defined(TARGET_ARCH_AIX) || defined(TARGET_ARCH_HPUX)
    flags |= O_NONBLOCK;
#else
    flags |= FNDELAY;
#endif

    if (fcntl(fd, F_SETFL, flags) < 0)
    {
	save_error = errno;
	rx_fatal("set_nonblocking(): fcntl F_SETFL: %s\n",
		  _nx_md_system_error_string(save_error));
    }

#ifdef F_SETFD
#ifndef NEXUS_RX_PROTO_MN_UDP
    if (fcntl(fd, F_SETFD, 1) < 0)
    {
	save_error = errno;
	rx_fatal("set_nonblocking(): fcntl F_SETFD: %s\n",
		  _nx_md_system_error_string(save_error));
    }
#endif
#endif
} /* set_nonblocking() */


/*
 * do_connect()
 */
static int do_connect(char *host, int port, rx_adaptor_funcs_t *adaptor_funcs)
{
    struct sockaddr_in his_addr, use_his_addr;
    int s;
    int rx_fd;
    nexus_bool_t connect_succeeded = NEXUS_FALSE;
    int connect_failures = 0;
    struct hostent *hp;
    int save_error;
#if defined(TARGET_ARCH_SOLARIS)
    struct hostent hp2;
    char hp_tsdbuffer[500];
    int  hp_errnop;
#endif    
#if defined(TARGET_ARCH_AIX)
    int rc;
    struct hostent hp2;
    struct hostent_data hp_data;
#endif

#ifdef BUILD_DEBUG
    if (NexusDebug(2))
	nexus_printf("Connecting to %s/%d\n", host, port);
#endif

#if defined(TARGET_ARCH_AIX) || defined(TARGET_ARCH_SOLARIS)
#if defined(TARGET_ARCH_SOLARIS)
    hp = gethostbyname_r( host, &hp2, hp_tsdbuffer, 500, &hp_errnop );
#endif
#if defined(TARGET_ARCH_AIX)
    rc = gethostbyname_r( host, &hp2, &hp_data );
    hp = &hp2;
#endif
#else    
    hp = gethostbyname(host);
#endif    
    
    if (hp == (struct hostent *) NULL)
    {
	rx_fatal("do_connect(): gethostbyname %s failed\n", host);
    }
    
    ZeroOutMemory((char *)&his_addr, sizeof(his_addr));

    his_addr.sin_port = htons(port);
    memcpy(&his_addr.sin_addr, hp->h_addr, hp->h_length);
    his_addr.sin_family = hp->h_addrtype;

    while (!connect_succeeded)
    {
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    save_error = errno;
	    rx_fatal("do_connect(): Failed to create socket: %s\n",
		      host, port, _nx_md_system_error_string(save_error));
	}
	use_his_addr = his_addr;

	if (connect(s, (struct sockaddr *) &use_his_addr,
		    sizeof(use_his_addr)) == 0)
	{
	    connect_succeeded = NEXUS_TRUE;
	}
	else
	{
	    save_error = errno;
	    if (save_error == EINPROGRESS)
	    {
		/*
		 * man connect: EINPROGRESS:
		 *   The socket is non-blocking and the connection
		 *   cannot be completed immediately. It is possible to
		 *   select(2) for completion by selecting the socket
		 *   for writing.
		 * So this connect has, for all practical purposes, succeeded.
		 */
                connect_succeeded = NEXUS_TRUE;
#ifdef BUILD_DEBUG	    
		if (NexusDebug(2))
		{
		    nexus_printf("do_connect(): Connect to %s/%d got EINPROGRESS: %s\n",
				 host, port, _nx_md_system_error_string(save_error));
		}
#endif
            }
	    else if (save_error == EINTR)
	    {
#ifdef TARGET_ARCH_SUNOS41
		/*
		 * XXX What about SUNOS4.1.3 and Quickthreads?
		 *
		 * With SunOS 4.1.3 (at least when used
		 * with FSU pthreads), connect() has
		 * an annoying little bug.  It may return EINTR, but
		 * still succeed.  When using round robin scheduling,
		 * where SIGALRM is used to timeslice between threads,
		 * this bug actually occurs with surprising frequency.
		 *
		 * My first attempted solution was to call getpeername(),
		 * with the idea that if it succeeds then the connect()
		 * succeeded, and if it failed then the connect() failed.
		 * This didn't work; getpeername() would sometime fail
		 * yet the connect() would still succeed.
		 *
		 * But it appears that selecting the file descriptor
		 * for writing causes it to wait until things are sane.
		 * (This idea comes from EINPROGRESS.)  And it also
		 * appears that when connect() returns EINTR it
		 * nonetheless always succeeds.
		 *
		 * Note that sometimes the select() returns EINTR,
		 * so we need to retry the select() in that case.
		 * And the EBADF check is just for good measure, as
		 * this would seem to catch the (as yet unobserved)
		 * case where the interrupted connect() actually
		 * failed.
		 */
		fd_set my_fd_set;
		int select_rc;
		int select_loop_done = NEXUS_FALSE;
		int select_error;

		while (!select_loop_done)
		{
		    FD_ZERO(&my_fd_set);
		    FD_SET(s, &my_fd_set);
		    select_rc = select(s+1,
				       NEXUS_FD_SET_CAST NULL,
				       NEXUS_FD_SET_CAST &my_fd_set,
				       NEXUS_FD_SET_CAST NULL,
				       NULL);
		    select_error = errno;
		    if (select_rc == 1)
		    {
			select_loop_done = NEXUS_TRUE;
			connect_succeeded = NEXUS_TRUE;
		    }
		    else
		    {
			if (select_error == EBADF)
			{
			    close(s);
			    select_loop_done = NEXUS_TRUE;
			}
			else if (select_error != EINTR)
			{
			    nexus_fatal("do_connect(): Ack! connect() returned EINTR, but selecting on the socket failed (select returned %d, errno=%d).  Guess its time to try another hack around this connect() bug.\n", select_rc, select_error);
			}
		    }
		}
		
#else  /* TARGET_ARCH_SUNOS41 */

		/*
		 * Do nothing.  Just try again.
		 */
		nexus_debug_printf(2, ("do_connect(): Connect to %s/%d, got EINTR.  Retrying...\n", host, port));
		close(s);
		
#endif /* TARGET_ARCH_SUNOS41 */
	    }
	    else if (save_error == ECONNREFUSED)
	    {
#ifdef BUILD_DEBUG	    
		if (NexusDebug(2))
		{
		    nexus_notice("do_connect(): Connect to %s/%d refused.  Retrying...\n",
				 host, port);
		}
#endif
		close(s);

		/*
		 * If the first few attempts fail, then backoff for a
		 * bit before trying again
		 */
		connect_failures++;
		if (connect_failures > CONNECT_BACKOFF_FAST_RETRY)
		{
#ifdef RX_PROTO_SINGLE_THREADED		
		    nexus_usleep(CONNECT_BACKOFF_USEC);
#else		
		    nexus_usleep(CONNECT_BACKOFF_USEC); /* XXX take this out? */
		    nexus_thread_yield();
#endif		
		}
	    }
	    else
	    {
		rx_fatal("do_connect(): Connect to %s/%d failed: %s\n",
			  host, port, _nx_md_system_error_string(save_error));
	    }

	}
    }
    
#ifdef BUILD_DEBUG

    if (NexusDebug(2))
    {
	struct sockaddr_in my_addr;
	int len;

	len = sizeof(my_addr);
	if (getsockname(s, (struct sockaddr *) &my_addr, &len) == 0)
	    nexus_printf("New conn: remote=%s/%d local=%d\n",
			hp->h_name, port, htons(my_addr.sin_port));
	else
	{
	    save_error = errno;
	    nexus_printf("New conn: remote=%s/%d local=?? <%s>\n",
			hp->h_name, port, _nx_md_system_error_string(save_error));
	}
    }
#endif

#if 0
    config_socket(s);
#endif 

    /*
     * Send our pid down the pipe so that we can identify the 
     * incoming connection on the other end.  Also, send the
     * rx_adaptor_type to the peer, so it can be set in the incoming
     */
    {
	long pid, type, rc;

	pid = NX_HTONL((long) _nx_md_getpid());
	if ( (rc = write(s, &pid, sizeof pid)) != sizeof pid ) 
	{
	    nexus_fatal("do_connect(): write of pid failed, error=%d\n",rc);
	}

	type = NX_HTONL((long) (adaptor_funcs->type));
	if ( (rc = write(s, &type,sizeof type)) != sizeof type ) 
	{
	    nexus_fatal("do_connect(): write of type failed, error=%d\n",rc);
	}
	
	rx_fd = adaptor_funcs->client(host, s);
	close(s);
	set_nonblocking(rx_fd); /* XXX can rx fd's be set non-blocking? */
    }
    
    return (rx_fd);
} /* do_connect() */


/*
 * setup_listener()
 *
 * Return the file descriptor of this listener.  Also, return the
 * listener port in 'port'.
 */
static int setup_listener(int *port)
{
    struct sockaddr_in my_addr;
    int len;
    int listen_fd;
    int save_error;
    
    ZeroOutMemory((char *)&my_addr, sizeof(my_addr));

    my_addr.sin_addr.s_addr = INADDR_ANY; /* XXX restrict to listening on the
					     RX interface */ 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = 0;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
	save_error = errno;
	rx_fatal("setup_listener(): socket() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    if (bind(listen_fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
    {
	save_error = errno;
	rx_fatal("setup_listener(): bind() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    if (listen(listen_fd, SOMAXCONN) < 0)
    {
	save_error = errno;
	rx_fatal("setup_listener(): listen() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    len = sizeof(my_addr);
    if (getsockname(listen_fd, (struct sockaddr *) &my_addr, &len) < 0)
    {
	save_error = errno;
	rx_fatal("setup_listener(): getsockname() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    *port = ntohs(my_addr.sin_port);

    return (listen_fd);
} /* setup_listener() */
    

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
	proto_table[i].proto = (rx_proto_t *) NULL;
	proto_table[i].next = (proto_table_entry_t *) NULL;
    }
} /* proto_table_init() */


/*
 * proto_table_hash()
 *
 * Hash the hostname and port for the proto table.
 */
static int proto_table_hash(char *host, int port)
{
    unsigned int hval = 0;
    char *s;

    for (s = host; *s != '\0'; s++)
    {
	hval <<= 1;
	hval += *s;
    }
    hval <<= 1;
    hval += port;
    return (hval % PROTO_TABLE_SIZE);
} /* proto_table_hash() */


/*
 * proto_table_insert()
 *
 * Insert the given proto into the table, hashing on its internal
 * hostname and port number.
 *
 * It is o.k. for the entry to be present in the table more than
 * once, if the other proto is being closed.
 */
static void proto_table_insert(rx_proto_t *proto)
{
    int bucket;
    proto_table_entry_t *new_ent;

    bucket = proto_table_hash(proto->host, proto->port);
    if (proto_table[bucket].proto == (rx_proto_t *) NULL)
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
	NexusMalloc(proto_table_insert(), new_ent, proto_table_entry_t *,
		    sizeof(struct _proto_table_entry_t));

	new_ent->proto = proto;
	new_ent->next = proto_table[bucket].next;

	proto_table[bucket].next = new_ent;
    }
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
	nexus_printf("proto_table_inserted(): Inserted proto=%x for %s/%d bucket=%d\n",
		     proto, proto->host, proto->port, bucket);
#endif
} /* proto_table_insert() */

/*
 * proto_table_delete()
 *
 * Remove a proto from the table.
 */
static void proto_table_delete(rx_proto_t *proto)
{
  int bucket;
  proto_table_entry_t *del_ent;
  proto_table_entry_t *tmp;

  bucket = proto_table_hash(proto->host, proto->port);
  NexusAssert2((proto_table[bucket].proto != (rx_proto_t *) NULL),
	       ("proto_table_delete(): proto(%d) not in table\n",proto));

  del_ent = proto_table[bucket].next;
  if (proto_table[bucket].proto == proto) 
  {
    /* Delete the proto in the preallocated bucket */
    if (del_ent == NULL)
    {
      proto_table[bucket].proto = (rx_proto_t *) NULL;
    }
    else 
    {
      proto_table[bucket].proto = del_ent->proto;
      proto_table[bucket].next = del_ent->next;
      NexusFree(del_ent);
    }
    return;
  }
  tmp = &proto_table[bucket]; /* Step back */
  while (tmp->next != NULL)
  {
    if (tmp->next->proto == proto)
    {
      del_ent = tmp->next;
      tmp->next = tmp->next->next;
      NexusFree(del_ent);
      return;
    }
    tmp = tmp->next;
  }
  NexusAssert2((NEXUS_TRUE == NEXUS_FALSE),
	       ("proto table_delete(): proto(%d) not in table\n",proto));
} /* proto_table_delete() */
  
/*
 * proto_table_lookup()
 *
 * Look up and return the rx_proto_t for the given hostname
 * and port. Return NULL if none exists.
 */
static rx_proto_t *proto_table_lookup(char *host, int port)
{
    proto_table_entry_t *ent;
    int bucket;

    bucket = proto_table_hash(host, port);

    for (ent = &(proto_table[bucket]);
	 ent != (proto_table_entry_t *) NULL;
	 ent = ent->next)
    {
	if (   (ent->proto != (rx_proto_t *) NULL)
	    && (ent->proto->being_closed == NEXUS_FALSE)
	    && (ent->proto->port == port)
	    && (strcmp(ent->proto->host, host) == 0) )
	{
#ifdef BUILD_DEBUG
	    if (NexusDebug(2))
		nexus_printf("proto_table_lookup(): Found entry %x proto=%x for %s/%d bucket=%d\n",
			    ent, ent->proto, host, port, bucket);
#endif
	    return (ent->proto);
	}
    }
    
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
	nexus_printf("proto_table_lookup(): Didn't find entry for %s/%d bucket=%d\n",
		    host, port, bucket);
#endif
    return ((rx_proto_t *) NULL);
} /* proto_table_lookup() */


#ifdef BUILD_DEBUG
static void print_fd_set(char *fmt_string, fd_set *set)
{
    char fd_buf[1024], fd_tmp_buf[10];
    int fd_i;

    fd_buf[0] = '\0';
    for (fd_i = 0; fd_i < fd_tablesize; fd_i++)
    {
	if (FD_ISSET(fd_i, set))
	{
	    sprintf(fd_tmp_buf, "%d ", fd_i);
	    strcat(fd_buf, fd_tmp_buf);
	}
    }
    nexus_printf(fmt_string, fd_buf);
} /* print_fd_set() */
#endif /* BUILD_DEBUG */


/* PRINTING_OFF */
/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

/*
 * rx_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void rx_set_buffer_size(nexus_buffer_t *buffer,
				int size, int n_elements)
{
    rx_buffer_t *rx_buffer;
    char *storage;

    NexusBufferMagicCheck(rx_check_buffer_size, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == SEND_BUFFER),
		 ("rx_set_buffer_size(): Internal error: Expected a send buffer\n"));

    if (size > 0 )
    {
	NexusMalloc(rx_set_buffer_size(), storage, char *,
		    (size + rx_buffer->u.send.header_size));

	rx_buffer->u.send.size = size;
	rx_buffer->u.send.n_elements = n_elements;
	rx_buffer->storage = storage;
	SetBufferPointers(rx_buffer);
	rx_buffer->u.send.current_pointer = rx_buffer->u.send.base_pointer;
    }

} /* rx_set_buffer_size() */


/*
 * rx_check_buffer_size()
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
static int rx_check_buffer_size(nexus_buffer_t *buffer,
				 int slack, int increment)
{
    rx_buffer_t *rx_buffer;
    int used;
    int needed;

    NexusBufferMagicCheck(rx_check_buffer_size, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == SEND_BUFFER),
		 ("rx_check_buffer_size(): Internal error: Expected a send buffer\n"));
    
    used = (rx_buffer->u.send.current_pointer
	    - rx_buffer->u.send.base_pointer);
    needed = used + slack;

    if (rx_buffer->u.send.size == 0)
    {
	rx_set_buffer_size(buffer, slack, -1);
    }
    else if (needed > rx_buffer->u.send.size)
    {
	char *new_storage;
	int new_size;

	if (increment <= 0)
	    return(NEXUS_FALSE);

	new_size = rx_buffer->u.send.size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

	NexusMalloc(rx_check_buffer_size(), new_storage, char *,
		    (new_size + rx_buffer->u.send.header_size));
	
	memcpy(new_storage + rx_buffer->u.send.header_size,
	       rx_buffer->u.send.base_pointer,
	       used);
	    
	NexusFree(rx_buffer->storage);

	rx_buffer->u.send.size = new_size;
	rx_buffer->storage = new_storage;
	SetBufferPointers(rx_buffer);
	rx_buffer->u.send.current_pointer = (rx_buffer->u.send.base_pointer
					      + used);
    }
    
    return(NEXUS_TRUE);
} /* rx_check_buffer_size() */


/*
 * rx_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since rx_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void rx_free_buffer(nexus_buffer_t *buffer)
{
    rx_buffer_t *rx_buffer;
    
    NexusAssert2((buffer),
		 ("rx_free_buffer(): Passed a NULL nexus_buffer_t *\n") );
    
    /* If the buffer was stashed, *buffer will have been set to NULL */
    if (!(*buffer))
	return;

    NexusBufferMagicCheck(rx_free_buffer, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_free_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!rx_buffer->u.recv.stashed),
		 ("rx_free_buffer(): Expected a non-stashed buffer\n"));

    if (rx_buffer->u.recv.handler_name)
    {
	NexusFree(rx_buffer->u.recv.handler_name);
    }
    FreeRxBuffer(rx_buffer);

    *buffer = (nexus_buffer_t) NULL;
    
} /* rx_free_buffer() */


/*
 * rx_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void rx_stash_buffer(nexus_buffer_t *buffer,
			     nexus_stashed_buffer_t *stashed_buffer)
{
    rx_buffer_t *rx_buffer;
    
    NexusBufferMagicCheck(rx_stash_buffer, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_stash_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!rx_buffer->u.recv.stashed),
		 ("rx_stash_buffer(): Expected an un-stashed buffer\n"));
    
    rx_buffer->u.recv.stashed = NEXUS_TRUE;
		  
    *stashed_buffer = (nexus_stashed_buffer_t) *buffer;
    
    *buffer = (nexus_buffer_t) NULL;
} /* rx_stash_buffer() */


/*
 * rx_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by rx_stash_buffer().
 */
static void rx_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
    rx_buffer_t *rx_buffer;

    NexusAssert2((stashed_buffer),
		 ("rx_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n") );
    NexusBufferMagicCheck(rx_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
    
    rx_buffer = (rx_buffer_t *) *stashed_buffer;
    
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_free_stashed_buffer(): Expected a receive buffer\n"));
    NexusAssert2((rx_buffer->u.recv.stashed),
		 ("rx_free_stashed_buffer(): Expected a stashed buffer\n"));

    FreeRxBuffer(rx_buffer);

    *stashed_buffer = (nexus_stashed_buffer_t) NULL;
    
} /* rx_free_stashed_buffer() */



/*
 * rx_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int rx_sizeof_float(nexus_buffer_t *buffer, int count)
{
    return(sizeof(float) * count);
}

static int rx_sizeof_double(nexus_buffer_t *buffer, int count)
{
    return(sizeof(double) * count);
}

static int rx_sizeof_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(short) * count);
}

static int rx_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned short) * count);
}

static int rx_sizeof_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(int) * count);
}

static int rx_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned int) * count);
}

static int rx_sizeof_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(long) * count);
}

static int rx_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned long) * count);
}

static int rx_sizeof_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(char) * count);
}

static int rx_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned char) * count);
}




/*
 * rx_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */

#ifdef BUILD_DEBUG
#define DO_PUT_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(rx_put_ ## TYPE, buf); \
    if (((rx_buf->u.send.current_pointer - rx_buf->u.send.base_pointer) + count * sizeof(TYPE)) > rx_buf->u.send.size) \
    { \
	rx_fatal("nexus_put_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("rx_put_" #TYPE "(): base:%x size:%lu current:%x limit:%x\n", \
		     rx_buf->u.send.base_pointer, \
		     rx_buf->u.send.size, \
		     rx_buf->u.send.current_pointer, \
		     rx_buf->u.send.base_pointer + rx_buf->u.send.size); \
    }

#else /* BUILD_DEBUG */
#define DO_PUT_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SPOOF_COPY
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    if (!_nx_spoof_copy) \
        memcpy(rx_buf->u.send.current_pointer, data, count * sizeof(TYPE)); \
    rx_buf->u.send.current_pointer = \
        (char *) rx_buf->u.send.current_pointer + (count * sizeof(TYPE)); \
}
#else
#define PUT(TYPE) \
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    memcpy(rx_buf->u.send.current_pointer, data, count * sizeof(TYPE)); \
    rx_buf->u.send.current_pointer = \
        (char *) rx_buf->u.send.current_pointer + (count * sizeof(TYPE)); \
}
#endif

static void rx_put_float(nexus_buffer_t *buf, float *data, int count)
{
    PUT(float);
}

static void rx_put_double(nexus_buffer_t *buf, double *data, int count)
{
    PUT(double);
}

static void rx_put_short(nexus_buffer_t *buf, short *data, int count)
{
    PUT(short);
}

static void rx_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    PUT(u_short);
}

static void rx_put_int(nexus_buffer_t *buf, int *data, int count)
{
    PUT(int);
}

static void rx_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    PUT(u_int);
}

static void rx_put_long(nexus_buffer_t *buf, long *data, int count)
{
    PUT(long);
}

static void rx_put_u_long(nexus_buffer_t *buf, unsigned long *data, int count)
{
    PUT(u_long);
}

static void rx_put_char(nexus_buffer_t *buf, char *data, int count)
{
    PUT(char);
}

static void rx_put_u_char(nexus_buffer_t *buf, unsigned char *data, int count)
{
    PUT(u_char);
}



/*
 * rx_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(rx_get_ ## TYPE, buf); \
    NexusAssert2(!rx_buf->u.recv.stashed, \
		 ("rx_get_*(): Expected an un-stashed buffer\n")); \
    if (rx_buf->u.recv.current_pointer > (rx_buf->storage + rx_buf->u.recv.size)) \
    { \
	rx_fatal("nexus_get_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("rx_get_" #TYPE "(): base:%x size:%lu current:%x limit:%x\n", \
		     rx_buf->storage, \
		     rx_buf->u.recv.size, \
		     rx_buf->u.recv.current_pointer, \
		     rx_buf->storage + rx_buf->u.recv.size); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SPOOF_COPY
#define GET(TYPE) \
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_GET_ASSERTIONS(TYPE); \
    if (!_nx_spoof_copy) \
        memcpy(dest, rx_buf->u.recv.current_pointer, sizeof(TYPE) * count); \
    rx_buf->u.recv.current_pointer += count * sizeof(TYPE); \
}
#else
#define GET(TYPE) \
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_GET_ASSERTIONS(TYPE); \
    memcpy(dest, rx_buf->u.recv.current_pointer, sizeof(TYPE) * count); \
    rx_buf->u.recv.current_pointer += count * sizeof(TYPE); \
}
#endif


static void rx_get_float(nexus_buffer_t *buf, float *dest, int count)
{
    GET(float);
}

static void rx_get_double(nexus_buffer_t *buf, double *dest, int count)
{
    GET(double);
}

static void rx_get_short(nexus_buffer_t *buf, short *dest, int count)
{
    GET(short);
}

static void rx_get_u_short(nexus_buffer_t *buf, unsigned short *dest,
			    int count)
{
    GET(u_short);
}

static void rx_get_int(nexus_buffer_t *buf, int *dest, int count)
{
    GET(int);
}

static void rx_get_u_int(nexus_buffer_t *buf, unsigned int *dest, int count)
{
    GET(u_int);
}

static void rx_get_long(nexus_buffer_t *buf, long *dest, int count)
{
    GET(long);
}

static void rx_get_u_long(nexus_buffer_t *buf, unsigned long *dest, int count)
{
    GET(u_long);
}

static void rx_get_char(nexus_buffer_t *buf, char *dest, int count)
{
    GET(char);
}

static void rx_get_u_char(nexus_buffer_t *buf, unsigned char *dest, int count)
{
    GET(u_char);
}



/*
 * rx_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */

#ifdef BUILD_DEBUG
#define DO_GET_STASHED_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(rx_get_stashed_ ## TYPE, (nexus_buffer_t *) buf); \
    NexusAssert2(rx_buf->u.recv.stashed, \
		 ("rx_get_stashed_*(): Expected a stashed buffer\n"));   \
    if (rx_buf->u.recv.current_pointer > (rx_buf->storage + rx_buf->u.recv.size)) \
    { \
	rx_fatal("nexus_get_stashed_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("rx_get_stashed_" #TYPE "(): base:%x size:%lu current:%x limit:%x\n", \
		     rx_buf->storage, \
		     rx_buf->u.recv.size, \
		     rx_buf->u.recv.current_pointer, \
		     rx_buf->storage + rx_buf->u.recv.size); \
    }

#else /* BUILD_DEBUG */
#define DO_GET_STASHED_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SPOOF_COPY
#define GET_STASHED(TYPE) \
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_GET_STASHED_ASSERTIONS(TYPE); \
    if (!_nx_spoof_copy) \
        memcpy(dest, rx_buf->u.recv.current_pointer, sizeof(TYPE) * count); \
    rx_buf->u.recv.current_pointer += count * sizeof(TYPE); \
}
#else
#define GET_STASHED(TYPE) \
{ \
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf; \
    DO_GET_STASHED_ASSERTIONS(TYPE); \
    memcpy(dest, rx_buf->u.recv.current_pointer, sizeof(TYPE) * count); \
    rx_buf->u.recv.current_pointer += count * sizeof(TYPE); \
}
#endif


static void rx_get_stashed_float(nexus_stashed_buffer_t *buf,
				  float *dest, int count)
{
    GET_STASHED(float);
}

static void rx_get_stashed_double(nexus_stashed_buffer_t *buf,
				   double *dest, int count)
{
    GET_STASHED(double);
}

static void rx_get_stashed_short(nexus_stashed_buffer_t *buf,
				  short *dest, int count)
{
    GET_STASHED(short);
}

static void rx_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				    unsigned short *dest, int count)
{
    GET_STASHED(u_short);
}

static void rx_get_stashed_int(nexus_stashed_buffer_t *buf,
				int *dest, int count)
{
    GET_STASHED(int);
}

static void rx_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				  unsigned int *dest, int count)
{
    GET_STASHED(u_int);
}

static void rx_get_stashed_long(nexus_stashed_buffer_t *buf,
				 long *dest, int count)
{
    GET_STASHED(long);
}

static void rx_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				   unsigned long *dest, int count)
{
    GET_STASHED(u_long);
}

static void rx_get_stashed_char(nexus_stashed_buffer_t *buf,
				 char *dest, int count)
{
    GET_STASHED(char);
}

static void rx_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				   unsigned char *dest, int count)
{
    GET_STASHED(u_char);
}

/*********************************************************************
 * 		Buffer management code XDR
 *********************************************************************/

/*
 * rx_xdr_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void rx_xdr_set_buffer_size(nexus_buffer_t *buffer,
				int size, int n_elements)
{
    rx_buffer_t *rx_buffer;
    char *storage;

    NexusBufferMagicCheck(rx_check_buffer_size, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == SEND_BUFFER),
		 ("rx_xdr_set_buffer_size(): Internal error: Expected a send buffer\n"));

    if (size > 0 )
    {
	NexusMalloc(rx_set_buffer_size(), storage, char *,
		    (size + rx_buffer->u.send.header_size));

	rx_buffer->u.send.size = size;
	rx_buffer->u.send.n_elements = n_elements;
	rx_buffer->storage = storage;
	SetBufferPointers(rx_buffer);
	rx_buffer->u.send.current_pointer = rx_buffer->u.send.base_pointer;

	/* do XDR stuff here */
	/* xdrmem_create */
	xdrmem_create(&(rx_buffer->xdrs), rx_buffer->u.send.base_pointer,
		      rx_buffer->u.send.size, XDR_ENCODE );
    }

} /* rx_xdr_set_buffer_size() */


/*
 * rx_xdr_check_buffer_size()
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
 * available in the buffer, then leave 'buffer' unchanged and
 * return NEXUS_FALSE.
 */
static int rx_xdr_check_buffer_size(nexus_buffer_t *buffer,
				 int slack, int increment)
{
    rx_buffer_t *rx_buffer;
    unsigned int used;
    int needed;

    NexusBufferMagicCheck(rx_xdr_check_buffer_size, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == SEND_BUFFER),
		 ("rx_xdr_check_buffer_size(): Internal error: Expected a send buffer\n"));

    /* do XDR stuff here */
    /* used should be based on xdr_getpos */

    used = xdr_getpos(&(rx_buffer->xdrs));
    
    needed = used + slack;

    if (rx_buffer->u.send.size == 0)
	{
	    rx_xdr_set_buffer_size(buffer, slack, -1);
	}
    else if (needed > rx_buffer->u.send.size)
	{
	    char *new_storage, *old_storage;
	    int new_size;

	if (increment <= 0)
	    return(NEXUS_FALSE);

	new_size = rx_buffer->u.send.size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

	NexusMalloc(rx_xdr_check_buffer_size(), new_storage, char *,
		    (new_size + rx_buffer->u.send.header_size));
	
	memcpy(new_storage + rx_buffer->u.send.header_size,
	       rx_buffer->u.send.base_pointer,
	       used);
	    
	old_storage = rx_buffer->storage;

	rx_buffer->u.send.size = new_size;
	rx_buffer->storage = new_storage;
	SetBufferPointers(rx_buffer);
	rx_buffer->u.send.current_pointer = (rx_buffer->u.send.base_pointer
					      + used);

	/* do XDR stuff here */
	/* xdr_destroy */
	xdr_destroy(&(rx_buffer->xdrs));
	NexusFree(old_storage);
	/* xdrmem_create */
	xdrmem_create(&(rx_buffer->xdrs), rx_buffer->storage,
		      new_size, XDR_ENCODE);
	/* xdr_setpos */
	xdrrc = xdr_setpos(&(rx_buffer->xdrs), used);
	NexusAssert2((xdrrc), 
	      ("rx_xdr_check_buffer_size(): xdr_setpos() returned error\n"));
    }
    
    return(NEXUS_TRUE);
} /* rx_xdr_check_buffer_size() */


/*
 * rx_xdr_free_buffer()
 *
 * Free the passed nexus_buffer_t.
 *
 * This should be called on the receiving end, after the handler
 * has completed.
 *
 * Note: The stashed flag could be set, since rx_xdr_stash_buffer()
 * just sets this flag and typecasts a buffer to a stashed buffer.
 * In this case, do not free the buffer.
 */
static void rx_xdr_free_buffer(nexus_buffer_t *buffer)
{
    rx_buffer_t *rx_buffer;
    
    NexusAssert2((buffer),
		 ("rx_xdr_free_buffer(): Passed a NULL nexus_buffer_t *\n") );
    
    /* If the buffer was stashed, *buffer will have been set to NULL */
    if (!(*buffer))
	return;

    NexusBufferMagicCheck(rx_xdr_free_buffer, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_xdr_free_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!rx_buffer->u.recv.stashed),
		 ("rx_xdr_free_buffer(): Expected a non-stashed buffer\n"));

    xdr_destroy(&(rx_buffer->xdrs));

    FreeRxBuffer(rx_buffer);

    *buffer = (nexus_buffer_t) NULL;
    
} /* rx_xdr_free_buffer() */


/*
 * rx_xdr_stash_buffer()
 *
 * Convert 'buffer' to a stashed buffer.
 */
static void rx_xdr_stash_buffer(nexus_buffer_t *buffer,
			     nexus_stashed_buffer_t *stashed_buffer)
{
    rx_buffer_t *rx_buffer;
    
    NexusBufferMagicCheck(rx_xdr_stash_buffer, buffer);

    rx_buffer = (rx_buffer_t *) *buffer;
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_xdr_stash_buffer(): Expected a receive buffer\n"));
    NexusAssert2((!rx_buffer->u.recv.stashed),
		 ("rx_xdr_stash_buffer(): Expected an un-stashed buffer\n"));
    
    rx_buffer->u.recv.stashed = NEXUS_TRUE;
		  
    *stashed_buffer = (nexus_stashed_buffer_t) *buffer;
    
    *buffer = (nexus_buffer_t) NULL;
} /* rx_xdr_stash_buffer() */


/*
 * rx_xdr_free_stashed_buffer()
 *
 * Free the passed nexus_stashed_buffer_t that was stashed
 * by rx_xdr_stash_buffer().
 */
static void rx_xdr_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer)
{
    rx_buffer_t *rx_buffer;

    NexusAssert2((stashed_buffer),
		 ("rx_xdr_free_stashed_buffer(): Passed a NULL nexus_stashed_buffer_t *\n") );
    NexusBufferMagicCheck(rx_xdr_free_stashed_buffer,
			  (nexus_buffer_t *) stashed_buffer);
    
    rx_buffer = (rx_buffer_t *) *stashed_buffer;
    
    NexusAssert2((rx_buffer->buffer_type == RECV_BUFFER),
		 ("rx_xdr_free_stashed_buffer(): Expected a receive buffer\n"));
    NexusAssert2((rx_buffer->u.recv.stashed),
		 ("rx_xdr_free_stashed_buffer(): Expected a stashed buffer\n"));
    xdr_destroy(&(rx_buffer->xdrs));

    FreeRxBuffer(rx_buffer);

    *stashed_buffer = (nexus_stashed_buffer_t) NULL;
    
} /* rx_xdr_free_stashed_buffer() */



/*
 * rx_xdr_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
#define XDR_SIZEOF(TYPE,cnt)        \
{                            \
    int i;                    \
    i=sizeof(TYPE) * cnt;    \
    while (i%4) i++;         \
    return i;                \
}

#define XDR_SIZEOF_CHARS(TYPE,cnt)  \
{                                       \
    int i;                               \
    i=sizeof(TYPE) * cnt;               \
    while (i%4) i++;                    \
    return i+4;                         \
}

static int rx_xdr_sizeof_float(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(float,count);
}

static int rx_xdr_sizeof_double(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(float,count);
}

static int rx_xdr_sizeof_short(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(short,count);
}

static int rx_xdr_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(unsigned short,count);
}

static int rx_xdr_sizeof_int(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(int,count);
}

static int rx_xdr_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(unsigned int,count);
}

static int rx_xdr_sizeof_long(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(long,count);
}

static int rx_xdr_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF(unsigned long,count);
}

static int rx_xdr_sizeof_char(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF_CHARS(char,count);
}

static int rx_xdr_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    XDR_SIZEOF_CHARS(unsigned char,count);
}




/*
 * rx_xdr_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */


static void rx_xdr_put_float(nexus_buffer_t *buf, float *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(float),
		       xdr_float);
    NexusAssert2((xdrrc), ("rx_xdr_put_float(): xdr_vector failed\n"));
}

static void rx_xdr_put_double(nexus_buffer_t *buf, double *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(double),
		       xdr_double);
    NexusAssert2((xdrrc), ("rx_xdr_put_double(): xdr_vector failed\n"));
}

static void rx_xdr_put_short(nexus_buffer_t *buf, short *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(short),
		       xdr_short);
    NexusAssert2((xdrrc), ("rx_xdr_put_short(): xdr_vector failed\n"));
}

static void rx_xdr_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(unsigned short),
		       xdr_u_short);
    NexusAssert2((xdrrc), ("rx_xdr_put_u_short(): xdr_vector failed\n"));
}

static void rx_xdr_put_int(nexus_buffer_t *buf, int *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(int),
		       xdr_int);
    NexusAssert2((xdrrc), ("rx_xdr_put_int(): xdr_vector failed\n"));
}

static void rx_xdr_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(unsigned int),
		       xdr_u_int);
    NexusAssert2((xdrrc), ("rx_xdr_put_u_int(): xdr_vector failed\n"));
}

static void rx_xdr_put_long(nexus_buffer_t *buf, long *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(long),
		       xdr_long);
    NexusAssert2((xdrrc), ("rx_xdr_put_long(): xdr_vector failed\n"));
}

static void rx_xdr_put_u_long(nexus_buffer_t *buf, unsigned long *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)data,
		       count, sizeof(unsigned long),
		       xdr_u_long);
    NexusAssert2((xdrrc), ("rx_xdr_put_u_long(): xdr_vector failed\n"));
}

static void rx_xdr_put_char(nexus_buffer_t *buf, char *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), &data, 
		      (unsigned int *)&count, (unsigned int)(count));
    NexusAssert2((xdrrc), ("rx_xdr_put_char(): xdr_bytes failed\n"));
}

static void rx_xdr_put_u_char(nexus_buffer_t *buf, unsigned char *data, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), (char **)&data,
		      (unsigned int *)&count, (unsigned int)(count));
    NexusAssert2((xdrrc), ("rx_xdr_put_u_char(): xdr_bytes failed\n"));
}



/*
 * rx_xdr_get_*()
 *
 * Get 'count' elements of the given type from 'buffer' and store
 * them into 'data'.
 */



static void rx_xdr_get_float(nexus_buffer_t *buf, float *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(float),
		       xdr_float);
    NexusAssert2((xdrrc), ("rx_xdr_get_float(): xdr_vector failed\n"));
}

static void rx_xdr_get_double(nexus_buffer_t *buf, double *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(double),
		       xdr_double);
    NexusAssert2((xdrrc), ("rx_xdr_get_double(): xdr_vector failed\n"));
}

static void rx_xdr_get_short(nexus_buffer_t *buf, short *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(short),
		       xdr_short);
    NexusAssert2((xdrrc), ("rx_xdr_get_short(): xdr_vector failed\n"));
}

static void rx_xdr_get_u_short(nexus_buffer_t *buf, unsigned short *dest,
			    int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned short),
		       xdr_u_short);
    NexusAssert2((xdrrc), ("rx_xdr_get_u_short(): xdr_vector failed\n"));
}

static void rx_xdr_get_int(nexus_buffer_t *buf, int *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(int),
		       xdr_int);
    NexusAssert2((xdrrc), ("rx_xdr_get_int(): xdr_vector failed\n"));
}

static void rx_xdr_get_u_int(nexus_buffer_t *buf, unsigned int *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned int),
		       xdr_u_int);
    NexusAssert2((xdrrc), ("rx_xdr_get_u_int(): xdr_vector failed\n"));
}

static void rx_xdr_get_long(nexus_buffer_t *buf, long *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(long),
		       xdr_u_long);
    NexusAssert2((xdrrc), ("rx_xdr_get_long(): xdr_vector failed\n"));
}

static void rx_xdr_get_u_long(nexus_buffer_t *buf, unsigned long *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned long),
		       xdr_u_long);
    NexusAssert2((xdrrc), ("rx_xdr_get_u_long(): xdr_vector failed\n"));
}

static void rx_xdr_get_char(nexus_buffer_t *buf, char *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), &dest, 
		      (unsigned int *)&count, (unsigned int)count);
    NexusAssert2((xdrrc), ("rx_xdr_get_char(): xdr_bytes failed\n"));
}

static void rx_xdr_get_u_char(nexus_buffer_t *buf, unsigned char *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), (char **)&dest,
		      (unsigned int *)&count, (unsigned int)count);
    NexusAssert2((xdrrc), ("rx_xdr_get_u_char(): xdr_bytes failed\n"));
}



/*
 * rx_xdr_get_stashed_*()
 *
 * Get 'count' elements of the given type from the stashed 'buffer' and store
 * them into 'data'.
 */



static void rx_xdr_get_stashed_float(nexus_stashed_buffer_t *buf,
				  float *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
	       count, sizeof(float),
	       xdr_float);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_float(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_double(nexus_stashed_buffer_t *buf,
				   double *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(double),
		       xdr_double);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_double(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_short(nexus_stashed_buffer_t *buf,
				  short *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(short),
		       xdr_short);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_short(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_u_short(nexus_stashed_buffer_t *buf,
				    unsigned short *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned short),
		       xdr_u_short);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_u_short(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_int(nexus_stashed_buffer_t *buf,
				int *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(int),
		       xdr_int);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_int(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_u_int(nexus_stashed_buffer_t *buf,
				  unsigned int *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned int),
		       xdr_u_int);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_u_int(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_long(nexus_stashed_buffer_t *buf,
				 long *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(long),
		       xdr_long);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_long(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_u_long(nexus_stashed_buffer_t *buf,
				   unsigned long *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_vector(&(rx_buf->xdrs), (char *)dest,
		       count, sizeof(unsigned long),
		       xdr_u_long);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_u_long(): xdr_vector failed\n"));
}

static void rx_xdr_get_stashed_char(nexus_stashed_buffer_t *buf,
				 char *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), &dest, 
		      (unsigned int *)&count, (unsigned int)count);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_char(): xdr_bytes failed\n"));
}

static void rx_xdr_get_stashed_u_char(nexus_stashed_buffer_t *buf,
				   unsigned char *dest, int count)
{
    rx_buffer_t *rx_buf = (rx_buffer_t *) *buf;
    xdrrc = xdr_bytes(&(rx_buf->xdrs), (char **)&dest,
		      (unsigned int *)&count, (unsigned int)count);
    NexusAssert2((xdrrc), ("rx_xdr_get_stashed_u_char(): xdr_bytes failed\n"));
}

/* PRINTING_ON */





/*
 * rx_get_proto_stats():
 */
static void rx_get_proto_stats(rx_stats_t *rx_stats)
{
  rx_stats->n_nacks 		= n_nacks;
  rx_stats->n_acks 		= n_acks;
  rx_stats->n_timeouts		= n_timeouts;
  rx_stats->n_retransmits 	= n_retransmits;
  rx_stats->n_rsrs		= n_rsrs;
  rx_stats->n_rsr_buffers 	= n_rsr_buffers;
  rx_stats->n_rtt		= n_rtt;
  rx_stats->ave_rtt_ticks	= ave_rtt_ticks;
  rx_stats->ave_rtt_usec	= ave_rtt_usec;

} /* rx_get_proto_stats() */
