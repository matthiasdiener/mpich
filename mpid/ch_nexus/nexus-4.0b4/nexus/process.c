/*
 * process.c
 *
 * Code to handle process startup.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/process.c,v 1.68 1996/12/11 05:41:23 tuecke Exp $";

#include "internal.h"
#include <fcntl.h>

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

nexus_bool_t		_nx_node_should_exit = NEXUS_FALSE;

static char *		arg_node_name;
static int		arg_node_number;
static nexus_bool_t	arg_use_node_name;
static int		arg_node_id;
static int              arg_num_nodes = 0;
static int		arg_is_context;
static int		arg_checkin_number;

static nexus_node_t     *other_nodes = NULL;
static int              n_other_nodes = 0;

static nexus_endpointattr_t process_ep_attr;

#ifdef BUILD_PROFILE
typedef struct _wait_for_node_id_t
{
    int *		node_id;
    nexus_bool_t	done;
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
} wait_for_node_id_t;

static int			next_node_id;
static nexus_mutex_t		next_node_id_mutex;

static void get_next_node_id(int *node_id, int n_nodes);

static void _nx_get_next_node_id_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static void _nx_get_next_node_id_reply_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
#endif /* BUILD_PROFILE */

#ifdef DONT_INCLUDE
static void _nx_node_defer_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
#endif /* DONT_INCLUDE */

#ifdef DONT_INCLUDE
static nexus_handler_t process_handlers[] =
{
#ifdef BUILD_PROFILE
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_get_next_node_id_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_get_next_node_id_reply_handler},
#endif /* BUILD_PROFILE */
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_node_defer_handler},
};
#else /* DONT_INCLUDE */
static nexus_handler_t *process_handlers = NULL;
#endif /* DONT_INCLUDE */


/*
 * _nx_process_usage_message()
 */
void _nx_process_usage_message(void)
{
    return;
} /* _nx_process_usage_message() */


/*
 * _nx_process_new_process_params()
 */
int _nx_process_new_process_params(char *buf, int size)
{
    char tmp_buf1[MAXHOSTNAMELEN + 32];
    int len;
    
    nexus_stdio_lock();

    sprintf(tmp_buf1, "-nx_mid %s ", _nx_master_id_string);
    len = strlen(tmp_buf1);
    if (len > size)
    {
	nexus_fatal("_nx_process_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    strcpy(buf, tmp_buf1);

    nexus_stdio_unlock();

    return (len);
} /* _nx_process_new_process_params() */


/*
 * nexus_master_node()
 */
int nexus_master_node(void)
{
    return (_nx_master_node);
} /* nexus_master_node() */


/*
 * _nx_process_init()
 *
 * Initialize this new process.  This process may be the master
 * node, a non-master node, or a new context.
 *
 * Create appropriate node and context structures for this process.
 *
 * If I am the master node, create other nodes (as requested via
 * parameters).  Those other nodes will subsequently reach this
 * same point.
 *
 * Return:
 *	On the master node:
 * 		In '*nodes', an array of nexus_node_t's
 *			to the other nodes.
 *		In '*n_nodes', the number of nodes in 'nodes'.
 *	On other processes:
 *		In '*nodes', return NULL.
 *		In '*n_nodes', return 0.
 */
void _nx_process_init(int *argc,
		      char ***argv,
		      char *package_designator,
		      int (*package_args_init_func)(int *argc, char **argv[]),
		      nexus_module_list_t module_list[],
		      nexus_node_t **nodes,
		      int *n_nodes)
{
    nexus_context_t *context = (nexus_context_t *) NULL;
    char hostname[MAXHOSTNAMELEN];
    int arg_num;
#ifdef BUILD_PROFILE
    int node_id;
    int context_id;
    int thread_id;
#endif /* BUILD_PROFILE */

    /*
     * Initialize the fundamental modules.
     */
    ports0_init(argc, argv, package_designator);
    _nx_args_init(argc, argv);
    if (package_args_init_func)
    {
	int rc;
	if ((rc = (package_args_init_func)(argc, argv)) != 0)
	{
	    _nx_md_exit(rc);
	}
    }
    _nx_md_init(argc, argv);
    nexus_fd_init(argc, argv);
    nexus_transform_init(argc, argv, module_list);
#ifndef BUILD_LITE
    _nx_thread_init(argc, argv);
#endif /* BUILD_LITE */
    _nx_context_init(argc, argv);
#ifdef DEPRICATED_HANDLER
    _nx_handler_init(argc, argv);
#endif
    _nx_buffer_init(argc, argv);
    _nx_rdb_init(argc, argv, module_list);
    _nx_fault_tolerance_init(argc, argv);
    _nx_startup_init(argc, argv);
    
    nexus_endpointattr_init(&process_ep_attr);
    nexus_endpointattr_set_handler_table(&process_ep_attr,
					 process_handlers,
#ifdef BUILD_PROFILE
#ifdef DONT_INCLUDE
					 3
#else /* DONT_INCLUDE */
					 2
#endif /* DONT_INCLUDE */
#else /* BUILD_PROFILE */
#ifdef DONT_INCLUDE
					 1
#else /* DONT_INCLUDE */
					 0
#endif /* DONT_INCLUDE */
#endif /* BUILD_PROFILE */
					 );

    /*
     * Get any of this module's arguments
     */
    arg_node_name = (char *) NULL;
    arg_node_number = -1;
    arg_use_node_name = NEXUS_FALSE;
    arg_node_id = -1;
    arg_is_context = NEXUS_FALSE;
    arg_checkin_number = -1;
    _nx_master_id_string = (char *) NULL;
    if ((arg_num = nexus_find_argument(argc, argv, "nx_node", 4)) >= 0)
    {
	arg_node_name = (*argv)[arg_num + 1];
	arg_node_number = atoi((*argv)[arg_num + 2]);
	arg_use_node_name = (*((*argv)[arg_num + 3]) == 'y');
	nexus_remove_arguments(argc, argv, arg_num, 4);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nx_node_id", 2)) >= 0)
    {
	arg_node_id = atoi((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nx_num_nodes", 2)) >= 0)
    {
	arg_num_nodes = atoi((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nx_context", 1)) >= 0)
    {
	arg_is_context = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nx_checkin", 2)) >= 0)
    {
	arg_checkin_number = atoi((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nx_mid", 2)) >= 0)
    {
	_nx_master_id_string = _nx_copy_string((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    
#ifdef BUILD_PROFILE
    next_node_id = -1;
    nexus_mutex_init(&next_node_id_mutex, (nexus_mutexattr_t *) NULL);
#endif /* BUILD_PROFILE */

    if (arg_node_name == (char *) NULL)
    {
	/*
	 * This process is the master node.
	 */
	_nx_master_node = NEXUS_TRUE;
	_nx_my_node_id = 0;
	_nx_my_node.number = 0;
	_nx_my_node.return_code = NEXUS_NODE_NEW;
	_nx_my_node.name = "";
	_nx_my_node.name = _nx_startup_get_master_node_name();
	nexus_startpoint_set_null(&(_nx_my_node.startpoint));
	_nx_master_id_string = _nx_md_get_unique_session_string();
	
	context = _nx_context_alloc(NEXUS_TRUE);

	_nx_process_is_context = NEXUS_FALSE;
    }
    else
    {
	/*
	 * From the arguments we can tell that this 
	 * process is for a non-master node or a context:
	 *	- A -nx_node argument is present, so this is not
	 *	  the master node.
	 *	- If the process is for a context, then there
	 *	  will also be a -nx_context argument.
	 */
	_nx_master_node = NEXUS_FALSE;
	_nx_my_node_id = arg_node_id;
	_nx_my_node.return_code = NEXUS_NODE_NEW;
	_nx_my_node.number = arg_node_number;
	nexus_startpoint_set_null(&(_nx_my_node.startpoint));
	if (arg_use_node_name)
	{
	    /*
	     * Use arg_node_name as the name of this node
	     */
	    _nx_my_node.name = _nx_copy_string(arg_node_name);
	}
	else
	{
	    /*
	     * Ignore the arg_node_name, and use
	     * gethostname() instead, so as to get a canonical
	     * name for this node.
	     */
	    _nx_my_node.name = "";
	    _nx_md_gethostname(hostname, MAXHOSTNAMELEN);
	    _nx_my_node.name = _nx_copy_string(hostname);
	}

	if (arg_is_context)
	{
	    /* This is a context */
	    _nx_process_is_context = NEXUS_TRUE;
	}
	else
	{
	    /*
	     * This is a non-master node
	     */
	    _nx_process_is_context = NEXUS_FALSE;
	}

	if (_nx_master_id_string == (char *) NULL)
	{
	    printf("Fatal error: _nx_process_init(): Internal error: Expected a -nx_mid argument\n");
	    nexus_abort();
	}
	
	/*
	 * Initialize my context structure.
	 * Do this even for a non-master node, just to have
	 * someplace to put the handlers, etc.
	 */
	context = _nx_context_alloc(!_nx_process_is_context);
    }
    
    /* Set the thread's context */
    _nx_set_context(context);
    _nx_initial_context = context;

    /*
     * Initialize the other modules.
     * We want to wait until now to do this so that the
     * thread and context structures are all in place.
     * This allows, for example, protocol modules to create threads.
     */
#ifdef BUILD_PROFILE    
    _nx_pablo_init(argc, argv);
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    _nx_thread_id(&thread_id);
    if (!_nx_process_is_context)
    {
	_nx_pablo_log_node_creation(node_id);
    }
    _nx_pablo_log_context_creation(node_id, context_id);
    _nx_pablo_log_thread_creation(node_id, context_id, thread_id);
#endif

    _nx_commlink_init(argc, argv);
    _nx_proto_init(argc, argv, module_list);
    _nx_attach_init(argc, argv);

#ifndef BUILD_LITE
    _nx_thread_create_handler_thread();
#endif /* BUILD_LITE */
    
    nexus_debug_printf(2, ("_nx_process_init(): _nx_master_node=%d\n", 
			   _nx_master_node));

    if (_nx_master_node)
    {
	_nx_nodelock_master_init();
	
#ifdef BUILD_PROFILE	
	/*
	 * Now that everything is initialize, setup the master_gp
	 */
	nexus_global_pointer(&(context->master_sp), NULL);
#endif	

    	/* Create the rest of the initial nodes */
	_nx_startup_initial_nodes(&_nx_my_node, nodes, n_nodes);

#ifdef BUILD_PROFILE	
	next_node_id = *n_nodes;
#endif
    }
    else
    {
	if (arg_num_nodes >= 1)
	{
	    _nx_startup_initial_nodes(&_nx_my_node,
				      &other_nodes,
				      &n_other_nodes);
	}
	else
	{
	    other_nodes = &_nx_my_node;
	    n_other_nodes = 1;
	}
	*nodes = (nexus_node_t *) NULL;
	*n_nodes = 0;
    }

} /* _nx_process_init() */


/*
 * _nx_process_start()
 *
 * Call NexusBoot() if this process is the master node
 * or if it is for a new context.
 *
 * Then, if this process is not the master node, connect the
 * process back to its creator.
 */
void _nx_process_start(void)
{
    int nexus_boot_rc = 0;
  
    if (_nx_master_node)
    {
	nexus_boot_rc = (*_nx_NexusBoot)(&_nx_my_node.startpoint);
    }
    else /* (!_nx_master_node) */
    {
	/*
	 * Checkin with the process that created this one
	 */
	nexus_startpoint_t creator_sp;
	
	/* Construct a sp to the creator process */
	_nx_proto_construct_creator_sp(&creator_sp);
	
	NexusAssert2((arg_checkin_number >= 0),
		     ("_nx_process_process_arguments(): Internal error: Got a negative checkin number (-nx_checkin) argument.\n"));
	
	/* Invoke the appropriate checkin handler */
	if (_nx_process_is_context)
	{
	    nexus_boot_rc = (*_nx_NexusBoot)(&_nx_my_node.startpoint);
	    _nx_context_checkin(&(_nx_my_node.startpoint),
				nexus_boot_rc,
				&creator_sp,
				arg_checkin_number
#ifdef BUILD_PROFILE
				, NEXUS_TRUE
#endif			    
				);
	}
	else
	{
	    _nx_node_should_exit = _nx_nodelock_check();
	    if (!_nx_node_should_exit)
	    {
		nexus_boot_rc = (*_nx_NexusBoot)(&_nx_my_node.startpoint);
	    }

	    _nx_node_checkin(arg_num_nodes >= 1 ? other_nodes : &_nx_my_node,
			     arg_num_nodes >= 1 ? n_other_nodes : 1,
			     &creator_sp,
			     arg_checkin_number
#ifdef BUILD_PROFILE
			     , NEXUS_TRUE
#endif			    
			     );
	}
	nexus_startpoint_destroy(&creator_sp);
    }

    /*
     * Exit the process if:
     *      NexusBoot() returns non-zero
     *   or There is already a node processes here with my node name.
     */
    if (nexus_boot_rc > 0 || _nx_node_should_exit)
    {
	nexus_debug_printf(1, ("exiting because of non-zero nexus_boot_rc or node is deferring\n"));
	_nx_exit_transient_process(0);
    }
} /* _nx_process_start() */


#ifdef BUILD_PROFILE
/*
 * get_next_node_id()
 *
 * Set *node_id to be the next available node id,
 * and increment next_node_id by 'n_nodes'.
 */
static void get_next_node_id(int *node_id, int n_nodes)
{
    nexus_mutex_lock(&next_node_id_mutex);
    *node_id = next_node_id;
    next_node_id += n_nodes;
    nexus_mutex_unlock(&next_node_id_mutex);
} /* get_next_node_id() */


/*
 * _nx_get_next_node_id_from_master()
 *
 * Set *node_id to be the next available node id,
 * and increment next_node_id by 'n_nodes'.
 *
 * We have to go to the master (context->master_gp) to get the node_id.
 */
void _nx_get_next_node_id_from_master(int *node_id,
				      int n_nodes,
				      nexus_context_t *context)
{
    wait_for_node_id_t wait;
    nexus_startpoint_t wait_sp;
    nexus_endpoint_t wait_ep;
    nexus_buffer_t buffer;
    int buf_size;

    /*
     * Wait for the master_gp to be filled in.
     * If it is NULL, it means the message containing the master_gp
     * is still in transit.
     */
    if (nexus_startpoint_is_null(&(context->master_sp)))
    {
	_nx_wait_for_master_sp_from_creator(context);
    }
    
    /* Initialize wait structure */
    wait.node_id = node_id;
    wait.done = NEXUS_FALSE;
    nexus_mutex_init(&(wait.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(wait.cond), (nexus_condattr_t *) NULL);

    /* Send message to master node */
    nexus_endpoint_init(&wait_ep, &process_ep_attr);
    nexus_endpoint_set_user_pointer(&wait_ep, (void *)&wait);
    nexus_startpoint_bind(&wait_sp, &wait_ep);

    buf_size = nexus_sizeof_startpoint(&wait_sp, 1);
    buf_size += nexus_sizeof_int(1);
    nexus_buffer_init(&buffer, buf_size, 0);

    nexus_put_int(&buffer, &n_nodes, 1);
    nexus_put_startpoint_transfer(&buffer, &wait_sp, 1);
    nexus_send_rsr(&buffer, &context->master_sp, 0, NEXUS_TRUE, NEXUS_FALSE);

    /* Wait for reply */
    nexus_mutex_lock(&(wait.mutex));
    while (!wait.done)
    {
        nexus_cond_wait(&(wait.cond), &(wait.mutex));
    }
    nexus_mutex_unlock(&(wait.mutex));

    nexus_mutex_destroy(&(wait.mutex));
    nexus_cond_destroy(&(wait.cond));
} /* _nx_get_next_node_id_from_master() */


/*
 * _nx_get_next_node_id_handler()
 */
static void _nx_get_next_node_id_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    int node_id;
    int n_nodes;
    nexus_startpoint_t wait_sp;
    nexus_buffer_t reply_buffer;
    int buf_size;

    nexus_get_int(buffer, &n_nodes, 1);
    nexus_get_startpoint(buffer, &wait_sp, 1);

    get_next_node_id(&node_id, n_nodes);

    /* Send reply message */
    buf_size = nexus_sizeof_int(1);
    nexus_buffer_init(&reply_buffer, buf_size, 0);
    nexus_put_int(&reply_buffer, &node_id, 1);
    nexus_send_rsr(&reply_buffer,
		   &wait_sp,
		   1,
		   NEXUS_TRUE,
		   called_from_non_threaded_handler);

    nexus_startpoint_destroy(&wait_sp);
} /* _nx_get_next_node_id_handler() */


/*
 * _nx_get_next_node_id_reply_handler()
 */
static void _nx_get_next_node_id_reply_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    wait_for_node_id_t *wait;
    int node_id;

    wait = (wait_for_node_id_t *)nexus_endpoint_get_user_pointer(endpoint);
    nexus_get_int(buffer, &node_id, 1);

    *(wait->node_id) = node_id;
    
    /* Signal completion of reply */
    nexus_mutex_lock(&(wait->mutex));
    wait->done = NEXUS_TRUE;
    nexus_cond_signal(&(wait->cond));
    nexus_mutex_unlock(&(wait->mutex));
} /* _nx_get_next_node_id_reply_handler() */
#endif /* BUILD_PROFILE */
