/*
 * p0_th_win32.h (see also p0_th_win32.c)
 *
 * External header for Win32
 *
 * #defines PORTS0_USE_MACROS   -- allow Ports0 functions to be macros
 *
 * $Id: p0_th_win32.h,v 1.1 1996/11/15 17:33:02 tuecke Exp $
 */

#undef PORTS0_USE_MACROS

#if 0 || defined(PORTS0_USE_MACROS)
#ifndef PORTS0_USE_MACROS
#define PORTS0_USE_MACROS
#endif
#endif

#define PORTS0_KEY_TABLE_SIZE 32

/* C++ - The Final Frontier */                      typedef int ports0_thread_once_t;
EXTERN_C_BEGIN

#include <windows.h>

typedef HANDLE                 _p0_mutex_t;
typedef HANDLE                 _p0_cond_t;
typedef int                  _p0_thread_key_t;
typedef LPVOID                 _p0_mutexattr_t;
typedef int                    _p0_condattr_t;

typedef struct _p0_threadattr
{
    size_t stacksize;
} _p0_threadattr_t;

typedef size_t p0_size;
typedef void *(*p0_func)(void *);
typedef void * any_t;

#define _P0_THR_SUCCESS 0

typedef int                 ports0_thread_t;
typedef _p0_thread_key_t       ports0_thread_key_t;
typedef _p0_threadattr_t       ports0_threadattr_t;
typedef int                    ports0_mutexattr_t;
typedef int                    ports0_condattr_t;


/*
 * ports0_mutex_t
 */
typedef struct _ports0_mutex_t
{
    _P0_START_MAGIC_COOKIE
    _p0_mutex_t mutex;
    _P0_END_MAGIC_COOKIE
} ports0_mutex_t;

/*
 * ports0_cond_t
 */
typedef struct _ports0_cond_t
{
    _P0_START_MAGIC_COOKIE
    HANDLE semaphore;
    HANDLE mutex;
    int numberOfWaiters;
    _p0_cond_t condition;
    _P0_END_MAGIC_COOKIE
} ports0_cond_t;

#define PORTS0_THREAD_ONCE_INIT 0
typedef int ports0_thread_once_t;

extern  int _p0_actual_thread_once(ports0_thread_once_t *once_control,
				   void (*init_routine)(void));



typedef void (*destructor_t)(void *value);

typedef struct _ports0_global_vars
{
    _p0_thread_key_t	ports0_thread_t_pointer;
    int			general_attribute;
    int			thread_flags;
    _p0_threadattr_t	thread_attr;
} ports0_global_vars;

PORTS0_GLOBAL ports0_global_vars ports0_all_global_vars;

extern void _p0_report_bad_rc( int, char * );

extern int ports0_thread_create(ports0_thread_t *thread,
			       ports0_threadattr_t *attr,
			       ports0_thread_func_t tar_func,
			       void *user_arg );
extern void ports0_thread_exit(void *status);


extern int ports0_thread_key_create(ports0_thread_key_t *key,
				   destructor_t func);

extern void *_p0_thread_getspecific(ports0_thread_key_t key);

/* GKT -> make this a function */
#define _p0_test_rc( a, b ) \
  do {                                                   \
    if( a != _P0_THR_SUCCESS && a != EINTR ) _p0_report_bad_rc( a, b ); \
  } while(0)

#define ports0_macro_i_am_only_thread() PORTS0_FALSE

#define ports0_macro_threadattr_init(attr) \
	((attr)->stacksize = ports0_all_global_vars.thread_attr.stacksize)

#define ports0_macro_threadattr_destroy(attr) 0 /* successful return */

#define ports0_macro_threadattr_setstacksize(attr, ss) \
	((attr)->stacksize = (ss), 0)

#define ports0_macro_threadattr_getstacksize(attr, ss) \
	(*(ss) = (attr)->stacksize, 0)


#define ports0_macro_thread_key_create(key, value) \
    key = TlsAlloc()

#define ports0_macro_thread_key_delete(key)	0 /* successful return */

#define ports0_macro_thread_setspecific(key, value) \
    TlsSetValue((key), (value))

#define ports0_macro_thread_getspecific(key) \
    TlsGetValue(key)

#define ports0_macro_thread_self() \
    _p0_thread_self()

#define ports0_macro_thread_equal( p0t1, p0t2 ) (p0t1 == p0t2)

#define ports0_macro_thread_once( once_control, init_routine ) \
     (*once_control ? 0 : _p0_actual_thread_once(once_control, init_routine))

#define ports0_macro_thread_yield() \
    0

#define ports0_i_am_only_thread() \
    ports0_macro_i_am_only_thread()

#define ports0_macro_mutex_init( mut, attr ) \
    (mut) = CreateMutex(NULL, FALSE, NULL)

#define ports0_macro_mutex_destroy( mut ) \
     ReleaseMutex( mut )

#define ports0_macro_cond_init( cv, attr ) \
     _p0_cond_init(cv, attr)

#define ports0_macro_cond_destroy( cv ) \
     _p0_cond_destroy(cv)

#define ports0_macro_cond_wait( cv, mut ) \
     _p0_cond_wait(cv, mut)

#define ports0_macro_cond_signal( cv ) \
     _p0_cond_signal(cv)

#define ports0_macro_mutex_lock( mut ) \
	  _p0_mutex_lock( mut )

#define ports0_macro_mutex_trylock( mut ) \
     _p0_mutex_trylock( mut )

#define ports0_macro_mutex_unlock( mut ) \
     _p0_mutex_unlock( mut )

#define ports0_macro_cond_broadcast( cv ) \
     _p0_cond_broadcast( cv )


#ifdef PORTS0_USE_MACROS

/*
#define ports0_threadattr_init(attr) \
    ports0_macro_threadattr_init(attr)

#define ports0_threadattr_destroy(attr) \
    ports0_macro_threadattr_destroy(attr)

#define ports0_threadattr_setstacksize(attr, stacksize) \
    ports0_macro_threadattr_setstacksize(attr, stacksize)

#define ports0_threadattr_getstacksize(attr, stacksize) \
    ports0_macro_threadattr_getstacksize(attr, stacksize) \

#define ports0_thread_key_create(key, value) \
    ports0_macro_thread_key_create(key, value)

#define ports0_thread_key_delete(key) \
    ports0_macro_thread_key_delete(key)

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

#define ports0_mutex_init( mut, attr ) \
    ports0_macro_mutex_init( mut, attr )

#define ports0_mutex_destroy( mut ) \
    ports0_macro_mutex_destroy( mut )

#define ports0_mutex_lock( mut ) \
    ports0_macro_mutex_lock( mut )

#define ports0_mutex_trylock( mut ) \
    ports0_macro_mutex_trylock( mut )

#define ports0_mutex_unlock( mut ) \
    ports0_macro_mutex_unlock( mut )

#define ports0_cond_init( cv, attr ) \
    ports0_macro_cond_init( cv, attr )

#define ports0_cond_destroy( cv ) \
    ports0_macro_cond_destroy( cv )

#define ports0_cond_wait( cv, mut ) \
    ports0_macro_cond_wait( cv, mut )

#define ports0_cond_signal( cv ) \
    ports0_macro_cond_signal( cv )

#define ports0_cond_broadcast( cv ) \
    ports0_macro_cond_broadcast( cv )
*/

#else  /* PORTS0_USE_MACROS */

extern int	ports0_threadattr_init(ports0_threadattr_t *attr);

extern int	ports0_threadattr_destroy(ports0_threadattr_t *attr);

extern int	ports0_threadattr_setstacksize(
              ports0_threadattr_t *attr,
				  size_t stacksize);

extern int	ports0_threadattr_getstacksize(
              ports0_threadattr_t *attr,
				  size_t *stacksize);

extern int	ports0_thread_key_create(
              ports0_thread_key_t *key,
              destructor_t func);

extern int	ports0_thread_key_delete(ports0_thread_key_t key);

extern int	ports0_thread_setspecific(
                 ports0_thread_key_t key,
					  void *value);

extern void *ports0_thread_getspecific(ports0_thread_key_t key);

extern ports0_thread_t ports0_thread_self(void);

extern int	ports0_thread_equal(ports0_thread_t t1, ports0_thread_t t2);

extern int	ports0_thread_once(ports0_thread_once_t *once_control,
				   void (*init_routine)(void));

extern void	ports0_thread_yield(void);

#undef ports0_i_am_only_thread

extern ports0_bool_t    ports0_i_am_only_thread(void);

extern int	ports0_mutex_init(ports0_mutex_t *mutex, ports0_mutexattr_t *attr);

extern int	ports0_mutex_destroy(ports0_mutex_t *mutex);

extern int	ports0_mutex_lock(ports0_mutex_t *mutex);

extern int	ports0_mutex_trylock(ports0_mutex_t *mutex);

extern int	ports0_mutex_unlock(ports0_mutex_t *mutex);

extern int	ports0_cond_init(ports0_cond_t *cond, ports0_condattr_t *attr);

extern int	ports0_cond_destroy(ports0_cond_t *cond);

extern int	ports0_cond_wait(ports0_cond_t *cond, ports0_mutex_t *mutex);

extern int	ports0_cond_signal(ports0_cond_t *cond);

extern int	ports0_cond_broadcast(ports0_cond_t *cond);

#endif /* PORTS0_USE_MACROS */

EXTERN_C_END
