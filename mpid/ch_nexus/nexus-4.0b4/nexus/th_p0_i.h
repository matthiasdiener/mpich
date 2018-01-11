/*
 * th_p_i.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/th_p0_i.h,v 1.5 1996/10/07 04:40:21 tuecke Exp $"
 */

typedef struct _nexus_thread_name_list_t
{
    char *				name;
    struct _nexus_thread_name_list_t *	next;
} nexus_thread_name_list_t;

typedef struct __nx_thread_t
{
    int					id;
    nexus_thread_name_list_t *		names;
    void *				context;
    nexus_thread_func_t			user_func;
    void *				user_arg;
    struct __nx_thread_t *		next_free;
} _nx_thread_t;

/*
 * _nx_thread_self()
 *
 * Set *Thread (_nx_thread_t **) to the calling thread.
 */
#define _nx_thread_self(Thread) \
{  \
    *(Thread) = (_nx_thread_t *)ports0_thread_getspecific(nexus_all_global_vars.nexus_thread_t_pointer); \
}

/*
 * _nx_thread_id()
 *
 * Set *Thread_ID (int *) to be the thread id of the calling thread.
 */
#define _nx_thread_id(Thread_ID) \
{ \
    _nx_thread_t *__thread; \
    _nx_thread_self(&__thread); \
    *(Thread_ID) = __thread->id; \
}

extern void _nx_thread_prefork(void);
extern void _nx_thread_postfork(void);
