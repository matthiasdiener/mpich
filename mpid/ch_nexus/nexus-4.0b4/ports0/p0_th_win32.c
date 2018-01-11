/*
 * p0_th_win32.c
 *
 * Win32 Threads Implementation for Nexus
 * 
 * Ported by:
 *   George K. Thiruvathukal
 *   Argonne National Laboratory, MCS Division
 *
 * Version: $Id: p0_th_win32.c,v 1.1 1996/11/15 17:33:02 tuecke Exp $
 */

static char *rcsid = "$Id: p0_th_win32.c,v 1.1 1996/11/15 17:33:02 tuecke Exp $";

#include "p0_internal.h"

#define MAX_ERR_SIZE    80
#define _P0_THREAD_GRAN 256
#define _P0_USER_THREAD 0

#ifndef PORTS0_DEFAULT_STACK_SIZE
#define PORTS0_DEFAULT_STACK_SIZE 0
#endif

static ports0_bool_t preemptive_threads = PORTS0_TRUE;

static ports0_bool_t arg_got_stack_size;

static long stack_size;

static ports0_bool_t using_idle_thread = PORTS0_FALSE;

static DWORD idle_thread;

static HANDLE idle_thread_handle;

static ports0_bool_t idle_thread_done;

static void *idle_thread_func(void *arg);

/*
 * Free list of _p0_thread_t structures
 */

static _p0_thread_t *Thread_Freelist;

static ports0_mutex_t thread_mem_mutex;

static int next_thread_id;

static int number_of_threads;

static void *thread_starter(void *temparg);

static _p0_thread_t *new_thread(void);

static void set_tsd(_p0_thread_t *);

static void (*idle_func_save)(void);

/* For ports0_thread_once() */

static ports0_mutex_t thread_once_mutex;

static DWORD tlsIndex;


/* Thread Local Storage Emulation Structures */

typedef struct __p0_keyInfo {
  int keyTableSize;
  int nextKey;
  ports0_mutex_t keyMutex;
  destructor_t *destructorFunction;
} _p0_keyInfo;

static _p0_keyInfo KeyInfo;


/* Ports0 - Initialization
 *
 *   _p0_thread_usage_function - show arguments which pertain to threads only.
 *   _p0_thread_new_process params - handle arguments pertaining to threads
 *   _p0_thread_preinit - does nothing here
 *   _p0_thread_init - major entry point
 */

/*
 * Initialization - _p0_thread_usage_message()
 *
 * Allow the stack size to be specified. Many of the capabilities of Solaris,
 * such as LWP and concurrency control/granularity, are not supported in Win32.
 */

void _p0_thread_usage_message(void)
{
  printf("      -stack <integer>          : Set the thread stack size.\n");
}

/*
 * Initialization - _p0_thread_new_process_params()
 */

int _p0_thread_new_process_params(char *buf, int size)
{
  char tmp_buf1[1024];
  char tmp_buf2[1024];
  int n_added;

  tmp_buf1[0] = '\0';

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
}

/*
 * Initialization - _p0_thread_preinit()
 *
 * If you need to call a thread package initialization routine, then
 * do it here.  This is called as the very first thing in ports0_init().
 */

void _p0_thread_preinit( void )
{

}

/*
 * Initialization - _p0_thread_init()
 *
 * This is where a number of activities are performed to get the ball rolling:
 *   1. Argument processing
 *   2. Key table initialization for Thread-specific data emulation.
 *   3. Initializing thread data structures, etc.
 */

void _p0_thread_init(int *argc, char **argv[])
{
  int arg_num;
  int i;
  int rc = 0;
  _p0_thread_t *thread;

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

  tlsIndex = TlsAlloc();

  KeyInfo.keyTableSize = PORTS0_KEY_TABLE_SIZE;
  KeyInfo.nextKey = 0;
  ports0_mutex_init(&KeyInfo.keyMutex, (ports0_mutexattr_t *) NULL);
  Ports0Malloc(_p0_thread_init(),
               KeyInfo.destructorFunction,
               destructor_t *,
               (KeyInfo.keyTableSize * sizeof(destructor_t))
               );
  for (i=0; i < KeyInfo.keyTableSize; i++)
    KeyInfo.destructorFunction[i] = NULL;


  preemptive_threads = PORTS0_TRUE;

  /* Initialize the Ports0 Globals */
  rc = ports0_thread_key_create( &ports0_all_global_vars.ports0_thread_t_pointer, NULL);
  if (rc != 0) {
    fprintf(stderr,"key create failed\n"); exit(0);
  }
  ports0_all_global_vars.general_attribute = 0;
  ports0_all_global_vars.thread_flags = 0;

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
}


/*
 * Termination - _p0_thread_shutdown()
 */

int _p0_thread_shutdown(void)
{
  return (0);
}


/*
 * Termination - _p0_thread_abort()
 */

void _p0_thread_abort(int return_code)
{

}

/*
 * Error Handling - _p0_report_bad_rc()
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
#ifdef NOT_SUPPORTED
        case EDEADLK:
          strcpy(achDesc, "deadlock detected (EDEADLK)");
          break;
#endif
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
}

/*
 * Support - new_thread()
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
      Ports0Malloc(new_thread(), Thread_Freelist, _p0_thread_t *,
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

  Ports0Malloc(new_thread(), new_thread->keyTable, void*, (KeyInfo.keyTableSize * sizeof(void*)));

  for (i=0; i < KeyInfo.keyTableSize; i++)
    new_thread->keyTable[i] = NULL;

  ports0_mutex_unlock(&thread_mem_mutex);

  return new_thread;

}

/*
 * Support - terminate_thread()
 */

static void terminate_thread(_p0_thread_t *thread, void *status)
{
  int i;

  destructor_t *destructor;


  /* Clear out TLS for Thread */
  for (i = 0; i < KeyInfo.keyTableSize; i++)
  {
    destructor = KeyInfo.destructorFunction[i];
    if (destructor && thread->keyTable[i]) {
      (*destructor)(thread->keyTable[i]);
      thread->keyTable[i] = NULL;
    }
  }

  /* Free up the thread storage */
  ports0_mutex_lock(&thread_mem_mutex);
  thread->next_free = Thread_Freelist;
  Thread_Freelist = thread;
  ports0_mutex_unlock(&thread_mem_mutex);

  /* Exit the thread */
  ExitThread(0);

} /* terminate_thread() */


_p0_thread_t *_p0_thread_self(void)
{
  _p0_thread_t *Thread = NULL;
  Thread = (_p0_thread_t *) TlsGetValue(tlsIndex);

  if (Thread == NULL)
    ports0_fatal("PORTS0: get thread-local data failed\n");
  return Thread;
}

void _p0_thread_id(int *threadId)
{
  _p0_thread_t *thread;
  thread = _p0_thread_self();
  *threadId = thread->id;
}


/*
 * Support - set_tsd()
 */

static void set_tsd( _p0_thread_t * thread )
{

  int rc;

  /* Right now, I'm assuming this succeeds. It probably will. */


  /* This is guaranteed to succeed in Windows NT, as long as the thread
   * creation was successful. This is because each thread gets 64 indices
   * of TLS. In this implementation, only one is used, so we can have
   * an arbitrary number of indices.
   */

  TlsSetValue(tlsIndex, (LPVOID) thread);
} /* set_tsd() */


/*
 * Support - thread_starter()
 */

static void *thread_starter( void *temparg )
{
  _p0_thread_t *thread;
  void *status;

  thread = (_p0_thread_t *)temparg;

  set_tsd( thread );

#ifdef PORTS0_PROFILE
  /*
    log_thread_creation(thread->id);
    */
#endif /* PORTS0_PROFILE */

  /* Call the user function */
  status = (*thread->user_func)(thread->user_arg);

  /* Terminate the thread */
  terminate_thread(thread, status);

  return (NULL);
} /* thread_starter() */




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
  size_t stacksize;


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


          idle_thread_handle = CreateThread(
                                            NULL,
                                            stacksize,
                                            (LPTHREAD_START_ROUTINE) thread_starter,
                                            thread,
                                            0, /* This creates a thread which is not suspended */
                                            &idle_thread
                                            );

          if (!idle_thread_handle) {
            fprintf(stderr,"CreateThread failed.\n");
            exit(0);
          }
          SetThreadPriority(idle_thread_handle, THREAD_PRIORITY_LOWEST);
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
          if ( GetCurrentThread() != idle_thread_handle )
            {
              /* In NT Thread Sync, you use the Thread as a Sync variable */
              WaitForSingleObject( idle_thread_handle, INFINITE);
              /* Test Error condition */
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
 * Ports0 - Thread Management - Thread Control
 *
 *   ports0_threadattr_init()
 *   ports0_threadattr_destroy()
 *   ports0_threadattr_setstacksize()
 *   ports0_threadattr_getstacksize()
 *   ports0_thread_create()
 *   ports0_thread_exit()
 *   ports0_thread_yield()
 *   ports0_thread_self()
 *   ports0_thread_equal()
 *   ports0_thread_once()
 */

/*
 * These functions may all be defined to operate as macros. Because of this
 * possibility, it is important to undefine these macros so the functions can
 * behave as, well, functions.
 */

#undef ports0_threadattr_init
#undef ports0_threadattr_destroy
#undef ports0_threadattr_setstacksize
#undef ports0_threadattr_getstacksize
#undef ports0_thread_create
#undef ports0_thread_exit
#undef ports0_thread_yield
#undef ports0_thread_self
#undef ports0_thread_equal
#undef ports0_thread_once

/*
 * Thread Control - ports0_threadattr_init()
 */

int ports0_threadattr_init(ports0_threadattr_t *attr)
{
  int rc = 0;

  rc = ports0_macro_threadattr_init(attr);
  _p0_test_rc(rc, "PORTS0: threadattr_init() failed\n");

  return (rc);
}

/*
 * Thread Control - ports0_threadattr_destroy()
 */

int ports0_thread_destroy(ports0_threadattr_t *attr)
{
  int rc = 0;

  rc = ports0_macro_threadattr_destroy(attr);
  _p0_test_rc(rc, "PORTS0: threadattr_destroy() failed\n");

  return (rc);
}

/*
 * Thread Control - ports0_threadattr_setstacksize()
 */

int ports0_threadattr_setstacksize(ports0_threadattr_t *attr,
                                   size_t stacksize)
{
  int rc;
  rc = ports0_macro_threadattr_setstacksize(attr, stacksize);
  _p0_test_rc(rc, "PORTS0: threadattr_setstacksize failed\n");
  return (rc);
}

/*
 * Thread Control - ports0_threadattr_getstacksize()
 */

int ports0_threadattr_getstacksize(ports0_threadattr_t *attr,
                                   size_t *stacksize)
{
  int rc;
  rc = ports0_macro_threadattr_getstacksize(attr, stacksize);
  _p0_test_rc(rc, "PORTS0: threadattr_getstacksize failed\n");
  return (rc);
}

/*
 * Thread Control - ports0_thread_create
 */

int ports0_thread_create(ports0_thread_t *user_thread,
                         ports0_threadattr_t *attr,
                         ports0_thread_func_t func,
                         void *user_arg )
{
  _p0_thread_t *thread;
  DWORD thread_id;
  HANDLE thread_handle;

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

  thread_handle = CreateThread(
                               NULL,
                               stacksize,
                               (LPTHREAD_START_ROUTINE) thread_starter,
                               thread,
                               0, /* This creates a thread which is not suspended */
                               &thread_id
                               );


  if (! thread_handle)
    ports0_fatal( "PORTS0: create thread failed\n" );

  if(user_thread)
    {
      *user_thread = thread->id;
    }

  return (0);
}

/*
 * Thread Control - ports0_thread_exit()
 */
void ports0_thread_exit( void *status )
{
  _p0_thread_t *victim = _p0_thread_self();
  terminate_thread(victim, status);
} /* ports0_thread_exit() */


/*
 * Thread Control ports0_thread_yield
 */

void ports0_thread_yield( void )
{
  ports0_macro_thread_yield();
}

/*
 * Thread Control - ports0_thread_self
 */

ports0_thread_t ports0_thread_self( void )
{

  _p0_thread_t *me;

  me = _p0_thread_self();

  return me->id;
}

/*
 * Thread Control - ports0_thread_equal()
 */

int ports0_thread_equal(ports0_thread_t t1,
                        ports0_thread_t t2)
{
  return (ports0_macro_thread_equal(t1, t2));
} /* ports0_thread_equal() */


/*
 * Thread Control - ports0_thread_once()
 */

int ports0_thread_once(ports0_thread_once_t *once_control,
                       void (*init_routine)(void))
{
  return (_p0_actual_thread_once(once_control, init_routine));
}

/*
 * Ports0 - Thread-Specific Data
 *
 *   ports0_thread_key_create()
 *   ports0_thread_key_delete()
 *   ports0_thread_getspecific()
 *   ports0_thread_setspecific()
 */

#undef ports0_thread_key_create
#undef ports0_thread_key_delete
#undef ports0_thread_getspecific
#undef ports0_thread_setspecific

/*
 * Thread-Specific Data - ports0_key_create()
 */

int ports0_thread_key_create(ports0_thread_key_t *key,
                             destructor_t func)
{
  int rc;

  ports0_mutex_lock(&KeyInfo.keyMutex);
  if (KeyInfo.nextKey >= KeyInfo.keyTableSize) {
    rc = EAGAIN;
  } else {
    *key = KeyInfo.nextKey++;
    KeyInfo.destructorFunction[*key] = func;
    rc = 0;
  }
  ports0_mutex_unlock(&KeyInfo.keyMutex);
  return(rc);
}

/*
 * Thread-Specific Data - ports0_key_delete()
 */

int ports0_thread_key_delete(ports0_thread_key_t key)
{
  int rc = 0;

  return rc;
}

/*
 * Thread-Specific Data - ports0_thread_setspecific()
 */

int ports0_thread_setspecific(ports0_thread_key_t key,
                              void *value)
{
  _p0_thread_t *thread;

  if ( (key < 0) && (key >= KeyInfo.keyTableSize) ) {
    fprintf(stderr,"ports0_thread_setspecific: Key %d out of range %d..%d\n",
            key, 0, KeyInfo.keyTableSize);
    exit(0);
  }

  thread = _p0_thread_self();

  thread->keyTable[key] = value;
  return 0;
}


/*
 * Thread-Specific Data - ports0_thread_getspecific()
 *
 * We use the NT Tls functions to
 */

void *ports0_thread_getspecific(ports0_thread_key_t key)
{
  void *value;
  _p0_thread_t *thread;


  if ( (key < 0) && (key >= KeyInfo.keyTableSize)) {
    fprintf(stderr,"ports0_thread_getspecific: Key %d out of range %d..%d\n",
            key, 0, KeyInfo.keyTableSize);
    exit(0);
  }

  thread = _p0_thread_self();
  value = thread->keyTable[key];
  return (value);
}


/*
 * Ports0 - Mutex Handling
 *
 *   ports0_mutexattr_init()
 *   ports0_mutexattr_destroy()
 *   ports0_mutex_init()
 *   ports0_mutex_destroy()
 *   ports0_mutex_lock()
 *   ports0_mutex_trylock()
 *   ports0_mutex_unlock()
 */

#undef ports0_mutexattr_init
#undef ports0_mutexattr_destroy
#undef ports0_mutex_init
#undef ports0_mutex_destroy
#undef ports0_mutex_lock
#undef ports0_mutex_unlock
#undef ports0_mutex_trylock

/*
 * Mutexes - ports0_mutexattr_init
 */

int ports0_mutexattr_init( ports0_mutexattr_t *attr)
{
  int rc = 0;

  return rc;
}

/*
 * Mutexes - ports0_mutexattr destroy
 */

int ports0_mutexattr_destroy( ports0_mutexattr_t *attr)
{
  int rc = 0;

  return rc;
}

/*
 * Mutexes - ports0_mutex_init
 */

int ports0_mutex_init( ports0_mutex_t *mut, ports0_mutexattr_t *attr )
{
  /* operation either succeeds or aborts. */
  int rc=0;

  _P0_INIT_START_MAGIC_COOKIE(mut);

  /* Windows NT: Create an unnamed mutex which is not yet acquired. */
  mut->mutex = CreateMutex(NULL, FALSE, NULL);

  if (mut->mutex == 0)
    ports0_fatal("ports0_mutex_init: failed.\n");

  _P0_INIT_END_MAGIC_COOKIE(mut);

  return rc;
}

/*
 * Mutexes - ports0_mutex_lock
 */

int ports0_mutex_lock( ports0_mutex_t *mut )
{
  int rc = 0;

  WaitForSingleObject(mut->mutex, INFINITE);

  return rc;
}


/* ports0_mutex_trylock - Implementation Notes
 *
 * Getting this one to work in WindowsNT requires that you use the
 * WaitForSingleObject on the mutex structure with a timeout of zero
 * milliseconds. According to Microsoft on-line docs, this timeout
 * "Specifies the time-out interval, in milliseconds. The function returns
 * if the interval elapses, even if the object's state is nonsignaled. If
 * dwMilliseconds is zero, the function tests the object's state and returns
 * immediately. If dwMilliseconds is INFINITE, the function's time-out
 * interval never elapses."
 */

/*
 * Mutexes - ports0_mutex_trylock
 */

int ports0_mutex_trylock( ports0_mutex_t *mut )
{
  int rc = 0;

  if (WaitForSingleObject(mut->mutex, 0) != WAIT_OBJECT_0)
    rc = EBUSY;
  return rc;
}

/*
 * Mutexes - ports0_mutex_unlock
 */

int ports0_mutex_unlock( ports0_mutex_t *mut )
{
  int rc=0;

  ReleaseMutex(mut->mutex);

  return rc;
}

/*
 * Mutexes - ports0_mutex_destroy()
 */

int ports0_mutex_destroy( ports0_mutex_t *mut )
{
  int rc=0;

  PORTS0_INTERROGATE(mut, _P0_MUTEX_T, "destroy");

  /* It appears Windows NT cannot free a Mutex structure. */
  return rc;
}


/* Ports0 - Condition Variables
 *
 *   ports0_condattr_init
 *   ports0_condattr_destroy
 *   ports0_cond_init
 *   ports0_cond_destroy
 *   ports0_cond_wait
 *   ports0_cond_signal
 *   ports0_cond_broadcast
 */

/*
 *
 * Implementation Notes - Condition Variables in Win32
 *
 * With Windows NT you really have to build Condition Variables from higher
 * level primitives. This really is how it should be done anyway, especially
 * when you consider the sloppiness in the implementation of CV's in Solaris
 * and DCE pthreads.
 *
 * Now that I've gotten off my soapbox, the approach is to maintain two
 * data structures with the ports0_cond_t: a semaphore to queue up the
 * waiting threads, and a variable indicating the number of threads which
 * are queued. You need this variable to determine when signalling the
 * condition is appropriate. It is only appropriate when at least one waiting
 * thread is queued at the semaphore. As well, you need to know the number of
 * waiting threads when a broadcast is performed.
 */

/*
 * Condition Variables - ports0_cond_init
 */

/* Implementation Notes - ports0_cond_init
 *
 * The design:
 *   initialize the number of waiting threads to zero
 *   create a semaphore to queue up the threads. This semaphore is NOT
 *     signalled
 *   initialize attributes (presently not done)
 *   return success
 */

int ports0_cond_init( ports0_cond_t *cv, ports0_condattr_t *attr )
{
  int rc = 0;

  cv->mutex = CreateMutex(NULL, FALSE, NULL);
  cv->numberOfWaiters = 0;
  cv->semaphore = CreateSemaphore(NULL, 0, 1, NULL);

  /* Initialize condition variable attributes, if needed */
  return rc;
}

/*
 * Condition Variables - ports0_cond_wait
 */

/* Implementation Notes - ports0_cond_wait
 *
 * #1 : A count of the number of waiters is maintained in the CV. This count,
 *      we argue, must be protected, because there are certain accesses where
 *      atomicity cannot be guaranteed.
 * #2 : Release the external mutex, as done in pthreads.
 * #3 : Now await the condition being signalled.
 * #4 : Reacquire the external mutex, as done in pthreads.
 * #5 : Upon the condition being signalled, one less thread is awaiting
 *      the condition.
 */

int ports0_cond_wait( ports0_cond_t *cv, ports0_mutex_t *aMutex)
{
  int rc = 0;

  /* Notes: #1 */
  WaitForSingleObject(cv->mutex, INFINITE);
  cv->numberOfWaiters++;
  ReleaseMutex(cv->mutex);

  /* Notes: #2 */
  ports0_mutex_unlock(aMutex);

  /* Notes: #3 */
  WaitForSingleObject(cv->semaphore, INFINITE);

  /* Notes: #4 */
  ports0_mutex_lock(aMutex);

  /* Notes: #5 */
  WaitForSingleObject(cv->mutex, INFINITE);
  cv->numberOfWaiters--;
  ReleaseMutex(cv->mutex);

  return rc;
}

/*
 * Condition Variables - ports0_cond_signal
 */

int ports0_cond_signal( ports0_cond_t *cv )
{
  int rc = 0;
  ReleaseSemaphore(cv->semaphore, 1, NULL);
  return rc;
}

/*
 * Condition Variables - ports0_cond_broadcast
 */

int ports0_cond_broadcast( ports0_cond_t *cv )
{
  int rc = 0;
  int waiter;

  WaitForSingleObject(cv->mutex, INFINITE);
  ReleaseSemaphore(cv->semaphore, cv->numberOfWaiters, NULL);
  ReleaseMutex(cv->mutex);
  return rc;
}

/*
 * Condition Variables - ports0_cond_destroy
 */

int ports0_cond_destroy( ports0_cond_t *cv )
{
  int rc = 0;

  cv->numberOfWaiters = 0;
  CloseHandle(cv->semaphore);
  CloseHandle(cv->mutex);

  return rc;
}



/*
 * ports0_i_am_only_thread()
 */

ports0_bool_t ports0_i_am_only_thread(void)
{
  return (ports0_macro_i_am_only_thread());
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


void ports0_thread_prefork(void)
{
  /* Do nothing */
}

void ports0_thread_postfork(void)
{
  /* Do nothing */
}



