/*
 * tcp_forwarder.c
 *
 * A standalone program to forward nexus messages between MPL<->TCP.
 * This must be used in conjuction with the pr_tcp_forward.c protocol
 * module.
 * 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/tcp_forwarder.c,v 1.4 1996/10/07 04:40:20 tuecke Exp $"
 */

#include <nexus_config.h>
#include <nexus_configlite.h>

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_LIBC_H
#include <libc.h>
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_ACCESS_H
#include <sys/access.h>
#endif

#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif

#include <sys/types.h>
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

#include "mpproto.h"

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef PORTS0_USE_BZERO
#define ZeroOutMemory(Where,Size)	bzero(Where,Size)
#else
#define ZeroOutMemory(Where,Size)	memset(Where,0,Size)
#endif

#define NexusMalloc(Func, Var, Type, Size) \
{ \
    if ((Size) > 0) \
    { \
	if (((Var) = (Type) malloc (Size)) == (Type) NULL) \
	{ \
	    fatal_error(("%s: malloc of size %d failed for %s %s in file %s line %d\n", \
                        #Func, (Size), #Type, #Var, __FILE__, __LINE__)); \
	} \
    } \
    else \
    { \
	(Var) = (Type) NULL; \
    } \
}

#define NexusFree(Ptr) \
{ \
    if ((Ptr) != NULL) \
    { \
	free(Ptr); \
    } \
}

#define NexusAssert(assertion) \
    if (!(assertion)) \
    { \
        fatal_error(("Assertion " #assertion " failed in file %s at line %d\n",\
		    __FILE__, __LINE__)); \
    }
#define fatal_error(Print_arg) { printf Print_arg ; exit(1); }

static int debug_level = 0;
#define nexus_debug_printf(level, message) \
do { \
    if (level <= debug_level) \
    { \
	printf message; \
    } \
} while (0)

/*
 * PackInt1(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt1(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt1(nexus_byte_t *Array, int Index, int Integer)
 */
#define UnpackInt1(Array, Index, Integer) \
{ \
    (Integer) = (int) (Array)[(Index)]; \
    (Index) += 1; \
}


/*
 * PackInt2(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt2(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF00) >> 8); \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt2(nexus_byte_t *Array, int Index, int Integer)
 */
#define UnpackInt2(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)]) << 8) \
		 |   ((int) (Array)[(Index)+1]) ); \
    (Index) += 2; \
}


/*
 * PackInt4(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt4(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF000000) >> 24); \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF0000) >> 16); \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF00) >> 8); \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt4(nexus_byte_t *Array, int Index, int Integer)
 */
#if SIZEOF_INT == 4
#define UnpackInt4(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)])   << 24) \
		 | ( ((int) (Array)[(Index)+1]) << 16) \
		 | ( ((int) (Array)[(Index)+2]) << 8) \
		 |   ((int) (Array)[(Index)+3]) ); \
    (Index) += 4; \
}
#else
#define UnpackInt4(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)])   << 24) \
		 | ( ((int) (Array)[(Index)+1]) << 16) \
		 | ( ((int) (Array)[(Index)+2]) << 8) \
		 |   ((int) (Array)[(Index)+3]) ); \
    if ((Integer) & 0x80000000) /* Sign extend */ \
    { \
	(Integer) |= 0xFFFFFFFF00000000; \
    } \
    (Index) += 4; \
}
#endif

extern char *sys_errlist[];
#define _nx_md_system_error_string(Errno) sys_errlist[Errno]

#define	NEXUS_TRUE		1
#define	NEXUS_FALSE		0

typedef int		nexus_bool_t;
typedef unsigned char	nexus_byte_t;

#define MAX_WRITE_PACKET 0


/*
 * Generic linked list structure and macros
 */
typedef struct _linked_list_t
{
    struct _linked_list_t *	next;
    void *			storage;
} linked_list_t;

static linked_list_t *linked_list_free_list = (linked_list_t *) NULL;

#define GetLink(Func, L) \
{ \
    if (linked_list_free_list) \
    { \
	(L) = linked_list_free_list; \
	linked_list_free_list = linked_list_free_list->next; \
    } \
    else \
    { \
	NexusMalloc(Func, \
		    L, \
		    linked_list_t *, \
		    sizeof(linked_list_t)); \
    } \
}

#define FreeLink(L) \
{ \
    (L)->next = linked_list_free_list; \
    linked_list_free_list = (L); \
}

#define Enqueue(Func, Head, Tail, Storage) \
{ \
    linked_list_t *__l; \
    GetLink(Func, __l); \
    __l->storage = (Storage); \
    if (Head) \
    { \
	(__l)->next = NULL; \
	(Tail)->next = (__l); \
	(Tail) = (__l); \
    } \
    else \
    { \
	(Head) = (Tail) = (__l); \
    } \
}

#define Dequeue(Head, Tail, Storage) \
{ \
    linked_list_t *__l; \
    if (Head) \
    { \
	__l = (Head); \
	(Head) = (Head)->next; \
        (Storage) = __l->storage; \
	FreeLink(__l); \
    } \
    else \
    { \
	(Storage) = NULL; \
    } \
}

#define QueueNotEmpty(Head, Tail) ((Head) != NULL)

static char hostname[MAXHOSTNAMELEN];
static int hostname_length;

static int my_node;
static int n_nodes;

#define TCP_OUTGOING 1
#define TCP_LISTENER 2
#define TCP_INCOMING 3

typedef struct _tcp_outgoing_t
{
    int		type;
    char *	host;
    u_short	port;
    int		fd;
} tcp_outgoing_t;

typedef struct _tcp_listener_t
{
    int		type;
    int		node;
    u_short	port;
    int		fd;
} tcp_listener_t;
static tcp_listener_t *tcp_listener_table;

#define TCP_MSG_HEADER_SIZE	17
typedef struct _tcp_incoming_t
{
    int		type;
    u_short	port;
    int		fd;
    int		node;

    int   recv_state;		/* receiving header or body */
    char *recv_buf;		/* Buffer we're receiving into */
    char *recv_buf_current;	/* Place in buffer that we're receving into */
    int   recv_n_left;		/* # bytes left to receive */
    int   recv_has_profiling;
    char  header_buf[TCP_MSG_HEADER_SIZE]; /* space to recv message header */
} tcp_incoming_t;

typedef struct _tcp_data_t
{
    char *buffer;
    int   node;
} tcp_data_t;

#define INCOMING_RECV_CLOSED           -1
#define INCOMING_RECV_HEADER		0
#define INCOMING_RECV_BODY		1

#define OPEN_FLAG               0
#define CLOSE_NORMAL_FLAG       1
#define CLOSE_SHUTDOWN_FLAG     3

#define FLAG_MASK       0x0F
#define XDR_MASK        0x10
#define PROFILE_MASK    0x20

#define ResetIncoming(Incoming) \
{ \
    (Incoming)->recv_state = INCOMING_RECV_HEADER; \
    (Incoming)->recv_buf = (char *) NULL; \
    (Incoming)->recv_buf_current = &((Incoming)->header_buf[0]); \
    (Incoming)->recv_n_left = TCP_MSG_HEADER_SIZE; \
}


/*
 * Stuff for keeping track of file descriptors.
 *   fd_tablesize:	Maximum number of fds that system supports
 *   fd_to_outgoing:	Array of pointers to tcp_outgoing_t's or
 *				tcp_incoming_t's.  This is used
 *				to map from an fd to the appropriate
 *				outgoing or incoming data structure.
 *   current_fd_set:	The current fd_set that should be used for select()
 */
static int fd_tablesize;
static void **fd_to_outgoing;
static fd_set current_fd_set;

#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_HPUX)
#define NUM_FDS sysconf(_SC_OPEN_MAX)
#else
/* This works for BSD machines */
#define NUM_FDS getdtablesize()
#endif


/*
 * tcp_outgoing_t table stuff.
 *
 * The tcp_outgoing_t table is hashed on host/port. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple sockets to the
 * same context.
 */
typedef struct _tcp_outgoing_entry_t
{
    tcp_outgoing_t *outgoing;
    struct _tcp_outgoing_entry_t *next;
} tcp_outgoing_entry_t;

#define TCP_OUTGOING_TABLE_SIZE 149

struct _tcp_outgoing_entry_t	tcp_outgoing[TCP_OUTGOING_TABLE_SIZE];

static void			tcp_outgoing_table_init(void);
static int			tcp_outgoing_hash(char *host,
						  u_short port);
static tcp_outgoing_t *         tcp_outgoing_add(char *hostname,
						 u_short port);
static tcp_outgoing_t *		tcp_outgoing_lookup(char *host,
						    u_short port);

static nexus_bool_t all_done;

/* MPL stuff */
static int mpl_pending_big_messages = 0;

/* function prototypes */
static void init_mpl(void);
static void check_for_mpl_messages(nexus_bool_t forward_immediately);
void forward_all_mpl_messages(void);
void forward_mpl_message(char *buf);
static void check_for_tcp_messages(nexus_bool_t forward_immediately);
static void read_incoming(tcp_incoming_t *incoming,
			  int forward_immediately);
void forward_all_tcp_messages(void);
void forward_tcp_message(int node, char *buf);
static void tcp_send(char *hostname,
		     u_short port,
		     char *buffer,
		     int start,
		     int size);
static void tcp_outgoing_close(tcp_outgoing_t *outgoing);
static void check_outgoing_for_close(tcp_outgoing_t *outgoing);
static void accept_new_connection(tcp_listener_t *listener);
static tcp_incoming_t *construct_incoming(int fd, int node);
static void close_incoming(tcp_incoming_t *incoming);
static void config_socket(int s);
static void set_nonblocking(int fd);
static void add_fd(void *outgoing, int fd);
static void remove_fd(void *outgoing, int fd);
char *_nx_copy_string(char *s);

/*
 * main()
 */
int main(int argc, char *argv[])
{
    char forwarder_info[MAXHOSTNAMELEN+4];
    int i;
    tcp_listener_t *listener;

    /*
     * Initialize the MPL side of the world
     */
    init_mpl();

    /*
     * Initialize the TCP side of the world
     */
    if (gethostname(hostname, MAXHOSTNAMELEN) < 0)
    {
	fatal_error(("main(): gethostname() failed\n"));
    }
    hostname_length = strlen(hostname);
    tcp_outgoing_table_init();
    
    /*
     * Set up file descriptor tables
     */
    fd_tablesize = NUM_FDS;
    NexusMalloc(main(),
		fd_to_outgoing,
		void **,
		(sizeof(void *) * fd_tablesize));
    for (i = 0; i < fd_tablesize; i++)
	fd_to_outgoing[i] = (void *) NULL;
    FD_ZERO(&current_fd_set);

    /*
     * Setup one listener for each node
     */
    NexusMalloc(main(),
		tcp_listener_table,
		tcp_listener_t *,
		(sizeof(tcp_listener_t) * n_nodes));
    for (i = 0; i < n_nodes; i++)
    {
	/* Setup a listener for node i */
	listener = &(tcp_listener_table[i]);
	listener->type = TCP_LISTENER;
	listener->node = i;
	listener->fd = setup_listener(&(listener->port));
	add_fd(listener, listener->fd);

	/* Send this host and port to the node this is proxying for */
	memcpy(forwarder_info, &(listener->port), 2);
	strcpy(forwarder_info+2, hostname);
	mpc_bsend(forwarder_info,
		  hostname_length+3,
		  listener->node,
		  0);
    }

    /*
     * Sit in a loop which alternately checks mpl and tcp for messages
     */
    all_done = NEXUS_FALSE;
    while (!all_done)
    {
	check_for_mpl_messages(NEXUS_TRUE);
	check_for_tcp_messages(NEXUS_TRUE);
    }
    
} /* main() */


/**********************************************************************
 * MPL code
 **********************************************************************/

#define MPL_MSG_TYPE 42
#define MPL_DEFAULT_RECEIVE_SIZE 1048576
#define MPL_DEFAULT_STORAGE_SIZE 32736

static int		mpl_msgid;
static char *		mpl_buffer = (char *) NULL;
static int		mpl_buffer_size;
static int		mpl_post_receive_source;
static int		mpl_post_receive_type;

static linked_list_t *	mpl_queue_head = (linked_list_t *) NULL;
static linked_list_t *	mpl_queue_tail;

static linked_list_t *	tcp_queue_head = (linked_list_t *) NULL;
static linked_list_t *	tcp_queue_tail;

/*
 * init_mpl()
 */
static void init_mpl(void)
{
    /* Initialize MPL */
    mpc_environ(&n_nodes, &my_node);
    n_nodes--;
    if (n_nodes <= 0)
    {
	fatal_error(("Must run with >1 node\n"));
    }

    /* Post the first buffer */
    mpl_buffer_size = MPL_DEFAULT_RECEIVE_SIZE;
    NexusMalloc(init_mpl(),
		mpl_buffer,
		char *,
		mpl_buffer_size);
    mpl_post_receive_source = DONTCARE;
    mpl_post_receive_type = MPL_MSG_TYPE;
    if (mpc_recv(mpl_buffer,
		 mpl_buffer_size,
		 &mpl_post_receive_source,
		 &mpl_post_receive_type,
		 &mpl_msgid) != 0)
    {
	fatal_error(("init_mpl(): mpc_recv() failed, mperrno=%d\n", mperrno));
    }
} /* init_mpl() */


/*
 * check_for_mpl_messages()
 *
 * Check to see if there are any mpl messages to receive.
 *
 * If forward_immediately==NEXUS_TRUE, then forward them on via tcp.
 * Else, enqueue them to be forwarded later.
 */
static void check_for_mpl_messages(nexus_bool_t forward_immediately)
{
    nexus_bool_t done = NEXUS_FALSE;
    int rc;
    int size;
    
    if (forward_immediately)
    {
	forward_all_mpl_messages();
    }

    while(!done)
    {
	if((rc = mpc_status(mpl_msgid)) >= 0)
	{
	    /* Got a message */
	    size = *((int *) mpl_buffer);
	    if(size == -1)
	    {
		/*
		 * Got a big message header.
		 * So make sure our buffer is big enough to hold
		 * the subsequent big message, and post another receive.
		 */
		size = *(((int *) mpl_buffer) + 1);
		mpl_pending_big_messages++;
		if (size > mpl_buffer_size)
		{
		    mpl_buffer_size = size;
		    NexusFree(mpl_buffer);
		    NexusMalloc(check_for_mpl_messages(),
				mpl_buffer,
				char *,
				mpl_buffer_size);
		}
		
		mpl_post_receive_source = DONTCARE;
		mpl_post_receive_type = MPL_MSG_TYPE;
		if (mpc_recv(mpl_buffer,
			     mpl_buffer_size,
			     &mpl_post_receive_source,
			     &mpl_post_receive_type,
			     &mpl_msgid) != 0)
		{
		    fatal_error(("check_for_mpl_messages(): mpc_recv() failed, mperrno=%d\n", mperrno));
		}
	    }
	    else if (size == -2)
	    {
		/*
		 * Got the sentinel for shutdown
		 */
		exit(0);
	    }
	    else
	    {
		/*
		 * Got a message that needs forwarding
		 */

		/* Forward or enqueue it */
		if (forward_immediately)
		{
		    forward_mpl_message(mpl_buffer);
		}
		else
		{
		    Enqueue(check_for_mpl_messages(),
			    mpl_queue_head, mpl_queue_tail,
			    (void *)mpl_buffer);
		}

		/* Post the next receive */
		NexusMalloc(check_for_mpl_messages(),
			    mpl_buffer,
			    char *,
			    mpl_buffer_size);
		mpl_post_receive_source = DONTCARE;
		mpl_post_receive_type = MPL_MSG_TYPE;
		if (mpc_recv(mpl_buffer,
			     mpl_buffer_size,
			     &mpl_post_receive_source,
			     &mpl_post_receive_type,
			     &mpl_msgid) != 0)
		{
		    fatal_error(("check_for_mpl_messages(): mpc_recv() failed, mperrno=%d\n", mperrno));
		}
	    }
	}
	else if(rc == -1)
	{
	    /* No messages available */
	    done = NEXUS_TRUE;
	}
	else
	{
	    fatal_error(("check_for_mpl_messages(): mpc_status() failed, mperrno=%d\n", mperrno));
	}
    }

} /* check_for_mpl_messages() */


/*
 * forward_all_mpl_messages()
 */
void forward_all_mpl_messages(void)
{
    char *buf;
    while (QueueNotEmpty(mpl_queue_head, mpl_queue_tail))
    {
	Dequeue(mpl_queue_head, mpl_queue_tail, buf);
	forward_mpl_message(buf);
    }
} /* forward_all_mpl_messages() */


/*
 * forward_mpl_message()
 */
void forward_mpl_message(char *buf)
{
    int start;
    int size;
    u_short port;
    int hostname_length;
    char *hostname;

    /*
     * Get the size, hostname, and port from the message headers.
     * Forward the rest of the message on via tcp.
     */
    memcpy(&size, buf, 4);
    memcpy(&hostname_length, buf+4, 4);
    memcpy(&port, buf+8, 2);
    hostname = buf+10;

    start = 10 + hostname_length;
    tcp_send(hostname, port, buf, start, size);

    NexusFree(buf);
} /* forward_mpl_message() */


/*
 * mpl_send()
 */
static void mpl_send(int node, char *buf, int size)
{
    int err;
    int message_id;

    /*
     * Send big message notification if necessary
     */
    if (size > MPL_DEFAULT_STORAGE_SIZE)
    {
	int notify[3];
	notify[0] = -1;
	notify[1] = size;
	notify[2] = my_node;
	err = mpc_bsend((char *) notify,
			sizeof(notify),
			node,
			MPL_MSG_TYPE);
	if (err != 0)
	{
	    fatal_error(("mpl_send(): first mpc_send() error, mperrno=%d\n", mperrno));
	}
	/* TODO: Wait for ack here */
    }

    /*
     * send message
     */
    err = mpc_send(buf,
		   size,
		   node,
		   MPL_MSG_TYPE,
		   &message_id);
    if (err != 0)
    {
        fatal_error(("mpl_send(): second mpc_send() error, errno=%d\n", mperrno));
    }

    /*
     * Wait for the send to complete.
     * In the mean time, try receiving any messages.
     */
    while (mpc_status(message_id) < 0)
    {
	check_for_mpl_messages(NEXUS_FALSE);
    }
    
} /* mpl_send() */


/**********************************************************************
 * TCP code
 **********************************************************************/

/*
 * check_for_tcp_messages()
 *
 * If forward_immediately==NEXUS_TRUE, then forward them on via mpl.
 * Else, enqueue them to be forwarded later.
 */
static void check_for_tcp_messages(nexus_bool_t forward_immediately)
{
    fd_set select_fd_set;
    int select_n_ready, n_checked;
    struct timeval timeout;
    int fd;
    int select_error;

    if (forward_immediately)
    {
	forward_all_tcp_messages();
    }

    select_n_ready = 1;
    while (select_n_ready > 0)
    {
	while (1)
	{
	    timeout.tv_sec = timeout.tv_usec = 0L;
	    select_fd_set = current_fd_set;
	    select_n_ready = select(fd_tablesize,
				    NEXUS_FD_SET_CAST &select_fd_set,
				    NEXUS_FD_SET_CAST NULL,
				    NEXUS_FD_SET_CAST NULL,
				    &timeout);
	    select_error = errno;
	    if (select_n_ready >= 0)
	    {
		break;
	    }
	    else if (select_n_ready < 0 && select_error != EINTR)
	    {
		fatal_error(("check_for_tcp_messages(): select failed: errno=%d: %s\n",
			     select_error,
			     _nx_md_system_error_string(select_error)));
	    }
	}
	
	for (n_checked = 0, fd = 0;
	     n_checked < select_n_ready;
	     fd++)
	{
	    if (FD_ISSET(fd, &select_fd_set))
	    {
		n_checked++;

                if (((tcp_outgoing_t *)(fd_to_outgoing[fd]))
		    == (tcp_outgoing_t *) NULL)
		{
		    fatal_error(("select_and_read(): Data is ready on fd %d, but there is no proto or incoming for it\n", fd));
		}
		
		if (((tcp_outgoing_t *)(fd_to_outgoing[fd]))->type
		    == TCP_LISTENER)
		{
		    /* Somebody wants to connect to me */
		    tcp_listener_t *listener
			= (tcp_listener_t *) fd_to_outgoing[fd];
		    accept_new_connection(listener);
		    forward_all_tcp_messages();
		}
		else if (((tcp_outgoing_t *)(fd_to_outgoing[fd]))->type
			 == TCP_INCOMING)
		{
		    read_incoming((tcp_incoming_t *) (fd_to_outgoing[fd]),
				  forward_immediately);
		}
		else /* type == TCP_OUTGOING */
		{
		    check_outgoing_for_close((tcp_outgoing_t *)
					     (fd_to_outgoing[fd]));
		}
	    }
	}
    }
    
} /* check_for_tcp_messages() */


/*
 * read_incoming()
 *
 * Read as much of a complete message as we can for this incoming connection.
 * The various stages of reading a complete message are:
 *   1) Non-blocking read of message header.
 *   2) Allocate a message buffer
 *   3) Non-blocking read of message body.
 *   4) Enqueue or forward complete message.
 *   5) Reset incoming for reading a new message.
 */
static void read_incoming(tcp_incoming_t *incoming, int forward_immediately)
{
    int n_read;
    int read_error;
    int flag;
    int tmp_int;
    nexus_byte_t *a;
    int i;
    int header_mod;
    char *buf;
    
    if (incoming->recv_state == INCOMING_RECV_CLOSED)
    {
	return;
    }

    do
    {
	n_read = read(incoming->fd,
		      incoming->recv_buf_current,
		      incoming->recv_n_left);
	read_error = errno;

	/*
	 * n_read: is > 0 if it successfully read some bytes
	 *         is < 0 on error -- need to check errno
	 *         is 0 on EOF
	 */

	if (n_read > 0)
	{
	    incoming->recv_n_left -= n_read;

	    /* Check for a closed stream */
	    if (   (n_read == 1)
		&& (incoming->recv_state == INCOMING_RECV_HEADER)
		&& (incoming->recv_n_left == TCP_MSG_HEADER_SIZE - 1)
		&& ((incoming->header_buf[0] & FLAG_MASK) != OPEN_FLAG) )
	    {
		flag = incoming->header_buf[0] & FLAG_MASK;
		incoming->recv_n_left += n_read;
		n_read = 0;	/* exit loop */
		if (   (flag == CLOSE_NORMAL_FLAG)
		    || (flag == CLOSE_SHUTDOWN_FLAG))
		{
		    close_incoming(incoming);
		}
		else
		{
		    fatal_error(("read_incoming(): Internal error: Got CLOSE_ABNORMAL_FLAG or an illegal first byte of message header: %d\n", (int) incoming->header_buf[0]));
		}
	    }
		    
	    else if (incoming->recv_n_left == 0)
	    {
		/* We have completed reading something... */
		if (incoming->recv_state == INCOMING_RECV_HEADER)
		{
		    /* We have completed reading the header. */
		    int tmp_int;
		    int body_size;
		    int liba_size;
		    int handler_id;
		    int handler_name_length;
		    int total_size;

		    /* Extract the message header information */
		    a = (nexus_byte_t *) &(incoming->header_buf[0]);
		    i = 0;
		    UnpackInt1(a, i, tmp_int);
		    UnpackInt4(a, i, body_size);
		    UnpackInt4(a, i, liba_size);
		    UnpackInt4(a, i, handler_id);
		    UnpackInt4(a, i, handler_name_length);
		    nexus_debug_printf(3, ("read_incoming(): got body_size=%d liba_size=%d handler_id=%d handler_name_length=%d\n",
					   body_size,
					   liba_size,
					   handler_id,
					   handler_name_length) );


		    /* Get a buffer and initialize it as an MPL message */
		    total_size = (16
				  + liba_size
				  + handler_name_length
				  + body_size);
		    NexusMalloc(read_incoming(),
				buf,
				char *,
				total_size);
		    memcpy(buf,      &total_size, 4);
		    memcpy(buf + 4,  &handler_id, 4);
		    memcpy(buf + 8,  &liba_size, 4);
		    memcpy(buf + 12, &handler_name_length, 4);

		    /* transition the incoming's recv_state */
		    incoming->recv_state = INCOMING_RECV_BODY;
		    incoming->recv_buf = buf;
		    incoming->recv_buf_current = buf + 16;
		    incoming->recv_n_left = total_size - 16;

		    nexus_debug_printf(3, ("read_incoming(): State now: INCOMING_RECV_BODY  expecting: %d bytes\n", incoming->recv_n_left));
		}
		else /* (incoming->recv_state == INCOMING_RECV_BODY) */
		{
		    char *buf = incoming->recv_buf;

		    ResetIncoming(incoming);
			
		    /* Enqueue or forward the message */
		    if (forward_immediately)
		    {
			forward_tcp_message(incoming->node, buf);
		    }
		    else
		    {
			tcp_data_t *tmp;
			NexusMalloc(read_incoming(),
				    tmp,
				    tcp_data_t *,
				    sizeof(tcp_data_t));
			tmp->buffer = buf;
			tmp->node = incoming->node;
			Enqueue(read_incoming(),
				tcp_queue_head, tcp_queue_tail,
				(void *)tmp);
		    }

		    /* 
		     * We have received an entire message and
		     * either forwarded or enqueued it.
		     * So set n_read=0 to break out of the read loop.
		     * If there are any more messages coming in on 
		     * this fd, we will catch them on the next
		     * select() go-around.
		     */
		    n_read = 0;
		}
	    }
	    else
	    {
		/*
		 * Else we didn't get a close message, and we didn't
		 * finish reading anything.  So we're in the middle of
		 * a message header or body, so just
		 * bump the recv pointer and keep trying...
		 */
		incoming->recv_buf_current += n_read;
	    }
	}
	else if (   (n_read == 0)
		 || (   (n_read < 0)
		     && (   (read_error == ECONNRESET)
			 || (read_error == EPIPE) ) ) )
	{
	    /*
	     * Got EOF or connection reset by peer on the read.
	     */
	    fatal_error(("read_incoming(): fd %d was unexpectedly closed: n_read=%d errno=%d: %s\n",
			 incoming->fd,
			 n_read,
			 read_error,
			 _nx_md_system_error_string(read_error)));
	}
	else /* n_read < 0 */
	{
	    /* Got an error on the read */
	    if (read_error == EINTR)
	    {
		/*
		 * The read() got interrupted (for example, by a signal),
		 * so retry the read.
		 */
		continue;
	    }
	    else if (read_error == EAGAIN || read_error == EWOULDBLOCK)
	    {
		/*
		 * The read() would have blocked without reading anything.
		 * So quit trying and return.
		 */
	    }
	    else
	    {
		fatal_error(("read_incoming(): Read failed (errno=%d): %s\n",
			     read_error,
			     _nx_md_system_error_string(read_error)));
	    }
	}

    } while (n_read > 0);
    
} /* read_incoming() */


/*
 * forward_all_tcp_messages()
 */
void forward_all_tcp_messages(void)
{
    char *buf;
    while (QueueNotEmpty(tcp_queue_head, tcp_queue_tail))
    {
	tcp_data_t *tmp;

	Dequeue(tcp_queue_head, tcp_queue_tail, tmp);
	forward_tcp_message(tmp->node, tmp->buffer);
	NexusFree(tmp);
    }
} /* forward_all_tcp_messages() */


/*
 * forward_tcp_message()
 */
void forward_tcp_message(int node, char *buf)
{
    int size;
    memcpy(&size, buf, 4);
    mpl_send(node, buf, size);
    NexusFree(buf);
} /* forward_tcp_message() */


/*
 * tcp_send()
 *
 * Send buffer[start..size] to hostname/port.
 */
static void tcp_send(char *hostname,
		     u_short port,
		     char *buffer,
		     int start,
		     int size)
{
    tcp_outgoing_t *outgoing;
    int fd;
    char *send_ptr;
    int n_left;
    int write_len;
    int n_sent;
    int write_error;

    /* Find or create a tcp_outgoing_t for this hostname/port */
    outgoing = tcp_outgoing_lookup(hostname, port);
    if (!outgoing)
    {
	outgoing = tcp_outgoing_add(hostname, port);
    }

    /* Make sure there is a file descriptor open for it */
    if (outgoing->fd < 0)
    {
	fd = tcp_outgoing_open(outgoing);
    }

    fd = outgoing->fd;
    send_ptr = buffer + start;
    n_left = size;
    
    while (n_left > 0)
    {
#if MAX_WRITE_PACKET > 0
	write_len = n_left > MAX_WRITE_PACKET ? MAX_WRITE_PACKET : n_left;
#else
	write_len = n_left;
#endif

	n_sent = write(fd, send_ptr, write_len);
	write_error = errno;
	
	/*
	 * n_sent: is > 0 on success -- number of bytes written
	 *         is < 0 on error -- need to check errno
	 *         is 0 (SysV) or (-1 && errno==EWOULDBLOCK) (BSD)
	 *             if the write would block without writing anything
	 */

	if (n_sent > 0)
	{
	    n_left -= n_sent;
	    send_ptr += n_sent;
	}
	else if ((n_sent == 0) || (n_sent < 0 && write_error == EWOULDBLOCK))
	{
	    /*
	     * We couldn't write (socket buffer is full).
	     *
	     * To avoid deadlock, we must try a read pass
	     * on all fds before retrying the write.
	     * (If two processes are sending big messages to
	     * each other simultaneously, they must each write some,
	     * then read some, then write some, then read some, etc.,
	     * so the the messages can slide past each other.)
	     */
	    check_for_tcp_messages(NEXUS_FALSE);
	}
	else /* n_sent < 0 */
	{
	    if (write_error == EINTR)
	    {
		/* Do nothing.  Just try again. */
	    }
	    else
	    {
		fatal_error(("tcp_send_remote_service_request(): Write failed: errno=%d: %s\n",
			     write_error,
			     _nx_md_system_error_string(write_error)));
	    }
	}
    }

} /* tcp_send() */


/*
 * check_outgoing_for_close()
 *
 * Do a non-blocking read from the passed outgoing.  The options are:
 *	1) If we get a close flag (single byte == CLOSE_NORMAL_FLAG),
 *		then close the proto.
 *	2) If we get EOF, then fatal out (the process on the other end
 *		probably died)
 *	3) If we get an error (including EWOULDBLOCK), then fatal out.
 *
 * Or in other words, this routine will either fatal out, or it will
 * return after closing the outgoing.
 */
static void check_outgoing_for_close(tcp_outgoing_t *outgoing)
{
    char buf[1];
    int n_read;
    int read_error;

 retry:
    n_read = read(outgoing->fd, buf, 1);
    read_error = errno;

    /*
     * n_read: is > 0 if it successfully read some bytes
     *         is < 0 on error -- need to check errno
     *         is 0 on EOF
     */
    
    if (n_read > 0)
    {
	if (buf[0] == CLOSE_NORMAL_FLAG)
	{
	    tcp_outgoing_close(outgoing);
	}
	else
	{
	    fatal_error(("check_outgoing_for_close(): read something other than CLOSE_NORMAL_FLAG on an outgoing\n"));
	}
    }
    else if (   (n_read == 0)
	     || (   (n_read < 0)
		 && (   (read_error == ECONNRESET)
		     || (read_error == EPIPE) ) ) )
    {
	fatal_error(("check_outgoing_for_close(): fd unexpectedly closed. Another process probably died: errno=%d: %s\n", read_error, _nx_md_system_error_string(read_error)));
    }
    else /* n_read < 0 */
    {
	if (read_error == EINTR)
	{
	    /*
	     * The read() got interrupted (for example, by a signal),
	     * so retry the read.
	     */
	    goto retry;
	}
	else
	{
	    fatal_error(("check_outgoing_for_close(): Read failed (errno=%d): %s\n",
			 read_error, _nx_md_system_error_string(read_error)));
	}
    }
} /* check_outgoing_for_close() */


/*
 * accept_new_connection()
 *
 * Do an accept() on the passed fd, 'listener', and then setup
 * a new tcp_incoming_t for this connection.
 *
 * Note_enqueue: This routine could cause messages to be enqueued.
 */
static void accept_new_connection(tcp_listener_t *listener)
{
    int new_fd;
    struct sockaddr_in from;
    int len;
    tcp_incoming_t *incoming;
    int accept_error;

    len = sizeof(from);
    if ((new_fd = accept(listener->fd, (struct sockaddr *) &from, &len)) < 0)
    {
	accept_error = errno;
	fatal_error(("accept_new_connection(): accept failed: errno=%d: %s\n",
		     accept_error, _nx_md_system_error_string(accept_error)));
    }

    if (debug_level >= 2)
    {
	struct hostent *hp;
	hp = gethostbyaddr((char *) &from.sin_addr,
			   sizeof(from.sin_addr),
			   from.sin_family);
	printf("Got incoming from %s/%hu on %d\n",
		     hp->h_name, ntohs(from.sin_port), new_fd);
    }

    config_socket(new_fd);
    
    incoming = construct_incoming(new_fd, listener->node);
    add_fd(incoming, new_fd);

} /* accept_new_connection() */


/*
 * construct_incoming()
 *
 * Construct a tcp_incoming_t for the given file descriptor, 'fd'.
 * This incoming should forward messages to 'node'.
 */
static tcp_incoming_t *construct_incoming(int fd, int node)
{
    tcp_incoming_t *incoming;

    NexusMalloc(construct_incoming(),
		incoming,
		tcp_incoming_t *,
		sizeof(tcp_incoming_t));

    incoming->type = TCP_INCOMING;
    incoming->fd = fd;
    incoming->node = node;

    ResetIncoming(incoming);

    return (incoming);
} /* construct_incoming() */


/*
 * close_incoming()
 *
 * Close an incoming connection.
 */
static void close_incoming(tcp_incoming_t *incoming)
{
    remove_fd(incoming, incoming->fd);
    close(incoming->fd);
    NexusFree(incoming);
} /* close_incoming() */


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

#ifndef NEXUS_NO_TCP_NODELAY
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &one,
		   sizeof(one)) < 0)
    {
	save_error = errno;
	fatal_error(("config_socket(): setsockopt TCP_NODELAY failed: errno=%d: %s\n",
		     save_error, _nx_md_system_error_string(save_error)));
    }
#endif
    
#ifdef SO_KEEPALIVE
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *) &one,
		   sizeof(one)) < 0)
    {
	save_error = errno;
	fatal_error(("config_socket(): setsockopt SO_KEEPALIVE failed: errno=%d: %s\n",
		     save_error, _nx_md_system_error_string(save_error)));
    }
#endif

    set_nonblocking(s);
} /* config_socket() */


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
	fatal_error(("set_nonblocking(): fcntl F_GETFL 1: errno=%d: %s\n",
		     save_error, _nx_md_system_error_string(save_error)));
    }

#if defined(TARGET_ARCH_AIX) || defined(TARGET_ARCH_HPUX)
    flags |= O_NONBLOCK;
#else
    flags |= FNDELAY;
#endif

    if (fcntl(fd, F_SETFL, flags) < 0)
    {
	save_error = errno;
	fatal_error(("set_nonblocking(): fcntl F_SETFL: errno=%d: %s\n",
		     save_error, _nx_md_system_error_string(save_error)));
    }

#ifdef F_SETFD
    if (fcntl(fd, F_SETFD, 1) < 0)
    {
	save_error = errno;
	fatal_error(("set_nonblocking(): fcntl F_SETFD: errno=%d: %s\n",
		     save_error, _nx_md_system_error_string(save_error)));
    }
#endif

} /* set_nonblocking() */


/*
 * do_connect()
 *
 * Return:
 *	The fd of the new connection.
 */
static int do_connect(char *host, u_short port)
{
    struct sockaddr_in his_addr, use_his_addr;
    int s;
    nexus_bool_t connect_succeeded;
    struct hostent *hp;
    char *interface;
    nexus_bool_t free_interface;
    int save_error;
    int rc;

    nexus_debug_printf(2, ("do_connect(): Connecting to %s/%hu\n",host,port));

    /*
     * First see if there is an alternate interface in the database,
     * and if so then try it.
     */
    interface = host;
    free_interface = NEXUS_FALSE;

    hp = gethostbyname(interface);
    
    ZeroOutMemory(&his_addr, sizeof(his_addr));
    his_addr.sin_port = htons(port);

    if (hp == (struct hostent *) NULL)
    {
	/*
	 * gethostbyname() on many machines does the right thing
	 * for IP addresses (i.e. "140.221.7.13").
	 * But on some machines (i.e. SunOS4.x) it doesn't.
	 * So hack it in this case.
	 */
	if (isdigit(interface[0]))
	{
	    unsigned long i1, i2, i3, i4;
	    rc = sscanf(interface, "%lu.%lu.%lu.%lu", &i1, &i2, &i3, &i4);
	    if (rc != 4)
	    {
		fatal_error(("do_connect(): gethostbyname failed for: %s\n", interface));
	    }
	    his_addr.sin_addr.s_addr = (  (i1 << 24)
					| (i2 << 16)
					| (i3 << 8)
					| i4);
	    his_addr.sin_family = AF_INET;
	}
	else
	{
	    fatal_error(("do_connect(): gethostbyname failed for: %s\n", interface));
	}
    }
    else
    {
	memcpy(&his_addr.sin_addr, hp->h_addr, hp->h_length);
	his_addr.sin_family = hp->h_addrtype;
    }
    

    connect_succeeded = NEXUS_FALSE;
    while (!connect_succeeded)
    {
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    save_error = errno;
	    fatal_error(("do_connect(): Failed to create socket: %s\n",
			 _nx_md_system_error_string(save_error)));
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
		nexus_debug_printf(2, ("do_connect(): Connect to %s/%hu got EINPROGRESS: %s\n",
				       interface, port,
				       _nx_md_system_error_string(save_error)));
            }
	    else if (save_error == EINTR)
	    {
#ifdef TARGET_ARCH_SUNOS41
		/*
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
		while (!select_loop_done)
		{
		    FD_ZERO(&my_fd_set);
		    FD_SET(s, &my_fd_set);
		    select_rc = select(s+1,
				       NEXUS_FD_SET_CAST NULL,
				       NEXUS_FD_SET_CAST &my_fd_set,
				       NEXUS_FD_SET_CAST NULL,
				       NULL);
		    if (select_rc == 1)
		    {
			select_loop_done = NEXUS_TRUE;
			connect_succeeded = NEXUS_TRUE;
		    }
		    else
		    {
			save_error = errno;
			if (save_error == EBADF)
			{
			    close(s);
			    select_loop_done = NEXUS_TRUE;
			}
			else if (save_error != EINTR)
			{
			    fatal_error(("do_connect(): Ack! connect() returned EINTR, but selecting on the socket failed (select returned %d, errno=%d).  Guess its time to try another hack around this connect() bug.\n", select_rc, save_error));
			}
		    }
		}
		
#else  /* TARGET_ARCH_SUNOS41 */
		
		/*
		 * Do nothing.  Just try again.
		 */
		nexus_debug_printf(2, ("do_connect(): Connect to %s/%hu, got EINTR.  Retrying...\n", interface, port));
		close(s);
		
#endif /* TARGET_ARCH_SUNOS41 */
	    }
	    else if (save_error == ETIMEDOUT)
	    {
		nexus_debug_printf(2, ("do_connect(): Connect to %s/%hu timed out.  Listener queue is probably full.  Retrying...\n", interface, port));
		close(s);
	    }
	    else
	    {
		fatal_error(("do_connect(): Connect to %s/%hu failed: %s\n", interface, port, _nx_md_system_error_string(save_error)))
	    }
	}
    }
    
    config_socket(s);

    if (free_interface)
    {
	NexusFree(interface);
    }
    
    return(s);
} /* do_connect() */


/*
 * add_fd()
 */
static void add_fd(void *outgoing, int fd)
{
    NexusAssert(fd_to_outgoing[fd] == (void *) NULL);
    NexusAssert(fd >= 0 && fd < fd_tablesize);
    fd_to_outgoing[fd] = outgoing;
    FD_SET(fd, &current_fd_set);
} /* add_fd() */


/*
 * remove_fd()
 */
static void remove_fd(void *outgoing, int fd)
{
    NexusAssert(fd_to_outgoing[fd] != (void *) NULL);
    NexusAssert(fd >= 0 && fd < fd_tablesize);
    fd_to_outgoing[fd] = (void *) NULL;
    FD_CLR(fd, &current_fd_set);
} /* remove_fd() */


/*
 * setup_listener()
 *
 * Return the file descriptor of this listener.  Also, return the
 * listener port in 'port'.
 */
static int setup_listener(u_short *port)
{
    struct sockaddr_in my_addr;
    int len;
    int listen_fd;
    int save_error;
    
    ZeroOutMemory(&my_addr, sizeof(my_addr));

    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(*port);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
	save_error = errno;
	fatal_error(("setup_listener(): socket() call failed: %s\n",
		    _nx_md_system_error_string(save_error)));
    }

#if defined(SO_REUSEADDR) && !defined(TARGET_ARCH_SUNOS41) \
    && !defined(TARGET_ARCH_NEXT040)
    /*
     * Set the port so it can be reused immediately if this
     * process dies.
     *
     * Under SunOS4.1 there is apparently a bug in this.
     * With this option set, a bind() will succeed on a port
     * even if that port is still in active use.  In fact, it will
     * succeed even if that port is still in active use by another
     * process.
     *
     * JGG:  Apparently, the NeXT has the same problem
     */
    if (*port != 0)
    {
	int one = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
		       sizeof(one)) < 0)
	{
	    save_error = errno;
	    printf("setup_listener(): setsockopt SO_REUSEADDR failed: %s\n",
		   _nx_md_system_error_string(save_error));
	}
    }
#endif /* SO_REUSEADDR */
    
    if (bind(listen_fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
    {
	save_error = errno;
	if (save_error == EADDRINUSE) /* Not sure if this is right */
	{
	    return(-1);
	}
	else
	{
	    fatal_error(("setup_listener(): bind() call failed: %s\n",
			 _nx_md_system_error_string(save_error)));
	}
    }

    if (listen(listen_fd, SOMAXCONN) < 0)
    {
	save_error = errno;
	fatal_error(("setup_listener(): listen() call failed: %s\n",
		     _nx_md_system_error_string(save_error)));
    }

    len = sizeof(my_addr);
    if (getsockname(listen_fd, (struct sockaddr *) &my_addr, &len) < 0)
    {
	save_error = errno;
	fatal_error(("setup_listener(): getsockname() call failed: %s\n",
		     _nx_md_system_error_string(save_error)));
    }

    *port = ntohs(my_addr.sin_port);

    return (listen_fd);
} /* setup_listener() */
    

/*
 * tcp_outgoing_table_init()
 *
 * Initialize the tcp outgoing table.
 */
static void tcp_outgoing_table_init(void)
{
    int i;

    for (i = 0; i < TCP_OUTGOING_TABLE_SIZE; i++)
    {
	tcp_outgoing[i].outgoing = (tcp_outgoing_t *) NULL;
	tcp_outgoing[i].next = (tcp_outgoing_entry_t *) NULL;
    }
} /* tcp_outgoing_table_init() */


/*
 * tcp_outgoing_hash()
 *
 * Hash the hostname and port for the outgoing table.
 */
static int tcp_outgoing_hash(char *host, u_short port)
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
    return (hval % TCP_OUTGOING_TABLE_SIZE);
} /* tcp_outgoing_hash() */


/*
 * tcp_outgoing_add()
 *
 * Insert the given outgoing into the table, hashing on its internal
 * hostname and port number.
 *
 * We assume that the entry is not present in the table.
 */
static tcp_outgoing_t *tcp_outgoing_add(char *hostname, u_short port)
{
    tcp_outgoing_t *outgoing;
    int bucket;
    tcp_outgoing_entry_t *new_ent;

    /* create the tcp_outgoing_t */
    NexusMalloc(tcp_outgoing_add(),
		outgoing,
		tcp_outgoing_t *,
		sizeof(tcp_outgoing_t));
    outgoing->type = TCP_OUTGOING;
    outgoing->host = _nx_copy_string(hostname);
    outgoing->port = port;
    outgoing->fd = -1;

    /* add it to the table */
    bucket = tcp_outgoing_hash(outgoing->host, outgoing->port);
    if (tcp_outgoing[bucket].outgoing == (tcp_outgoing_t *) NULL)
    {
	/* Drop it into the preallocated table entry */
	tcp_outgoing[bucket].outgoing = outgoing;
    }
    else
    {
	/*
	 * Need to allocate a new tcp_outgoing_entry_t and add it
	 * to the bucket
	 */
	NexusMalloc(tcp_outgoing_add(), new_ent, tcp_outgoing_entry_t *,
		    sizeof(struct _tcp_outgoing_entry_t));

	new_ent->outgoing = outgoing;
	new_ent->next = tcp_outgoing[bucket].next;

	tcp_outgoing[bucket].next = new_ent;
    }
} /* tcp_outgoing_add() */


/*
 * tcp_outgoing_lookup()
 *
 * Look up and return the tcp_outgoing_t for the given hostname
 * and port. Return NULL if none exists.
 */
static tcp_outgoing_t *tcp_outgoing_lookup(char *host, u_short port)
{
    tcp_outgoing_entry_t *ent;
    int bucket;

    bucket = tcp_outgoing_hash(host, port);

    for (ent = &(tcp_outgoing[bucket]);
	 ent != (tcp_outgoing_entry_t *) NULL;
	 ent = ent->next)
    {
	if (   (ent->outgoing != (tcp_outgoing_t *) NULL)
	    && (ent->outgoing->port == port)
	    && (strcmp(ent->outgoing->host, host) == 0) )
	{
	    nexus_debug_printf(2, ("tcp_outgoing_lookup(): Found entry %x outgoing=%x for %s/%hu bucket=%d\n",
				   ent, ent->outgoing, host, port, bucket));
	    return (ent->outgoing);
	}
    }
    
    nexus_debug_printf(2, ("tcp_outgoing_lookup(): Didn't find entry for %s/%hu bucket=%d\n",
			   host, port, bucket));

    return ((tcp_outgoing_t *) NULL);
} /* tcp_outgoing_lookup() */


/*
 * tcp_outgoing_open()
 */
static int tcp_outgoing_open(tcp_outgoing_t *outgoing)
{
    outgoing->fd = do_connect(outgoing->host, outgoing->port);
    add_fd(outgoing, outgoing->fd);
    return (outgoing->fd);
} /* tcp_outgoing_open() */


/*
 * tcp_outgoing_close()
 */
static void tcp_outgoing_close(tcp_outgoing_t *outgoing)
{
    close(outgoing->fd);
    remove_fd((void *) outgoing, outgoing->fd);
    outgoing->fd = -1;
} /* tcp_outgoing_close() */


/*
 * _nx_copy_string()
 *
 * Copy the string into malloced space and return it.
 *
 * Reminder: This function returns a pointer to malloced memory, so
 * don't forget to use NexusFree() on it...
 */
char *_nx_copy_string(char *s)
{
    char *rc;

    NexusMalloc(_nx_copy_string(), rc, char *, (strlen(s) + (size_t)(1)));
    strcpy(rc, s);
    return (rc);
} /* _nx_copy_string() */


