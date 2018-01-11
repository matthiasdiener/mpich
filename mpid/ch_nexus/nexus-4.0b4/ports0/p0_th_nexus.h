/*
 * p0_th_nexus.h
 *
 * This ports0 thread module just redirects all thread calls
 * to the equivalent Nexus routines.
 * This allows a native Nexus thread library to be used with the
 * rest of ports0.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_nexus.h,v 1.2 1996/02/02 01:13:14 patton Exp $";
 */

#ifdef NEXUS_TH_HEADER

/*
 * The include of this file is occurring while compiling nexus or
 * nexus user code.
 */
#include "nx_sanity.h"
#include NEXUS_TH_HEADER

#else  /* NEXUS_TH_HEADER */

/*
 * The include of this file is occurring while compiling ports0
 */
#ifdef USE_MACROS
#define GOT_USE_MACROS
#undef USE_MACROS
#endif
#include "nexus.h"

#endif /* NEXUS_TH_HEADER */

typedef nexus_thread_t		ports0_thread_t;
typedef nexus_thread_attr_t	ports0_threadattr_t;
typedef nexus_mutexattr_t	ports0_mutexattr_t;
typedef nexus_condattr_t	ports0_condattr_t;
typedef nexus_mutex_t		ports0_mutex_t;
typedef nexus_cond_t		ports0_cond_t;
typedef nexus_thread_once_t	ports0_thread_once_t;
typedef nexus_thread_key_t	ports0_thread_key_t;

#ifndef ENOSYS
#define ENOSYS 87
#endif

#define ports0_thread_create(t,a,f,u) nexus_thread_create(t,a,f,u)
#define ports0_thread_exit(s) nexus_thread_exit(s)
#define ports0_thread_key_create(k,f) nexus_thread_key_create(k,f)
#define ports0_thread_key_delete(k) nexus_thread_key_delete(k)
#define ports0_thread_setspecific(k,v) nexus_thread_setspecific(k,v)
#define ports0_thread_getspecific(k,v) nexus_thread_getspecific(k,v)
#define ports0_threadattr_init(attr)	0 /* successful return */
#define ports0_threadattr_destroy(attr)	0 /* successful return */
#define ports0_threadattr_setstacksize(attr, stacksize) \
	ENOSYS /* unsuccessful return - not implemented */
#define ports0_threadattr_getstacksize(attr, stacksize) \
	ENOSYS /* unsuccessful return - not implemented */
#define ports0_thread_yield() nexus_thread_yield()
#define ports0_thread_self() nexus_thread_self()
#define ports0_thread_equal(t1,t2) nexus_thread_equal(t1,t2)
#define ports0_thread_once(o,i) nexus_thread_once(o,i)
#define ports0_mutex_init(m,a) (nexus_mutex_init(m,a), 0)
#define ports0_mutex_destroy(m) (nexus_mutex_destroy(m), 0)
#define ports0_mutex_lock(m) (nexus_mutex_lock(m), 0)
#define ports0_mutex_trylock(m) (EBUSY)
#define ports0_mutex_unlock(m) (nexus_mutex_unlock(m), 0)
#define ports0_cond_init(c,a) (nexus_cond_init(c,a), 0)
#define ports0_cond_destroy(c) (nexus_cond_destroy(c), 0)
#define ports0_cond_signal(c) (nexus_cond_signal(c), 0)
#define ports0_cond_broadcast(c) (nexus_cond_broadcast(c), 0)
#define ports0_cond_wait(c,m) (nexus_cond_wait(c,m), 0)

#ifdef GOT_USE_MACROS
#define USE_MACROS
#endif
