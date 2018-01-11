/* nexus_config.h.  Generated automatically by configure.  */
/* nexus_config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if you have alloca, as a function or macro.  */
/* #undef HAVE_ALLOCA */

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your system has its own `getloadavg' function.  */
/* #undef HAVE_GETLOADAVG */

/* Define if you have the getmntent function.  */
/* #undef HAVE_GETMNTENT */

/* Define if the `long double' type works.  */
/* #undef HAVE_LONG_DOUBLE */

/* Define if you support file names longer than 14 characters.  */
/* #undef HAVE_LONG_FILE_NAMES */

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if system calls automatically restart after interruption
   by a signal.  */
/* #undef HAVE_RESTARTABLE_SYSCALLS */

/* Define if your struct stat has st_blksize.  */
/* #undef HAVE_ST_BLKSIZE */

/* Define if your struct stat has st_blocks.  */
/* #undef HAVE_ST_BLOCKS */

/* Define if you have the strcoll function and it is properly defined.  */
/* #undef HAVE_STRCOLL */

/* Define if your struct stat has st_rdev.  */
/* #undef HAVE_ST_RDEV */

/* Define if you have the strftime function.  */
/* #undef HAVE_STRFTIME */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define if you have <unistd.h>.  */
#define HAVE_UNISTD_H 1

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
/* #undef HAVE_UTIME_NULL */

/* Define if you have <vfork.h>.  */
/* #undef HAVE_VFORK_H */

/* Define if you have the vprintf function.  */
/* #undef HAVE_VPRINTF */

/* Define if you have the wait3 system call.  */
#define HAVE_WAIT3 1

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Application specific options */
/* #undef PORTS0_MALLOC_DEBUG */
/* #undef PORTS0_MALLOC_PAD */
#define BUILD_USING_MACROS 1
#define BUILD_LITE 1
/* #undef BUILD_DEBUG */
/* #undef BUILD_PROFILE */

#define NEXUS_ALIGN 8

/* #undef HAVE_SOLARISTHREADS */
/* #undef HAVE_QUICKTHREADS */
/* #undef HAVE_CTHREADS */
/* #undef HAVE_PTHREAD */
/* #undef HAVE_PTHREAD_DRAFT_4 */
/* #undef HAVE_PTHREAD_DRAFT_6 */
/* #undef HAVE_PTHREAD_DRAFT_8 */
/* #undef HAVE_PTHREAD_DRAFT_10 */
/* #undef HAVE_PTHREAD_SCHED */
/* #undef HAVE_PTHREAD_INIT_FUNC */

/* #undef HAVE_LWP */
/* #undef HAVE_SPROC */
/* #undef _SGI_MP_SOURCE */
/* #undef P0_QT_MULTIPROCESSOR */
/* #undef _REENTRANT */
/* #undef _CMA_REENTRANT_CLIB_ */
/* #undef __USE_FIXED_PROTOTYPES__ */
/* #undef HAVE_THREAD_SAFE_STDIO */
/* #undef HAVE_THREAD_SAFE_SELECT */

/* #undef HAVE_IBMDS */
/* #undef HAVE_INX */
/* #undef HAVE_MPL */
#define HAVE_POSIX_SIGNALS 1
#define HAVE_RSH 1
/* #undef HAVE_SS */
/* #undef HAVE_SSL */
#define HAVE_TCP 1
/* #undef HAVE_X_ACC */
#define HAVE_X_OK 1
/* #undef NEXUS_CRITICAL_PATH_TIMER */
/* #undef NEXUS_DONT_CATCH_SIGSEGV */
#define NEXUS_FD_SET_CAST (fd_set *)
#define NEXUS_LISTENER "NONE/bin/listener"
#define NEXUS_RSH "/usr/ucb/rsh"
/* #undef NEXUS_LISTENER_EXEC_HACK */
#define USE_FCNTL 1
#define USE_FLOCK 1
#define USE_LOCKF 1

#define TARGET_ARCH_SUNOS41 1
/* #undef TARGET_ARCH_SOLARIS */
/* #undef TARGET_ARCH_RS6000 */
/* #undef TARGET_ARCH_AIX */
/* #undef TARGET_ARCH_MPL */
/* #undef TARGET_ARCH_AIX3 */
/* #undef TARGET_ARCH_AIX4 */
/* #undef TARGET_ARCH_HP9000S700 */
/* #undef TARGET_ARCH_HPUX */
/* #undef TARGET_ARCH_PARAGON */
/* #undef TARGET_ARCH_SGI */
/* #undef TARGET_ARCH_IRIX */
/* #undef TARGET_ARCH_IRIX5 */
/* #undef TARGET_ARCH_IRIX6 */
/* #undef TARGET_ARCH_AXP */
/* #undef TARGET_ARCH_OSF1 */
/* #undef TARGET_ARCH_CRAYC90 */
/* #undef TARGET_ARCH_UNICOS */	
/* #undef TARGET_ARCH_FREEBSD */
/* #undef TARGET_ARCH_NEXTSTEP */

/* The number of bytes in a int.  */
/* #undef SIZEOF_INT */

/* The number of bytes in a int *.  */
/* #undef SIZEOF_INT_P */

/* Define if you have the accept function.  */
#define HAVE_ACCEPT 1

/* Define if you have the bind function.  */
#define HAVE_BIND 1

/* Define if you have the connect function.  */
#define HAVE_CONNECT 1

/* Define if you have the fork function.  */
#define HAVE_FORK 1

/* Define if you have the fork1 function.  */
/* #undef HAVE_FORK1 */

/* Define if you have the getsockname function.  */
#define HAVE_GETSOCKNAME 1

/* Define if you have the listen function.  */
#define HAVE_LISTEN 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the sigaction function.  */
#define HAVE_SIGACTION 1

/* Define if you have the sigaddset function.  */
#define HAVE_SIGADDSET 1

/* Define if you have the sigblock function.  */
#define HAVE_SIGBLOCK 1

/* Define if you have the sigemptyset function.  */
#define HAVE_SIGEMPTYSET 1

/* Define if you have the sighold function.  */
/* #undef HAVE_SIGHOLD */

/* Define if you have the sigprocmask function.  */
#define HAVE_SIGPROCMASK 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the sproc function.  */
/* #undef HAVE_SPROC */

/* Define if you have the strtoul function.  */
/* #undef HAVE_STRTOUL */

/* Define if you have the waitpid function.  */
#define HAVE_WAITPID 1

/* Define if you have the <dce/cma_ux.h> header file.  */
/* #undef HAVE_DCE_CMA_UX_H */

/* Define if you have the <libc.h> header file.  */
/* #undef HAVE_LIBC_H */

/* Define if you have the <limits.h> header file.  */
/* #undef HAVE_LIMITS_H */

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H 1

/* Define if you have the <sys/access.h> header file.  */
/* #undef HAVE_SYS_ACCESS_H */

/* Define if you have the <sys/errno.h> header file.  */
#define HAVE_SYS_ERRNO_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/select.h> header file.  */
/* #undef HAVE_SYS_SELECT_H */

/* Define if you have the <sys/signal.h> header file.  */
#define HAVE_SYS_SIGNAL_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/uio.h> header file.  */
#define HAVE_SYS_UIO_H 1

/* Define if you have the <ulocks.h> header file.  */
/* #undef HAVE_ULOCKS_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1
