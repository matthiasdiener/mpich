/* Define if void * is 8 bytes */
#undef POINTER_64_BITS

/* Define if uname function available */
#undef HAVE_UNAME

/* Define in gethostbyname function available */
#undef HAVE_GETHOSTBYNAME

/* define is stdarg.h is available */
#undef HAVE_STDARG_H

/* Define if stdarg can be used */
#undef USE_STDARG

/* Define if C supports prototypes (but isn't ANSI C) */
#undef HAVE_PROTOTYPES

/* Define if Fortran uses lowercase name mangling */
#undef F77_NAME_LOWER

/* Define if Fortran use lowercase followed by an underscore */
#undef F77_NAME_LOWER_USCORE

/* Define if Fortran uses uppercase */
#undef F77_NAME_UPPER

/* Define if Fortran uses two underscores for names with an underscore 
   (and one for names without an underscore) */
#undef F77_NAME_LOWER_2USCORE

/* Define if Fortran leaves case unchanged */
#undef F77_NAME_MIXED

/* Define if Fortran leaves case unchanged, followed by an underscore */
#undef F77_NAME_MIXED_USCORE

/* Define Fortran logical values used in MPI C program  */
#undef MPE_F77_TRUE_VALUE
#undef MPE_F77_FALSE_VALUE

/* Define if MPI_Wtime is there  */
#undef HAVE_MPI_WTIME

/* Define if MPI_Fint if necessary */
#undef MPI_Fint

/* Define MPI_STATUS_SIZE */
#undef MPI_STATUS_SIZE

/*  define MPI Fortran logical  */
#undef MPIR_F_TRUE
#undef MPIR_F_FALSE

/* Define if MPI_xxx_f2c and c2f routines defined */
#undef HAVE_MPI_XXXX_F2C

/* Define if MPI_Status_f2c and c2f rotines defined */
#undef HAVE_MPI_STATUS_C2F

/* Define in MPI_RECV etc. does not set status on MPI_PROC_NULL */
#undef HAVE_BROKEN_STATUS_ON_PROC_NULL

/* Define if sighandler_t defined in signal.h */
#undef HAVE_SIGHANDLER_T
