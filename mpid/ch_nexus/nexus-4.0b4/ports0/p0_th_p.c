/*
 * p0_th_p.c
 *
 * General purpose pthreads module.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_p.c,v 1.24 1997/01/21 22:57:13 tuecke Exp $";

#include "p0_internal.h"

#define MAX_ERR_SIZE 80
#define _P0_THREAD_GRAN 256
#define _P0_USER_THREAD 0

#ifdef BUILD_DEBUG
static int hasThreads = 0;
#endif

#ifdef PORTS0_ARCH_MIT_PTHREADS
static struct sched_param ports0_sched_param;

#define pthread_attr_setsched(a, b) pthread_attr_setschedpolicy(a, b)
#define pthread_attr_setprio(attr, p) \
	(ports0_sched_param.prio = p, \
		pthread_attr_setschedparam(attr, &ports0_sched_param))
#define pthread_setscheduler(t, s, p) \
	(ports0_sched_param.prio = p, \
		pthread_setschedparam(t, s, &ports0_sched_param))

#ifndef PTHREAD_DEFAULT_SCHED
#define PTHREAD_DEFAULT_SCHED 0
#endif
#endif /* PORTS0_ARCH_MIT_PTHREADS */

#ifndef PORTS0_DEFAULT_STACK_SIZE
#define PORTS0_DEFAULT_STACK_SIZE -1
#endif

#ifdef HAVE_PTHREAD_SCHED
static ports0_bool_t	arg_got_sched;
static int	 	scheduler;
static int		priority_min;
static int		priority_max;
static int		priority_mid;
#endif

static ports0_bool_t	preemptive_threads;
static ports0_bool_t	arg_got_stack_size;
static long		stack_size;

static ports0_bool_t	using_idle_thread = PORTS0_FALSE;
static pthread_t	idle_thread;
static ports0_bool_t	idle_thread_done;
static void *		idle_thread_func(void *arg);

/*
 * Free list of _p0_thread_t structures
 */
static _p0_thread_t *	thread_freelist;
static ports0_mutex_t	thread_mem_mutex;
static int              next_thread_id;

static void *		thread_starter(void *temparg);
static _p0_thread_t *	new_thread(void);
static void		set_tsd(_p0_thread_t *);

static void (*idle_func_save)(void);

#define _P0_THR_SUCCESS 0
#define _p0_test_rc(rc, msg) \
do {\
    if (rc != _P0_THR_SUCCESS) _p0_report_bad_rc(rc, msg); \
} while(0)


/*
 * _p0_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _p0_thread_usage_message(void)
{
#ifdef HAVE_PTHREAD_SCHED
    printf("	-sched <string>           : Specify which scheduler to use:\n");
    printf("                                <string> = rr: round robin (default)\n");
    printf("                                <string> = fifo: first in first out\n");
#endif /* HAVE_PTHREAD_SCHED */
    printf("	-stack <integer>          : Set the thread stack size.\n");
} /* _p0_thread_usage_message() */


/*
 * _p0_thread_new_process_params()
 *
 * Write any new process parameters for this module into 'buf',
 * up to a maximum of 'size' characters.
 *
 * Return:	The number of characters written into 'buf'.
 */
int _p0_thread_new_process_params(char *buf, int size)
{
    char tmp_buf1[256];
    char tmp_buf2[128];
    int n_added = 0;
    
    tmp_buf1[0] = '\0';

#ifdef HAVE_PTHREAD_SCHED
    if (arg_got_sched)
    {
	if (scheduler == SCHED_FIFO)
	{
	    strcat(tmp_buf1, "-sched fifo ");
	}
	else /* (scheduler == SCHED_RR) */
	{
	    strcat(tmp_buf1, "-sched rr ");
	}
    }
#endif /* HAVE_PTHREAD_SCHED */

    if (arg_got_stack_size)
    {
	sprintf(tmp_buf2, "-stack %ld ", stack_size);
	strcat(tmp_buf1, tmp_buf2);
    }
    
    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
	ports0_fatal("_p0_thread_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    
    strcpy(buf, tmp_buf1);
    
    return (n_added);
    
} /* _p0_thread_new_process_params() */


/*
 * _p0_thread_preinit()
 *
 * If you need to call a thread package initialization routine, then
 * do it here.  This is called as the very first thing in ports0_init().
 *
 * Note: You should not use ports0 thread stuff until after
 * _p0_thread_init() is called.
 */
void _p0_thread_preinit( void )
{
#ifdef HAVE_PTHREAD_INIT_FUNC
    /*
     * FSU pthreads uses its own read/write/malloc routines, which
     * might get called during the argument handling, before
     * _p0_thread_init() is called.
     */
    pthread_init();
#endif /* HAVE_PTHREAD_INIT_FUNC */
} /* _p0_thread_preinit() */


/*
 * _p0_thread_init()
 *
 * This should be used to initialize all the thread related things
 * that Ports0 will be using, including things that might be
 * specified by arguments.
 */
void _p0_thread_init(int *argc, char **argv[])
{
    int rc;
    _p0_thread_t *thread;
    int arg_num;
    char *arg;
#if defined(HAVE_PTHREAD_SCHED) && !defined(HAVE_PTHREAD_DRAFT_4) && !defined(HAVE_PTHREAD_DRAFT_6)
    struct sched_param my_sched_param;
#endif

#ifdef HAVE_PTHREAD_SCHED
    /* Get the -sched command line argument */
    arg_got_sched = PORTS0_FALSE;
    scheduler = SCHED_RR;
    if ((arg_num = ports0_find_argument(argc, argv, "sched", 2)) >= 0)
    {
	arg = (*argv)[arg_num + 1];
	if (strcmp(arg, "fifo") == 0)
	{
	    scheduler = SCHED_FIFO;
	    arg_got_sched = PORTS0_TRUE;
	}
	else if (strcmp(arg, "rr") == 0)
	{
	    scheduler = SCHED_RR;
	    arg_got_sched = PORTS0_TRUE;
	}
	else
	{
	    ports0_fatal("-sched argument must be either \"fifo\" or \"rr\"\n");
	}
	ports0_remove_arguments(argc, argv, arg_num, 2);
    }
#endif /* HAVE_PTHREAD_SCHED */

    /* Get the -stack command line argument */
    if ((arg_num = ports0_find_argument(argc, argv, "stack", 2)) >= 0)
    {
	stack_size = atol((*argv)[arg_num + 1]);
	arg_got_stack_size = PORTS0_TRUE;
	ports0_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	stack_size = (long) PORTS0_DEFAULT_STACK_SIZE;
	arg_got_stack_size = PORTS0_FALSE;
    }

#ifdef HAVE_PTHREAD_DRAFT_4
    /*
     * Set the default attributes for mutex and condition
     */
    rc = pthread_mutexattr_create(&(ports0_all_global_vars.mutexattr));
    _p0_test_rc(rc, "PORTS0: pthread_mutexattr_create() failed\n");

#ifndef HAVE_NO_PTHREAD_SETKIND
#ifdef BUILD_DEBUG    
    rc = pthread_mutexattr_setkind_np(&(ports0_all_global_vars.mutexattr),
				      MUTEX_NONRECURSIVE_NP );
#else  /* BUILD_DEBUG */    
    rc = pthread_mutexattr_setkind_np(&(ports0_all_global_vars.mutexattr),
				      MUTEX_FAST_NP );
#endif /* BUILD_DEBUG */    
    _p0_test_rc(rc, "PORTS0: pthread_mutexattr_setkind() failed\n");
#endif /* HAVE_NO_PTHREAD_SETKIND */
    
#ifdef HAVE_NO_CONDATTR_DEFAULT
    rc = pthread_condattr_create(&(ports0_all_global_vars.condattr));
    _nx_test_rc(rc, "PORTS0: pthread_condattr_create() failed\n");
#else  /* HAVE_NO_CONDATTR_DEFAULT */
    ports0_all_global_vars.condattr = pthread_condattr_default;
#endif /* HAVE_NO_CONDATTR_DEFAULT */
    
#endif /* HAVE_PTHREAD_DRAFT_4 */

    
#ifndef HAVE_THREAD_SAFE_STDIO
    rc = ports0_mutex_init(&ports0_reentrant_mutex, (ports0_mutexattr_t *) NULL);
    _p0_test_rc(rc, "PORTS0: ports0_mutex_init() failed\n");
#endif /* HAVE_THREAD_SAFE_STDIO */

    /*
     * Define the parameters for the scheduler being used
     */
#ifdef HAVE_PTHREAD_SCHED
    if (scheduler == SCHED_FIFO)
    {
        preemptive_threads = PORTS0_FALSE;
#if defined(HAVE_PTHREAD_DRAFT_4) || defined(PORTS0_ARCH_MIT_PTHREADS_1_51_2)
	priority_min = PRI_FIFO_MIN;
	priority_max = PRI_FIFO_MAX;
#elif defined(PORTS0_ARCH_MIT_PTHREADS)
	priority_min = PTHREAD_MIN_PRIORITY;
	priority_max = PTHREAD_MAX_PRIORITY;
#elif defined(HAVE_PTHREAD_PRIO_MINMAX)
	priority_min = PTHREAD_PRIO_MIN;
	priority_max = PTHREAD_PRIO_MAX;
#else
	priority_min = sched_get_priority_min( SCHED_FIFO );
	priority_max = sched_get_priority_max( SCHED_FIFO );
#endif
	priority_mid = (priority_min + priority_max) / 2;
    }
    else /* (scheduler == SCHED_RR) */
    {
	preemptive_threads = PORTS0_TRUE;
#if defined(HAVE_PTHREAD_DRAFT_4) || defined(PORTS0_ARCH_MIT_PTHREADS_1_51_2)
	priority_min = PRI_RR_MIN;
	priority_max = PRI_RR_MAX;
#elif defined(PORTS0_ARCH_MIT_PTHREADS)
	priority_min = PTHREAD_MIN_PRIORITY;
	priority_max = PTHREAD_MAX_PRIORITY;
#elif defined(HAVE_PTHREAD_PRIO_MINMAX)
	priority_min = PTHREAD_PRIO_MIN;
	priority_max = PTHREAD_PRIO_MAX;
#else
	priority_min = sched_get_priority_min( SCHED_RR );
	priority_max = sched_get_priority_max( SCHED_RR );
#endif
	priority_mid = (priority_min + priority_max) / 2;
    }
#else  /* HAVE_PTHREAD_SCHED */
#ifdef HAVE_PTHREAD_PREEMPTIVE
    preemptive_threads = PORTS0_TRUE;
#else
    preemptive_threads = PORTS0_FALSE;
#endif
#endif /* HAVE_PTHREAD_SCHED */

    /*
     * Setup the default thread attributes
     */
#ifdef HAVE_PTHREAD_DRAFT_4
    rc = pthread_attr_create(&(ports0_all_global_vars.threadattr));
    _p0_test_rc( rc, "PORTS0: pthread_attr_create() failed\n" );
#else
    rc = pthread_attr_init(&(ports0_all_global_vars.threadattr));
    _p0_test_rc( rc, "PORTS0: pthread_attr_init() failed\n" );
#endif

#ifdef HAVE_PTHREAD_SCHED
#if defined(HAVE_PTHREAD_DRAFT_4) || defined(HAVE_PTHREAD_DRAFT_6)
    rc = pthread_attr_setinheritsched(&(ports0_all_global_vars.threadattr),
				      PTHREAD_DEFAULT_SCHED);
    _p0_test_rc( rc, "PORTS0: pthread_attr_setinheritsched() failed\n" );
    rc = pthread_attr_setsched(&(ports0_all_global_vars.threadattr),
			       scheduler);
    _p0_test_rc( rc, "PORTS0: pthread_attr_setsched() failed\n" );
    rc = pthread_attr_setprio(&(ports0_all_global_vars.threadattr),
			      priority_mid);
    _p0_test_rc( rc, "PORTS0: pthread_attr_setprio() failed\n" );
#else
    rc = pthread_attr_setinheritsched(&(ports0_all_global_vars.threadattr),
				      PTHREAD_EXPLICIT_SCHED);
    _p0_test_rc( rc, "PORTS0: pthread_attr_setinheritsched() failed\n" );
    my_sched_param.sched_policy = scheduler;
    my_sched_param.sched_priority = priority_mid;
    rc = pthread_attr_setschedparam(&(ports0_all_global_vars.threadattr),
				    &my_sched_param);
    _p0_test_rc( rc, "PORTS0: pthread_attr_setschedparam() failed\n" );
#endif
#endif /* HAVE_PTHREAD_SCHED */
    
    if (stack_size > 0)
    {
	rc = pthread_attr_setstacksize(&(ports0_all_global_vars.threadattr),
				       stack_size);
	_p0_test_rc( rc, "PORTS0: pthread_attr_setstacksize() failed\n" );
    }
    
    /*
     * Make this initial thread have the default thread attributes
     */
#ifdef HAVE_PTHREAD_SCHED
#if defined(HAVE_PTHREAD_DRAFT_4) || defined(PORTS0_ARCH_MIT_PTHREADS)
    pthread_setscheduler(pthread_self(), scheduler, priority_mid);
    /*
     * Note: Do not check this return code for 0.  It apparently returns 
     * the old scheduler.
     */
#elif defined(HAVE_PTHREAD_DRAFT_6)
    rc = pthread_setschedattr(pthread_self(),
			      ports0_all_global_vars.threadattr);
    _p0_test_rc(rc, "PORTS0: pthread_setschedattr() failed\n");
#else
    rc = pthread_setschedparam(pthread_self(),
			       my_sched_param.sched_policy,
			       &my_sched_param);
    _p0_test_rc(rc, "PORTS0: pthread_setschedparam() failed\n");
#endif
#endif /* HAVE_PTHREAD_SCHED */

    /*
     * Setup thread specific storage which contains
     * a pointer to the _p0_thread_t structure for the thread.
     */
    rc = ports0_thread_key_create(
			     &(ports0_all_global_vars.ports0_thread_t_pointer),
			     NULL);
    _p0_test_rc(rc, "PORTS0: pthread_key_create() failed\n");
    
    ports0_mutex_init(&(thread_mem_mutex), (ports0_mutexattr_t *) NULL);
    next_thread_id = 0;

    /*
     * Initialize the _p0_thread_t structure for this initial thread
     */
    thread = new_thread();
    thread->names = (ports0_thread_name_list_t *) NULL;
    set_tsd(thread);

#ifdef BUILD_DEBUG
    hasThreads = 1;
#endif    
} /* _p0_thread_init() */


/*
 * _p0_thread_shutdown()
 */
int _p0_thread_shutdown(void)
{
    return (0);
} /* _p0_thread_shutdown() */


/*
 * _p0_thread_abort()
 */
void _p0_thread_abort(int return_code)
{
} /* _p0_thread_abort() */


/*
 * _p0_report_bad_rc()
 */
void _p0_report_bad_rc(int rc, char *message)
{
    char achMessHead[] = "[Thread System]";
    char achDesc[MAX_ERR_SIZE];
    
    if (rc != _P0_THR_SUCCESS)
    {
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
	    ports0_fatal("%s %s\n%s unknown error number: %d\n",
			 achMessHead, message, achMessHead, errno );
	    break;
	}
	/*
	_p0_imprison_thread( "Error code detected\n" );
	*/
	ports0_fatal("%s %s\n%s %s",
		     achMessHead, message, achMessHead, achDesc);
    }
} /* _p0_report_bad_rc() */


/*
 * new_thread()
 *
 * Allocate and return a _p0_thread_t thread structure.
 */
static _p0_thread_t *new_thread( void )
{
    int i;
    _p0_thread_t *new_thread;
    int mem_req_size;
    
    ports0_mutex_lock(&thread_mem_mutex);
    
    if (thread_freelist == NULL)
    {
	mem_req_size = sizeof(_p0_thread_t) * _P0_THREAD_GRAN;
	Ports0Malloc(new_thread(),
		     thread_freelist,
		     _p0_thread_t *, 
		     mem_req_size);
	
	for( i = 0; i < _P0_THREAD_GRAN-1; i++ )
	{
	    thread_freelist[i].next_free = &thread_freelist[i+1];
	}
	thread_freelist[_P0_THREAD_GRAN-1].next_free = NULL;
    }
    new_thread = thread_freelist;
    if (thread_freelist != NULL)
    {
		thread_freelist = thread_freelist->next_free;
    }

    new_thread->id = next_thread_id++;
    
    ports0_mutex_unlock(&thread_mem_mutex);
    
    return (new_thread);
} /* new_thread() */


/*
 * terminate_thread()
 */
static void terminate_thread(_p0_thread_t *thread,
			     void *status,
			     ports0_bool_t really_terminate)
{
#ifdef BUILD_PROFILE
    /*
    log_thread_destruction(thread->id);
    */
#endif /* BUILD_PROFILE */    

    /* Free up the thread storage */
    ports0_mutex_lock(&thread_mem_mutex);
    thread->next_free = thread_freelist;
    thread_freelist = thread;
    ports0_mutex_unlock(&thread_mem_mutex);

    /* Exit the thread */
    if (really_terminate)
    {
	pthread_exit(NULL);
    }

} /* terminate_thread() */


/*
 * ports0_thread_exit()
 */
void ports0_thread_exit(void *status)
{
    _p0_thread_t *victim;
    _p0_thread_self(&victim);
    terminate_thread(victim, status, PORTS0_TRUE);
} /* ports0_thread_exit() */


/*
 * set_tsd()
 *
 * Save the _p0_thread_t thread structure in the pthread's thread
 * specific storage.
 */
static void set_tsd( _p0_thread_t *thread )
{
    ports0_thread_setspecific(ports0_all_global_vars.ports0_thread_t_pointer,
			      (void *) thread );
} /* set_tsd() */


/*
 * thread_starter()
 *
 * Wrapper to get a Ports0 thread function started.
 */
static void *thread_starter( void *temparg )
{
    _p0_thread_t *thread;
    void *status;

    thread = (_p0_thread_t *) temparg;

    set_tsd(thread);

#ifdef BUILD_PROFILE
    /*
    log_thread_creation(thread->id);
    */
#endif /* BUILD_PROFILE */    
    
    /* Call the user function */
    status = (*thread->user_func)(thread->user_arg);
    
    /* Terminate the thread */
    terminate_thread(thread, status, PORTS0_FALSE);
  
    return (NULL);
} /* thread_starter() */


/*
 * ports0_thread_create
 */
int ports0_thread_create(ports0_thread_t *user_thread,
			 ports0_threadattr_t *attr,
			 ports0_thread_func_t func,
			 void *user_arg )
{
    int rc;
    _p0_thread_t *thread;
    pthread_t thread_id;

    thread = new_thread();
    thread->names = (ports0_thread_name_list_t *) NULL;
  
    /* Initialize the thread data that needs to be passed to the new thread */
    thread->user_func = func;
    thread->user_arg = user_arg;
  
#if defined(HAVE_PTHREAD_DRAFT_8) || defined(HAVE_PTHREAD_DRAFT_10)
    rc = pthread_attr_setdetachstate(attr ? attr : &(ports0_all_global_vars.threadattr), PTHREAD_CREATE_DETACHED);
    _p0_test_rc(rc, "PORTS0:pthread_attr_setdetachstate() failed\n");
#endif

    rc = pthread_create(&thread_id,
#ifdef HAVE_PTHREAD_DRAFT_4			
			(attr ? *attr : ports0_all_global_vars.threadattr),
#else			
			(attr ? attr : &(ports0_all_global_vars.threadattr)),
#endif
			thread_starter,
			thread);
    _p0_test_rc( rc, "PORTS0: pthread_create() failed\n" );

    /*
     * Note: With AIX 3.1.x DCE threads, the pthread_detach(&thread_id)
     * wipes out the thread_id structure.  So we need to assign
     * it to the user thread here, before the detach.
     */
    if (user_thread)
    {
	*user_thread = thread_id;
    }

#if !defined(HAVE_PTHREAD_DRAFT_8) && !defined(HAVE_PTHREAD_DRAFT_10)
#ifdef PORTS0_ARCH_MIT_PTHREADS
    rc = pthread_detach(thread_id);
#else
    rc = pthread_detach(&thread_id);
#endif /* PORTS0_ARCH_MIT_PTHREADS */
#endif /* !HAVE_PTHREAD_DRAFT_8 && !HAVE_PTHREAD_DRAFT_10 */
    _p0_test_rc( rc, "PORTS0: pthread_detach() failed\n" );

    return (0);
} /* ports0_thread_create() */


/*
 * ports0_idle_callback()
 *
 * Create a low priority thread which will only become active when
 * all other threads have suspended.  This thread should call
 * the supplied 'idle_func' function.
 *
 * If a thread module does not support priorities, this same
 * functionality can be hacked into ports0_cond_wait() using a
 * counter of suspend threads.
 *
 * If 'idle_func' is NULL, then shutdown a previously created idle thread.
 *
 * TODO: We can probably just use ports0_thread_create() once attributes are
 * implemented.
 */
void ports0_idle_callback(void (*idle_func)(void))
{
    int rc;
    void *status;
    _p0_thread_t *thread;
    pthread_attr_t my_attr;
#if defined(HAVE_PTHREAD_SCHED) && !defined(HAVE_PTHREAD_DRAFT_4) && !defined(HAVE_PTHREAD_DRAFT_6)
    struct sched_param my_sched_param;
#endif

    if (idle_func)
    {
	if (using_idle_thread)
	{
	    ports0_debug_printf(2,("ports0_idle_callback(): Changing idle callback for existing idle thread\n"));
	    idle_func_save = idle_func;
	}
	else
	{
	    ports0_debug_printf(2, ("ports0_idle_callback(): Creating idle thread\n"));
    
	    using_idle_thread = PORTS0_TRUE;
	    idle_thread_done = PORTS0_FALSE;
	    idle_func_save = idle_func;
	    
	    thread = new_thread();
	    thread->names = (ports0_thread_name_list_t *) NULL;
	    thread->user_func = idle_thread_func;
	    thread->user_arg = NULL;
	    
#ifdef HAVE_PTHREAD_DRAFT_4
	    rc = pthread_attr_create(&my_attr);
	    _p0_test_rc( rc, "PORTS0: pthread_attr_create() failed\n" );
#else
	    rc = pthread_attr_init(&my_attr);
	    _p0_test_rc( rc, "PORTS0: pthread_attr_init() failed\n" );
#endif
	    
#ifdef HAVE_PTHREAD_SCHED
#if defined(HAVE_PTHREAD_DRAFT_4) || defined(HAVE_PTHREAD_DRAFT_6)
	    rc = pthread_attr_setinheritsched(&my_attr,
					     PTHREAD_DEFAULT_SCHED);
	    _p0_test_rc(rc, "PORTS0: pthread_attr_setinheritsched() failed\n");
	    rc = pthread_attr_setsched(&my_attr, scheduler);
	    _p0_test_rc(rc, "PORTS0: pthread_attr_setsched() failed\n");
	    rc = pthread_attr_setprio(&my_attr, priority_min);
	    _p0_test_rc(rc, "PORTS0: pthread_attr_setprio() failed\n");
#else
	    rc = pthread_attr_setinheritsched(&my_attr,
					     PTHREAD_EXPLICIT_SCHED);
	    _p0_test_rc(rc, "PORTS0: pthread_attr_setinheritsched() failed\n");
	    my_sched_param.sched_policy = scheduler;
	    my_sched_param.sched_priority = priority_min;
	    rc = pthread_attr_setschedparam(&my_attr,
					    &my_sched_param);
	    _p0_test_rc( rc, "PORTS0: pthread_attr_setschedparam() failed\n" );
#endif
#endif
	    
	    if (stack_size > 0)
	    {
		rc = pthread_attr_setstacksize(&my_attr, stack_size);
		_p0_test_rc(rc,"PORTS0: pthread_attr_setstacksize() failed\n");
	    }
	    
	    rc = pthread_create(&idle_thread,
#ifdef HAVE_PTHREAD_DRAFT_4			
				my_attr,
#else			
				&my_attr,
#endif
				thread_starter,
				thread);
	    _p0_test_rc( rc, "PORTS0: pthread_create() failed\n" );
	    
#ifdef HAVE_PTHREAD_DRAFT_4
	    rc = pthread_attr_delete(&my_attr);
	    _p0_test_rc( rc, "PORTS0: pthread_attr_delete() failed\n" );
#else
	    rc = pthread_attr_destroy(&my_attr);
	    _p0_test_rc( rc, "PORTS0: pthread_attr_destroy() failed\n" );
#endif /* HAVE_PTHREAD_DRAFT_4 */
	}
    }
    else
    {
	if (using_idle_thread)
	{
	    ports0_debug_printf(2,("ports0_idle_callback(): Shutting down idle thread\n"));
	    
	    idle_thread_done = PORTS0_TRUE;
	
	    /*
	     * If this is not the idle thread, then wait for
	     * the idle thread to terminate.
	     */
	    if (!pthread_equal(pthread_self(), idle_thread))
	    {
		rc = pthread_join( idle_thread, &status );
		_p0_test_rc( rc, "PORTS0: pthread_join() failed\n" );
	    }
	    
	    using_idle_thread = PORTS0_FALSE;
	}
    }

} /* ports0_idle_callback() */


/*
 * idle_thread_func()
 */
static void *idle_thread_func(void *arg)
{
    ports0_debug_printf(2, ("idle_thread_func(): Entering\n"));

    while (!idle_thread_done)
    {
        (*idle_func_save)();
	ports0_debug_printf(6, ("idle_thread_func(): calling yield\n"));
	ports0_macro_thread_yield();
	ports0_debug_printf(6, ("idle_thread_func(): Has been scheduled\n"));
    }
    
    ports0_debug_printf(2,("idle_thread_func(): Exiting\n"));
    
    return (NULL);
} /* idle_thread_func() */


/*
 * ports0_preemptive_threads
 *
 * Return PORTS0_TRUE (non-zero) if we are using preemptive threads.
 */
ports0_bool_t ports0_preemptive_threads(void)
{
    return (preemptive_threads);
} /* ports0_preemptive_threads() */



/*
 * ports0_threadattr_init()
 */
#undef ports0_threadattr_init
int ports0_threadattr_init(ports0_threadattr_t *attr)
{
    return (ports0_macro_threadattr_init(attr));
}

/*
 * ports0_threadattr_destroy()
 */
#undef ports0_threadattr_destroy
int ports0_threadattr_destroy(ports0_threadattr_t *attr)
{
    return (ports0_macro_threadattr_destroy(attr));
}

/*
 * ports0_threadattr_setstacksize()
 */
#undef ports0_threadattr_setstacksize
int ports0_threadattr_setstacksize(ports0_threadattr_t *attr,
				   size_t stacksize)
{
    return (ports0_macro_threadattr_setstacksize(attr, stacksize));
}

/*
 * ports0_threadattr_getstacksize()
 */
#undef ports0_threadattr_getstacksize
int ports0_threadattr_getstacksize(ports0_threadattr_t *attr,
				   size_t *stacksize)
{
    return (ports0_macro_threadattr_getstacksize(attr, stacksize));
}

/*
 * ports0_thread_key_create()
 */
#undef ports0_thread_key_create
int ports0_thread_key_create(ports0_thread_key_t *key,
			ports0_thread_key_destructor_func_t destructor_func)
{
    int rc;
    rc = ports0_macro_thread_key_create(key, destructor_func);
    if (rc != 0 && rc != EAGAIN)
    {
	_p0_test_rc(rc, "PORTS0: ports0_thread_key_create() failed\n");\
    }
    return(rc);
} /* ports0_thread_key_create() */


/*
 * ports0_thread_key_delete()
 */
#undef ports0_thread_key_delete
int ports0_thread_key_delete(ports0_thread_key_t key)
{
    int rc;
    rc = ports0_macro_thread_key_delete(key);
    _p0_test_rc(rc, "PORTS0: ports0_thread_key_delete() failed\n");\
    return(rc);
} /* ports0_thread_key_delete() */


/*
 * ports0_thread_setspecific()
 */
#undef ports0_thread_setspecific
int ports0_thread_setspecific(ports0_thread_key_t key,
			      void *value)
{
    int rc;
    rc = ports0_macro_thread_setspecific(key, value);
    _p0_test_rc(rc, "PORTS0: ports0_thread_setspecific() failed\n");
    return(rc);
} /* ports0_thread_setspecific() */


/*
 * ports0_thread_getspecific()
 */
#undef ports0_thread_getspecific
void *ports0_thread_getspecific(ports0_thread_key_t key)
{
    void *value;

    value = ports0_macro_thread_getspecific(key);
    return (value);
} /* ports0_thread_getspecific() */

#ifdef _P0_THREAD_GETSPECIFIC
void *_p0_thread_getspecific(ports0_thread_key_t key)
{
    void *value;

    pthread_getspecific(key, &value);
    return (value);
} /* _p0_thread_getspecific() */
#endif /* _P0_THREAD_GETSPECIFIC */

/*
 * ports0_thread_self()
 */
#undef ports0_thread_self
ports0_thread_t ports0_thread_self(void)
{
    return(ports0_macro_thread_self());
} /* ports0_thread_self() */


/*
 * ports0_thread_equal()
 */
#undef ports0_thread_equal
int ports0_thread_equal(ports0_thread_t t1,
			ports0_thread_t t2)
{
    return (ports0_macro_thread_equal(t1, t2));
} /* ports0_thread_equal() */


/*
 * ports0_thread_once()
 */
#undef ports0_thread_once
int ports0_thread_once(ports0_thread_once_t *once_control,
		       void (*init_routine)(void))
{
    return (ports0_macro_thread_once(once_control, init_routine));
} /* ports0_thread_once() */


/*
 * ports0_thread_yield
 */
#undef ports0_thread_yield
void ports0_thread_yield(void)
{
    ports0_macro_thread_yield();
} /* ports0_thread_yield() */


/*
 * ports0_i_am_only_thread()
 */
#undef ports0_i_am_only_thread
ports0_bool_t ports0_i_am_only_thread(void)
{
    return (ports0_macro_i_am_only_thread());
}


/*
 * ports0_mutexattr_init()
 */
#undef ports0_mutexattr_init
int ports0_mutexattr_init(ports0_mutexattr_t *attr)
{
    int rc;
    rc = ports0_macro_mutexattr_init(attr);
    _p0_test_rc(rc, "PORTS0: pthread_mutexattr_init() failed\n");
    return (rc);
}

/*
 * ports0_mutexattr_destroy()
 */
#undef ports0_mutexattr_destroy
int ports0_mutexattr_destroy(ports0_mutexattr_t *attr)
{
    int rc;
    rc = ports0_macro_mutexattr_destroy(attr);
    _p0_test_rc(rc, "PORTS0: pthread_mutexattr_destroy() failed\n");
    return (rc);
}

/*
 * ports0_mutex_init()
 */
#undef ports0_mutex_init
int ports0_mutex_init(ports0_mutex_t *mut, ports0_mutexattr_t *attr)
{
    int rc;
    _P0_INIT_START_MAGIC_COOKIE(mut);
    rc = ports0_macro_mutex_init(mut, attr);
    _P0_INIT_END_MAGIC_COOKIE(mut);
    _p0_test_rc( rc, "PORTS0: pthread_mutex_init() failed\n" );
    return(rc);
} /* ports0_mutex_init() */


/*
 *  ports0_mutex_destroy()
 */
#undef ports0_mutex_destroy
int ports0_mutex_destroy(ports0_mutex_t *mut)
{
    int rc; 
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "destroy" ); 
    rc = ports0_macro_mutex_destroy(mut);
    _p0_test_rc( rc, "PORTS0: pthread_mutex_destroy() failed\n" );
    return(rc);
} /* ports0_mutex_destroy() */


/* 
 *  ports0_mutex_lock()
 */
#undef ports0_mutex_lock
int ports0_mutex_lock(ports0_mutex_t *mut)
{
    int rc;
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "lock before" );
    rc = ports0_macro_mutex_lock(mut);
    _p0_test_rc( rc, "PORTS0: pthread_mutex_lock() failed\n" );
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "lock after" );
    return(rc);
} /* ports0_mutex_lock() */


/* 
 *  ports0_mutex_trylock()
 */
#undef ports0_mutex_trylock
int ports0_mutex_trylock(ports0_mutex_t *mut)
{
    int rc;
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "trylock before" );
    rc = ports0_macro_mutex_trylock(mut);
    if (rc != EBUSY)
    {
	_p0_test_rc( rc, "PORTS0: pthread_mutex_trylock() failed\n" );
    }
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "trylock after" );
    return(rc);
} /* ports0_mutex_trylock() */


/*
 *  ports0_mutex_unlock()
 */
#undef ports0_mutex_unlock
int ports0_mutex_unlock(ports0_mutex_t *mut)
{
    int rc;
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "unlock before" );
    rc = ports0_macro_mutex_unlock(mut);
    _p0_test_rc( rc, "PORTS0: pthread_mutex_unlock() failed\n" );
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "unlock after" );
    return(rc);
} /* ports0_mutex_unlock() */


/*
 * ports0_condattr_init()
 */
#undef ports0_condattr_init
int ports0_condattr_init(ports0_condattr_t *attr)
{
    int rc;
    rc = ports0_macro_condattr_init(attr);
    return (rc);
}

/*
 * ports0_condattr_destroy()
 */
#undef ports0_condattr_destroy
int ports0_condattr_destroy(ports0_condattr_t *attr)
{
    int rc;
    rc = ports0_macro_condattr_destroy(attr);
    return (rc);
}

/*
 * ports0_cond_init()
 */
#undef ports0_cond_init
int ports0_cond_init(ports0_cond_t *cv, ports0_condattr_t *attr)
{
    int rc;
    _P0_INIT_START_MAGIC_COOKIE(cv); 
    rc = ports0_macro_cond_init(cv, attr);
    _p0_test_rc( rc, "PORTS0: pthread_cond_init() failed\n" );
    _P0_INIT_END_MAGIC_COOKIE(cv); 
    return(rc);
} /* ports0_cond_init() */


/*
 *  ports0_cond_destroy()
 */
#undef ports0_cond_destroy
int ports0_cond_destroy(ports0_cond_t *cv)
{
    int rc; 
    PORTS0_INTERROGATE( cv, _P0_COND_T, "destroy" ); 
    rc = ports0_macro_cond_destroy(cv);
    _p0_test_rc( rc, "PORTS0: pthread_cond_destroy() failed\n" );
    return(rc);
} /* ports0_cond_destroy() */


/*
 *  ports0_cond_wait()
 */
#undef ports0_cond_wait
int ports0_cond_wait(ports0_cond_t *cv, ports0_mutex_t *mut)
{
    int rc; \
    PORTS0_INTERROGATE( cv, _P0_COND_T, "wait before" ); \
    rc = ports0_macro_cond_wait(cv, mut);
    PORTS0_INTERROGATE( cv, _P0_COND_T, "wait after" ); \
    _p0_test_rc( rc, "PORTS0: pthread_cond_wait() failed\n" );
    return(rc);
} /* ports0_cond_wait() */


/*
 *  ports0_cond_signal()
 */
#undef ports0_cond_signal
int ports0_cond_signal(ports0_cond_t *cv)
{
    int rc; 
    PORTS0_INTERROGATE( cv, _P0_COND_T, "signal" ); 
    rc = ports0_macro_cond_signal(cv); 
    _p0_test_rc( rc, "PORTS0: pthread_cond_signal() failed\n" );
    return(rc);
} /* ports0_cond_signal () */


/*
 *  ports0_cond_broadcast()
 */
#undef ports0_cond_broadcast
int ports0_cond_broadcast(ports0_cond_t *cv)
{
    int rc; 
    PORTS0_INTERROGATE( cv, _P0_COND_T, "broadcast" ); 
    rc = ports0_macro_cond_broadcast(cv); 
    _p0_test_rc( rc, "PORTS0: pthread_cond_broadcast() failed\n" );
    return(rc);
} /* ports0_cond_broadcast() */

void ports0_thread_prefork(void)
{
#ifdef HAVE_PTHREAD_DRAFT_6
pthread_attr_t new;
    sigset_t set;

/*
 * Currently, all threads run at a default priority, so it is not
 * necessary to save the priority of the thread before temporarily
 * changing it.  However, if the user is given control over thread
 * priorities, this should be saved in a thread specific variable
 */
pthread_attr_setprio(&new, sched_get_priority_max(SCHED_RR));
pthread_setschedattr(pthread_self(), new);

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
#endif
}

void ports0_thread_postfork(void)
{
#ifdef JGG
#ifdef HAVE_PTHREAD_DRAFT_6
pthread_attr_t old;
    sigset_t set;

pthread_attr_setprio(&old, ports0_all_global_vars.threadattr);
pthread_setschedattr(pthread_self(), old);
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
#endif
#endif
}
