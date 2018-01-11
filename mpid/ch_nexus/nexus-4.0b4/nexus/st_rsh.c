/*
 * st_rsh.c
 *
 * Remote node startup using rsh or a configuration file.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_rsh.c,v 1.45 1996/10/07 04:40:19 tuecke Exp $";

#include "internal.h"

#define ST_RSH_USER_NAME_KEY "rsh_login"

static int	st_rsh_start_node(nexus_startup_node_t *node,
				  nexus_startpoint_t *reply_sp,
				  int first_checkin_number);
static nexus_bool_t	start_remote_node(char *host,
				  char *cmd,
				  char *current_directory,
				  char *user_name);

static nexus_startup_funcs_t st_rsh_funcs =
{
    NULL /* st_rsh_preinit */,
    NULL /* st_rsh_get_master_node_name */,
    NULL /* st_rsh_init */,
    NULL /* st_rsh_shutdown */,
    NULL /* st_rsh_abort */,
    st_rsh_start_node,
    NULL /* st_rsh_start_context */,
};

extern char *  _nx_debug_command;
extern char *  _nx_debug_display;

/*
 * _nx_st_rsh_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_rsh_info(void)
{
    return ((void *) (&st_rsh_funcs));
}


/*
 * st_rsh_start_node()
 *
 * Start the node(s) for the passed nexus_startup_node_t structure.
 */
static int st_rsh_start_node(nexus_startup_node_t *node,
			     nexus_startpoint_t *reply_sp,
			     int first_checkin_number)
{
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    int cmd2_len;
    int i, j;
    char *user_name;
    char **environment;
    char *var, *value;
    int n_env;

    nexus_debug_printf(1, ("st_rsh_start_node(): starting node %s#%d\n", node->name, node->number));

    for (i = 0; i < node->count; i++)
    {
	/* Get all arguments that need to go to this process */
	_nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
	cmd2_len = strlen(cmd2);

	_nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
					NEXUS_MAX_COMMAND_LENGTH - cmd2_len,
					reply_sp);

	nexus_stdio_lock();
	sprintf(cmd,
		"%s %s -nx_node %s %d n -nx_node_id %d -nx_checkin %d %s %s",
		node->executable_path,
		_nx_get_package_id_start(),
		node->name,
		node->number + i,
		node->id + i,
		first_checkin_number + i,
		cmd2,
		_nx_get_package_id_end() );
	nexus_stdio_unlock();

	user_name = nexus_rdb_lookup(node->name, ST_RSH_USER_NAME_KEY);

	_nx_startup_modify_command_for_type(node,
					    cmd,
					    NEXUS_MAX_COMMAND_LENGTH,
					    &environment,
					    &n_env);
	
	nexus_stdio_lock();
	for(j = 0; j < n_env; j++)
	{
	    _nx_get_next_value(environment[j], '=', &value, &var);
	    sprintf(cmd2, "%s=%s; export %s;\n %s", var, value, var, cmd);
	    strcpy(cmd, cmd2);
	    NexusFree(value);
	    NexusFree(environment[j]);
	}
	nexus_stdio_unlock();
	NexusFree(environment);

	start_remote_node(node->name, cmd, node->directory_path, user_name);
    }

    return 0;
	
} /* st_rsh_start_node() */


/*
 * start_remote_node()
 */
static nexus_bool_t start_remote_node(char *host,
				      char *cmd,
				      char *current_directory,
				      char *user_name)
{
    int child;
    FILE *fp;
    int p[2];
    int rc;
    int save_error;
#ifdef DONT_INCLUDE
    int pid, signal, retcode; 
#ifdef NEXUS_HAS_UNION_WAIT
    union wait status;
#else
    int status;
#endif
#endif /* DONT_INCLUDE */

    if (_nx_dont_start_processes)
    {
        if (user_name == (char *) NULL)
	{
	    nexus_printf("Would run <cd %s; %s> on host %s\n",
			 current_directory, cmd, host);
	}
	else
	{
	    nexus_printf("Would run <cd %s; %s> on host %s using user_name %s\n",
			 current_directory, cmd, host, user_name);
	}
	return NEXUS_TRUE;
    }

    nexus_stdio_lock();
    rc = pipe(p);
#ifdef JGG
    save_error = errno;
#else
    save_error = 0;
#endif
    nexus_stdio_unlock();
    if (rc < 0)
    {
	nexus_fatal("start_remote_node(): pipe() failed: %s\n",
		    _nx_md_system_error_string(save_error));
    }

    child = _nx_md_fork();
    if (child == 0)
    {
#ifndef TARGET_ARCH_FREEBSD
	close(0);
#endif
	dup2(p[0], 0);
	close(p[0]);
	close(p[1]);
	/*
	close(1);
	close(2);
	open("/dev/null", O_WRONLY);
	dup(1);
	*/

#ifndef TARGET_ARCH_HPUX
#define RSH_ARG0 "rsh"
#else
#define RSH_ARG0 "remsh"
#endif
	if (user_name == (char *) NULL)
	{
	    execl(NEXUS_RSH, RSH_ARG0, host, "/bin/sh", NULL);
	}
	else
	{
	    execl(NEXUS_RSH, RSH_ARG0, "-l", user_name, host, "/bin/sh", NULL);
	}
	nexus_perror("exec");
	exit(1);
    }
    else if (child < 0)
    {
	save_error = errno;
	nexus_fatal("start_remote_node(): fork failed: %s\n",
		    _nx_md_system_error_string(save_error));
    }

    close(p[0]);

    nexus_stdio_lock();
    fp = fdopen(p[1], "w");
    
    /*
     * If we have a -log flag, then we can just redirect stdout
     * to that file.  Then the rsh process will go away.
    fprintf(fp, "cd %s; %s < /dev/null > /dev/null 2>&1 &\n",
            current_directory, cmd);
     */
    if (_nx_debug_command)
    {
	char context_name[1024];
	char context_args[1024];
	int i=0;
	int j=0;
	
#ifdef TARGET_ARCH_SGI
	for ( ; cmd[i] != ' '; i++) context_name[i] = cmd[i];
#else
	while (cmd[i]!=' ') context_name[i] = cmd[i++];
#endif
	context_name[i]='\0';
	while (cmd[++i])
	{
	    if(cmd[i]=='\"')
	    {
		context_args[j++]='\\';
	    }
	    context_args[j++] = cmd[i];
	}
	context_args[j++]='\0';
	
	fprintf(fp, "cd %s ; NEXUS_DEBUG_ARGS=\"%s\" ; NEXUS_DEBUG_CONTEXT=\"%s\" ; NEXUS_DEBUG_DISPLAY=\"%s\" ; export NEXUS_DEBUG_ARGS NEXUS_DEBUG_CONTEXT NEXUS_DEBUG_DISPLAY ; %s &\n", current_directory, context_args, context_name, _nx_debug_display, _nx_debug_command);
    }
    else
    {
	fprintf(fp, "cd %s;\n %s < /dev/null &\n", current_directory, cmd);
    }
    
    fclose(fp);
    nexus_stdio_unlock();

#ifdef DONT_INCLUDE    
    /*
     * This wait is to check the status of rsh when it exits, but
     * the signal is being caught by the SIGCHLD handler.
     */
    pid = wait(&status);

#ifdef TARGET_ARCH_AIX
    signal = WTERMSIG(status);
    retcode = WEXITSTATUS(status);
#else
    signal = status.w_termsig;
    retcode = status.w_retcode;
#endif
    nexus_printf("Process %d returns with retcode %d signal %d\n",
		 pid, status.w_retcode, status.w_termsig);

    if (signal != 0 || retcode != 0)
    {
	return NEXUS_FALSE;
    }
    else
#endif /* DONT_INCLUDE */
    {
	return NEXUS_TRUE;
    }
} /* start_remote_node() */
