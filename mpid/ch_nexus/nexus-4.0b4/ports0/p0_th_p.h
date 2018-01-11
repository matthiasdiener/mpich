/*
 * p0_th_p.h
 *
 * General purpose pthreads module.  Supports:
 *	IBM DCE threads
 *	HP DCE threads
 *	Florida State U. pthreads under SunOS 4.1.3
 *	POSIX threads draft 8 (if such a thing really exists in practice)
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_p.h,v 1.17 1997/01/21 22:57:14 tuecke Exp $"
 */

/*
 * #defines
 * USE_MACROS      -- Allow Ports0 functions to be macros.  If this
 *				is not defined, then all Ports0 functions
 *				must be real functions.
 */

#ifdef HAVE_PTHREAD_DRAFT_6
/* This could be possibly only for FSU pthreads??? */
/*
#define IO
*/
#endif

#ifdef TARGET_ARCH_AIX
/* Only add the 'extern "C" {' for AIX since doing so for some other platforms
 * (e.g. SunOS 4.1.3 using FSU pthreads and G++) can cause problems.  For
 * FSU pthreads it is necessary to add the 'extern "C" {' to right places
 * within the pthread.h file itself (see the FSU pthreads modified for CC++).
 */
EXTERN_C_BEGIN
#include <pthread.h>
EXTERN_C_END
#else  /* TARGET_ARCH_AIX */
#include <pthread.h>
#endif /* TARGET_ARCH_AIX */

typedef pthread_t		ports0_thread_t;
typedef pthread_attr_t		ports0_threadattr_t;
typedef pthread_mutexattr_t	ports0_mutexattr_t;
typedef pthread_condattr_t	ports0_condattr_t;
typedef pthread_key_t		ports0_thread_key_t;
typedef pthread_once_t		ports0_thread_once_t;

#ifdef PORTS0_ARCH_MPINX
/* Fix up MPI/Nexus problems */
#undef errno
#undef PRI_RR_MIN
#define PRI_RR_MIN PRI_FIFO_MIN
#undef PRI_RR_MAX
#define PRI_RR_MAX PRI_FIFO_MAX
#define MUTEX_FAST_NP MUTEX_FAST
#define MUTEX_RECURSIVE_NP MUTEX_RECURSIVE
#define MUTEX_NONRECURSIVE_NP MUTEX_NONRECURSIVE
#endif

#ifdef HAVE_PTHREAD_DRAFT_4
#define PORTS0_THREAD_ONCE_INIT	pthread_once_init
#else
#define PORTS0_THREAD_ONCE_INIT	PTHREAD_ONCE_INIT
#endif

typedef struct _ports0_mutex_t
{
    _P0_START_MAGIC_COOKIE
    pthread_mutex_t mutex;
    _P0_END_MAGIC_COOKIE
} ports0_mutex_t;

typedef struct _ports0_cond_t
{
    _P0_START_MAGIC_COOKIE
    pthread_cond_t cond;
    _P0_END_MAGIC_COOKIE
} ports0_cond_t;


#ifdef HAVE_PTHREAD_DRAFT_4
/* DCE threads missed an underscore */
#define pthread_key_create pthread_keycreate
#endif

#if defined(HAVE_PTHREAD_DRAFT_4) || defined(HAVE_PTHREAD_DRAFT_6)
/* These do not have pthread_key_delete */
#define pthread_key_delete(key) 0
#endif

#if defined(HAVE_PTHREAD_DRAFT_4) || defined(HAVE_PTHREAD_DRAFT_6)
/* Later pthreads return error codes directly, rather than through errno */
#define PORTS0_RETURN_USES_ERRNO
#endif

typedef void (*ports0_thread_key_destructor_func_t)(void *value);

typedef struct _ports0_global_vars_t
{
    ports0_thread_key_t		ports0_thread_t_pointer;
    ports0_threadattr_t		threadattr;
#ifdef HAVE_PTHREAD_DRAFT_4
    ports0_mutexattr_t		mutexattr;
    ports0_condattr_t		condattr;
#endif    
} ports0_global_vars_t;

PORTS0_GLOBAL ports0_global_vars_t ports0_all_global_vars;
extern void	_p0_report_bad_rc(int rc, char *message);

extern int	ports0_thread_create(ports0_thread_t *thread,
				     ports0_threadattr_t *attr,
				     ports0_thread_func_t func,
				     void *user_arg);
extern void	ports0_thread_exit(void *status);

#define ports0_macro_i_am_only_thread() PORTS0_FALSE


#ifdef PORTS0_RETURN_USES_ERRNO

/*
 * These macros are for draft 6 and DCE threads, which
 * use errno to return error values.
 */
#define _P0_THREAD_GETSPECIFIC
extern void *   _p0_thread_getspecific(ports0_thread_key_t key);

#ifndef HAVE_PTHREAD_DRAFT_4
#define ports0_macro_threadattr_init(attr) \
    (pthread_attr_init(attr) ? errno : 0)
#define ports0_macro_threadattr_destroy(attr) \
    (pthread_attr_destroy(attr) ? errno : 0)
#define ports0_macro_threadattr_setstacksize(attr, stacksize) \
    (pthread_attr_setstacksize(attr, stacksize) ? errno : 0)
#define ports0_macro_threadattr_getstacksize(attr, stacksize) \
    (pthread_attr_getstacksize(attr, stacksize) ? errno : 0)
#else
#define ports0_macro_threadattr_init(attr) \
    (pthread_attr_create(attr) ? errno : 0)
#define ports0_macro_threadattr_destroy(attr) \
    (pthread_attr_delete(attr) ? errno : 0)
#define ports0_macro_threadattr_setstacksize(attr, stacksize) \
    (pthread_attr_setstacksize(attr, stacksize) ? errno : 0)
#define ports0_macro_threadattr_getstacksize(attr, stacksize) \
    ((*(stacksize) = pthread_attr_getstacksize(*(attr))) ? 0 : 999)
#endif /* HAVE_PTHREAD_DRAFT_4 */

#define ports0_macro_thread_key_create(key, func) \
    (pthread_key_create(key, func) ? errno : 0)
#define ports0_macro_thread_key_delete(key) \
    (pthread_key_delete(key) ? errno : 0)
#define ports0_macro_thread_setspecific(key, value) \
    (pthread_setspecific(key, value) ? errno : 0)
#define ports0_macro_thread_getspecific(key) \
    _p0_thread_getspecific(key)

#define ports0_macro_thread_self() \
    pthread_self()
#define ports0_macro_thread_equal(t1, t2) \
    pthread_equal(t1, t2)
#define ports0_macro_thread_once(once_control, init_routine) \
    (pthread_once(once_control, init_routine) ? errno : 0)
#ifdef HAVE_PTHREAD_DRAFT_6
#define ports0_macro_thread_yield() \
    pthread_yield(NULL)
#else
#define ports0_macro_thread_yield() \
    pthread_yield()
#endif

#ifdef HAVE_PTHREAD_DRAFT_4
#define ports0_macro_mutexattr_init(attr) \
    (pthread_mutexattr_create(attr) ? errno : 0)
#define ports0_macro_mutex_init(mut, attr) \
    (pthread_mutex_init(&((mut)->mutex), \
	(attr ? *(attr) : ports0_all_global_vars.mutexattr)) \
    ? errno : 0)
#define ports0_macro_mutexattr_destroy(attr) \
    (pthread_mutexattr_delete(attr) ? errno : 0)
#else /* HAVE_PTHREAD_DRAFT_4 */
#define ports0_macro_mutexattr_init(attr) \
    (pthread_mutexattr_init(attr) ? errno : 0)
#define ports0_macro_mutex_init(mut, attr) \
    (pthread_mutex_init(&((mut)->mutex), attr) ? errno : 0)
#define ports0_macro_mutexattr_destroy(attr) \
    (pthread_mutexattr_destroy(attr) ? errno : 0)
#endif
#define ports0_macro_mutex_destroy(mut) \
    (pthread_mutex_destroy(&((mut)->mutex)) ? errno : 0)
#define ports0_macro_mutex_lock(mut) \
    (pthread_mutex_lock(&((mut)->mutex)) ? errno : 0)
#ifndef HAVE_PTHREAD_DRAFT_6
#define ports0_macro_mutex_trylock(mut) \
    (pthread_mutex_trylock(&((mut)->mutex)) ? 0 : EBUSY)
#else
#define ports0_macro_mutex_trylock(mut) \
    (pthread_mutex_trylock(&((mut)->mutex)) ? errno : 0)
#endif
#define ports0_macro_mutex_unlock(mut) \
    (pthread_mutex_unlock(&((mut)->mutex)) ? errno : 0)

#ifdef HAVE_PTHREAD_DRAFT_4
#define ports0_macro_condattr_init(attr) \
    (pthread_condattr_create(attr) ? errno : 0)
#define ports0_macro_condattr_destroy(attr) \
    (pthread_condattr_delete(attr) ? errno : 0)
#define ports0_macro_cond_init(cv, attr) \
    (pthread_cond_init(&((cv)->cond), \
	(attr ? *(attr) : ports0_all_global_vars.condattr)) \
    ? errno : 0)
#else /* HAVE_PTHREAD_DRAFT_4 */
#define ports0_macro_condattr_init(attr) \
    (pthread_condattr_init(attr) ? errno : 0)
#define ports0_macro_condattr_destroy(attr) \
    (pthread_condattr_destroy(attr) ? errno : 0)
#define ports0_macro_cond_init(cv, attr) \
    (pthread_cond_init(&((cv)->cond), attr) ? errno : 0)
#endif
#define ports0_macro_cond_destroy(cv) \
    (pthread_cond_destroy(&((cv)->cond)) ? errno : 0)
#define ports0_macro_cond_wait(cv, mut) \
    (pthread_cond_wait(&((cv)->cond), &((mut)->mutex)) ? errno : 0)
#define ports0_macro_cond_signal(cv) \
    (pthread_cond_signal(&((cv)->cond)) ? errno : 0)
#define ports0_macro_cond_broadcast(cv) \
    (pthread_cond_broadcast(&((cv)->cond)) ? errno : 0)

#else /* PORTS0_RETURN_USES_ERRNO */

/*
 * These macros are for draft 8 and draft 10 pthreads, which return values
 * directly rather than using errno.
 */
#define ports0_macro_threadattr_init(attr) \
    pthread_attr_init(attr) 
#define ports0_macro_threadattr_destroy(attr) \
    pthread_attr_destroy(attr)
#define ports0_macro_threadattr_setstacksize(attr, stacksize) \
    pthread_attr_setstacksize(attr, stacksize)
#define ports0_macro_threadattr_getstacksize(attr, stacksize) \
    pthread_attr_getstacksize(attr, stacksize)

#define ports0_macro_thread_key_create(key, func) \
    pthread_key_create(key, func)
#define ports0_macro_thread_key_delete(key) \
    pthread_key_delete(key)
#define ports0_macro_thread_setspecific(key, value) \
    pthread_setspecific(key, value)
#define ports0_macro_thread_getspecific(key) \
    pthread_getspecific(key)
#define ports0_macro_thread_self() \
    pthread_self()
#define ports0_macro_thread_equal(t1, t2) \
    pthread_equal(t1, t2)
#define ports0_macro_thread_once(once_control, init_routine) \
    pthread_once(once_control, init_routine)
#ifdef HAVE_PTHREAD_DRAFT_8
#define ports0_macro_thread_yield() \
    pthread_yield()
#else
#define ports0_macro_thread_yield() \
    sched_yield()
#endif

#define ports0_macro_mutexattr_init(attr) \
    pthread_mutexattr_init(attr)
#define ports0_macro_mutexattr_destroy(attr) \
    pthread_mutexattr_destroy(attr)
#define ports0_macro_mutex_init(mut, attr) \
    pthread_mutex_init(&((mut)->mutex), attr)
#define ports0_macro_mutex_destroy(mut) \
    pthread_mutex_destroy(&((mut)->mutex))
#define ports0_macro_mutex_lock(mut) \
    pthread_mutex_lock(&((mut)->mutex))
#define ports0_macro_mutex_trylock(mut) \
    pthread_mutex_trylock(&((mut)->mutex))
#define ports0_macro_mutex_unlock(mut) \
    pthread_mutex_unlock(&((mut)->mutex))

#define ports0_macro_condattr_init(attr) \
    pthread_condattr_init(attr)
#define ports0_macro_condattr_destroy(attr) \
    pthread_condattr_destroy(attr)
#define ports0_macro_cond_init(cv, attr) \
    pthread_cond_init(&((cv)->cond), attr)
#define ports0_macro_cond_destroy(cv) \
    pthread_cond_destroy(&((cv)->cond))
#define ports0_macro_cond_wait(cv, mut) \
    pthread_cond_wait(&((cv)->cond), &((mut)->mutex))
#define ports0_macro_cond_signal(cv) \
    pthread_cond_signal(&((cv)->cond))
#define ports0_macro_cond_broadcast(cv) \
    pthread_cond_broadcast(&((cv)->cond))

#endif /* PORTS0_RETURN_USES_ERRNO */

     
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

#else  /* USE_MACROS */

extern int		ports0_threadattr_init(ports0_threadattr_t *attr);
extern int		ports0_threadattr_destroy(ports0_threadattr_t *attr);
extern int		ports0_threadattr_setstacksize(
						   ports0_threadattr_t *attr,
						   size_t stacksize);
extern int		ports0_threadattr_getstacksize(
						   ports0_threadattr_t *attr,
						   size_t *stacksize);

extern int		ports0_thread_key_create(ports0_thread_key_t *key,
				   ports0_thread_key_destructor_func_t destructor_func);
extern int		ports0_thread_key_delete(ports0_thread_key_t key);
extern int		ports0_thread_setspecific(ports0_thread_key_t key,
						  void *value);
extern void *		ports0_thread_getspecific(ports0_thread_key_t key);

extern ports0_thread_t	ports0_thread_self(void);
extern int		ports0_thread_equal(ports0_thread_t t1,
					    ports0_thread_t t2);
extern int		ports0_thread_once(ports0_thread_once_t *once_control,
					   void (*init_routine)(void));
extern void		ports0_thread_yield(void);
extern ports0_bool_t    ports0_i_am_only_thread(void);

extern int		ports0_mutexattr_init(ports0_mutexattr_t *attr);
extern int		ports0_mutexattr_destroy(ports0_mutexattr_t *attr);
extern int		ports0_mutex_init(ports0_mutex_t *mutex,
					  ports0_mutexattr_t *attr);
extern int		ports0_mutex_destroy(ports0_mutex_t *mutex);
extern int		ports0_mutex_lock(ports0_mutex_t *mutex);
extern int		ports0_mutex_trylock(ports0_mutex_t *mutex);
extern int		ports0_mutex_unlock(ports0_mutex_t *mutex);

extern int		ports0_condattr_init (ports0_condattr_t *attr);
extern int		ports0_condattr_destroy (ports0_condattr_t *attr);
extern int		ports0_cond_init(ports0_cond_t *cond,
					 ports0_condattr_t *attr);
extern int		ports0_cond_destroy(ports0_cond_t *cond);
extern int		ports0_cond_wait(ports0_cond_t *cond,
					 ports0_mutex_t *mutex);
extern int		ports0_cond_signal(ports0_cond_t *cond);
extern int		ports0_cond_broadcast(ports0_cond_t *cond);

#endif /* USE_MACROS */
