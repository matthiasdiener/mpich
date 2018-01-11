/*
 * p0_th_sol_i.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_sol_i.h,v 1.2 1995/03/30 18:40:34 tuecke Exp $"
 */

typedef struct _ports0_thread_name_list_t
{
    char *				name;
    struct _ports0_thread_name_list_t *	next;
} ports0_thread_name_list_t;

typedef struct __p0_thread_t
{
  int         id;
  ports0_thread_name_list_t *    names;
  void *                        context;
  ports0_thread_func_t           user_func;
  void *                        user_arg;
  struct __p0_thread_t *        next_free;
} _p0_thread_t;

/*
 * _p0_thread_self()
 *
 * Set *Thread (_p0_thread_t **) to the calling thread.
 */
#define _p0_thread_self(Thread) \
{  \
    int rc;  \
     rc = thr_getspecific(ports0_all_global_vars.ports0_thread_t_pointer, \
			  (any_t *)(Thread)); \
     _p0_test_rc( rc, "PORTS0: get thread-local data failed\n"); \
}


/*
 * _p0_thread_id()
 *
 * Set *Thread_ID (int *) to be the thread id of the calling thread.
 */
#define _p0_thread_id(Thread_ID) \
{ \
    _p0_thread_t *__thread; \
    _p0_thread_self(&__thread); \
    *(Thread_ID) = __thread->id; \
}
