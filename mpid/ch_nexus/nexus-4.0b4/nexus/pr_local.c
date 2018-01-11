/*
 * Nexus
 * Authors:     Steven Tuecke and Robert Olson
 *              Argonne National Laboratory
 *
 * pr_local.c		- local protocol module
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_local.c,v 1.41 1996/12/12 07:48:23 tuecke Exp $";

#include "internal.h"


/*
 * Some forward typedef declarations...
 */
typedef struct _local_buffer_t	local_buffer_t;
typedef struct _local_proto_t	local_proto_t;


#ifdef BUILD_DEBUG
/*
static char tmpbuf1[10240];
static char tmpbuf2[10240];
*/
#endif

#ifdef BUILD_LITE
#define local_enter()
#define local_exit()
#else  /* BUILD_LITE */
static nexus_mutex_t	local_mutex;
#define local_enter()	nexus_mutex_lock(&local_mutex);
#define local_exit()	nexus_mutex_unlock(&local_mutex);
#endif /* BUILD_LITE */

static nexus_bool_t		handle_in_progress;
static struct _nexus_buffer_t *	handle_q_head;
static struct _nexus_buffer_t *	handle_q_tail;

/*
 * local_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * local specific information to that structure.
 */
struct _local_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_LOCAL */
    nexus_proto_funcs_t *funcs;
};


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
static nexus_proto_type_t	local_proto_type(void);
static void		local_init(int *argc, char ***argv);
static void		local_shutdown(nexus_bool_t shutdown_others);
static void		local_abort(void);
static int		local_send_rsr(nexus_buffer_t *buffer,
				       nexus_startpoint_t *sp_array,
				       int num_sp,
				       int handler_id,
				       nexus_bool_t destroy_buffer);
static nexus_bool_t	local_construct_from_mi_proto(nexus_proto_t **proto,
						    nexus_mi_proto_t *mi_proto,
						    nexus_byte_t *array,
						    int size);
static int              local_direct_info_size(void);
static int              local_direct_get(nexus_byte_t *dest,
					 size_t n_bytes,
					 int action,
					 unsigned long info);

static nexus_proto_funcs_t local_proto_funcs =
{
    local_proto_type,
    local_init,
    local_shutdown,
    local_abort,
    NULL /* local_poll */,
    NULL /* local_poll_blocking */,
    NULL /* local_increment_reference_count */,
    NULL /* local_decrement_reference_count */,
    NULL /* local_get_my_mi_proto */,
    local_construct_from_mi_proto,
    NULL /* local_test_proto */,
    local_send_rsr,
    local_direct_info_size,
    local_direct_get,
};


/*
 * _nx_pr_local_info()
 *
 * Return the nexus_proto_funcs_t function table for this protocol module.
 *
 * This procedure is used for bootstrapping the protocol module.
 * The higher level Nexus code needs to call this routine to
 * retrieve the functions it needs to use this protocol module.
 */
void *_nx_pr_local_info(void)
{
    return((void *) (&local_proto_funcs));
} /* _nx_pr_local_info() */


/*
 * local_proto_type()
 *
 * Return the nexus_proto_type_t for this protocol module.
 */
static nexus_proto_type_t local_proto_type(void)
{
    return (NEXUS_PROTO_TYPE_LOCAL);
} /* local_proto_type() */


/*
 * local_init()
 *
 * Initialize the LOCAL protocol.
 */
static void local_init(int *argc, char ***argv)
{
    handle_in_progress = NEXUS_FALSE;
#ifndef BUILD_LITE
    nexus_mutex_init(&local_mutex, (nexus_mutexattr_t *) NULL);
#endif
    
    return ;
} /* local_init() */


/*
 * local_shutdown()
 *
 * This routine is called during normal shutdown of a process.
 */
static void local_shutdown(nexus_bool_t shutdown_others)
{
} /* local_shutdown() */


/*
 * local_abort()
 *
 * This routine is called during the _abnormal_ shutdown of a process.
 */
static void local_abort(void)
{
} /* local_abort() */


/*
 * local_send_rsr()
 *
 * Generate a local remote service request for 'nexus_buffer'.
 */
static int local_send_rsr(nexus_buffer_t *buffer,
			  nexus_startpoint_t *startpoint,
			  int handler_id,
			  nexus_bool_t destroy_buffer,
			  nexus_bool_t called_from_non_threaded_handler)
{
    nexus_base_segment_t *base_segment;
    nexus_direct_segment_t *direct_segment;
    unsigned long total_direct_puts;
    int i;
    struct _nexus_buffer_t *r_buffer;
    
    NexusBufferMagicCheck(local_send_remote_service_request, buffer);
    NEXUS_INTERROGATE(startpoint, _NX_STARTPOINT_T, "tcp_send_rsr");
    nexus_debug_printf(2, ("local_send_rsr(): invoked with buffer:%x\n",buffer));

    total_direct_puts = 0;
    for (direct_segment = (*buffer)->direct_segments;
	 direct_segment;
	 direct_segment = direct_segment)
    {
        for (i = 0; i < direct_segment->size; i++)
	{
	    direct_segment->storage[i].action
	        = NEXUS_DIRECT_INFO_ACTION_POINTER;
	}
	total_direct_puts += direct_segment->size;
    }

    _nx_buffer_coalesce(*buffer,
			&r_buffer,
			startpoint,
			handler_id,
			total_direct_puts,
			called_from_non_threaded_handler,
			NEXUS_FALSE,
			destroy_buffer);
    r_buffer->funcs = &local_proto_funcs;

    local_enter();
    Enqueue(handle_q_head, handle_q_tail, r_buffer);

    if (!handle_in_progress)
    {
	handle_in_progress  = NEXUS_TRUE;
	while (QueueNotEmpty(handle_q_head))
	{
	    Dequeue(handle_q_head, handle_q_tail, r_buffer);
	    local_exit();
	    _nx_buffer_dispatch(r_buffer);
	    local_enter();
	}
	handle_in_progress = NEXUS_FALSE;
    }

    local_exit();

    nexus_poll();

    return(0);
    
} /* local_send_rsr() */


/*
 * local_construct_from_mi_proto()
 *
 * Return my proto.  This is called only during initialization by
 * pr_iface.c:_nx_proto_init(), so that it can cache this proto.
 */
static nexus_bool_t local_construct_from_mi_proto(nexus_proto_t **proto,
						  nexus_mi_proto_t *mi_proto,
						  nexus_byte_t *array,
						  int size)
{
    local_proto_t *local_proto;
    
    NexusMalloc(local_construct_from_mi_proto(),
		local_proto,
		local_proto_t *,
		sizeof(local_proto_t) );
    local_proto->type = NEXUS_PROTO_TYPE_LOCAL;
    local_proto->funcs = &local_proto_funcs;
    *proto = (nexus_proto_t *) local_proto;
    
    return (NEXUS_TRUE);
} /* local_construct_from_mi_proto() */


/*
 * local_direct_info_size()
 */
int local_direct_info_size(void)
{
    return 0;
}


/*
 * local_direct_get()
 */
int local_direct_get(nexus_byte_t *dest,
		     size_t n_bytes,
		     int action,
		     unsigned long info)
{
    memcpy(dest, (void *)info, n_bytes);
    return(0);
} /* local_direct_get() */
