/*
 * p0_th_c_i.h
 *
 * Internal header for a C-threads based ports0 threads library.
 * This will be included only by ports0 modules.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_c_i.h,v 1.2 1995/03/30 18:39:11 tuecke Exp $"
 */


typedef struct _ports0_thread_name_list_t
{
    char *				name;
    struct _ports0_thread_name_list_t *	next;
} ports0_thread_name_list_t;

typedef struct __p0_thread_t
{
    int				id;
    ports0_thread_name_list_t *	names;
    void **			key_table_values;
    
    /* These are used only during thread startup */
    void *			context;
    ports0_thread_func_t		user_func;
    void *			user_arg;
    
    /* This is only used when this structure is on the free list */
    struct __p0_thread_t *	next_free;
} _p0_thread_t;

#define _P0_THREAD_SIZE() \
    (sizeof(_p0_thread_t) + (key_table_size * sizeof(void *)))


/*
 * _p0_thread_self()
 *
 * Set *Thread (_p0_thread_t **) to the calling thread.
 */
#define _p0_thread_self(Thread) \
    *(Thread) = ((_p0_thread_t *) cthread_data(cthread_self()))


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
