/*
 * st_mpl.c
 *
 * Remote node startup using IBM SP2 MPL
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_mpl.c,v 1.23 1996/10/07 04:40:19 tuecke Exp $";

#include "internal.h"
#include "mpproto.h"

#ifndef NEXUS_MPL_NODE_NAME
#define NEXUS_MPL_NODE_NAME "sp2_mpl"
#endif

#define INITIAL_MSG_TYPE   67
#define _NX_MP_ACQUIRE_NODE_HANDLER_HASH 786

static int my_node;
static int n_nodes;
static nexus_bool_t nodes_started = NEXUS_FALSE;
static char *master_name = NULL;
static char **HACK = NULL;

extern char *	_nx_session_string;
extern int	_nx_session_string_length;

static nexus_bool_t	st_mpl_preinit(nexus_startup_node_t **nl_head,
				       nexus_startup_node_t **nl_tail);
static char *		st_mpl_get_master_node_name(void);
static void     	st_mpl_init(int *argc, char ***argv);
#ifdef NEXUS_USE_FORWARDER
static void             st_mpl_shutdown(nexus_bool_t shutdown_others);
#endif
static int		st_mpl_start_node(nexus_startup_node_t *node,
					  nexus_startpiont_t *reply_sp,
					  int first_checkin_number);
#ifndef HAVE_IBMDS
static nexus_bool_t	st_mpl_start_context(char *executable_path,
				     nexus_startpoint_t *reply_sp,
				     int checkin_number,
				     int *return_code,
				     nexus_context_t **new_local_context);
#endif /* !HAVE_IBMDS */
static void     st_mpl_register_handlers(void);
static void _nx_mp_acquire_node_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static nexus_startup_funcs_t st_mpl_funcs =
{
    st_mpl_preinit,
    st_mpl_get_master_node_name,
    st_mpl_init /* st_mpl_init */,
#ifdef NEXUS_USE_FORWARDER
    st_mpl_shutdown,
#else
    NULL /* st_mpl_shutdown */,
#endif
    NULL /* st_mpl_abort */,
    st_mpl_start_node,
#ifdef HAVE_IBMDS    
    NULL /* st_mpl_start_context */,
#else    
    st_mpl_start_context,
#endif
};

static nexus_endpointattr_t mpl_ep_attr;

static nexus_handler_t mpl_startup_handlers[] =
{ \
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_mp_acquire_node_handler},
};

/*
 * _nx_st_mpl_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_mpl_info(void)
{
    return ((void *) (&st_mpl_funcs));
}


/*
 * st_mpl_preinit()
 */
static nexus_bool_t st_mpl_preinit(nexus_startup_node_t **nl_head,
				   nexus_startup_node_t **nl_tail)
{
    int source, type;
    size_t n_bytes, n_bytes_out;
    nexus_bool_t ignore_command_line_args;

    *nl_head = *nl_tail = (nexus_startup_node_t *) NULL;
    
    /*
     * TODO: Need a way to sense if MPL is being used
     */
    mpc_environ(&n_nodes, &my_node);

#ifdef NEXUS_USE_FORWARDER
    {
	/*
	 * The last node is always the forwarder node.
	 * Remove it from the set of nexus nodes.
	 * Get the tcp forwarder information from the forwarder node.
	 * This needs to be done here to avoid any race conditions
	 * in the mpl setup.
	 */
	extern u_short	_nx_tcpf_my_forwarder_port;
	extern char	_nx_tcpf_my_forwarder_host[MAXHOSTNAMELEN];
	extern int	_nx_tcpf_my_forwarder_node;
	char forwarder_info[MAXHOSTNAMELEN+4];

	n_nodes--;
	_nx_tcpf_my_forwarder_node = n_nodes;
	source = n_nodes;
	type = DONTCARE;
	mpc_brecv(forwarder_info,
		  MAXHOSTNAMELEN+4,
		  &source,
		  &type,
		  &n_bytes_out);
	memcpy(&_nx_tcpf_my_forwarder_port, forwarder_info, 2);
	strcpy(_nx_tcpf_my_forwarder_host, forwarder_info+2);
    }
#endif /* NEXUS_USE_FORWARDER */

    /*
     * There is only one node in the computation, and it should NOT
     * ignore the command line parameters.
     */
    if (n_nodes == 1)
	return (NEXUS_FALSE);
    
    if (my_node == 0)
    {
	/*
	 * This is the master node.
	 * Setup a nexus_startup_node_t structure for the rest
	 * of the MPL nodes.  That will cause st_mpl_start_node
	 * to be called during Nexus initialization.
	 */
	nexus_startup_node_t *initial_node;
	NexusMalloc(st_mpl_preinit(),
		    initial_node,
		    nexus_startup_node_t *,
		    sizeof(nexus_startup_node_t));
	initial_node->name = _nx_copy_string(NEXUS_MPL_NODE_NAME);
	HACK = &(initial_node->name);
	initial_node->number = 1;
	initial_node->count = n_nodes - 1;
	initial_node->count_other_nodes = 0;
	initial_node->startup_module = _nx_copy_string("mpl");
	initial_node->startup_funcs = &st_mpl_funcs;
	initial_node->next = (nexus_startup_node_t *) NULL;
	initial_node->directory_path = (char *) NULL;
	initial_node->executable_path = (char *) NULL;
	initial_node->st_ptr = NULL;

	*nl_head = *nl_tail = initial_node;

	ignore_command_line_args = NEXUS_FALSE;
    }
    else
    {
	/*
	 * This is a non-master node.
	 * Wait for a message from the master node that contains
	 * _nx_my_process_params for my node.
	 * That message is sent from st_mpl_start_node.
	 */
	n_bytes = NEXUS_MAX_COMMAND_LENGTH;
	NexusMalloc(st_mpl_preinit(),
		    _nx_my_process_params,
		    char *,
		    n_bytes);
	source = DONTCARE;
	type = DONTCARE;
	mpc_brecv(_nx_my_process_params,
		  n_bytes,
		  &source,
		  &type,
		  &n_bytes_out);

	if (   (source != 0)
	    && (type != INITIAL_MSG_TYPE) )
	{
	    fprintf(_nx_stdout,
		    "ERROR: MPL node %d expected initial message\n");
	    mpc_stopall(1);
	    exit(1);
	}
	nodes_started = NEXUS_TRUE;
	ignore_command_line_args = NEXUS_TRUE;
    }

    return(ignore_command_line_args);
} /* st_mpl_preinit() */


/*
 * st_mpl_init()
 */
static void st_mpl_init(int *argc, char ***argv)
{
    int arg_num;

    nexus_endpointattr_init(&mpl_ep_attr);
    nexus_endpointattr_set_handler_table(&mpl_ep_attr,
					 mpl_startup_handlers,
					 1);
    
    if ((arg_num = nexus_find_argument(argc, argv, "nx_mpl_session", 2)) >= 0)
    {
        _nx_session_string = _nx_copy_string((*argv)[arg_num+1]);
        nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	_nx_session_string = _nx_md_get_unique_session_string();
    }
    _nx_session_string_length = strlen(_nx_session_string);

    if ((arg_num = nexus_find_argument(argc, argv, "nx_mpl_master", 2)) >= 0)
    {
	master_name = _nx_copy_string((*argv)[arg_num+1]);
	if (HACK)
	{
	    *HACK = master_name;
	}
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	master_name = _nx_copy_string(NEXUS_MPL_NODE_NAME);
    }
}

#ifdef NEXUS_USE_FORWARDER
/*
 * We need to send a message to the forwarder that it should shutdown
 * so that wierd things don't happen.
 */
static void st_mpl_shutdown(nexus_bool_t shutdown_others)
{
    int buffer = -2;
    int id;
    int type = 42;
    extern int _nx_tcpf_my_forwarder_node;

    mpc_send((char *)&buffer,
	     sizeof(int),
	     _nx_tcpf_my_forwarder_node,
	     type,
	     &id);
}
#endif


/*
 * st_mpl_get_master_node_name()
 */
static char *st_mpl_get_master_node_name(void)
{
    return(master_name);
} /* st_mpl_get_master_node_name() */


/*
 * _nx_start_nodes()
 */
static int st_mpl_start_node(nexus_startup_node_t *node,
			     nexus_startpoint_t *reply_sp,
			     int first_checkin_number)
{
    int i;
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    int cmd2_len;
    int msglen;
    int type = INITIAL_MSG_TYPE;

    /* Cannot start dynamic nodes, but must call NexusAcquiredAsNode
     * on requested node
     */
    if (strcmp(node->name, master_name) != 0)
    {
	/* trying to start a different MPL partition */
	return(1);
    }
    if (nodes_started == NEXUS_TRUE)
    {
        /*
	 * JGG
	 *
	 * This whole section needs reworking because hacking together a global
	 * pointer is not possible--especially for a version of Nexus with
	 * signed global pointers so that they cannot be spoofed.
	 * Otherwise, anyone could pull this stunt and transfer where their RSR
	 * goes to.
	 *
	 * JGG
	 */
	/*
	 * Hack together a global pointer to the node we want
	 * to acquire.  It should be within the current
	 * set of nodes that we have.  Send an RSR to that node
	 * to invoke a handler which will return a "good" global
	 * pointer to itself and also call NexusAcquiredAsNode
	 */
	nexus_startpoint_t my_sp;
	nexus_endpoint_t my_ep;
	nexus_startpoint_t remote_sp;
	nexus_byte_t *a;
	int i;
	int tmp_int;
	int node_number;
	int context_differentiator;
	char *node_name;
	nexus_buffer_t buffer;
	int buf_len = 0;

	/* Get one of my sp's to start with */
	nexus_endpoint_init(&my_ep, &mpl_ep_attr);
	nexus_startpoint_bind(&my_sp, &my_ep);

	remote_sp = my_sp;

	/* Fix up the profiling info */
#ifdef BUILD_PROFILE
	remote_sp.node_id = node->number;
	remote_sp.context_id = 0;
#endif /* BUILD_PROFILE */

	/* Copy the liba, and zero out the context field */
	if (my_sp.liba_is_inline)
	{
	    memcpy(remote_sp.liba.array, my_sp.liba.array, my_sp.liba_size);
	    a = remote_sp.liba.array;
	}
	else
	{
	    NexusMalloc(st_mpl_start_node(),
			remote_sp.liba.pointer,
			nexus_byte_t *,
			my_sp.liba_size);
	    memcpy(remote_sp.liba.pointer,my_sp.liba.pointer,my_sp.liba_size);
	    a = remote_sp.liba.pointer;
	}
	ZeroOutMemory((char *) a, sizeof(unsigned long));

	/* Duplicate the mi_proto */
	NexusMalloc(st_mpl_start_node(),
		    remote_sp.mi_proto,
		    nexus_mi_proto_t *,
		    (sizeof(nexus_mi_proto_t) + my_sp.mi_proto->size));
	remote_sp.mi_proto->proto = (nexus_proto_t *) NULL;
	remote_sp.mi_proto->next = (nexus_mi_proto_t *) NULL;
	remote_sp.mi_proto->size = my_sp.mi_proto->size;
	remote_sp.mi_proto->array
	    = (nexus_byte_t *) (((char *) remote_sp.mi_proto)
				+ sizeof(nexus_mi_proto_t));
	memcpy(remote_sp.mi_proto->array, my_sp.mi_proto->array,
	       my_sp.mi_proto->size);
	       
	/* Hack the mi_proto header to have the right node number */
	a = remote_sp.mi_proto->array;
	i = 0;
	PackInt4(a, i, node->number);
	i = 0;
	UnpackMIProtoHeader(a, i,
			    node_number,
			    context_differentiator,
			    node_name);

	/* Hack the mpl mi_proto entry to have the right node number */
	UnpackInt2(a, i, tmp_int); /* type */
	if (((nexus_proto_type_t) tmp_int) != NEXUS_PROTO_TYPE_MPL)
	{
	    nexus_fatal("st_mpl_start_node(): Internal error: Expected MPL proto to be first in mi_proto list\n");
	}
	UnpackInt2(a, i, tmp_int); /* size */
	i += _nx_session_string_length + 1; /* session string */
	memcpy((char *) (a + i), (char *) &(node->number), sizeof(int));

	/* Instatiate the new proto */
	_nx_mi_proto_instantiate(remote_sp.mi_proto);

	/*
	 * We now should have a usable remote_gp.
	 * So sent a message to the node to acquire it.
	 */
	nexus_init_remote_service_request(&buffer,
					  &remote_gp,
					  "_nx_mp_acquire_node_handler",
					  _NX_MP_ACQUIRE_NODE_HANDLER_HASH);
	buf_len = nexus_sizeof_global_pointer(&buffer, reply_gp, 1,
					      &n_elements);
	nexus_set_buffer_size(&buffer, (buf_len + nexus_sizeof_int(&buffer,1)),
			      (n_elements + 1));
	nexus_put_global_pointer(&buffer, reply_gp, 1);
	nexus_put_int(&buffer, &first_checkin_number, 1);
	nexus_send_remote_service_request(&buffer);

	/* Clean up */
	nexus_destroy_global_pointer(&my_gp);
	nexus_destroy_global_pointer(&remote_gp);
    }
    else
    {
	/* Start the nodes on the machines listed in arg_host_list */
	for (i = 0; i < node->count; i++)
	{
	    /* Get all arguments that need to go to this process */
	    _nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
	    cmd2_len = strlen(cmd2);
	    _nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
					    NEXUS_MAX_COMMAND_LENGTH-cmd2_len,
					    reply_sp);
	    
	    nexus_stdio_lock();
	    sprintf(cmd, "%s -nx_node %s %d y -nx_node_id %d -nx_checkin %d %s -nx_mpl_session %s -nx_mpl_master %s %s ",
		    _nx_get_package_id_start(),
		    node->name,
		    node->number + i,
		    node->id + i,
		    first_checkin_number + i,
		    cmd2,
		    _nx_session_string,
		    st_mpl_get_master_node_name(),
		    _nx_get_package_id_end() );
	    nexus_stdio_unlock();
	    
	    msglen = strlen(cmd) + 1;
	    mpc_bsend(cmd, msglen, node->number + i, type);
	    
	    nodes_started = NEXUS_TRUE;
	}
    }

    return(0);
} /* _nx_start_nodes() */


#ifndef HAVE_IBMDS
/*
 * st_mpl_start_context()
 */
static nexus_bool_t st_mpl_start_context(char *executable_path,
				   nexus_startpoint_t *reply_sp,
				   int checkin_number,
				   int *return_code,
				   nexus_context_t **new_local_context)
{
    /*
     * TODO: Should get rid of this fatal error, so that other
     * startup and protocol modules can work simultaneously.
     */
    nexus_fatal("st_mpl_start_context(): ERROR: Cannot create other processes with MPL\n");
    
    return (NEXUS_FALSE);
    
} /* st_mpl_start_context() */
#endif /* !HAVE_IBMDS */


/*
 * _nx_mp_acquire_node_handler()
 */
static void _nx_mp_acquire_node_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_startpoint_t reply_sp;
    nexus_endpoint_t my_fake_ep;
    int first_checkin_number;
    nexus_node_t my_fake_node;

    nexus_debug_printf(2, ("_nx_mp_acquire_node_handler(): this node being acquired\n"));

    my_fake_node = _nx_my_node;
    my_fake_node.return_code = NEXUS_NODE_OLD_DIFF_EXE;

    nexus_endpoint_init(&my_fake_ep, &mpl_ep_attr);
    nexus_startpoint_bind(&my_fake_node.sp, &my_fake_ep);

    nexus_get_startpoint(buffer, &reply_sp, 1);
    nexus_get_int(buffer, &first_checkin_number, 1);

    _nx_node_checkin( &my_fake_node, 1, &reply_sp, first_checkin_number
#ifdef BUILD_PROFILE
		      , NEXUS_TRUE
#endif
		      );

    nexus_startpoint_destroy(&reply_sp);
    nexus_startpoint_destroy(&my_fake_node.sp);
    nexus_endpoint_destroy(&my_fake_ep);
    nexus_debug_printf(2, ("_nx_mp_acquire_node_handler(): returning\n"));
} /* _nx_mp_acquire_node_handler() */
