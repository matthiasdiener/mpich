/*
 * th_qt_i.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_qt_i.h,v 1.4 1995/04/25 20:51:11 geisler Exp $"
 */

#define PORTS0_KEY_TABLE_SIZE 16

typedef struct _ports0_thread_name_list_t
{
    char *				name;
    struct _ports0_thread_name_list_t *	next;
} ports0_thread_name_list_t;

typedef struct __p0_thread_t
{
  int         id;
  ports0_thread_name_list_t *    names;
  void *                       key_table_values[PORTS0_KEY_TABLE_SIZE];

  void *                        context;
  ports0_thread_func_t           user_func;
  void *                        user_arg;

  struct __p0_thread_t *        next_free;
} __p0_thread_t;

#define _NX_THREAD_SIZE() \
    (sizeof(_p0_thread_t) + (key_table_size * sizeof(void *)))

/*
 * _p0_thread_id()
 *
 * Set *Thread_ID (int *) to be the thread id of the calling thread.
 */

#define _p0_thread_id(Thread_ID) \
	(*(Thread_ID) = get_mytid())
