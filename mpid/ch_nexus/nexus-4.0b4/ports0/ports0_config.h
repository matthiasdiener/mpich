/* ports0_config.h.  Generated automatically by configure.  */
/* ports0_config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

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
/* #undef HAVE_PTHREAD_PREEMPTIVE */

/* #undef HAVE_LWP */
/* #undef HAVE_SPROC */
/* #undef _SGI_MP_SOURCE */
/* #undef P0_QT_MULTIPROCESSOR */
#define TH_MODULE p0_th_null
/* #undef _REENTRANT */
/* #undef _CMA_REENTRANT_CLIB_ */
/* #undef __USE_FIXED_PROTOTYPES__ */
/* #undef HAVE_THREAD_SAFE_STDIO */
/* #undef HAVE_THREAD_SAFE_SELECT */

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

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getwd function.  */
#define HAVE_GETWD 1

/* Define if you have the memmove function.  */
/* #undef HAVE_MEMMOVE */

/* Define if you have the sproc function.  */
/* #undef HAVE_SPROC */

/* Define if you have the <libc.h> header file.  */
/* #undef HAVE_LIBC_H */

/* Define if you have the <limits.h> header file.  */
/* #undef HAVE_LIMITS_H */

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H 1

/* Define if you have the <sys/access.h> header file.  */
/* #undef HAVE_SYS_ACCESS_H */

/* Define if you have the <sys/errno.h> header file.  */
#define HAVE_SYS_ERRNO_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/lwp.h> header file.  */
/* #undef HAVE_SYS_LWP_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/sysmp.h> header file.  */
/* #undef HAVE_SYS_SYSMP_H */

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <ulocks.h> header file.  */
/* #undef HAVE_ULOCKS_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1
