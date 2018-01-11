/*
 * John W. Garnett
 * California Institute of Technology
 * Compositional C++ Group
 * 1994 May 6
 *
 * st_pvm.c - Remote node startup for Nexus using PVM 3.2.6.
 */

/*
todo: change module names to _nx_st_pvm_ instead of _nx_st_rsh_.  Using
      _st_rsh for now to avoid having to change other modules depending on
      those names.
todo: test the "-nodes host1:host2" option with (host1 != host2)
*/

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_pvm.c,v 1.15 1996/10/07 04:40:19 tuecke Exp $";

#include "pr_pvm.h"

static char **arg_host_list;
static int arg_host_list_len;
static char *arg_startup_file;
static char *arg_directory;
static int arg_n_nodes;

nexus_bool_t _nx_st_rsh_start_remote_node(char *host, char *cmd);
nexus_bool_t _nx_st_rsh_start_local_node(char *executable_name, char *cmd);

/* from pr_pvm.c */
void _nx_ipvm_enter(void);
void _nx_ipvm_exit(void);

/*
 * _nx_st_rsh_process_arguments_init()
 *
 * Initialize any data structures that will hold command line arguments.
 */
void _nx_st_rsh_process_arguments_init(void)
{
    arg_host_list = (char **) NULL;
    arg_host_list_len = 0;
    arg_startup_file = (char *) NULL;
    arg_directory = (char *) NULL;
    arg_n_nodes = 1;
}

/*
 * _nx_st_rsh_process_arguments()
 *
 * See if the 'current_arg' argument (of the package arguments)
 * is meant for us.  If so, then extract it, and return a
 * new 'current_arg' value.  If not, then just return 'current_arg'
 * unchanged.
 */
int _nx_st_rsh_process_arguments(int current_arg, int arg_count)
{
	char *arg;
    
	arg = nexus_package_getarg(current_arg);
	if ((strcmp(arg, "-directory") == 0) && ((current_arg + 1) < arg_count)) {
		int rc;
		char *dir;

		dir = nexus_package_getarg(current_arg + 1);
		rc = chdir(dir);
		current_arg += 2;
		if (rc < 0) {
			nexus_fatal("Unable to change directory to %s\n", dir);
		}
	} else if ((strcmp(arg, "-s") == 0) && ((current_arg + 1) < arg_count)) {
		if (arg_host_list_len > 0) {
			nexus_fatal("Cannot use -s with -nodes\n");
		}
		if (arg_n_nodes > 1) {
			nexus_fatal("Cannot use -s with -n\n");
		}
		arg_startup_file =
			_nx_copy_string(nexus_package_getarg(current_arg + 1));
		current_arg += 2;
	} else if ((strcmp(arg, "-n") == 0) && ((current_arg + 1) < arg_count)) {
		if (arg_host_list_len > 0) {
			nexus_fatal("Cannot use -n with -nodes\n");
		}
		if (arg_startup_file != (char *) NULL) {
			nexus_fatal("Cannot use -n with -s\n");
		}
		arg_n_nodes = NEXUS_MAX(atoi(nexus_package_getarg(current_arg + 1)),1);
		current_arg += 2;
	} else if ((strcmp(arg, "-nodes") == 0) && ((current_arg + 1) < arg_count)) {
		char *nlist, *nlist_copy, *nptr, *node, *s;
		int done;

		if (arg_n_nodes > 1) {
			nexus_fatal("Cannot use -nodes with -n\n");
		}
		if (arg_startup_file != (char *) NULL) {
			nexus_fatal("Cannot use -nodes with -s\n");
		}
		nlist = nexus_package_getarg(current_arg + 1);
		/*
		 * Count the number of hosts and allocate
		 * the arg_host_list array.
		 */
		for (nptr = nlist, done = NEXUS_FALSE, arg_host_list_len = 0; !done;
			arg_host_list_len++)
		{
			if ((s = strchr(nptr, ':')) != (char *)NULL) {
				nptr = s + 1;
			} else {
				done = 1;
			}
		}
		NexusMalloc(_nx_tcp_process_arguments(), arg_host_list, char **,
			arg_host_list_len * sizeof(char *));
		/*
		 * Put the host names into the arg_host_list array
		 */
		nlist_copy = _nx_copy_string(nlist);
		for (nptr = nlist_copy, done = NEXUS_FALSE, arg_host_list_len = 0;
			!done; arg_host_list_len++)
		{
			node = nptr;
			if ((s = strchr(nptr, ':')) != (char *)NULL) {
				*s = '\0';
				nptr = s + 1;
			} else {
				done = 1;
			}
			arg_host_list[arg_host_list_len] = _nx_copy_string(node);
		}
		NexusFree(nlist_copy);
		current_arg += 2;
	}
	return (current_arg);
} /* _nx_st_rsh_process_arguments() */

/*
 * _nx_st_rsh_usage_message()
 */
void _nx_st_rsh_usage_message(void)
{
    printf("    -s <file>                 : Use this startup file\n");
    printf(
"    -nodes <hostlist>         : Start on these nodes.  <hostlist> is a colon\n"
);
    printf("                                separated list of hostnames\n");
    printf(
"    -n <integer>              : Start <integer> nodes on this machine\n");
    printf(
"    -directory <pathname>     : Change current directory to <pathname>.\n");
}

/*
 * _nx_st_rsh_new_process_params()
 */
int _nx_st_rsh_new_process_params(char *buf, int size)
{
	char current_directory[MAX_PATH_LENGTH];
	char tmpbuf[MAX_PATH_LENGTH + 32];
	int len;

	if (getwd(current_directory) == 0) {
		strcpy(current_directory, ".");
	} else {
		_nx_strip_tmp_mnt_from_path(current_directory);
	}
	sprintf(tmpbuf, "-directory %s", current_directory);
	if ((len = strlen(tmpbuf)) > size) {
		nexus_fatal(
			"_nx_st_rsh_new_process_params: Not enough room in buffer\n");
	}
	strcpy(buf, tmpbuf);
    return len;
}

/*
 * _nx_start_nodes()
 *
 * Start up all nodes specified on the PVM command line arguments.
 * This will only be called on the master node.
 *
 * Note: Non-master nodes should be given two extra arguments:
 * 	-node <node_id>
 *	-gp <context> <address> <checkin_number>: This is used to
 *		reconstruct a gp back the creating context for checkin.
 *
 * Return:	In '*node_gps', an array of global pointers
 *			to the other nodes.
 *			Leave the first gp empty for the master node,
 *		In '*n_node_gps', the number of nodes in 'node_gps'.
 *			Count the empty space for the master node.
 */

void _nx_start_nodes(nexus_global_pointer_t **node_gps, int *n_node_gps)
{
	char current_directory[MAX_PATH_LENGTH];
	int save_errno;

	/* Get the current directory */
	if (getwd(current_directory) == 0) {
		save_errno = errno;
		nexus_fatal("_nx_start_nodes(): getwd failed: %s\n",
			_nx_md_system_error_string(save_errno));
	}
	*node_gps = (nexus_global_pointer_t *)NULL;
    
	if ((arg_host_list == (char **) NULL) &&
		(arg_startup_file == (char *) NULL) &&
		(arg_n_nodes <= 1) )
	{
		/*
		 * This is a single node run.
		 */
		*node_gps = (nexus_global_pointer_t *)
			nexus_malloc(sizeof(nexus_global_pointer_t));
		*n_node_gps = 1;
    } else if (arg_host_list_len > 0) { /* -nodes */
		/* We've got a host list (-nodes flag) */
		char cmd[NEXUS_MAX_COMMAND_LENGTH];
		char remote_args[NEXUS_MAX_COMMAND_LENGTH];
		char *executable_name;
		int i, n_nodes;
		int *infos, rc;
		nexus_new_nodes_monitor_t *new_nodes_monitor;
		nexus_context_t *context;
#ifdef BUILD_PROFILE
		int my_node_id, my_context_id;
		char profile_cmd[32];
#endif

		_nx_context(&context);
	
		_nx_strip_tmp_mnt_from_path(current_directory);
	
		/* Determine the name used to invoke this node */
		executable_name = _nx_executable_name();

		/* Setup the array of gps for return nodes */
		n_nodes = arg_host_list_len + 1;
		nexus_debug_printf(2, ("_nx_start_nodes: n_nodes = %d\n", n_nodes));
		*node_gps = (nexus_global_pointer_t *)
			nexus_malloc(n_nodes * sizeof(nexus_global_pointer_t));
		*n_node_gps = n_nodes;
	
		/* Setup the new nodes checkin monitor */
		new_nodes_monitor = _nx_wait_for_new_nodes_init(*node_gps, n_nodes-1);
	
		/* Start the nodes on the machines listed in arg_host_list */
		for (i = 1; i < n_nodes; i++) {
			/* Get all arguments that need to go to this process */
			_nx_new_process_params(remote_args,
					       NEXUS_MAX_COMMAND_LENGTH);

			/* pass directory in via -directory flag since pvm_spawn()
			   doesn't allow the directory to be specified.
			*/
			sprintf(cmd, "%s %s -directory %s -node %d -gp %lu %lu %d %s",
				executable_name,
				_nx_get_package_designator(),
				current_directory,
				i,
				(unsigned long) context,
				(unsigned long) new_nodes_monitor,
				i,
				remote_args
			);
#ifdef BUILD_PROFILE
			_nx_node_id(&my_node_id);
			_nx_context_id(&my_context_id);
			sprintf(profile_cmd, " -gp_id %d %d", my_node_id, my_context_id);
			strcat(cmd, profile_cmd);
#endif /* BUILD_PROFILE */
			_nx_st_rsh_start_remote_node(arg_host_list[i-1], cmd);
		}
		_nx_wait_for_new_nodes(new_nodes_monitor);
    } /* -nodes */
    else if (arg_n_nodes > 1) { /* -n */
	/*
	 * We've got a number of nodes (-n flag)
	 */
	char cmd[NEXUS_MAX_COMMAND_LENGTH];
	char remote_args[NEXUS_MAX_COMMAND_LENGTH];
	char *executable_name;
	int i, n_nodes;
	nexus_new_nodes_monitor_t *new_nodes_monitor;
	nexus_context_t *context;
#ifdef BUILD_PROFILE
    int my_node_id, my_context_id;
    char profile_cmd[32];
#endif

	_nx_context(&context);

	/* Determine the name used to invoke this node */
	executable_name = _nx_executable_name();

	/* Setup the array of gps for return nodes */
	n_nodes = arg_n_nodes;
	*node_gps = (nexus_global_pointer_t *)
	    nexus_malloc(n_nodes * sizeof(nexus_global_pointer_t));
	*n_node_gps = n_nodes;
	
	/* Setup the new nodes checkin monitor */
	new_nodes_monitor = _nx_wait_for_new_nodes_init(*node_gps, n_nodes-1);
	
	/* Start the nodes on the machines listed in arg_host_list */
	for (i = 1; i < n_nodes; i++) {
	    /* Get all arguments that need to go to this process */
	    _nx_new_process_params(remote_args, NEXUS_MAX_COMMAND_LENGTH);

		/* pass directory in via -directory flag since pvm_spawn()
		   doesn't allow the directory to be specified.
		*/
	    sprintf(cmd, "%s %s -directory %s -node %d -gp %lu %lu %d %s",
		    executable_name, _nx_get_package_designator(),
			current_directory, i,
		    (unsigned long) context,
		    (unsigned long) new_nodes_monitor,
		    i, remote_args
		);
#ifdef BUILD_PROFILE
		_nx_node_id(&my_node_id);
		_nx_context_id(&my_context_id);
		sprintf(profile_cmd, " -gp_id %d %d", my_node_id, my_context_id);
		strcat(cmd, profile_cmd);
#endif /* BUILD_PROFILE */
		_nx_st_rsh_start_local_node(executable_name, cmd);
	}
	_nx_wait_for_new_nodes(new_nodes_monitor);
	} /* -n */
	NexusAssert2((*node_gps != (nexus_global_pointer_t *) NULL),
		("_nx_start_nodes(): Finished with *node_gps==NULL\n") );
}

/* todo: allow the other PVM debugger flags to be set in a single
   environment variable and parse them into the right value.  E.g.
      setenv PVM_DEBUG PvmTaskDebug|PvmTaskTrace
*/
static int UseDebugger(void)
{
	return (getenv("PVM_DEBUG") ? PvmTaskDebug : 0);
}

static void nx_handle_spawn(int info, int rc, char *caller, char *executable)
{
	IPVM_TEST(info, caller);
	if (info < 0) {
		nexus_printf("_nx_st_rsh_start_local_node(): pvm_spawn() failed.\n");
		nexus_fatal("Try starting your application without a pvmd running.\n");
	} else if (info == 0) {
		IPVM_TEST(rc, caller);
		nexus_printf("_nx_st_rsh_start_local_node(): pvm_spawn() failed.\n");
		nexus_fatal("Try starting your application without a pvmd running.\n");
	}
}

/*
 * start_local_node()
 *
 * todo: move this into util.c
 */
nexus_bool_t _nx_st_rsh_start_local_node(char *executable_name, char *cmd)
{
	int argc;
	int numt, tids[1];
	char *argv[1024];
	char string_buffer[NEXUS_MAX_COMMAND_LENGTH];
	char tmpbuf[1024];
	int flags;
	static int been_there = 0;
	static char ipvm_local_host[MAXHOSTNAMELEN];
	int ipvm_local_pid;

	if (!been_there) { /* todo: find a better place to put this. */
		_nx_md_gethostname(ipvm_local_host, MAXHOSTNAMELEN);
		ipvm_local_pid = _nx_md_getpid();
		been_there = 1;
	}
	if (_nx_dont_start_processes) {
		nexus_printf("Would pvm_spawn() and run <%s>\n", cmd);
		return NEXUS_TRUE;
    }
	argc = _nx_split_args(cmd, 0, NEXUS_MAX_COMMAND_LENGTH - 1, argv, string_buffer);
	argv[argc] = (char *) NULL;
	/*
	   We don't care about the returned 'tid' since we are using pvm_spawn()
	   as a portable replacement for fork() (see st_rsh.c).
	*/
	flags = PvmTaskHost | UseDebugger();
	nexus_debug_printf(1, ("_nx_st_rsh_start_local_node: spawning %s\n",
		executable_name));
	_nx_ipvm_enter();
	numt = pvm_spawn(executable_name, argv+1, flags, ipvm_local_host, 1, tids);
	_nx_ipvm_exit();
	nx_handle_spawn(numt, tids[0], "_nx_st_rsh_start_local_node", executable_name);
	return NEXUS_TRUE;
}

/*
 * _nx_st_rsh_start_remote_node()
 */
nexus_bool_t _nx_st_rsh_start_remote_node(char *host, char *cmd)
{
	int argc;
	int numt, tids[1];
	char *argv[1024];
	char string_buffer[NEXUS_MAX_COMMAND_LENGTH];
	int flags;

	if (_nx_dont_start_processes) {
		nexus_printf("Would spawn %s on host %s\n", cmd, host);
		return NEXUS_TRUE;
	}

	argc = _nx_split_args(cmd, 0, NEXUS_MAX_COMMAND_LENGTH - 1, argv, string_buffer);
	argv[argc] = (char *) NULL;
	_nx_ipvm_enter();
	if (pvm_mstat(host) != PvmOk) {
		char *hosts[1];
		int infos[1];
		int info;

		hosts[0] = host;
		info = pvm_addhosts(hosts, 1, infos);
		IPVM_TEST(info, "_nx_st_rsh_start_remote_node");
	}
	_nx_ipvm_exit();
	/*
	   We don't care about the task id here since we are using pvm_spawn()
	   as a portable replacement for fork() (see st_rsh.c).
	*/
	flags = PvmTaskHost | UseDebugger();
	nexus_debug_printf(1, ("_nx_st_rsh_start_remote_node: spawning %s\n", argv[0]));
	_nx_ipvm_enter();
	numt = pvm_spawn(argv[0], argv + 1, flags, host, 1, tids);
	_nx_ipvm_exit();
	nx_handle_spawn(numt, tids[0], "_nx_st_rsh_start_remote_node", argv[0]);
	return NEXUS_TRUE;
}
