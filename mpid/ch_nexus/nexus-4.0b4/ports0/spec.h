/*
 * spec.h
 *
 * This is the spec for the ports0 interface.
 * This is not a complete header file.  Use ports0.h instead.
 *
 * All functions,
 * unless otherwise indicated, will return an integer error status
 * of 0 for success and non-zero for an error, where the return 
 * value corresponds to a standard error from <errno.h>.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/spec.h,v 1.2 1995/03/30 18:41:41 tuecke Exp $"
 */

#ifndef PORTS0_H
#define PORTS0_H

#define PORTS0_THREAD_ONCE_INIT ??

/*-------------------------------------------------------------------
 * datatypes
 *-------------------------------------------------------------------*/

typedef ?? ports0_thread_t
typedef ?? ports0_threadattr_t
typedef ?? ports0_thread_once_t
typedef ?? ports0_thread_key_t
typedef ?? ports0_mutex_t
typedef ?? ports0_mutexattr_t
typedef ?? ports0_cond_t
typedef ?? ports0_condattr_t

/*-------------------------------------------------------------------
 * public interface
 *-------------------------------------------------------------------*/

/* -- package initialization, shutdown -- */

extern int ports0_init (int *argc,
			char **argv[],
			char *package_id);

/*
** package id is something like "ports" or "nexus" or "chant", 
** and allows each system to name its arguments distinctly.
** all recognized arguments must either be prefixed with the 
** package id, such as "-ports_version", or come between the 
** identifiers <package_id> and <package_id>_end, such as
** "-ports -version -ports_end".
*/

extern int ports0_shutdown (void);

/* -- thread attributes -- */

extern int ports0_threadattr_init (ports0_threadattr_t *attr);

extern int ports0_threadattr_destroy (ports0_threadattr_t *attr);

extern int ports0_threadattr_setstacksize (ports0_threadattr_t *attr,
					   size_t stacksize);

extern int ports0_threadattr_getstacksize (ports0_threadattr_t *attr,
					   size_t *stacksize);

/* -- thread management -- */

extern int ports0_thread_create (ports0_thread_t *thread,
				 const ports0_threadattr_t *attr,
				 void *(*start)(void*),
				 void *arg);

extern int ports0_thread_once (ports0_thread_once_t *once_control,
			       void (*start)(void));

extern int ports0_thread_exit (void *value);

extern int ports0_thread_yield (void);

extern ports0_thread_t ports0_thread_self (void);

extern int ports0_thread_equal (ports0_thread_t t1,
                                ports0_thread_t t2);

/* -- thread-specific data -- */

extern int ports0_thread_key_create (ports0_thread_key_t *key,
			             void (*destructor)(void*));

extern int ports0_thread_key_delete (ports0_thread_key_t key);

extern int ports0_thread_setspecific (ports0_thread_key_t key, 
			              const void *value);

extern void *ports0_thread_getspecific (ports0_thread_key_t key);

/* -- mutexes -- */

extern int ports0_mutexattr_init (ports0_mutexattr_t *attr);

extern int ports0_mutexattr_destroy (ports0_mutexattr_t *attr);

extern int ports0_mutex_init (ports0_mutex_t *mutex,
		              ports0_mutexattr_t *attr);

extern int ports0_mutex_destroy (ports0_mutex_t *mutex);

extern int ports0_mutex_lock (ports0_mutex_t *mutex);

extern int ports0_mutex_trylock (ports0_mutex_t *mutex);

extern int ports0_mutex_unlock (ports0_mutex_t *mutex);

/* -- condition variables -- */

extern int ports0_condattr_init (ports0_condattr_t *attr);

extern int ports0_condattr_destroy (ports0_condattr_t *attr);

extern int ports0_cond_init (ports0_cond_t *cond, 
			     ports0_condattr_t *attr);

extern int ports0_cond_destroy (ports0_cond_t *cond);

extern int ports0_cond_broadcast (ports0_cond_t *cond);

extern int ports0_cond_signal (ports0_cond_t *cond);

extern int ports0_cond_wait (ports0_cond_t *cond,
		             ports0_mutex_t *mutex);

/* -- reentrant functions -- */

extern int ports0_reentrant_lock (void);

extern int ports0_reentrant_unlock (void);

extern void *ports0_malloc (size_t bytes);

extern void *ports0_realloc (void *p,
			     size_t bytes);

extern void *ports0_calloc (size_t nobj, 
			    size_t bytes);

extern int ports0_free (void *ptr);

extern int ports0_open (char *path,
			int flags,
			int mode);

extern int ports0_close (int fd);

extern int ports0_read (int fd,
			char *buf,
			int nbytes);

extern int ports0_write (int fd,
			 char *buf,
			 int nbytes);

extern int ports0_fstat (int fd,
			 struct stat *buf);

extern int ports0_lseek (int fd,
			 off_t offset,
			 int whence);

#endif  PORTS0_H
