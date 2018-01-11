/*
 * st_inx.c
 *
 * Remote node startup using rsh or a configuration file.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_inx.c,v 1.39 1996/11/13 23:21:29 tuecke Exp $";

#include "internal.h"

#include <nx.h>
#include <values.h>

#ifndef NEXUS_INX_NODE_NAME
#define NEXUS_INX_NODE_NAME "paragon_inx"
#endif

#define TASKBITS  11
#define PIDBITS   10
#define NODEBITS  11

#define TASKSHIFT 0
#define PIDSHIFT  TASKBITS
#define NODESHIFT (PIDSHIFT + PIDBITS)

#define NX_ON "-on"

#define INX_PARAMS_MSG_TYPE 3

/* What node we are on */
static unsigned long mnode;
/* Our process id */
static unsigned long pid;
/* task limit */
static unsigned long task_limit = 1 << TASKBITS;

static char *master_name = NULL;
static char **HACK = NULL;

extern char *	_nx_session_string;
extern int	_nx_session_string_length;

extern char **  environ;

static nexus_bool_t	st_inx_preinit(nexus_startup_node_t **nl_head,
			       nexus_startup_node_t **nl_tail);
static char *		st_inx_get_master_node_name(void);
static void		st_inx_init(int *argc, char ***argv);
static int		st_inx_start_node(nexus_startup_node_t *node,
					  nexus_global_pointer_t *reply_gp,
					  int first_checkin_number);
static nexus_bool_t	start_process(char *host, char* exe, char *cmd, int node);
static nexus_bool_t	st_inx_start_context(char *executable_path,
				          nexus_startpoint_t *reply_sp,
				          int checkin_number,
				          int *return_code,
					  nexus_context_t **new_local_context);
static nexus_bool_t	check_for_on(void);

static nexus_startup_funcs_t st_inx_funcs =
{
    st_inx_preinit,
    st_inx_get_master_node_name,
    st_inx_init,
    NULL /* st_inx_shutdown */,
    NULL /* st_inx_abort */,
    st_inx_start_node,
    st_inx_start_context,
};
static unsigned long newptype(void);


/*
 * _nx_st_inx_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_inx_info(void)
{
    return ((void *) (&st_inx_funcs));
}

/*
 * st_inx_preinit()
 */
static nexus_bool_t st_inx_preinit(nexus_startup_node_t **nl_head,
			     nexus_startup_node_t **nl_tail)
{
    nexus_bool_t ignore_command_line_args = NEXUS_FALSE;
    
    *nl_head = *nl_tail = (nexus_startup_node_t *) NULL;

    /*
     * Ptype only get 2^31 bits rather than 2^32.  Solution for now is to
     * drop the high order bit on the node. Hence the MAXLONG. tal 6/24/94
     */
    mnode = (mynode() << NODESHIFT) & MAXLONG;
    pid = (unsigned long) (PIDBITS & _nx_md_getpid()) << PIDSHIFT;

    if (!check_for_on())
    {
	if(myptype() == 0)
	{
	    /* This is one of the initial nodes. */
	    if(mynode() == 0)
	    {
		/* This is the master node */
		if(numnodes() > 1)
		{
		    /*
		     * Setup a nexus_startup_node_t structure for the rest
		     * of the initial INX nodes.
		     */
		    nexus_startup_node_t *initial_node;
		    NexusMalloc(st_inx_preinit(),
				initial_node,
				nexus_startup_node_t *,
				sizeof(nexus_startup_node_t));
		    initial_node->name = _nx_copy_string(NEXUS_INX_NODE_NAME);
		    HACK = &(initial_node->name);
		    initial_node->number = 1;
		    initial_node->count = numnodes() - 1;
		    initial_node->count_other_nodes = 0;
		    initial_node->startup_module = _nx_copy_string("inx");
		    initial_node->startup_funcs = &st_inx_funcs;
		    initial_node->next = (nexus_startup_node_t *) NULL;
		    initial_node->directory_path = (char *) NULL;
		    initial_node->executable_path = (char *) NULL;
		    initial_node->st_ptr = (void *) 1;
		    
		    *nl_head = *nl_tail = initial_node;
		}
	    }
	    else
	    {
		/*
		 * This is a non-master initial node.
		 * Wait for a message from the master node that contains
		 * _nx_my_process_params for my node.
		 * That message is sent from st_inx_start_node.
		 */
		int n_bytes = NEXUS_MAX_COMMAND_LENGTH;
		NexusMalloc(st_inx_preinit(),
			    _nx_my_process_params,
			    char *,
			    n_bytes);
		crecv(INX_PARAMS_MSG_TYPE, _nx_my_process_params, n_bytes);
		/*
		printf("(%ld,%ld) Got _nx_my_process_params=%s\n",
		       mynode(), myptype(), _nx_my_process_params);
		*/
		
		ignore_command_line_args = NEXUS_TRUE;
	    }
	}
    }
    /*
     * Else this is not an initial node.  So everything we need to know
     * is in the command line arguments.
     */
    
    return(ignore_command_line_args);
} /* st_inx_preinit() */


/*
 * st_inx_get_master_node_name()
 */
static char *st_inx_get_master_node_name(void)
{
    if (master_name)
    {
	return master_name;
    }
    else
    {
	return (_nx_copy_string(NEXUS_INX_NODE_NAME));
    }
} /* st_inx_get_master_node_name() */


/*
 * st_inx_init()
 */
static void st_inx_init(int *argc, char ***argv)
{
    int arg_num;

    if ((arg_num = nexus_find_argument(argc, argv, "nx_inx_session", 2)) >= 0)
    {
        _nx_session_string = _nx_copy_string((*argv)[arg_num+1]);
        nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	_nx_session_string = _nx_md_get_unique_session_string();
    }
    _nx_session_string_length = strlen(_nx_session_string);

    if ((arg_num = nexus_find_argument(argc, argv, "nx_inx_master", 2)) >= 0)
    {
	master_name = _nx_copy_string((*argv)[arg_num+1]);
	if (HACK)
	{
	    *HACK = master_name;
	}
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
}


/*
 * st_inx_start_node()
 *
 * Start the node(s) for the passed nexus_startup_node_t structure.
 */
static int st_inx_start_node(nexus_startup_node_t *node,
			     nexus_startpoint_t *reply_sp,
			     int first_checkin_number)
{
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    char args[NEXUS_MAX_COMMAND_LENGTH];
    int cmd2_len;
    int i;

    for (i = 0; i < node->count; i++)
    {
	/* Get all arguments that need to go to this process */
	_nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
	cmd2_len = strlen(cmd2);
	_nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
					NEXUS_MAX_COMMAND_LENGTH - cmd2_len,
					reply_sp);
	nexus_stdio_lock();
	sprintf(args,
		"%s -nx_node %s %d y -nx_node_id %d -nx_checkin %d %s -nx_inx_session %s -nx_inx_master %s %s ",
		_nx_get_package_id_start(),
		node->name,
		node->number + i,
		node->id + i,
		first_checkin_number + i,
		cmd2,
		_nx_session_string,
		st_inx_get_master_node_name(),
		_nx_get_package_id_end() );
	nexus_stdio_unlock();

	if(node->st_ptr)
	{
	    /*
	     * This node is a non-master initial node.
	     * So pass it arguments to use as it _nx_my_process_params
	     */
	    csend(INX_PARAMS_MSG_TYPE, args, strlen(args)+1,
		  node->number + i, 0);
	}
	else
	{
	    /*
	     * This node needs to be started
	     */
	    sprintf(cmd, "%s %s",
		    node->executable_path,
		    args);
	    start_process(node->name, node->executable_path, cmd,
			  node->number+i);
	}
    }

    return 0;
	
} /* st_inx_start_node() */


/*
 * start_process()
 */
static nexus_bool_t start_process(char *host, char* exe, char *cmd, int node)
{
    long node_list[2];
    long pid_list[1];
    char* executable_name;
    char* argv[1024];
    char string_buffer [NEXUS_MAX_COMMAND_LENGTH];
    int argc;
    unsigned long ptype;
    int err, err_count = 0;

    if (_nx_dont_start_processes)
    {
	nexus_printf("Would run <%s %s> on host %s\n",
		     exe, cmd, host);
	return NEXUS_TRUE;
    }

    node_list[0] = node;
    node_list[1] = NULL;

    argc = _nx_split_args (cmd,
			   0,
			   NEXUS_MAX_COMMAND_LENGTH-1,
			   argv,
			   string_buffer);
    argv[argc] = NULL;
    ptype = newptype();
	
    if (nx_loadve (node_list, 1, ptype, pid_list, argv[0], argv, environ) < 0)
    {
	nx_perror ("start_process(): nx_loadve failed");
	nexus_silent_fatal();
    }
	
    return(NEXUS_TRUE);
} /* start_process() */


/*
 * st_inx_start_context()
 *
 * Create a process on the same node as the calling thread, using
 * 'executable_name', and the passed Nexus arguments 'args'.
 *
 * Return: If this function cannot be used, return NEXUS_FALSE.  In this
 *	case, the startup interface will try to start the local process
 *	using another startup module.
 *	Otherwise, return NEXUS_TRUE, and set 'return_code' to 0 on
 *	success or non-0 on failure.
 */
static nexus_bool_t st_inx_start_context(char *executable_path,
					 nexus_startpoint_t *reply_sp,
					 int checkin_number,
					 int *return_code,
					 nexus_context_t **new_local_context)
{
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    char string_buffer[NEXUS_MAX_COMMAND_LENGTH];
    int argc;
    int cmd2_len;
    int my_node_id;
    
    _nx_node_id(&my_node_id);
    
    /* Get all arguments that need to go to this process */
    _nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
    cmd2_len = strlen(cmd2);
    _nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
				    NEXUS_MAX_COMMAND_LENGTH - cmd2_len,
				    reply_sp);
    nexus_stdio_lock();
    sprintf(cmd,
	    "%s %s -nx_node %s %d y -nx_node_id %d -nx_context -nx_checkin %d %s -nx_inx_session %s %s",
	    executable_path,
	    _nx_get_package_id_start(),
	    _nx_my_node.name,
	    _nx_my_node.number,
	    my_node_id,
	    checkin_number,
	    cmd2,
	    _nx_session_string,
	    _nx_get_package_id_end() );
    nexus_stdio_unlock();
    
    *return_code = 0;
    return(start_process (_nx_my_node.name, executable_path, cmd, mynode()));
} /* st_inx_start_context() */

#define TASKRESET 1    /* Number to start task at */
static unsigned long newptype (void)
{
    /* task counter */
    static unsigned long task = TASKRESET;
    
    /* Should do something if task counter resets */
    task++;
    if (task+1 == TASKBITS)
    {
	task = TASKRESET;
    }

    nexus_debug_printf(1, ("mnode %u, pid %u, task %u, ptype %u.\n", 
			   mnode, pid, task, mnode|pid|task));
    /* Should be doing some hashing on pid. */
    return (mnode | pid | task);
}


/*
 * check_for_on()
 *
 * Return NEXUS_TRUE if the -on flag was used, and NEXUS_FALSE otherwise.
 *
 * The entire argument list lies in raw form between argv[0]..environ[0].
 * So go back through environ to find the argv[0], and then scan
 * forward through the raw memory from argv[0] to environ[0] to
 * look for the -on flag.
 */
static nexus_bool_t check_for_on(void)
{
#define ArgStart 2
    int null_pos = ArgStart;
    int base_arg;
    char *start_address;
    char *end_address;
    char *arg;
    extern char **environ;

    /* Find the first argument */
    for (base_arg = ArgStart;
	 (int) environ[-base_arg] != base_arg - ArgStart;
	 base_arg++)
    {
	if (environ[-base_arg] == NULL)
	{
	    null_pos = base_arg;
	}
    }
   
    start_address = environ[-(base_arg-1)];
    end_address = environ[0];
    
    for(arg = start_address; arg < end_address; arg += strlen(arg)+1 )
    {
	if (strcmp(arg,"-on") == 0)
	{
	    return(NEXUS_TRUE);
	}
    }
    return(NEXUS_FALSE);
} /* check_for_on() */
