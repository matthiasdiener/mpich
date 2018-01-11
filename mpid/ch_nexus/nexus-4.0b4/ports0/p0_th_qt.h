/*
 * th_qt.h
 *
 * author: Craig Lee at Aerospace.
 *
 * External header for QuickThreads running on the
 * whatever QuickThreads runs on
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_qt.h,v 1.25 1996/02/28 20:43:19 patton Exp $"
 */

/* qt.h is C++ hostile */
#define new new_th
#include <qt.h>
#undef new

EXTERN_C_BEGIN

#if defined(HAVE_SYS_LWP_H)
#include <sys/lwp.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_ULOCKS_H
#include <ulocks.h>
#endif

#ifndef PAGESIZE
#if defined(TARGET_ARCH_OSF1) || defined(TARGET_ARCH_SGI) \
    || defined(TARGET_ARCH_SOLARIS)
#define PAGESIZE (sysconf(_SC_PAGESIZE))
#elif defined(TARGET_ARCH_HPUX)
#include <signal.h> 
#define PAGESIZE (sysconf(_SC_PAGE_SIZE))
#endif
#endif

#define DEFAULT_STACK_PAGES 10
#define PORTS0_STACK_MIN  DEFAULT_STACK_PAGES*PAGESIZE
#define IDLE_THREAD_STACKSIZE 10*PAGESIZE

#define OK_TO_RETURN_NULL 0
#define MUST_SCHED_SOMETHING 1

/* Time quanta in usec: clk_res <= TIME_SLICE < 10^6 */
#define TIME_SLICE 10000

#define WAKEUP_SIGNAL SIGUSR1

#ifdef TARGET_ARCH_SGI
#define StartTimer() \
{ \
    struct sigaction sigact; \
 \
    sigaction(SIGVTALRM, NULL, &sigact); \
    sigact.sa_handler = time_slicer; \
    sigact.sa_flags = SA_NODEFER; \
    sigaction(SIGVTALRM, &sigact, NULL); \
    time_slice_val.it_interval.tv_sec = \
	time_slice_val.it_value.tv_sec = 0; \
    time_slice_val.it_interval.tv_usec = \
	time_slice_val.it_value.tv_usec = TIME_SLICE; \
    if (setitimer(ITIMER_VIRTUAL, &time_slice_val, NULL)) \
    { \
	_p0_report_bad_rc(1, "setitimer() failed\n"); \
    } \
} 
#else
#define StartTimer() \
{ \
    struct sigaction sigact; \
 \
    sigaction(SIGVTALRM, NULL, &sigact); \
    sigact.sa_handler = time_slicer; \
    sigaction(SIGVTALRM, &sigact, NULL); \
    time_slice_val.it_interval.tv_sec = \
	time_slice_val.it_value.tv_sec = 0; \
    time_slice_val.it_interval.tv_usec = \
	time_slice_val.it_value.tv_usec = TIME_SLICE; \
    if (setitimer(ITIMER_VIRTUAL, &time_slice_val, NULL)) \
    { \
	_p0_report_bad_rc(1, "setitimer() failed\n"); \
    } \
}
#endif


#define START_PREEMPTION \
{ \
    if (pP->timer_hit && preemptive_threads) \
    { \
	StartTimer(); \
    } \
    pP->timer_hit = 0; \
    pP->dont_preempt = 0; \
}
#define DONT_PREEMPT pP->dont_preempt = 1
#define CHECK_SLICE { \
		      if  ( pP->timer_hit )  { \
		        check_slice_hits++; \
				ports0_thread_yield(); \
		      } \
		      pP->dont_preempt = 0; \
		    }

#define PORTS0_QT
#define _P0_THR_SUCCESS 0

#define _P0_THREAD_GRAN 256
#define _P0_USER_THREAD 0

/*
 * lock stuff
 */
#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_AXP) \
    || defined(TARGET_ARCH_HPUX) || defined(TARGET_ARCH_AIX)
#define LOCK_INIT_VAL 0
typedef  int  spin_lock_t;
void spin_lock_init( spin_lock_t *lock );
void spin_lock( spin_lock_t *lock );
int  test_and_get_spin_lock( spin_lock_t *lock );  /* not currently used */
void spin_unlock( spin_lock_t *lock );
#endif

#ifdef TARGET_ARCH_SGI
#include <abi_mutex.h>
#define LOCK_INIT_VAL UNLOCKED
typedef  abilock_t  spin_lock_t;
#define  spin_lock_init(x)  init_lock(x)
#define  spin_lock(x)  spin_lock(x)
#define  test_and_get_spin_lock(x)  (!acquire_lock(x))
#define  spin_unlock(x)  release_lock(x)
#endif

#ifdef TARGET_ARCH_SUNOS41
typedef int spin_lock_t;
#define LOCK_INIT_VAL 0
#define spin_lock_init(lp)
#define spin_lock(lp)
#define test_and_get_spin_lock(lp)
#define spin_unlock(lp)
#endif

/*
 *  malloc stuff.  In shared memory, all malloc-related functions have to
 *  be mapped to locking thread-safe versions.
 */
#if defined(TARGET_ARCH_SOLARIS) || defined(TARGET_ARCH_SGI)
  /*  In Solaris, compiling with -D _REENTRANT should make
   *  use of libmalloc reentrant and thread-safe.
   */
  /*  In IRIX, the default is the right thing.  */
#define nq_minit(X)
#define nq_malloc(X) malloc(X)
#define nq_memalign(X,Y) memalign(X,Y)
#define nq_free(X) free(X)

#else  

#define nq_minit(X)
#define nq_malloc(X) malloc(X)
#ifdef TARGET_ARCH_OSF1
#define nq_memalign(X,Y) valloc(Y)
#elif defined(TARGET_ARCH_HPUX) || defined(TARGET_ARCH_AIX)
#define nq_memalign(X,Y) mmap(NULL, (Y), \
    PROT_READ | PROT_WRITE, \
    MAP_ANONYMOUS | MAP_VARIABLE | MAP_PRIVATE, \
    -1, NULL)
#else
#define nq_memalign(X,Y) memalign(X,Y)
#endif
#define nq_free(X) free(X)

#endif  /* malloc stuff */

/*
 *  atomic increment/decrement stuff.
 */
#ifdef TARGET_ARCH_SOLARIS
typedef  int  atomic_int_t;
int atomic_inc( int *x );
int atomic_dec( int *x );
#define atom_inc(x) atomic_inc( &(x) )
#define atom_dec(x) atomic_dec( &(x) )
#endif

#ifdef TARGET_ARCH_SGI
#include <mutex.h>
typedef  unsigned long  atomic_int_t;
#define atom_inc(x) add_then_test( &(x), 1 )
#define atom_dec(x) add_then_test( &(x), ULONG_MAX )
#endif

#if !defined(TARGET_ARCH_SOLARIS) && !defined(TARGET_ARCH_SGI)
typedef  int  atomic_int_t;
#define atom_inc(x) (++(x))
#define atom_dec(x) (--(x))

#endif  /* atomic inc/dec */


typedef void * any_t;

/*
 * ports0_thread_t stuff
 */

#define PORTS0_DATAKEYS_MAX 16
typedef void (*ports0_thread_key_destructor_func_t)(void *value);
typedef int ports0_thread_key_t;

typedef struct ports0_thread_struct {
    int tid;
    int stack_size;
    char *stack_base;
    qt_t *sp;
    void *key_data[PORTS0_DATAKEYS_MAX];
    struct ports0_thread_struct *next;
}  ports0_thread_s;

typedef  ports0_thread_s * ports0_thread_t;

typedef struct ports0_threadattr_s {
  int stack_addr;
  int stack_size;
}  ports0_threadattr_t;


typedef struct thread_queue {
  spin_lock_t lock;
  ports0_thread_t qhead;
  ports0_thread_t qtail;
} ThreadQ;

typedef struct _p0_idle
{
    pid_t pid;
    struct _p0_idle *next;
} _p0_idle_t;

typedef struct idle_queue {
    spin_lock_t lock;
    _p0_idle_t *qhead;
    _p0_idle_t *qtail;
} _p0_idle_queue_t;


/*
 * ports0_mutex_t stuff
 */

#define  MUTEX_UNLOCKED  0
#define  MUTEX_LOCKED    1

typedef struct ports0_mutex_s {
  int locked;
  ThreadQ waiters;
}  p0_mutex;

typedef struct ports0_mutexattr_s {
  int dummy;
}  ports0_mutexattr_t;

/*
 * ports0_mutex_t
 */
typedef struct _ports0_mutex_t {
  _P0_START_MAGIC_COOKIE
  p0_mutex mutex;
  _P0_END_MAGIC_COOKIE
} ports0_mutex_t;

/*
 * ports0_cond_t stuff
 */

typedef struct ports0_cond_s {
  ports0_mutex_t *mutex;
  ThreadQ waiters;
}  p0_cond;

typedef struct ports0_condattr_s {
  int dummy;
}  ports0_condattr_t;

 
/*
 * ports0_cond_t
 */
typedef struct _ports0_cond_t {
  _P0_START_MAGIC_COOKIE
  p0_cond cond;
  _P0_END_MAGIC_COOKIE
} ports0_cond_t;

typedef struct _ports0_global_vars {
  ports0_thread_key_t        ports0_thread_t_pointer;
} ports0_global_vars;
 
PORTS0_GLOBAL ports0_global_vars ports0_all_global_vars;

/*
 * Thread Kernel stuff
 */

#define  MAXPROCS 128

  /* Global information */
typedef struct thread_kernel_s {
  atomic_int_t IdleProcs;
  int nProcs;
  atomic_int_t activeIdle;
  ports0_thread_t handlerThread;
  ports0_bool_t shutdown;
  int idle_vec[MAXPROCS];
  atomic_int_t ActiveThreads;
  atomic_int_t nThreads;
  ThreadQ *firstQ, *lastQ;
  ThreadQ readyList[MAXPROCS];
  int free_count;
  ThreadQ freeQ;
  _p0_idle_queue_t idleQ;
}  thread_kernel_t;


  /* Process private information */
  /* NOTE: On the SGI, this struct must not be larger than usr_prda, see <sys/prctl.h> */
typedef struct private_proc {
  int myId;
  char *signal_stack;    /* not currently used */
  int timer_hit;
  int dont_preempt;
  ports0_thread_t activeThread;
  ports0_thread_t idleThread;
  ports0_thread_t handlerThread;
  ThreadQ *lastLook;
  ThreadQ *putQ;
  /* A per process free list could be implemented here. */
}  PrivateProcessor, *PrivateProcessor_p;


#define PORTS0_THREAD_ONCE_INIT {LOCK_INIT_VAL,1}
typedef struct ports0_thread_once_s {
  spin_lock_t lock;
  int init;
}  ports0_thread_once_t;


#define NextQ(q) ((q == TKp->lastQ) ? TKp->firstQ : (q + 1))

#define ThreadQInit(t) {\
	spin_lock_init( &((t)->lock) ); \
	(t)->qhead = (t)->qtail = NULL; \
        }

#define ThreadPut(t,q) { \
	if  ( (q)->qhead ) \
          { (q)->qtail->next = t;  (q)->qtail = t; } \
        else \
          { (q)->qhead = (q)->qtail = t; } \
        }

#define ThreadCat(hd,tl,q) { \
	if  ( q->qhead )  { q->qtail->next = hd; } \
        else              { q->qhead = hd; } \
        q->qtail = tl; \
        }

#define ThreadGet(t,q) { \
        (t) = (q)->qhead; \
	(q)->qhead = (t)->next; \
        (t)->next = NULL; \
        }

#define ReadyQPut(t) { \
	spin_lock( &(pP->putQ->lock) ); \
	ThreadPut( t, (pP->putQ) ); \
	spin_unlock( &(pP->putQ->lock) ); \
	WakeUpIdleThread(); \
	}

#define ReadyQCat(hd,tl) { \
	spin_lock( &(pP->putQ->lock) ); \
	ThreadCat( hd, tl, (pP->putQ) ); \
	spin_unlock( &(pP->putQ->lock) ); \
	}

#define FreeQPut(t) { \
	spin_lock( &(TKp->freeQ.lock) ); \
	ThreadPut( t, (&(TKp->freeQ)) ); \
	(TKp->free_count)++; \
	spin_unlock( &(TKp->freeQ.lock) ); \
	}

#define FreeQGet(t,ss) { \
	spin_lock( &(TKp->freeQ.lock) ); \
	if  ( ((t) = TKp->freeQ.qhead) ) { \
          if  ( (ss) <= (t)->stack_size )  { \
	    TKp->freeQ.qhead = (t)->next; \
	    (TKp->free_count)--; \
          }  else \
            { (t) = NULL; } \
        } \
	spin_unlock( &(TKp->freeQ.lock) ); \
	}

#define IdleQPut() \
{ \
    _p0_idle_t temp; \
 \
    spin_lock(&(TKp->idleQ.lock)); \
    temp.pid = getpid(); \
    temp.next = NULL; \
    ThreadPut(&temp, &(TKp->idleQ)); \
    spin_unlock(&(Tkp->idleQ.lock)); \
}

#define IdleQGet(i) \
{ \
    spin_lock(&(TKp->idleQ.lock)); \
    ThreadGet((i), &(TKp->idleQ)); \
    spin_unlock(&(TKp->idleQ.lock)); \
}

#define WakeUpIdleThread() \
{ \
    _p0_idle_t *temp; \
 \
    if (TKp->activeIdle) \
    { \
	IdleQGet(temp); \
	kill(temp->pid, WAKEUP_SIGNAL); \
    } \
}

/*  When running multiprocessor, the queue lock pointer has to be passed
    to csw_helper to unlock the queue after the calling thread has been
    safely put away.  When running uniprocessor, this is an extraneous
    operation.
*/
#define ReadyQPut_switch(t,n) { \
	spin_lock( &(pP->putQ->lock) ); \
	ThreadPut( t, (pP->putQ) ); \
	QT_BLOCK( csw_helper, (t), &(pP->putQ->lock), (n)->sp ); \
	}

#define FreeQPut_switch(t,n) { \
	spin_lock( &(TKp->freeQ.lock) ); \
	ThreadPut( t, (&(TKp->freeQ)) ); \
	(TKp->free_count)++; \
	QT_ABORT( csw_helper, (t), &(TKp->freeQ.lock), (n)->sp ); \
	}


extern void _p0_more_thread_mem( void );
extern void _p0_init_thread_package( void );
extern void *_p0_thread_starter( void * );
extern void _p0_report_bad_rc( int, char * );

extern void _p0_thread_process_arguments_init( void );
extern int _p0_thread_process_arguments( int, int );
extern void _p0_thread_usage_message( void );
extern int _p0_thread_new_process_params( char *, int );
extern void _p0_thread_init( int *argc, char **argv[] );

extern int ports0_threadattr_init( ports0_threadattr_t *attrp );
extern int ports0_threadattr_setstacksize( ports0_threadattr_t *, size_t );
extern int ports0_threadattr_getstacksize( ports0_threadattr_t *, size_t * );
extern int get_activethreads( void );
extern int get_mytid( void );

#define _p0_samethread( t1, t2 ) ( ports0_thread_equal( t1, t2 ) )

#define _p0_current_thread( t1 ) ((t1)=ports0_thread_self())

#define _p0_test_rc( a, b ) \
  do {                                                   \
    if( a != _P0_THR_SUCCESS ) _p0_report_bad_rc( a, b ); \
  } while(0)


extern int   ports0_thread_create(ports0_thread_t *thread,
				   ports0_threadattr_t *attr,
				   ports0_thread_func_t func,
				   void *user_arg);
extern void  ports0_thread_exit(void *status);

extern int  ports0_thread_key_create(ports0_thread_key_t *key,
				    ports0_thread_key_destructor_func_t func);
extern int  ports0_thread_setspecific(ports0_thread_key_t key,
				     void *value);
extern void *_p0_thread_getspecific(ports0_thread_key_t key);

extern ports0_thread_t _p0_thread_self(void);
extern void _p0_thread_yield(int return_null);
extern ports0_bool_t _p0_i_am_only_thread(void);

extern int _p0_mutex_init( ports0_mutex_t *, ports0_mutexattr_t * );
extern int _p0_mutex_destroy( ports0_mutex_t * );
extern int _p0_cond_init( ports0_cond_t *, ports0_condattr_t *);
extern int _p0_cond_destroy( ports0_cond_t *);
extern int _p0_mutex_lock( ports0_mutex_t * );
extern int _p0_mutex_unlock( ports0_mutex_t * );
extern int _p0_cond_wait( ports0_cond_t *, ports0_mutex_t * );
extern int _p0_cond_signal( ports0_cond_t * );
extern int _p0_cond_broadcast( ports0_cond_t * );


#ifdef PORTS0_SKIP_USE_MACROS
#undef USE_MACROS
#endif


#define ports0_macro_threadattr_init(attr) \
	_p0_threadattr_init(attr)
/*
   todo: doublecheck this implementation of threadattr_destroy for
   correctness.
*/
#define ports0_macro_threadattr_destroy(attr) _P0_THR_SUCCESS 
#define ports0_macro_threadattr_setstacksize(attr, stacksize) \
	_p0_threadattr_setstacksize(attr, stacksize)
#define ports0_macro_threadattr_getstacksize(attr, stacksize) \
	_p0_threadattr_getstacksize(attr, stacksize)
#define ports0_macro_thread_key_create(key, func) \
	_p0_thread_key_create(key, func)
#define ports0_macro_thread_key_delete(key) 0 /* successful return */
#define ports0_macro_thread_setspecific(key, value) \
	_p0_thread_setspecific(key, value)
#define ports0_macro_thread_getspecific(key) \
	_p0_thread_getspecific(key)
#define ports0_macro_thread_self() \
	_p0_thread_self()
#define ports0_macro_thread_equal(t1,t2) ((t1) == (t2))
#define ports0_macro_thread_once(once_control, init_routine) \
	_p0_thread_once(once_control, init_routine)
#define ports0_macro_thread_yield() \
	_p0_thread_yield(OK_TO_RETURN_NULL)
#define ports0_macro_i_am_only_thread() \
	_p0_i_am_only_thread()

#define ports0_macro_mutexattr_init(attr) 0 /* successful return */
#define ports0_macro_mutexattr_destroy(attr) 0 /* successful return */
#define ports0_macro_mutex_init( mut, attr ) \
	_p0_mutex_init( mut, attr )
#define ports0_macro_mutex_destroy( mut ) \
	_p0_mutex_destroy( mut )
#define ports0_macro_mutex_lock( mut ) \
	_p0_mutex_lock( mut )
#define ports0_macro_mutex_trylock(mut) \
    _p0_mutex_trylock(mut)
#define ports0_macro_mutex_unlock( mut ) \
	_p0_mutex_unlock( mut )

#define ports0_macro_condattr_init(attr) 0 /* successful return */
#define ports0_macro_condattr_destroy(attr) 0 /* successful return */
#define ports0_macro_cond_init( cv, attr ) \
	_p0_cond_init( cv, attr )
#define ports0_macro_cond_destroy( cv ) \
	_p0_cond_destroy( cv )
#define ports0_macro_cond_wait( cv, mut ) \
	_p0_cond_wait( cv, mut )
#define ports0_macro_cond_signal( cv ) \
	_p0_cond_signal( cv )
#define ports0_macro_cond_broadcast( cv ) \
	_p0_cond_broadcast( cv )

#ifdef USE_MACROS
#define ports0_threadattr_init(attr) \
	ports0_macro_threadattr_init(attr)
#define ports0_threadattr_destroy(attr) \
	ports0_macro_threadattr_destroy(attr)
#define ports0_threadattr_setstacksize(attr, stacksize) \
	ports0_macro_threadattr_setstacksize(attr, stacksize)
#define ports0_threadattr_getstacksize(attr, stacksize) \
	ports0_macro_threadattr_getstacksize(attr, stacksize)
#define ports0_thread_key_create(key, func) \
	ports0_macro_thread_key_create(key, func)
#define ports0_thread_key_destroy(key) \
	ports0_macro_thread_key_destroy(key)
#define ports0_thread_setspecific(key, value) \
	ports0_macro_thread_setspecific(key, value)
#define ports0_thread_getspecific(key) \
	ports0_macro_thread_getspecific(key)
#define ports0_thread_self() \
	ports0_macro_thread_self()
#define ports0_thread_equal(t1, t2) \
	ports0_macro_thread_equal(t1, t2)
#define ports0_thread_once(once_control, init_routine) \
	ports0_macro_thread_once(once_control, init_routine)
#define ports0_thread_yield() \
	ports0_macro_thread_yield()
#define ports0_i_am_only_thread() \
	ports0_macro_i_am_only_thread()

#define ports0_mutexattr_init(attr) \
	ports0_macro_mutexattr_init(attr)
#define ports0_mutexattr_destroy(attr) \
	ports0_macro_mutexattr_destroy(attr)
#define ports0_mutex_init(mut, attr) \
	ports0_macro_mutex_init(mut, attr)
#define ports0_mutex_destroy(mut) \
	ports0_macro_mutex_destroy(mut)
#define ports0_mutex_lock(mut) \
	ports0_macro_mutex_lock(mut)
#define ports0_mutex_trylock(mut) \
	ports0_macro_mutex_trylock(mut)
#define ports0_mutex_unlock(mut) \
	ports0_macro_mutex_unlock(mut)

#define ports0_condattr_init(attr) \
	ports0_macro_condattr_init(attr)
#define ports0_condattr_destroy(attr) \
	ports0_macro_condattr_destroy(attr)
#define ports0_cond_init(cv, attr) \
	ports0_macro_cond_init(cv, attr)
#define ports0_cond_destroy(cv) \
	ports0_macro_cond_destroy(cv)
#define ports0_cond_wait(cv, mut) \
	ports0_macro_cond_wait(cv, mut)
#define ports0_cond_signal(cv) \
	ports0_macro_cond_signal(cv)
#define ports0_cond_broadcast(cv) \
	ports0_macro_cond_broadcast(cv)

#else /* USE_MACROS */

extern int				ports0_threadattr_init(ports0_threadattr_t *attr);
extern int				ports0_threadattr_destroy(ports0_threadattr_t *attr);
extern int				ports0_threadattr_setstacksize(ports0_threadattr_t *attr, size_t stacksize);
extern int				ports0_threadattr_getstacksize(ports0_threadattr_t *attr, size_t *stacksize);

extern int				ports0_thread_key_create(ports0_thread_key_t *key, ports0_thread_key_destructor_func_t destructor_func);
extern int				ports0_thread_key_delete(ports0_thread_key_t *key);
extern int				ports0_thread_setspecific(ports0_thread_key_t key, void *value);
extern void *			ports0_thread_getspecific(ports0_thread_key_t key);

extern ports0_thread_t	ports0_thread_self(void);
extern int				ports0_thread_equal(ports0_thread_t t1, ports0_thread_t t2);
extern int				ports0_thread_once(ports0_thread_once_t *once_control, void (*init_routine)(void));
extern void				ports0_thread_yield(void);
extern ports0_bool_t    ports0_i_am_only_thread(void);

extern int				ports0_mutexattr_init(ports0_mutexattr_t *attr);
extern int				ports0_mutexattr_destroy(ports0_mutexattr_t *attr);
extern int				ports0_mutex_init(ports0_mutex_t *mutex, ports0_mutexattr_t *attr);
extern int				ports0_mutex_destroy(ports0_mutex_t *mutex);
extern int				ports0_mutex_lock(ports0_mutex_t *mutex);
extern int				ports0_mutex_trylock(ports0_mutex_t *mutex);
extern int				ports0_mutex_unlock(ports0_mutex_t *mutex);

extern int				ports0_condattr_init(ports0_condattr_t *attr);
extern int				ports0_condattr_destroy(ports0_condattr_t *attr);
extern int				ports0_cond_init(ports0_cond_t *cond, ports0_condattr_t *attr);
extern int				ports0_cond_destroy(ports0_cond_t *cond);
extern int				ports0_cond_wait(ports0_cond_t *cond, ports0_mutex_t *mutex);
extern int				ports0_cond_signal(ports0_cond_t *cond);
extern int				ports0_cond_broadcast(ports0_cond_t *cond);

#endif  /* USE_MACROS */


/* Gets ports0 wrapper thread id from pthread specific data */

typedef void *p0_any;

EXTERN_C_END
