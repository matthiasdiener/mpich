/* The PRIMARY source of this file is acconfig.h */
/* These are needed for ANY declaration that may be made by an AC_DEFINE */

/* Define if Fortran functions are pointers to pointers */
#undef FORTRAN_SPECIAL_FUNCTION_PTR

/* Define is C supports volatile declaration */
#undef HAS_VOLATILE

/* Define if XDR libraries available */
#undef HAS_XDR

/* Define if message catalog programs available */
#undef HAVE_GENCAT

/* Define if getdomainname function available */
#undef HAVE_GETDOMAINNAME

/* Define in gethostbyname function available */
#undef HAVE_GETHOSTBYNAME

/* Define if C has long long int */
#undef HAVE_LONG_LONG_INT

/* Define if C supports long doubles */
#undef HAVE_LONG_DOUBLE 

/* Define if msem_init function available */
#undef HAVE_MSEM_INIT

/* Define if C does NOT support const */
#undef HAVE_NO_C_CONST

/* Define if C supports prototypes (but isn't ANSI C) */
#undef HAVE_PROTOTYPES

/* Define if uname function available */
#undef HAVE_UNAME

/* Define if an int is smaller than void * */
#undef INT_LT_POINTER

/* Define if malloc returns void * (and is an error to return char *) */
#undef MALLOC_RET_VOID

/* Define if MPE extensions are included in MPI libraries */
#undef MPE_USE_EXTENSIONS

/* Define if MPID contains special case code for collective over world */
#undef MPID_COLL_WORLD

/* Define if MPID supports ADI collective */
#undef MPID_USE_ADI_COLLECTIVE

/* Define is ADI should maintain a send queue for debugging */
#undef MPI_KEEP_SEND_QUEUE

/* Define if mpe debug features should NOT be included */
#undef MPI_NO_MPEDBG

/* Define if struct msemaphore rather than msemaphore required */
#undef MSEMAPHORE_IS_STRUCT

/* Define if void * is 8 bytes */
#undef POINTER_64_BITS

/* Define if stdarg can be used */
#undef USE_STDARG

/* For Cray, define two word character descriptors in use */
#undef _TWO_WORD_FCD

/* Define if extra traceback information should be kept */
#undef DEBUG_TRACE

/* Define if Fortran is NOT available */
#undef MPID_NO_FORTRAN

/* Define if memory debugging should be enabled */
#undef MPIR_MEMDEBUG

/* Define if object debugging should be enabled */
#undef MPIR_OBJDEBUG

/* Define if ADI is ADI-2 (required!) */
#undef MPI_ADI2


