/*
 * p0_th_c.h
 *
 * External header for a C-threads based Ports0 threads library.
 * This will be included by user code.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_c.h,v 1.11 1996/02/28 20:43:14 patton Exp $"
 */

#include <mach/cthreads.h>

typedef struct cthread		ports0_thread_t;
typedef int			ports0_threadattr_t;
typedef int			ports0_mutexattr_t;
typedef int			ports0_condattr_t;

typedef struct _ports0_mutex_t
{
    _P0_START_MAGIC_COOKIE
    struct mutex mutex;
    _P0_END_MAGIC_COOKIE
} ports0_mutex_t;

typedef struct _ports0_cond_t {
    _P0_START_MAGIC_COOKIE
    struct condition cond;
    _P0_END_MAGIC_COOKIE
} ports0_cond_t;

#define PORTS0_THREAD_ONCE_INIT 0
typedef int ports0_thread_once_t;
extern  int _p0_actual_thread_once(ports0_thread_once_t *once_control,
				   void (*init_routine)(void));

typedef int ports0_thread_key_t;
typedef void (*ports0_thread_key_destructor_func_t)(void *value);

#ifndef ENOSYS
/* On the NeXT this is only defined with _POSIX_SOURCE */
#define ENOSYS 87
#endif

/*
 * Some externs...
 */
extern int		ports0_thread_create(ports0_thread_t *thread,
					     ports0_threadattr_t *attr,
					     ports0_thread_func_t func,
					     void *user_arg);
extern void		ports0_thread_exit(void *status);

extern int		ports0_thread_key_create(ports0_thread_key_t *key,
				ports0_thread_key_destructor_func_t func);
extern int		ports0_thread_setspecific(ports0_thread_key_t key,
						  void *value);
extern void *           ports0_thread_getspecific(ports0_thread_key_t key);

#define ports0_threadattr_init(attr)	0 /* successful return */
#define ports0_threadattr_destroy(attr)	0 /* successful return */
#define ports0_threadattr_setstacksize(attr, stacksize) \
	ENOSYS /* unsuccessful return - not implemented */
#define ports0_threadattr_getstacksize(attr, stacksize) \
	ENOSYS /* unsuccessful return - not implemented */
#define ports0_thread_key_delete(key)	0 /* successful return */


/*
 * macros and prototypes for thread manipulation routines
 */
#define ports0_macro_thread_yield() \
    cthread_yield()
#define ports0_macro_thread_self() \
    (*(ports0_thread_t *)(cthread_self()))
#define ports0_macro_thread_equal(t1,t2) \
    ((int)&(t1) == (int)&(t2))
#define ports0_macro_thread_once(once_control, init_routine) \
    (*once_control ? 0 : _p0_actual_thread_once(once_control, init_routine))
#ifndef USE_MACROS
extern void		ports0_thread_yield(void);
extern ports0_thread_t	ports0_thread_self(void);
extern int		ports0_thread_equal(ports0_thread_t t1,
					   ports0_thread_t t2);
extern int		ports0_thread_once(ports0_thread_once_t *once_control,
					  void (*init_routine)(void));
extern ports0_bool_t ports0_i_am_only_thread(void);
#else  /* USE_MACROS */
#define ports0_thread_yield() \
    ports0_macro_thread_yield()
#define ports0_thread_self() \
    ports0_macro_thread_self()
#define ports0_thread_equal(t1,t2) \
    ports0_macro_thread_equal(t1,t2)
#define ports0_thread_once(once_control, init_routine) \
    ports0_macro_thread_once(once_control, init_routine)
#define ports0_i_am_only_thread() PORTS0_FALSE
#endif /* USE_MACROS */


/*
 * lock macros and prototypes
 */

#define ports0_macro_mutex_init(mut, attr) \
    (mutex_init(&((mut)->mutex)), 0)
#ifdef TARGET_ARCH_NEXTSTEP
#define ports0_macro_mutex_destroy(mutex)	0 /* successful return */
#define ports0_macro_mutex_lock(mut) \
    (mutex_try_lock(&((mut)->mutex)) ? 0 : \
     (mutex_wait_lock(&((mut)->mutex)), 0) )
#else  /* TARGET_ARCH_NEXTSTEP */
#define ports0_macro_mutex_destroy(mut)	mutex_clear(&((mut)->mutex))
#define ports0_macro_mutex_lock(mut) \
    (mutex_lock(&((mut)->mutex)), 0)
#endif /* TARGET_ARCH_NEXTSTEP */
#define ports0_macro_mutex_trylock(mut) \
    (mutex_try_lock(&((mut)->mutex)) ? 0 : EBUSY)
#define ports0_macro_mutex_unlock(mut) \
    (mutex_unlock(&((mut)->mutex)), 0)
#define ports0_mutexattr_init(attr) 0 /* successful return */
#define ports0_mutexattr_destroy(attr) 0 /* successful return */

#ifndef USE_MACROS
extern int		ports0_mutex_init(ports0_mutex_t *mutex,
					  ports0_mutexattr_t *attr);
extern int		ports0_mutex_destroy(ports0_mutex_t *mutex);
extern int		ports0_mutex_lock(ports0_mutex_t *mutex);
extern int		ports0_mutex_trylock(ports0_mutex_t *mutex);
extern int		ports0_mutex_unlock(ports0_mutex_t *mutex);
#else  /* USE_MACROS */
#define ports0_mutex_init(mutex, attr) \
    ports0_macro_mutex_init(mutex, attr)
#define ports0_mutex_destroy(mutex) \
    ports0_macro_mutex_destroy(mutex)
#define ports0_mutex_lock(mutex) \
    ports0_macro_mutex_lock(mutex)
#define ports0_mutex_trylock(mutex) \
    ports0_macro_mutex_trylock(mutex)
#define ports0_mutex_unlock(mutex) \
    ports0_macro_mutex_unlock(mutex)
#endif /* USE_MACROS */


/*
 * condition macros and prototypes
 */
#define ports0_macro_cond_wait(cv, mut) \
    (condition_wait(&((cv)->cond), &((mut)->mutex)), 0)
#define ports0_condattr_init(attr) 0 /* successful return */
#define ports0_condattr_destroy(attr) 0 /* successful return */

#ifndef USE_MACROS
extern int	ports0_cond_wait(ports0_cond_t *cond,
				 ports0_mutex_t *mutex);
#else /* USE_MACROS */
#define ports0_cond_wait(cv, mut) \
    ports0_macro_cond_wait(cv, mut)
#endif /* USE_MACROS */

#ifdef TARGET_ARCH_NEXTSTEP
/* These cannot be done within a comma operator to get the right return code */
#define ports0_macro_cond_init(cv, attr) \
    condition_init(&((cv)->cond))
#define ports0_macro_cond_destroy(cv) \
    condition_clear(&((cv)->cond))
#define ports0_macro_cond_signal(cv) \
    condition_signal(&((cv)->cond))
#define ports0_macro_cond_broadcast(cv) \
    condition_broadcast(&((cv)->cond))
#else  /* !TARGET_ARCH_NEXTSTEP */
#define ports0_macro_cond_init(cv, attr) \
    (condition_init(&((cv)->cond)), 0)
#define ports0_macro_cond_destroy(cv) \
    (condition_clear(&((cv)->cond)), 0)
#define ports0_macro_cond_signal(cv) \
    (condition_signal(&((cv)->cond)), 0)
#define ports0_macro_cond_broadcast(cv) \
    (condition_broadcast(&((cv)->cond)), 0)
#endif /* !TARGET_ARCH_NEXTSTEP */

#if !defined(USE_MACROS) || defined(TARGET_ARCH_NEXTSTEP)
extern int	ports0_cond_init(ports0_cond_t *cond, ports0_condattr_t *attr);
extern int	ports0_cond_destroy(ports0_cond_t *cond);
extern int	ports0_cond_signal(ports0_cond_t *cond);
extern int	ports0_cond_broadcast(ports0_cond_t *cond);
#else
#define ports0_cond_init(cv) \
    ports0_macro_cond_init(cv)
#define ports0_cond_destroy(cv) \
    ports0_macro_cond_destroy(cv)
#define ports0_cond_signal(cv) \
    ports0_macro_cond_signal(cv)
#define ports0_cond_broadcast(cv) \
    ports0_macro_cond_broadcast(cv)
#endif
