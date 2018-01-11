/*
 * init.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/init.c,v 1.63 1996/12/11 05:43:47 tuecke Exp $";

#define NEXUS_DEFINE_GLOBALS
#include "internal.h"

extern nexus_bool_t		_nx_node_should_exit;

nexus_bool_t			_nx_nexus_started = NEXUS_FALSE;

static nexus_mutex_t		shutdown_mutex;
static nexus_cond_t		shutdown_cond;
static nexus_bool_t		do_shutdown = NEXUS_FALSE;

static nexus_bool_t		reached_nexus_start = NEXUS_FALSE;
static int			exit_rc_save = 0;
static nexus_bool_t		secondary_context = NEXUS_FALSE;

static void real_exit(int rc);
static void main_shutdown(nexus_bool_t shutdown_others);
static void finish_shutdown(void);


/*
 * _nx_init_nexus()
 *
 * Initialize nexus and return.
 */
void _nx_init_nexus(int *argc,
		    char ***argv,
		    char *args_env_variable,
		    char *package_designator,
		    int (*package_args_init_func)(int *argc, char ***argv),
		    void (*usage_message_func)(void),
		    int (*new_process_params_func)(char *buf, int size),
		    nexus_module_list_t module_list[],
		    nexus_node_t **nodes,
		    int *n_nodes)
{
    int			local_argc;
    char **		local_argv;
    int *		use_argc;
    char ***		use_argv;
    nexus_bool_t	ignore_command_line_args;

    if (!argc || !argv)
    {
	/* The user does not want back argc/argv */
	local_argc = 0;
	use_argc = &local_argc;
	use_argv = &local_argv;
    }
    else
    {
	use_argc = argc;
	use_argv = argv;
    }
    
    /* SJT: This should not be needed! */
    if (_nx_context_list != NULL)
    {
	/*
	 * nexus_init has already been called
	 */
	secondary_context = NEXUS_TRUE;
	return;
    }
    
    /*
     * This will bootstrap the thread package if necessary, so that
     * basic calls such as printf, malloc, etc can be called.
     */
    ports0_preinit();
#ifndef BUILD_LITE
    _nx_thread_preinit();
#endif /* BUILD_LITE */

    /*
     * Do some global initializations
     */
    _nx_pausing_for_fatal = NEXUS_FALSE;
    _nx_pause_on_fatal = NEXUS_FALSE;
    _nx_stdout = stdout;
    /*
    _nx_stdout = stderr;
    */

    /*
     * Do any startup module preinitizliation.
     * This may be used to fill in _nx_my_process_params.
     */
    ignore_command_line_args = _nx_startup_preinit(module_list);

    /*
     * Split and parse the arguments
     */
    _nx_get_args(use_argc,
		 use_argv,
		 args_env_variable,
		 package_designator,
		 usage_message_func,
		 new_process_params_func,
		 ignore_command_line_args);

    _nx_pausing_for_startup = _nx_pause_on_startup;
    while(_nx_pausing_for_startup) ;
    
    /*
     * Initialize the process.
     * This creates appropriate node and context structures for this
     * process, and fills them into the current thread.
     * Other module inits are called from here.
     */
    _nx_process_init(use_argc,
		     use_argv,
		     package_designator,
		     package_args_init_func,
		     module_list,
		     nodes,
		     n_nodes);

    _nx_args_cleanup(use_argc, use_argv);
    
} /* _nx_init_nexus() */


/*
 * nexus_start()
 */
void nexus_start()
{
    nexus_debug_printf(1, ("nexus_start(): Process starting\n"));

    nexus_mutex_init(&shutdown_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&shutdown_cond, (nexus_condattr_t *) NULL);

    nexus_start_nonblocking();

    if (!_nx_master_node)
    {
	nexus_debug_printf(2, ("nexus_start(): Process not master_node.\n"));

	/*
	 * Suspend until nexus_shutdown() is called,
	 * and then shutdown.
	 */
	nexus_mutex_lock(&shutdown_mutex);
	while (!do_shutdown)
	{
	    nexus_cond_wait(&shutdown_cond, &shutdown_mutex);
	}
	nexus_mutex_unlock(&shutdown_mutex);
	nexus_mutex_destroy(&shutdown_mutex);
	nexus_cond_destroy(&shutdown_cond);

	finish_shutdown();
	real_exit(exit_rc_save);
    }
} /* nexus_start() */


/*
 * nexus_start_nonblocking()
 */
void nexus_start_nonblocking(void)
{
    nexus_debug_printf(1, ("nexus_start_nonblocking(): Process starting\n"));

    /*
     * This is a general purpose flag to indicate that nexus_start()
     * has been run on this process/data segment.  Dynamic context loading
     * under AIX uses this flag to distinguish between the first load call
     * of a new process and the first call to load() for the initial
     * program (which already being in memory, will not be reloaded, but
     * has not had the dynamic data segment data structures initialized
     * from the XCOFF file)
     *
     */
    _nx_nexus_started = NEXUS_TRUE;
    
    if (secondary_context == NEXUS_TRUE)
    {
	return;
    }
    
    nexus_mutex_init(&_nx_orphan_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&_nx_orphan_cond, (nexus_condattr_t *) NULL);
    _nx_num_outstanding_creates = 0;

    reached_nexus_start = NEXUS_TRUE;
    
    _nx_process_start();
    
} /* nexus_start_nonblocking() */


/*
 * nexus_exit()
 *
 * Terminate the computation, with a return code of 'rc'.
 *
 * If 'shutdown' is not 0, then call nexus_shutdown().
 * Have it shutdown other processes.
 */
void nexus_exit(int rc, int shutdown)
{
    nexus_debug_printf(2, ("nexus_exit(): rc %d shutdown %d\n", rc, shutdown));
    exit_rc_save = rc;
    if (shutdown)
    {
	nexus_debug_printf(2, ("nexus_exit(): Going to main_shutdown().\n"));
	main_shutdown(NEXUS_TRUE);
    }
    nexus_debug_printf(2, ("nexus_exit(): Going to Going to real_exit().\n"));
    real_exit(rc);
} /* nexus_exit() */


/*
 * _nx_exit_transient_process()
 *
 * This should be called by a context or non-master node process
 * when it is ready to terminate.  This will shutdown the node
 * without shutting down other processes.
 */
void _nx_exit_transient_process(int rc)
{
    exit_rc_save = rc;
    nexus_debug_printf(2, ("_nx_exit_transient_process(): rc %d.\n", rc));
    main_shutdown(NEXUS_FALSE);
    real_exit(rc);
} /* _nx_exit_transient_process() */


/*
 * nexus_shutdown()
 *
 * Shutdown Nexus in this process, and also have it shutdown other
 * processes.
 */
void nexus_shutdown(void)
{
    nexus_debug_printf(2, ("nexus_shutdown().\n"));
    main_shutdown(NEXUS_TRUE);
} /* nexus_shutdown() */


/*
 * nexus_shutdown_nonexiting()
 */
void nexus_shutdown_nonexiting(void)
{
    nexus_mutex_lock(&_nx_orphan_mutex);
    while(_nx_num_outstanding_creates > 0)
    {
	nexus_debug_printf(1, ("nexus_shutdown_nonexiting(): waiting for %d outstanding creates to complete\n", _nx_num_outstanding_creates));
	nexus_cond_wait(&_nx_orphan_cond, &_nx_orphan_mutex);
    }
    /*
     * I'm not releasing this mutex to prevent any other startups
     * while things are being torn down.
     * i.e. Die with the mutex clutched in my hands.
     */

    nexus_debug_printf(1, ("nexus_shutdown_nonexiting(): Process exiting\n"));

#ifndef BUILD_LITE
    _nx_thread_shutdown_handler_thread();
#endif /* BUILD_LITE */
    _nx_startup_shutdown(NEXUS_FALSE);
    _nx_proto_shutdown(NEXUS_FALSE);
    _nx_rdb_shutdown();
    _nx_nodelock_cleanup();
    nexus_fd_shutdown();
#ifndef BUILD_LITE
    _nx_thread_shutdown();
#endif /* BUILD_LITE */
    ports0_shutdown();
} /* nexus_shutdown_nonexiting() */


/*
 * real_exit()
 *
 * Perform the actual exit.
 */
static void real_exit(int rc)
{
    _nx_md_exit(rc);
} /* real_exit() */


/*
 * main_shutdown()
 *
 * Shutdown Nexus in this process.
 *
 * If the calling thread is not the main thread, then signal
 * the main thread to wake up and terminate.  (Except on the
 * master node.)
 *
 * If shutdown_others==NEXUS_TRUE, then try to shutdown other
 * processes cleanly.  This is useful when called from nexus_exit().
 * If shutdown_others==NEXUS_FALSE, then just shut this process down,
 * leaving other intact.  This is useful when a context process is
 * done and needs to die.  (See _nx_exit_transient_process().)
 */
static void main_shutdown(nexus_bool_t shutdown_others)
{

    nexus_mutex_lock(&_nx_orphan_mutex);
    while(_nx_num_outstanding_creates > 0)
    {
	nexus_debug_printf(1, ("main_shutdown(): waiting for %d outstanding creates to complete\n", _nx_num_outstanding_creates));
	/*
	 * TODO: This should not happen here, since this can be
	 * called from a non-threaded handler
	 */
	nexus_cond_wait(&_nx_orphan_cond, &_nx_orphan_mutex);
    }
    /*
     * I'm not releasing this mutex to prevent any other startups
     * while things are being torn down.
     * i.e. Die with the mutex clutched in my hands.
     */

    nexus_debug_printf(2,
		       ("main_shutdown(): calling _nx_startup_shutdown()\n"));
    _nx_startup_shutdown(shutdown_others);
    
    nexus_debug_printf(2, ("main_shutdown(): calling _nx_proto_shutdown()\n"));
    _nx_proto_shutdown(shutdown_others);
    
    nexus_debug_printf(2, ("main_shutdown(): calling _nx_rdb_shutdown()\n"));
    _nx_rdb_shutdown();
    
    nexus_debug_printf(2, ("main_shutdown(): _nx_master_node=%d, reached_nexus_start=%d\n", _nx_master_node, reached_nexus_start));
    
    if (_nx_master_node || !reached_nexus_start || _nx_node_should_exit)
    {
	finish_shutdown();
    }
    else
    {
	/*
	 * If I'm not the master node, then this must not be
	 * the main thread (which is suspended in nexus_start().
	 * So signal the main thread to finish shutdown.
	 */

        /*
	 * I could be the main thread if this node is exiting
	 * because another node already exists here
	 */
	nexus_mutex_lock(&shutdown_mutex);
	do_shutdown = NEXUS_TRUE;
	nexus_cond_signal(&shutdown_cond);
	nexus_mutex_unlock(&shutdown_mutex);

#ifndef BUILD_LITE
#ifndef NEXUS_ARCH_MPINX
	nexus_thread_exit((void *) NULL);
#endif
#endif /* BUILD_LITE */
    }
} /* main_shutdown() */


/*
 * finish_shutdown()
 *
 * If 'shutdown_others' is NEXUS_TRUE,
 * then try to shutdown all other processes.
 */
static void finish_shutdown(void)
{
#ifdef BUILD_PROFILE    
    int node_id;
    int context_id;
    int thread_id;
    
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    _nx_thread_id(&thread_id);

    _nx_pablo_log_thread_destruction(node_id, context_id, thread_id);
    _nx_pablo_log_context_destruction(node_id, context_id);
    if (!_nx_process_is_context)
    {
	_nx_pablo_log_node_destruction(node_id);
    }
    
    _nx_pablo_shutdown();
#endif
    
    nexus_debug_printf(2, ("finish_shutdown(): calling _nx_nodelock_cleanup()\n"));
    _nx_nodelock_cleanup();
    
#ifndef BUILD_LITE
    nexus_debug_printf(2, ("finish_shutdown(): calling _nx_thread_shutdown_handler_thread()\n"));
    _nx_thread_shutdown_handler_thread();
#endif /* BUILD_LITE */

    nexus_debug_printf(2, ("finish_shutdown(): calling nexus_fd_shutdown()\n"));
    nexus_fd_shutdown();
    
#ifndef BUILD_LITE
    nexus_debug_printf(2, ("finish_shutdown(): calling _nx_thread_shutdown()\n"));
    _nx_thread_shutdown();
#endif /* BUILD_LITE */

    nexus_debug_printf(2, ("finish_shutdown(): calling ports0_shutdown()\n"));
    ports0_shutdown();
    
    _nx_md_shutdown();
} /* finish_shutdown() */


/*
 * nexus_abort()
 *
 * Terminate the computation, using whatever means necessary.
 */
void nexus_abort(void)
{
    _nx_startup_abort();
    _nx_proto_abort();
    _nx_rdb_abort();
    _nx_nodelock_cleanup();
#ifndef BUILD_LITE
    _nx_thread_abort(1);
#endif /* BUILD_LITE */
    _nx_md_abort(1);
} /* nexus_abort() */

/*
 * nexus_set_path()
 *
 * Allow user to set path for certain files and helper programs
 */
void nexus_set_path(nexus_path_type_t path_type,
		    char *path_arg)
{
    int i=0;

    NexusAssert2((path_type < NEXUS_PATH_MAXVAL),
		 ("nexus_set_path(): Invalid path_type:%d\n",path_type));
    do {
	_nx_path_array[path_type][i] = path_arg[i];
    } while( path_arg[i] && ((i++) < NEXUS_MAX_EXECUTABLE_PATH_LENGTH ));
    _nx_path_array[path_type][NEXUS_MAX_EXECUTABLE_PATH_LENGTH] = '\0';
}
