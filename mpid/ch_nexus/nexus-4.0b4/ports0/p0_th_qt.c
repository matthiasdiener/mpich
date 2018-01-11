/*
 * Ports0 Threads built on top of QuickThreads
 *
 * author: Craig Lee at Aerospace.
 * 1994 May
 * Preemption, July 1994
 * Solaris version, Sept 1994
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_qt.c,v 1.33 1996/10/19 22:35:58 carl Exp $";

#include "p0_internal.h"
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>

#ifdef HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif

/*
 *  Const shared data (const at least after init-time)
 */

static int maxProcessors = 1;
static int numProcessors = 1;  /* This should probably be in TKp */

static ports0_threadattr_t default_attrs;
static ports0_mutexattr_t default_mutex_attrs; /* initializers ?? */
static ports0_condattr_t default_cond_attrs;

/*
 *  Dynamic shared data
 */

static thread_kernel_t *TKp;

static ports0_bool_t		using_idle_thread = PORTS0_FALSE;
static ports0_thread_t   idle_thread = NULL;
static ports0_bool_t		idle_thread_done = PORTS0_FALSE;
static void *		idle_thread_func(void *arg);
static ports0_mutex_t    idle_mutex;
static ports0_cond_t     idle_cond;
static void			(*idle_func_save)(void);
static ports0_bool_t handler_blocked = PORTS0_FALSE;

static ports0_bool_t		preemptive_threads = PORTS0_TRUE;
static volatile int     timer_hits = 0;
static volatile int     check_slice_hits = 0;
static int              preemption_cnt = 0;
static struct itimerval time_slice_val;

#if defined(HAVE_LWP)
ucontext_t *lwp_cxt_vecp[MAXPROCS];
lwpid_t lwpid_vec[MAXPROCS];
#endif

#ifndef HAVE_THREAD_SAFE_STDIO
ports0_mutex_t ports0_stdio_mutex;
#endif

/*
 *  Processor private data
 *  There are three ways of storing processor private data:
 *	(1) one global in non-shared addr space region of each process,
 *	(2) an array of globals in one shared addr space, or
 *	(3) malloced and stored as thread-private data.
 *  SunOS and IRIX use (1) while Solaris uses (3).
 */

#if defined(HAVE_LWP)
#define pP ((PrivateProcessor_p)_lwp_getprivate())

#elif defined(HAVE_SPROC)
PrivateProcessor_p pP = (PrivateProcessor_p)&(PRDA->usr_prda);

#else
static PrivateProcessor ProcPriv;
#define pP (&ProcPriv)

#endif


/*
 *  Thread specific data key maintainance stuff
 */
static int used_keys;
static ports0_thread_key_destructor_func_t key_destructor[PORTS0_DATAKEYS_MAX];
static spin_lock_t key_lock;


PrivateProcessor *ProcessInit( PrivateProcessor *newpP, ports0_thread_t newt, ports0_thread_t idlet );
void ReadyqInit(void);
ports0_thread_t sched_next_activethread(int must_sched_something);

ports0_thread_t alloc_thread(void);
void alloc_stack( ports0_thread_t tp, int stacksize );
void only( void *argp, ports0_thread_t t, void *(*funcp)(void *arg) );
void cleanup(ports0_thread_t self);
void *csw_helper(qt_t *old_sp, void *old_thr, void *qlock);
void wakeup_handler(void);

void idle_loop( void *dummy );

#ifdef HAVE_SYS_SIGWAITING
void sigwaiting_catcher(void);
#endif
#ifdef HAVE_SPROC
void sigusr2_handler(void);
#endif

void time_slicer(void);
void *time_slice_helper(qt_t *old_sp, void *old_thr, void *qlock);

spin_lock_t nProc_lock;
spin_lock_t dummy_lock;

#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_HPUX) || \
    defined(TARGET_ARCH_AXP) || defined(TARGET_ARCH_AIX)
void spin_lock_init(spin_lock_t *lock)
{
	*lock = LOCK_INIT_VAL;
}

void spin_lock(spin_lock_t *lock)
{
	while (*lock) ;
	*lock = 1;
}

int test_and_get_spin_lock(spin_lock_t *lock)
{
	spin_lock_t init = *lock;

	spin_lock(lock);
	return((int)init);
}

void spin_unlock(spin_lock_t *lock)
{
	*lock = LOCK_INIT_VAL;
}
#endif
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
  char tmp_buf[100];
  int n_added;
  
ports0_stdio_lock();
  tmp_buf[0] = '\0';
  if  ( maxProcessors != 1 )  {
    sprintf( tmp_buf, "-maxProc %d ", maxProcessors );
  }
  if  ( default_attrs.stack_size != PORTS0_STACK_MIN )  {
    sprintf( tmp_buf+strlen(tmp_buf), "-stack %d ", default_attrs.stack_size );
  }
  if (!preemptive_threads)
  {
	sprintf(tmp_buf+strlen(tmp_buf), "-sched fifo ");
  }

  n_added = strlen(tmp_buf);
  if  ( n_added > size )
    ports0_fatal("_p0_thread_new_process_params(): Not enough room in buffer for arguments\n");

  strcpy( buf, tmp_buf );
ports0_stdio_unlock();
  return( n_added );
}


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
 * _p0_thread_usage_message()
 *
 * Print a usage message for this module's arguments to stdout.
 */
void _p0_thread_usage_message(void)
{
  printf("    -maxProc <integer> : max virtual processors (arch-dep),\n");
  printf("    -stack <integer>   : Set the default thread stack size.\n");
  printf("    -sched <string>    : Set the scheduler.\n\n");
  printf("                         fifo = non-preemptive\n");
  printf("                         rr   = preemptive (Round robin)\n");
}


void _p0_thread_init( int *argc, char **argv[] )
{
  int i;
  int arg_num;
  int len;
  ports0_thread_t newt, idlet;
  struct sigaction set;

  /*  Misc. initialization
   */

  default_attrs.stack_addr = 0;
#ifdef PORTS0_DEFAULT_STACK_SIZE
  default_attrs.stack_size = (PORTS0_DEFAULT_STACK_SIZE + (PAGESIZE-1))
	& ~(PAGESIZE-1);
#else
  default_attrs.stack_size = PORTS0_STACK_MIN;
#endif
  if  ((arg_num = ports0_find_argument(argc, argv, "maxProc", 2)) >= 0)
  {
    maxProcessors = atoi((*argv)[arg_num + 1]);
    if  ( MAXPROCS < maxProcessors )  {
      ports0_fatal("th_qt: maximum processors (%d) exceeded.\n", MAXPROCS );
    }
	if (maxProcessors <= 0)
	{
		ports0_fatal("th_qt: must have at least on processor.\n");
	}
	ports0_remove_arguments(argc, argv, arg_num, 2);
  }
  if ((arg_num = ports0_find_argument(argc, argv, "stack", 2)) >= 0)
  {
      len = atoi((*argv)[arg_num + 1]);
      if  ( len < PORTS0_STACK_MIN )  {
	    ports0_fatal("th_qt: requested stack size too small.\n");
      }
      default_attrs.stack_size = (len + (PAGESIZE-1)) & ~(PAGESIZE-1);
	  ports0_remove_arguments(argc, argv, arg_num, 2);
    }
  if ((arg_num = ports0_find_argument(argc, argv, "sched", 2)) >= 0)
  {
		if (!strcmp((*argv)[arg_num + 1], "fifo"))
		{
			preemptive_threads = PORTS0_FALSE;
		}
		else if (!strcmp((*argv)[arg_num + 1], "rr"))
		{
			preemptive_threads = PORTS0_TRUE;
		}
		else
		{
			ports0_fatal("th_qt: scheduler not defined.\n");
		}
		ports0_remove_arguments(argc, argv, arg_num, 2);
	}

  spin_lock_init( &key_lock );
  spin_lock_init( &nProc_lock );

  sigaction(WAKEUP_SIGNAL, NULL, &set);
  set.sa_handler = wakeup_handler;
  set.sa_flags = 0;
  sigaction(WAKEUP_SIGNAL, &set, NULL);

#ifdef HAVE_SPROC
/* JGG
  if  ( usconfig( CONF_INITUSERS, maxProcessors) == -1 )
JGG */
  if  ( usconfig( CONF_INITUSERS, maxProcessors * 2) == -1 )
    ports0_fatal("_p0_thread_init: us_config failed\n");

	sigaction(SIGUSR2, NULL, &set);
	set.sa_handler = sigusr2_handler;
	set.sa_flags = 0;
	sigaction(SIGUSR2, &set, NULL);

  if  ( prctl( PR_SETEXITSIG, SIGUSR2 ) == -1 )
    ports0_fatal("_p0_thread_init: prctl failed\n");
#endif

  /*  Malloc and init the Thread Kernel structure which has all of the
   *  ready queues and the free queue.
   */

  TKp = (thread_kernel_t *)nq_malloc( sizeof(thread_kernel_t) );
  if  ( TKp == NULL )  ports0_fatal("_p0_thread_init: out of memory\n");
#ifdef HAVE_SPROC
  TKp->nProcs = sysmp(MP_NPROCS);
#else
  TKp->nProcs = MAXPROCS;
#endif
  TKp->IdleProcs = 0;
  TKp->shutdown = PORTS0_FALSE;
  TKp->handlerThread = NULL;
  TKp->activeIdle = 0;
  TKp->ActiveThreads = TKp->nThreads = 1;
  ReadyqInit();
  TKp->free_count = 0;
  ThreadQInit( &(TKp->freeQ) );
  /*
   * Although _p0_idle_t is a different type than ThreadQ, the macros
   * that were written for ThreadQ should work for _p0_idle_t if they
   * are general, like init(), put(), && get().  Specifics should not be
   * used, like FreeQget(), ReadyQCat(), etc.
   */
  ThreadQInit(&(TKp->idleQ));
  for  ( i=0; i<MAXPROCS; i++ )  TKp->idle_vec[i] = 0;

  /*  th_qt uses the idea of a virtual processor: something that can runs threads
   *  and is schedulable by the OS.  Under Solaris, this is an lwp.  Under IRIX,
   *  this is an sproc.  Under SunOS, it could be a unix process.  In any case,
   *  the first VP is special since it already has a stack and Ports0 is running on it.
   *  We must still, however, alloc a header for it.  The stack point in the header
   *  will be set the first time it context switches.
   */

  newt = alloc_thread();
  newt->tid = 0;
  newt->stack_size = 0;
  newt->stack_base = 0;
  newt->sp = NULL;

  /*  The current vp also needs an idle thread.  Idle threads don't need
   *  to inherit the context pointer.
  */

  idlet = alloc_thread();
  alloc_stack( idlet, IDLE_THREAD_STACKSIZE );
  idlet->sp = QT_ARGS( idlet->sp, idlet, idlet, idle_loop, only );

  /*  Init the processor-private data which includes the new current thread and
   *  idle thread headers.
   */

#if defined(HAVE_LWP)
  /*  For the preemptive Solaris version, we have to call _lwp_setprivate
   *  before actually initializing the processor private data because
   *  ProcessInit kicks off VTARLM immediately after initialization.
   */
  { PrivateProcessor *pPtemp = malloc(sizeof(PrivateProcessor));
    _lwp_setprivate( (void *)pPtemp );
    ProcessInit( pPtemp, newt, idlet );
  }
#else
  ProcessInit( pP, newt, idlet );
#endif

  /* Initialize thread specific keys */

  used_keys = 0;
  for  ( i=0; i<PORTS0_DATAKEYS_MAX; i++ )  key_destructor[i] = NULL;

  /* Initialize monitor for handler thread */

  ports0_mutex_init( &idle_mutex, NULL );
  ports0_cond_init( &idle_cond, NULL );

#ifndef HAVE_THREAD_SAFE_STDIO
  ports0_mutex_init( &ports0_stdio_mutex, NULL );
#endif
}

/*
 *  Initialize process private data
 */
PrivateProcessor *ProcessInit( PrivateProcessor *newpP, ports0_thread_t newt, ports0_thread_t idlet )
{
  if  ( newpP == NULL )  ports0_fatal("ProcessInit: null new pP\n");
  newpP->myId = ( newt == idlet ) ? newt->tid : 0 ;
#ifdef DONT_INCLUDE
#ifdef HAVE_SPROC
  sysmp(MP_MUSTRUN, newpP->myId % TKp->nProcs);
#endif
#endif
  newpP->timer_hit = newpP-> dont_preempt = 0;
  newpP->lastLook = newpP->putQ = &(TKp->readyList[newpP->myId % maxProcessors]);
  newpP->activeThread = newt;
  newpP->idleThread = idlet;

  /*  handlerThread represents the global handler thread _if_ it gets created.
      When created, the global handler thread runs as a normal thread with no special
      handling except when executing _p0_thread_shutdown_handler_thread(), i.e., when it's
      trying to shut itself down.  In this case, it just sets the done flag and doesn't block.
  */

  newpP->handlerThread = NULL;

  /*
   *  Setup vtimer
   */

  if  ( preemptive_threads )  {
    StartTimer();
  }

#if defined(HAVE_SYS_SIGWAITING)
  signal( SIGWAITING, sigwaiting_catcher );
#endif

  /*  tmp_affinity done here, if possible and desired */
  /*  Can't do ports0_debug_printf because it requires the context and that
      isn't init'ed yet
  */
  /*
     printf("Process %d initialized\n", getpid());
  */
  return newpP;
}


void wakeup_handler(void)
{
    ports0_debug_printf(3, ("idle thread wakeup\n"));
}


#ifdef HAVE_SYS_SIGWAITING
void sigwaiting_catcher(void)
{
  ports0_debug_printf(2,("\tSIGWAITING caught\n"));
}
#endif


#ifdef HAVE_SPROC
void sigusr2_handler(void)
{
  prctl( PR_SETEXITSIG, 0 );
  if  ( pP->activeThread == pP->idleThread )
    ports0_debug_printf(2,("Process %d terminating\n", getpid() ));
  else
    ports0_debug_printf(2,("Warning: Process %d terminating while not in idle thread\n", getpid() ));
  exit(0);
}
#endif


void *csw_helper( qt_t *old_sp, void *old_thr, void *qlock )
{
  /* Save previous thread's stack pointer. */

  ((ports0_thread_t)old_thr)->sp = old_sp;

  /* Unlock previous thread's queue now that it's safely put away. */

  spin_unlock( qlock );

  /* Since we just context-switched, throw away any timer hits
  ** and allow preemption.
  */
  START_PREEMPTION;
  return NULL;
}


void cleanup( ports0_thread_t self )
{
  int i;

  DONT_PREEMPT;
  Ports0Assert2( (self != NULL), ("cleanup: null self\n") );

  /* Don't need to CHECK_SLICE later since we're headed for a csw anyway. */

  for  ( i=0; i<used_keys; i++ )  {
    if  ( key_destructor[i] && self->key_data[i] )
      (*(key_destructor[i]))( self->key_data[i] );
    self->key_data[i] = NULL;
  }

  atom_dec(TKp->ActiveThreads);
  pP->activeThread = sched_next_activethread(MUST_SCHED_SOMETHING);

  /*  Can't let anybody see me on the freeq until I switch.  */

  if  ( ((long)self) & 0x3 )  {
    ports0_debug_printf(2,("cleanup: alignment error: %x %x\n",pP->activeThread,self));
  }

  Ports0Assert2( (pP->activeThread != NULL), ("cleanup: null activeThread\n") );
  FreeQPut_switch( self, pP->activeThread );

  /*  Control never gets here.  */
}


ports0_thread_t sched_next_activethread( int must_sched_something )
{
  int n = maxProcessors;
  ThreadQ *q = pP->lastLook;
  ports0_thread_t next;

  /*  Scan all ready queues starting with mine.  */
  
  while  ( 0 < n-- )  {
    spin_lock( &(q->lock) );
    if  ( q->qhead )  {		/* Got a ready thread */
      ThreadGet( next, q );
      spin_unlock(&(q->lock));
      return next;
    }
    spin_unlock(&(q->lock));
    q = NextQ(q);
  }
  
  /*  If I can't find any ready threads (and I don't have a global handler
   *  thread) _and_ I have to schedule something, then switch to the idle thread.
   */
  
	if  ( must_sched_something )
	{
		if (using_idle_thread && TKp->handlerThread != pP->activeThread && pP->handlerThread && !handler_blocked && TKp->IdleProcs == TKp->activeIdle)
		{
			return pP->handlerThread;
		}
		else
		{
			return pP->idleThread;
		}
	}

  
  return NULL;
}


void ReadyqInit(void)
{
  int i;

  for  ( i=0; i<maxProcessors; i++ )  {
    ThreadQInit( &(TKp->readyList[i]) );
  }
  TKp->firstQ = TKp->readyList;
  TKp->lastQ = TKp->readyList + (maxProcessors - 1);
}


/*
 * Allocate a red-zoned stack segment for a thread.
 * Threads and their stacks are never deallocated;
 * they are retained on a free list for future use.
 */

void alloc_stack( ports0_thread_t tp, int stacksize )
{
  char *dead_page;

  tp->stack_size = stacksize;
  tp->stack_base = (char *)nq_memalign( PAGESIZE, stacksize );

  if  ( tp->stack_base == NULL )
    { errno = ENOMEM; _p0_report_bad_rc( 1, "<Stack allocation>" ); }

  tp->sp = QT_SP( tp->stack_base, stacksize );

#ifdef QT_GROW_DOWN
  dead_page = tp->stack_base;
#else
  dead_page = tp->stack_base + stacksize - PAGESIZE;
#endif

  if  ( mprotect( dead_page, PAGESIZE, PROT_NONE ) )
    _p0_report_bad_rc( 1, "Redzoning failed" );
}


ports0_thread_t alloc_thread(void)
{
  ports0_thread_t newt;
  void **kdp, **kdp_end;

  newt = (ports0_thread_t)nq_malloc(sizeof(ports0_thread_s));
  if  ( newt == NULL )
    { errno = ENOMEM; _p0_report_bad_rc( 1, "<Thread allocation>" ); }

  kdp = newt->key_data;
  kdp_end = kdp + PORTS0_DATAKEYS_MAX;
  while  ( kdp < kdp_end )  *kdp++ = NULL;
  newt->next = NULL;

  return newt;
}


/*  time_slicer runs on the interrupted thread's stack so I can block
    from here.  Since time_slicer (the sig handler) doesn't actually
    return until the thread is rescheduled, time_slice_helper has to
    explicitly reenable SIGVTALRM and restart the timer.
*/

void time_slicer(void)
{
  ports0_thread_t self, next;

  timer_hits++;
  if  ( pP->dont_preempt )  pP->timer_hit = 1;
  else if  ( (next = sched_next_activethread(OK_TO_RETURN_NULL)) )  {
    preemption_cnt++;
    self = pP->activeThread;
    pP->activeThread = next;
    ThreadPut( self, (pP->putQ) );
    QT_BLOCK( time_slice_helper, self, NULL, next->sp );
  }
}


void *time_slice_helper( qt_t *old_sp, void *old_thr, void *qlock )
{
  ((ports0_thread_t)old_thr)->sp = old_sp;
  if  ( setitimer( ITIMER_VIRTUAL, &time_slice_val, NULL ) )
    _p0_report_bad_rc( 1, "setitimer failed\n" );

  return NULL;
}

/*
 * ports0_thread_create
 */
/*
  Note: the new thread has to inherit the calling threads context.

  FreeQGet is a simple macro that only checks the head of the free queue
  for a stack that is at least as big as stk_size.  Hence, the user will
  always get a stack as big as requested but if the requested stack size
  varies widely and often, this will tend to create more thread headers
  and stacks than are absolutely necessary.
*/

int ports0_thread_create( ports0_thread_t *tp,
			 ports0_threadattr_t *attrp,
			 ports0_thread_func_t funcp,
			 void *argp )
{
  int newProcnum = 0;
  ports0_thread_t newt;
  int stk_size = ((attrp) ? attrp->stack_size : default_attrs.stack_size );

  DONT_PREEMPT;
  FreeQGet( newt, stk_size );
  if  ( newt == NULL )  {
    newt = alloc_thread();
    alloc_stack( newt, stk_size );
  }  else  {
    newt->sp = QT_SP( newt->stack_base, newt->stack_size );
    newt->next = NULL;
  }

  /* Put single arg on stack */

  newt->sp = QT_ARGS( newt->sp, argp, newt, funcp, only );

  atom_inc(TKp->ActiveThreads);
  if (!using_idle_thread || (using_idle_thread && pP->handlerThread)) {
    ReadyQPut( newt );
  }
  if  ( tp )  *tp = newt;

  /*  The purpose of the following chunks of code is to start another
   *  virtual processor if needed.  Every vp (except the first vp) starts
   *  by running the idle loop which looks for work to do.  Note that the
   *  processor id is passed to the new vp on the tid of the idle thread.
   *  I know this is a hack but idle threads don't really use their tids.
   */

  spin_lock( &nProc_lock );
  if  ( numProcessors < TKp->ActiveThreads && numProcessors < maxProcessors )
    newProcnum = numProcessors++;
  spin_unlock( &nProc_lock );

#ifdef HAVE_LWP
  if  ( newProcnum )  {
    ucontext_t *new_cxt;
    PrivateProcessor *new_pP;
    int error;

    newt = alloc_thread();
    newt->tid = newProcnum;
    alloc_stack( newt, IDLE_THREAD_STACKSIZE );
    new_pP = (PrivateProcessor *)malloc(sizeof(PrivateProcessor));
    new_pP = ProcessInit( new_pP, newt, newt );
    new_cxt = lwp_cxt_vecp[new_pP->myId] = (ucontext_t *)malloc(sizeof(ucontext_t));
    if  ( new_cxt == NULL )  ports0_fatal("ports0_thread_create: out of memory\n");

    sigprocmask( SIG_SETMASK, NULL, &(new_cxt->uc_sigmask) );
    _lwp_makecontext( new_cxt, idle_loop, (void *)newt, (void *)new_pP,
		      newt->stack_base, newt->stack_size );
    error = _lwp_create( new_cxt, NULL, lwpid_vec + new_pP->myId );
    if  ( error )  ports0_fatal("ports0_thread_create: _lwp_create failed\n");
  }
#endif

#ifdef HAVE_SPROC
  if  ( newProcnum )  {
    newt = alloc_thread();
    newt->tid = newProcnum;
    newt->stack_size = 0;
    newt->stack_base = 0;
    newt->sp = NULL;
    
    /*  I may want to call sprocsp instead so I can specify the stack size.
     *  (Idle threads don't need big stacks unless they do polling.)
     *  I may also want to keep the child pid returned by sproc.  Finally,
     *  since process private data is in PRDA, ProcessInit can't be called
     *  until after the sproc.  This means putting the call in the beginning
     *  of idle_loop.
     */
    if  ( (sproc( idle_loop, PR_SALL, newt )) == -1 )
      ports0_fatal("ports0_thread_create: sproc failed\n");
  }
#endif

  CHECK_SLICE;
  return 0;
}


void only( void *argp, ports0_thread_t t, void *(*funcp)(void *arg) )
{
  t->tid = atom_inc(TKp->nThreads) - 1;

  /* Call the user function */
  (*funcp)(argp);

  /* It's an error for only to return; cleanup never does. */
  cleanup( t );
}


/*  Every virtual processor has an idle thread running the idle loop.  Besides
 *  checking for runnable threads, idle_loop must also poll all of the protocol
 *  modules since this may unblock some threads and this may be what we're
 *  actually waiting for.
 */
void idle_loop( void *self )
{
  ports0_thread_t next;
  ports0_thread_t myself;
#ifdef HAVE_SPROC
  int ticksNsec = sysconf(3);
#endif

  /*  self must be assigned to an automatic variable since Solaris seems to
   *  muck with formal variables across context switches like below.
   */
  atom_inc(TKp->IdleProcs);
  myself = (ports0_thread_t)self;

  Ports0Assert2( (myself != NULL), ("idle_loop: null self\n") );

#ifdef HAVE_SPROC
  if  ( myself->stack_base == 0 )  ProcessInit( pP, myself, myself );
#endif

  atom_inc(TKp->activeIdle);
  for  (;;)  {
    DONT_PREEMPT;
    /*ports0_poll();*/
/* JGG
    if  ( (next = sched_next_activethread( OK_TO_RETURN_NULL )) )  {
JGG */ 
    next = sched_next_activethread(MUST_SCHED_SOMETHING);
    if (next != pP->idleThread) {
      /* cxt to next thread without putting self (idleThread) on any readyQ */
      pP->activeThread = next;
      atom_dec(TKp->activeIdle);
      QT_BLOCK( csw_helper, (ports0_thread_t)myself, &dummy_lock, next->sp );
      atom_inc(TKp->activeIdle);
    }
    else {
      if (TKp->shutdown) {
	    atom_dec(TKp->IdleProcs);
	    numProcessors--;
      }
#ifdef HAVE_SPROC
	  sginap(ticksNsec);
#else
#ifdef JGG
	IdleQPut();
	pause();
#endif
#endif
    }
  }
}


ports0_thread_t _p0_thread_self( void )
{
	return pP->activeThread;
}


int _p0_threadattr_init( ports0_threadattr_t *attrp )
{
  if  ( attrp == NULL )  return EINVAL;
  attrp->stack_addr  = default_attrs.stack_addr;
  attrp->stack_size  = default_attrs.stack_size;
  return 0;
}


int _p0_threadattr_setstacksize( ports0_threadattr_t *attrp, size_t len )
{
  if  ( (attrp == NULL) || (len < PORTS0_STACK_MIN) )  return EINVAL;
  attrp->stack_size = ((int)len + (PAGESIZE-1)) & ~(PAGESIZE-1);
  return 0;
}


int _p0_threadattr_getstacksize( ports0_threadattr_t *attrp, size_t *lenp )
{
  if  ( (attrp == NULL) || (lenp == NULL) )  return EINVAL;
  *lenp = (size_t)(attrp->stack_size);
  return 0;
}

/*
 * _p0_thread_create_handler_thread()
 *
 **  This bunch of routines have been lifted from th_c.c.
 *
 * If we have preemptive threads:
 *   Create a normal priority thread that will sit in a
 *   loop calling ports0_poll().
 *
 * If we have non-preemptive threads:
 *   Create a low priority thread which will only become active when
 *   all other threads have suspended.  This thread should:
 *	1) Call _p0_blocking_poll(), to check for available messages.
 *	2) Keep track of idle time.
 *
 *   If a thread module does not support priorities, this same
 *   functionality can be hacked into ports0_cond_wait() using a
 *   counter of suspend threads.
 *
 * TODO: We can probably just use ports0_thread_create() once attributes are
 * implemented.
 *
 **  I just used ports0_thread_create since it seems to do everything required.
 **  If the global handler thread cannot be identical to the idle thread, then
 **  a general priority scheme has to be implemented.
 */
void ports0_idle_callback(void (*idle_func)(void))
{
    ports0_debug_printf(2,("ports0_idle_callback(): Creating idle thread\n"));
	if (idle_func)
	{
		if (using_idle_thread)
		{
			ports0_debug_printf(2,("ports0_idle_callback():Changing idle callback for existing idle thread\n"));
			idle_func_save = idle_func;
		}
		else
		{
			ports0_debug_printf(2,("ports0_idle_callback(): Creating idle thread\n"));
			using_idle_thread = PORTS0_TRUE;
			idle_thread_done = PORTS0_FALSE;
			idle_func_save = idle_func;

			ports0_thread_create(&idle_thread, NULL, idle_thread_func, NULL);
			TKp->handlerThread = idle_thread;
			pP->handlerThread = idle_thread;
		}
	}
	else
	{
		_p0_thread_shutdown_idle_thread();
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
  int im_done;

    ports0_debug_printf(2,("idle_thread_func(): Entering\n"));

	_p0_thread_yield(MUST_SCHED_SOMETHING);
    for  (;;)  {

      ports0_mutex_lock( &idle_mutex );
      im_done = idle_thread_done;
      ports0_mutex_unlock( &idle_mutex );
      if  ( im_done )  break;

	  (*idle_func_save)();
	  ports0_debug_printf(6,("idle_thread_func(): calling yield\n"));
	  _p0_thread_yield(MUST_SCHED_SOMETHING);
	  ports0_debug_printf(6,("idle_thread_func(): Has been scheduled\n"));
    }

    ports0_debug_printf(2,("idle_thread_func(): Exiting\n"));
    ports0_cond_signal( &idle_cond );
    ports0_thread_exit( ports0_thread_self() );
    return (NULL);
} /* idle_thread_func() */


/*
 * _p0_thread_shutdown_idle_thread()
 * This in effect does a join on the handler thread _unless_ this message is being
 * handled as a result of the handler thread doing a poll, that is to say, the
 * handler thread is trying to shut itself down.
 */
void _p0_thread_shutdown_idle_thread(void)
{
    if (using_idle_thread)
    {
      ports0_mutex_lock( &idle_mutex );
      idle_thread_done = PORTS0_TRUE;
      if  ( ports0_thread_self() != TKp->handlerThread )
	ports0_cond_wait( &idle_cond, &idle_mutex );
      ports0_mutex_unlock( &idle_mutex );
    }
} /* _p0_thread_shutdown_idle_thread() */

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
 * _p0_thread_key_create()
 */
int _p0_thread_key_create( ports0_thread_key_t *key, ports0_thread_key_destructor_func_t dfunc )
{
  int i;

  DONT_PREEMPT;
  spin_lock( &key_lock );
  if  ( used_keys == PORTS0_DATAKEYS_MAX )  {
    spin_unlock( &key_lock );
    errno = EAGAIN;
    return -1;
  }
  i = used_keys++;

  pP->activeThread->key_data[i] = NULL;
  key_destructor[i] = dfunc;
  spin_unlock( &key_lock );
  CHECK_SLICE;

  *key = i;
  return 0;
}  /* _p0_thread_key_create() */


/*
 * _p0_thread_setspecific()
 */
int _p0_thread_setspecific( ports0_thread_key_t key, void *value )
{
  if  ( key < 0  || used_keys <= key )
    { errno = EINVAL;  return -1;  }

  pP->activeThread->key_data[key] = value;
  return 0;
} /* _p0_thread_setspecific() */


/*
 * _p0_thread_getspecific()
 */
void *_p0_thread_getspecific( ports0_thread_key_t key)
{
  void *value;

  if  ( key < 0  || used_keys <= key )
  {
	return NULL;
  }

  value = pP->activeThread->key_data[key];
  return (value);
} /* _p0_thread_getspecific() */


/*
 *  _p0_thread_once()
 */
int _p0_thread_once( ports0_thread_once_t *once_cntl, void (*init_routine)(void) )
{
  DONT_PREEMPT;
  spin_lock( &(once_cntl->lock) );
  if  ( once_cntl->init )  {
    once_cntl->init = 0;
    spin_unlock( &(once_cntl->lock) );
    (*init_routine)();
  }  else  {
    spin_unlock( &(once_cntl->lock) );
  }
  CHECK_SLICE;
  return 0;
} /* _p0_thread_once() */


/*
 * _p0_thread_yield()
 */
void _p0_thread_yield(int return_null)
{
  ports0_thread_t self, next;

  DONT_PREEMPT;
  if  ( (next = sched_next_activethread(return_null)) )  {
    self = pP->activeThread;
    pP->activeThread = next;
    if (self != TKp->handlerThread) {
      ReadyQPut_switch( self, next );
    }
    else {
	  spin_lock(&(pP->putQ->lock));
      QT_BLOCK( csw_helper, (ports0_thread_t)self, &(pP->putQ->lock), next->sp );
    }
    /* Don't need to CHECK_SLICE here since we just did a csw. */
  }  else  {
    START_PREEMPTION;
    /* We can just throw away timer hits since there's no one to csw to. */
  }
} /* _p0_thread_yield() */


/*
 * _p0_i_am_only_thread()
 */
ports0_bool_t _p0_i_am_only_thread(void)
{
	int n = maxProcessors;
	ThreadQ *q = pP->lastLook;

	while (0 < n--)
	{
		if (q->qhead)
		{
			return PORTS0_FALSE;
		}
		q = NextQ(q);
	}
	if (TKp->activeIdle == TKp->IdleProcs)
	{
		return PORTS0_TRUE;
	}
	if (pP->activeThread == TKp->handlerThread)
	{
		if (TKp->activeIdle + 1 == TKp->IdleProcs)
		{
			return PORTS0_TRUE;
		}
	}
	return PORTS0_FALSE;
} /* _p0_i_am_only_thread() */


/*
 * ports0_thread_exit()
 */
void ports0_thread_exit( void *xstatus )
{
  cleanup( pP->activeThread );
} /* ports0_thread_exit() */


/*
 *
 *  Mutex stuff
 *
 */

int _p0_mutex_init( ports0_mutex_t *nmutex, ports0_mutexattr_t *attr )
{
  nmutex->mutex.locked = MUTEX_UNLOCKED;
  ThreadQInit( &(nmutex->mutex.waiters) );
  return 0;
}


int _p0_mutex_destroy( ports0_mutex_t *nmutex )
{
  nmutex->mutex.locked = MUTEX_UNLOCKED;
  ThreadQInit( &(nmutex->mutex.waiters) );
  return 0;
}


int _p0_mutex_lock( ports0_mutex_t *nmutex )
{
  ports0_thread_t self;

  DONT_PREEMPT;
  spin_lock( &(nmutex->mutex.waiters.lock) );

  if  ( nmutex->mutex.locked )  {
    self = pP->activeThread;
    if (self == TKp->handlerThread) {
      handler_blocked = PORTS0_TRUE;
    }
    Ports0Assert2(( (nmutex->mutex.locked >> 1) != self->tid),
		 ("Recursive mutex error: tid = %d\n", self->tid) );
    ThreadPut( self, &(nmutex->mutex.waiters) );
    pP->activeThread = sched_next_activethread(MUST_SCHED_SOMETHING);
    QT_BLOCK( csw_helper, self, &(nmutex->mutex.waiters.lock), pP->activeThread->sp );

    /*  This thread is resumed with MUTEX_LOCKED already.       */
    /*  Don't need to CHECK_SLICE here since we just did a csw. */

  }  else  {

    /*  This simply allows a debugger to see who's holding a mutex. */
    nmutex->mutex.locked = (pP->activeThread->tid << 1) | MUTEX_LOCKED;
    spin_unlock( &(nmutex->mutex.waiters.lock) );
    CHECK_SLICE;

  }

  return 0;
}

int _p0_mutex_trylock(ports0_mutex_t *nmutex)
{
    ports0_thread_t self;
    int rc;

    DONT_PREEMPT;
    rc = 0;
    spin_lock( &(nmutex->mutex.waiters.lock) );

    if (nmutex->mutex.locked)
    {
	rc = EBUSY;
    }
    else
    {
	/* This simply allows a debugger to see who's holding a mutex */
	nmutex->mutex.locked = (pP->activeThread->tid << 1) | MUTEX_LOCKED;
    }

    spin_unlock( &(nmutex->mutex.waiters.lock) );
    CHECK_SLICE;

    return (rc);
} /* _p0_mutex_trylock() */


int _p0_mutex_unlock( ports0_mutex_t *nmutex )
{
  ports0_thread_t head_waiter;

  DONT_PREEMPT;
  spin_lock( &(nmutex->mutex.waiters.lock) );

  if  ( nmutex->mutex.waiters.qhead )  {

    ThreadGet( head_waiter, &(nmutex->mutex.waiters) );
    nmutex->mutex.locked = (head_waiter->tid << 1) | MUTEX_LOCKED;
    spin_unlock( &(nmutex->mutex.waiters.lock) );
    /*  Leave MUTEX_LOCKED and resume this guy.  */
    if (head_waiter == TKp->handlerThread) {
      handler_blocked = PORTS0_FALSE;
    }
    ReadyQPut( head_waiter );

  }  else  {

    nmutex->mutex.locked = MUTEX_UNLOCKED;
    spin_unlock( &(nmutex->mutex.waiters.lock) );

  }

  CHECK_SLICE;
  return 0;
}


/*
 *
 *  Condition variable stuff
 *
 */
int _p0_cond_init( ports0_cond_t *ncond, ports0_condattr_t *attr )
{
  ncond->cond.mutex = NULL;
  ThreadQInit( &(ncond->cond.waiters) );
  return 0;
}


int _p0_cond_destroy( ports0_cond_t *ncond )
{
  ncond->cond.mutex = NULL;
  ThreadQInit( &(ncond->cond.waiters) );
  return 0;
}

/*
 *  This routine is just like _p0_mutex_unlock except
 *  that it does not call DONT_PREEMPT or CHECK_SLICE
 *  because this is in the middle of a _p0_cond_wait.
 */
int cond_mutex_unlock( ports0_mutex_t *nmutex )
{
  ports0_thread_t head_waiter;

  spin_lock( &(nmutex->mutex.waiters.lock) );

  if  ( nmutex->mutex.waiters.qhead )  {

    ThreadGet( head_waiter, &(nmutex->mutex.waiters) );
    nmutex->mutex.locked = (head_waiter->tid << 1) | MUTEX_LOCKED;
    spin_unlock( &(nmutex->mutex.waiters.lock) );
    /*  Leave MUTEX_LOCKED and resume this guy.  */
    if (head_waiter == TKp->handlerThread) {
      handler_blocked = PORTS0_FALSE;
    }
    ReadyQPut( head_waiter );

  }  else  {

    nmutex->mutex.locked = MUTEX_UNLOCKED;
    spin_unlock( &(nmutex->mutex.waiters.lock) );

  }

  return 0;
}


int _p0_cond_wait( ports0_cond_t *ncond, ports0_mutex_t *mutex )
{
  ports0_thread_t self;
  ports0_mutex_t *mymutex = mutex;

  DONT_PREEMPT;
  /* Don't need to CHECK_SLICE later since we're headed for a csw anyway. */
  spin_lock( &(ncond->cond.waiters.lock) );

  /*  Put myself on the condition waiters list, release the mutex,
      and schedule another thread.
  */

  self = pP->activeThread;
  ThreadPut( self, &(ncond->cond.waiters) );
  cond_mutex_unlock( mymutex );
  pP->activeThread = sched_next_activethread(MUST_SCHED_SOMETHING);
  if (self == TKp->handlerThread) {
    handler_blocked = PORTS0_TRUE;
  }
  QT_BLOCK( csw_helper, self, &(ncond->cond.waiters.lock), pP->activeThread->sp );

  /*  When I come back here, I need to have the mutex.  Since the signaler
   *  need not own the mutex lock, I can't be guarenteed of being resumed
   *  from the mutex waiter list with the mutex lock.  I've got to contend
   *  for it from my ReadyQ like everybody else.
   */
  if (self == TKp->handlerThread) {
    handler_blocked = PORTS0_FALSE;
  }
  ports0_mutex_lock( mymutex );

  return 0;
}


int _p0_cond_signal( ports0_cond_t *ncond )
{
  ports0_thread_t head_waiter;

  DONT_PREEMPT;
  spin_lock( &(ncond->cond.waiters.lock) );

  /*  Resume head waiter (if one), unlock and return.  */

  if  ( ncond->cond.waiters.qhead )  {
    ThreadGet( head_waiter, &(ncond->cond.waiters) );
    spin_unlock( &(ncond->cond.waiters.lock) );
    ReadyQPut( head_waiter );
  }  else  {
    spin_unlock( &(ncond->cond.waiters.lock) );
  }    

  CHECK_SLICE;
  return 0;
}


int _p0_cond_broadcast( ports0_cond_t *ncond )
{
  DONT_PREEMPT;
  spin_lock( &(ncond->cond.waiters.lock) );

  /*  Resume all waiters (if any), unlock and return.  */

  if  ( ncond->cond.waiters.qhead )  {
    ReadyQCat( ncond->cond.waiters.qhead, ncond->cond.waiters.qtail );
    ThreadQInit( &(ncond->cond.waiters) );
  }  else  {
    spin_unlock( &(ncond->cond.waiters.lock) );
  }    

  CHECK_SLICE;
  return 0;
}


int get_activethreads(void)
{ return TKp->ActiveThreads; }

int get_mytid(void)
{ return pP->activeThread->tid; }


/*
 * _p0_thread_shutdown()
 */
int _p0_thread_shutdown(void)
{
  ports0_debug_printf(2,("th_qt: preemptions: %d  deferred yields: %d\n", preemption_cnt, check_slice_hits ));
  ports0_debug_printf(2,("numProcessors = %d\n", numProcessors ));
  TKp->shutdown = PORTS0_TRUE;
  return (0);
} /* _p0_thread_shutdown() */


/*
 * _p0_thread_abort()
 */
void _p0_thread_abort(int return_code)
{
} /* _p0_thread_abort() */


void _p0_report_bad_rc( int rc, char *message )
{
  char achMessHead[] = "[Thread System]";

  if( rc!=_P0_THR_SUCCESS ) {
    ports0_stdio_lock();
    fprintf( stderr, message );
    ports0_stdio_unlock();
    DONT_PREEMPT;
    switch( errno ) {
    case EAGAIN:
      ports0_fatal("%s %s\n%s system out of resources (EAGAIN)\n",
		  achMessHead, message, achMessHead);
      break;
    case ENOMEM:
      ports0_fatal("%s %s\n%s insufficient memory (ENOMEM)\n",
		  achMessHead, message, achMessHead );
      break;
    case EINVAL:
      ports0_fatal("%s %s\n%s invalid value passed to thread interface (EINVAL)\n",
		  achMessHead, message, achMessHead );
      break;
    case EPERM:
      ports0_fatal("%s %s\n%s user does not have adequate permission (EPERM)\n",
		  achMessHead, message, achMessHead );
      break;
    case ERANGE:
      ports0_fatal("%s %s\n%s a parameter has an invalid value (ERANGE)\n",
		  achMessHead, message, achMessHead );
      break;
    case EBUSY:
      ports0_fatal("%s %s\n%s mutex is locked (EBUSY)\n",
		  achMessHead, message, achMessHead );
      break;
    case EDEADLK:
      ports0_fatal("%s %s\n%s deadlock detected (EDEADLK)\n",
		  achMessHead, message, achMessHead );
      break;
    case ESRCH:
      ports0_fatal("%s %s\n%s could not find specified thread (ESRCH)\n",
		  achMessHead, message, achMessHead );
      break;
    default:
      ports0_fatal("%s %s\n%s unknown error number: %d\n",
		  achMessHead, message, achMessHead, errno );
      break;
    }
    _p0_imprison_thread( "Error code detected\n" );
/*    _p0_exit_process( 1 ); */
  }
}

void ports0_thread_prefork(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
}

void ports0_thread_postfork(void)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}


/*
 * Function versions of a bunch of the macros
 */
#undef ports0_thread_yield
void ports0_thread_yield(void)
{
    ports0_macro_thread_yield();
}

#undef ports0_thread_equal
int ports0_thread_equal(ports0_thread_t t1, ports0_thread_t t2)
{
    return(ports0_macro_thread_equal(t1, t2));
}

#undef ports0_thread_setspecific
int ports0_thread_setspecific(ports0_thread_key_t key,
			      void *value)
{
    return(ports0_macro_thread_setspecific(key, value));
}

#undef ports0_thread_getspecific
void *ports0_thread_getspecific(ports0_thread_key_t key)
{
    return(ports0_macro_thread_getspecific(key));
}

#undef ports0_thread_self
ports0_thread_t ports0_thread_self(void)
{
    return(ports0_macro_thread_self());
}

#undef ports0_thread_key_create
int ports0_thread_key_create(ports0_thread_key_t *key,
			     ports0_thread_key_destructor_func_t func)
{
    return(ports0_macro_thread_key_create(key, func));
}

#undef ports0_threadattr_init
int ports0_threadattr_init(ports0_threadattr_t *attr)
{
    return(ports0_macro_threadattr_init(attr));
}

#undef ports0_threadattr_destroy
int ports0_threadattr_destroy(ports0_threadattr_t *attr)
{
    return(ports0_macro_threadattr_destroy(attr));
}

#undef ports0_threadattr_setstacksize
int ports0_threadattr_setstacksize(ports0_threadattr_t *attr,
				   size_t stacksize)
{
    return(ports0_macro_threadattr_setstacksize(attr, stacksize));
}

#undef ports0_threadattr_getstacksize
int ports0_threadattr_getstacksize(ports0_threadattr_t *attr,
				   size_t *stacksize)
{
    return(ports0_macro_threadattr_getstacksize(attr, stacksize));
}


#undef ports0_i_am_only_thread
ports0_bool_t ports0_i_am_only_thread(void)
{
    return (ports0_macro_i_am_only_thread());
}

#undef ports0_mutex_init
int ports0_mutex_init(ports0_mutex_t *mutex,
		      ports0_mutexattr_t *attr)
{
    int rc;
    _P0_INIT_START_MAGIC_COOKIE(mutex);
    rc = ports0_macro_mutex_init(mutex, attr);
    _p0_test_rc(rc, "PORTS0: allocate lock failed\n");
    _P0_INIT_END_MAGIC_COOKIE(mutex);
    return (rc);
}

#undef ports0_mutex_destroy
int ports0_mutex_destroy(ports0_mutex_t *mutex)
{
    int rc;
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "destroy");
    rc = ports0_macro_mutex_destroy(mutex);
    _p0_test_rc(rc, "PORTS0: free lock failed\n");
    return (rc);
}

#undef ports0_cond_init
int ports0_cond_init(ports0_cond_t *cond, ports0_condattr_t *attr)
{
    int rc;
    _P0_INIT_START_MAGIC_COOKIE(cond);
    rc = ports0_macro_cond_init(cond, attr);
    _p0_test_rc(rc, "PORTS0: allocate condition variable failed\n");
    _P0_INIT_END_MAGIC_COOKIE(cond);
    return (rc);
}

#undef ports0_cond_destroy
int ports0_cond_destroy(ports0_cond_t *cond)
{
    int rc;
    PORTS0_INTERROGATE(cond, _P0_COND_T, "destroy");
    rc = ports0_macro_cond_destroy(cond);
    _p0_test_rc(rc, "PORTS0: free condition variable failed\n");
    return (rc);
}

#undef ports0_mutex_lock
int ports0_mutex_lock(ports0_mutex_t *mutex)
{
    int rc;
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "lock1");
    rc = ports0_macro_mutex_lock(mutex);
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "lock2");
    _p0_test_rc(rc, "PORTS0: mutex lock failed\n");
    return (rc);
}

#undef ports0_mutex_trylock
int ports0_mutex_trylock(ports0_mutex_t *mutex)
{
    int rc;
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "trylock1");
    rc = ports0_macro_mutex_trylock(mutex);
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "trylock2");
    _p0_test_rc(rc == EBUSY ? 0 : rc, "PORTS0: mutex trylock failed\n");
    return (rc);
}

#undef ports0_mutex_unlock
int ports0_mutex_unlock(ports0_mutex_t *mutex)
{
    int rc;
    PORTS0_INTERROGATE(mutex, _P0_MUTEX_T, "unlock1");
    rc = ports0_macro_mutex_unlock(mutex);
    _p0_test_rc(rc, "PORTS0: mutex unlock failed\n");
    return (rc);
}

#undef ports0_cond_wait
int ports0_cond_wait(ports0_cond_t *cond, ports0_mutex_t *mutex)
{
    int rc;
    PORTS0_INTERROGATE(cond, _P0_COND_T, "wait1");
    rc = ports0_macro_cond_wait(cond, mutex);
    PORTS0_INTERROGATE(cond, _P0_COND_T, "wait2");
    _p0_test_rc(rc, "PORTS0: condition variable wait failed\n");
    return (rc);
}

#undef ports0_cond_signal
int ports0_cond_signal(ports0_cond_t *cond)
{
    int rc;
    PORTS0_INTERROGATE(cond, _P0_COND_T, "signal1");
    rc = ports0_macro_cond_signal(cond);
    _p0_test_rc(rc, "PORTS0: condition variable signal failed\n");
    return (rc);
}

#undef ports0_cond_broadcast
int ports0_cond_broadcast(ports0_cond_t *cond)
{
    int rc;
    PORTS0_INTERROGATE(cond, _P0_COND_T, "broadcast1");
    rc = ports0_macro_cond_broadcast(cond);
    _p0_test_rc(rc, "PORTS0: condition variable broadcast failed\n");
    return (rc);
}


