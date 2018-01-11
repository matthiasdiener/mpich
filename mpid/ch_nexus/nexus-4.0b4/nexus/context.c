/*
 * context.c
 *
 * Code for maintaining contexts.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/context.c,v 1.80 1996/12/09 23:03:57 tuecke Exp $";

#include "internal.h"

static int		n_contexts;
static int		next_context_id;
static nexus_mutex_t	context_mutex;

static nexus_endpointattr_t context_ep_attr;

/*
 * This will be used in place of a nexus_context_create_handle_t
 * when the reply needs to be forwarded from a new process context
 * to the creator context.
 */
typedef struct _new_context_reply_info_t
{
    int			type;
    nexus_endpoint_t	reply_ep;
    nexus_startpoint_t	reply_sp;
} new_context_reply_info_t;
#define CREATE_CONTEXT_HANDLE		1
#define NEW_CONTEXT_REPLY_INFO		2


#ifdef BUILD_PROFILE
static void _nx_set_master_gp_handler(nexus_endpoint_t *address,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
#endif /* BUILD_PROFILE */

static void context_create_done_handler(nexus_endpoint_t *ep,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
static nexus_handler_t context_create_handlers[] =
{
    {NEXUS_HANDLER_TYPE_NON_THREADED,
     (nexus_handler_func_t) context_create_done_handler},
};
#define CONTEXT_CREATE_HANDLERS_SIZE	1
#define CONTEXT_CREATE_DONE_HANDLER_ID	0


static void finish_destroy_current_context(int called_from_non_threaded_handler,
					   nexus_context_t *context);


/*
 * _nx_context_usage_message()
 */
void _nx_context_usage_message(void)
{
    return;
} /* _nx_context_usage_message() */


/*
 * _nx_context_new_process_params()
 */
int _nx_context_new_process_params(char *buf, int size)
{
    return (0);
} /* _nx_context_new_process_params() */


/*
 * _nx_context_init()
 *
 * Initialize the context module.
 */
void _nx_context_init(int *argc, char ***argv)
{
    n_contexts = 0;
    next_context_id = 0;
    _nx_context_list = NULL;
    nexus_mutex_init(&context_mutex, (nexus_mutexattr_t *) NULL);
#ifndef BUILD_LITE
    nexus_thread_key_create(&_nx_context_key, NULL);
#endif /* BUILD_LITE */

    /*
     * setup context endpoint attr
     */
    nexus_endpointattr_init(&context_ep_attr);
    nexus_endpointattr_set_handler_table(&context_ep_attr,
					 context_create_handlers,
					 CONTEXT_CREATE_HANDLERS_SIZE);
} /* _nx_context_init() */


/*
 * _nx_context_alloc()
 *
 * Initialize my context data structures.
 *
 * Note: When assigning a context id for this context, this routine
 * assumes once context per process.  If that assumption is not valid,
 * then the assignment of context ids must become more sophisticated.
 */
nexus_context_t *_nx_context_alloc(nexus_bool_t is_node)
{
    nexus_context_t *context;
    nexus_segment_t *dummy_segment;
    
    NexusMalloc(_nx_context_alloc(),
		context,
		nexus_context_t *,
		sizeof(nexus_context_t) );
    
    nexus_mutex_init(&(context->mutex), (nexus_mutexattr_t *) NULL);
    nexus_mutex_lock(&context_mutex);
    
    context->context_list_next = _nx_context_list;
    _nx_context_list = context;

    n_contexts++;
    context->id = next_context_id++;

    nexus_mutex_unlock(&context_mutex);
    
    context->handle = NULL;
    context->switchingfunc = NULL;
    context->destructofunc = NULL;
    context->pfnNexusBoot = NULL;

    /* First segment of segment list is a dummy */
    NexusMalloc(_nx_context_alloc(),
		dummy_segment,
		struct _nexus_segment_t *,
		sizeof(struct _nexus_segment_t));
    dummy_segment->data = (void *) NULL;
    dummy_segment->size = 0;
    dummy_segment->context = context;
    context->segment_list = dummy_segment;

    context->n_segments = 0;
    dummy_segment->next = dummy_segment;
    dummy_segment->prev = dummy_segment;
    
    context->is_node = is_node;
    
#ifdef BUILD_PROFILE
    context->snapshot_id = 0;
    context->rsr_profile_count = 0;
    nexus_startpoint_set_null(&(context->master_sp));
    context->waiting_on_master_sp = NEXUS_FALSE;
    nexus_cond_init(&(context->cond), (nexus_condattr_t *) NULL);
#endif /* BUILD_PROFILE */
    
    /*
     * Stow away the context pointer in thread specific storage
     * so that we can get it easily later.
     */
    _nx_set_context(context);
    
    return (context);
} /* _nx_context_alloc() */


/*
 * nexus_context_create_init()
 *
 * Initialize the context creation handle, contexts, for n_contexts
 * simultaneous context creations.  'contexts' should be reinitialized
 * before each reuse.
 */
void nexus_context_create_init(nexus_context_create_handle_t *contexts,
			       int n_contexts)
{
    void **storage;
    
    NexusMalloc(nexus_context_create_init(),
		storage, void **,
		(2 * n_contexts * sizeof(void *)) );
    contexts->type = CREATE_CONTEXT_HANDLE;
    contexts->sp = (nexus_startpoint_t **) storage;
    contexts->rc = (int **) (storage + n_contexts);
    contexts->total = n_contexts;
    contexts->checked_in = 0;
    contexts->next_sp = 0;
    nexus_mutex_init(&(contexts->mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(contexts->cond), (nexus_condattr_t *) NULL);

    /*
     * Set up the endpoint for this handle
     */
    NexusMalloc(nexus_context_create_init(),
		contexts->ep,
		nexus_endpoint_t *,
		sizeof(nexus_endpoint_t));
    nexus_endpoint_init(contexts->ep, &context_ep_attr);
    nexus_endpoint_set_user_pointer(contexts->ep, (void *)contexts);

    nexus_debug_printf(1, ("nexus_context_create_init(): %d contexts\n", n_contexts));
} /* nexus_context_create_init() */


/*
 * nexus_context_create()
 *
 * The context creation protocol is:
 *   1) nexus_context_create() sends an rsr to the
 *	_nx_context_create_handler() on the node_sp
 *	where the context is supposed to be created.
 *   2) _nx_context_create_handler() will create the new context.
 *   3) The new context will reply to this context using the
 *	context_create_done_handler().  The reply contains
 *	the return code of NexusBoot from that context, and
 *	a global pointer to the new context (null_gp if return code != 0).
 *
 * This should really take a 'directory_path' argument.  But since
 * contexts may share a single Unix process, each context cannot
 * control its own directory path.  Therefore contexts are assumed
 * to run in the same directory as the node pointed to by 'node_sp'.
 * This is broken, but we do not have much choice.
 */
void nexus_context_create(nexus_startpoint_t *node_sp,
                          char *executable_path,
                          nexus_startpoint_t *new_context_sp,
			  int *return_code,
                          nexus_context_create_handle_t *contexts)
{
    nexus_buffer_t buffer;
    nexus_startpoint_t create_startpoint;
    int buf_size;
    int checkin_number;
    nexus_context_create_handle_t local_contexts;
    nexus_context_create_handle_t *use_contexts;
    nexus_bool_t using_local_contexts;
    char *default_executable_path;
    int default_executable_path_length;
    int executable_path_length;

    nexus_mutex_lock(&_nx_orphan_mutex);
    _nx_num_outstanding_creates++;
    nexus_mutex_unlock(&_nx_orphan_mutex);

    /* Get the default executable path */
    default_executable_path = _nx_get_argv0();

    /* Get the sizes of the paths */
    default_executable_path_length = strlen(default_executable_path);
    if (executable_path)
    {
	executable_path_length = strlen(executable_path);
    }
    else
    {
	executable_path_length = -1;
    }

    NexusAssert2((executable_path_length <= NEXUS_MAX_EXECUTABLE_PATH_LENGTH),
		 ("nexus_context_create(): executable path, %s, is too long. Maximum length is %d\n",
		  executable_path, NEXUS_MAX_EXECUTABLE_PATH_LENGTH) );
    NexusAssert2((default_executable_path_length <= NEXUS_MAX_EXECUTABLE_PATH_LENGTH),
		 ("nexus_context_create(): default executable path, %s, is too long. Maximum length is %d\n",
		  default_executable_path, NEXUS_MAX_EXECUTABLE_PATH_LENGTH) );

    /* Decide if we need to create a context_handle or use the passed one */
    if (contexts == (nexus_context_create_handle_t *) NULL)
    {
	use_contexts = &local_contexts;
	using_local_contexts = NEXUS_TRUE;
	nexus_context_create_init(use_contexts, 1);
    }
    else
    {
	use_contexts = contexts;
	using_local_contexts = NEXUS_FALSE;
    }

    /* Get a checkin_number and setup the context handle */
    nexus_mutex_lock(&(use_contexts->mutex));
    checkin_number = use_contexts->next_sp++;
    use_contexts->sp[checkin_number] = new_context_sp;
    use_contexts->rc[checkin_number] = return_code;
    nexus_mutex_unlock(&(use_contexts->mutex));

    /* Send rsr to node_sp to run _nx_context_create_handler() */
    nexus_startpoint_bind(&create_startpoint, use_contexts->ep);
    
    buf_size = nexus_sizeof_startpoint(&create_startpoint, 1);
    buf_size += 3 * nexus_sizeof_int(1);
    if (executable_path_length >= 0)
    {
	buf_size += nexus_sizeof_char(executable_path_length);
    }
    buf_size += nexus_sizeof_char(default_executable_path_length);
    nexus_buffer_init(&buffer, buf_size, 0);

    nexus_put_int(&buffer, &executable_path_length, 1);
    if (executable_path_length >= 0)
    {
	nexus_put_char(&buffer, executable_path, executable_path_length);
    }
    nexus_put_int(&buffer, &default_executable_path_length, 1);
    nexus_put_char(&buffer, default_executable_path,
		   default_executable_path_length);
    nexus_put_startpoint_transfer(&buffer, &create_startpoint, 1);
    nexus_put_int(&buffer, &checkin_number, 1);
    nexus_debug_printf(1, ("nexus_context_create(): Creating [%s] with checkin number: %d\n", executable_path, checkin_number));

    nexus_send_rsr(&buffer,
		   node_sp,
		   NEXUS_CONTEXT_CREATE_HANDLER_ID,
		   NEXUS_TRUE /* destroy_buffer */,
		   NEXUS_FALSE /* called_from_nonthreaded_handler */);

    if (using_local_contexts)
    {
	nexus_context_create_wait(use_contexts);
    }
} /* nexus_context_create() */


/*
 * _nx_context_create_handler()
 */
void _nx_context_create_handler(nexus_endpoint_t *ep,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_startpoint_t reply_sp;
    int checkin_number;
    int executable_path_length;
    int default_executable_path_length;
    char executable_path[NEXUS_MAX_EXECUTABLE_PATH_LENGTH + 1];
    char default_executable_path[NEXUS_MAX_EXECUTABLE_PATH_LENGTH + 1];
    char *path;
    new_context_reply_info_t *new_context_reply_info;
    nexus_startpoint_t new_context_reply_info_sp;
    nexus_context_t *this_context;
    nexus_context_t *new_local_context;
    int failure_rc;

    /*
     * Extract the contexts of the buffer
     */
    nexus_get_int(buffer, &executable_path_length, 1);
    if (executable_path_length >= 0)
    {
	nexus_get_char(buffer, executable_path, executable_path_length);
	executable_path[executable_path_length] = '\0';
    }
    nexus_get_int(buffer, &default_executable_path_length, 1);
    nexus_get_char(buffer,
		   default_executable_path,
		   default_executable_path_length);
    default_executable_path[default_executable_path_length] = '\0';

    nexus_get_startpoint(buffer, &reply_sp, 1);
    nexus_get_int(buffer, &checkin_number, 1);

    nexus_stdio_lock();

    if (executable_path_length == -1)
    {
	/* Use default executable path */
	strcpy(executable_path, default_executable_path);
    }
    else if (   (strncmp(executable_path,
			 NEXUS_DATABASE_PREFIX,
			 NEXUS_DATABASE_PREFIX_SIZE) == 0)
	     && (executable_path[NEXUS_DATABASE_PREFIX_SIZE] == ':') )
    {
	/* Lookup the path in the database */
	char *key = executable_path + NEXUS_DATABASE_PREFIX_SIZE + 1;

	nexus_stdio_unlock();
	if (   (*key == '\0')
	    || ((path = nexus_rdb_lookup(_nx_my_node.name,
					 key)) == (char *) NULL) )
	{
	    /* Empty key or the lookup did not find anything */
	    nexus_stdio_lock();
	    strcpy(executable_path, default_executable_path);
	}
	else
	{
	    /* Use the path returned from the database */
	    NexusAssert2((strlen(path) <= NEXUS_MAX_EXECUTABLE_PATH_LENGTH),
			 ("_nx_context_create_handler(): executable path from database, %s, for node %s%d and key %s, is too long. Maximum length is %d\n",
			  path, _nx_my_node.name, _nx_my_node.number, key,
			  NEXUS_MAX_EXECUTABLE_PATH_LENGTH) );
	    nexus_stdio_lock();
	    strcpy(executable_path, path);
	    NexusFree(path);
	}
    }
    /* else use the executable_path as it is */

    nexus_stdio_unlock();
    
    nexus_debug_printf(1, ("_nx_context_create_handler(): Creating:[%s] with checkin number:%d\n", executable_path, checkin_number));
    
    _nx_context(&this_context);

    /*
     * The new context will checkin to this context.
     * This context must, in turn, forward the checkin to
     * the originating context.
     * So create a structure to hold the reply info.  The
     * new context will invoke a handler on this structure.
     */
    NexusMalloc(create_context_in_new_process(),
		new_context_reply_info,
		new_context_reply_info_t *,
		sizeof(new_context_reply_info_t) );
    new_context_reply_info->type = NEW_CONTEXT_REPLY_INFO;
    new_context_reply_info->reply_sp = reply_sp;

    nexus_endpoint_init(&(new_context_reply_info->reply_ep), &context_ep_attr);
    nexus_endpoint_set_user_pointer(&(new_context_reply_info->reply_ep),
				    (void *) new_context_reply_info);
    nexus_startpoint_bind(&new_context_reply_info_sp,
			  &(new_context_reply_info->reply_ep));

    /*
     * Startup the new context
     */
    new_local_context = NULL;
    failure_rc = _nx_startup_context(executable_path,
				     &new_context_reply_info_sp,
				     checkin_number,
				     &new_local_context);
    if (failure_rc != 0)
    {
	/* Startup failed, so checkin with a failure */
	nexus_endpoint_destroy(&(new_context_reply_info->reply_ep));
	NexusFree(new_context_reply_info);
	_nx_context_checkin((nexus_startpoint_t *) NULL,
			    failure_rc,
			    &reply_sp,
			    checkin_number
#ifdef BUILD_PROFILE
			    , NEXUS_FALSE
#endif			    
			    );
	nexus_startpoint_destroy(&reply_sp);
	return;
    }

    if (new_local_context)
    {
	/*
	 * _nx_startup_context() created a context that
	 * lives in this same process.  This requires
	 * a little extra work.
	 */
	int nexus_boot_rc;
	nexus_startpoint_t context_sp;
	
	_nx_set_context(new_local_context);
	
	/*
	 * Call NexusBoot(), since there is no new process
	 * in which to call nexus_start().
	 */
	nexus_startpoint_set_null(&context_sp);
	nexus_debug_printf(1, ("create_context_locally(): calling NexusBoot()=%lu\n", (unsigned long) new_local_context->pfnNexusBoot));
	nexus_boot_rc = (*new_local_context->pfnNexusBoot)(&context_sp);
	nexus_debug_printf(1, ("create_context_locally(): NexusBoot() returned %d\n", nexus_boot_rc));

	/*
	 * Checkin this new local context
	 */
	_nx_context_checkin(&context_sp, nexus_boot_rc,
			    &reply_sp, checkin_number
#ifdef BUILD_PROFILE
			    , NEXUS_TRUE
#endif
			    );
	
	_nx_set_context(this_context);
	nexus_startpoint_destroy(&reply_sp);
    }

} /* _nx_context_create_handler() */


/*
 * _nx_context_checkin()
 *
 * Send a context reply message to 'reply_gp', with the
 * given new context global pointer, 'context_gp'.
 */
void _nx_context_checkin(nexus_startpoint_t *context_sp,
			 int nexus_boot_rc,
			 nexus_startpoint_t *reply_sp,
			 int checkin_number
#ifdef BUILD_PROFILE
			 , int need_master_sp
#endif			 
			 )
{
    nexus_buffer_t buffer;
    int buf_size;
    nexus_startpoint_t *sp;
    nexus_startpoint_t null_sp;

    nexus_debug_printf(2, ("In _nx_context_checkin(): checkin_number %d.\n", checkin_number));
    
    /* Return a null global pointer if nexus_boot_rc != 0 */
    if (nexus_boot_rc != 0)
    {
	nexus_startpoint_set_null(&null_sp);
	sp = &null_sp;
#ifdef BUILD_PROFILE	
	need_master_sp = NEXUS_FALSE;
#endif	
    }
    else
    {
	sp = context_sp;
    }
    
    /* Reply to the creating node */
    buf_size = nexus_sizeof_startpoint(sp, 1);
#ifdef BUILD_PROFILE
    buf_size += nexus_sizeof_int(1) * 3;
#else
    buf_size += nexus_sizeof_int(1) * 2;
#endif
    nexus_buffer_init(&buffer, buf_size, 0);
#ifdef BUILD_PROFILE
    nexus_debug_printf(2, ("_nx_context_checkin(): need_master_gp=%d\n", need_master_gp));
    nexus_put_int(&buffer, &need_master_gp, 1);
#endif    
    nexus_put_int(&buffer, &checkin_number, 1);
    nexus_put_int(&buffer, &nexus_boot_rc, 1);
    nexus_put_startpoint_transfer(&buffer, sp, 1);
    
    nexus_debug_printf(1, ("_nx_context_checkin(): Sending checkin:%d to creator with nexus_boot_rc=%d\n", checkin_number, nexus_boot_rc));

    nexus_send_rsr(&buffer,
		   reply_sp,
		   CONTEXT_CREATE_DONE_HANDLER_ID,
		   NEXUS_TRUE /* destroy_buffer */,
		   NEXUS_TRUE /* called_from_nonthreaded_handler */);

} /* _nx_context_checkin() */


/*
 * context_create_done_handler()
 *
 * When a new context is created, it replies to the creator context
 * using this handler.  (_nx_context_checkin() generates the reply.)
 *
 * The address points to either a nexus_context_create_handle_t
 * or to a new_context_reply_info_t.
 * If it is the former, then the creation reply has reached
 * the creator.  If it is the later, then the creation reply
 * needs to be forwarded on to the originating context.
 */
static void context_create_done_handler(nexus_endpoint_t *ep,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_context_create_handle_t *contexts;
    int checkin_number;
#ifdef BUILD_PROFILE
    int need_master_sp;
#endif    

    contexts
	= (nexus_context_create_handle_t *)nexus_endpoint_get_user_pointer(ep);
    
#ifdef BUILD_PROFILE
    nexus_get_int(buffer, &need_master_sp, 1);
    nexus_debug_printf(2, ("context_create_done_handler(): need_master_gp=%d\n", need_master_sp));
#endif    
    nexus_get_int(buffer, &checkin_number, 1);

    if (contexts->type == CREATE_CONTEXT_HANDLE)
    {
	int *rc;
	nexus_startpoint_t *sp;
	
	rc = contexts->rc[checkin_number];
	sp = contexts->sp[checkin_number];
	nexus_get_int(buffer, rc, 1);
	nexus_get_startpoint(buffer, sp, 1);
	
	nexus_debug_printf(1, ("context_create_done_handler(): received checkin number:%d with rc:%d\n", checkin_number, *rc));

#ifdef BUILD_PROFILE
	if (need_master_sp)
	{
	    _nx_send_master_sp_to_context(sp);
	}
#endif /* BUILD_PROFILE */	
	nexus_debug_printf(2, ("context_create_done_handler(): beginning wait for context checkins\n"));
	nexus_mutex_lock(&(contexts->mutex));
	contexts->checked_in++;
	if (contexts->checked_in >= contexts->total)
	{
	    nexus_cond_signal(&(contexts->cond));
	}
	nexus_mutex_unlock(&(contexts->mutex));
    }
    else /* if (contexts->type == NEW_CONTEXT_REPLY_INFO) */
    {
	new_context_reply_info_t *new_context_reply_info;
	nexus_startpoint_t context_sp;
	int nexus_boot_rc;

	new_context_reply_info
	    = (new_context_reply_info_t *) nexus_endpoint_get_user_pointer(ep);
	nexus_get_int(buffer, &nexus_boot_rc, 1);
	nexus_get_startpoint(buffer, &context_sp, 1);
	nexus_debug_printf(2,("context_create_done_handler(): forwarding reply\n"));
	_nx_context_checkin(&context_sp, nexus_boot_rc,
			    &(new_context_reply_info->reply_sp),
			    checkin_number
#ifdef BUILD_PROFILE
			    , NEXUS_FALSE
#endif			    
			    );

#ifdef BUILD_PROFILE
	if (need_master_sp)
	{
	    _nx_send_master_gp_to_context(&context_sp);
	}
#endif /* BUILD_PROFILE */	
	
	nexus_startpoint_destroy(&context_sp);
	nexus_startpoint_destroy(&(new_context_reply_info->reply_sp));
	nexus_endpoint_destroy(ep);
	NexusFree(new_context_reply_info);
    }
} /* context_create_done_handler() */


/*
 * nexus_context_create_wait()
 *
 * Wait for all context creations held by 'contexts' to complete.
 * Contexts will checkin via context_create_done_handler().
 *
 * After everyone has checked in, free up contexts.
 */
void nexus_context_create_wait(nexus_context_create_handle_t *contexts)
{
    nexus_mutex_lock(&(contexts->mutex));
    while (contexts->checked_in < contexts->total)
    {
	nexus_cond_wait(&(contexts->cond), &(contexts->mutex));
    }
    nexus_mutex_unlock(&(contexts->mutex));

    nexus_mutex_lock(&_nx_orphan_mutex);
    _nx_num_outstanding_creates -= contexts->total;
    if (_nx_num_outstanding_creates == 0)
    {
	nexus_cond_signal(&_nx_orphan_cond);
    }
    nexus_mutex_unlock(&_nx_orphan_mutex);

    nexus_mutex_destroy(&(contexts->mutex));
    nexus_cond_destroy(&(contexts->cond));
    nexus_endpoint_destroy(contexts->ep);
    NexusFree(contexts->sp);

} /* nexus_context_create_wait() */


/*
 * nexus_context_destroy()
 *
 * Terminate the current context
 */
void nexus_context_destroy(int called_from_non_threaded_handler)
{
    nexus_context_t *context;

    _nx_context(&context);

    if (context->is_node)
    {
	/*
	 * Shutdown this context as a node.
	 */
	_nx_nodelock_shutdown();
    }
    
    nexus_debug_printf(1,("nexus_context_destroy(): initiating context destruction, called_from_non_threaded_handler=%d\n", called_from_non_threaded_handler));

    /* Call NexusExit() */
    _nx_NexusExit();
    
#ifdef BUILD_PROFILE
    /*
     * Wait for the master_gp to be filled in.
     * If it is NULL, it means the message containing the master_gp
     * is still in transit.  (We managed to create the context
     * and destroy it before the master_gp message arrived.  In practice
     * this shouldn't be a problem, but just to be safe...)
     */
    if (nexus_startpoint_is_null(&(context->master_sp)))
    {
	if (called_from_non_threaded_handler)
	{
	    /*
	     * If this is called from a non-threaded handler,
	     * then we cannot call _nx_wait_for_master_gp_from_creator().
	     * That would block this thread and cause deadlock, since
	     * that handler thread would not be available to handle
	     * the subsequent message with the master_gp.
	     *
	     * In that case, set a flag and return right away without
	     * blocking.  When the master_gp finally shows up,
	     * _nx_set_master_gp_handler() will check this flag and
	     * finish the context destruction then.
	     *
	     * BTW -- We must wait for it.  Otherwise, the sending side
	     * will error out when it eventually gets around to sending
	     * the master_gp.
	     */
	    nexus_mutex_lock(&(context->mutex));
	    if (nexus_startpoint_is_null(&(context->master_sp)))
	    {
		context->waiting_on_master_sp = NEXUS_TRUE;
	    }
	    nexus_mutex_unlock(&(context->mutex));
	    nexus_debug_printf(2, ("nexus_context_destroy(): waiting on master_gp flag, so postponing context destruction until its arrival\n"));
	    return;
	}
	else
	{
	    _nx_wait_for_master_sp_from_creator(context);
	}
    }
#endif /* BUILD_PROFILE */

    finish_destroy_current_context(called_from_non_threaded_handler, context);
} /* nexus_context_destroy() */


/*
 * finish_destroy_current_context()
 */
static void finish_destroy_current_context(
					  int called_from_non_threaded_handler,
					  nexus_context_t *context)
{
    nexus_segment_t *dummy_segment, *segment, *next_segment;

    nexus_debug_printf(2, ("finish_destroy_current_context(): entering\n"));

#ifdef BUILD_PROFILE
    if (_nx_pablo_count_remote_service_requests())
    {
	_nx_dump_rsr_profile_info(context);
    }
#endif /* BUILD_PROFILE */
    
    nexus_mutex_destroy(&(context->mutex));

    /* The first segment is a dummy.  The segment_list is circular. */
    dummy_segment = context->segment_list;
    segment = dummy_segment->next;
    while (segment != dummy_segment)
    {
	next_segment = segment->next;
	NexusFree(segment);
	segment = next_segment;
    }
    NexusFree(dummy_segment);
    
#ifdef BUILD_PROFILE
    nexus_startpoint_destroy(&(context->master_sp));
    nexus_cond_destroy(&(context->cond));
#endif    
    
    if (context->destructofunc != (int (*)(nexus_context_t*))(NULL) )
    { 
	int rc;
	rc = (*context->destructofunc)(context);
    }

    NexusFree(context);
    
    nexus_mutex_lock(&context_mutex);
    n_contexts--;
    nexus_mutex_unlock(&context_mutex);
    
    if (n_contexts <= 0)
    {
	/* There are no more contexts in this process, so shut it down. */
	_nx_exit_transient_process(0);
    }
#ifndef BUILD_LITE
    else if (!called_from_non_threaded_handler)
    {
	/*
	 * If nexus_context_destroy() is called from a handler
	 * thread (i.e. from a non-threaded handler), then we should
	 * not exit the thread.  Instead, we should return from this
	 * nexus_context_destroy() call.
	 * However, if it is called from a normal user thread it should
	 * exit the thread.
	 * We decided to add a user supplied argument since that does
	 * not partially hide things from the user.  The user is going
	 * to have to know that nexus_context_destroy()
	 * returns when it is called from a non-threaded handler,
	 * so we might as well let that user tell us if they are in one.
	 */
	nexus_debug_printf(2, ("finish_destroy_current_context(): exiting thread\n"));
        nexus_thread_exit(NULL);
    }
#endif /* BUILD_LITE */

    nexus_debug_printf(2, ("finish_destroy_current_context(): returning\n"));
    
} /* finish_destroy_current_context() */


/*
 * nexus_malloc()
 */
void *nexus_malloc(size_t size)
{
    nexus_context_t *context;
    nexus_segment_t *segment;

    if (size <= 0)
    {
	return (NULL);
    }
    
    _nx_context(&context);

    NexusMalloc(nexus_malloc(), segment, nexus_segment_t *,
		(NEXUS_SEGMENT_HEADER_SIZE + size));

    segment->data = (void *) (((char *) segment) + NEXUS_SEGMENT_HEADER_SIZE);
    segment->size = size;
    segment->context = context;
    nexus_mutex_lock(&(context->mutex));
    segment->prev = context->segment_list;
    segment->next = context->segment_list->next;
    context->segment_list->next->prev = segment;
    context->segment_list->next = segment;
    context->n_segments++;
    nexus_mutex_unlock(&(context->mutex));

    return (segment->data);
} /* nexus_malloc() */


/*
 * nexus_free()
 */
void nexus_free(void *data_segment)
{
    nexus_context_t *context;
    nexus_segment_t *segment;
    
    segment = (nexus_segment_t *) (((char *) data_segment)
				   - NEXUS_SEGMENT_HEADER_SIZE);
    context = segment->context;
    nexus_mutex_lock(&(context->mutex));
    segment->next->prev = segment->prev;
    segment->prev->next = segment->next;
    context->n_segments--;
    nexus_mutex_unlock(&(context->mutex));

    NexusFree((void *) segment);
} /* nexus_free() */


#ifdef BUILD_PROFILE

/*
 * _nx_wait_for_master_sp_from_creator()
 *
 * In order to assign node_id's for profiling, the master node
 * must act as a broker for node_id assignment.  But this means
 * that each process needs access to the master node process.
 *
 * When a new node or context is created, the newly created thing
 * calls _nx_wait_for_master_gp_from_creator().  The creating side
 * calls _nx_send_master_gp_to_context().  These two routines
 * take care of getting the master_gp to the new process.
 */
void _nx_wait_for_master_sp_from_creator(nexus_context_t *context)
{

    nexus_debug_printf(2, ("_nx_wait_for_master_gp_from_creator(): beginning wait\n"));
    nexus_mutex_lock(&(context->mutex));
    while (nexus_startpoint_is_null(&(context->master_sp)))
    {
	nexus_cond_wait(&(context->cond), &(context->mutex));
    }
    nexus_mutex_unlock(&(context->mutex));
    nexus_debug_printf(2, ("_nx_wait_for_master_gp_from_creator(): ending wait\n"));
} /* _nx_wait_for_master_sp_from_creator() */


/*
 * _nx_set_master_sp_handler()
 */
static void _nx_set_master_sp_handler(nexus_endpoint_t *ep,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_context_t *context;
    
    nexus_debug_printf(2,("_nx_set_master_gp_handler(): got master_gp\n"));
    _nx_context(&context);
    nexus_debug_printf(2,("_nx_set_master_gp_handler(): got context\n"));
    nexus_debug_printf(2,("context:%x\n",context));
    nexus_mutex_lock(&(context->mutex));
    nexus_debug_printf(2, ("_nx_set_master_gp_handler(): got lock\n"));
    nexus_get_startpoint(buffer, &(context->master_sp), 1);
    nexus_cond_signal(&(context->cond));
    nexus_mutex_unlock(&(context->mutex));

    nexus_debug_printf(2, ("_nx_set_master_gp_handler(): checking on pending destroy\n"));

    if (context->waiting_on_master_sp)
    {
	nexus_debug_printf(2, ("_nx_set_master_gp_handler(): finishing destroy\n"));
	finish_destroy_current_context(NEXUS_TRUE, context);
    }

    nexus_debug_printf(2, ("_nx_set_master_gp_handler(): returning\n"));
} /* _nx_set_master_sp_handler() */


/*
 * _nx_send_master_sp_to_context()
 */
void _nx_send_master_sp_to_context(nexus_startpoint_t *context_sp)
{
    nexus_buffer_t buffer;
    nexus_context_t *context;
    int buf_size;
    nexus_startpoint_t sp_copy;

    nexus_debug_printf(2,("_nx_send_master_gp_to_context(): sending\n"));

    _nx_context(&context);
    
    /*
     * Wait for the master_gp to be filled in.
     * If it is NULL, it means the message containing the master_gp
     * is still in transit.
     */
    if (nexus_startpoint_is_null(&(context->master_sp)))
    {
	_nx_wait_for_master_sp_from_creator(context);
    }

    nexus_debug_printf(2, ("_nx_send_master_gp_to_context(): received master_gp from creator\n"));

    /* Send master_gp to context_gp */
    nexus_startpoint_copy(&sp_copy, &context->master_gp);
    buf_size = nexus_sizeof_startpoint(&sp_copy, 1);
    nexus_buffer_init(&buffer, buf_size, 0);
    nexus_put_startpoint_transfer(&buffer, &sp_copy, 1);
    nexus_send_rsr(&buffer, context_sp, 1, 2, NEXUS_TRUE, NEXUS_FALSE);

    nexus_debug_printf(2,("_nx_send_master_gp_to_context(): sent\n"));
} /* _nx_send_master_gp_to_context() */

#endif /* BUILD_PROFILE */
