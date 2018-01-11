/*
 * pr_iface.c
 *
 * Protocol interface routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_iface.c,v 1.77 1997/02/24 21:45:46 tuecke Exp $";

#include "internal.h"

#include <netinet/in.h>
#ifdef TARGET_ARCH_AXP
#define NX_NTOHL(m) m
#define NX_HTONL(m) m
#else
#define NX_NTOHL(m) ntohl(m)
#define NX_HTONL(m) htonl(m)
#endif

static nexus_skip_poll_callback_t *skip_poll_callback;
#ifdef BUILD_DEBUG
/* These are used by nexus_global_pointer_string() */
#define GP_N_STRINGS 10
static nexus_mutex_t gp_string_mutex;
#endif /* BUILD_DEBUG */

#ifdef BUILD_PROFILE
#define NEXUS_POLL_RANGE_CHECK
static nexus_bool_t check_poll_ranges;
static nexus_bool_t check_poll_traceback;
static double last_poll_time;
static double poll_range_min;
static double poll_range_max;
#endif


/*
 * mi_proto table stuff.
 *
 * The mi_proto table contains linked lists of nexus_mi_proto_t
 * structures.
 *
 * This table is used to avoid creating multiple nexus_mi_proto_t
 * objects that are the same.  Multiple global pointers which have
 * the same mi_proto information share a single nexus_mi_proto_t.
 */
#define MI_PROTO_TABLE_SIZE 1021
static nexus_mi_proto_t *	mi_proto_table[MI_PROTO_TABLE_SIZE];
static nexus_mutex_t		mi_proto_table_mutex;
static void			mi_proto_table_init(void);
static int			mi_proto_array_hash(nexus_byte_t *array,
						    int size);
/*
 * Other typedefs
 */
void				_nx_mi_proto_instantiate(
						nexus_mi_proto_t *mi_proto);

/*
 * Proto module list
 */
typedef struct _proto_module_list_t
{
    nexus_proto_funcs_t *		funcs;
    char *				name;
    nexus_proto_type_t			type;
    int					mi_proto_size;
    nexus_byte_t *			mi_proto_array;
    struct _proto_module_list_t *	next;
} proto_module_list_t;
static proto_module_list_t *	proto_module_list_head;
static proto_module_list_t *	proto_module_list_tail;

#define AddProtoModuleToList(Caller, Name, Funcs, Type) \
{ \
    proto_module_list_t *__p; \
    NexusMalloc(Caller, __p, proto_module_list_t *, \
		sizeof(proto_module_list_t)); \
    __p->name = _nx_copy_string(Name); \
    __p->funcs = (Funcs); \
    __p->type = (Type); \
    __p->mi_proto_size = 0; \
    __p->mi_proto_array = (nexus_byte_t *) NULL; \
    __p->next = (proto_module_list_t *) NULL; \
    if (proto_module_list_head) \
    { \
	proto_module_list_tail->next = __p; \
	proto_module_list_tail = __p; \
    } \
    else \
    { \
	proto_module_list_head = proto_module_list_tail = __p; \
    } \
}

static int			n_protos_with_poll;
static proto_module_list_t *	proto_module_with_poll;
static nexus_proto_t *		local_proto;


/*
 * saved proto arguments
 */
static char saved_no_proto_arg[256];

/*
 * _nx_proto_usage_message()
 *
 * Call the usage_message() function for each protocol
 */
void _nx_proto_usage_message(void)
{
#ifdef NEXUS_POLL_RANGE_CHECK    
    printf("    -poll_check <min> <max>   : Print a notice if the time between\n");
    printf("                                nexus_poll() calls does not fall in the\n");
    printf("                                supplied range. (<min> and <max> are reals)\n");
    printf("    -poll_trace               : Print stack tracebacks with\n");
    printf("                                poll_check messages\n");
#endif /* NEXUS_POLL_RANGE_CHECK */	
} /* _nx_proto_usage_message() */


/*
 * _nx_proto_new_process_params()
 *
 * Call the new_process_params() function for each protocol
 *
 * Each of those functions may add stuff to 'buf', returning the number
 * of characters that they added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int _nx_proto_new_process_params(char *buf, int size)
{
    char tmp_buf[256];
    int n_left = size;
    char *b = buf;
    int n_added;

    n_added = 0;
    tmp_buf[0] = '\0';

    nexus_stdio_lock();

    if (strlen(saved_no_proto_arg) > 0)
    {
	char tmp_buf2[256];

	sprintf(tmp_buf2, "-no_pr %s ", saved_no_proto_arg);
	strcat(tmp_buf, tmp_buf2);
    }
#ifdef NEXUS_POLL_RANGE_CHECK
    if (check_poll_ranges)
    {
	char tmp_buf2[256];

	sprintf(tmp_buf2, "-poll_check %f %f ",
		poll_range_min, poll_range_max);
	if (check_poll_traceback)
	{
	    strcat(tmp_buf2, "-poll_trace ");
	}
	strcat(tmp_buf, tmp_buf2);
    }
#endif /* NEXUS_POLL_RANGE_CHECK */	
    n_added = strlen(tmp_buf);
    if (n_added > n_left)
    {
        nexus_fatal("_nx_proto_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    strcpy(b, tmp_buf);
    b += n_added;
    n_left -= n_added;

    nexus_stdio_unlock();

    return (size - n_left);
} /* _nx_proto_new_process_params() */


/*
 * _nx_proto_init()
 *
 * Initialize the protocol modules.
 *
 * TODO: This needs to be made more dynamic
 */
void _nx_proto_init(int *argc, char ***argv,
		    nexus_module_list_t module_list[])
{
    int i;
    int rc;
    nexus_proto_funcs_t *proto_funcs;
    nexus_proto_type_t proto_type;
    proto_module_list_t *proto_module;
    nexus_byte_t *array;
    int size;
    int node_name_length;
    int arg_num;
    char *arg;
    char *next, *no_proto;
    nexus_bool_t add_proto;
    int pid;

    /*
     * Parse any arguments
     */
    if ((arg_num = nexus_find_argument(argc, argv, "no_pr", 2)) >= 0)
    {
	arg = (*argv)[arg_num + 1];
	strcpy(saved_no_proto_arg, arg);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    
#ifdef NEXUS_POLL_RANGE_CHECK
    check_poll_ranges = NEXUS_FALSE;
    check_poll_traceback = NEXUS_FALSE;
    last_poll_time = -1.0;
    if ((arg_num = nexus_find_argument(argc, argv, "poll_check", 3)) >= 0)
    {
	check_poll_ranges = NEXUS_TRUE;
	poll_range_min = atof((*argv)[arg_num + 1]);
	poll_range_max = atof((*argv)[arg_num + 2]);
	nexus_remove_arguments(argc, argv, arg_num, 3);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "poll_trace", 1)) >= 0)
    {
	check_poll_traceback = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
#endif /* NEXUS_POLL_RANGE_CHECK */

#ifdef BUILD_DEBUG
    nexus_mutex_init(&gp_string_mutex, (nexus_mutexattr_t *) NULL);
#endif

    /*
     * Scan the module_list looking for protocol modules.
     * For each of these, get the function table,
     * and add that module to the proto_module_list.
     * Put the local protocol module at the front of the list.
     */
    proto_module_list_head = proto_module_list_tail
	= (proto_module_list_t *) NULL;
    proto_funcs = _nx_pr_local_info();
    proto_type = (*proto_funcs->proto_type)();
    AddProtoModuleToList(_nx_proto_init(),
			 "local",
			 proto_funcs,
			 proto_type);

    for (i = 0; module_list[i].family_name != (char *) NULL; i++)
    {
	if ((strcmp(module_list[i].family_name, "protocols") == 0))
	{
	    add_proto = NEXUS_TRUE;
	    next = saved_no_proto_arg;
	    /* 
	     * This can be changed to while(next && add_proto) if the
	     * number of no_pr options increase.  With only tcp, atm,
	     * and inx, this is not an optimization.
	     */
	    while (next)
	    {
		_nx_get_next_value(next, ':', &next, &no_proto);
		if (strcmp(module_list[i].module_name, no_proto) == 0)
		{
		    add_proto = NEXUS_FALSE;
		}
	    }
	    /*
	    rc = _nx_module_load(module_list[i].family_name,
				 module_list[i].module_name,
				 (void *) &proto_funcs);
	    */
	    rc = 0;
	    if (rc == 0 && add_proto)
	    {
	        proto_funcs
		    = (nexus_proto_funcs_t *) (*module_list[i].info_func)();
		proto_type = (*proto_funcs->proto_type)();
		AddProtoModuleToList(_nx_proto_init(),
				     module_list[i].module_name,
				     proto_funcs,
				     proto_type);
	    }
	}
    }

    /*
     * Initialize each protocol module
     */
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	(*proto_module->funcs->init)(argc, argv);
    }

    /*
     * Find out how many of the protos have poll functions.
     */
    n_protos_with_poll = 0;
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->funcs->poll)
	{
	    n_protos_with_poll++;
	    proto_module_with_poll = proto_module;
	}
    }

    /*
     * Get the nexus_proto_t for the local protocol module
     * and fill it into _nx_my_mi_proto.
     */
    (proto_module_list_head->funcs->construct_from_mi_proto)(&local_proto,
						(nexus_mi_proto_t *) NULL,
						(nexus_byte_t *) NULL,
						0);

    /*
     * Construct my nexus_mi_proto_t.
     * First ask each protocol module what size its component will be,
     * then malloc an array big enough, and then ask
     * then ask each protocol module to fill their component into
     * the array.
     */
    mi_proto_table_init();
    node_name_length = strlen(_nx_my_node.name);
    size = 8 + node_name_length + 1;
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->funcs->get_my_mi_proto)
	{
	    (proto_module->funcs->get_my_mi_proto)(
					      &(proto_module->mi_proto_array),
					      &(proto_module->mi_proto_size) );
	    size += 4 + proto_module->mi_proto_size;
	}
    }
    _nx_my_mi_proto = _nx_mi_proto_create(size, (nexus_byte_t *) NULL, 
			local_proto);
    array = _nx_my_mi_proto->array;
    i = 0;
    PackInt4(array, i, _nx_my_node.number);
    pid = _nx_md_getpid();  /* TODO: Generalize this context differentiator */
    PackInt4(array, i, pid);
    memcpy(&(array[i]), _nx_my_node.name, node_name_length + 1);
    i += node_name_length + 1;
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->mi_proto_size > 0)
	{
	    PackMIProtoEntry(array, i,
			     proto_module->type,
			     proto_module->mi_proto_size,
			     proto_module->mi_proto_array);
	}
    }
    _nx_mi_proto_table_insert(_nx_my_mi_proto);

} /* _nx_proto_init() */


/*
 * _nx_proto_shutdown()
 *
 * If 'shutdown_others' is NEXUS_TRUE,
 * then try to shutdown all other nodes and contexts.
 */
void _nx_proto_shutdown(nexus_bool_t shutdown_others)
{
    proto_module_list_t *proto_module;
    
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->funcs->shutdown)
	{
	    (proto_module->funcs->shutdown)(shutdown_others);
	}
    }
} /* _nx_proto_shutdown() */


/*
 * _nx_proto_abort()
 */
void _nx_proto_abort(void)
{
    proto_module_list_t *proto_module;
    
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->funcs->abort)
	{
	    (proto_module->funcs->abort)();
	}
    }
} /* _nx_proto_abort() */

/*
 * nexus_register_skip_poll_callback
 *
 *  Get a function from the user to call from each protocol poll module
 *  to allow for adaptive polling.
 */
void nexus_register_skip_poll_callback(nexus_skip_poll_callback_t *func)
{
    skip_poll_callback = func;
}

/*
 * nexus_startpoint_test()
 */
int nexus_startpoint_test(nexus_startpoint_t *startpoint)
{
    /*
     * TODO: This needs to be implemented as a round-trip
     */
    return 0;
} /* nexus_startpoint_test() */

/*
 * nexus_poll
 *
 * Poll each proto, so that they can handle any outstanding remote
 * service requests.
 *
 * No need to poll the local protocol.
 */
void nexus_poll(void)
{
    proto_module_list_t *proto_module;
    nexus_bool_t message_handled;

    nexus_fd_handle_events(NEXUS_FD_POLL_NONBLOCKING_ALL, (int *) NULL);
    for (proto_module = proto_module_list_head;
	 proto_module;
	 proto_module = proto_module->next)
    {
	if (proto_module->funcs->poll)
	{
	    message_handled = (proto_module->funcs->poll)();
	    if (skip_poll_callback)
	    {
		(*skip_poll_callback)(&_nx_skip_poll_count,
				      proto_module->type,
				      message_handled);
	    }
	}
    }
} /* nexus_poll() */


/*
 * nexus_poll_profile
 *
 * Poll each proto, so that they can handle any outstanding remote
 * service requests.
 *
 * No need to poll the local protocol.
 */
void nexus_poll_profile(int id)
{
    
#ifdef NEXUS_POLL_RANGE_CHECK
    if (check_poll_ranges && last_poll_time >= 0)
    {
	double time_since_last_poll;
	time_since_last_poll = nexus_wallclock() - last_poll_time;
	if (time_since_last_poll < poll_range_min)
	{
	    nexus_notice("poll id %d: Time since last nexus_poll() (%f) is less than minimum\n",
			 id, time_since_last_poll);
	    if (check_poll_traceback)
	    {
		_nx_traceback();
	    }
	}
	else if (time_since_last_poll > poll_range_max)
	{
	    nexus_notice("poll id %d: Time since last nexus_poll() (%f) exceeds maximum\n",
			 id, time_since_last_poll);
	    if (check_poll_traceback)
	    {
		_nx_traceback();
	    }
	}
    }
#endif /* NEXUS_POLL_RANGE_CHECK */    

    nexus_poll();
    
#ifdef NEXUS_POLL_RANGE_CHECK
    if (check_poll_ranges)
    {
	last_poll_time = nexus_wallclock();
    }
#endif /* NEXUS_POLL_RANGE_CHECK */    
    
} /* nexus_poll_profile() */


/*
 * nexus_poll_blocking()
 *
 * If only one proto has a poll routine, then call
 * its poll_blocking() routine.
 * If more than one proto has a poll routine, then call
 * each of those poll() routines.
 *
 * No need to poll the local protocol.
 */
void nexus_poll_blocking(void)
{
    if (n_protos_with_poll == 0)
    {
	nexus_fd_handle_events(NEXUS_FD_POLL_BLOCKING_ALL, (int *) NULL);
    }
    else if (n_protos_with_poll > 0)
    {
	nexus_poll();
    }
} /* nexus_poll_blocking() */


/*
 * _nx_proto_get_creator_gp_params()
 *
 * Fill in 'buf', up to 'buf_size' bytes, with a command line
 * argument encoding * of the passed global pointer, 'gp', so
 * that a new process can connect to this one.
 */
void _nx_proto_get_creator_sp_params(char *buf,
				     int buf_size,
				     nexus_startpoint_t *sp)
{
    int len;
    char tmp_buf[64];
#ifdef BUILD_PROFILE
    int my_node_id, my_context_id;
#endif /* BUILD_PROFILE */

    /*
     * Put parameters into 'buf' to specify:
     *   -nf_nx_sp_id <context_id>:<node_id>	id's for profiling
     *   -nf_nx_sp_liba <hex_encoded_liba>	startpoint's liba
     *   -nf_nx_sp_mi <hex_encoded_mi_proto>	startpoint's mi_proto
     */

#ifdef BUILD_PROFILE
    _nx_node_id(&my_node_id);
    _nx_context_id(&my_context_id);
    nexus_stdio_lock();
    sprintf(tmp_buf, "-nf_nx_sp_id :%d:%d ", my_context_id, my_node_id);
    nexus_stdio_unlock();
#else  /* BUILD_PROFILE */
    tmp_buf[0] = '\0';
#endif /* BUILD_PROFILE */

    if ((size_t) buf_size < (100  /* strings + fudging */
			     + strlen(tmp_buf)
			     + (2*sp->liba_size)
			     + (2*sp->mi_proto->size) ))
    {
	nexus_fatal("_nx_proto_get_creator_sp_params(): Internal error: Buffer not big enough for arguments\n");
    }

    nexus_stdio_lock();
    sprintf(buf, "%s-nf_nx_sp_liba ",
	    tmp_buf);
    nexus_stdio_unlock();
    len = strlen(buf);
    _nx_hex_encode_byte_array((sp->liba_is_inline
			       ? sp->liba.array
			       : sp->liba.pointer),
			      sp->liba_size,
			      &(buf[len]) );
    len += (2 * sp->liba_size);

    memcpy(&(buf[len]), " -nf_nx_sp_mi ", 14);	/* without null terminator */
    len += 14;
    _nx_hex_encode_byte_array(sp->mi_proto->array,
			      sp->mi_proto->size,
			      &(buf[len]) );
    len += (2 * sp->mi_proto->size);

    memcpy(&(buf[len]), " ", 2);		/* with null terminator */
} /* _nx_proto_get_creator_sp_params */


/*
 * _nx_proto_construct_creator_sp()
 */
void _nx_proto_construct_creator_sp(nexus_startpoint_t *creator_sp)
{
    char *string;
    nexus_mi_proto_t *mi_proto;
    int size;
    int arg_num;
    int *argc;
    char ***argv;

    nexus_get_retained_arguments(&argc, &argv);
    
    if ((arg_num = nexus_find_argument(argc, argv, "nf_nx_sp_liba", 2)) >= 0)
    {
	string = (*argv)[arg_num + 1];
	nexus_remove_arguments(argc, argv, arg_num, 2);

	creator_sp->liba_size = strlen(string) / 2;
	if (creator_sp->liba_size <= NEXUS_DEFAULT_LIBA_SIZE)
	{
	    creator_sp->liba_is_inline = 1;
	    _nx_hex_decode_byte_array(string,
				      creator_sp->liba_size,
				      creator_sp->liba.array);
	}
	else
	{
	    creator_sp->liba_is_inline = 0;
	    NexusMalloc(_nx_proto_construct_creator_sp(),
			creator_sp->liba.pointer,
			nexus_byte_t *,
			creator_sp->liba_size);
	    _nx_hex_decode_byte_array(string,
				      creator_sp->liba_size,
				      creator_sp->liba.pointer);
	}
    }
    else
    {
	nexus_fatal("_nx_proto_construct_creator_sp(): Internal error: Expected a -nf_nx_sp_liba argument.\n");
    }

    if ((arg_num = nexus_find_argument(argc, argv, "nf_nx_sp_mi", 2)) >= 0)
    {
	string = (*argv)[arg_num + 1];
	nexus_remove_arguments(argc, argv, arg_num, 2);

	size = strlen(string) / 2;
	mi_proto = _nx_mi_proto_create(size,
				   (nexus_byte_t *) NULL,
				   (nexus_proto_t *) NULL);
	mi_proto->size = size;
	_nx_hex_decode_byte_array(string,
				  mi_proto->size,
				  mi_proto->array);
	creator_sp->mi_proto = _nx_mi_proto_table_insert(mi_proto);
    }
    else
    {
	nexus_fatal("_nx_proto_construct_creator_sp(): Internal error: Expected a -nf_nx_sp_liba argument.\n");
    }

#ifdef BUILD_PROFILE    
    /*
     * Get the startpoint context and address
     */
    if ((arg_num = nexus_find_argument(argc, argv, "nf_nx_sp_id", 2)) >= 0)
    {
	int rc;
	string = (*argv)[arg_num + 1];
	nexus_remove_arguments(argc, argv, arg_num, 2);
	
	nexus_stdio_lock();
	rc = sscanf(string, ":%d:%d",
		    &(creator_sp->context_id),
		    &(creator_sp->node_id));
	nexus_stdio_unlock();
	if (rc != 2)
	{
	    nexus_fatal("_nx_proto_construct_creator_sp(): Internal error: Invalid -nf_nx_sp_id argument.\n");
	}
    }
    else
    {
	creator_sp->context_id = -1;
	creator_sp->node_id = -1;
	nexus_debug_printf(1, ("_nx_proto_construct_creator_sp(): No -nf_nx_sp_id argument.\n"));
    }
#endif /* BUILD_PROFILE */
    
} /* _nx_proto_construct_creator_sp() */


/*
 * nexus_test_startpoint()
 *
 * Return 0 if the passed startpoint is (probably) still working,
 * else return non-0.
 *
 * If the protocol module for this sp does not implement the test_proto
 * function, then return 0.
 */
int nexus_test_startpoint(nexus_startpoint_t *sp)
{
    int rc;
    if (   (sp)
	&& (sp->mi_proto)
	&& (sp->mi_proto->proto->funcs->test_proto))
    {
	rc = (sp->mi_proto->proto->funcs->test_proto)(sp->mi_proto->proto);
    }
    else
    {
	rc = 0;
    }
    return(rc);
} /* nexus_test_startpoint() */


/*
 * _nx_mi_proto_create()
 *
 * Return an mi_proto for the passed 'size, 'array', and 'proto'.
 * If array==NULL, then do not try to copy it.  It will be filled in later.
 */
nexus_mi_proto_t *_nx_mi_proto_create(int size,
				      nexus_byte_t *array,
				      nexus_proto_t *proto)
{
    nexus_mi_proto_t *new_mi_proto;

    NexusMalloc(_nx_mi_proto_create(),
		new_mi_proto,
		nexus_mi_proto_t *,
		(sizeof(nexus_mi_proto_t) + size));
    new_mi_proto->proto = proto;
    new_mi_proto->next = (nexus_mi_proto_t *) NULL;
    new_mi_proto->size = size;
    new_mi_proto->array = (nexus_byte_t *) (((char *) new_mi_proto)
					    + sizeof(nexus_mi_proto_t));
    if (array)
    {
	memcpy(new_mi_proto->array, array, size);
    }
    
    return (new_mi_proto);
    
} /* _nx_mi_proto_create() */


/*
 * _nx_mi_proto_destroy()
 */
void _nx_mi_proto_destroy(nexus_mi_proto_t *mi_proto)
{
    if (mi_proto->proto->funcs->decrement_reference_count)
    {
	if ((mi_proto->proto->funcs->decrement_reference_count)(
	                                                  mi_proto->proto))
	{
	    /* The proto has been freed */
	    mi_proto->proto = (nexus_proto_t *) NULL;
	}
    }
} /* _nx_mi_proto_destroy() */


/*
 * _nx_mi_proto_instantiate()
 *
 * Go through the entries in the mi_proto's array, looking for ones
 * that we can use.  When one is found, instantiate it into
 * a nexus_proto_t object.
 */
void _nx_mi_proto_instantiate(nexus_mi_proto_t *mi_proto)
{
    proto_module_list_t *proto_module;
    nexus_proto_t *proto;
    nexus_byte_t *a;
    nexus_byte_t *proto_array;
    int i;
    nexus_proto_type_t type;
    int size;
    nexus_bool_t done;
    int node_number;
    int context_differentiator;
    char *node_name;

    a = mi_proto->array;
    i = 0;

    NexusAssert2(((i + 9) <= mi_proto->size),
		 ("_nx_mi_proto_instantiate(): Invalid mi_proto array.  Not enough room for header.\n"));
    
    UnpackMIProtoHeader(a, i,
			node_number,
			context_differentiator,
			node_name);
    
    done = NEXUS_FALSE;
    while (!done && (i < mi_proto->size))
    {
	NexusAssert2(((i + 4) <= mi_proto->size),
		     ("_nx_mi_proto_instantiate(): Invalid mi_proto array.  Not enough room for protocol entry header.\n"));

	UnpackMIProtoEntry(a, i, type, size, proto_array);

	NexusAssert2((i <= mi_proto->size),
		     ("_nx_mi_proto_instantiate(): Invalid mi_proto array.  Not enough room for protocol entry.\n"));

	/* See if this protocol is in my proto_module_list */
	for (proto_module = proto_module_list_head;
	     !done && proto_module;
	     proto_module = proto_module->next)
	{
	    if (proto_module->type == type)
	    {
		/* Found a type match.  So call the proto to instantiate it */
		if ((proto_module->funcs->construct_from_mi_proto)(&proto,
								   mi_proto,
								   proto_array,
								   size))
		{
		    /* Successful instantiation */
		    if (!proto)
		    {
			/* Use local_proto */
			proto = local_proto;
		    }
		    mi_proto->proto = proto;
		    done = NEXUS_TRUE;
		}
	    }
	}
    }
    
    NexusAssert2((mi_proto->proto != (nexus_proto_t *) NULL),
		 ("_nx_mi_proto_instantiate(): Could not find a usable protocol.\n"));

} /* _nx_mi_proto_instantiate() */


/*
 * mi_proto_table_init()
 *
 * Initialize the mi_proto hash table.
 */
static void mi_proto_table_init(void)
{
    int i;
    for (i = 0; i < MI_PROTO_TABLE_SIZE; i++)
    {
	mi_proto_table[i] = (nexus_mi_proto_t *) NULL;
    }
    nexus_mutex_init(&mi_proto_table_mutex, (nexus_mutexattr_t *) NULL);
} /* mi_proto_table_init() */


/*
 * mi_proto_array_hash()
 *
 * Hash the passed mi_proto 'array'.
 */
static int mi_proto_array_hash(nexus_byte_t *array, int size)
{
    unsigned long hval = 0;
    int i;

    for (i = 0; i < size; i++)
    {
	hval <<= 1;
	hval += (unsigned long) array[i];
    }
    return ((int) (hval % MI_PROTO_TABLE_SIZE));
} /* mi_proto_array_hash() */


/*
 * _nx_mi_proto_table_insert()
 *
 * If 'new_mi_proto' already exists in the mi_proto_table,
 * then free it and return the existing copy.
 * Otherwise, add new_mi_proto to the table and return it.
 */
nexus_mi_proto_t *_nx_mi_proto_table_insert(nexus_mi_proto_t *new_mi_proto)
{
    int bucket;
    nexus_mi_proto_t *mi_proto;

    bucket = mi_proto_array_hash(new_mi_proto->array, new_mi_proto->size);

    /* Search the bucket for this mi_proto */
    for (mi_proto = mi_proto_table[bucket];
	 mi_proto != (nexus_mi_proto_t *) NULL;
	 mi_proto = mi_proto->next)
    {
	if (   (new_mi_proto->size == mi_proto->size)
	    && (memcmp(new_mi_proto->array, mi_proto->array,
		       new_mi_proto->size) == 0) )
	{
	    /*
	     * Found mi_proto with this array in the table.
	     * So use the existing one, and free the new one.
	     * Bump the reference count of the proto for the existing one.
	     */
	    NexusFree(new_mi_proto);
	    if (mi_proto->proto)
	    {
		if (mi_proto->proto->funcs->increment_reference_count)
		{
		    (mi_proto->proto->funcs->increment_reference_count)(
							mi_proto->proto);
		}
	    }
	    else
	    {
		/*
		 * This mi_proto is in the table, but its proto has
		 * be freed.  So re-instantiate it.
		 */
		_nx_mi_proto_instantiate(mi_proto);
	    }
	    return(mi_proto);
	}
    }
    
    /*
     * We did not find new_mi_proto in the table.
     * So add this new one to this hash bucket.
     */
    new_mi_proto->next = mi_proto_table[bucket];
    mi_proto_table[bucket] = new_mi_proto;

    if (new_mi_proto->proto == (nexus_proto_t *) NULL)
    {
	_nx_mi_proto_instantiate(new_mi_proto);
    }

    return (new_mi_proto);
    
} /* _nx_mi_proto_table_insert() */


