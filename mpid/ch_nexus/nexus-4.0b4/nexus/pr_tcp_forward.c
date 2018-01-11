/*
 * Nexus
 * Authors:     Steven Tuecke
 *              Argonne National Laboratory
 *
 * pr_tcp_forward.c	- TCP/IP forwarder protocol module
 *
 * This protocol module can be used instead of pr_tcp to forward
 * TCP packets via some other message protocol through a forwarder.
 * For example, on some MPP's it might be more efficient to not have
 * every node speak TCP directly, but instead setup a single node to
 * forward TCP packets over the native messaging protocol.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_tcp_forward.c,v 1.5 1996/10/07 04:40:14 tuecke Exp $";

#include "internal.h"

#ifdef NEXUS_SPOOF_COPY
nexus_bool_t _nx_spoof_copy = NEXUS_FALSE;
#endif

/*
 * Only one thread is allowed to be in the tcpf code (and thus
 * mucking with data structures) at a time.  And only one send
 * can be done at a time.
 */
static nexus_mutex_t		tcpf_mutex;
#define tcpf_enter() nexus_mutex_lock(&tcpf_mutex);
#define tcpf_exit()  nexus_mutex_unlock(&tcpf_mutex);

#define tcpf_fatal tcpf_exit(); nexus_fatal


/*
 * Other useful defines
 */
#define OPEN_FLAG		0
#define CLOSE_NORMAL_FLAG	1
#define CLOSE_ABNORMAL_FLAG	2
#define CLOSE_SHUTDOWN_FLAG	3

#define FLAG_MASK	0x0F
#define XDR_MASK	0x10
#define PROFILE_MASK	0x20


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
 * TCPF_MSG_HEADER_SIZE
 *
 * A message has the following format:
 *	message size			(4 byte native integer)
 *	host name length		(4 byte native integer)
 *	port				(2 byte native integer)
 *	host name string
 *		Including null terminator
 *
 *	flags				(1 byte integer)
 *		flags & FLAG_MASK	Message flag
 *				OPEN_FLAG
 *				|| CLOSE_NORMAL_FLAG
 *				|| CLOSE_ABNORMAL_FLAG
 *				|| CLOSE_SHUTDOWN_FLAG
 *		flags & XDR_MASK	Use XDR format (boolean)
 *		flags & PROFILE_MASK	Contains profile info (boolean)
 *	message body size		(4 byte, big endian integer)
 *	liba size			(4 byte, big endian integer)
 *	handler id			(4 byte, big endian integer)
 *	handler name string length	(4 byte, big endian integer)
 *		Includes null terminator
 *#ifdef BUILD_PROFILE
 *	node_id of sender		(4 byte, big endian integer)
 *	context_id of sender		(4 byte, big endian integer)
 *#else
 *	liba
 *	handler name string
 *		Including null terminator
 *	message body
 *
 * TCPF_MSG_HEADER_SIZE is the size of the header, in bytes,
 * not including the host name string and the handler name string.
 *
 * The message forwarder should be able to simply strip off the first
 * for fields, use them to open the appropriate socket, and forward
 * the rest of the message on using write().
 */
#define TCPF_MSG_HEADER_SIZE	27


/*
 * Some forward typedef declarations...
 */
typedef struct _tcpf_buffer_t	tcpf_buffer_t;
typedef struct _tcpf_proto_t	tcpf_proto_t;


/*
 * tcpf_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * tcp specific information to that structure.
 */
struct _tcpf_buffer_t
{
#ifdef BUILD_DEBUG
    int				magic;
#endif
    nexus_buffer_funcs_t *	funcs;
    tcpf_buffer_t *		next;
    char *			storage;
    int				malloc_pad;

    /* send stuff */
    nexus_global_pointer_t *	gp;
    int				handler_id;
    char *			handler_name;
    int				handler_name_length;
    int				header_size;
    int				size;
    int				n_elements;
    char *			base_pointer;
    char *			current_pointer;
};

/*
 * A tcpf_buffer_t free list, to avoid malloc calls on the
 * main body of a message buffer.
 */
static tcpf_buffer_t *buffer_free_list = (tcpf_buffer_t *) NULL;

#ifdef BUILD_LITE
#define lock_buffer_free_list()
#define unlock_buffer_free_list()
#else  /* BUILD_LITE */
static nexus_mutex_t	buffer_free_list_mutex;
#define lock_buffer_free_list()   nexus_mutex_lock(&buffer_free_list_mutex)
#define unlock_buffer_free_list() nexus_mutex_unlock(&buffer_free_list_mutex)
#endif /* BUILD_LITE */

#define SEND_BUFFER	1
#define RECV_BUFFER	2

#ifdef BUILD_DEBUG
#define MallocTcpfBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, tcpf_buffer_t *, \
		    sizeof(struct _tcpf_buffer_t)); \
	Buf->magic = NEXUS_BUFFER_MAGIC; \
    }
#else  /* BUILD_DEBUG */
#define MallocTcpfBuffer(Routine, Buf) \
    { \
	NexusMalloc(Routine, Buf, tcpf_buffer_t *, \
		    sizeof(struct _tcpf_buffer_t)); \
    }
#endif /* BUILD_DEBUG */

#define GetTcpfBuffer(Routine, Buf) \
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
	MallocTcpfBuffer(Routine, Buf); \
    }

#define FreeTcpfBuffer(Buf) \
{ \
    if ((Buf)->storage != (char *) NULL) \
	NexusFree((Buf)->storage); \
    lock_buffer_free_list(); \
    (Buf)->next = buffer_free_list; \
    buffer_free_list = (Buf); \
    unlock_buffer_free_list(); \
}

#define SetBufferPointers(Buf) \
{ \
    (Buf)->base_pointer \
	= ((Buf)->storage + (Buf)->header_size); \
}


/*
 * tcpf_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * tcp specific information to that structure.
 */
struct _tcpf_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_TCP */
    nexus_proto_funcs_t *funcs;
    char *		host;
    int			host_length;
    u_short		port;
    int			reference_count;
};


/*
 * My local TCP connection information.
 * These are filled in by st_mpl.c.
 */
u_short	_nx_tcpf_my_forwarder_port;
char	_nx_tcpf_my_forwarder_host[MAXHOSTNAMELEN];
int	_nx_tcpf_my_forwarder_node;


/*
 * Protocol table stuff.
 *
 * The protocol table is hashed on host/port. The table itself is an
 * array of header structures pointing to a linked list of buckets.
 *
 * This table is used to avoid creating multiple tcpf_proto_t
 * objects to the same context.  Multiple global pointers to the same
 * context share a tcpf_proto_t.
 */
typedef struct _proto_table_entry_t
{
    tcpf_proto_t *proto;
    struct _proto_table_entry_t *next;
} proto_table_entry_t;

#define PROTO_TABLE_SIZE 149

struct _proto_table_entry_t	proto_table[PROTO_TABLE_SIZE];

static void			proto_table_init(void);
static int			proto_table_hash(char *host, u_short port);
static void			proto_table_insert(tcpf_proto_t *proto);
static tcpf_proto_t *		proto_table_lookup(char *host, u_short port);

#ifdef BUILD_DEBUG
static char tmpbuf1[10240];
static char tmpbuf2[10240];
#endif


/*
 * Various forward declarations of procedures
 */
extern void             _nx_mp_send_bytes(int dest_node,
					  nexus_byte_t *buf,
					  int size);

static nexus_proto_type_t	tcpf_proto_type(void);
static void		tcpf_init(int *argc, char ***argv);
static void		tcpf_shutdown(nexus_bool_t shutdown_others);
static void		tcpf_abort(void);

static void		tcpf_init_remote_service_request(
					    nexus_buffer_t *buffer,
					    nexus_global_pointer_t *gp,
					    char *handler_name,
					    int handler_id);
static int		tcpf_send_remote_service_request(
						nexus_buffer_t *buffer);
static int		tcpf_send_urgent_remote_service_request(
						nexus_buffer_t *buffer);
static void             tcpf_increment_reference_count(nexus_proto_t *nproto);
static void		tcpf_decrement_reference_count(nexus_proto_t *nproto);
static void		tcpf_get_my_mi_proto(nexus_byte_t **array,
					    int *size);
static nexus_bool_t	tcpf_construct_from_mi_proto(nexus_proto_t **proto,
						    nexus_mi_proto_t *mi_proto,
						    nexus_byte_t *proto_array,
						    int size);


static tcpf_proto_t *	construct_proto(char *host, u_short port);

static void		tcpf_set_buffer_size(nexus_buffer_t *buffer,
					    int size, int n_elements);
static int		tcpf_check_buffer_size(nexus_buffer_t *buffer,
					      int slack, int increment);


static int	tcpf_sizeof_float	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_double	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_short	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_u_short	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_int		(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_u_int	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_long		(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_u_long	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_char		(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_u_char	(nexus_buffer_t *buffer, int count);
static int	tcpf_sizeof_byte		(nexus_buffer_t *buffer, int count);


static void	tcpf_put_float	(nexus_buffer_t *buffer,
				 float *data, int count);
static void	tcpf_put_double	(nexus_buffer_t *buffer,
				 double *data, int count);
static void	tcpf_put_short	(nexus_buffer_t *buffer,
				 short *data, int count);
static void	tcpf_put_u_short	(nexus_buffer_t *buffer,
				 unsigned short *data, int count);
static void	tcpf_put_int	(nexus_buffer_t *buffer,
				 int *data, int count);
static void	tcpf_put_u_int	(nexus_buffer_t *buffer,
				 unsigned int *data, int count);
static void	tcpf_put_long	(nexus_buffer_t *buffer,
				 long *data, int count);
static void	tcpf_put_u_long	(nexus_buffer_t *buffer,
				 unsigned long *data, int count);
static void	tcpf_put_char	(nexus_buffer_t *buffer,
				 char *data, int count);
static void	tcpf_put_u_char	(nexus_buffer_t *buffer,
				 unsigned char *data, int count);
static void	tcpf_put_byte	(nexus_buffer_t *buffer,
				 unsigned char *data, int count);

static nexus_proto_funcs_t tcpf_proto_funcs =
{
    tcpf_proto_type,
    tcpf_init,
    tcpf_shutdown,
    tcpf_abort,
    NULL /* tcpf_poll */,
    NULL /* tcpf_blocking_poll */,
    tcpf_init_remote_service_request,
    tcpf_increment_reference_count,
    tcpf_decrement_reference_count,
    tcpf_get_my_mi_proto,
    tcpf_construct_from_mi_proto,
    NULL /* tcpf_test_proto */,
};

static nexus_buffer_funcs_t tcpf_buffer_funcs =
{
    tcpf_set_buffer_size,
    tcpf_check_buffer_size,
    tcpf_send_remote_service_request,
    tcpf_send_urgent_remote_service_request,
    NULL /* tcpf_free_buffer */,
    NULL /* tcpf_stash_buffer */,
    NULL /* tcpf_free_stashed_buffer */,
    tcpf_sizeof_float,
    tcpf_sizeof_double,
    tcpf_sizeof_short,
    tcpf_sizeof_u_short,
    tcpf_sizeof_int,
    tcpf_sizeof_u_int,
    tcpf_sizeof_long,
    tcpf_sizeof_u_long,
    tcpf_sizeof_char,
    tcpf_sizeof_u_char,
    tcpf_sizeof_byte,
    tcpf_put_float,
    tcpf_put_double,
    tcpf_put_short,
    tcpf_put_u_short,
    tcpf_put_int,
    tcpf_put_u_int,
    tcpf_put_long,
    tcpf_put_u_long,
    tcpf_put_char,
    tcpf_put_u_char,
    tcpf_put_byte,
    NULL /* tcpf_get_float */,
    NULL /* tcpf_get_double */,
    NULL /* tcpf_get_short */,
    NULL /* tcpf_get_u_short */,
    NULL /* tcpf_get_int */,
    NULL /* tcpf_get_u_int */,
    NULL /* tcpf_get_long */,
    NULL /* tcpf_get_u_long */,
    NULL /* tcpf_get_char */,
    NULL /* tcpf_get_u_char */,
    NULL /* tcpf_get_byte */,
    NULL /* tcpf_get_stashed_float */,
    NULL /* tcpf_get_stashed_double */,
    NULL /* tcpf_get_stashed_short */,
    NULL /* tcpf_get_stashed_u_short */,
    NULL /* tcpf_get_stashed_int */,
    NULL /* tcpf_get_stashed_u_int */,
    NULL /* tcpf_get_stashed_long */,
    NULL /* tcpf_get_stashed_u_long */,
    NULL /* tcpf_get_stashed_char */,
    NULL /* tcpf_get_stashed_u_char */,
    NULL /* tcpf_get_stashed_byte */,
};


/*
 * _nx_pr_tcpf_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *_nx_pr_tcpf_info(void)
{
    return((void *) (&tcpf_proto_funcs));
} /* _nx_pr_tcpf_info() */


/*
 * tcpf_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t tcpf_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_TCP);
} /* tcpf_proto_type() */


/*
 * tcpf_init()
 *
 * Initialize the TCP protocol.
 *
 * _nx_tcpf_my_forwarder_{port,host} are filled in by st_mpl.c
 */
static void tcpf_init(int *argc, char ***argv)
{
    proto_table_init();

#ifndef BUILD_LITE
    nexus_mutex_init(&tcpf_mutex, (nexus_mutexattr_t *) NULL);
    nexus_mutex_init(&buffer_free_list_mutex, (nexus_mutexattr_t *) NULL);
#endif /* BUILD_LITE */
    
} /* tcpf_init() */


/*
 * tcpf_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 */
static void tcpf_shutdown(nexus_bool_t shutdown_others)
{
} /* tcpf_shutdown() */


/*
 * tcpf_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void tcpf_abort()
{
} /* tcpf_abort() */


/*
 * tcpf_init_remote_service_request()
 *
 * Initiate a remote service request, via the forwarder node,
 * to the node and context specified in the gp,
 * to the handler specified by handler_id and handler_name.
 *
 * Note: The things pointed to by 'gp' and 'handler_name' should
 * NOT change between this init_rsr call and the subsequent send_rsr.
 * Only the pointers are stashed away, and not the full data.
 *
 * Return: Fill in 'buffer' with a nexus_buffer_t.
 */
static void tcpf_init_remote_service_request(nexus_buffer_t *buffer,
					     nexus_global_pointer_t *gp,
					     char *handler_name,
					     int handler_id)
{
    tcpf_buffer_t *tcpf_buffer;
    tcpf_proto_t *proto = (tcpf_proto_t *) gp->mi_proto->proto;
#if ROUND_MALLOC > 1
    int header_mod;
#endif    

    NexusAssert2((gp->mi_proto->proto->type == NEXUS_PROTO_TYPE_TCP),
		 ("tcpf_init_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_TCP\n"));

    NEXUS_INTERROGATE(gp, _NX_GLOBAL_POINTER_T, "tcpf_init_remote_service_request");

    GetTcpfBuffer(tcpf_init_remote_service_request(), tcpf_buffer);

    nexus_debug_printf(2, ("tcpf_init_remote_service_request(): to: (%s/%d) %s-%d\n", ((tcpf_proto_t *)(gp->mi_proto->proto))->host, ((tcpf_proto_t *)(gp->mi_proto->proto))->port, handler_name, handler_id));

    tcpf_buffer->funcs = &tcpf_buffer_funcs;
    tcpf_buffer->next = (tcpf_buffer_t *) NULL;
    tcpf_buffer->storage = (char *) NULL;
    tcpf_buffer->gp = gp;
    tcpf_buffer->handler_id = handler_id;
    tcpf_buffer->handler_name = handler_name;
    tcpf_buffer->handler_name_length = strlen(handler_name)+1;
    tcpf_buffer->size = 0;
    tcpf_buffer->n_elements = -1;
    tcpf_buffer->base_pointer = (char *) NULL;
    tcpf_buffer->current_pointer = (char *) NULL;

    tcpf_buffer->header_size
	= (TCPF_MSG_HEADER_SIZE
#ifdef BUILD_PROFILE
	   + 8
#endif
	   + proto->host_length
	   + gp->liba_size
	   + tcpf_buffer->handler_name_length);

#if ROUND_MALLOC > 1
    header_mod = tcpf_buffer->header_size % ROUND_MALLOC;
    if (header_mod != 0)
    {
	tcpf_buffer->malloc_pad = (ROUND_MALLOC - header_mod);
    }
    else
#endif
    {
	tcpf_buffer->malloc_pad = 0;
    }
    tcpf_buffer->header_size += tcpf_buffer->malloc_pad;
		   
    NexusAssert2((tcpf_buffer->handler_name_length < NEXUS_MAX_HANDLER_NAME_LENGTH),
		 ("tcpf_init_remote_service_request(): Handler name exceeds maximum length (%d): %s\n", NEXUS_MAX_HANDLER_NAME_LENGTH, handler_name));
    
    *buffer = (nexus_buffer_t) tcpf_buffer;
} /* tcpf_init_remote_service_request() */


/*
 * tcpf_send_remote_service_request()
 *
 * Generate a remote service request message, via the forwarder node,
 * to the node and context saved in the 'nexus_buffer'.
 */
static int tcpf_send_remote_service_request(nexus_buffer_t *nexus_buffer)
{
    int return_code = 0;
    tcpf_buffer_t *buf;
    tcpf_proto_t *proto;
    int msg_size, total_size;
    char *send_ptr;
    nexus_byte_t *a;
    int i;
    int tmp_int;
#ifdef BUILD_PROFILE
    int node_id, context_id;
#endif    

    NexusBufferMagicCheck(tcpf_send_remote_service_request, nexus_buffer);

    buf = (tcpf_buffer_t *) *nexus_buffer;
    
    NEXUS_INTERROGATE((buf->gp), _NX_GLOBAL_POINTER_T, "tcpf_init_remote_service_request");

    proto = (tcpf_proto_t *) buf->gp->mi_proto->proto;
    NexusAssert2((proto->type == NEXUS_PROTO_TYPE_TCP),
		 ("tcpf_send_remote_service_request(): Internal error: proto_type is not NEXUS_PROTO_TYPE_TCP\n"));

    nexus_debug_printf(2, ("tcpf_send_remote_service_request(): invoked with buffer:%x\n", nexus_buffer));

    if (buf->storage == (char *) NULL)
    {
	/*
	 * This is a zero length message.
	 * Neither tcpf_set_buffer_size() nor tcpf_check_buffer_size()
	 * was called for this buffer.
	 * So create a buffer for the header information.
	 */
	msg_size = 0;
	NexusMalloc(tcpf_send_remote_service_request(),
		    buf->storage,
		    char *,
		    buf->header_size);
	SetBufferPointers(buf);
    }
    else
    {
	msg_size = buf->current_pointer - buf->base_pointer;
    }
	
    /*
     * Figure out the total size and starting point to send
     */
    total_size = (buf->header_size
		  - buf->malloc_pad
		  + msg_size);
    send_ptr = buf->storage + buf->malloc_pad;

    /*
     * Pack the message header
     */
    a = (nexus_byte_t *) send_ptr;
    i = 0;
    tmp_int = total_size - 10 - proto->host_length;
    memcpy(&(a[i]), &tmp_int, 4);
    i += 4;
    memcpy(&(a[i]), &(proto->host_length), 4);
    i += 4;
    memcpy(&(a[i]), &(proto->port), 2);
    i += 2;
    memcpy(&(a[i]), proto->host, proto->host_length);
    i += proto->host_length;
    
    tmp_int = OPEN_FLAG;
#ifdef BUILD_PROFILE
    tmp_int |= PROFILE_MASK;
#endif
    PackInt1(a, i, tmp_int);
    PackInt4(a, i, msg_size);
    PackInt4(a, i, buf->gp->liba_size);
    PackInt4(a, i, buf->handler_id);
    PackInt4(a, i, buf->handler_name_length);
#ifdef BUILD_PROFILE
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    PackInt4(a, i, node_id);
    PackInt4(a, i, context_id);
#endif
    if (buf->gp->liba_is_inline)
    {
	memcpy(&(a[i]),
	       buf->gp->liba.array,
	       buf->gp->liba_size);
    }
    else
    {
	memcpy(&(a[i]),
	       buf->gp->liba.pointer,
	       buf->gp->liba_size);
    }
    i += buf->gp->liba_size;
    memcpy(&(a[i]),
	   buf->handler_name,
	   buf->handler_name_length);

#ifdef BUILD_PROFILE
    _nx_pablo_log_remote_service_request_send(buf->gp->node_id,
					      buf->gp->context_id,
					      buf->handler_name,
					      buf->handler_id,
					      msg_size);
#endif
	
    /*
     * Send 'total_size' bytes, starting at 'send_ptr'.
     */
    _nx_mp_send_bytes(_nx_tcpf_my_forwarder_node,
		      (nexus_byte_t *)send_ptr,
		      total_size);

    FreeTcpfBuffer(buf);

    nexus_poll();

    return(return_code);
    
} /* tcpf_send_remote_service_request() */


/*
 * tcpf_send_urgent_remote_service_request()
 */
static int tcpf_send_urgent_remote_service_request(nexus_buffer_t *buffer)
{
    return(tcpf_send_remote_service_request(buffer));
} /* tcpf_send_urgent_remote_service_request() */


/*
 * tcpf_increment_reference_count()
 *
 * Increase the reference count on the associated proto and copy the
 * pointer to the nexus_proto_t
 *
 */
static void tcpf_increment_reference_count(nexus_proto_t *nproto)
{
    tcpf_proto_t *proto = (tcpf_proto_t *) nproto;
    tcpf_enter();
    proto->reference_count++;
    tcpf_exit();
} /* tcpf_increment_reference_count() */


/*
 * tcpf_decrement_reference_count()
 *
 * Decrement the reference count for this proto.  If it goes to 0
 * then close the fd used by this proto.
 */
static void tcpf_decrement_reference_count(nexus_proto_t *nproto)
{
    tcpf_proto_t *proto = (tcpf_proto_t *) nproto;
    tcpf_enter();
    proto->reference_count--;
    NexusAssert2((proto->reference_count >= 0),
		 ("tcpf_decrement_reference_count(): Internal error: Reference count < 0\n"));
    tcpf_exit();
} /* tcpf_decrement_reference_count() */


/*
 * tcpf_get_my_mi_proto()
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
 * If the hostname and port are those that the tcp forwarder gave me.
 */
static void tcpf_get_my_mi_proto(nexus_byte_t **array,
				int *size)
{
    int i;
    int tmp_int;
    char *host;
    int host_length;

    if (strcmp(_nx_tcpf_my_forwarder_host, _nx_my_node.name) == 0)
    {
	host = "";
    }
    else
    {
	host = _nx_tcpf_my_forwarder_host;
    }
    
    host_length = (strlen(host) + 1);
    *size = 4 + host_length;
    NexusMalloc(tcpf_get_my_mi_proto(),
		*array,
		nexus_byte_t *,
		*size);
    tmp_int = (int) _nx_tcpf_my_forwarder_port;
    i = 0;
    PackInt4(*array, i, tmp_int);
    memcpy(&((*array)[i]), host, host_length);
    
} /* tcpf_get_my_mi_proto() */


/*
 * tcpf_construct_from_mi_proto()
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
static nexus_bool_t tcpf_construct_from_mi_proto(nexus_proto_t **proto,
						nexus_mi_proto_t *mi_proto,
						nexus_byte_t *proto_array,
						int size)
{
    char *host;
    u_short port;
    int i;
    int tmp_int;

    NexusAssert2((size >= 5),
		 ("tcpf_construct_from_mi_proto(): Invalid tcp information in mi_proto\n"));
		 
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
		     ("tcpf_construct_from_mi_proto(): Invalid node name field in mi_proto\n"));
    }

	
    /*
     * Test to see if this proto points to myself.
     * If it does, then return the _nx_local_proto.
     */
    if (   (port == _nx_tcpf_my_forwarder_port)
	&& (strcmp(host, _nx_tcpf_my_forwarder_host) == 0) )
    {
	*proto = (nexus_proto_t *) NULL;
    }
    else
    {
	tcpf_enter();
	*proto = (nexus_proto_t *) construct_proto(host, port);
	tcpf_exit();
    }
    return (NEXUS_TRUE);
} /* tcpf_construct_from_mi_proto() */


/*
 * construct_proto()
 *
 * Construct a tcpf_proto_t for the given host and port. Look up in the
 * proto table to see if one already exists. If it does, bump its reference
 * count and return that one. Otherwise create one, insert into the
 * table with a reference count of 1 and return it.
 */
static tcpf_proto_t *construct_proto(char *host, u_short port)
{
    tcpf_proto_t *proto;

    proto = proto_table_lookup(host, port);
    nexus_debug_printf(3, ("construct_proto(): Table lookup returns proto=%x\n", proto));
    if (proto == (tcpf_proto_t *) NULL)
    {
	NexusMalloc(construct_proto(), proto, tcpf_proto_t *,
		    sizeof(tcpf_proto_t));

	proto->type = NEXUS_PROTO_TYPE_TCP;
	proto->funcs = &tcpf_proto_funcs;
	proto->host = _nx_copy_string(host);
	proto->host_length = strlen(host) + 1;
	proto->port = port;
	proto->reference_count = 1;
	
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
static void free_proto(tcpf_proto_t *proto)
{
    if (proto->host != (char *) NULL)
    {
	NexusFree(proto->host);
    }
    NexusFree(proto);
} /* free_proto() */


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
	proto_table[i].proto = (tcpf_proto_t *) NULL;
	proto_table[i].next = (proto_table_entry_t *) NULL;
    }
} /* proto_table_init() */


/*
 * proto_table_hash()
 *
 * Hash the hostname and port for the proto table.
 */
static int proto_table_hash(char *host, u_short port)
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
 * We assume that the entry is not present in the table.
 */
static void proto_table_insert(tcpf_proto_t *proto)
{
    int bucket;
    proto_table_entry_t *new_ent;

    bucket = proto_table_hash(proto->host, proto->port);
    if (proto_table[bucket].proto == (tcpf_proto_t *) NULL)
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
    nexus_debug_printf(2, ("proto_table_inserted(): Inserted proto=%x for %s/%hu bucket=%d\n",
			   proto, proto->host, proto->port, bucket));
} /* proto_table_insert() */


/*
 * proto_table_lookup()
 *
 * Look up and return the tcpf_proto_t for the given hostname
 * and port. Return NULL if none exists.
 */
static tcpf_proto_t *proto_table_lookup(char *host, u_short port)
{
    proto_table_entry_t *ent;
    int bucket;

    bucket = proto_table_hash(host, port);

    for (ent = &(proto_table[bucket]);
	 ent != (proto_table_entry_t *) NULL;
	 ent = ent->next)
    {
	if (   (ent->proto != (tcpf_proto_t *) NULL)
	    && (ent->proto->port == port)
	    && (strcmp(ent->proto->host, host) == 0) )
	{
	    nexus_debug_printf(2, ("proto_table_lookup(): Found entry %x proto=%x for %s/%hu bucket=%d\n",
				   ent, ent->proto, host, port, bucket));
	    return (ent->proto);
	}
    }
    
    nexus_debug_printf(2, ("proto_table_lookup(): Didn't find entry for %s/%hu bucket=%d\n",
			   host, port, bucket));

    return ((tcpf_proto_t *) NULL);
} /* proto_table_lookup() */


/*********************************************************************
 * 		Buffer management code
 *********************************************************************/

#ifdef NEXUS_SANITY_CHECK
#define SANITY_TYPE_CHECK_SIZE (sizeof(unsigned long) + sizeof(int))
#else  /* NEXUS_SANITY_CHECK */
#define SANITY_TYPE_CHECK_SIZE 0
#endif /* NEXUS_SANITY_CHECK */

/*
 * tcpf_set_buffer_size()
 * 
 * Allocate message buffer space for 'size' bytes, that will be used to
 * hold 'n_elements'.
 */
static void tcpf_set_buffer_size(nexus_buffer_t *buffer,
				int size, int n_elements)
{
    tcpf_buffer_t *tcpf_buffer;
    char *storage;

    NexusBufferMagicCheck(tcpf_check_buffer_size, buffer);

    tcpf_buffer = (tcpf_buffer_t *) *buffer;
    
#ifdef NEXUS_SANITY_CHECK
    if (n_elements != -1) {
	size += SANITY_TYPE_CHECK_SIZE * n_elements;
    }
#endif

    if (size > 0 )
    {
	NexusMalloc(tcpf_set_buffer_size(), storage, char *,
		    (size + tcpf_buffer->header_size));

	tcpf_buffer->size = size;
	tcpf_buffer->n_elements = n_elements;
	tcpf_buffer->storage = storage;
	SetBufferPointers(tcpf_buffer);
	tcpf_buffer->current_pointer = tcpf_buffer->base_pointer;
    }

} /* tcpf_set_buffer_size() */


/*
 * tcpf_check_buffer_size()
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
static int tcpf_check_buffer_size(nexus_buffer_t *buffer,
				 int slack, int increment)
{
    tcpf_buffer_t *tcpf_buffer;
    int used;
    int needed;

    NexusBufferMagicCheck(tcpf_check_buffer_size, buffer);

    tcpf_buffer = (tcpf_buffer_t *) *buffer;
    
    used = (tcpf_buffer->current_pointer
	    - tcpf_buffer->base_pointer);
    needed = used + slack;

    if (tcpf_buffer->size == 0)
    {
	tcpf_set_buffer_size(buffer, slack, -1);
    }
    else if (needed > tcpf_buffer->size)
    {
	char *new_storage;
	int new_size;

	if (increment <= 0)
	    return(NEXUS_FALSE);

	new_size = tcpf_buffer->size;
	while (new_size < needed)
	{
	    new_size += increment;
	}

	NexusMalloc(tcpf_check_buffer_size(), new_storage, char *,
		    (new_size + tcpf_buffer->header_size));
	
	memcpy(new_storage + tcpf_buffer->header_size,
	       tcpf_buffer->base_pointer,
	       used);
	    
	NexusFree(tcpf_buffer->storage);

	tcpf_buffer->size = new_size;
	tcpf_buffer->storage = new_storage;
	SetBufferPointers(tcpf_buffer);
	tcpf_buffer->current_pointer = (tcpf_buffer->base_pointer
					      + used);
    }
    
    return(NEXUS_TRUE);
} /* tcpf_check_buffer_size() */


/*
 * tcpf_sizeof_*()
 *
 * Return the size (in bytes) that 'count' elements of the given
 * type will require to be put into the 'buffer'.
 */
static int tcpf_sizeof_float(nexus_buffer_t *buffer, int count)
{
    return(sizeof(float) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_double(nexus_buffer_t *buffer, int count)
{
    return(sizeof(double) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(short) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_u_short(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned short) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(int) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_u_int(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned int) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(long) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_u_long(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned long) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(char) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_u_char(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned char) * count + SANITY_TYPE_CHECK_SIZE);
}

static int tcpf_sizeof_byte(nexus_buffer_t *buffer, int count)
{
    return(sizeof(unsigned char) * count + SANITY_TYPE_CHECK_SIZE);
}


#ifdef TARGET_ARCH_AIX
#define DO_TRACEBACK _nx_traceback()
#else
#define DO_TRACEBACK
#endif


/*
 * tcpf_put_*()
 *
 * Put 'count' elements, starting at 'data', of the given type,
 * into 'buffer'.
 */

#ifdef BUILD_DEBUG
#define DO_PUT_ASSERTIONS(TYPE) \
    NexusBufferMagicCheck(tcpf_put_ ## TYPE, buf); \
    if (((tcpf_buf->current_pointer - tcpf_buf->base_pointer) + count * sizeof(TYPE) + SANITY_TYPE_CHECK_SIZE) > tcpf_buf->size) \
    { \
	tcpf_fatal("nexus_put_" #TYPE ": Buffer overrun\n"); \
    } \
    if(NexusDebug(3)) \
    { \
        nexus_printf("tcpf_put_" #TYPE "(): base:%x size:%lu current:%x limit:%x count:%d\n", \
		     tcpf_buf->base_pointer, \
		     tcpf_buf->size, \
		     tcpf_buf->current_pointer, \
		     tcpf_buf->base_pointer + tcpf_buf->size, \
		     count); \
    }

#else /* BUILD_DEBUG */
#define DO_PUT_ASSERTIONS(TYPE)
#endif /* BUILD_DEBUG */

#ifdef NEXUS_SPOOF_COPY
#define PUT(TYPE) \
{ \
    tcpf_buffer_t *tcpf_buf = (tcpf_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    if (!_nx_spoof_copy) \
        memcpy(tcpf_buf->current_pointer, data, count * sizeof(TYPE)); \
    tcpf_buf->current_pointer = \
        (char *) tcpf_buf->current_pointer + (count * sizeof(TYPE)); \
}
#else

#ifdef NEXUS_SANITY_CHECK
#define PUT(TYPE) \
{ \
    tcpf_buffer_t *tcpf_buf = (tcpf_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    { \
        unsigned long sanity_flag_long   = 0xfcb1b00a;  \
        unsigned long sanity_flag_int    = 0xfcb1b00b;  \
        unsigned long sanity_flag_char   = 0xfcb1b00c;  \
        unsigned long sanity_flag_double = 0xfcb1b00d;  \
        unsigned long sanity_flag_short  = 0xfcb1b00e;  \
        unsigned long sanity_flag_float  = 0xfcb1b00f;  \
                                                        \
        unsigned long sanity_flag_u_long  = 0xfcb1b01a; \
        unsigned long sanity_flag_u_int   = 0xfcb1b01b; \
        unsigned long sanity_flag_u_char  = 0xfcb1b01c; \
        unsigned long sanity_flag_byte    = 0xfcb1b01d; \
                                                        \
        unsigned long sanity_flag_u_short = 0xfcb1b01e; \
                                                        \
        memcpy(tcpf_buf->current_pointer, & sanity_flag_ ##TYPE, sizeof(unsigned long)); \
        tcpf_buf->current_pointer = \
            (char *) tcpf_buf->current_pointer + (sizeof(unsigned long)); \
        memcpy(tcpf_buf->current_pointer, &count, sizeof(int)); \
        tcpf_buf->current_pointer = \
            (char *) tcpf_buf->current_pointer + (sizeof(int)); \
    } \
    memcpy(tcpf_buf->current_pointer, data, count * sizeof(TYPE)); \
    tcpf_buf->current_pointer = \
        (char *) tcpf_buf->current_pointer + (count * sizeof(TYPE)); \
} 
#else  /* NEXUS_SANITY_CHECK */
#define PUT(TYPE) \
{ \
    tcpf_buffer_t *tcpf_buf = (tcpf_buffer_t *) *buf; \
    DO_PUT_ASSERTIONS(TYPE); \
    memcpy(tcpf_buf->current_pointer, data, count * sizeof(TYPE)); \
    tcpf_buf->current_pointer = \
        (char *) tcpf_buf->current_pointer + (count * sizeof(TYPE)); \
}
#endif /* NEXUS_SANITY_CHECK */
#endif

static void tcpf_put_float(nexus_buffer_t *buf, float *data, int count)
{
    PUT(float);
}

static void tcpf_put_double(nexus_buffer_t *buf, double *data, int count)
{
    PUT(double);
}

static void tcpf_put_short(nexus_buffer_t *buf, short *data, int count)
{
    PUT(short);
}

static void tcpf_put_u_short(nexus_buffer_t *buf, unsigned short *data,
			    int count)
{
    PUT(u_short);
}

static void tcpf_put_int(nexus_buffer_t *buf, int *data, int count)
{
    PUT(int);
}

static void tcpf_put_u_int(nexus_buffer_t *buf, unsigned int *data, int count)
{
    PUT(u_int);
}

static void tcpf_put_long(nexus_buffer_t *buf, long *data, int count)
{
    PUT(long);
}

static void tcpf_put_u_long(nexus_buffer_t *buf, unsigned long *data, int count)
{
    PUT(u_long);
}

static void tcpf_put_char(nexus_buffer_t *buf, char *data, int count)
{
    PUT(char);
}

static void tcpf_put_u_char(nexus_buffer_t *buf, unsigned char *data, int count)
{
    PUT(u_char);
}

static void tcpf_put_byte(nexus_buffer_t *buf, unsigned char *data, int count)
{
    PUT(u_char);
}
