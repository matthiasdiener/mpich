/*
 * th_p0.c
 *
 * General purpose ports0 module.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/th_p0.c,v 1.19 1996/12/12 16:08:51 tuecke Exp $";

#include "internal.h"

#ifndef BUILD_LITE

#define MAX_ERR_SIZE 80
#define _NX_THREAD_GRAN 256
#define _NX_USER_THREAD 0

#if BUILD_DEBUG
static int hasThreads = 0;
#endif

#if defined(TARGET_ARCH_PARAGON) || defined(NEXUS_ARCH_MIT_PTHREADS_1_51_2)
/* Paragon doesn't support changing the scheduler or set priority */
#define SCHED_RR   0
#define SCHED_FIFO 1
#define PRI_FIFO_MIN 0
#define PRI_FIFO_MAX 0
#define PRI_RR_MIN 0
#define PRI_RR_MAX 0
#define MUTEX_NONRECURSIVE_NP 0
#define PTHREAD_DEFAULT_SCHED 0
#endif /* TARGET_ARCH_PARAGON */


#ifndef NEXUS_DEFAULT_STACK_SIZE
#define NEXUS_DEFAULT_STACK_SIZE -1
#endif

static nexus_bool_t		using_handler_thread = NEXUS_FALSE;
static ports0_thread_t		handler_thread;
static nexus_bool_t		handler_thread_done = NEXUS_FALSE;
static nexus_bool_t		handler_thread_exited = NEXUS_FALSE;
static void *			handler_thread_func(void *arg);
static ports0_cond_t		handler_thread_cond;
static ports0_mutex_t		handler_thread_mutex;

static nexus_bool_t		using_idle_thread = NEXUS_FALSE;
static _nx_thread_t *	idle_thread;
static void		idle_func(void);

/*
 * Free list of _nx_thread_t structures
 */
static _nx_thread_t *	thread_freelist;
static nexus_mutex_t	thread_mem_mutex;
static int              next_thread_id;

static void *		thread_starter(void *temparg);
static _nx_thread_t *	new_thread(void);
static void		set_tsd(_nx_thread_t *);


/*
 * _nx_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _nx_thread_usage_message(void)
{
} /* _nx_thread_usage_message() */


/*
 * _nx_thread_new_process_params()
 *
 * Write any new process parameters for this module into 'buf',
 * up to a maximum of 'size' characters.
 *
 * Return:	The number of characters written into 'buf'.
 */
int _nx_thread_new_process_params(char *buf, int size)
{
    return (0);
} /* _nx_thread_new_process_params() */


/*
 * _nx_thread_preinit()
 *
 * If you need to call a thread package initialization routine, then
 * do it here.  This is called as the very first thing in nexus_init().
 *
 * Note: You should not use nexus thread stuff until after
 * _nx_thread_init() is called.
 */
void _nx_thread_preinit( void )
{
} /* _nx_thread_preinit() */


/*
 * _nx_thread_init()
 *
 * This should be used to initialize all the thread related things
 * that Nexus will be using, including things that might be
 * specified by arguments.
 */
void _nx_thread_init(int *argc, char ***argv)
{
    int rc;
    _nx_thread_t *thread;

#ifndef HAVE_THREAD_SAFE_STDIO
    nexus_mutex_init(&nexus_stdio_mutex, (nexus_mutexattr_t *) NULL);
#endif /* HAVE_THREAD_SAFE_STDIO */

    /*
     * Setup thread specific storage which contains
     * a pointer to the _nx_thread_t structure for the thread.
     */
    rc = ports0_thread_key_create(&(nexus_all_global_vars.nexus_thread_t_pointer),
				  NULL);
    _nx_test_rc(rc, "NEXUS: ports0_thread_key_create() failed\n");
    
    nexus_mutex_init(&(thread_mem_mutex), (nexus_mutexattr_t *) NULL);
    next_thread_id = 0;

    ports0_mutex_init(&handler_thread_mutex, (ports0_mutexattr_t *) NULL);
    ports0_cond_init(&handler_thread_cond, (ports0_condattr_t *) NULL);

    /*
     * Initialize the _nx_thread_t structure for this initial thread
     */
    thread = new_thread();
    thread->names = (nexus_thread_name_list_t *) NULL;
    set_tsd(thread);

#if BUILD_DEBUG
    hasThreads = 1;
#endif
    
} /* _nx_thread_init() */


/*
 * _nx_thread_shutdown()
 */
void _nx_thread_shutdown(void)
{
} /* _nx_thread_shutdown() */


/*
 * _nx_thread_abort()
 */
void _nx_thread_abort(int return_code)
{
} /* _nx_thread_abort() */


/*
 * _nx_report_bad_rc()
 */
void _nx_report_bad_rc(int rc, char *message)
{
    char achMessHead[] = "[Thread System]";
    char achDesc[MAX_ERR_SIZE];
    
    if( rc!=_NX_THR_SUCCESS )
    {
	nexus_stdio_lock();
	fprintf( stderr, message );
	nexus_stdio_unlock();
	switch( errno )
	{
	case EAGAIN:
	    strcpy(achDesc, "system out of resources (EAGAIN)");
	    break;
	case ENOMEM:
	    strcpy(achDesc, "insufficient memory (ENOMEM)");
	    break;
	case EINVAL:
	    strcpy(achDesc,
		   "invalid value passed to pthread interface (EINVAL)");
	    break;
	case EPERM:
	    strcpy(achDesc,
		   "user does not have adequate permission (EPERM)");
	    break;
	case ERANGE:
	    strcpy(achDesc, "a parameter has an invalid value (ERANGE)");
	    break;
	case EBUSY:
	    strcpy(achDesc, "mutex is locked (EBUSY)");
	    break;
	case EDEADLK:
	    strcpy(achDesc, "deadlock detected (EDEADLK)");
	    break;
	case ESRCH:
	    strcpy(achDesc, "could not find specified thread (ESRCH)");
	    break;
	default:
	    nexus_fatal("%s %s\n%s unknown error number: %d\n",
			achMessHead, message, achMessHead, errno );
	    break;
	}
	/*
	_nx_imprison_thread( "Error code detected\n" );
	*/
	nexus_fatal("%s %s\n%s %s",
		    achMessHead, message, achMessHead, achDesc);
    }
} /* _nx_report_bad_rc() */


/*
 * new_thread()
 *
 * Allocate and return a _nx_thread_t thread structure.
 */
static _nx_thread_t *new_thread( void )
{
    int i;
    _nx_thread_t *new_thread;
    int mem_req_size;
    
    nexus_mutex_lock(&thread_mem_mutex);
    
    if (thread_freelist == NULL)
    {
	mem_req_size = sizeof(_nx_thread_t) * _NX_THREAD_GRAN;
	NexusMalloc(new_thread(),
		    thread_freelist,
		    _nx_thread_t *, 
		    mem_req_size);
	
	for( i = 0; i < _NX_THREAD_GRAN-1; i++ )
	{
	    thread_freelist[i].next_free = &thread_freelist[i+1];
	}
	thread_freelist[_NX_THREAD_GRAN-1].next_free = NULL;
    }
    new_thread = thread_freelist;
    if (thread_freelist != NULL)
    {
	thread_freelist = thread_freelist->next_free;
    }

    new_thread->id = next_thread_id++;
    
    nexus_mutex_unlock(&thread_mem_mutex);
    
    return (new_thread);
} /* new_thread() */


/*
 * terminate_thread()
 */
static void terminate_thread(_nx_thread_t *thread, void *status)
{
#ifdef BUILD_PROFILE
    int node_id, context_id;
#endif

#ifdef BUILD_PROFILE
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    _nx_pablo_log_thread_destruction(node_id, context_id, thread->id);
#endif /* BUILD_PROFILE */    

    /* Free up the thread storage */
    nexus_mutex_lock(&thread_mem_mutex);
    thread->next_free = thread_freelist;
    thread_freelist = thread;
    nexus_mutex_unlock(&thread_mem_mutex);

    /* Signal the handler thread completion */
    if (ports0_thread_equal(ports0_thread_self(), handler_thread))
    {
	nexus_debug_printf(2, ("terminate_thread(): signaling handler completion\n"));
	ports0_mutex_lock(&handler_thread_mutex);
	handler_thread_exited = NEXUS_TRUE;
	ports0_cond_broadcast(&handler_thread_cond);
	ports0_mutex_unlock(&handler_thread_mutex);
	nexus_debug_printf(2, ("terminate_thread(): done signaling handler completion\n"));
    }
    
    /* Exit the thread */
    ports0_thread_exit(NULL);

} /* terminate_thread() */


/*
 * nexus_thread_exit()
 */
void nexus_thread_exit(void *status)
{
    _nx_thread_t *victim;
    _nx_thread_self(&victim);
    terminate_thread(victim, status);
} /* nexus_thread_exit() */


/*
 * set_tsd()
 *
 * Save the _nx_thread_t thread structure in the pthread's thread
 * specific storage.
 */
static void set_tsd( _nx_thread_t *thread )
{
    int rc;
    rc = ports0_thread_setspecific(nexus_all_global_vars.nexus_thread_t_pointer,
				   (void *) thread );
    _nx_test_rc( rc, "NEXUS: ports0_thread_setspecific() failed\n" );
} /* set_tsd() */


/*
 * thread_starter()
 *
 * Wrapper to get a Nexus thread function started.
 */
static void *thread_starter( void *temparg )
{
    _nx_thread_t *thread;
    void *status;
#ifdef BUILD_PROFILE
    int node_id, context_id;
#endif

    thread = (_nx_thread_t *) temparg;

    set_tsd(thread);

    /* Set the context of this thread */
    nexus_thread_setspecific(_nx_context_key, thread->context);
    
#ifdef BUILD_PROFILE
    _nx_node_id(&node_id);
    _nx_context_id(&context_id);
    _nx_pablo_log_thread_creation(node_id, context_id, thread->id);
#endif /* BUILD_PROFILE */    
    
    /* Call the user function */
    status = (*thread->user_func)(thread->user_arg);
    
    /* Terminate the thread */
    terminate_thread(thread, status);
  
    return (NULL);
} /* thread_starter() */


/*
 * nexus_thread_create
 */
int nexus_thread_create(nexus_thread_t *user_thread,
			nexus_thread_attr_t *attr,
			nexus_thread_func_t func,
			void *user_arg )
{
    int rc;
    _nx_thread_t *thread;
    ports0_thread_t thread_id;

    thread = new_thread();
    thread->names = (nexus_thread_name_list_t *) NULL;
  
    /* Initialize the thread data that needs to be passed to the new thread */
    thread->context = nexus_thread_getspecific(_nx_context_key);
    thread->user_func = func;
    thread->user_arg = user_arg;
  
    rc = ports0_thread_create(&thread_id,
			      NULL,
			      thread_starter,
			      thread);
    _nx_test_rc( rc, "NEXUS: ports0_thread_create() failed\n" );

    if (user_thread)
    {
	*user_thread = thread_id;
    }

    return (0);
} /* nexus_thread_create() */


/*
 * _nx_thread_create_handler_thread()
 *
 * If we have preemptive threads:
 *   Create a normal priority thread that will sit in a
 *   loop calling nexus_poll().
 *
 * If we have non-preemptive threads:
 *   Create a low priority thread which will only become active when
 *   all other threads have suspended.  This thread should:
 *	1) Call nexus_poll_blocking(), to check for available messages.
 *	2) Keep track of idle time.
 *
 *   If a thread module does not support priorities, this same
 *   functionality can be hacked into nexus_cond_wait() using a
 *   counter of suspend threads.
 *
 * TODO: We can probably just use nexus_thread_create() once attributes are
 * implemented.
 */
void _nx_thread_create_handler_thread(void)
{
    int rc;
    _nx_thread_t *thread;

    if (nexus_preemptive_threads())
    {
	nexus_debug_printf(2, ("_nx_thread_create_handler_thread(): Creating handler thread\n"));
	
	using_handler_thread = NEXUS_TRUE;
	thread = new_thread();
	thread->names = (nexus_thread_name_list_t *) NULL;
	thread->context = nexus_thread_getspecific(_nx_context_key);
	thread->user_func = handler_thread_func;
	thread->user_arg = NULL;
	
	rc = ports0_thread_create(&handler_thread,
				  NULL,
				  thread_starter,
				  thread);
	_nx_test_rc( rc, "NEXUS: ports0_thread_create() failed\n" );
    }
    else
    {
	nexus_debug_printf(2, ("_nx_thread_create_handler_thread(): Creating idle thread\n"));

	using_idle_thread = NEXUS_TRUE;
	idle_thread = new_thread();
	idle_thread->names = (nexus_thread_name_list_t *) NULL;
	idle_thread->context = nexus_thread_getspecific(_nx_context_key);

	ports0_idle_callback(idle_func); 
    }
    
} /* _nx_thread_create_handler_thread() */


/*
 * handler_thread_func()
 *
 * TODO: Should count 'unblocked_threads' as they call and return
 * from cond_wait.  Thus, when all threads except this one are blocked,
 * we know we can call nexus_poll_blocking() instead of nexus_poll when
 * using preemptive threads.
 */
static void *handler_thread_func(void *arg)
{
    nexus_debug_printf(2, ("handler_thread_func(): Entering\n"));

    nexus_fd_set_handler_thread(NEXUS_TRUE);
    
    while (!handler_thread_done)
    {
	if (ports0_i_am_only_thread())
	{
	    nexus_poll_blocking();
	}
	else
	{
	    nexus_poll();
	}
	nexus_debug_printf(6, ("handler_thread_func(): calling yield\n"));
	nexus_thread_yield();
	nexus_debug_printf(6, ("handler_thread_func(): Has been scheduled\n"));
    }
    
    nexus_debug_printf(2,("handler_thread_func(): Exiting\n"));
    return(NULL);
} /* handler_thread_func() */


/*
 * idle_func()
 *
 * This is called by ports0 whenever there are no other threads left
 * to schedule.
 */
static void idle_func(void)
{
    _nx_thread_t *thread;
    
    /*
     * Setup this thread properly.
     * If this was a special thread created by ports0, then it will
     * not have had the initialization done to it by thread_starter().
     * So do it here.
     */
    thread = (_nx_thread_t *)ports0_thread_getspecific(nexus_all_global_vars.nexus_thread_t_pointer);
    if (!thread)
    {
	set_tsd(idle_thread);
	nexus_thread_setspecific(_nx_context_key, idle_thread->context);
    }
    
    nexus_debug_printf(2, ("idle_func(): Entering\n"));

    /*
    start_idle_timer();
    */
    nexus_poll_blocking();
    /*
    stop_idle_timer();
    */
    
    nexus_debug_printf(2,("idle_func(): Exiting\n"));

} /* idle_func() */


/*
 * _nx_thread_shutdown_handler_thread()
 */
void _nx_thread_shutdown_handler_thread(void)
{
    int rc;
    /*
    void *status;
    */

    if (using_handler_thread)
    {
	handler_thread_done = NEXUS_TRUE;
	
	/*
	 * If this is not the idle thread, then wait for
	 * the idle thread to terminate.
	 */
	if (!ports0_thread_equal(ports0_thread_self(), handler_thread))
	{
	    /*
	    rc = pthread_join( handler_thread, &status );
	    */
	    nexus_debug_printf(2, ("_nx_thread_shutdown_handler_thread(): waiting for handler thread completion\n"));
	    ports0_mutex_lock(&handler_thread_mutex);
	    while (!handler_thread_exited)
	    {
		rc = ports0_cond_wait(&handler_thread_cond,
				      &handler_thread_mutex);
		_nx_test_rc( rc, "NEXUS: ports0_cond_wait() failed\n" );
	    }
	    ports0_mutex_unlock(&handler_thread_mutex);
	    nexus_debug_printf(2, ("_nx_thread_shutdown_handler_thread(): done waiting for handler thread completion\n"));
	}
    }
    else if (using_idle_thread)
    {
	nexus_debug_printf(2, ("_nx_thread_shutdown_handler_thread(): calling ports0_idle_callback()\n"));
	ports0_idle_callback(NULL);
	nexus_debug_printf(2, ("_nx_thread_shutdown_handler_thread(): returned from ports0_idle_callback()\n"));
    }
} /* _nx_thread_shutdown_handler_thread() */


/*
 * nexus_preemptive_threads
 *
 * Return NEXUS_TRUE (non-zero) if we are using preemptive threads.
 */
nexus_bool_t nexus_preemptive_threads(void)
{
    return (ports0_preemptive_threads());
} /* nexus_preemptive_threads() */


/*
 * nexus_thread_key_create()
 */
int nexus_thread_key_create(nexus_thread_key_t *key,
			    nexus_thread_key_destructor_func_t func)
{
    int rc;
    rc = ports0_thread_key_create(key, func);
    if ((rc < 0) && (errno != EAGAIN))
    {
	_nx_test_rc(rc, "NEXUS: ports0_thread_key_create() failed\n");\
    }
    return (rc);
} /* nexus_key_create() */


/*
 * nexus_thread_setspecific()
 */
#undef nexus_thread_setspecific
int nexus_thread_setspecific(nexus_thread_key_t key,
			     void *value)
{
    int rc;

    rc = nexus_macro_thread_setspecific(key, value);
    _nx_test_rc(rc, "NEXUS: nexus_thread_setspecific() failed\n");
    return (rc);
} /* nexus_thread_setspecific() */


/*
 * nexus_thread_getspecific()
 */
#undef nexus_thread_getspecific
void *nexus_thread_getspecific(nexus_thread_key_t key)
{
    void *value;

    value = nexus_macro_thread_getspecific(key);
    return (value);
} /* nexus_thread_getspecific() */


/*
 * nexus_thread_self()
 */
#undef nexus_thread_self
nexus_thread_t nexus_thread_self( void )
{
    return(nexus_macro_thread_self());
} /* nexus_thread_self() */


/*
 * nexus_thread_equal()
 */
#undef nexus_thread_equal
int nexus_thread_equal(nexus_thread_t t1,
		       nexus_thread_t t2)
{
    return (nexus_macro_thread_equal(t1, t2));
} /* nexus_thread_equal() */


/*
 * nexus_thread_once()
 */
#undef nexus_thread_once
int nexus_thread_once(nexus_thread_once_t *once_control,
		      void (*init_routine)(void))
{
    return (nexus_macro_thread_once(once_control, init_routine));
} /* nexus_thread_once() */


/*
 * nexus_thread_yield
 */
#undef nexus_thread_yield
void nexus_thread_yield( void )
{
    nexus_macro_thread_yield();
} /* nexus_thread_yield() */


/*
 * nexus_mutex_init()
 */
#undef nexus_mutex_init
int nexus_mutex_init(nexus_mutex_t *mut, nexus_mutexattr_t *attr)
{
    int rc;

    _NX_INIT_START_MAGIC_COOKIE(mut);
    rc = nexus_macro_mutex_init(mut, attr);
    _nx_test_rc(rc, "NEXUS: nexus_mutex_init() failed\n");
    _NX_INIT_END_MAGIC_COOKIE(mut);
    return (rc);
} /* nexus_mutex_init() */


/*
 *  nexus_mutex_destroy()
 */
#undef nexus_mutex_destroy
int nexus_mutex_destroy(nexus_mutex_t *mut)
{
    int rc;

    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "destroy");
    rc = nexus_macro_mutex_destroy(mut);
    _nx_test_rc(rc, "NEXUS: nexus_mutex_destroy() failed\n");
    return (rc);
} /* nexus_mutex_destroy() */


/* 
 *  nexus_mutex_lock()
 */
#undef nexus_mutex_lock
int nexus_mutex_lock(nexus_mutex_t *mut)
{
    int rc;

    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "lock before");
    rc = nexus_macro_mutex_lock(mut);
    _nx_test_rc(rc, "NEXUS: nexus_mutex_lock() failed\n");
    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "lock after");
    return (rc);
} /* nexus_mutex_lock() */


/*
 *  nexus_mutex_unlock()
 */
#undef nexus_mutex_unlock
int nexus_mutex_unlock(nexus_mutex_t *mut)
{
    int rc;

    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "unlock before");
    rc = nexus_macro_mutex_unlock(mut);
    _nx_test_rc(rc, "NEXUS: nexus_mutex_unlock() failed\n");
    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "unlock after");
    return (rc);
} /* nexus_mutex_unlock() */


/*
 *  nexus_mutex_trylock()
 */
#undef nexus_mutex_trylock
int nexus_mutex_trylock(nexus_mutex_t *mut)
{
    int rc;

    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "trylock before");
    rc = nexus_macro_mutex_trylock(mut);
    /*
     * trylock is allowed to return non-0 value, so don't call
     * _nx_test_rc() on the return code
     */
#ifdef DONT_INCLUDE
/* 
 * This could probably be checked in all cases except EBUSY, though.
 */
    _nx_test_rc(rc, "NEXUS: nexus_mutex_trylock() failed\n");
#endif
    NEXUS_INTERROGATE(mut, _NX_MUTEX_T, "trylock after");
    return (rc);
} /* nexus_mutex_trylock() */


/*
 * nexus_cond_init()
 */
#undef nexus_cond_init
int nexus_cond_init(nexus_cond_t *cv, nexus_condattr_t *attr)
{
    int rc;

    _NX_INIT_START_MAGIC_COOKIE(cv);
    rc = nexus_macro_cond_init(cv, attr);
    _nx_test_rc(rc, "NEXUS: nexus_cond_init() failed\n");
    _NX_INIT_END_MAGIC_COOKIE(cv);
    return (rc);
} /* nexus_cond_init() */


/*
 *  nexus_cond_destroy()
 */
#undef nexus_cond_destroy
int nexus_cond_destroy(nexus_cond_t *cv)
{
    int rc;

    NEXUS_INTERROGATE(cv, _NX_COND_T, "destroy");
    rc = nexus_macro_cond_destroy(cv);
    _nx_test_rc(rc, "NEXUS: nexus_cond_destroy() failed\n");
    return (rc);
} /* nexus_cond_destroy() */


/*
 *  nexus_cond_wait()
 */
#undef nexus_cond_wait
int nexus_cond_wait(nexus_cond_t *cv, nexus_mutex_t *mut)
{
    int rc;

    NEXUS_INTERROGATE(cv, _NX_COND_T, "wait before");
    rc = nexus_macro_cond_wait(cv, mut);
    NEXUS_INTERROGATE(cv, _NX_COND_T, "wait after");
    _nx_test_rc(rc, "NEXUS: nexus_cond_wait() failed\n");
    return (rc);
} /* nexus_cond_wait() */


/*
 *  nexus_cond_signal()
 */
#undef nexus_cond_signal
int nexus_cond_signal(nexus_cond_t *cv)
{
    int rc;

    NEXUS_INTERROGATE(cv, _NX_COND_T, "signal");
    rc = nexus_macro_cond_signal(cv);
    _nx_test_rc(rc, "NEXUS: nexus_cond_signal() failed\n");
    return (rc);
} /* nexus_cond_signal () */


/*
 *  nexus_cond_broadcast()
 */
#undef nexus_cond_broadcast
int nexus_cond_broadcast(nexus_cond_t *cv)
{
    int rc;

    NEXUS_INTERROGATE(cv, _NX_COND_T, "broadcast");
    rc = nexus_macro_cond_broadcast(cv);
    _nx_test_rc(rc, "NEXUS: nexus_cond_broadcast() failed\n");
    return (rc);
} /* nexus_cond_broadcast() */

void _nx_thread_prefork(void)
{
    ports0_thread_prefork();
}

void _nx_thread_postfork(void)
{
    ports0_thread_postfork();
}

#endif /* BUILD_LITE */
