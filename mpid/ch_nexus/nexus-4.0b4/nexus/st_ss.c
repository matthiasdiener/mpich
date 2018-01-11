/*
 * st_ss.c
 *
 * Remote node startup using secure server
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_ss.c,v 1.33 1996/10/29 03:40:58 tuecke Exp $";

#include "internal.h"
#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <pwd.h>

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

/*
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
*/

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#endif

#ifdef BUILD_DEBUG
#define nexus_dbgss_printf( operation, level, message ) nexus_new_debug_printf( NXDBG_ST, NXDBG_ST_SS, operation, level, message )
#else
#define nexus_dbgss_printf( operation, level, message )
#endif /* BUILD_DEBUG */
#define NXDBG_SS_NODE 0x00000001

static int	st_ss_start_node(nexus_startup_node_t *node,
				 nexus_startpoint_t *reply_sp,
				 int first_checkin_number);

static nexus_startup_funcs_t st_ss_funcs =
{
    NULL /* st_ss_preinit */,
    NULL /* st_ss_get_master_node_name */,
    NULL /* st_ss_init */,
    NULL /* st_ss_shutdown */,
    NULL /* st_ss_abort */,
    st_ss_start_node,
    NULL /* st_ss_start_context */,
};

extern char *  _nx_debug_command;
extern char *  _nx_debug_display;

#ifdef BUILD_LITE
#define SS_Fork() _nx_md_fork()
#define SS_Malloc(S) nexus_malloc(S)
#include "ss_start.h"
#endif /* BUILD_LITE */

/*
 * _nx_st_ss_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_ss_info(void)
{
    return ((void *) (&st_ss_funcs));
}


/*
 * st_ss_start_node()
 *
 * Start the node(s) for the passed nexus_startup_node_t structure.
 */
static int st_ss_start_node(nexus_startup_node_t *node,
			    nexus_startpoint_t *reply_sp,
			    int first_checkin_number)
{
    char args[NEXUS_MAX_COMMAND_LENGTH], args2[NEXUS_MAX_COMMAND_LENGTH];
    int args2_len;
    int i;
    char *port;
    char *username;
    char *ss_type;
    char type[100];
    char **environment;
    int n_env;
#ifndef BUILD_LITE
    char *listener_path;
    char listener_exe[1024];
#endif

    nexus_dbgss_printf( NXDBG_SS_NODE, 1, ("st_ss_start_node(): starting node %s#%d\n", node->name, node->number));

    for (i = 0; i < node->count; i++)
    {
	/* Get all arguments that need to go to this process */
	_nx_new_process_params(args2, NEXUS_MAX_COMMAND_LENGTH);
	args2_len = strlen(args2);

	_nx_proto_get_creator_sp_params(args2 + args2_len,
					NEXUS_MAX_COMMAND_LENGTH - args2_len,
					reply_sp);

	nexus_stdio_lock();
	sprintf(args,
		"%s -nx_node %s %d n -nx_node_id %d -nx_checkin %d %s %s",
		_nx_get_package_id_start(),
		node->name,
		node->number + i,
		node->id + i,
		first_checkin_number + i,
		args2,
		_nx_get_package_id_end() );
	nexus_stdio_unlock();

	port = nexus_rdb_lookup(node->name, "ss_port");

	if( port )
	{
	    nexus_dbgss_printf( NXDBG_SS_NODE, 3, ("Using port %s\n", port));
	}

	ss_type = nexus_rdb_lookup(node->name, "ss_type");
	if (ss_type && (strcmp(ss_type, "iway") == 0))
	{
	    strcpy(type, "iway");
	    username = nexus_rdb_lookup(node->name, "ss_token");
	}
	else
	{
	    if (!ss_type)
	    {
		strcpy(type, "normal");
	    }
	    else
	    {
		strcpy(type, ss_type);
	    }
	    username = nexus_rdb_lookup(node->name, "ss_login");
	}
	if (!username)
	{
	    /*
	     * Send a fake username so that the listener's arg list is
	     * not terminated prematurely by a NULL username.  This
	     * makes the assumption that no one with the username
	     * "USE_LOCAL_USERNAME" will run the secure server.
	     */
	    username = _nx_copy_string("USE_LOCAL_USERNAME");
	}
	nexus_dbgss_printf(NXDBG_SS_NODE, 3, ("Using username %s\n",username));
	nexus_rdb_free(ss_type);

#ifndef BUILD_LITE
	/*
	 * Find the path to the listener program
	 */
	if ((listener_path = getenv("NEXUS_SS_LISTENER")) != (char *) NULL)
	{ 	    
	    nexus_dbgss_printf( NXDBG_SS_NODE, 3,
			       ("Using listener %s from environment variable\n",
				listener_path)); 
	}
	else if ((listener_path = nexus_rdb_lookup(node->name,
							"ss_listener"))
		 != (char *) NULL) 
	{
	    nexus_dbgss_printf( NXDBG_SS_NODE, 3,
			       ("Using listener %s from database\n",
				listener_path)); 
	}
	else if((listener_path = _nx_path_array[NEXUS_PATH_TYPE_LISTENER])
		!= (char *) NULL)
	{
	    nexus_dbgss_printf( NXDBG_SS_NODE, 3,
			       ("Using listener %s from path array\n",
				listener_path));
	}
	else
	{
	    listener_path = "";
	}

	if (*listener_path)
	{
	    sprintf(listener_exe, "%s/%s", listener_path, NEXUS_LISTENER);
	}
	else
	{
	    sprintf(listener_exe, "%s", NEXUS_LISTENER);
	}
	
	/*
	 * See if the listener_exe we constructed will work
	 */
	if (access(listener_exe, X_OK) != 0)
	{
	    nexus_fatal("st_ss_start_node(): listener program, \"%s\", does not exist or is not executable\n", listener_exe);
	}
#endif /* !BUILD_LITE */

	/*
	 * Modify the command as necessary
	 *
	 * This could modify the node->name and node->number.
	 * So make sure all the nexus_rdb_lookup() calls
	 * are made before we modify the command.
	 */
	_nx_startup_modify_command_for_type(node,
					    args,
					    NEXUS_MAX_COMMAND_LENGTH,
					    &environment,
					    &n_env);

#ifdef BUILD_LITE
	
	/*
	 * With NexusLite we can fork a listener directly.
	 */
	if (_nx_dont_start_processes)
	{
	    nexus_printf("Would tell server at %s:%s to run: <cd %s; %s %s>\n",
			 node->name,
			 port,
			 node->directory_path,
			 node->executable_path,
			 args);
	}
	else
	{
	    char *message;
	    if ((message = start_secure_remote_node(node->name,
						    atoi(port),
						    username,
						    node->directory_path,
						    environment,
						    n_env,
						    node->executable_path,
						    args,
						    type)) != (char *) NULL)
	    {
		nexus_fatal("st_ss_start_node(): Failed to start node %s: %s\n", node->name, message);
	    }
	}
	
#else  /* BUILD_LITE */
	
	/*
	 * Broken single-thread-fork semantics means we have to
	 * use a separate listener program which we fork and exec off.
	 */
	if (_nx_dont_start_processes)
	{
	    nexus_printf("Would run: <%s %s %s %s %s %s '%s' %s>\n",
			 listener_exe,
			 node->name,
			 port,
			 username,
			 node->directory_path,
			 node->executable_path,
			 args,
			 type);
	}
	else
	{
	    nexus_dbgss_printf( NXDBG_SS_NODE, 3, ("st_ss_start_node(): fork/exec <%s %s %s %s %s %s '%s' %s>\n", listener_exe, node->name, port, username, node->directory_path, node->executable_path, args, type));
	    
	    /* Now fork and exec the listener */
	    if (!_nx_md_fork())
	    {
		char **exec_argv;
		char tmp_argv[10];
		int i;
		
		NexusMalloc(st_ss_start_node,
			    exec_argv,
			    char **,
			    sizeof(char *) * (9 + n_env));
		exec_argv[0] = listener_exe;
		exec_argv[1] = node->name;
		exec_argv[2] = port;
		exec_argv[3] = username;
		exec_argv[4] = node->directory_path;
		exec_argv[5] = node->executable_path;
		exec_argv[6] = args;
		exec_argv[7] = type;
		sprintf(tmp_argv, "%d", n_env);
		exec_argv[8] = _nx_copy_string(tmp_argv);
		for (i = 0; i < n_env; i++)
		{
		    exec_argv[9 + i] = environment[i];
		}
		execv(listener_exe, exec_argv);
		nexus_fatal("st_ss_start_node(): exec of \"%s\" failed, errno=%d\n", listener_exe, errno);
	    }
	}
	    
#endif /* BUILD_LITE */
	    
    }

    return 0;
	
} /* st_ss_start_node() */
