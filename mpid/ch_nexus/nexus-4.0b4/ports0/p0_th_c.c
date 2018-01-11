/*
 * p0_th_c.c
 *
 * Routines for C-threads based Ports0 threads library.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_c.c,v 1.17 1996/02/28 20:43:13 patton Exp $";

#include "p0_internal.h"

#ifndef PORTS0_KEY_TABLE_SIZE
#define PORTS0_KEY_TABLE_SIZE 16
#endif

static ports0_thread_key_destructor_func_t *key_table_destructor_funcs;

/*
 * Thread specific data key maintainnance stuff
 */
static int		key_table_size;
static int		next_key;
static ports0_mutex_t	key_mutex;

/*
 * Free list of _p0_thread_t structures
 */
static _p0_thread_t *	free_list;
static ports0_mutex_t	free_list_mutex;
static int		next_thread_id;


/*
 * For ports0_thread_once()
 */
static ports0_mutex_t	thread_once_mutex;


static ports0_bool_t		using_idle_thread = PORTS0_FALSE;
static cthread_t	idle_thread;
static ports0_bool_t		idle_thread_done;
static void *		idle_thread_func(void *arg);
static ports0_bool_t		preemptive_threads = PORTS0_TRUE;

static any_t		thread_startup(any_t arg);
static void (*idle_func_save)(void);


/*
 * _p0_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _p0_thread_usage_message(void)
{
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
    return (0);
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
    _p0_thread_t *thread;
    int i;
    
#ifndef HAVE_THERAD_SAFE_STDIO
    ports0_mutex_init(&ports0_reentrant_mutex, (ports0_mutexattr_t *) NULL);
#endif    

    /* Initialize thread specific keys */
    key_table_size = PORTS0_KEY_TABLE_SIZE;
    next_key = 0;
    ports0_mutex_init(&key_mutex, (ports0_mutexattr_t *) NULL);
    
    /* Initialize the free list */
    free_list = (_p0_thread_t *) NULL;
    ports0_mutex_init(&free_list_mutex, (ports0_mutexattr_t *) NULL);
    next_thread_id = 1;

    /* Initialize the thread_once_mutex */
    ports0_mutex_init(&thread_once_mutex, (ports0_mutexattr_t *) NULL);

    /* Initialize the key_table_destructor_funcs */
    Ports0Malloc(_p0_thread_init(),
		 key_table_destructor_funcs,
		 ports0_thread_key_destructor_func_t *,
		 (key_table_size*sizeof(ports0_thread_key_destructor_func_t)));
    for (i = 0; i < key_table_size; i++)
    {
	key_table_destructor_funcs[i] = NULL;
    }
    
    /* Initialize the _p0_thread_t structure for this initial thread */
    Ports0Malloc(_p0_thread_init(),
		thread,
		_p0_thread_t *,
		_P0_THREAD_SIZE() );
    thread->id = 0;
    thread->names = (ports0_thread_name_list_t *) NULL;
    thread->key_table_values
	= (void **) (((char *) thread) + sizeof(_p0_thread_t));
    for (i = 0; i < key_table_size; i++)
    {
		thread->key_table_values[i] = (void *) NULL;
    }
    
    cthread_set_data(cthread_self(), thread);
    
} /* _p0_thread_init() */


/*
 * _p0_thread_shutdown()
 */
int _p0_thread_shutdown(void)
{
    return 0;
} /* _p0_thread_shutdown() */


/*
 * _p0_thread_abort()
 */
void _p0_thread_abort(int return_code)
{
} /* _p0_thread_abort() */


/*
 * terminate_thread()
 *
 * Put the thread's data structure onto the free_list, and exit
 * from the thread.
 */
static void terminate_thread(_p0_thread_t *thread, void *status)
{
    int i;
    ports0_thread_key_destructor_func_t func;
    void *value;
    
    /* Call the thread specific data destructors */
    for (i = 0; i < key_table_size; i++)
    {
		func = key_table_destructor_funcs[i];
		value = thread->key_table_values[i];
		if (func && value)
		{
	    	(*func)(value);
		}
    }

#ifdef BUILD_PROFILE
    /*
    log_thread_destruction(thread->id);
    */
#endif /* BUILD_PROFILE */    

    /* Free up the thread storage */
    ports0_mutex_lock(&free_list_mutex);
    thread->next_free = free_list;
    free_list = thread;
    ports0_mutex_unlock(&free_list_mutex);

    /* Exit the thread */
    cthread_exit((any_t) status);
} /* terminate_thread() */


/*
 * thread_startup()
 *
 * This is the entry point for a new cthread that is created
 * by ports0_thread_create().  It does some bookkeeping and then
 * calls the user function.  If the user function returns, then
 * it terminates the thread.
 */
static any_t thread_startup(any_t arg)
{
    _p0_thread_t *thread = (_p0_thread_t *) arg;
    int i;
    void *status;

    /* Initialize the thread specific key table */
    for (i = 0; i < key_table_size; i++)
    {
	thread->key_table_values[i] = (void *) NULL;
    }

    /* Setup pointers so we can find the thread data again */
    cthread_set_data(cthread_self(), thread);

#ifdef BUILD_PROFILE
    /*
    log_thread_creation(thread->id);
    */
#endif /* BUILD_PROFILE */    
    
    /* Call the user function */
    status = (*thread->user_func)(thread->user_arg);

    /* Terminate the thread */
    terminate_thread(thread, status);

    return (0);
    
} /* thread_startup() */


/*
 * get_new_thread()
 *
 * Return a _p0_thread_t for a new thread.
 */
static _p0_thread_t *get_new_thread(void)
{
    _p0_thread_t *thread;
    
    ports0_mutex_lock(&free_list_mutex);
    if (free_list != (_p0_thread_t *) NULL)
    {
	thread = free_list;
	free_list = free_list->next_free;
    }
    else
    {
	Ports0Malloc(ports0_thread_create(),
		     thread,
		     _p0_thread_t *,
		     _P0_THREAD_SIZE() );
    }
    thread->id = next_thread_id++;
    ports0_mutex_unlock(&free_list_mutex);

    return (thread);
} /* get_new_thread() */


/*
 * ports0_thread_create()
 */
int ports0_thread_create(ports0_thread_t *user_thread,
			ports0_threadattr_t *attr,
			ports0_thread_func_t func,
			void *user_arg)
{
    _p0_thread_t *thread;
    cthread_t cthread_id;

    thread = get_new_thread();

    /* Initialize the thread data */
    thread->names = (ports0_thread_name_list_t *) NULL;
    thread->key_table_values
	= (void **) (((char *) thread) + sizeof(_p0_thread_t));
    
    /* Initialize the thread data that needs to be passed to the new thread */
    thread->user_func = func;
    thread->user_arg = user_arg;
    
    cthread_id = cthread_fork(thread_startup, thread);
    cthread_detach(cthread_id);

    if (user_thread)
    {
	*user_thread = *cthread_id;
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
    _p0_thread_t *thread;

    if (idle_func)
    {
	if (using_idle_thread)
	{
	    ports0_debug_printf(2,("ports0_idle_callback(): Changing idle callback for existing idle thread\n"));
	    idle_func_save = idle_func;
	}
	else
	{
	    ports0_debug_printf(2,("ports0_idle_callback(): Creating idle thread\n"));
	    
	    using_idle_thread = PORTS0_TRUE;
	    idle_thread_done = PORTS0_FALSE;
	    idle_func_save = idle_func;
	    thread = get_new_thread();
	    
	    /* Initialize the thread data */
	    thread->names = (ports0_thread_name_list_t *) NULL;
	    thread->key_table_values
		= (void **) (((char *) thread) + sizeof(_p0_thread_t));
	    thread->user_func = idle_thread_func;
	    thread->user_arg = NULL;
	    
	    idle_thread = cthread_fork(thread_startup, thread);
	    cthread_priority(idle_thread, /* min? */ 0, FALSE);
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
	    if (cthread_self() != idle_thread)
	    {
		cthread_join(idle_thread);
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
    /* ports0_debug_printf(2, ("idle_thread_func(): Entering\n")); */

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
 * ports0_thread_exit()
 */
void ports0_thread_exit(void *status)
{
    _p0_thread_t *thread;
    _p0_thread_self(&thread);
    terminate_thread(thread, status);
} /* ports0_thread_exit() */


/*
 * ports0_thread_key_create()
 */
int ports0_thread_key_create(ports0_thread_key_t *key,
			    ports0_thread_key_destructor_func_t func)
{
    int rc;
    
    ports0_mutex_lock(&key_mutex);
    if (next_key >= key_table_size)
    {
	/* Attempt to allocate a key when the key name space is exhausted */
	rc = EAGAIN;
    }
    else
    {
	*key = next_key++;
	key_table_destructor_funcs[*key] = func;
	rc = 0;
    }
    ports0_mutex_unlock(&key_mutex);
    return(rc);
} /* ports0_thread_key_create() */


/*
 * ports0_thread_setspecific()
 */
int ports0_thread_setspecific(ports0_thread_key_t key,
			      void *value)
{
    _p0_thread_t *thread;

    Ports0Assert2(((key >= 0) && (key < key_table_size)),
		  ("ports0_thread_setspecific(): Invalid key: %d\n", key) );

    _p0_thread_self(&thread);
    thread->key_table_values[key] = value;
    return 0;
} /* ports0_thread_setspecific() */


/*
 * ports0_thread_getspecific()
 */
void *ports0_thread_getspecific(ports0_thread_key_t key)
{
    void *value;
    _p0_thread_t *thread;
    
    Ports0Assert2(((key >= 0) && (key < key_table_size)),
		  ("ports0_thread_getspecific(): Invalid key: %d\n", key) );

    _p0_thread_self(&thread);
    value = thread->key_table_values[key];
    return (value);
} /* ports0_thread_getspecific() */


/*
 * _p0_actual_thread_once()
 */
int _p0_actual_thread_once(ports0_thread_once_t *once_control,
			   void (*init_routine)(void))
{
    int rc=-1;
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



/*
 * Now provide the function versions of each of the macros
 * for when USE_MACROS is not defined.
 */
#undef ports0_thread_yield
void ports0_thread_yield(void)
{
    ports0_macro_thread_yield();
} /* ports0_thread_yield() */

#undef ports0_thread_self
ports0_thread_t ports0_thread_self(void)
{
    return (ports0_macro_thread_self());
}

#undef ports0_thread_equal
int ports0_thread_equal(ports0_thread_t t1,
			ports0_thread_t t2)
{
    return (ports0_macro_thread_equal(t1,t2));
}
		       
#undef ports0_thread_once
int ports0_thread_once(ports0_thread_once_t *once_control,
		       void (*init_routine)(void))
{
    return (ports0_macro_thread_once(once_control, init_routine));
}

#undef ports0_i_am_only_thread
ports0_bool_t ports0_i_am_only_thread(void)
{
	/*
	 * return FALSE always because we never know what the number of
	 * running threads is, so we can never assume this is the only
	 * thread.
	 */
	return PORTS0_FALSE;
}


#undef ports0_mutex_init
int ports0_mutex_init(ports0_mutex_t *mutex, ports0_mutexattr_t *attr)
{
    int rc;
    
    _P0_INIT_START_MAGIC_COOKIE(mutex);
    rc = ports0_macro_mutex_init(mutex, attr);
    _P0_INIT_END_MAGIC_COOKIE(mutex);
    return rc;
}
#undef ports0_mutex_destroy
int ports0_mutex_destroy(ports0_mutex_t *mutex)
{
    int rc;
    
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "destroy");
    rc = ports0_macro_mutex_destroy(mutex);
    return rc;
}

#undef ports0_mutex_lock
int ports0_mutex_lock(ports0_mutex_t *mutex)
{
    int rc;
    
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "lock before");
    rc = ports0_macro_mutex_lock(mutex);
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "lock after");
    return rc;
}

#undef ports0_mutex_trylock
int ports0_mutex_trylock(ports0_mutex_t *mutex)
{
    int rc;
    
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "trylock before");
    rc = ports0_macro_mutex_trylock(mutex);
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "trylock after");
    return rc;
}

#undef ports0_mutex_unlock
int ports0_mutex_unlock(ports0_mutex_t *mutex)
{
    int rc;
    
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "unlock before");
    rc = ports0_macro_mutex_unlock(mutex);
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "unlock after");
    return rc;
}

#undef ports0_cond_init
int ports0_cond_init(ports0_cond_t *cond, ports0_condattr_t *attr)
{
    int rc;
    
    _P0_INIT_START_MAGIC_COOKIE(cond);
#ifdef TARGET_ARCH_NEXTSTEP
    ports0_macro_cond_init(cond, attr);
    rc = 0;
#else    
    rc = ports0_macro_cond_init(cond, attr);
#endif
    _P0_INIT_END_MAGIC_COOKIE(cond);
    return (rc);
}

#undef ports0_cond_destroy
int ports0_cond_destroy(ports0_cond_t *cond)
{
    int rc;
    
    PORTS0_INTERROGATE(cond, _P0_COND_T, "destroy");
#ifdef TARGET_ARCH_NEXTSTEP
    ports0_macro_cond_destroy(cond);
    rc = 0;
#else
    rc = ports0_macro_cond_destroy(cond);
#endif
    return (rc);
}

#undef ports0_cond_signal
int ports0_cond_signal(ports0_cond_t *cond)
{
    int rc;
    
    PORTS0_INTERROGATE(cond, _P0_COND_T, "signal");
#ifdef TARGET_ARCH_NEXTSTEP
    ports0_macro_cond_signal(cond);
    rc = 0;
#else
    rc = ports0_macro_cond_signal(cond);
#endif
    return (rc);
}

#undef ports0_cond_broadcast
int ports0_cond_broadcast(ports0_cond_t *cond)
{
    int rc;
    
    PORTS0_INTERROGATE(cond, _P0_COND_T, "broadcast");
#ifdef TARGET_ARCH_NEXTSTEP
    ports0_macro_cond_broadcast(cond);
    rc = 0;
#else
    rc = ports0_macro_cond_broadcast(cond);
#endif
    return (rc);
}

#undef ports0_cond_wait
int ports0_cond_wait(ports0_cond_t *cond,
		     ports0_mutex_t *mutex)
{
    int rc;

    PORTS0_INTERROGATE(cond, _P0_COND_T, "wait before");
    rc = ports0_macro_cond_wait(cond, mutex);
    PORTS0_INTERROGATE(cond, _P0_COND_T, "wait after");
    return (rc);
}

void ports0_thread_prefork(void)
{
/* Do Nothing */
}

void ports0_thread_postfork(void)
{
/* Do Nothing */
}
