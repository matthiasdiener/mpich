/*
 * mac_func.c
 *
 * Many of the Nexus functions can be efficiently implemented
 * as macros.  However, in some situations you may not want to use
 * the macros (i.e. when debugging).  If the user
 * defines USE_MACROS before including nexus.h, then the
 * macro versions will be used.
 *
 * This file holds the function versions of these various macros.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/mac_func.c,v 1.22 1997/01/22 05:43:00 tuecke Exp $";

#include "internal.h"


/*
 * nexus_stdio_lock()
 */
#undef nexus_stdio_lock
void nexus_stdio_lock(void)
{
#if !defined(HAVE_THREAD_SAFE_STDIO) && !defined(BUILD_LITE)
    nexus_mutex_lock(&nexus_stdio_mutex);
#endif
} /* nexus_stdio_lock() */


/*
 * nexus_stdio_unlock()
 */
#undef nexus_stdio_unlock
void nexus_stdio_unlock(void)
{
#if !defined(HAVE_THREAD_SAFE_STDIO) && !defined(BUILD_LITE)
    nexus_mutex_unlock(&nexus_stdio_mutex);
#endif
} /* nexus_stdio_unlock() */


/*
 * nexus_send_rsr()
 */
#undef nexus_send_rsr
int nexus_send_rsr(nexus_buffer_t *buffer,
		   nexus_startpoint_t *startpoint,
		   int handler_id,
		   nexus_bool_t destroy_buffer,
		   nexus_bool_t called_from_non_threaded_handler)
{
    return(nexus_macro_send_rsr(buffer,
				startpoint,
				handler_id,
				destroy_buffer,
				called_from_non_threaded_handler));
} /* nexus_send_rsr() */


/*
 * nexus_endpoint_set_user_pointer()
 */
#undef nexus_endpoint_set_user_pointer
void nexus_endpoint_set_user_pointer(nexus_endpoint_t *endpoint, void *address)
{
    nexus_macro_endpoint_set_user_pointer(endpoint, address);
} /* nexus_endpoint_set_user_pointer() */


/*
 * nexus_endpoint_get_user_pointer()
 */
#undef nexus_endpoint_get_user_pointer
void *nexus_endpoint_get_user_pointer(nexus_endpoint_t *endpoint)
{
    return(nexus_macro_endpoint_get_user_pointer(endpoint));
} /* nexus_endpoint_get_user_pointer() */


/*
 * nexus_startpoint_set_null()
 */
#undef nexus_startpoint_set_null
void nexus_startpoint_set_null(nexus_startpoint_t *sp)
{
    nexus_macro_startpoint_set_null(sp);
} /* nexus_startpoint_set_null() */


/*
 * nexus_startpoint_is_null()
 *
 * Return non-zero if gp is a NULL global pointer, otherwise return zero.
 */
#undef nexus_startpoint_is_null
int nexus_startpoint_is_null(nexus_startpoint_t *sp)
{
    return (nexus_macro_startpoint_is_null(sp));
} /* nexus_startpoint_is_null() */


#ifdef BUILD_LITE
/*
 * nexus_mutex_init()
 */
#undef nexus_mutex_init
int nexus_mutex_init(nexus_mutex_t *mut, nexus_mutexattr_t *attr)
{
    return (nexus_macro_mutex_init(mut, attr));
} /* nexus_mutex_init() */


/*
 *  nexus_mutex_destroy()
 */
#undef nexus_mutex_destroy
int nexus_mutex_destroy(nexus_mutex_t *mut)
{
    return (nexus_macro_mutex_destroy(mut));
} /* nexus_mutex_destroy() */


/* 
 *  nexus_mutex_lock()
 */
#undef nexus_mutex_lock
int nexus_mutex_lock(nexus_mutex_t *mut)
{
    return (nexus_macro_mutex_lock(mut));
} /* nexus_mutex_lock() */


/*
 *  nexus_mutex_unlock()
 */
#undef nexus_mutex_unlock
int nexus_mutex_unlock(nexus_mutex_t *mut)
{
    return (nexus_macro_mutex_unlock(mut));
} /* nexus_mutex_unlock() */


/*
 * nexus_cond_init()
 */
#undef nexus_cond_init
int nexus_cond_init(nexus_cond_t *cv, nexus_condattr_t *attr)
{
    return (nexus_macro_cond_init(cv, attr));
} /* nexus_cond_init() */


/*
 *  nexus_cond_destroy()
 */
#undef nexus_cond_destroy
int nexus_cond_destroy(nexus_cond_t *cv)
{
    return (nexus_macro_cond_destroy(cv));
} /* nexus_cond_destroy() */


/*
 *  nexus_cond_wait()
 */
#undef nexus_cond_wait
int nexus_cond_wait(nexus_cond_t *cv, nexus_mutex_t *mut)
{
    return (nexus_macro_cond_wait(cv, mut));
} /* nexus_cond_wait() */


/*
 *  nexus_cond_signal()
 */
#undef nexus_cond_signal
int nexus_cond_signal(nexus_cond_t *cv)
{
    return (nexus_macro_cond_signal(cv));
} /* nexus_cond_signal () */


/*
 *  nexus_cond_broadcast()
 */
#undef nexus_cond_broadcast
int nexus_cond_broadcast(nexus_cond_t *cv)
{
    return (nexus_macro_cond_broadcast(cv));
} /* nexus_cond_broadcast() */


/*
 *  nexus_thread_yield()
 */
#undef nexus_thread_yield
void nexus_thread_yield(void)
{
    nexus_macro_thread_yield();
} /* nexus_cond_broadcast() */

#endif /* BUILD_LITE */
