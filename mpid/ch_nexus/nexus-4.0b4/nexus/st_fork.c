/*
 * st_fork.c
 *
 * Process startup using fork.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_fork.c,v 1.21 1996/11/13 23:21:29 tuecke Exp $";

#include "internal.h"

static nexus_bool_t	st_fork_start_context(char *executable_path,
					  nexus_startpoint_t *reply_sp,
					  int checkin_number,
					  int *return_code,
					  nexus_context_t **new_local_context);

static nexus_startup_funcs_t st_fork_funcs =
{
    NULL /* st_fork_preinit */,
    NULL /* st_fork_get_master_node_name */,
    NULL /* st_fork_init */,
    NULL /* st_fork_shutdown */,
    NULL /* st_fork_abort */,
    NULL /* st_fork_start_node */,
    st_fork_start_context,
};

extern char *  _nx_debug_command;
extern char *  _nx_debug_display;

/*
 * _nx_st_fork_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_fork_info(void)
{
    return ((void *) (&st_fork_funcs));
}


/*
 * st_fork_start_context()
 *
 * Create a process on the same node as the calling thread, using
 * 'executable_name', and the passed Nexus arguments 'args'.
 *
 * Do not set 'new_local_process' since this new context is in a
 * separate process.
 *
 * Return: If this function cannot be used, return NEXUS_FALSE.  In this
 *	case, the startup interface will try to start the local process
 *	using another startup module.
 *	Otherwise, return NEXUS_TRUE, and set 'return_code' to 0 on
 *	success or non-0 on failure.
 */
static nexus_bool_t st_fork_start_context(char *executable_path,
					  nexus_startpoint_t *reply_sp,
					  int checkin_number,
					  int *return_code,
					  nexus_context_t **new_local_context)
{
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    int cmd2_len;
    int rc;
    int child;
    
    /* Get all arguments that need to go to this process */
    _nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
    cmd2_len = strlen(cmd2);

    _nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
				    NEXUS_MAX_COMMAND_LENGTH - cmd2_len,
				    reply_sp);

    nexus_stdio_lock();

    sprintf(cmd,
	    "%s -nx_node %s %d y -nx_node_id %d -nx_context -nx_checkin %d %s %s",
	    _nx_get_package_id_start(),
	    _nx_my_node.name,
	    _nx_my_node.number,
	    _nx_my_node_id,
	    checkin_number,
	    cmd2,
	    _nx_get_package_id_end() );

    nexus_stdio_unlock();
    
    if (_nx_dont_start_processes)
    {
	nexus_printf("Would fork and run <%s>\n", cmd);
	*return_code = 0;
	return(NEXUS_TRUE);
    }

    /* fork and exec the process */
    nexus_debug_printf(1, ("st_fork_start_context(): Preparing to fork\n"));
    child = _nx_md_fork();
    if ( child == 0 )
    {
	/* This is the child process */
	close(0);
	if (_nx_debug_command)
	{
	    char tmpstring[1024];
	    char *debug_envvar;
	    char *envp[4];

	    sprintf(tmpstring, "NEXUS_DEBUG_ARGS=%s",cmd);
	    debug_envvar=_nx_copy_string(tmpstring);
	    envp[0] = debug_envvar;

	    sprintf(tmpstring, "NEXUS_DEBUG_CONTEXT=%s",executable_path);
	    debug_envvar=_nx_copy_string(tmpstring);
	    envp[1] = debug_envvar;

	    sprintf(tmpstring, "NEXUS_DEBUG_DISPLAY=%s",_nx_debug_display);
	    debug_envvar=_nx_copy_string(tmpstring);
	    envp[2] = debug_envvar;

	    envp[3] = (char *) NULL;
	    
	    rc = execle(_nx_debug_command, _nx_debug_command, 0, envp);
	}
	else
	{
	    int argc;
	    char *argv[1024];
	    
	    argv[0] = executable_path;
	    argc = _nx_split_args(cmd, 1, 1024, argv, cmd2);
	    argv[argc] = (char *) NULL;
    
	    rc = execv(executable_path, argv);
	}

	/* We shouldn't get to here */
	nexus_fatal("st_fork_start_context(): exec failed, returned %d, errno = %d\n", rc, errno);
	exit(1);
    }
    else if (child < 0)
    {
	/* Error code: Fork failed */
	*return_code = -3;
	return(NEXUS_FALSE);
    }

    nexus_debug_printf(1, ("st_fork_start_context(): Forked: %d\n",child));

    *return_code = 0;
    return(NEXUS_TRUE);
} /* st_fork_start_context() */
