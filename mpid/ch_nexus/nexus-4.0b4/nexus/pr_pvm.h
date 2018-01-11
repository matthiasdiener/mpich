/*
 * Author:      John W. Garnett
 *              California Institute of Technology
 *              Compositional C++ Group
 *              1994 May 12
 *
 * pr_pvm.h		- header file for PVM 3.2.6 protocol module for Nexus.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/pr_pvm.h,v 1.11 1996/10/07 04:40:10 tuecke Exp $"
 */

#ifndef _PR_PVM_H
#define _PR_PVM_H

#include <pvm3.h>
#include "internal.h"

#ifdef NEXUS_SINGLE_THREADED
#define IPVM_PROTO_SINGLE_THREADED
#endif

#define IPVM_POLL           1
#define IPVM_HANDLER_THREAD 2

#define HANDLER_TAG 42
#define COMMAND_TAG 21

#define CLOSE_SHUTDOWN_FLAG            1
#define CLOSE_ABNORMAL_FLAG            2
#define CLOSE_SELF_SHUTDOWN_FLAG       3
#define CLOSE_SELF_ABNORMAL_FLAG       4

#define _nx_set_i_am_handler_thread() \
    nexus_thread_setspecific(i_am_handler_thread_key, (void *) 1)
#define _nx_i_am_handler_thread(Result) \
    *(Result) = (nexus_bool_t)nexus_thread_getspecific(i_am_handler_thread_key)

#define IPVM_TEST(code, msg) \
	do { if (code < 0) pvm_perror(msg); } while (0)

/*
 * Only one thread is allowed to be in the PVM code (and thus
 * mucking with data structures) at a time.
 */
#ifdef IPVM_PROTO_SINGLE_THREADED

#define ipvm_enter()
#define ipvm_exit()
#define ipvm_wait()

#else  /* IPVM_PROTO_SINGLE_THREADED */

#define ipvm_enter() \
do { \
    nexus_mutex_lock(&ipvm_mutex); \
    while (ipvm_in_use) { \
	ipvm_threads_waiting++; \
	nexus_cond_wait(&ipvm_cond, &ipvm_mutex); \
	ipvm_threads_waiting--; \
    } \
    ipvm_in_use = NEXUS_TRUE; \
    nexus_mutex_unlock(&ipvm_mutex); \
} while (0)

#define ipvm_exit() \
do { \
    nexus_mutex_lock(&ipvm_mutex); \
    ipvm_in_use = NEXUS_FALSE; \
    if (ipvm_threads_waiting > 0) { \
	nexus_cond_signal(&ipvm_cond); \
    } \
    nexus_mutex_unlock(&ipvm_mutex); \
} while (0)

#define ipvm_wait() \
do { \
    nexus_mutex_lock(&ipvm_mutex); \
    ipvm_in_use = NEXUS_FALSE; \
    ipvm_threads_waiting++; \
    nexus_cond_wait(&ipvm_cond, &ipvm_mutex); \
    ipvm_threads_waiting--; \
    while (ipvm_in_use) { \
	ipvm_threads_waiting++; \
	nexus_cond_wait(&ipvm_cond, &ipvm_mutex); \
	ipvm_threads_waiting--; \
    } \
    ipvm_in_use = NEXUS_TRUE; \
    nexus_mutex_unlock(&ipvm_mutex); \
} while (0)
#endif /* IPVM_PROTO_SINGLE_THREADED */

#define ipvm_fatal ipvm_exit(); nexus_fatal

/* Some forward typedef declarations...  */

typedef struct _ipvm_buffer_t	ipvm_buffer_t;
typedef struct _ipvm_proto_t	ipvm_proto_t;
typedef struct _ipvm_incoming_t	ipvm_incoming_t;

/*
 * ipvm_buffer_t
 *
 * This is an overload of nexus_buffer_t.  It adds the
 * PVM specific information to that structure.
 */

struct _ipvm_buffer_t {
#ifdef BUILD_DEBUG
    int				magic;
#endif
    nexus_buffer_funcs_t* funcs;
    ipvm_buffer_t* next; /* for managing the freelist */
	nexus_global_pointer_t* gp;
    int buffer_type; /* SEND_BUFFER || RECV_BUFFER */
	int bufid; /* PVM buffer id */
	nexus_bool_t stashed;
#ifdef BUILD_PROFILE
	char* handler_name;
	int handler_id;
#endif
};

#ifdef LOCAL_PROTO_SINGLE_THREADED
#define lock_buffer_free_list()
#define unlock_buffer_free_list()
#else
#define lock_buffer_free_list() nexus_mutex_lock(&buffer_free_list_mutex)
#define unlock_buffer_free_list() nexus_mutex_unlock(&buffer_free_list_mutex)
#endif

#define SEND_BUFFER	1
#define RECV_BUFFER	2

#ifdef BUILD_DEBUG
#define MallocPvmBuffer(Routine, Buf) \
do { \
	NexusMalloc(Routine,Buf,ipvm_buffer_t*,sizeof(struct _ipvm_buffer_t)); \
	Buf->magic = NEXUS_BUFFER_MAGIC; \
} while (0)
#else  /* BUILD_DEBUG */
#define MallocPvmBuffer(Routine, Buf) \
do { \
	NexusMalloc(Routine,Buf,ipvm_buffer_t*,sizeof(struct _ipvm_buffer_t)); \
} while (0)
#endif /* BUILD_DEBUG */

#define GetPvmBuffer(Routine, Buf) \
do { \
    lock_buffer_free_list(); \
    if (buffer_free_list) { \
	Buf = buffer_free_list; \
	(Buf)->magic = NEXUS_BUFFER_MAGIC; \
	buffer_free_list = buffer_free_list->next; \
	unlock_buffer_free_list(); \
    } \
    else { \
	unlock_buffer_free_list(); \
	MallocPvmBuffer(Routine, Buf); \
    } \
} while (0)

#define FreePvmBuffer(Buf) \
do { \
	pvm_freebuf((Buf)->bufid); \
	lock_buffer_free_list(); \
	(Buf)->next = buffer_free_list; \
	buffer_free_list = (Buf); \
	(Buf)->magic = 0xdeadbeef; \
	unlock_buffer_free_list(); \
} while (0)

/*
 * ipvm_proto_t
 *
 * This is an overload of nexus_proto_t.  It adds the
 * PVM specific information to that structure.
 */
struct _ipvm_proto_t
{
    nexus_proto_type_t	type;	/* NEXUS_PROTO_TYPE_PVM */
    nexus_proto_funcs_t *funcs;
	int tid;
};

static void ipvm_process_arguments_init(void);
static int ipvm_process_arguments(int current_arg, int arg_count);
static void ipvm_usage_message(void);
static int ipvm_new_process_params(char *buf, int size);

static void		ipvm_init(void);
static void		ipvm_shutdown(nexus_bool_t shutdown_others);
static void		ipvm_abort(void);
static void		ipvm_poll(void);

static void ipvm_init_remote_service_request(
	nexus_buffer_t *buffer, nexus_global_pointer_t *gp,
	char *handler_name, int handler_id);
static void  ipvm_send_remote_service_request(nexus_buffer_t *buffer);
static void ipvm_send_urgent_remote_service_request(nexus_buffer_t *buffer);
static void ipvm_destroy_proto(nexus_proto_t *nexus_proto);
static nexus_mi_proto_t* ipvm_get_my_mi_proto(void);
static nexus_bool_t ipvm_construct_from_mi_proto(nexus_proto_t **proto,
	nexus_mi_proto_t *mi_proto);
static void ipvm_construct_creator_proto(nexus_proto_t **proto);
static int ipvm_compare_protos(nexus_proto_t *proto1, nexus_proto_t *proto2);

static void ipvm_set_buffer_size(nexus_buffer_t *buffer,
	int size, int n_elements);
static int ipvm_check_buffer_size(nexus_buffer_t *buffer,
	int slack, int increment);
static void ipvm_free_buffer(nexus_buffer_t *buffer);
static void ipvm_stash_buffer(nexus_buffer_t *buffer,
	nexus_stashed_buffer_t *stashed_buffer);
static void ipvm_free_stashed_buffer(nexus_stashed_buffer_t *stashed_buffer);

static int	ipvm_sizeof_float(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_double(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_short(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_u_short(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_int(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_u_int(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_long(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_u_long(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_char(nexus_buffer_t *buffer, int count);
static int	ipvm_sizeof_u_char(nexus_buffer_t *buffer, int count);

static void	ipvm_put_float(nexus_buffer_t *buffer, float *data, int count);
static void	ipvm_put_double(nexus_buffer_t *buffer, double *data, int count);
static void	ipvm_put_short(nexus_buffer_t *buffer, short *data, int count);
static void	ipvm_put_u_short(nexus_buffer_t *buffer,
	unsigned short *data, int count);
static void	ipvm_put_int(nexus_buffer_t *buffer, int *data, int count);
static void	ipvm_put_u_int(nexus_buffer_t *buffer,
	unsigned int *data, int count);
static void	ipvm_put_long(nexus_buffer_t *buffer, long *data, int count);
static void	ipvm_put_u_long(nexus_buffer_t *buffer,
	unsigned long *data, int count);
static void	ipvm_put_char(nexus_buffer_t *buffer, char *data, int count);
static void	ipvm_put_u_char(nexus_buffer_t *buffer,
	unsigned char *data, int count);
static void	ipvm_get_float(nexus_buffer_t *buffer, float *dest, int count);
static void	ipvm_get_double(nexus_buffer_t *buffer, double *dest, int count);
static void	ipvm_get_short(nexus_buffer_t *buffer, short *dest, int count);
static void	ipvm_get_u_short(nexus_buffer_t *buffer,
	unsigned short *dest, int count);
static void	ipvm_get_int(nexus_buffer_t *buffer, int *dest, int count);
static void	ipvm_get_u_int(nexus_buffer_t *buffer,
	unsigned int *dest, int count);
static void	ipvm_get_long(nexus_buffer_t *buffer, long *dest, int count);
static void	ipvm_get_u_long(nexus_buffer_t *buffer,
	unsigned long *dest, int count);
static void	ipvm_get_char(nexus_buffer_t *buffer, char *dest, int count);
static void	ipvm_get_u_char(nexus_buffer_t *buffer,
	unsigned char *dest, int count);
static void	ipvm_get_stashed_float(nexus_stashed_buffer_t *buffer,
	float *dest, int count);
static void	ipvm_get_stashed_double(nexus_stashed_buffer_t *buffer,
	double *dest, int count);
static void	ipvm_get_stashed_short(nexus_stashed_buffer_t *buffer,
	short *dest, int count);
static void	ipvm_get_stashed_u_short(nexus_stashed_buffer_t *buffer,
	unsigned short *dest, int count);
static void	ipvm_get_stashed_int(nexus_stashed_buffer_t *buffer,
	int *dest, int count);
static void	ipvm_get_stashed_u_int(nexus_stashed_buffer_t *buffer,
	unsigned int *dest, int count);
static void	ipvm_get_stashed_long(nexus_stashed_buffer_t *buffer,
	long *dest, int count);
static void	ipvm_get_stashed_u_long(nexus_stashed_buffer_t *buffer,
	unsigned long *dest, int count);
static void	ipvm_get_stashed_char(nexus_stashed_buffer_t *buffer,
	char *dest, int count);
static void	ipvm_get_stashed_u_char(nexus_stashed_buffer_t *buffer,
	unsigned char *dest, int count);

#endif /* _PR_PVM_H */
