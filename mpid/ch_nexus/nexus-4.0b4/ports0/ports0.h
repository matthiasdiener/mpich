/*
 * ports0.h
 *
 * This header contains the exported interface of Ports0.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/ports0.h,v 1.28 1996/10/19 17:10:06 carl Exp $"
 */

#ifndef _PORTS0_H
#define _PORTS0_H

/*
 * EXTERN_C_BEGIN and EXTERN_C_END should surround any Ports0 prototypes in
 * ports0.h or Ports0 .h files that are included by ports0.h.  This will
 * allow C++ codes to include ports0.h and properly link against the
 * Ports0 library.
 */
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#include <ports0_config.h>
#include <ports0_config_md.h>

#ifdef BUILD_USING_MACROS
#  ifndef USE_MACROS
#    define USE_MACROS
#  endif
#endif

#ifdef BUILD_LITE
#undef PORTS0_STDIO_NOT_REENTRANT
#undef PORTS0_MALLOC_NOT_REENTRANT
#undef PORTS0_FILEIO_NOT_REENTRANT
#endif /* BUILD_LITE */


/*********************************************************************
 * Various machine specific configuration, typedefs, etc.
 * This allows us to assume some basic amount of consistency.
 */

#ifndef MAX_PATH_LENGTH
#define MAX_PATH_LENGTH 1024
#endif /* MAX_PATH_LENGTH */

#ifdef _ALL_SOURCE
#include <standards.h>
#endif

#ifndef PORTS0_DONT_INCLUDE_STRING
#include <string.h>
#endif
#ifdef PORTS0_INCLUDE_STRINGS
#include <strings.h>
#endif

#ifndef PORTS0_DONT_INCLUDE_ANSI

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#else  /* !PORTS0_DONT_INCLUDE_ANSI */

#ifndef atof
extern double atof(void);
#endif /* atof */

EXTERN_C_BEGIN
extern void free(void);
extern void *malloc(void);
extern void *realloc(void);
EXTERN_C_END

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#endif /* !PORTS0_DONT_INCLUDE_ANSI */

typedef int		ports0_bool_t;

#if defined(TARGET_ARCH_SUNOS41) && defined(__GNUC__) && !defined(PORTS0_DONT_FIX_PROTOTYPES)
EXTERN_C_BEGIN
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
extern int socket(int domain, int type, int protocol);
extern int bind(int s, struct sockaddr *name, int namelen);
extern int connect(int s, struct sockaddr *name, int namelen);
extern int accept(int s, struct sockaddr *addr, int *addrlen);
extern int listen(int s, int backlog);
extern int getsockname(int s, struct sockaddr *name, int *namelen);
extern int getpeername(int s, struct sockaddr *name, int *namelen);
extern int getsockopt(int s, int level, int optname, char *optval, int *optlen);
extern int setsockopt(int s, int level, int optname, char *optval, int optlen);

extern int select (int width, 
		   fd_set *readfds, 
		   fd_set *writefds, 
		   fd_set *exceptfds, 
		   struct timeval *timeout);
extern int send(int s, const char *msg, int len, int flags);
extern int recv(int s, char *buf, int len, int flags);

#ifndef __USE_FIXED_PROTOTYPES__
extern int fclose(FILE *stream);
#endif

extern int getdtablesize(void);
EXTERN_C_END
#endif /* TARGET_ARCH_SUNOS41 */

#ifdef TARGET_ARCH_SYSV
#include <stdio.h>
extern FILE *fdopen(void);
#endif /* TARGET_ARCH_SYSV */

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

/*
 * Some procedure defines to provide for portability...
 */
#ifdef PORTS0_STRCHR_TO_INDEX
#define strchr(S,C)     index(S,C)
#define strrchr(S,C)    rindex(S,C)
#endif

#ifdef PORTS0_MEMCPY_TO_BCOPY
#define memcpy(Dest,Src,Length)		bcopy(Src,Dest,Length)
#endif

#ifndef HAVE_MEMMOVE
#define memmove(Dest,Src,Length) \
{ \
    char *__Src = ((char *) (Src)); \
    char *__Dest = ((char *) (Dest)); \
    char *__EndSrc = (__Src + (Length)); \
    char *__EndDest = (__Dest + (Length)); \
    if ( (__Src < __Dest) && (__Dest < __EndSrc) ) \
    { \
	/* Overlapping, with __Dest > __Src, so copy from end */ \
	while (__EndSrc > __Src) \
	{ \
	    *(--__EndDest) = *(--__EndSrc); \
	} \
    } \
    else if ( (__Dest < __Src) && (__Src < __EndDest) ) \
    { \
	/* Overlapping, with __Dest < __Src, so copy from beginning */ \
	while (__Src < __EndSrc) \
	{ \
	    *(__Dest++) = *(__Src++); \
	} \
    } \
    else if ( __Src != __Dest ) \
    { \
	/* Non-overlapping (and not the same), so memcpy() can be trusted */ \
	memcpy(Dest,Src,Length); \
    } \
}
#endif /* HAVE_MEMMOVE */

#ifdef PORTS0_USE_BZERO
#define ZeroOutMemory(Where,Size)	bzero(Where,Size)
#else
#define ZeroOutMemory(Where,Size)	memset(Where,0,Size)
#endif

EXTERN_C_BEGIN
#if defined(HAVE_GETCWD) && !defined(HAVE_GETWD)
extern char *getcwd(char *, size_t);
#define getwd(S) getcwd(S,MAX_PATH_LENGTH)
#else
extern char *getwd( char * );
#endif
EXTERN_C_END


/*********************************************************************
 * Some ports0 definitions...
 */

EXTERN_C_BEGIN

#define	PORTS0_TRUE		1
#define	PORTS0_FALSE		0

#define PORTS0_MAX(V1,V2) (((V1) > (V2)) ? (V1) : (V2))
#define PORTS0_MIN(V1,V2) (((V1) < (V2)) ? (V1) : (V2))

#ifdef PORTS0_DEFINE_GLOBALS
#define PORTS0_GLOBAL
#else
#define PORTS0_GLOBAL extern
#endif


#ifdef BUILD_DEBUG
#define Ports0Assert(assertion) \
    if (!(assertion)) \
    { \
        ports0_fatal("Assertion " #assertion " failed in file %s at line %d\n",\
		   __FILE__, __LINE__); \
    }
#define Ports0Assert2(assertion, print_args) \
    if (!(assertion)) \
    { \
        ports0_fatal("Assertion " #assertion " failed in file %s at line %d: %s", \
		   __FILE__, __LINE__, ports0_assert_sprintf print_args); \
    }
#else /* BUILD_DEBUG */
#define Ports0Assert(assertion)
#define Ports0Assert2(assertion, print_args)
#endif /* BUILD_DEBUG */


/*
 * Some typedefs...
 */
typedef void *(*ports0_thread_func_t)(void *user_arg);

EXTERN_C_END


#ifndef BUILD_LITE
/*********************************************************************
 * Sanity checking inspects the data structures and
 * tries to determine if corruption has occurred
 * before the system would crash.  Threads then
 * can be "imprisoned" so that they may be inspected.
 * Important since multithreaded applications can
 * be non-deterministic.
 * sanity.h is placed into ports0.h because the macros
 * that are here should be available to the user,
 * since they are called from macros that the user
 * calls.
 * Sanity checking is turned on/off by defining
 * PORTS0_SANITY_CHECK
 * JMP 02/28/94
 */

EXTERN_C_BEGIN

typedef enum _p0_structure_types_enum {
  _P0_THREAD_T,
  _P0_MUTEX_T,
  _P0_COND_T,
  _P0_THREAD_FREELIST_T
} _p0_structure_types;

#define _P0_START_COOKIE_VAL 0x6666
#define _P0_END_COOKIE_VAL   0xbbbb

#ifdef PORTS0_SANITY_CHECK

#define PORTS0_INTERROGATE(mut, code, caller) \
	_p0_interrogate_suspect((void*)mut, code, caller);
#define _P0_START_MAGIC_COOKIE unsigned int _p0_start_magic_cookie;
#define _P0_END_MAGIC_COOKIE   unsigned int _p0_end_magic_cookie;
#define _P0_INIT_START_MAGIC_COOKIE(v1) \
         do {(v1)->_p0_start_magic_cookie=_P0_START_COOKIE_VAL;} while(0)
#define _P0_INIT_END_MAGIC_COOKIE(v1) \
         do {(v1)->_p0_end_magic_cookie=_P0_END_COOKIE_VAL;} while(0)
#define _P0_CHECK_START_MAGIC_COOKIE(tar) \
	((tar)->_p0_start_magic_cookie==_P0_START_COOKIE_VAL)
#define _P0_CHECK_END_MAGIC_COOKIE(tar) \
	((tar)->_p0_end_magic_cookie==_P0_END_COOKIE_VAL)
#define _P0_EXTRACT_START_MAGIC_COOKIE(m) (m)->_p0_start_magic_cookie
#define _P0_EXTRACT_END_MAGIC_COOKIE(m) (m)->_p0_end_magic_cookie

#else

#define PORTS0_INTERROGATE(mut, code, caller) do { int i; i=3; } while(0)
#define _P0_START_MAGIC_COOKIE
#define _P0_END_MAGIC_COOKIE
#define _P0_INIT_START_MAGIC_COOKIE(v1) \
        do { int i; i=3; } while(0)
#define _P0_INIT_END_MAGIC_COOKIE(v1) \
        do { int i; i=3; } while(0)
#define _P0_CHECK_START_MAGIC_COOKIE(tar) 0
#define _P0_CHECK_END_MAGIC_COOKIE(tar) 0
#define _P0_EXTRACT_START_MAGIC_COOKIE(m) 0
#define _P0_EXTRACT_END_MAGIC_COOKIE(m) 0

#endif /* PORTS0_SANITY_CHECK */

void _p0_imprison_thread(char *);
void _p0_interrogate_suspect(void *evidence,
			     _p0_structure_types suspect_type,
			     char *caller);

EXTERN_C_END
#endif /* BUILD_LITE */


#ifndef BUILD_LITE

#define p0_cat_aux(x, y) x ## y
#define p0_cat(x, y) p0_cat_aux(x, y)
#define p0_quoter_aux(x) #x
#define p0_quoter(x) p0_quoter_aux(x)

/*********************************************************************
 * Include the thread module's external header, which should contain:
 *   1) #include's for system thread headers
 *   2) typedefs for external ports0 types
 *   3) macros for external ports0 routines
 */
/*
 * #ifndef PORTS0_TH_HEADER
 * #define PORTS0_TH_HEADER "p0_th.h"
 * #endif
 */

#define PORTS0_TH_HEADER p0_quoter(p0_cat(TH_MODULE,.h))

#include PORTS0_TH_HEADER

#endif /* BUILD_LITE */


/*********************************************************************
 * 		Exported interface declarations
 */

EXTERN_C_BEGIN

extern void     ports0_init (int *argc,
			     char **argv[],
			     char *package_id);

extern int	ports0_shutdown(void);


/*
 * Reentrant lock
 */
#ifdef BUILD_LITE

#define ports0_macro_reentrant_lock() (0)
#define ports0_macro_reentrant_unlock() (0)

#else  /* BUILD_LITE */

PORTS0_GLOBAL ports0_mutex_t ports0_reentrant_mutex;
#define ports0_macro_reentrant_lock() \
    ports0_mutex_lock(&ports0_reentrant_mutex)
#define ports0_macro_reentrant_unlock() \
    ports0_mutex_unlock(&ports0_reentrant_mutex)

#endif /* BUILD_LITE */

#ifdef USE_MACROS
#define ports0_reentrant_lock()   ports0_macro_reentrant_lock()
#define ports0_reentrant_unlock() ports0_macro_reentrant_unlock()
#else  /* USE_MACROS */
extern int ports0_reentrant_lock(void);
extern int ports0_reentrant_unlock(void);
#endif /* USE_MACROS */



#ifdef PORTS0_STDIO_NOT_REENTRANT
#define ports0_stdio_lock() ports0_reentrant_lock()
#define ports0_stdio_unlock() ports0_reentrant_unlock()
#else
#define ports0_stdio_lock()
#define ports0_stdio_unlock()
#endif


/*
 * Malloc #defines.
 * Must handle three cases:
 *    1) PORTS0_MALLOC_DEBUG: Call ports0_debug_*() routines.
 *    2) PORTS0_MALLOC_NOT_REENTRANT: Call ports0_reentrant_*() routines.
 *    3) Else: Call underlying memory allocation routines.
 */
#ifdef PORTS0_MALLOC_DEBUG

/* This doesn't work yet */
extern void *		ports0_debug_malloc(int bytes, char *file, int line);
extern void		ports0_debug_malloc_check(char *file, int line);
extern void		ports0_debug_show_freed_blocks(void);
extern void		ports0_debug_show_malloc_stats(void);
extern void		ports0_debug_free(void *ptr, char *file, int line);
extern ports0_bool_t		ports0_malloc_debug_level(void);

#define ports0_macro_malloc(bytes) \
    ports0_debug_malloc(bytes,__FILE__,__LINE__)
#define ports0_macro_realloc(ptr,bytes) \
    ports0_debug_realloc(ptr,bytes,__FILE__,__LINE__)
#define ports0_macro_calloc(nobj,bytes) \
    ports0_debug_calloc(nobj,bytes,__FILE__,__LINE__)
#define ports0_macro_free(ptr) \
    ports0_debug_free(ptr,__FILE__,__LINE__)

#else /* PORTS0_MALLOC_DEBUG */

#ifdef PORTS0_MALLOC_NOT_REENTRANT

#define ports0_macro_malloc(bytes) \
    ports0_reentrant_malloc(bytes)
#define ports0_macro_realloc(ptr,bytes) \
    ports0_reentrant_realloc(ptr,bytes)
#define ports0_macro_calloc(nobj,bytes) \
    ports0_reentrant_calloc(nobj,bytes)
#define ports0_macro_free(ptr) \
    ports0_reentrant_free(ptr)

#else  /* PORTS0_MALLOC_NOT_REENTRANT */

#define ports0_macro_malloc(bytes) \
    malloc(bytes)
#define ports0_macro_realloc(ptr,bytes) \
    realloc(ptr,bytes)
#define ports0_macro_calloc(nobj,bytes) \
    calloc(nobj,bytes)
#define ports0_macro_free(ptr) \
    free(ptr)

#endif /* PORTS0_MALLOC_NOT_REENTRANT */

#endif /* PORTS0_MALLOC_DEBUG */

/*
 * Memory allocation routines
 */
#ifndef USE_MACROS
extern void *ports0_malloc(size_t bytes);
extern void *ports0_realloc(void *ptr,
			    size_t bytes);
extern void *ports0_calloc(size_t nobj, 
			   size_t bytes);
extern void ports0_free(void *ptr);
#else /* USE_MACROS */
#define ports0_malloc(bytes) \
    ports0_macro_malloc(bytes)
#define ports0_realloc(ptr,bytes) \
    ports0_macro_realloc(ptr,bytes)
#define ports0_calloc(nobj,bytes) \
    ports0_macro_calloc(nobj,bytes)
#define ports0_free(data_segment) \
    ports0_macro_free(data_segment)
#endif /* USE_MACROS */


#define Ports0Malloc(Func, Var, Type, Size) \
{ \
    if ((Size) > 0) \
    { \
	if (((Var) = (Type) ports0_macro_malloc (Size)) == (Type) NULL) \
	{ \
	    ports0_fatal("%s: malloc of size %d failed for %s %s in file %s line %d\n", \
                        #Func, (Size), #Type, #Var, __FILE__, __LINE__); \
	} \
    } \
    else \
    { \
	(Var) = (Type) NULL; \
    } \
}

#define Ports0Free(Ptr) \
{ \
    if ((Ptr) != NULL) \
    { \
	ports0_macro_free(Ptr); \
    } \
}

/*
 * File I/O routines
 */
#ifdef PORTS0_FILEIO_NOT_REENTRANT

#define ports0_macro_open(path, flags, mode) \
    ports0_reentrant_open(path, flags, mode)
#define ports0_macro_close(fd) \
    ports0_reentrant_close(fd)
#define ports0_macro_read(fd, buf, nbytes) \
    ports0_reentrant_read(fd, buf, nbytes)
#define ports0_macro_write(fd, buf, nbytes) \
    ports0_reentrant_write(fd, buf, nbytes)
#define ports0_macro_fstat(fd, buf) \
    ports0_reentrant_fstat(fd, buf)
#define ports0_macro_lseek(fd, offset, whence) \
    ports0_reentrant_lseek(fd, offset, whence)

#else  /* PORTS0_FILEIO_NOT_REENTRANT */

#define ports0_macro_open(path, flags, mode) \
        open(path, flags, mode)
#define ports0_macro_close(fd) \
        close(fd)
#define ports0_macro_read(fd, buf, nbytes) \
        read(fd, buf, nbytes)
#define ports0_macro_write(fd, buf, nbytes) \
        write(fd, buf, nbytes)
#define ports0_macro_fstat(fd, buf) \
        fstat(fd, buf)
#define ports0_macro_lseek(fd, offset, whence) \
        lseek(fd, offset, whence)

#endif /* PORTS0_FILEIO_NOT_REENTRANT */

#ifndef USE_MACROS
extern int ports0_open(char *path,
		       int flags,
		       int mode);
extern int ports0_close(int fd);
extern int ports0_read(int fd,
		       char *buf,
		       int nbytes);
extern int ports0_write(int fd,
			char *buf,
			int nbytes);
extern int ports0_fstat(int fd,
			struct stat *buf);
extern int ports0_lseek(int fd,
			off_t offset,
			int whence);
#else  /* USE_MACROS */
#define ports0_open(path, flags, mode) \
    ports0_macro_open(path, flags, mode)
#define ports0_close(fd) \
    ports0_macro_close(fd)
#define ports0_read(fd, buf, nbytes) \
    ports0_macro_read(fd, buf, nbytes)
#define ports0_write(fd, buf, nbytes) \
    ports0_macro_write(fd, buf, nbytes)
#define ports0_fstat(fd, buf) \
    ports0_macro_fstat(fd, buf)
#define ports0_lseek(fd, offset, whence) \
    ports0_macro_lseek(fd, offset, whence)
#endif /* USE_MACROS */


/*********************************************************************
 * Other exported interface declarations that are not
 * officially part of the ports0 interface.
 */

/*
 * args.c
 */
extern int		ports0_set_package_id(char *package_id_base);
extern char *		ports0_get_package_id(void);
extern int		ports0_find_argument(int *argc,
					     char **argv[],
					     char *arg,
					     int count);
extern void		ports0_remove_arguments(int *argc,
						char **argv[],
						int arg_num,
						int count);
extern void		ports0_usage_message(void);
#ifndef BUILD_LITE
extern int		ports0_new_process_params(char *buf, int size);
#endif /* BUILD_LITE */

/*
 * error.c
 */
extern void		ports0_silent_fatal(void);
#ifdef __STDC__
extern void		ports0_fatal(char *msg, ...);
extern void		ports0_error(char *msg, ...);
extern void		ports0_warning(char *msg, ...);
extern void		ports0_notice(char *msg, ...);
extern void		ports0_printf(char *msg, ...);
extern void		ports0_perror(char *msg, ...);
extern void		ports0_fatal_perror(char *msg, ...);
extern char *		ports0_assert_sprintf(char *msg, ...);
#else /* __STDC__ */
/*
 * These functions have a variable number of parameters, so they must be
 * declared this way so the non-standard compilers work with them.
 */
extern void		ports0_fatal();
extern void		ports0_error();
extern void		ports0_warning();
extern void		ports0_notice();
extern void		ports0_printf();
extern void		ports0_perror();
extern void		ports0_fatal_perror();
extern char *		ports0_assert_sprintf();
#endif /* __STDC__ */

/*
 * init.c
 */
extern void		ports0_preinit(void);

/*
 * th_*.c
 */
#ifndef BUILD_LITE
extern ports0_bool_t	ports0_preemptive_threads(void);
extern void		ports0_idle_callback(void (*idle_func)(void));
extern void             ports0_thread_prefork(void);
extern void             ports0_thread_postfork(void);
#endif /* BUILD_LITE */

/*
 * trace.c
 */
extern void		ports0_traceback(char *output_prefix);

EXTERN_C_END

#endif /* _PORTS0_H */
