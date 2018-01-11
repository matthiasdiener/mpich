
static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/error.c,v 1.23 1996/10/07 04:39:54 tuecke Exp $";

#include "internal.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIBC_H
#include <libc.h>
#endif

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * descriptor_string()
 *
 * Note: This assumes nexus_stdio_lock() has been called.
 */
static void descriptor_string(char *fmt, char *s1, char *s2, char *s3)
{
    int context_id, thread_id;
    
    _nx_context_id(&context_id);
    _nx_thread_id(&thread_id);
    
    sprintf(fmt, "%s#%d:c%d:t%d:p%d%s%s%s%s%s%s",
	    _nx_my_node.name, _nx_my_node.number,
	    context_id, thread_id, _nx_md_getpid(),
	    (s1 ? ": " : ""),
	    (s1 ? s1 : ""),
	    (s2 ? ": " : ""),
	    (s2 ? s2 : ""),
	    (s3 ? ": " : ""),
	    (s3 ? s3 : "") );

} /* descriptor_string() */


/*
 * nexus_silent_fatal()
 *
 * Fatal error out without printing any messages.
 */
void nexus_silent_fatal(void)
{
    if (_nx_pause_on_fatal)
    {
	nexus_printf("Process %d pausing on fatal error\n", _nx_md_getpid());
	_nx_pausing_for_fatal = NEXUS_TRUE;
	nexus_pause();
    }

    nexus_abort();
} /* nexus_silent_fatal() */


/*
 * nexus_fatal()
 */
#ifdef __STDC__
void nexus_fatal(char *msg, ...)
#else
void nexus_fatal(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    nexus_stdio_lock();
    descriptor_string(fmt, "Fatal error", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    va_end(ap);

    fflush(_nx_stdout);
    nexus_stdio_unlock();

    nexus_silent_fatal();
    
} /* nexus_fatal() */


/*
 * nexus_error()
 */
#ifdef __STDC__
void nexus_error(char *msg, ...)
#else
void nexus_error(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    nexus_stdio_lock();
    descriptor_string(fmt, "Error", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();
} /* nexus_error() */


/*
 * nexus_warning()
 */
#ifdef __STDC__
void nexus_warning(char *msg, ...)
#else
void nexus_warning(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;
    
    nexus_stdio_lock();
    descriptor_string(fmt, "Warning", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();
} /* nexus_warning() */


/*
 * nexus_notice()
 */
#ifdef __STDC__
void nexus_notice(char *msg, ...)
#else
void nexus_notice(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    nexus_stdio_lock();
    descriptor_string(fmt, "Notice", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();
} /* nexus_notice() */


/*
 * nexus_printf()
 */
#ifdef __STDC__
void nexus_printf(char *msg, ...)
#else
void nexus_printf(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    nexus_stdio_lock();
    descriptor_string(fmt, msg, (char *) NULL, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();
} /* nexus_printf() */


/*
 * nexus_perror()
 */
#ifdef __STDC__
void nexus_perror(char *msg, ...)
#else
void nexus_perror(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;
    int save_error;

    nexus_stdio_lock();
    save_error = errno;
    descriptor_string(fmt, "", msg, _nx_md_system_error_string(save_error));

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();
} /* nexus_perror() */


#ifdef __STDC__
void nexus_fatal_perror(char *msg, ...)
#else
void nexus_fatal_perror(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;
    int save_error;

    nexus_stdio_lock();
    save_error = errno;
    descriptor_string(fmt, "Fatal error: ", msg, _nx_md_system_error_string(save_error));

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_nx_stdout, fmt, ap);
    fflush(_nx_stdout);
    va_end(ap);
    nexus_stdio_unlock();

    nexus_silent_fatal();
}

/*
 * nexus_assert_sprintf()
 *
 * This is used by the NexusAssert2() macro...
 */
#ifdef __STDC__
char *nexus_assert_sprintf(char *msg, ...)
#else
char *nexus_assert_sprintf(msg, va_alist)
char *msg;
va_dcl
#endif
{
    static char assert_sprintf_buf[1024];
    va_list ap;
    
    nexus_stdio_lock();
#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vsprintf(assert_sprintf_buf, msg, ap);
    va_end(ap);
    nexus_stdio_unlock();

    return (assert_sprintf_buf);
} /* nexus_assert_sprintf() */


