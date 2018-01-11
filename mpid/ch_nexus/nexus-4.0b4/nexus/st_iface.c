/*
 * st_iface.c
 *
 * Startup module interface routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_iface.c,v 1.81 1997/02/14 02:57:08 tuecke Exp $";

#include "internal.h"
#include <pwd.h>

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static void finish_node_list_init(nexus_startup_node_t *node_list);
static void finish_node_init(nexus_startup_node_t *node);
static nexus_startup_node_t *node_list_parse(char *input_string,
					     FILE *input_fp);

/*
 * Startup module list
 */
typedef struct _startup_module_list_t
{
    char *				name;
    nexus_startup_funcs_t *		funcs;
    struct _startup_module_list_t *	next;
} startup_module_list_t;
static startup_module_list_t *	startup_module_list_head;
static startup_module_list_t *	startup_module_list_tail;

#define AddStartupModuleToList(Caller, Name, Funcs) \
{ \
    startup_module_list_t *__s; \
    NexusMalloc(Caller, __s, startup_module_list_t *, \
		sizeof(startup_module_list_t)); \
    __s->name = _nx_copy_string(Name); \
    __s->funcs = (Funcs); \
    __s->next = (startup_module_list_t *) NULL; \
    if (startup_module_list_head) \
    { \
	startup_module_list_tail->next = __s; \
	startup_module_list_tail = __s; \
    } \
    else \
    { \
	startup_module_list_head = startup_module_list_tail = __s; \
    } \
}


/*
 * Command line arguments
 */
static char *		arg_nodes_list;
static char *		arg_startup_file;
static int		arg_n_nodes;
static nexus_bool_t	arg_dont_duplicate_master_node;
char *			_nx_debug_command;
char *			_nx_debug_display;

/*
 * Initial nodes list
 */
static nexus_startup_node_t *	initial_nodes_list_head;
static nexus_startup_node_t *	initial_nodes_list_tail;


static void start_node_list(nexus_startup_node_t *node_list,
			    nexus_bool_t leave_room_for_master,
			    nexus_node_t **nodes,
			    int *n_nodes);
static nexus_new_nodes_monitor_t *wait_for_new_nodes_init(nexus_node_t *nodes,
							  int n_nodes);
static void wait_for_new_nodes(nexus_new_nodes_monitor_t *new_nodes_monitor);

static void _nx_new_node_checkin_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static nexus_handler_t startup_handlers[] =
{ \
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) _nx_new_node_checkin_handler},
};


/*
 * _nx_startup_usage_message()
 *
 * Call the usage_message() function for each protocol
 */
void _nx_startup_usage_message(void)
{
    printf("    -nodes <nodelist>         : Start on these nodes.  <nodelist> is a colon\n");
    printf("                                separated list of nodes, where each node\n");
    printf("                                is <machine_name>[#<node_number>][,<count>]\n");
    printf("    -s <file>                 : Use this startup file\n");
    printf("    -n <integer>              : Start <integer> nodes on this machine\n");
    printf("    -debug_command <command>  : Intercept context creation calling <command>\n");
    printf("                              : and passing the following environment\n");
    printf("                              : variables:\n");
    printf("                              : NEXUS_DEBUG_CONTEXT - context executable\n");
    printf("                              : NEXUS_DEBUG_ARGS - arguments to the context\n");
    printf("                              : NEXUS_DEBUG_DISPLAY - host x-windows display\n");
    printf("    -debug_display <display>  : Set the NEXUS_DEBUG_DISPLAY environment\n");
    printf("                              : variable to <display>\n");
} /* _nx_startup_usage_message() */


/*
 * _nx_startup_new_process_params()
 *
 * Call the new_process_params() function for each protocol
 *
 * Each of those functions may add stuff to 'buf', returning the number
 * of characters that they added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int _nx_startup_new_process_params(char *buf, int size)
{
    char tmp_buf1[1024];
    char tmp_buf2[1024];
    int n_added;
    
    tmp_buf1[0] = '\0';

    if ( _nx_debug_command )
    {
	nexus_stdio_lock();
	sprintf(tmp_buf2, "-debug_command %s ", _nx_debug_command);
	nexus_stdio_unlock();
	strcat(tmp_buf1, tmp_buf2);
    }
    if ( _nx_debug_display )
    {
	nexus_stdio_lock();
	sprintf(tmp_buf2, "-debug_display %s ", _nx_debug_display);
	nexus_stdio_unlock();
	strcat(tmp_buf1, tmp_buf2);
    }
    
    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
	nexus_fatal("_nx_startup_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    
    strcpy(buf, tmp_buf1);

    return (n_added);
    
} /* _nx_startup_new_process_params() */


/*
 * _nx_startup_modify_command_for_type()
 */
void _nx_startup_modify_command_for_type(nexus_startup_node_t *node,
					 char *cmd,
					 int max_cmd_length,
					 char ***environment,
					 int *n_env)
{
    char *type;
    char *tmp;
    char *s;

    *environment = NULL;
    *n_env = 0;
    
    type = nexus_rdb_lookup(node->name, "startup_type");
    if (type)
    {
        if (strcmp(type, "mpl") == 0)
        {
	    char tmp_env[512];
	    char *hostfile;
	    char *cmdfile;
	    char *pgmmodel;
	    int env_pos = 0;

	    NexusMalloc(_nx_startup_modify_command_for_type(),
		        tmp,
		        char *,
		        sizeof(char) * max_cmd_length);

	    /*
	     * go through cmd and change "-nx_node %s %d n" to
	     * "-nx_node %s %d y"
	     */
	    if ((s = strstr(cmd, "-nx_node")) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Expected -nx_node argument\n");
	    }
	    if ((s = strchr(s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    if ((s = strchr(++s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    if ((s = strchr(++s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    s++;
	    NexusAssert2(((*s == 'n') || (*s == 'y')),
			 ("_nx_startup_modify_command_for_type(): Invalid command.  Last -nx_node argument should be 'n' or 'y'.\n"));
	    *s = 'y';

	    /*
	     * Add some arguments to the command line
	     */
	    nexus_stdio_lock();
    	    sprintf(tmp, "%s %s -nx_num_nodes %d -nx_mpl_master %s %s",
		    cmd,
		    _nx_get_package_id_start(),
		    node->count_other_nodes + 1,
		    node->name,
		    _nx_get_package_id_end());
	    nexus_stdio_unlock();

	    /*
	     * Setup the necessary environment variables
	     */
	    hostfile = nexus_rdb_lookup(node->name, "startup_mpl_hostfile");
	    if (hostfile == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): startup_type=mpl must be accompanied by startup_mpl_hostfile= attribute\n");
	    }

	    /* 
      	     * From the POE man page:
             * 
             *   MP_CMDFILE Determines the name of a POE commands file used to
             *              load the nodes of your partition.  If set, POE
             *              will read the commands file rather than STDIN.
             *              Valid values are any file specifier.
             */
	    cmdfile = nexus_rdb_lookup(node->name, "startup_mpl_cmdfile");
	    if (cmdfile != (char *) NULL)
	    {
		(*n_env)++;
	    }

	    /* 
      	     * From the POE man page:
             *
             *  MP_PGMMODEL Determines the programming model you are using.
             *              Valid values are spmd or mpmd.  If not set, the
             *              default is spmd.
             *
             * I believe this must be set to MPMD for a computation with
             * multiple executables to work correctly.
             */
	    pgmmodel = nexus_rdb_lookup(node->name, "startup_mpl_pgmmodel");
	    if (pgmmodel != (char *) NULL)
	    {
	        (*n_env)++;
	    }
	    *n_env += 5;
	    NexusMalloc(_nx_startup_modify_command_for_type,
			(*environment),
			char **,
			sizeof (char *) * (*n_env));
	    nexus_stdio_lock();
	    sprintf(tmp_env, "MP_HOSTFILE=%s", hostfile);
	    (*environment)[env_pos++] = _nx_copy_string(tmp_env);
	    (*environment)[env_pos++] = _nx_copy_string("MP_EUILIB=us");
	    sprintf(tmp_env, "MP_PROCS=%d", node->count_other_nodes+1);
	    (*environment)[env_pos++] = _nx_copy_string(tmp_env);
	    (*environment)[env_pos++] = _nx_copy_string("MP_INFOLEVEL=0");
	    (*environment)[env_pos++] = _nx_copy_string("MP_PULSE=0");
	    /*
	    (*environment)[env_pos++] = _nx_copy_string("NLSPATH=/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/prime/%N");
	     */
	    if (cmdfile != (char *) NULL)
	    {
		sprintf(tmp_env, "MP_CMDFILE=%s", cmdfile);
		(*environment)[env_pos++] = _nx_copy_string(tmp_env);
		nexus_rdb_free(cmdfile);
	    }
	    if (pgmmodel != (char *) NULL)
	    {
		sprintf(tmp_env, "MP_PGMMODEL=%s", pgmmodel);
		(*environment)[env_pos++] = _nx_copy_string(tmp_env);
		nexus_rdb_free(pgmmodel);
	    }
	    nexus_stdio_unlock();
	    nexus_rdb_free(hostfile);

	    strcpy(cmd, tmp);
	    NexusFree(tmp);
	    
	    s = nexus_rdb_lookup(node->name, "startup_mpl_hostname");
	    if (s)
	    {
		NexusFree(node->name);
		node->name = _nx_copy_string(s);
		nexus_rdb_free(s);
	    }
        }
        else if (strcmp(type, "network") == 0)
        {
	    char *net_nodes;
	    char *other_nodes;
    
	    NexusMalloc(_nx_startup_modify_command_for_type(),
		        tmp,
		        char *,
		        sizeof(char) * max_cmd_length);
    
	    net_nodes = nexus_rdb_lookup(node->name, "startup_network_nodes");

	    /*
	     * Extract net_nodes into 2 parts:
	     *
	     * 1. Initial node to contact (put into node->name)
	     * 2. Other nodes left over
	     */
	    _nx_get_next_value(net_nodes,
	    		       ':',
			       &other_nodes,
			       &(node->name));
	    
	    nexus_stdio_lock();
	    sprintf(tmp, "%s %s -nx_num_nodes %d -nodes %s %s",
		    cmd,
		    _nx_get_package_id_start(),
		    node->count_other_nodes+1,
		    other_nodes,
		    _nx_get_package_id_end());
	    nexus_stdio_unlock();
    
	    strcpy(cmd, tmp);
	    NexusFree(tmp);
	    
	    NexusMalloc(_nx_startup_modify_command_for_type,
			(*environment),
			char **,
			sizeof (char *));
	    (*environment)[0] = NULL;
	    *n_env = 0;

	    nexus_rdb_free(net_nodes);
        }
        else if (strcmp(type, "paragon") == 0)
        {
	    NexusMalloc(_nx_startup_modify_command_for_type(),
		        tmp,
		        char *,
		        sizeof(char) * max_cmd_length);

	    /*
	     * go through cmd and change "-nx_node %s %d n" to
	     * "-nx_node %s %d y"
	     */
	    if ((s = strstr(cmd, "-nx_node")) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Expected -nx_node argument\n");
	    }
	    if ((s = strchr(s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    if ((s = strchr(++s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    if ((s = strchr(++s, ' ')) == (char *) NULL)
	    {
		nexus_fatal("_nx_startup_modify_command_for_type(): Invalid command.  -nx_node argument is munged.\n");
	    }
	    s++;
	    NexusAssert2(((*s == 'n') || (*s == 'y')),
			 ("_nx_startup_modify_command_for_type(): Invalid command.  Last -nx_node argument should be 'n' or 'y'.\n"));
	    *s = 'y';

	    /*
	     * Figure out what the -pn argument should be.
	     * Default to "open".
	     */
	    s = nexus_rdb_lookup(node->name, "startup_paragon_pn");

	    /*
	     * Add some arguments to the command line
	     */
	    nexus_stdio_lock();
    	    sprintf(tmp, "%s -sz %d -pn %s %s -nx_num_nodes %d -nx_inx_master %s %s",
		    cmd,
		    node->count_other_nodes + 1,
		    (s ? s : "open"),
		    _nx_get_package_id_start(),
		    node->count_other_nodes + 1,
		    node->name,
		    _nx_get_package_id_end());
	    nexus_stdio_unlock();
	    
	    strcpy(cmd, tmp);
	    NexusFree(tmp);

	    nexus_rdb_free(s);
	    
	    s = nexus_rdb_lookup(node->name, "startup_paragon_hostname");
	    if (s)
	    {
		NexusFree(node->name);
		node->name = _nx_copy_string(s);
		nexus_rdb_free(s);
	    }
        }
        else
        {
	    nexus_fatal("unrecognized startup type: \"%s\"\n", type);
        }
    }
    nexus_rdb_free(type);
} /* _nx_startup_modify_command_for_type() */


/*
 * _nx_startup_preinit()
 *
 * This is called immediately upon entering nexus_init().
 * It can be useful for bootstrapping, for example, to 
 * fill in _nx_my_process_params.
 *
 * There is a chicken and egg problem, however.  This is called
 * before any argument handling.  Therefore, we cannot use any
 * arguments to tell us which startup module to use.  So we just
 * call the preinit routine in each of the preloaded modules.
 *
 * Return: NEXUS_TRUE if command line arguments should be ignored.
 */
nexus_bool_t _nx_startup_preinit(nexus_module_list_t module_list[])
{
    nexus_bool_t ignore_command_line_args = NEXUS_FALSE;
    int i;
    int rc;
    nexus_startup_funcs_t *startup_funcs;
    nexus_startup_node_t *nl_head, *nl_tail;
    startup_module_list_t *startup_module;

    /*
     * Scan the module_list looking for startup modules.
     * For each of these, get the function table,
     * and add that module to the startup_module_list.
     */
    startup_module_list_head = startup_module_list_tail
	= (startup_module_list_t *) NULL;
    for (i = 0; module_list[i].family_name != (char *) NULL; i++)
    {
	if (strcmp(module_list[i].family_name, "startups") == 0)
	{
	    /*
	    rc = _nx_module_load(module_list[i].family_name,
				 module_list[i].module_name,
				 (void *) &startup_funcs);
	    */
	    startup_funcs = (nexus_startup_funcs_t *)
		(*module_list[i].info_func)();
	    rc = 0;
	    if (rc == 0)
	    {
		AddStartupModuleToList(_nx_startup_preinit(),
				       module_list[i].module_name,
				       startup_funcs);
	    }
	}
    }

    /* Call the preinit function in each of the startup modules */
    initial_nodes_list_head = initial_nodes_list_tail
	= (nexus_startup_node_t *) NULL;
    for (startup_module = startup_module_list_head;
	 startup_module;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->preinit)
	{
	    ignore_command_line_args
		= (*startup_module->funcs->preinit)(&nl_head, &nl_tail)
		  || ignore_command_line_args;
	    if (nl_head)
	    {
		/*
		 * The preinit routine returned some initial nodes
		 */
		if (initial_nodes_list_head)
		{
		    initial_nodes_list_tail->next = nl_head;
		    initial_nodes_list_tail = nl_tail;
		}
		else
		{
		    initial_nodes_list_head = nl_head;
		    initial_nodes_list_tail = nl_tail;
		}
	    }
	}
    }

    return(ignore_command_line_args);
} /* _nx_startup_preinit() */


/*
 * _nx_startup_init()
 */
void _nx_startup_init(int *argc, char ***argv)
{
    int arg_num;
    startup_module_list_t *startup_module;
    
    arg_nodes_list = (char *) NULL;
    arg_startup_file = (char *) NULL;
    arg_n_nodes = 0;
    arg_dont_duplicate_master_node = NEXUS_FALSE;
    _nx_debug_command = (char *) NULL;
    _nx_debug_display = (char *) NULL;
    
    if ((arg_num = nexus_find_argument(argc, argv, "nodes", 2)) >= 0)
    {
	arg_nodes_list = _nx_copy_string((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "n", 2)) >= 0)
    {
	int tmp_n_nodes;
	if (arg_nodes_list != (char *) NULL)
	{
	    nexus_fatal("Cannot use -n with -nodes\n");
	}
	if ((tmp_n_nodes = atoi((*argv)[arg_num + 1])) > 0)
	{
	    arg_n_nodes = tmp_n_nodes;
	}
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "s", 2)) >= 0)
    {
	if (arg_nodes_list != (char *) NULL)
	{
	    nexus_fatal("Cannot use -s with -nodes\n");
	}
	if (arg_n_nodes > 0)
	{
	    nexus_fatal("Cannot use -s with -n\n");
	}
	arg_startup_file = _nx_copy_string((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "nodup", 1)) >= 0)
    {
	arg_dont_duplicate_master_node = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "debug_command", 2)) >= 0)
    {
	_nx_debug_command = _nx_copy_string((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "debug_display", 2)) >= 0)
    {
	_nx_debug_display = _nx_copy_string((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }

    /*
     * Initialize each startup module
     */
    for (startup_module = startup_module_list_head;
	 startup_module;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->init)
	{
	    (*startup_module->funcs->init)(argc, argv);
	}
    }
    
} /* _nx_startup_init() */


/*
 * _nx_startup_get_master_node_name()
 *
 * Go through the startup modules, and call the first one that
 * support get_master_node_name().
 * If none support this function, then return the host name.
 *
 * The returned string should be in newly malloced space.
 */
char *_nx_startup_get_master_node_name(void)
{
    char hostname[MAXHOSTNAMELEN];
    startup_module_list_t *startup_module;
    char *name = (char *) NULL;

    /*
     * Try to find a startup module to give us the master node name
     */
    for (startup_module = startup_module_list_head;
	 startup_module && !name;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->get_master_node_name)
	{
	    name = (*startup_module->funcs->get_master_node_name)();
	}
    }

    /*
     * Default to using the host name
     */
    if (!name)
    {
	_nx_md_gethostname(hostname, MAXHOSTNAMELEN);
	name = _nx_copy_string(hostname);
    }

    return (name);
} /* _nx_startup_get_master_node_name() */


/*
 * _nx_startup_initial_nodes()
 *
 * Start up all of the initial nodes.  This is only called by the
 * master node.  It should startup all other initial nodes with
 * the appropriate arguments.
 *
 * Return:	In '*nodes', an array of nexus_node_t's
 *			to the other nodes.
 *		In '*n_nodes', the number of nodes in 'nodes'.
 */
void _nx_startup_initial_nodes(nexus_node_t *my_node,
			       nexus_node_t **nodes,
			       int *n_nodes)
{
    /* Add nodes from command line arguments into the initial_nodes_list */
    if (arg_nodes_list != (char *) NULL)
    {
	/* -nodes <nodelist> */
	nexus_startup_node_t *node_list;
	node_list = node_list_parse(arg_nodes_list, NULL);
	finish_node_list_init(node_list);
    }
    else if (arg_startup_file != (char *) NULL)
    {
	FILE *startup_file;
	nexus_startup_node_t *node_list = NULL;
	if ((startup_file = fopen(arg_startup_file, "r")) == NULL)
	{
	    nexus_fatal("_nx_startup_initial_nodes(): Can't open startup file\n");
	}
	node_list = node_list_parse(NULL, startup_file);
	fclose(startup_file);
	finish_node_list_init(node_list);
    }
    else if (arg_n_nodes > 0)
    {
	/*
	 * Allocate and initialize a nexus_startup_node_t for
	 * arg_n_nodes nodes.
	 */
	nexus_startup_node_t *initial_node;
    	NexusMalloc(_nx_startup_initial_nodes(),
		    initial_node,
		    nexus_startup_node_t *,
		    sizeof(nexus_startup_node_t));
	initial_node->name = _nx_copy_string(my_node->name);
	initial_node->number = 1;
	initial_node->count = arg_n_nodes;
	initial_node->count_other_nodes = 0;
	initial_node->startup_module = (char *) NULL;
	initial_node->startup_funcs = (nexus_startup_funcs_t *) NULL;
	initial_node->next = (nexus_startup_node_t *) NULL;
	initial_node->st_ptr = NULL;
	finish_node_init(initial_node);
    }

    /*
     * We now have a complete list of the initial
     * nodes (initial_nodes_list_head) that need to be started.
     * So start them, leaving room for the master node.
     */
    nexus_debug_printf(2, ("_nx_startup_initial_nodes(): calling start_node_list\n"));
    start_node_list(initial_nodes_list_head, NEXUS_TRUE, nodes, n_nodes);
    nexus_debug_printf(2, ("_nx_startup_initial_nodes(): returned from start_node_list\n"));
    /*
     * Initialize the master node's nexus_node_t
     * Use nexus_malloc() so that the user can free this with nexus_free().
     */
    (*nodes)[0].name = nexus_malloc(strlen(my_node->name) + 1);
    strcpy((*nodes)[0].name, my_node->name);
    (*nodes)[0].number = my_node->number;
    nexus_startpoint_set_null(&((*nodes)[0].startpoint));
    (*nodes)[0].return_code = NEXUS_NODE_NEW;
} /* _nx_startup_initial_nodes() */


/*
 * finish_node_list_init()
 *
 * Go through the node_list and finish initializing the structures
 * by calling finish_node_init().  However, remove any entries
 * from the node_list that are the same as the master node.
 */
static void finish_node_list_init(nexus_startup_node_t *node_list)
{
    nexus_startup_node_t *cur_node;
    nexus_startup_node_t *next_node;
    nexus_bool_t keep_it;

    cur_node = node_list;
    while(cur_node)
    {
	next_node = cur_node->next;
	keep_it = NEXUS_TRUE;
	if (   (arg_dont_duplicate_master_node)
	    && (strcmp(_nx_my_node.name, cur_node->name) == 0)
	    && (_nx_my_node.number == cur_node->number) )
	{
	    if (cur_node->count > 1)
	    {
		cur_node->count--;
		cur_node->number++;
	    }
	    else
	    {
		NexusFree(cur_node);
		keep_it = NEXUS_FALSE;
	    }
	}
	if (keep_it)
	{
	    finish_node_init(cur_node);
	}
	cur_node = next_node;
    }
} /* finish_node_list_init() */


/*
 * finish_node_init()
 */
static void finish_node_init(nexus_startup_node_t *initial_node)
{
    char my_directory_path[MAX_PATH_LENGTH];
    char *my_executable_path;
    int save_error;
    char *s;
    
    /* Get various info about my node */
    if (getwd(my_directory_path) == 0)
    {
	save_error = errno;
	nexus_fatal("_nx_startup_nodes(): getwd failed: %s\n",
		    _nx_md_system_error_string(save_error));
    }
    _nx_strip_tmp_mnt_from_path(my_directory_path);
    my_executable_path = _nx_executable_name();

    /* Grab the directory_path from the database */
    s = nexus_rdb_lookup(initial_node->name, "startup_dir");
    if (s)
    {
        initial_node->directory_path = _nx_copy_string(s);
	nexus_rdb_free(s);
    }
    else
    {
        initial_node->directory_path = _nx_copy_string(my_directory_path);
    }
    
    /* Grab the executable_path from the database */
    s = nexus_rdb_lookup(initial_node->name, "startup_exe");
    if (s)
    {
        initial_node->executable_path = _nx_copy_string(s);
	nexus_rdb_free(s);
    }
    else
    {
        initial_node->executable_path = _nx_copy_string(my_executable_path);
    }

    /* Grab the count_other_nodes from the database */
    s = nexus_rdb_lookup(initial_node->name, "startup_count");
    if (s)
    {
        initial_node->count_other_nodes = (atoi(s)
					   - initial_node->count);
	nexus_rdb_free(s);
    }
    else
    {
	initial_node->count_other_nodes = 0;
    }

    /* Add this to the initial_nodes_list */
    if (initial_nodes_list_head)
    {
        initial_nodes_list_tail->next = initial_node;
        initial_nodes_list_tail = initial_node;
	initial_node->next = NULL;
    }
    else
    {
        initial_nodes_list_head = initial_node;
        initial_nodes_list_tail = initial_node;
    }
} /* finish_node_init() */


/*
 * nexus_node_acquire()
 *
 * Acquire the specified nodes.
 *
 * Return:	In '*nodes[].sp', an startpoints to the acquired nodes.
 *		In '*n_nodes', the number of nodes in 'nodes'.
 */
void nexus_node_acquire(char *node_name,
			int node_number,
			int count,
			char *directory_path,
			char *executable_path,
			nexus_node_t **nodes,
			int *n_nodes)
{
    nexus_startup_node_t *startup_node;
    char *tmp;
    int save_error;
    
    nexus_mutex_lock(&_nx_orphan_mutex);
    _nx_num_outstanding_creates++;
    nexus_mutex_unlock(&_nx_orphan_mutex);

    /*
     * Allocate and initialize a nexus_startup_node_t.
     */
    NexusMalloc(nexus_node_acquire(),
		startup_node,
		nexus_startup_node_t *,
		sizeof(nexus_startup_node_t));
    startup_node->name = _nx_copy_string(node_name);
    startup_node->number = node_number;
    startup_node->count = count;
    startup_node->count_other_nodes = 0;
    startup_node->startup_module = (char *) NULL;
    startup_node->startup_funcs = (nexus_startup_funcs_t *) NULL;
    startup_node->next = (nexus_startup_node_t *) NULL;
    startup_node->st_ptr = NULL;

    /*
     * Figure out what executable path to use
     */
    if (executable_path == (char *) NULL)
    {
	/* Use my path */
	startup_node->executable_path = _nx_executable_name();
    }
    else if (   (strncmp(executable_path,
			 NEXUS_DATABASE_PREFIX,
			 NEXUS_DATABASE_PREFIX_SIZE) == 0)
	     && (executable_path[NEXUS_DATABASE_PREFIX_SIZE] == ':') )
    {
	/* Lookup the path in the database */
	char *key = executable_path + NEXUS_DATABASE_PREFIX_SIZE + 1;
	if (   (*key == '\0')
	    || ((tmp = nexus_rdb_lookup(node_name, key)) == (char *) NULL) )
	{
	    /* Empty key or the lookup did not find anything */
	    startup_node->executable_path = _nx_executable_name();
	}
	else
	{
	    startup_node->executable_path = _nx_copy_string(tmp);
	    nexus_rdb_free(tmp);
	}
    }
    else
    {
	/* Use the passed path as is */
        startup_node->executable_path = _nx_copy_string(executable_path);
    }
    
    /*
     * Figure out what directory path to use
     */
    if (directory_path == (char *) NULL)
    {
	/* Use my cwd */
	startup_node->directory_path = _nx_current_working_directory();
    }
    else if (   (strncmp(directory_path,
			 NEXUS_DATABASE_PREFIX,
			 NEXUS_DATABASE_PREFIX_SIZE) == 0)
	     && (directory_path[NEXUS_DATABASE_PREFIX_SIZE] == ':') )
    {
	/* Lookup the path in the database */
	char *key = directory_path + NEXUS_DATABASE_PREFIX_SIZE + 1;
	if (   (*key == '\0')
	    || ((tmp = nexus_rdb_lookup(node_name, key)) == (char *) NULL) )
	{
	    /* Empty key or the lookup did not find anything */
	    startup_node->directory_path = _nx_current_working_directory();
	}
	else
	{
	    startup_node->directory_path = _nx_copy_string(tmp);
	    nexus_rdb_free(tmp);
	}
    }
    else
    {
	/* Use the passed path as is */
        startup_node->directory_path = _nx_copy_string(directory_path);
    }
    
    /*
     * Start the node
     */
    start_node_list(startup_node, NEXUS_FALSE, nodes, n_nodes);

    nexus_mutex_lock(&_nx_orphan_mutex);
    _nx_num_outstanding_creates--;
    if (_nx_num_outstanding_creates == 0)
    {
	nexus_cond_signal(&_nx_orphan_cond);
    }
    nexus_mutex_unlock(&_nx_orphan_mutex);

} /* nexus_node_acquire() */


/*
 * start_node_list()
 *
 * Start the nodes in 'node_list'.  Fillin 'nodes' with an
 * array of nexus_node_t structures for the nodes, and
 * set 'n_nodes' to be the number of element in that array.
 *
 * If leave_room_for_master==NEXUS_TRUE, then leave an extra
 * empty element at the beginning of the 'nodes' array.
 */
static void start_node_list(nexus_startup_node_t *node_list,
			    nexus_bool_t leave_room_for_master,
			    nexus_node_t **nodes,
			    int *n_nodes)
{
    nexus_startup_node_t *startup_node, *next_startup_node;
    int node_counter;
    char *candidates;
    char *db_candidates;
    char *value, *next;
    startup_module_list_t *startup_module;
    nexus_bool_t done;
    nexus_new_nodes_monitor_t *new_nodes_monitor;
    nexus_startpoint_t new_nodes_monitor_sp;
    nexus_endpoint_t new_nodes_monitor_ep;
    nexus_startpoint_t new_nodes_monitor_sp_copy;
    int first_node;
    int rc;
    nexus_endpointattr_t startup_ep_attr;

    if (leave_room_for_master)
    {
	first_node = 1;
    }
    else
    {
	first_node = 0;
    }
    
    /*
     * Go through this node_list to:
     *	- Count how many there are.
     *	- Assign each a unique node_id.
     *
     * Note: An entry in the node_list may
     * have count > 1.  These nodes will be started through
     * a single call to the appropriate startup module, but
     * that call will need to fill in 'count' nexus_node_t's,
     * one for each of the 'count' nodes.
     * So the number of nodes is actually the sum of the counts
     * in the node_list.
     */
    for (node_counter = first_node, startup_node = node_list;
	 startup_node;
	 startup_node = startup_node->next)
    {
#ifndef BUILD_PROFILE
	startup_node->id = node_counter;
#endif /* BUILD_PROFILE */
	node_counter += (startup_node->count
			 + startup_node->count_other_nodes);
    }

#ifdef BUILD_PROFILE
    {
	nexus_context_t *context;
	int next_node_id;
	_nx_context(&context);
	_nx_get_next_node_id_from_master(&next_node_id, node_counter, context);
	for (startup_node = node_list;
	     startup_node;
	     startup_node = startup_node->next)
	{
	    startup_node->id = next_node_id;
	    next_node_id += (startup_node->count
			     + startup_node->count_other_nodes);
	}
    }
#endif /* BUILD_PROFILE */

    /*
     * Allocate nexus_node_t's for node checkin
     * Use nexus_malloc() so that the user can free this with nexus_free().
     */
    *nodes = nexus_malloc(node_counter * sizeof(nexus_node_t));
    *n_nodes = node_counter;

    if ((*n_nodes - first_node) == 0)
    {
	return;
    }

    /* Setup the new nodes checkin monitor */
    new_nodes_monitor = wait_for_new_nodes_init(*nodes,
						(*n_nodes - first_node));

    /*
     * Startup_ep_attr cannot be initialize in _nx_startup_init()
     * because the context is ready at that point.
     */
    nexus_endpointattr_init(&startup_ep_attr);
    nexus_endpointattr_set_handler_table(&startup_ep_attr,
					 startup_handlers,
					 1);
    
    nexus_endpoint_init(&new_nodes_monitor_ep, &startup_ep_attr);
    nexus_endpoint_set_user_pointer(&new_nodes_monitor_ep,
				    (void *)new_nodes_monitor);
    nexus_startpoint_bind(&new_nodes_monitor_sp, &new_nodes_monitor_ep);

    /* Call into the startup module for each initial node */
    for (node_counter = first_node, startup_node = node_list;
	 startup_node;
	 startup_node = startup_node->next)
    {
	nexus_startpoint_copy(&new_nodes_monitor_sp_copy,
			      &new_nodes_monitor_sp);
	if (startup_node->startup_funcs)
	{
	    /*
	     * A startup method was pre-specified for this node.
	     * So use it.
	     */
	    rc = (*startup_node->startup_funcs->start_node)(startup_node,
						&new_nodes_monitor_sp_copy,
						node_counter);
	    if (rc != 0)
	    {
		nexus_fatal("start_node_list(): Could not start %s#%d using startup module: %s\n", startup_node->name, startup_node->number, startup_node->startup_module);
	    }
	}
	else
	{
	    /*
	     * No startup method has been specified for this node.
	     * So lookup all candidates in the resource database,
	     * and find one that I can use.
	     */
	    db_candidates = nexus_rdb_lookup(startup_node->name, "startups");
	    if (db_candidates)
	    {
		candidates = db_candidates;
	    }
	    else
	    {
		/* Default values */
#if defined(HAVE_MPL)
#ifdef HAVE_TCP
		candidates = "mpl,rsh,ss";
#else
		candidates = "mpl";
#endif
#elif defined(HAVE_INX)
#ifdef HAVE_TCP
		candidates = "inx,rsh,ss";
#else
		candidates = "inx";
#endif
#else
		candidates = "rsh,ss";
#endif
	    }
	    nexus_debug_printf(2, ("start_node_list(): startup candidates for %s#%d: %s\n", startup_node->name, startup_node->number, candidates));
	    
	    /*
	     * Compare this candidate list against the startup
	     * modules that this process has at its disposal, to
	     * look for a match.
	     */
	    done = NEXUS_FALSE;
	    next = candidates;
	    while (!done && next)
	    {
		_nx_get_next_value(next, ',', &next, &value);
		for (startup_module = startup_module_list_head;
		     !done && startup_module;
		     startup_module = startup_module->next)
		{
		    if (   (strcmp(value, startup_module->name) == 0)
			&& (startup_module->funcs->start_node != NULL) )
		    {
			nexus_debug_printf(3,("start_node_list(): startup %s#%d trying module: %s\n", startup_node->name, startup_node->number, startup_module->name));

			rc = (*startup_module->funcs->start_node)(startup_node,
						&new_nodes_monitor_sp_copy,
						node_counter);
			if (rc == 0)
			{
			    done = NEXUS_TRUE;
			}
		    }
		}
		NexusFree(value);
	    }
	    if (!done)
	    {
		nexus_fatal("startup_node_list(): This node does not have a startup module that can be used to start node %s#%d.\n", startup_node->name, startup_node->number);
	    }
	    nexus_rdb_free(db_candidates);
	}
        node_counter += (startup_node->count
			 + startup_node->count_other_nodes);
    }

    /* Wait for all of the nodes to checkin */
    wait_for_new_nodes(new_nodes_monitor);

    /* Free the elements in node_list */
    for (startup_node = node_list;
	 startup_node;
	 startup_node = next_startup_node)
    {
	next_startup_node = startup_node->next;
	NexusFree(startup_node->name);
	if (startup_node->executable_path)
	{
	    NexusFree(startup_node->executable_path);
	}
	if (startup_node->directory_path)
	{
	    NexusFree(startup_node->directory_path);
	}
	NexusFree(startup_node);
    }
    
    nexus_startpoint_destroy(&new_nodes_monitor_sp);
    nexus_endpoint_destroy(&new_nodes_monitor_ep);
} /* start_node_list() */


/*
 * _nx_startup_context()
 *
 * Create a process on the same node as the calling thread.
 * Use the passed 'executable_name' as the executable to use
 * for this process.  The string 'args' contains the Nexus
 * arguments that must be passed to that new process.
 */
int _nx_startup_context(char *executable_path,
			nexus_startpoint_t *reply_sp,
			int checkin_number,
			nexus_context_t **new_local_context)
{
    nexus_bool_t done = NEXUS_FALSE;
    int return_code;
    int access_rc;
    startup_module_list_t *startup_module;
    
    /*
     * Test that the executable_path is executable by this user
     */
#if !defined(HAVE_X_ACC) && defined(HAVE_X_OK)
#define X_ACC X_OK
#endif
    if ((access_rc = access(executable_path, X_ACC)) != 0)
    {
#ifdef BUILD_DEBUG
	int access_error;
	access_error = errno;
	nexus_debug_printf(1, ("_nx_startup_context(): Cannot execute %s, access() failed, rc=%d, errno=%d\n", executable_path, access_rc, access_error) );
#endif
	/* Error code: Cannot access the executable_path */
	return(-1);
    }

    /*
     * Find the first startup module that can start a local process
     */
    for (startup_module = startup_module_list_head;
	 !done && startup_module;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->start_context)
	{
	    done = (*startup_module->funcs->start_context)(executable_path,
							   reply_sp,
							   checkin_number,
							   &return_code,
							   new_local_context);
	}
    }
    if (!done)
    {
	/* Error code: No startup module can create a context */
	nexus_debug_printf(1, ("_nx_startup_context(): no startup module can create a context\n"));
	return_code = -2;
    }
    
    return (return_code);
} /* _nx_startup_context() */


/*
 * _nx_startup_shutdown()
 *
 * If 'shutdown_others' is NEXUS_TRUE,
 * then try to shutdown all other nodes and contexts.
 */
void _nx_startup_shutdown(nexus_bool_t shutdown_others)
{
    startup_module_list_t *startup_module;
    for (startup_module = startup_module_list_head;
	 startup_module;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->shutdown)
	{
	    (*startup_module->funcs->shutdown)(shutdown_others);
	}
    }
} /* _nx_startup_shutdown() */


/*
 * _nx_startup_abort()
 */
void _nx_startup_abort(void)
{
    startup_module_list_t *startup_module;
    for (startup_module = startup_module_list_head;
	 startup_module;
	 startup_module = startup_module->next)
    {
	if (startup_module->funcs->abort)
	{
	    (*startup_module->funcs->abort)();
	}
    }
} /* _nx_startup_abort() */



/*
 * wait_for_new_nodes_init()
 *
 * 'nodes' is an array global pointers with at least
 * 'n_nodes' elements.  This routine initializes and returns
 * a new_nodes_monitor, set to wait for 'n_nodes' checkins.
 */
static nexus_new_nodes_monitor_t *wait_for_new_nodes_init(nexus_node_t *nodes,
							  int n_nodes)
{
    nexus_new_nodes_monitor_t *new_nodes_monitor;

    NexusMalloc(wait_for_new_nodes_init(),
		new_nodes_monitor,
		nexus_new_nodes_monitor_t *,
		sizeof(nexus_new_nodes_monitor_t));
    nexus_mutex_init(&(new_nodes_monitor->mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(new_nodes_monitor->cond), (nexus_condattr_t *) NULL);
    new_nodes_monitor->total = n_nodes;
    new_nodes_monitor->checked_in = 0;
    new_nodes_monitor->nodes = nodes;
    
    return (new_nodes_monitor);
} /* wait_for_new_nodes_init() */


/*
 * wait_for_new_nodes()
 *
 * Wait for all the nodes for 'new_nodes_monitor' to checkin.
 * They will come via the _nx_new_node_checkin_handler().
 */
static void wait_for_new_nodes(nexus_new_nodes_monitor_t *new_nodes_monitor)
{
    nexus_mutex_lock(&(new_nodes_monitor->mutex));
    while (new_nodes_monitor->checked_in < new_nodes_monitor->total)
    {
	nexus_cond_wait(&(new_nodes_monitor->cond),
			&(new_nodes_monitor->mutex));
    }
    nexus_mutex_unlock(&(new_nodes_monitor->mutex));

    nexus_mutex_destroy(&(new_nodes_monitor->mutex));
    nexus_cond_destroy(&(new_nodes_monitor->cond));
    NexusFree(new_nodes_monitor);
} /* wait_for_new_nodes() */


/*
 * _nx_new_node_checkin_handler()
 */
static void _nx_new_node_checkin_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_new_nodes_monitor_t *new_nodes_monitor;
    int checkin_number;
    int node_name_length;
    char *node_name;
    int node_number;
    nexus_startpoint_t sp;
    int return_code;
    int n_nodes;
    int i;
#ifdef BUILD_PROFILE
    int need_master_sp;
#endif    

    new_nodes_monitor =
        (nexus_new_nodes_monitor_t *)nexus_endpoint_get_user_pointer(endpoint);
    nexus_get_int(buffer, &n_nodes, 1);
#ifdef BUILD_PROFILE
    nexus_get_int(buffer, &need_master_sp, 1);
#endif    
    for (i = 0; i < n_nodes; i++)
    {
        nexus_get_int(buffer, &checkin_number, 1);
        nexus_get_int(buffer, &node_name_length, 1);
        /* Use nexus_malloc() so that the user can free this with nexus_free(). */
        node_name = (char *) nexus_malloc(node_name_length + 1);
        nexus_get_char(buffer, node_name, node_name_length);
        node_name[node_name_length] = '\0';
        nexus_get_int(buffer, &node_number, 1);
        nexus_get_startpoint(buffer, &sp, 1);
        nexus_get_int(buffer, &return_code, 1);

#ifdef BUILD_PROFILE
        if (need_master_sp)
        {
	    _nx_send_master_gp_to_context(&sp);
        }
#endif /* BUILD_PROFILE */	

        nexus_mutex_lock(&(new_nodes_monitor->mutex));
        new_nodes_monitor->nodes[checkin_number].name = node_name;
        new_nodes_monitor->nodes[checkin_number].number = node_number;
        new_nodes_monitor->nodes[checkin_number].startpoint = sp;
        new_nodes_monitor->nodes[checkin_number].return_code = return_code;
        new_nodes_monitor->checked_in++;
	nexus_mutex_unlock(&(new_nodes_monitor->mutex));
    } /* for */
    nexus_mutex_lock(&(new_nodes_monitor->mutex));
    if (new_nodes_monitor->checked_in >= new_nodes_monitor->total)
    {
	nexus_cond_signal(&(new_nodes_monitor->cond));
    }
    nexus_mutex_unlock(&(new_nodes_monitor->mutex));

} /* _nx_new_node_checkin_handler() */


/*
 * _nx_node_checkin()
 *
 * Send an rsr to the _nx_new_node_checkin_handler() at creator_sp.
 */
void _nx_node_checkin(nexus_node_t *nodes,
		      int n_nodes,
		      nexus_startpoint_t *creator_sp,
		      int checkin_number
#ifdef BUILD_PROFILE
		      , int need_master_sp
#endif			 
		      )
{
    nexus_buffer_t buffer;
    int size;
    int name_len;
    int i;
    
    /*
     * Figure out the buffer size
     */
    size = 0;
    size += nexus_sizeof_int(1);		/* n_nodes */
#ifdef BUILD_PROFILE
    size += nexus_sizeof_int(1);		/* need_master_gp */
#endif
    for (i = 0; i < n_nodes; i++)
    {
        size += nexus_sizeof_int(1);		/* checking_number */
        size += nexus_sizeof_int(1);		/* node.name length */
        name_len = strlen(nodes[i].name);
        size += nexus_sizeof_char(name_len);	/* node.name */
        size += nexus_sizeof_int(1);		/* node.number */
        size += nexus_sizeof_startpoint(&(nodes[i].startpoint), 1);
        size += nexus_sizeof_int(1);		/* return_code */
    }

    /*
     * Set the buffer size and send the message
     */
    nexus_buffer_init(&buffer, size, 0);
    nexus_put_int(&buffer, &n_nodes, 1);
#ifdef BUILD_PROFILE
    nexus_put_int(&buffer, &need_master_sp, 1);
#endif    
    for (i = 0; i < n_nodes; i++)
    {
        nexus_put_int(&buffer, &checkin_number, 1);
        checkin_number++; /* for next node */

        name_len = strlen(nodes[i].name);
        nexus_put_int(&buffer, &name_len, 1);
        nexus_put_char(&buffer, nodes[i].name, name_len);
        nexus_put_int(&buffer, &nodes[i].number, 1);
        nexus_put_startpoint_transfer(&buffer, &(nodes[i].startpoint), 1);
        nexus_put_int(&buffer, &nodes[i].return_code, 1);

    }
    nexus_send_rsr(&buffer, creator_sp, 0, NEXUS_TRUE, NEXUS_FALSE);
} /* _nx_node_checkin() */


/*
 * node_list_parse()
 *
 * Parse nodes list.  This is a colon or newline separated list
 * of node specifiers, where a node specifier is of the form:
 *	node_name[#node_number][,node_count]
 * Whitespace can appear anywhere in this.
 *
 * Input for this parser either comes from a single string passed
 * as an argument, or from a FILE*.  (One of these arguments
 * should be NULL.)
 *
 * Return a list of nexus_startup_node_t structures of the
 * node specifiers.
 */
static nexus_startup_node_t *node_list_parse(char *input_string,
					     FILE *input_fp)
{
    nexus_startup_node_t *node_list_head = NULL;
    nexus_startup_node_t *node_list_tail = NULL;
    nexus_bool_t done;
#define S_LEN 4096
    char input_line[S_LEN];
    char *in;
    char *next;
    char *tmp;
    char *node_name;
    char *node_number_string;
    char *node_count_string;
    int node_number;
    int node_count;

    for (done = NEXUS_FALSE; !done; )
    {
	if (input_fp)
	{
	    if (!fgets(input_line, S_LEN, input_fp))
	    {
		done = NEXUS_TRUE;
		continue;
	    }
	    if (input_line[strlen(input_line)-1] != '\n')
	    {
		nexus_fatal("-s file contains a line that is too long (max length is %i), or the last line does not end with a newline\n", S_LEN);
	    }
	    in = input_line;
	}
	else if (input_string)
	{
	    if (strlen(input_string) > S_LEN-1)
	    {
		nexus_fatal("-nodes argument is too long: Maximum length is %i bytes\n", S_LEN-1);
	    }
	    strcpy(input_line, input_string);
	    done = NEXUS_TRUE;
	}
	else
	{
	    done = NEXUS_TRUE;
	    continue;
	}
	in = input_line;

	/* remove all whitespace from the string */
	for (tmp = in, next = in; *next != '\0'; next++)
	{
	    if (*next != ' '
		&& *next != '\t'
		&& *next != '\n')
	    {
		*tmp++ = *next;
	    }
	}
	*tmp = '\0';
	
	next = in;
	while (*next != '\0')
	{
	    in = next;
	    node_name = in;
	    node_number_string = NULL;
	    node_count_string = NULL;
	    node_number = 0;
	    node_count = 1;

	    /* Find next separator */
	    for (tmp = in; *tmp != ':' && *tmp != '\0'; tmp++)
		;
	    if (*tmp == ':')
		*tmp++ = '\0';
	    next = tmp;

	    /* Find the node number */
	    for (tmp = in; *tmp != '#' && *tmp != '\0'; tmp++)
		;
	    if (*tmp == '#')
	    {
		*tmp++ = '\0';
		in = tmp;
		node_number_string = tmp;
	    }

	    /* Find the node count */
	    for (tmp = in; *tmp != ',' && *tmp != '\0'; tmp++)
		;
	    if (*tmp == ',')
	    {
		*tmp++ = '\0';
		in = tmp;
		node_count_string = tmp;
	    }

	    if (*node_name != '\0')
	    {
		nexus_startup_node_t *new_node;
		
		if (node_number_string)
		    node_number = atoi(node_number_string);
		if (node_count_string)
		    node_count = atoi(node_count_string);

		if (node_number < 0 || node_count < 1)
		{
		    nexus_fatal("%s has an invalid node specifier\n",
				(input_fp ? "-s" : "-nodes"));
		}

		NexusMalloc(node_list_parse(),
			    new_node,
			    nexus_startup_node_t *,
			    sizeof(nexus_startup_node_t));
    
		new_node->name = _nx_copy_string(node_name);
		new_node->number = node_number;
		new_node->count = node_count;
		new_node->count_other_nodes = 0;
		new_node->directory_path = NULL;
		new_node->executable_path = NULL;
		new_node->startup_module = (char *) NULL;
		new_node->startup_funcs = (nexus_startup_funcs_t *) NULL;
		new_node->next = (nexus_startup_node_t *) NULL;
		new_node->st_ptr = NULL;
		if (node_list_head)
		{
		    node_list_tail->next = new_node;
		    node_list_tail = new_node;
		}
		else
		{
		    node_list_head = new_node;
		    node_list_tail = new_node;
		}
	    }
	}
    }

    return(node_list_head);
		    
} /* node_list_parse() */


