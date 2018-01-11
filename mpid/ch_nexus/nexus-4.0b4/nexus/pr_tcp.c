/*
 * Nexus
 * Authors:     Steven Tuecke and Robert Olson
 *              Argonne National Laboratory
 *
 * pr_tcp.c		- TCP/IP protocol module
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_tcp.c,v 1.124 1997/01/22 02:43:43 tuecke Exp $";

#include "internal.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

/*
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/param.h>
#include <limits.h>
*/

/*
 * Only one thread is allowed to be in the tcp code (and thus
 * mucking with data structures) at a time.
 */

#ifdef BUILD_LITE
#define tcp_enter()
#define tcp_exit()
#else  /* BUILD_LITE */
static nexus_mutex_t	tcp_mutex;
#define tcp_enter()	nexus_mutex_lock(&tcp_mutex);
#define tcp_exit()	nexus_mutex_unlock(&tcp_mutex);
#endif /* BUILD_LITE */

static nexus_bool_t	tcp_done;
static nexus_bool_t	tcp_aborting;

#define tcp_fatal tcp_exit(); nexus_fatal

/*
 * Other useful defines
 */
#define FLAG_MASK	0xf0

#define TCP_INCOMING_DEFAULT_SIZE 4096

#define OPEN_FLAG		0x80
#define CLOSE_NORMAL_FLAG	0x40
#define CLOSE_ABNORMAL_FLAG	0x20
#define CLOSE_SHUTDOWN_FLAG	0x10

static char close_flag = CLOSE_NORMAL_FLAG;

/*
 * Some forward typedef declarations...
 */
typedef struct _tcp_outgoing_t	tcp_outgoing_t;
typedef struct _tcp_incoming_t	tcp_incoming_t;


/*
 * tcp_outgoing_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * tcp specific information to that structure.
 */
struct _tcp_outgoing_t
{
    nexus_proto_type_t		type;
    nexus_proto_funcs_t *	funcs;
    struct _tcp_outgoing_t *	next;
    char *			host;
    unsigned short		port;
    int				fd;
    int				state;
    nexus_bool_t		fault_code;
    int				reference_count;
    nexus_bool_t		write_in_progress;
    struct _nexus_buffer_t *	write_q_head;
    struct _nexus_buffer_t *	write_q_tail;
    char			read_buf[1];
};

enum
{
    OUTGOING_STATE_UNOPENED		= 0,
    OUTGOING_STATE_OPEN			= 1,
    OUTGOING_STATE_CLOSE_PENDING	= 2,
    OUTGOING_STATE_CLOSE_POSTED		= 3,
    OUTGOING_STATE_CLOSING		= 4,
    OUTGOING_STATE_CLOSED		= 5,
    OUTGOING_STATE_WRITE_ERROR		= 6,
    OUTGOING_STATE_FAULT		= 7
};


/*
 * tcp_incoming_t
 *
 * One of these is allocated for each incoming file descriptor.
 * It stores partial incoming messages until they are complete
 * and can be handled or queued.
 */
struct _tcp_incoming_t
{
    int				fd;
    int				state;
    int				format;
    unsigned long		msg_size;
    unsigned long		nbytes_parsed;
    unsigned long		nbytes_unparsed;
    unsigned long		storage_size;
    nexus_byte_t *		storage;
    nexus_byte_t *		current;
    unsigned long		save_storage_size;
    nexus_byte_t *		save_storage;
    nexus_byte_t *		save_current;
    nexus_bool_t		dispatch_in_progress;
    struct _nexus_buffer_t *	dispatch_q_head;
    struct _nexus_buffer_t *	dispatch_q_tail;
};

enum
{
    INCOMING_STATE_FORMAT,
    INCOMING_STATE_MSG_SIZE,
    INCOMING_STATE_BODY
};

/*
 * MSG_FORMAT_AND_HEADER_SIZE
 *
 * This should be >1, so that the close messages get handled by
 * the error callback instead of the read callback.  And ideally
 * it would be big enough to include the message size field.  But
 * we do not know the size of the message size field until we know
 * the format, so the best we can do is guess.  Finally, it must be
 * smaller than the total message header size of any format.
 */
#define MSG_FORMAT_AND_HEADER_SIZE 9


/*
 * Tables to map from fd to outgoings and incomings
 */
typedef enum
{
    FD_TABLE_TYPE_NONE,
    FD_TABLE_TYPE_OUTGOING,
    FD_TABLE_TYPE_INCOMING
} fd_table_type_t;

typedef struct _fd_table_t
{
    fd_table_type_t	type;
    void *		value;
} fd_table_t;

static int		fd_tablesize;
static fd_table_t *	fd_table;
static int		n_fds_open;

/*
 * My local TCP connection information
 */
static unsigned short	tcp_local_port;
static char		tcp_local_host[MAXHOSTNAMELEN];

static int		shutdown_waiting = 0;

/*
 * Outgoing table stuff.
 *
 * The outgoing table is hashed on host/port. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple tcp_outgoing_t
 * objects to the same context.  Multiple global pointers to the same
 * context share a tcp_outgoing_t.
 */
typedef struct _outgoing_table_entry_t
{
    tcp_outgoing_t *			outgoing;
    struct _outgoing_table_entry_t *	next;
} outgoing_table_entry_t;

#define OUTGOING_TABLE_SIZE 149

struct _outgoing_table_entry_t	outgoing_table[OUTGOING_TABLE_SIZE];

static void		outgoing_table_init(void);
static int		outgoing_table_hash(char *host, u_short port);
static void		outgoing_table_insert(tcp_outgoing_t *outgoing);
static void		outgoing_table_remove(tcp_outgoing_t *outgoing);
static tcp_outgoing_t *	outgoing_table_lookup(char *host, u_short port);

/*
 * Some useful queue macros
 */
#define Enqueue(Qhead, Qtail, Item) \
{ \
    if (Qhead) \
    { \
	(Qtail)->next = (Item); \
	(Qtail) = (Item); \
    } \
    else \
    { \
	(Qhead) = (Qtail) = (Item); \
    } \
}

#define Dequeue(Qhead, Qtail, Item) \
{ \
    (Item) = (Qhead); \
    (Qhead) = (Qhead)->next; \
}

#define QueueNotEmpty(Qhead)	(Qhead)


/*
 * Various forward declarations of procedures
 */
static nexus_proto_type_t	tcp_proto_type(void);
static void		tcp_init(int *argc, char ***argv);
static void		tcp_shutdown(nexus_bool_t shutdown_others);
static void		tcp_abort(void);

static int		tcp_send_rsr(nexus_buffer_t *buffer,
				nexus_startpoint_t *startpoint,
				int handler_id,
				nexus_bool_t destroy_buffer,
				nexus_bool_t called_from_non_threaded_handler);
static void             tcp_increment_reference_count(nexus_proto_t *nproto);
static nexus_bool_t	tcp_decrement_reference_count(nexus_proto_t *nproto);
static void		tcp_get_my_mi_proto(nexus_byte_t **array,
					    int *size);
static nexus_bool_t	tcp_construct_from_mi_proto(nexus_proto_t **proto,
						    nexus_mi_proto_t *mi_proto,
						    nexus_byte_t *proto_array,
						    int size);
static int		tcp_test_proto(nexus_proto_t *proto);
static int              tcp_direct_info_size(void);

static void		shutdown_write_callback(void *arg,
						int fd,
						char *buf,
						size_t nbytes);
static void		shutdown_write_error_callback(void *arg,
						      int fd,
						      char *buf,
						      size_t nbytes,
						      int error);

static void		outgoing_register_next_write(tcp_outgoing_t *outgoing);
static void		outgoing_write_callback(void *arg,
						int fd,
						char *buf,
						size_t nbytes);
static void		outgoing_writev_callback(void *arg,
						 int fd,
						 struct iovec *iov,
						 size_t iovcnt,
						 size_t nbytes);
static void		outgoing_write_error_callback(void *arg,
						      int fd,
						      char *buf,
						      size_t n_bytes,
						      int error);
static void		outgoing_writev_error_callback(void *arg,
						       int fd,
						       struct iovec *iov,
						       size_t iovcnt,
						       size_t n_bytes,
						       int error);

static void		outgoing_read_callback(void *arg,
					       int fd,
					       char *buf,
					       size_t nbytes,
					       char **new_buf,
					       size_t *new_max_nbytes,
					       size_t *new_wait_for_nbytes);
static void		outgoing_read_error_callback(void *arg,
						     int fd,
						     char *buf,
						     size_t nbytes,
						     int error);

static tcp_outgoing_t *	outgoing_construct(char *host, u_short port, int fd);
static void		outgoing_free(tcp_outgoing_t *outgoing);
static void		outgoing_open(tcp_outgoing_t *outgoing);
static void		outgoing_close(tcp_outgoing_t *outgoing,
				       int new_state);
static void		outgoing_close_normal(tcp_outgoing_t *outgoing);


static void		incoming_construct(int fd);
static void		incoming_close(tcp_incoming_t *incoming);
static void		incoming_free(tcp_incoming_t *incoming);

static void		incoming_read_callback(void *arg,
					       int fd,
					       char *buf,
					       size_t nbytes,
					       char **new_buf,
					       size_t *new_max_nbytes,
					       size_t *new_wait_for_nbytes);
static void		incoming_read_error_callback(void *arg,
						     int fd,
						     char *buf,
						     size_t nbytes,
						     int error);

static void		accept_internal_connection(void *arg, int fd);

static nexus_proto_funcs_t tcp_proto_funcs =
{
    tcp_proto_type,
    tcp_init,
    tcp_shutdown,
    tcp_abort,
    NULL /* tcp_poll */,
    NULL /* tcp_poll_blocking */,
    tcp_increment_reference_count,
    tcp_decrement_reference_count,
    tcp_get_my_mi_proto,
    tcp_construct_from_mi_proto,
    tcp_test_proto,
    tcp_send_rsr,
    tcp_direct_info_size,
    NULL /* tcp_direct_get */,
};



/*
 * _nx_pr_tcp_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *_nx_pr_tcp_info(void)
{
    return((void *) (&tcp_proto_funcs));
} /* _nx_pr_tcp_info() */


/*
 * tcp_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t tcp_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_TCP);
} /* tcp_proto_type() */


/*
 * tcp_init()
 *
 * Initialize the TCP protocol.
 */
static void tcp_init(int *argc, char ***argv)
{
    int i;
    int rc;

#ifndef BUILD_LITE
    nexus_mutex_init(&tcp_mutex, (nexus_mutexattr_t *) NULL);
#endif
    
    outgoing_table_init();
    tcp_done = NEXUS_FALSE;
    tcp_aborting = NEXUS_FALSE;

    /* Initialize the fd_table_* tables */ 
    fd_tablesize = nexus_fd_tablesize();
    NexusMalloc(tcp_init(),
		fd_table,
		fd_table_t *,
		(sizeof(fd_table_t) * fd_tablesize));
    for (i = 0; i < fd_tablesize; i++)
    {
	fd_table[i].type = FD_TABLE_TYPE_NONE;
	fd_table[i].value = NULL;
    }
    n_fds_open= 0;
    
    /*
     * Set up the listener for this process.
     */
    tcp_local_port = 0;
    rc = nexus_fd_create_listener(&tcp_local_port,
				  -1,
				  accept_internal_connection,
				  NULL);
    if (rc != 0)
    {
	nexus_fatal("tcp_init(): nexus_fd_create_listener() failed\n");
    }
    _nx_md_gethostname(tcp_local_host, MAXHOSTNAMELEN);
    nexus_debug_printf(3, ("tcp_init(): Listening on %d\n", tcp_local_port));

} /* tcp_init() */


/*
 * tcp_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 *
 * Shutdown the tcp protocol module by closing all off of the
 * outgoing's that have open fd's.  This will result in an orderly
 * shutdown of fd's, so that other nodes will not detect an
 * abnormal eof on the other ends of these fd's.
 *
 * It is ok if this procedure takes a little while to complete -- for
 * example to wait for another process to read off its fd and
 * free up some socket buffer space.
 *
 * An orderly closing of a connection is done as follows:
 *   1) Send the CLOSE_NORMAL_FLAG message down the fd (1 byte,
 *      signaling a close due to a normal termination) down the fd,
 *	if shutdown_others==NEXUS_FALSE.  Otherwise send the
 *	CLOSE_SHUTDOWN_FLAG.
 *   2) Then close the fd and its associated outgoing.
 */
static void tcp_shutdown(nexus_bool_t shutdown_others)
{
    int i;
    int fd;
    tcp_outgoing_t *outgoing;
    tcp_incoming_t *incoming;
    int rc;

    tcp_enter();
    
    nexus_fd_close_listener(tcp_local_port);
    
    tcp_done = NEXUS_TRUE;
    if (shutdown_others)
    {
	close_flag = CLOSE_SHUTDOWN_FLAG;
    }

    for (i = 0; i < fd_tablesize; i++)
    {
	switch(fd_table[i].type)
	{
	case FD_TABLE_TYPE_OUTGOING:
	    outgoing = (tcp_outgoing_t *) fd_table[i].value;
	    if (outgoing->state == OUTGOING_STATE_OPEN)
	    {
		nexus_debug_printf(1, ("tcp_shutdown(): closing outgoing 0x%lx\n", (unsigned long) outgoing));
		outgoing->state = OUTGOING_STATE_CLOSE_PENDING;
		if (!outgoing->write_in_progress)
		{
		    outgoing_register_next_write(outgoing);
		}
	    }
	    break;
	case FD_TABLE_TYPE_INCOMING:
	    incoming = (tcp_incoming_t *) fd_table[i].value;
	    nexus_debug_printf(1, ("tcp_shutdown(): closing incoming 0x%lx on fd=%d\n", (unsigned long) incoming, fd));
	    rc = nexus_fd_register_for_write(incoming->fd,
					     &close_flag,
					     1,
					     shutdown_write_callback,
					     shutdown_write_error_callback,
					     (void *) &(fd_table[i]));
	    /* TODO: Check the return code */
	    break;
	default:
	    /* Nothing registered for this fd */
	    break;
	}
    }

    tcp_exit();

    nexus_debug_printf(1, ("tcp_shutdown(): waiting for n_fds_open=%d fds to close\n", n_fds_open));
    while (n_fds_open > 0)
    {
	nexus_debug_printf(1, ("tcp_shutdown(): calling nexus_fd_handle_events() returned, n_fds_open=%d\n", n_fds_open));
	nexus_fd_handle_events(NEXUS_FD_POLL_NONBLOCKING_ALL, NULL);
	nexus_debug_printf(1, ("tcp_shutdown(): nexus_fd_handle_events() returned, n_fds_open=%d\n", n_fds_open));
	nexus_thread_yield();
    }
    nexus_debug_printf(1, ("tcp_shutdown(): done\n"));
    
} /* tcp_shutdown() */


/*
 * shutdown_write_callback()
 */
static void shutdown_write_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes)
{
    fd_table_t *fd_table_entry = (fd_table_t *) arg;
    tcp_incoming_t *incoming;

    nexus_debug_printf(1, ("shutdown_write_callback(): entering\n"));

    tcp_enter();
    
    if (fd_table_entry->type == FD_TABLE_TYPE_INCOMING)
    {
	incoming = (tcp_incoming_t *) fd_table_entry->value;
	incoming_close(incoming);
	incoming_free(incoming);
    }
    /*
     * else, the fd must have been closed between the tcp_shutdown()
     * and the invocation of this callback
     */

    tcp_exit();
    
    nexus_debug_printf(1, ("shutdown_write_callback(): exiting\n"));

} /* shutdown_write_callback() */


/*
 * shutdown_write_error_callback()
 */
static void shutdown_write_error_callback(void *arg,
					  int fd,
					  char *buf,
					  size_t nbytes,
					  int error)
{
    shutdown_write_callback(arg, fd, buf, nbytes);
} /* shutdown_write_error_callback() */


/*
 * tcp_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 *
 * Shutdown the tcp protocol module by closing all off of the
 * outgoing's that have open fd's.  This will hopefully result
 * in a reasonably  orderly shutdown of fd's, so that
 * other nodes do not spew error messages when they detect
 * closed sockets.
 *
 * However, this procedure should not wait to do things cleanly,
 * like tcp_shutdown().  If it cannot close a fd cleanly (first
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
static void tcp_abort()
{
    tcp_outgoing_t *outgoing;
    tcp_incoming_t *incoming;
    int i;

    close_flag = CLOSE_ABNORMAL_FLAG;

    tcp_done = NEXUS_TRUE;
    tcp_aborting = NEXUS_TRUE;
    nexus_fd_close_listener(tcp_local_port);

    for (i = 0; i < fd_tablesize; i++)
    {
	switch(fd_table[i].type)
	{
	case FD_TABLE_TYPE_OUTGOING:
	    outgoing = (tcp_outgoing_t *) fd_table[i].value;
	    if (outgoing->state == OUTGOING_STATE_OPEN)
	    {
		nexus_debug_printf(1, ("tcp_abort(): writing byte down outgoing, fd=%i\n", outgoing->fd));
		nexus_fd_write_one_nonblocking(outgoing->fd,
					       close_flag);
		outgoing_close(outgoing,
			       OUTGOING_STATE_FAULT);
		outgoing_free(outgoing);
	    }
	    break;
	case FD_TABLE_TYPE_INCOMING:
	    incoming = (tcp_incoming_t *) fd_table[i].value;
	    nexus_fd_write_one_nonblocking(incoming->fd,
					   close_flag);
	    incoming_close(incoming);
	    incoming_free(incoming);
	    break;
	default:
	    /* Nothing registered for this fd */
	    break;
	}
    }

} /* tcp_abort() */


/*
 * tcp_send_rsr()
 *
 * Generate a remote service request message to the node and context
 * saved in the 'nexus_buffer'.
 */
static int tcp_send_rsr(nexus_buffer_t *buffer,
			nexus_startpoint_t *startpoint,
			int handler_id,
			nexus_bool_t destroy_buffer,
			nexus_bool_t called_from_non_threaded_handler)
{
    int rc = 0;
    tcp_outgoing_t *outgoing;
    int fd;
    nexus_base_segment_t *base_segment;
    nexus_direct_segment_t *direct_segment;
    unsigned long total_direct_puts;
    int i;
    struct _nexus_buffer_t *r_buffer;
    nexus_barrier_t barrier;
    nexus_bool_t must_wait_for_write;

    NexusBufferMagicCheck(tcp_send_rsr(), buffer);
    NEXUS_INTERROGATE(startpoint, _NX_STARTPOINT_T, "tcp_send_rsr");
    nexus_debug_printf(2,("tcp_send_rsr(): invoked with buffer:%x\n",buffer));

#define THRESHOLD 2000 /* totally arbitrary and should be changed */
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
    
    if (r_buffer->n_direct > 0)
    {
	nexus_mutex_init(&(barrier.mutex), (nexus_mutexattr_t *) NULL);
	nexus_cond_init(&(barrier.cond), (nexus_condattr_t *) NULL);
	barrier.count = 0;
	must_wait_for_write = NEXUS_TRUE;
	r_buffer->barrier = &barrier;
    }
    else
    {
	must_wait_for_write = NEXUS_FALSE;
	r_buffer->barrier = (nexus_barrier_t *) NULL;
    }
    
    tcp_enter();
    
    outgoing = (tcp_outgoing_t *) startpoint->mi_proto->proto;
    NexusAssert2((outgoing->type == NEXUS_PROTO_TYPE_TCP),
		 ("tcp_send_rsr(): Internal error: proto_type is not NEXUS_PROTO_TYPE_TCP\n"));

    /*
     * Open up the outgoing if it is not already open.
     */
    if (outgoing->state != OUTGOING_STATE_OPEN)
    {
	if (outgoing->state == OUTGOING_STATE_UNOPENED)
	{
	    outgoing_open(outgoing); /* TODO: Deal with failed open */
	}
	else
	{
	    rc = outgoing->fault_code;
	    goto abort_send;
	}
    }
    
    fd = outgoing->fd;
    NexusAssert2((fd >= 0), ("tcp_send_rsr(): Internal error: Failed to open outgoing\n"));

    /* Enqueue this message on the outgoing */
    Enqueue(outgoing->write_q_head, outgoing->write_q_tail, r_buffer);
    
    if (!outgoing->write_in_progress)
    {
	/*
	 * Nobody else has registered a write on this outgoing,
	 * so register the one we just enqueued.
	 *
	 * If there is already a write in progress,
	 * the write callback will take care of registering this
	 * write (and any other ones that are enqueued) when
	 * the current write completes.
	 */
	outgoing_register_next_write(outgoing);
    }
    
    tcp_exit();

    /*
     * Wait for completion of the send.
     */
    if (must_wait_for_write)
    {
        nexus_mutex_lock(&(barrier.mutex));
	while (barrier.count > 0)
	{
	    nexus_cond_wait(&(barrier.cond), &(barrier.mutex));
	}
        nexus_mutex_unlock(&(barrier.mutex));
        nexus_mutex_destroy(&(barrier.mutex));
        nexus_cond_destroy(&(barrier.cond));
    }

    nexus_poll();

abort_send:
    return (rc);
} /* tcp_send_rsr() */


/*
 * outgoing_register_next_write()
 *
 * Register the next write operation for this outgoing.
 *
 * If the buffer at the head of the write_q list has remaining
 * direct componenents, then the next one is registered.
 * Otherwise, we go on to the next buffer in the
 * write_q (if there is one), and register it.
 *
 * If the write is complete on the buffer at the head of the write_q,
 * then this function destroys it.
 *
 * This function assumes that tcp_enter() has already been called.
 *
 * This function should not be called if 
 * outgoing->write_in_progres==NEXUS_TRUE.
 */
static void outgoing_register_next_write(tcp_outgoing_t *outgoing)
{
    struct _nexus_buffer_t *buffer;
    struct _nexus_buffer_t *completed_buffer = (struct _nexus_buffer_t *) NULL;
    nexus_bool_t done;
    int rc;

    outgoing->write_in_progress = NEXUS_FALSE;
    
    for (done = NEXUS_FALSE; !done; )
    {
	buffer = outgoing->write_q_head;
	if (!buffer)
	{
	    done = NEXUS_TRUE;
	}
	else if (buffer->current_base_segment)
	{
	    /*
	     * The first buffer in the queue as not been sent yet.
	     * So send its base segment.
	     */
	    
	    nexus_debug_printf(1, ("outgoing_register_next_write(): begin sending buffer base segment 0x%lx on fd=%i\n", (unsigned long) buffer, outgoing->fd));

	    /* Mark the base segment as written */
	    buffer->current_base_segment = (nexus_base_segment_t *) NULL;
	    outgoing->write_in_progress = NEXUS_TRUE;

	    /* Register the write on the base segment */
	    if (buffer->iovec_formatted)
	    {
		struct iovec *iov
		    = (struct iovec *) buffer->base_segments->current;
		size_t iovcnt = (int) buffer->base_segments->size_used;
		rc = nexus_fd_register_for_writev(
					outgoing->fd,
					iov,
					iovcnt,
					outgoing_writev_callback,
					outgoing_writev_error_callback,
					(void *) outgoing);
		/* TODO: Check the return code */
	    }
	    else
	    {
		nexus_byte_t *buf = buffer->base_segments->current;
		size_t size = buffer->base_segments->size_used;
		rc = nexus_fd_register_for_write(
					outgoing->fd,
					(char *) buf,
					size,
					outgoing_write_callback,
					outgoing_write_error_callback,
					(void *) outgoing);
		/* TODO: Check the return code */
	    }

	    done = NEXUS_TRUE;
	}
	else if (   buffer->direct_segments
		 && (buffer->direct_segments->n_left > 0))
	{
	    /*
	     * There is at least one more direct segment to send.
	     * So register the next direct segment.
	     */
	    nexus_direct_info_t *direct_info;
	
	    nexus_debug_printf(1, ("outgoing_register_next_write(): begin sending buffer direct segment 0x%lx on fd=%i\n", (unsigned long) buffer, outgoing->fd));

	    direct_info = buffer->direct_segments->current;
	    buffer->direct_segments->current++;
	    buffer->direct_segments->n_left--;
	    outgoing->write_in_progress = NEXUS_TRUE;
	
	    rc = nexus_fd_register_for_write(outgoing->fd,
					     (char *) direct_info->data,
					     direct_info->size,
					     outgoing_write_callback,
					     outgoing_write_error_callback,
					     (void *) outgoing);
	    /* TODO: Check the return code */
	    
	    done = NEXUS_TRUE;
	}
	else
	{
	    /*
	     * We are done with this buffer.
	     * So setup the completed buffer to be free below,
	     * and go on to the next buffer in the write_q.
	     */
	    Dequeue(outgoing->write_q_head,
		    outgoing->write_q_tail,
		    completed_buffer);
	    
	    nexus_debug_printf(1, ("outgoing_register_next_write(): done sending buffer 0x%lx on fd=%i\n", (unsigned long) completed_buffer, outgoing->fd));
	}
    }

    if (!buffer)
    {
	/*
	 * All of the buffers have been sent for this outgoing.
	 */
	if (outgoing->state == OUTGOING_STATE_CLOSE_PENDING)
	{
	    /*
	     * This outgoing is closing.
	     * So post a write of the CLOSE_NORMAL_FLAG.
	     */
	    outgoing->write_in_progress = NEXUS_TRUE;
	    outgoing->state = OUTGOING_STATE_CLOSE_POSTED;
	    nexus_debug_printf(1, ("outgoing_register_next_write(): registering close byte=%i on fd=%i\n", (int) close_flag, outgoing->fd));
	    rc = nexus_fd_register_for_write(outgoing->fd,
					     &close_flag,
					     1,
					     outgoing_write_callback,
					     outgoing_write_error_callback,
					     (void *) outgoing);
	    /* TODO: Check the return code */
	}
	else if (outgoing->state == OUTGOING_STATE_CLOSE_POSTED)
	{
	    /*
	     * A CLOSE_NORMAL_FLAG was just successfully sent
	     * on outgoing->fd.
	     * So close this outgoing.
	     */
	    nexus_debug_printf(1, ("outgoing_register_next_write(): close byte sent on fd=%i, closing outgoing\n", outgoing->fd));
	    outgoing_close(outgoing, OUTGOING_STATE_CLOSED);
	    outgoing_free(outgoing);
	}
    }

    if (completed_buffer)
    {
	/* Signal the thread waiting in tcp_send_rsr(), if there is one */
	if (completed_buffer->barrier)
	{
	    nexus_mutex_lock(&(completed_buffer->barrier->mutex));
	    completed_buffer->barrier->count--;
	    nexus_cond_signal(&(completed_buffer->barrier->cond));
	    nexus_mutex_unlock(&(completed_buffer->barrier->mutex));
	}

	/* Destroy the buffer */
	nexus_buffer_destroy(&completed_buffer);
    }
    
} /* outgoing_register_next_write() */


/*
 * outgoing_write_callback()
 */
static void outgoing_write_callback(void *arg,
				    int fd,
				    char *buf,
				    size_t nbytes)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;
    nexus_debug_printf(1, ("outgoing_write_callback(): entering\n"));
    tcp_enter();
    outgoing_register_next_write(outgoing);
    tcp_exit();
    nexus_debug_printf(1, ("outgoing_write_callback(): exiting\n"));
} /* outgoing_write_callback() */


/*
 * outgoing_writev_callback()
 */
static void outgoing_writev_callback(void *arg,
				     int fd,
				     struct iovec *iov,
				     size_t iovcnt,
				     size_t nbytes)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;
    tcp_enter();
    outgoing_register_next_write(outgoing);
    tcp_exit();
} /* outgoing_writev_callback() */


/*
 * outgoing_write_error_callback()
 *
 * TODO: This should try to re-establish the connection and retry the send.
 */
static void outgoing_write_error_callback(void *arg,
					  int fd,
					  char *buf,
					  size_t n_bytes,
					  int error)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;

    nexus_debug_printf(1, ("outgoing_write_error_callback(): entering\n"));

    if (tcp_done)
    {
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
    }
    
    if (error == EPIPE)
    {
        /*
	 * The outgoing fd was closed unexpectedly.
	 * If the process at the other end died, or
	 * erroneously closed the fd.
	 */
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_DIED;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_fatal("outgoing_write_error_callback(): fd unexpectedly closed. Another process probably died: errno=%d: %s\n", error, _nx_md_system_error_string(error));
	}
    }
    else
    {
        nexus_fatal("outgoing_write_error_callback(): Write failed (errno=%i): %s\n", error, _nx_md_system_error_string(error));
    }

    nexus_debug_printf(1, ("outgoing_write_error_callback(): exiting\n"));

} /* outgoing_write_error_callback() */


/*
 * outgoing_writev_error_callback()
 *
 * TODO: This should try to re-establish the connection and retry the send.
 */
static void outgoing_writev_error_callback(void *arg,
					   int fd,
					   struct iovec *iov,
					   size_t iovcnt,
					   size_t n_bytes,
					   int error)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;
    
    if (tcp_done)
    {
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
    }
    
    if (error == EPIPE)
    {
        /*
	 * The outgoing fd was closed unexpectedly.
	 * If the process at the other end died, or
	 * erroneously closed the fd.
	 */
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_DIED;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_fatal("outgoing_writev_error_callback(): fd unexpectedly closed. Another process probably died: errno=%d: %s\n", error, _nx_md_system_error_string(error));
	}
    }
    else
    {
        nexus_fatal("outgoing_writev_error_callback(): Write failed (errno=%i): %s\n", error, _nx_md_system_error_string(error));
    }
} /* outgoing_writev_error_callback() */


/*
 * outgoing_read_callback()
 *
 * This function is called if a byte is received on an outgoing fd
 */
static void outgoing_read_callback(void *arg,
				   int fd,
				   char *buf,
				   size_t nbytes,
				   char **new_buf,
				   size_t *new_max_nbytes,
				   size_t *new_wait_for_nbytes)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;

    nexus_debug_printf(1, ("outgoing_read_callback(): entering\n"));
    NexusAssert2((nbytes == 1), ("outgoing_read_callback(): got nbytes=%lu", (unsigned long) nbytes));

    if (buf[0] == CLOSE_NORMAL_FLAG)
    {
	/*
	 * The other end closed this fd normally.
	 * So just close this outgoing on proceed normally.
	 */
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
    }
    else if (buf[0] == CLOSE_ABNORMAL_FLAG)
    {
	/*
	 * We got an abnormal close of a outgoing.
	 * So silent fatal out if we are not fault tolerant,
	 * or set the outgoing to be bad if we are.
	 */
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_ABNORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_silent_fatal();
	}
    }
    else if (buf[0] == CLOSE_SHUTDOWN_FLAG)
    {
	/*
	 * The other end closed this fd in preparation for shutdown.
	 * So close this outgoing, and then exit this process while
	 * propogating the shutdown.
	 */
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_exit(0, NEXUS_TRUE);
	}
    }
    else
    {
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_BAD_PROTOCOL;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_fatal("outgoing_read_callback(): Internal error: Read unexpected data from a outgoing\n");
	}
    }
    nexus_debug_printf(1, ("outgoing_read_callback(): exiting\n"));
	
} /* outgoing_read_callback() */


/*
 * outgoing_read_error_callback()
 *
 * This function is called if an outgoing fd receives an error while reading
 */
static void outgoing_read_error_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes,
					 int error)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) arg;

    nexus_debug_printf(1, ("outgoing_read_error_callback(): entering\n"));

    if (tcp_done)
    {
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
    }
    
    if (   (error == 0)
	|| (error == ECONNRESET)
	|| (error == EPIPE) )
    {
	tcp_enter();
	outgoing->fault_code = NEXUS_FAULT_PROCESS_DIED;
	outgoing_close(outgoing, OUTGOING_STATE_FAULT);
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_fatal("outgoing_read_error_callback(): fd unexpectedly closed. Another process probably died: errno=%d: %s\n", error, _nx_md_system_error_string(error));
	}
    }
    else
    {
	tcp_fatal("outgoing_read_error_callback(): Read failed on outgoing (errno=%d): %s\n", error, _nx_md_system_error_string(error));
    }
    
    nexus_debug_printf(1, ("outgoing_read_error_callback(): exiting\n"));
    
} /* outgoing_read_error_callback() */


/*
 * outgoing_construct()
 *
 * Construct a tcp_outgoing_t for the given host and port. Look up in the
 * outgoing table to see if one already exists. If it does, bump its reference
 * count and return that one. Otherwise create one, insert into the
 * table with a reference count of 1 and return it.
 */
static tcp_outgoing_t *outgoing_construct(char *host, u_short port, int fd)
{
    tcp_outgoing_t *outgoing;

    outgoing = outgoing_table_lookup(host, port);
    nexus_debug_printf(3, ("outgoing_construct(): Table lookup returns outgoing=%x\n", outgoing));
    if (outgoing == (tcp_outgoing_t *) NULL)
    {
	NexusMalloc(outgoing_construct(), outgoing, tcp_outgoing_t *,
		    sizeof(tcp_outgoing_t));

	outgoing->type = NEXUS_PROTO_TYPE_TCP;
	outgoing->funcs = &tcp_proto_funcs;
	outgoing->host = _nx_copy_string(host);
	outgoing->port = port;
	outgoing->fd = fd;
	outgoing->state = OUTGOING_STATE_UNOPENED;
	outgoing->fault_code = NEXUS_FAULT_NONE;
	outgoing->reference_count = 1;
	outgoing->write_in_progress = NEXUS_FALSE;
	outgoing->write_q_head = (struct _nexus_buffer_t *) NULL;
	outgoing->write_q_tail = (struct _nexus_buffer_t *) NULL;
	
	outgoing_table_insert(outgoing);
    }
    else
    {
	outgoing->reference_count++;
    }
	
    return (outgoing);
} /* outgoing_construct() */


/*
 * outgoing_free()
 *
 * Free the passed 'outgoing'.
 */
static void outgoing_free(tcp_outgoing_t *outgoing)
{
    if (outgoing->host != (char *) NULL)
    {
	NexusFree(outgoing->host);
    }
    NexusFree(outgoing);
} /* outgoing_free() */


/*
 * outgoing_open()
 *
 * Open the connection for the passed outgoing.  If it is already
 * connected, then just return.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 */
static void outgoing_open(tcp_outgoing_t *outgoing)
{
    int rc;
    
    if (outgoing->state == OUTGOING_STATE_OPEN)
    {
	/* This outgoing is already open, so return */
	return;
    }

    rc = nexus_fd_connect(outgoing->host, outgoing->port, &outgoing->fd);
    if (rc == 0)
    {
	nexus_debug_printf(1, ("outgoing_open(): do_connect(%s/%hu) returns fd=%d\n",
			       outgoing->host, outgoing->port, outgoing->fd));
	fd_table[outgoing->fd].type = FD_TABLE_TYPE_OUTGOING;
	fd_table[outgoing->fd].value = (void *) outgoing;
	n_fds_open++;

	outgoing->state = OUTGOING_STATE_OPEN;

	rc = nexus_fd_register_for_read(outgoing->fd,
					outgoing->read_buf,
					1,
					1,
					outgoing_read_callback,
					outgoing_read_error_callback,
					(void *) outgoing);
	/* TODO: Check the return code */
    }
    else
    {
	outgoing->state = OUTGOING_STATE_FAULT;
	outgoing->fault_code = NEXUS_FAULT_CONNECT_FAILED;
	tcp_exit();
	if (_nx_fault_detected(outgoing->fault_code) != 0)
	{
	    nexus_fatal("outgoing_open(): Failed to connect to %s:%hu\n",
			outgoing->host, outgoing->port);
	}
	tcp_enter();
    }

} /* outgoing_open() */


/*
 * outgoing_close()
 *
 * Close a outgoing:
 *   1) Remove the outgoing from the fd_table table
 *   2) Close the fd
 *   3) Modify outgoing data structure
 */
static void outgoing_close(tcp_outgoing_t *outgoing,
			   int new_state)
{
#ifdef BUILD_DEBUG
    if (NexusDebug(1))
    {
        struct sockaddr_in local, remote;
        int len;
	len = sizeof(local);
        getsockname(outgoing->fd, (struct sockaddr *) &local, &len);
	len = sizeof(remote);
        getpeername(outgoing->fd, (struct sockaddr *) &remote, &len);

	nexus_printf("outgoing_close(): closing outgoing %x %s/%hu fd=%d local=%hu remote=%hu\n",
		     outgoing,
		     outgoing->host,
		     outgoing->port,
		     outgoing->fd,
		     htons(local.sin_port),
		     htons(remote.sin_port));
    }
#endif

    outgoing->state = OUTGOING_STATE_CLOSING;
    nexus_fd_unregister(outgoing->fd, NULL);
    nexus_fd_close(outgoing->fd);
    outgoing->state = new_state;
    outgoing->fd = -1;

    fd_table[outgoing->fd].type = FD_TABLE_TYPE_NONE;
    fd_table[outgoing->fd].value = NULL;
    n_fds_open--;
    
} /* outgoing_close() */


/*
 * outgoing_close_normal()
 */
static void outgoing_close_normal(tcp_outgoing_t *outgoing)
{
    outgoing->state = OUTGOING_STATE_CLOSE_PENDING;
    if (!outgoing->write_in_progress)
    {
	outgoing_register_next_write(outgoing); /* This calls tcp_exit() */
    }
} /* outgoing_close_normal() */

				       
/*
 * tcp_increment_reference_count()
 *
 * Increase the reference count on the associated proto and copy the
 * pointer to the nexus_proto_t
 *
 */
static void tcp_increment_reference_count(nexus_proto_t *nproto)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) nproto;
    tcp_enter();
    outgoing->reference_count++;
    tcp_exit();
} /* tcp_increment_reference_count() */


/*
 * tcp_decrement_reference_count()
 *
 * Decrement the reference count for this proto.  If it goes to 0
 * then close the fd used by this outgoing.
 *
 * Return NEXUS_TRUE if this functioin frees the proto.
 */
static nexus_bool_t tcp_decrement_reference_count(nexus_proto_t *nproto)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) nproto;
    nexus_bool_t proto_freed = NEXUS_FALSE;
    
    tcp_enter();
    outgoing->reference_count--;
    
    NexusAssert2((outgoing->reference_count >= 0), ("tcp_decrement_reference_count(): Internal error: Reference count < 0\n"));
	       
    if (outgoing->reference_count == 0)
    {
	outgoing_table_remove(outgoing);
	if (outgoing->state == OUTGOING_STATE_OPEN)
	{
	    outgoing_close_normal(outgoing);
	}
	proto_freed = NEXUS_TRUE;
    }
    
    tcp_exit();
    return(proto_freed);
    
} /* tcp_decrement_reference_count() */


/*
 * tcp_get_my_mi_proto()
 *
 * Return in 'array' and 'size' a byte array containing
 * enough information to enable another process to connect
 * to this one.  This byte array will be assembled with those
 * of the other protocol modules and placed in a nexus_mi_proto_t.
 *
 * Fill in 'array' with:
 *	port  (4 byte, big endian integer)
 *	hostname (null terminated string)
 *
 * If the hostname is the same as my node name, then just place an
 * empty string in the hostname field.
 */
static void tcp_get_my_mi_proto(nexus_byte_t **array,
				int *size)
{
    int i;
    int tmp_int;
    char *host;
    int host_length;

    if (strcmp(tcp_local_host, _nx_my_node.name) == 0)
    {
	host = "";
    }
    else
    {
	host = tcp_local_host;
    }
    
    host_length = (strlen(host) + 1);
    *size = 4 + host_length;
    NexusMalloc(tcp_get_my_mi_proto(),
		*array,
		nexus_byte_t *,
		*size);
    tmp_int = (int) tcp_local_port;
    i = 0;
    PackInt4(*array, i, tmp_int);
    memcpy(&((*array)[i]), host, host_length);
    
} /* tcp_get_my_mi_proto() */


/*
 * tcp_construct_from_mi_proto()
 *
 * From the passed machine independent protocol list ('mi_proto'), plus
 * the tcp specific entry from that list ('proto_array' and 'size'),
 * see if I can use the information to create a nexus_proto_t object
 * that can be used to connect to the node:
 *	- If I cannot use this protocol to attach to the node, then
 *		return NEXUS_FALSE.  (This option is useful if two nodes
 *		both speak a particular protocol, but they cannot
 *		talk to each other via that protocol.  For example,
 *		on two MPP's, the nodes within a single MPP can
 *		talk to each other via the native messaging protocol,
 *		but cannot talk to the nodes on the other MPP
 *		using that native protocol.)
 *	- If this tcp protocol points to myself, and thus the local
 *		protocol module should be used, then set
 *		*proto=NULL, and return NEXUS_TRUE.
 *	- Otherwise, construct a tcp protocol object for this mi_proto
 *		and put it in *proto.  Then return NEXUS_TRUE.
 *
 * The 'proto_array' should contain:
 *	port  (4 byte, big endian integer)
 *	hostname (null terminated string)
 *
 * If the hostname is an empty string, then use the node name
 * from the mi_proto.
 */
static nexus_bool_t tcp_construct_from_mi_proto(nexus_proto_t **proto,
						nexus_mi_proto_t *mi_proto,
						nexus_byte_t *proto_array,
						int size)
{
    char *host;
    u_short port;
    int i;
    int tmp_int;

    NexusAssert2((size >= 5),
		 ("tcp_construct_from_mi_proto(): Invalid tcp information in mi_proto\n"));
		 
    /* Get the port and hostname */
    i = 0;
    UnpackInt4(proto_array, i, tmp_int);
    port = (u_short) tmp_int;
    host = (char *) &(proto_array[i]);
    if (host[0] == '\0')
    {
	int node_number, context_differentiator;
	i = 0;
	UnpackMIProtoHeader(mi_proto->array, i,
			    node_number,
			    context_differentiator,
			    host);
	NexusAssert2((strlen(host) > 0),
		     ("tcp_construct_from_mi_proto(): Invalid node name field in mi_proto\n"));
    }

	
    /*
     * Test to see if this proto points to myself.
     * If it does, then return the _nx_local_proto.
     */
    if (   (port == tcp_local_port)
	   && (strcmp(host, tcp_local_host) == 0) )
    {
	*proto = (nexus_proto_t *) NULL;
    }
    else
    {
	tcp_enter();
	*proto = (nexus_proto_t *) outgoing_construct(host, port, -1);
	tcp_exit();
    }
    return (NEXUS_TRUE);
} /* tcp_construct_from_mi_proto() */


/*
 * tcp_test_proto()
 */
static int tcp_test_proto(nexus_proto_t *proto)
{
    tcp_outgoing_t *outgoing = (tcp_outgoing_t *) proto;
    return(outgoing->fault_code);
} /* tcp_test_proto() */


/*
 * tcp_direct_info_size()
 */
static int tcp_direct_info_size(void)
{
    /* TODO: This needs to be filled in */
    return(0);
} /* tcp_direct_info_size() */


/*
 * incoming_construct()
 *
 * Construct a tcp_incoming_t for the given file descriptor, 'fd'.
 */
static void incoming_construct(int fd)
{
    tcp_incoming_t *incoming;
    int rc;

    NexusMalloc(incoming_construct(),
		incoming,
		tcp_incoming_t *,
		sizeof(tcp_incoming_t));
    NexusMalloc(incoming_construct(),
		incoming->storage,
		nexus_byte_t *,
		TCP_INCOMING_DEFAULT_SIZE);

    incoming->fd = fd;
    incoming->state = INCOMING_STATE_FORMAT;
    incoming->nbytes_parsed = 0;
    incoming->nbytes_unparsed = 0;
    incoming->storage_size = TCP_INCOMING_DEFAULT_SIZE;
    incoming->current = incoming->storage;
    incoming->save_storage_size = 0;
    incoming->dispatch_in_progress = NEXUS_FALSE;
    incoming->dispatch_q_head = (struct _nexus_buffer_t *) NULL;
    incoming->dispatch_q_tail = (struct _nexus_buffer_t *) NULL;

    fd_table[fd].type = FD_TABLE_TYPE_INCOMING;
    fd_table[fd].value = (void *) incoming;
    n_fds_open++;
    
    rc = nexus_fd_register_for_read(incoming->fd,
				    (char *) incoming->storage,
				    incoming->storage_size,
				    MSG_FORMAT_AND_HEADER_SIZE,
				    incoming_read_callback,
				    incoming_read_error_callback,
				    (void *) incoming);
    /* TODO: Check the return code */

} /* incoming_construct() */


/*
 * incoming_close()
 *
 * Close an incoming connection:
 *   1) Remove the tcp_incoming_t from the fd_table table
 *   2) Close the fd
 *   3) Put the tcp_incoming_t back on the free list.
 */
static void incoming_close(tcp_incoming_t *incoming)
{
#ifdef BUILD_DEBUG
    if (NexusDebug(2))
    {
        struct sockaddr_in local, remote;
        int len;
	len = sizeof(local);
        getsockname(incoming->fd, (struct sockaddr *) &local, &len);
	len = sizeof(remote);
        getpeername(incoming->fd, (struct sockaddr *) &remote, &len);

	nexus_printf("incoming_close(): closing %x fd=%d local=%hu remote=%hu\n",
		     incoming,
		     incoming->fd,
		     htons(local.sin_port),
		     htons(remote.sin_port));
    }
#endif

    nexus_fd_unregister(incoming->fd, NULL);
    nexus_fd_close(incoming->fd);

    fd_table[incoming->fd].type = FD_TABLE_TYPE_NONE;
    fd_table[incoming->fd].value = NULL;
    n_fds_open--;
    
} /* incoming_close() */


/*
 * incoming_free()
 */
static void incoming_free(tcp_incoming_t *incoming)
{
    NexusFree(incoming->storage);
    NexusFree(incoming);
} /* incoming_free() */


/*
 * incoming_read_callback()
 *
 * EOF and the CLOSE_* flags that accompany and EOF are
 * handled by incoming_read_error_callback().
 */
static void incoming_read_callback(void *arg,
				   int fd,
				   char *buf,
				   size_t nbytes_read,
				   char **new_buf,
				   size_t *new_max_nbytes,
				   size_t *new_wait_for_nbytes)
{
    tcp_incoming_t *incoming = (tcp_incoming_t *) arg;
    nexus_bool_t done;
    int sizeof_u_long;
    unsigned long tmp_u_long;
    struct _nexus_buffer_t *buffer;
    nexus_bool_t message_enqueued = NEXUS_FALSE;

    tcp_enter();

    if (tcp_done)
    {
	/* Flush incoming data */
	*new_buf = (char *) incoming->storage;
	*new_max_nbytes = incoming->storage_size;
	*new_wait_for_nbytes = *new_max_nbytes;
	return;
    }
    
    NexusAssert2((   (incoming->state == INCOMING_STATE_FORMAT)
		  || (incoming->state == INCOMING_STATE_MSG_SIZE)
		  || (incoming->state == INCOMING_STATE_BODY) ),
		 ("incoming_read_callback(): Internal error: Invalid incoming->state = %d\n", incoming->state) );

    nexus_debug_printf(4, ("starting with %d bytes parsed and %d bytes unparsed and with %d new bytes read", incoming->nbytes_parsed, incoming->nbytes_unparsed, nbytes_read));

    incoming->nbytes_unparsed += nbytes_read;

    for (done = NEXUS_FALSE; !done; )
    {
        switch(incoming->state)
	{
	case INCOMING_STATE_FORMAT:
	    nexus_debug_printf(3, ("parsing format\n"));
	    if (incoming->nbytes_unparsed < MSG_FORMAT_AND_HEADER_SIZE)
	    {
		nexus_debug_printf(3, ("not enough bytes...posting read for more\n"));
		/*
		 * Not enough bytes in the buffer to get the format.
		 * So post a read for it.
		 */
		*new_buf = (char *) (incoming->current
				     + incoming->nbytes_unparsed);
		*new_max_nbytes = (incoming->storage_size
				   - incoming->nbytes_unparsed
				   - incoming->nbytes_parsed);
		*new_wait_for_nbytes = (MSG_FORMAT_AND_HEADER_SIZE
					- incoming->nbytes_unparsed);
		done = NEXUS_TRUE;
	    }
	    else
	    {
		/*
		 * Get the format from the buffer.
		 */
		incoming->format = (int) *(incoming->current);
		incoming->current++;
		incoming->nbytes_parsed++;
		incoming->nbytes_unparsed--;
		nexus_debug_printf(4, ("F: nbytes_parsed is %d and nbytes_unparsed is %d\n", incoming->nbytes_parsed, incoming->nbytes_unparsed));
		nexus_debug_printf(3, ("format is %d\n", incoming->format));

		incoming->state = INCOMING_STATE_MSG_SIZE;
		nexus_debug_printf(3, ("state is INCOMING_STATE_MSG_SIZE\n"));
	    }
	    break;
	    
	case INCOMING_STATE_MSG_SIZE:
	    nexus_debug_printf(3, ("getting message size\n"));
	    /* Get the message size */
	    sizeof_u_long = nexus_dc_sizeof_remote_u_long(1, incoming->format);
	    if (incoming->nbytes_unparsed < sizeof_u_long)
	    {
		nexus_debug_printf(3, ("not enough bytes...posting read for more\n"));
		/*
		 * Not enough bytes in the buffer to get the message size.
		 * So post a read for it.
		 */
		*new_buf = (char *) (incoming->current
				     + incoming->nbytes_unparsed);
		*new_max_nbytes = (incoming->storage_size
				   - incoming->nbytes_unparsed
				   - incoming->nbytes_parsed);
		*new_wait_for_nbytes = (sizeof_u_long
					- incoming->nbytes_unparsed);
		done = NEXUS_TRUE;
	    }
	    else
	    {
		/* Get the message size */
		nexus_dc_get_u_long(&(incoming->current),
				    &(incoming->msg_size),
				    1,
				    incoming->format);
		incoming->nbytes_parsed += sizeof_u_long;
		incoming->nbytes_unparsed -= sizeof_u_long;
		nexus_debug_printf(4, ("B: nbytes_parsed is %d and nbytes_unparsed is %d\n", incoming->nbytes_parsed, incoming->nbytes_unparsed));
	        nexus_debug_printf(3, ("size is %lu\n", incoming->msg_size));

		if (incoming->msg_size > incoming->storage_size)
		{
		    nexus_debug_printf(3, ("message too big to fit in current storage\n"));
		    /*
		     * The message is too big to fit in this storage.
		     * So allocate new storage that is big enough,
		     * copy the message into the new storage.
		     */
		    incoming->save_storage_size = incoming->storage_size;
		    incoming->save_storage = incoming->storage;
		    incoming->save_current = incoming->storage;

		    /* Allocate new storage */
		    NexusMalloc(incoming_read_callback(),
				incoming->storage,
				nexus_byte_t *,
				incoming->msg_size);
		    incoming->storage_size = incoming->msg_size;

		    /* Copy message to new storage */
		    tmp_u_long = (incoming->nbytes_parsed
				  + incoming->nbytes_unparsed);
		    memcpy(incoming->storage,
			   incoming->save_storage,
			   tmp_u_long);
		    incoming->current = (incoming->storage
		    			 + incoming->nbytes_parsed);

		    nexus_debug_printf(3, ("post read for rest of message\n"));
		    /* Setup new read for rest of message */
		    *new_buf = (char *) (incoming->current
		    			 + incoming->nbytes_unparsed);
		    *new_max_nbytes = (incoming->msg_size
				       - incoming->nbytes_unparsed
				       - incoming->nbytes_parsed);
		    *new_wait_for_nbytes = *new_max_nbytes;
		    nbytes_read = 0;
		}
		
		/* Reset the state to get the body */
		incoming->state = INCOMING_STATE_BODY;
		nexus_debug_printf(3, ("state is INCOMING_STATE_BODY\n"));
	    }
	    break;
	    
	case INCOMING_STATE_BODY:
	    nexus_debug_printf(3, ("reading body of message\n"));
	    if ((incoming->nbytes_parsed + incoming->nbytes_unparsed)
		< incoming->msg_size)
	    {
		nexus_debug_printf(3, ("still need more...posting read\n"));
		/*
		 * The whole message has not been received yet.
		 * So post a read for it.
		 */
		*new_buf = (char *) (incoming->current
				     + incoming->nbytes_unparsed);
		*new_max_nbytes = (incoming->storage_size
				   - incoming->nbytes_unparsed
				   - incoming->nbytes_parsed);
		*new_wait_for_nbytes = (incoming->msg_size
					- incoming->nbytes_parsed
					- incoming->nbytes_unparsed);
		done = NEXUS_TRUE;
	    }
	    else
	    {
		nexus_debug_printf(3, ("got all of the message\n"));
		if ((incoming->nbytes_parsed + incoming->nbytes_unparsed)
		    > incoming->msg_size)
		{
		    nexus_debug_printf(3, ("got part of next message\n"));
		    /*
		     * We've read past the end of this message.
		     * So allocate new storage to hold the extra bytes.
		     *
		     * We know that incoming->storage is of default size,
		     * so we can just allocate another default size
		     * storage to hold the overflow.
		     * If the message was bigger than the default
		     * storage size, then storage was allocated to
		     * be exactly the right size for the message, and
		     * a read was posted only for that many bytes.
		     */
		    NexusAssert2((incoming->storage_size == TCP_INCOMING_DEFAULT_SIZE), ("incoming_read_callback(): Internal error: Got a read overflow on a large message.  This shouldn't happen.\n"));
				  
		    NexusMalloc(incoming_read_callback(),
				incoming->save_storage,
				nexus_byte_t *,
				TCP_INCOMING_DEFAULT_SIZE);
		    incoming->save_storage_size = TCP_INCOMING_DEFAULT_SIZE;

		    tmp_u_long = (incoming->nbytes_parsed
				  + incoming->nbytes_unparsed
				  - incoming->msg_size);
		    memcpy(incoming->save_storage,
			   (incoming->storage + incoming->msg_size),
			   tmp_u_long);
		    incoming->save_current = (incoming->save_storage
					      + tmp_u_long);
		    incoming->nbytes_unparsed -= tmp_u_long;
		}
		
		/* Enqueue the message for dispatch */
		_nx_buffer_create_from_raw(incoming->storage,
					   incoming->msg_size,
					   &buffer);
		Enqueue(incoming->dispatch_q_head,
			incoming->dispatch_q_tail,
			buffer);
		message_enqueued = NEXUS_TRUE;
		nexus_debug_printf(3, ("message enqueued\n"));
		    
		/* Reset the incoming state to INCOMING_STATE_FORMAT */
		if (incoming->save_storage_size > 0)
		{
		    nexus_debug_printf(3, ("next message using saved storage\n"));
		    /* There is a buffer in the save_storage to use */
		    incoming->state = INCOMING_STATE_FORMAT;
		    incoming->storage_size = incoming->save_storage_size;
		    incoming->storage = incoming->save_storage;
		    incoming->current = incoming->save_storage;
		    incoming->save_storage_size = 0;
		    incoming->nbytes_parsed = 0;
		    incoming->nbytes_unparsed = (incoming->save_current
						 - incoming->save_storage);
		    nexus_debug_printf(4, ("D: nbytes_parsed is %d and nbytes_unparsed is %d\n", incoming->nbytes_parsed, incoming->nbytes_unparsed));
		}
		else
		{
		    nexus_debug_printf(3, ("next message using new storage\n"));
		    /* Must allocate new storage */
		    incoming->state = INCOMING_STATE_FORMAT;
		    incoming->storage_size = TCP_INCOMING_DEFAULT_SIZE;
		    NexusMalloc(incoming_read_callback(),
				incoming->storage,
				nexus_byte_t *,
				TCP_INCOMING_DEFAULT_SIZE);
		    incoming->current = incoming->storage;
		    incoming->nbytes_parsed = 0;
		    incoming->nbytes_unparsed = 0;
		    nexus_debug_printf(4, ("E: nbytes_parsed is %d and nbytes_unparsed is %d\n", incoming->nbytes_parsed, incoming->nbytes_unparsed));
		}
		nexus_debug_printf(3, ("state is INCOMING_STATE_FORMAT\n"));
	    }
	    break;
	}
    }

    if (message_enqueued && !incoming->dispatch_in_progress)
    {
	incoming->dispatch_in_progress = NEXUS_TRUE;
	while (QueueNotEmpty(incoming->dispatch_q_head))
	{
	    Dequeue(incoming->dispatch_q_head,
		    incoming->dispatch_q_tail,
		    buffer);
	    tcp_exit();
	    nexus_debug_printf(3, ("dispatching buffer\n"));
	    _nx_buffer_dispatch(buffer);
	    tcp_enter();
	}
	incoming->dispatch_in_progress = NEXUS_FALSE;
    }

    tcp_exit();

} /* incoming_read_callback() */


/*
 * incoming_read_error_callback()
 */
static void incoming_read_error_callback(void *arg,
					 int fd,
					 char *buf,
					 size_t nbytes_read,
					 int error)
{
    tcp_incoming_t *incoming = (tcp_incoming_t *) arg;
    nexus_byte_t flag;

    if (tcp_done)
    {
	incoming_close(incoming);
	return;
    }
    
    tcp_enter();
    
    nbytes_read += incoming->nbytes_unparsed;

    if (   (incoming->state == INCOMING_STATE_FORMAT)
	&& (error == 0)
	&& (nbytes_read == 1))
    {
	/*
	 * It appears we have a valid CLOSE_* message, followed
	 * by an end-of-file.
	 */
	flag = incoming->storage[0] & FLAG_MASK;
	incoming_close(incoming);

	if (flag == CLOSE_NORMAL_FLAG)
	{
	    /* Do nothing */
	}
	else if (flag == CLOSE_ABNORMAL_FLAG)
	{
	    /*
	     * We got an abnormal close message on this fd.
	     * This means some other node fataled out,
	     * and sent this down the pipe just before dieing.
	     * So if fault detection is not turned on, then
	     * fatal out silently, since the node that
	     * generated this message will have complained
	     * already.  (No need for cascading fatal messages.)
	     */
	    tcp_exit();
	    if (_nx_fault_detected(NEXUS_FAULT_PROCESS_SHUTDOWN_ABNORMALLY) != 0)
	    {
		nexus_silent_fatal();
	    }
	    tcp_enter();
	}
	else if (flag == CLOSE_SHUTDOWN_FLAG)
	{
	    /*
	     * We got an shutdown close message on this fd.
	     * This means some other node is shutting down
	     * and is telling us to do the same.
	     * So if fault detection is not turned on, then
	     * shutdown and propogate it to other nodes.
		 */
	    tcp_exit();
	    if (_nx_fault_detected(NEXUS_FAULT_PROCESS_SHUTDOWN_NORMALLY) != 0)
	    {
		nexus_exit(0, NEXUS_TRUE);
	    }
	    tcp_enter();
	}
	else
	{
	    tcp_exit();
	    if (_nx_fault_detected(NEXUS_FAULT_BAD_PROTOCOL) != 0)
	    {
		nexus_fatal("incoming_read_error_callback(): Internal error: Got an illegal first byte of message header: %i\n", (int) incoming->storage[0]);
	    }
	    tcp_enter();
	}
	incoming_free(incoming);
    }
    else if (error == 0)
    {
	incoming_close(incoming);
	tcp_exit();
	if (_nx_fault_detected(NEXUS_FAULT_BAD_PROTOCOL) != 0)
	{
	    nexus_fatal("incoming_read_error_callback(): Internal error: Got an unexpected end-of-file\n");
	}
	tcp_enter();
	incoming_free(incoming);
    }
    else if (   (error == ECONNRESET)
	     || (error == EPIPE) )
    {
	/*
	 * Got connection reset by peer on the read, so:
	 *   if fault tolerance is not enabled then die
	 *   else, close the incoming and keep on going
	 *
	 * We should never get an EOF on read under normal
	 * circumstances.  We should always get (and handle) a
	 * close message (the leading byte of a message) before
	 * a fd is closed, so we can handle the closed fd
	 * before we ever get an EOF.  This allows us to
	 * detect abnormal termination of any process we
	 * are connected to.
	 */
	incoming_close(incoming);
	tcp_exit();
	if (_nx_fault_detected(NEXUS_FAULT_PROCESS_DIED) != 0)
	{
	    /* die */
	    int len;
	    struct sockaddr_in addr;
	
	    len = sizeof(addr);
	    if (getpeername(incoming->fd,
			    (struct sockaddr *) &addr,
			    &len) == 0)
	    {
		tcp_fatal("incoming_read_error_callback(): fd %d connected to %s/%d was unexpectedly closed: n_read=%d\n",
			  incoming->fd,
			  inet_ntoa(addr.sin_addr),
			  htons(addr.sin_port),
			  incoming->current - incoming->storage);
	    }
	    else
	    {
		tcp_fatal("incoming_read_error_callback(): fd %d was unexpectedly closed: n_read=%d\n",
			  incoming->fd,
			  incoming->current - incoming->storage);
	    }
	}
	tcp_enter();
	incoming_free(incoming);
    }
    else /* Some other read() error */
    {
	incoming_free(incoming);
	tcp_exit();
	if (_nx_fault_detected(NEXUS_FAULT_READ_FAILED) != 0)
	{
	    nexus_fatal("incoming_read_error_callback(): Internal error: Read failed with errno=%i\n", error);
	}
	tcp_enter();
	incoming_free(incoming);
    }
    tcp_exit();
} /* incoming_read_error_callback() */


/*
 * accept_internal_connection()
 */
static void accept_internal_connection(void *arg, int fd)
{
    if (tcp_aborting)
    {
	nexus_fd_write_one_nonblocking(fd, close_flag);
	nexus_fd_close(fd);
    }
    tcp_enter();
    incoming_construct(fd);
    if (tcp_done)
    {
	tcp_incoming_t *incoming = (tcp_incoming_t *) fd_table[fd].value;
	int rc;
	rc = nexus_fd_register_for_write(incoming->fd,
					 &close_flag,
					 1,
					 shutdown_write_callback,
					 shutdown_write_error_callback,
					 (void *) &(fd_table[fd]));
	/* TODO: Check the return code */
    }
    tcp_exit();
} /* accept_internal_connection() */


/*
 * outgoing_table_init()
 *
 * Initialize the outgoing table.
 */
static void outgoing_table_init(void)
{
    int i;

    for (i = 0; i < OUTGOING_TABLE_SIZE; i++)
    {
	outgoing_table[i].outgoing = (tcp_outgoing_t *) NULL;
	outgoing_table[i].next = (outgoing_table_entry_t *) NULL;
    }
} /* outgoing_table_init() */


/*
 * outgoing_table_hash()
 *
 * Hash the hostname and port for the outgoing table.
 */
static int outgoing_table_hash(char *host, u_short port)
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
    return (hval % OUTGOING_TABLE_SIZE);
} /* outgoing_table_hash() */


/*
 * outgoing_table_insert()
 *
 * Insert the given outgoing into the table, hashing on its internal
 * hostname and port number.
 *
 * We assume that the entry is not present in the table.
 */
static void outgoing_table_insert(tcp_outgoing_t *outgoing)
{
    int bucket;
    outgoing_table_entry_t *new_ent;

    bucket = outgoing_table_hash(outgoing->host, outgoing->port);
    if (outgoing_table[bucket].outgoing == (tcp_outgoing_t *) NULL)
    {
	/* Drop it into the preallocated table entry */
	outgoing_table[bucket].outgoing = outgoing;
    }
    else
    {
	/*
	 * Need to allocate a new outgoing_table_entry_t and add it
	 * to the bucket
	 */
	NexusMalloc(outgoing_table_insert(), new_ent, outgoing_table_entry_t *,
		    sizeof(struct _outgoing_table_entry_t));

	new_ent->outgoing = outgoing;
	new_ent->next = outgoing_table[bucket].next;

	outgoing_table[bucket].next = new_ent;
    }
    nexus_debug_printf(2, ("outgoing_table_inserted(): Inserted outgoing=%x for %s/%hu bucket=%d\n",
			   outgoing, outgoing->host, outgoing->port, bucket));
} /* outgoing_table_insert() */


/*
 * outgoing_table_remove()
 *
 * Remove the given outgoing from the table.
 *
 * We assume that the entry is present in the table.
 */
static void outgoing_table_remove(tcp_outgoing_t *outgoing)
{
    int bucket;
    outgoing_table_entry_t *ent, *remove_ent;

    bucket = outgoing_table_hash(outgoing->host, outgoing->port);

    if (outgoing_table[bucket].outgoing == outgoing)
    {
	if (outgoing_table[bucket].next)
	{
	    outgoing_table[bucket].outgoing
		= outgoing_table[bucket].next->outgoing;
	    outgoing_table[bucket].next
		= outgoing_table[bucket].next->next;
	}
	else
	{
	    outgoing_table[bucket].outgoing = (tcp_outgoing_t *) NULL;
	    outgoing_table[bucket].next = (outgoing_table_entry_t *) NULL;
	}
    }
    else
    {
	for (ent = &(outgoing_table[bucket]);
	     ent->next->outgoing != outgoing;
	     ent = ent->next)
	    ;
	remove_ent = ent->next;
	ent->next = remove_ent->next;
	NexusFree(remove_ent);
    }
    nexus_debug_printf(2, ("outgoing_table_remove(): Removed outgoing=%x for %s/%hu from bucket=%d\n",
			   outgoing, outgoing->host, outgoing->port, bucket));
} /* outgoing_table_remove() */


/*
 * outgoing_table_lookup()
 *
 * Look up and return the tcp_outgoing_t for the given hostname
 * and port. Return NULL if none exists.
 */
static tcp_outgoing_t *outgoing_table_lookup(char *host, u_short port)
{
    outgoing_table_entry_t *ent;
    int bucket;

    bucket = outgoing_table_hash(host, port);

    for (ent = &(outgoing_table[bucket]);
	 ent != (outgoing_table_entry_t *) NULL;
	 ent = ent->next)
    {
	if (   (ent->outgoing != (tcp_outgoing_t *) NULL)
	    && (ent->outgoing->port == port)
	    && (strcmp(ent->outgoing->host, host) == 0) )
	{
	    nexus_debug_printf(2, ("outgoing_table_lookup(): Found entry %x outgoing=%x for %s/%hu bucket=%d\n",
				   ent, ent->outgoing, host, port, bucket));
	    return (ent->outgoing);
	}
    }
    
    nexus_debug_printf(2, ("outgoing_table_lookup(): Didn't find entry for %s/%hu bucket=%d\n",
			   host, port, bucket));

    return ((tcp_outgoing_t *) NULL);
} /* outgoing_table_lookup() */
