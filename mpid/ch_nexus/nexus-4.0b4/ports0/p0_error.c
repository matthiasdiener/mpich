/*
 * p0_error.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_error.c,v 1.6 1996/02/28 20:44:04 patton Exp $";

#include "p0_internal.h"

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

extern int errno;
extern char *sys_errlist[];


#ifdef BUILD_LITE

/*
 * descriptor_string()
 *
 * Note: This assumes ports0_stdio_lock() has been called.
 */
static void descriptor_string(char *fmt, char *s1, char *s2, char *s3)
{
    sprintf(fmt, "p%d%s%s%s%s%s%s",
	    getpid(),
	    (s1 ? ": " : ""),
	    (s1 ? s1 : ""),
	    (s2 ? ": " : ""),
	    (s2 ? s2 : ""),
	    (s3 ? ": " : ""),
	    (s3 ? s3 : "") );
} /* descriptor_string() */

#else  /* BUILD_LITE */

/*
 * descriptor_string()
 *
 * Note: This assumes ports0_stdio_lock() has been called.
 */
static void descriptor_string(char *fmt, char *s1, char *s2, char *s3)
{
    int thread_id;
    _p0_thread_id(&thread_id);
    sprintf(fmt, "t%d:p%d%s%s%s%s%s%s",
	    thread_id, getpid(),
	    (s1 ? ": " : ""),
	    (s1 ? s1 : ""),
	    (s2 ? ": " : ""),
	    (s2 ? s2 : ""),
	    (s3 ? ": " : ""),
	    (s3 ? s3 : "") );
} /* descriptor_string() */

#endif /* BUILD_LITE */

/*
 * ports0_silent_fatal()
 *
 * Fatal error out without printing any messages.
 */
void ports0_silent_fatal(void)
{
    ports0_shutdown();
    exit(1);
} /* ports0_silent_fatal() */


/*
 * ports0_fatal()
 */
#ifdef __STDC__
void ports0_fatal(char *msg, ...)
#else
void ports0_fatal(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, "Fatal error", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    va_end(ap);

    fflush(_p0_stdout);
    ports0_stdio_unlock();

    ports0_silent_fatal();
    
} /* ports0_fatal() */


/*
 * ports0_error()
 */
#ifdef __STDC__
void ports0_error(char *msg, ...)
#else
void ports0_error(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, "Error", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();
} /* ports0_error() */


/*
 * ports0_warning()
 */
#ifdef __STDC__
void ports0_warning(char *msg, ...)
#else
void ports0_warning(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;
    
    ports0_stdio_lock();
    descriptor_string(fmt, "Warning", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();
} /* ports0_warning() */


/*
 * ports0_notice()
 */
#ifdef __STDC__
void ports0_notice(char *msg, ...)
#else
void ports0_notice(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, "Notice", msg, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();
} /* ports0_notice() */


/*
 * ports0_printf()
 */
#ifdef __STDC__
void ports0_printf(char *msg, ...)
#else
void ports0_printf(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, msg, (char *) NULL, (char *) NULL);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();

} /* ports0_printf() */


/*
 * ports0_perror()
 */
#ifdef __STDC__
void ports0_perror(char *msg, ...)
#else
void ports0_perror(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, "", msg, sys_errlist[errno]);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();
} /* ports0_perror() */


#ifdef __STDC__
void ports0_fatal_perror(char *msg, ...)
#else
void ports0_fatal_perror(msg, va_alist)
char *msg;
va_dcl
#endif
{
    char fmt[1024];
    va_list ap;

    ports0_stdio_lock();
    descriptor_string(fmt, "Fatal error: ", msg, sys_errlist[errno]);

#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vfprintf(_p0_stdout, fmt, ap);
    fflush(_p0_stdout);
    va_end(ap);
    ports0_stdio_unlock();

    ports0_silent_fatal();
}

/*
 * ports0_assert_sprintf()
 *
 * This is used by the Ports0Assert2() macro...
 */
#ifdef __STDC__
char *ports0_assert_sprintf(char *msg, ...)
#else
char *ports0_assert_sprintf(msg, va_alist)
char *msg;
va_dcl
#endif
{
    static char assert_sprintf_buf[1024];
    va_list ap;
    
    ports0_stdio_lock();
#ifdef __STDC__
    va_start(ap, msg);
#else
    va_start(ap);
#endif
    vsprintf(assert_sprintf_buf, msg, ap);
    va_end(ap);
    ports0_stdio_unlock();

    return (assert_sprintf_buf);
} /* ports0_assert_sprintf() */


