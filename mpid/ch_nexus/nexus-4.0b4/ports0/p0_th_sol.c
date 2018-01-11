/*
 * p0_th_sol.h
 *
 * Solaris threads module
 */


static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_sol.c,v 1.15 1996/01/29 22:47:32 patton Exp $";

#include "p0_internal.h"

#define MAX_ERR_SIZE	80
#define _P0_THREAD_GRAN 256
#define _P0_USER_THREAD 0

#ifndef PORTS0_DEFAULT_CONCURRENCY_LEVEL
#define PORTS0_DEFAULT_CONCURRENCY_LEVEL 5
#endif

#ifndef PORTS0_DEFAULT_STACK_SIZE
#define PORTS0_DEFAULT_STACK_SIZE 0
#endif

static ports0_bool_t	preemptive_threads = PORTS0_TRUE;
static ports0_bool_t	arg_got_sched;
static int	scheduler;
static int	priority_min;
static int	priority_max;
static int	priority_mid;

static ports0_bool_t	arg_got_concurrency_level;
static int	arg_concurrency_level;
static ports0_bool_t	arg_got_stack_size;
static long	arg_stack_size;
static long     stack_size;
static int      concurrency_level;

static ports0_bool_t		using_idle_thread = PORTS0_FALSE;
static unsigned int	idle_thread;
static ports0_bool_t		idle_thread_done;
static void *		idle_thread_func(void *arg);

/*
 * Free list of _p0_thread_t structures
 */
static _p0_thread_t *	Thread_Freelist;
static ports0_mutex_t	thread_mem_mutex;
static int              next_thread_id;
static int		number_of_threads;

static void *           thread_starter(void *temparg);
static _p0_thread_t *   new_thread(void);
static void             set_tsd(_p0_thread_t *);

static void (*idle_func_save)(void);

/* For ports0_thread_once() */
static ports0_mutex_t	thread_once_mutex;


/*
 * _p0_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _p0_thread_usage_message(void)
{
    printf("	-concurrency_level <integer> : Solaris concurrency level,\n");
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
    char tmp_buf1[1024];
    char tmp_buf2[1024];
    int n_added;

    tmp_buf1[0] = '\0';

    if (arg_got_concurrency_level)
    {
	sprintf(tmp_buf2, "-concurrency_level %d ", concurrency_level);
	strcat(tmp_buf1, tmp_buf2);
    }
    
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
 */
void _p0_thread_preinit( void )
{
} /* _p0_thread_preinit() */


/*
 * _p0_thread_init()
 */
void _p0_thread_init(int *argc, char **argv[])
{
    int i;
    int rc=0;
    int arg_num;
    _p0_thread_t *thread;

    /* Get the -concurrency_level command line argument */
    if ((arg_num
	 = ports0_find_argument(argc, argv, "concurrency_level", 2)) >= 0)
    {
	concurrency_level = atol((*argv)[arg_num + 1]);
	arg_got_concurrency_level = PORTS0_TRUE;
	ports0_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	concurrency_level = PORTS0_DEFAULT_CONCURRENCY_LEVEL;
	arg_got_concurrency_level = PORTS0_FALSE;
    }

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

    preemptive_threads = PORTS0_TRUE;
    if (arg_got_concurrency_level)
    {
	while((rc = thr_setconcurrency( concurrency_level ))==4) ;
	_p0_test_rc( rc, "PORTS0: thr_setconcurrency failed\n");
    }

    while((rc = thr_keycreate( &(ports0_all_global_vars.ports0_thread_t_pointer), NULL))==4) ;
    _p0_test_rc( rc, "PORTS0: thr_keycreate failed\n");
    
    (ports0_all_global_vars.general_attribute) = USYNC_THREAD;
    (ports0_all_global_vars.thread_flags) = THR_DETACHED;

    ports0_threadattr_setstacksize(&ports0_all_global_vars.thread_attr,
				   (size_t) stack_size);
    
    ports0_mutex_init( &(thread_mem_mutex), (ports0_mutexattr_t *) NULL );
    
    /* Initialize the thread_once_mutex */
    ports0_mutex_init(&thread_once_mutex, (ports0_mutexattr_t *) NULL);
    next_thread_id = 0;
    
    /* Initialize the _p0_thread_t structure for this initial thread */
    thread = new_thread();
    thread->names = (ports0_thread_name_list_t *) NULL;
    set_tsd( thread );
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
void _p0_report_bad_rc( int rc, char *message )
{
    char achMessHead[] = "[Thread System]";
    char achDesc[MAX_ERR_SIZE];
    
    if( rc!=_P0_THR_SUCCESS )
    {
	switch( rc )
	{
	case EAGAIN:
	    strcpy(achDesc, "system out of resources (EAGAIN)");
	    break;
	case ENOMEM:
	    strcpy(achDesc, "insufficient memory (ENOMEM)");
	    break;
	case EINVAL:
	    strcpy(achDesc, "invalid value passed to thread interface (EINVAL)");
	    break;
	case EPERM:
	    strcpy(achDesc, "user does not have adequate permission (EPERM)");
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
			 achMessHead, message, achMessHead, rc );
	    break;
	}
	ports0_fatal("%s %s\n%s %s",
		     achMessHead, message, achMessHead, achDesc);
	_p0_imprison_thread( "Error code detected\n" );
	/*    _p0_exit_process( 1 ); */
    }
} /* _p0_report_bad_rc() */


/*
 * new_thread()
 */
static _p0_thread_t *new_thread( void )
{
    int i;
    _p0_thread_t *new_thread;
    int mem_req_size;
    
    ports0_mutex_lock(&thread_mem_mutex);
    
    if( Thread_Freelist==NULL )
    {
	mem_req_size = sizeof(_p0_thread_t) * _P0_THREAD_GRAN;
	Ports0Malloc(_p0_more_thread_mem(), Thread_Freelist, _p0_thread_t *, 
		     mem_req_size);
	
	for( i=0; i<_P0_THREAD_GRAN-1; i++ )
	{
	    Thread_Freelist[i].next_free=&Thread_Freelist[i+1];
	}
	Thread_Freelist[_P0_THREAD_GRAN-1].next_free=NULL;
    }
    new_thread = Thread_Freelist;
    if( Thread_Freelist!=NULL ) 
    {
	Thread_Freelist=Thread_Freelist->next_free;
    }
    
    new_thread->id = next_thread_id++;
    
    ports0_mutex_unlock(&thread_mem_mutex);
    
    return new_thread;
    
} /* new_thread() */


/*
 * terminate_thread()
 */
static void terminate_thread(_p0_thread_t *thread, void *status)
{
    int i;

#ifdef BUILD_PROFILE
    /*
    log_thread_destruction(thread->id);
    */
#endif /* BUILD_PROFILE */    

    /* Free up the thread storage */
    ports0_mutex_lock(&thread_mem_mutex);
    thread->next_free = Thread_Freelist;
    Thread_Freelist = thread;
    ports0_mutex_unlock(&thread_mem_mutex);

    /* Exit the thread */
    thr_exit(NULL);

} /* terminate_thread() */


/*
 * ports0_thread_exit()
 */
void ports0_thread_exit( void *status )
{
    _p0_thread_t *victim;
    _p0_thread_self(&victim);
    terminate_thread(victim, status);
} /* ports0_thread_exit() */


/*
 * set_tsd()
 */
static void set_tsd( _p0_thread_t * thread )
{
    int rc=0;
    
    while((rc=thr_setspecific( ports0_all_global_vars.ports0_thread_t_pointer, (void *) thread ))==4) ;
    _p0_test_rc( rc, "PORTS0: set thread-local data failed\n" );
} /* set_tsd() */


/*
 * thread_starter()
 */
static void *thread_starter( void *temparg )
{
    _p0_thread_t *thread;
    int i;
    void *status;

    thread = (_p0_thread_t *)temparg;
    
    set_tsd( thread );
    
#ifdef BUILD_PROFILE
    /*
    log_thread_creation(thread->id);
    */
#endif /* BUILD_PROFILE */    
    
    /* Call the user function */
    status = (*thread->user_func)(thread->user_arg);
    
    /* Terminate the thread */
    terminate_thread(thread, status);
    
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
    _p0_thread_t *thread;
    thread_t thread_id;
    int rc=0;
    size_t stacksize = 0;
    
    thread = new_thread();
    thread->names = (ports0_thread_name_list_t *) NULL;
    
    /* Initialize the thread data that needs to be passed to the new thread */
    thread->user_func = func;
    thread->user_arg = user_arg;
    
    if (attr)
    {
	ports0_threadattr_getstacksize(attr, &stacksize);
    }
    else
    {
	ports0_threadattr_getstacksize(&ports0_all_global_vars.thread_attr,
				       &stacksize);
    }
    
    while((rc = thr_create( NULL, stacksize, thread_starter,
			   thread, (ports0_all_global_vars.thread_flags),
			   &thread_id ))==4);
    _p0_test_rc( rc, "PORTS0: create thread failed\n" );
    
    if(user_thread)
    {
	*user_thread = thread->id;
    }

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
    _p0_thread_t *thread;
    int thread_id;
    size_t stacksize;
    void *status;

    
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
	    ports0_threadattr_getstacksize(&ports0_all_global_vars.thread_attr,
					   &stacksize);
    
	    thread = new_thread();
	    thread->names = (ports0_thread_name_list_t *) NULL;
	    thread->user_func = idle_thread_func;
	    thread->user_arg = NULL;
    
	    while((rc = thr_create( NULL, stacksize, thread_starter, thread, 0, &idle_thread))==4);
	    _p0_test_rc( rc, "PORTS0: create thread failed\n" );
    
	    rc = thr_setprio(idle_thread, /* min? */ 0);
	    _p0_test_rc(rc, "PORTS0: thr_setprio() failed\n");
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
	    if ( thr_self() != idle_thread )
	    {
		rc = thr_join( idle_thread, NULL, &status );
		_p0_test_rc( rc, "PORTS0: thread join failed\n" );
	    }

	    using_idle_thread = PORTS0_FALSE;
	}
    }

} /* ports0_idle_callback() */


/*
 * idle_thread_func()
 *
 * TODO: Should count 'unblocked_threads' as they call and return
 * from cond_wait.  Thus, when all threads except this one are blocked,
 * we know we can call _p0_blocking_poll() instead of ports0_poll when
 * using preemptive threads.
 */
static void *idle_thread_func(void *arg)
{
    ports0_debug_printf(2, ("idle_thread_func(): Entering\n"));

    while (!idle_thread_done)
    {
	(*idle_func_save)();
	ports0_debug_printf(6, ("idle_thread_func(): calling yield\n"));
	ports0_thread_yield();
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
    int rc;
    rc = ports0_macro_threadattr_init(attr);
    _p0_test_rc(rc, "PORTS0: threadattr_init() failed\n");
    return (rc);
}

/*
 * ports0_threadattr_destroy()
 */
#undef ports0_threadattr_destroy
int ports0_thread_destroy(ports0_threadattr_t *attr)
{
    int rc;
    rc = ports0_macro_threadattr_destroy(attr);
    _p0_test_rc(rc, "PORTS0: threadattr_destroy() failed\n");
    return (rc);
}

/*
 * ports0_threadattr_setstacksize()
 */
#undef ports0_threadattr_setstacksize
int ports0_threadattr_setstacksize(ports0_threadattr_t *attr, 
		size_t stacksize)
{
    int rc;
    rc = ports0_macro_threadattr_setstacksize(attr, stacksize);
    _p0_test_rc(rc, "PORTS0: threadattr_setstacksize failed\n");
    return (rc);
}

/*
 * ports0_threadattr_getstacksize()
 */
#undef ports0_threadattr_getstacksize
int ports0_threadattr_getstacksize(ports0_threadattr_t *attr,
				   size_t *stacksize)
{
    int rc;
    rc = ports0_macro_threadattr_getstacksize(attr, stacksize);
    _p0_test_rc(rc, "PORTS0: threadattr_getstacksize failed\n");
    return (rc);
}

/*
 * ports0_key_create()
 */
#undef ports0_thread_key_create
int ports0_thread_key_create(ports0_thread_key_t *key,
			    ports0_thread_key_destructor_func_t func)
{
   int rc=0;
   while((rc = ports0_macro_thread_key_create( key, func )) == 4);
   _p0_test_rc( rc, "PORTS0: keycreate failed\n" );
    return(rc);
} /* ports0_key_create() */


/*
 * ports0_thread_setspecific()
 */
#undef ports0_thread_setspecific
int ports0_thread_setspecific(ports0_thread_key_t key,
			     void *value)
{
    int rc=0;
    while((rc = ports0_macro_thread_setspecific(key, value))==4) ;
    _p0_test_rc(rc, "PORTS0: set specific failed\n");
    return rc;
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


void *_p0_thread_getspecific(ports0_thread_key_t key)
{
	int rc;
	void *value;

    while((rc = thr_getspecific(key, &value))==4) ;
    _p0_test_rc(rc, "PORTS0: get specific failed\n");
    return (value);
} /* ports0_thread_getspecific() */


/*
 * ports0_thread_self
 */
#undef ports0_thread_self
ports0_thread_t ports0_thread_self( void )
{
    return (ports0_macro_thread_self());
}

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
 * ports0_thread_yield
 */
#undef ports0_thread_yield
void ports0_thread_yield( void )
{
    ports0_macro_thread_yield();
}

/*
 * ports0_i_am_only_thread()
 */
#undef ports0_i_am_only_thread
ports0_bool_t ports0_i_am_only_thread(void)
{
    return (ports0_macro_i_am_only_thread());
}

#undef ports0_mutex_init
int ports0_mutex_init( ports0_mutex_t *mut, ports0_mutexattr_t *attr )
{
    int rc=0;
    _P0_INIT_START_MAGIC_COOKIE(mut);
    while((rc =ports0_macro_mutex_init(mut, attr))==4) ;
    _p0_test_rc(rc, "PORTS0: allocate lock failed\n");
    _P0_INIT_END_MAGIC_COOKIE(mut);
    return rc;
}

/*
 *  ports0_mutex_destroy()
 */
#undef ports0_mutex_destroy
int ports0_mutex_destroy( ports0_mutex_t *mut )
{
    int rc=0;
    PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "destroy");
    while((rc = ports0_macro_mutex_destroy(mut))==4) ;
    _p0_test_rc(rc, "PORTS0: free lock failed\n");
    return rc;
}

/*
 * ports0_cond_init()
 */
#undef ports0_cond_init
int ports0_cond_init( ports0_cond_t *cv, ports0_condattr_t *attr )
{
    int rc=0;
    _P0_INIT_START_MAGIC_COOKIE(cv);
    while((rc = ports0_macro_cond_init(cv, attr))==4) ;
    _p0_test_rc(rc, "PORTS0: allocate condition variable failed\n");
    return rc;
}

/*
 *  ports0_cond_destroy()
 */
#undef ports0_cond_destroy
int ports0_cond_destroy( ports0_cond_t *cv )
{
    int rc=0;
    PORTS0_INTERROGATE(cv, _P0_COND_T, "destroy");
    while ((rc = ports0_macro_cond_destroy(cv))==4) ;
    _p0_test_rc(rc, "PORTS0: free condition variable failed\n");
    return rc;
}

/* 
 *  ports0_mutex_lock()
 */
#undef ports0_mutex_lock
int ports0_mutex_lock( ports0_mutex_t *mut )
{
    int rc=0;
    PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "lock1");
    while((rc = ports0_macro_mutex_lock(mut))==4) ;
    PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "lock2");
    _p0_test_rc(rc, "PORTS0: mutex lock failed\n");
    return rc;
}


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
        _p0_test_rc( rc, "PORTS0: port0_mutex_trylock() failed\n" );
    }
    PORTS0_INTERROGATE( mut, _P0_MUTEX_T, "trylock after" );
    return(rc);
} /* ports0_mutex_trylock() */


/*
 *  ports0_mutex_unlock()
 */
#undef ports0_mutex_unlock
int ports0_mutex_unlock( ports0_mutex_t *mut )
{
    int rc=0;
    PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "unlock1");
    while((rc = ports0_macro_mutex_unlock(mut))==4) ;
    PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "unlock2");
    _p0_test_rc(rc, "PORTS0: mutex unlock failed\n");
    return rc;
}

/*
 *  ports0_cond_wait()
 */
#undef ports0_cond_wait
int ports0_cond_wait( ports0_cond_t *cv, ports0_mutex_t *mut )
{
    int rc=0;
    PORTS0_INTERROGATE(cv, _P0_COND_T, "wait1");
    rc=ports0_macro_cond_wait(cv, mut);
    PORTS0_INTERROGATE(cv, _P0_COND_T, "wait2");
    _p0_test_rc(rc, "PORTS0: condition variable wait failed\n");
    return rc;
}

/*
 *  ports0_cond_signal()
 */
#undef ports0_cond_signal
int ports0_cond_signal( ports0_cond_t *cv )
{
    int rc=0;
    PORTS0_INTERROGATE(cv, _P0_COND_T, "signal1");
    while((rc = ports0_macro_cond_signal(cv))==4) ;
    _p0_test_rc(rc, "PORTS0: condition variable signal failed\n");
    return rc;
}

/*
 *  ports0_cond_broadcast()
 */
#undef ports0_cond_broadcast
int ports0_cond_broadcast( ports0_cond_t *cv )
{
    int rc=0;
    PORTS0_INTERROGATE(cv, _P0_COND_T, "broadcast1");
    while((rc = ports0_macro_cond_broadcast(cv))==4) ;
    _p0_test_rc(rc, "PORTS0: condition variable broadcast failed\n");
    return rc;
}


/*
 * _p0_actual_thread_once()
 */
int _p0_actual_thread_once(ports0_thread_once_t *once_control,
			   void (*init_routine)(void))
{
    int rc;
    ports0_mutex_lock(&thread_once_mutex);
    if (*once_control)
    {
	/* Someone beat us to it.  */
	rc = 0;
    }
    else
    {
	/* We're the first one here */
	(*init_routine)();
	*once_control = 1;
	rc = 0;
    }
    ports0_mutex_unlock(&thread_once_mutex);
    return (rc);
} /* _p0_actual_thread_once() */

#undef ports0_thread_once
int ports0_thread_once(ports0_thread_once_t *once_control,
		       void (*init_routine)(void))
{
    return (_p0_actual_thread_once(once_control, init_routine));
}

void ports0_thread_prefork(void)
{
/* Do nothing */
}

void ports0_thread_postfork(void)
{
/* Do nothing */
}
