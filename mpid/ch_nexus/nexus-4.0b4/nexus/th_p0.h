/*
 * th_p.h
 *
 * General purpose ports0 module.  Supports:
 *  DCE threads
 *    HP
 *    IBM
 *  Solaris 2.3 threads
 *  quickthreads
 *    Solaris
 *    SunOS
 *    SGI
 *  C threads
 *  Florida State U. pthreads under SunOS 4.1.3
 *  Soon to be: MIT pthreads under
 *    Linux
 *    FreeBSD
 *    SunOS
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/th_p0.h,v 1.12 1996/12/12 07:48:21 tuecke Exp $"
   */

/*
 * #defines
 * USE_MACROS      -- Allow Nexus functions to be macros.  If this
 *				is not defined, then all Nexus functions
 *				must be real functions.
 */

#ifndef __NEXUS_TH_PORTS0_H
#define __NEXUS_TH_PORTS0_H


EXTERN_C_BEGIN

typedef ports0_mutex_t		_nx_ports0_mutex_t;
typedef ports0_cond_t		_nx_ports0_cond_t;
typedef ports0_thread_t		_nx_ports0_thread_t;
typedef ports0_threadattr_t	_nx_ports0_thread_attr_t;
typedef ports0_mutexattr_t	_nx_ports0_mutexattr_t;
typedef ports0_condattr_t	_nx_ports0_condattr_t;
typedef ports0_thread_key_t	_nx_ports0_thread_key_t;
typedef ports0_thread_once_t	_nx_ports0_thread_once_t;

#define _NX_PORTS0_THREAD_ONCE_INIT	PORTS0_THREAD_ONCE_INIT

#define _NX_THR_SUCCESS 0

typedef _nx_ports0_thread_t		nexus_thread_t;
typedef _nx_ports0_thread_attr_t	nexus_thread_attr_t;
typedef _nx_ports0_mutexattr_t		nexus_mutexattr_t;
typedef _nx_ports0_condattr_t		nexus_condattr_t;
typedef _nx_ports0_thread_key_t		nexus_thread_key_t;
typedef _nx_ports0_thread_once_t	nexus_thread_once_t;
#define NEXUS_THREAD_ONCE_INIT		_NX_PORTS0_THREAD_ONCE_INIT

typedef struct _nexus_mutex_t
{
    _NX_START_MAGIC_COOKIE
    _nx_ports0_mutex_t mutex;
    _NX_END_MAGIC_COOKIE
} nexus_mutex_t;

typedef struct _nexus_cond_t
{
    _NX_START_MAGIC_COOKIE
    _nx_ports0_cond_t cond;
    _NX_END_MAGIC_COOKIE
} nexus_cond_t;


typedef void (*nexus_thread_key_destructor_func_t)(void *value);

typedef struct _nexus_global_vars_t
{
    _nx_ports0_thread_key_t		nexus_thread_t_pointer;
} nexus_global_vars_t;

NEXUS_GLOBAL nexus_global_vars_t nexus_all_global_vars;
extern void	_nx_report_bad_rc(int rc, char *message);

extern int	nexus_thread_create(nexus_thread_t *thread,
				    nexus_thread_attr_t *attr,
				    nexus_thread_func_t func,
				    void *user_arg);
extern void	nexus_thread_exit(void *status);
extern int	nexus_thread_key_create(nexus_thread_key_t *key,
				     nexus_thread_key_destructor_func_t func);


#define _nx_test_rc( a, b ) \
do \
{ \
    if( a != _NX_THR_SUCCESS ) _nx_report_bad_rc( a, b ); \
} while(0)


#define nexus_macro_thread_setspecific(key, value) \
    ports0_thread_setspecific(key, value)

#define nexus_macro_thread_getspecific(key) \
    ports0_thread_getspecific(key)

#define nexus_macro_thread_self() \
    ports0_thread_self()

#define nexus_macro_thread_equal(t1, t2) \
    ports0_thread_equal(t1, t2)

#define nexus_macro_thread_once(once_control, init_routine) \
    ports0_thread_once(once_control, init_routine)

#define nexus_macro_thread_yield() \
    ports0_thread_yield();

#define nexus_macro_mutex_init( mut, attr ) \
    ports0_mutex_init( &((mut)->mutex), attr )

#define nexus_macro_mutex_destroy( mut ) \
    ports0_mutex_destroy( &((mut)->mutex) )

#define nexus_macro_mutex_lock( mut ) \
    ports0_mutex_lock( &((mut)->mutex) )

#define nexus_macro_mutex_unlock( mut ) \
    ports0_mutex_unlock( &((mut)->mutex) )

#define nexus_macro_mutex_trylock( mut ) \
    ports0_mutex_trylock( &((mut)->mutex) )

#define nexus_macro_cond_init( cv, attr ) \
    ports0_cond_init( &((cv)->cond), attr )

#define nexus_macro_cond_destroy( cv ) \
    ports0_cond_destroy( &((cv)->cond) )

#define nexus_macro_cond_wait( cv, mut ) \
    ports0_cond_wait( &((cv)->cond), &((mut)->mutex) )

#define nexus_macro_cond_signal( cv ) \
    ports0_cond_signal( &((cv)->cond) )

#define nexus_macro_cond_broadcast( cv ) \
    ports0_cond_broadcast( &((cv)->cond) )

#ifdef USE_MACROS

#define nexus_thread_setspecific(key, value) \
    nexus_macro_thread_setspecific(key, value)
#define nexus_thread_getspecific(key) \
    nexus_macro_thread_getspecific(key)
#define nexus_thread_self() \
    nexus_macro_thread_self()
#define nexus_thread_equal(t1, t2) \
    nexus_macro_thread_equal(t1, t2)
#define nexus_thread_once(once_control, init_routine) \
    nexus_macro_thread_once(once_control, init_routine)
#define nexus_thread_yield() \
    nexus_macro_thread_yield()
#define nexus_mutex_init( mut, attr ) \
    nexus_macro_mutex_init( mut, attr )
#define nexus_mutex_destroy( mut ) \
    nexus_macro_mutex_destroy( mut )
#define nexus_mutex_lock( mut ) \
    nexus_macro_mutex_lock( mut )
#define nexus_mutex_unlock( mut ) \
    nexus_macro_mutex_unlock( mut )
#define nexus_mutex_trylock( mut ) \
    nexus_macro_mutex_trylock( mut )
#define nexus_cond_init( cv, attr ) \
    nexus_macro_cond_init( cv, attr )
#define nexus_cond_destroy( cv ) \
    nexus_macro_cond_destroy( cv )
#define nexus_cond_wait( cv, mut ) \
    nexus_macro_cond_wait( cv, mut )
#define nexus_cond_signal( cv ) \
    nexus_macro_cond_signal( cv )
#define nexus_cond_broadcast( cv ) \
    nexus_macro_cond_broadcast( cv )

#else  /* USE_MACROS */

extern int		nexus_thread_setspecific(nexus_thread_key_t key,
						 void *value);
extern void *		nexus_thread_getspecific(nexus_thread_key_t key);
extern nexus_thread_t	nexus_thread_self(void);
extern int		nexus_thread_equal(nexus_thread_t t1,
					   nexus_thread_t t2);
extern int		nexus_thread_once(nexus_thread_once_t *once_control,
					  void (*init_routine)(void));
extern void		nexus_thread_yield(void);
extern int		nexus_mutex_init(nexus_mutex_t *mutex,
					 nexus_mutexattr_t *attr);
extern int		nexus_mutex_destroy(nexus_mutex_t *mutex);
extern int		nexus_mutex_lock(nexus_mutex_t *mutex);
extern int		nexus_mutex_unlock(nexus_mutex_t *mutex);
extern int              nexus_mutex_trylock(nexus_mutex_t *mutex);
extern int		nexus_cond_init(nexus_cond_t *cond,
					nexus_condattr_t *attr);
extern int		nexus_cond_destroy(nexus_cond_t *cond);
extern int		nexus_cond_wait(nexus_cond_t *cond,
					nexus_mutex_t *mutex);
extern int		nexus_cond_signal(nexus_cond_t *cond);
extern int		nexus_cond_broadcast(nexus_cond_t *cond);

#endif /* USE_MACROS */

EXTERN_C_END

#endif /* __NEXUS_TH_PORTS0_H */
