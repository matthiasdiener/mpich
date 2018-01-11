s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MPI_KEEP_SEND_QUEUE\([ 	][ 	]*\)[^ 	]*@\1#\2MPI_KEEP_SEND_QUEUE\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPI_KEEP_SEND_QUEUE\([ 	]\)@\1#\2define\3MPI_KEEP_SEND_QUEUE 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPI_KEEP_SEND_QUEUE$@\1#\2define\3MPI_KEEP_SEND_QUEUE 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MPE_USE_EXTENSIONS\([ 	][ 	]*\)[^ 	]*@\1#\2MPE_USE_EXTENSIONS\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPE_USE_EXTENSIONS\([ 	]\)@\1#\2define\3MPE_USE_EXTENSIONS 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPE_USE_EXTENSIONS$@\1#\2define\3MPE_USE_EXTENSIONS 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_PROTOTYPES\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_PROTOTYPES\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_PROTOTYPES\([ 	]\)@\1#\2define\3HAVE_PROTOTYPES 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_PROTOTYPES$@\1#\2define\3HAVE_PROTOTYPES 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_GETTIMEOFDAY\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_GETTIMEOFDAY\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GETTIMEOFDAY\([ 	]\)@\1#\2define\3HAVE_GETTIMEOFDAY 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GETTIMEOFDAY$@\1#\2define\3HAVE_GETTIMEOFDAY 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAS_XDR\([ 	][ 	]*\)[^ 	]*@\1#\2HAS_XDR\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAS_XDR\([ 	]\)@\1#\2define\3HAS_XDR 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAS_XDR$@\1#\2define\3HAS_XDR 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_UNAME\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_UNAME\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_UNAME\([ 	]\)@\1#\2define\3HAVE_UNAME 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_UNAME$@\1#\2define\3HAVE_UNAME 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_GETHOSTBYNAME\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_GETHOSTBYNAME\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GETHOSTBYNAME\([ 	]\)@\1#\2define\3HAVE_GETHOSTBYNAME 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GETHOSTBYNAME$@\1#\2define\3HAVE_GETHOSTBYNAME 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_GENCAT\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_GENCAT\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GENCAT\([ 	]\)@\1#\2define\3HAVE_GENCAT 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_GENCAT$@\1#\2define\3HAVE_GENCAT 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STDLIB_H\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_STDLIB_H\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STDLIB_H\([ 	]\)@\1#\2define\3HAVE_STDLIB_H 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STDLIB_H$@\1#\2define\3HAVE_STDLIB_H 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_UNISTD_H\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_UNISTD_H\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_UNISTD_H\([ 	]\)@\1#\2define\3HAVE_UNISTD_H 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_UNISTD_H$@\1#\2define\3HAVE_UNISTD_H 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STDARG_H\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_STDARG_H\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STDARG_H\([ 	]\)@\1#\2define\3HAVE_STDARG_H 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STDARG_H$@\1#\2define\3HAVE_STDARG_H 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)USE_STDARG\([ 	][ 	]*\)[^ 	]*@\1#\2USE_STDARG\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)USE_STDARG\([ 	]\)@\1#\2define\3USE_STDARG 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)USE_STDARG$@\1#\2define\3USE_STDARG 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MALLOC_RET_VOID\([ 	][ 	]*\)[^ 	]*@\1#\2MALLOC_RET_VOID\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MALLOC_RET_VOID\([ 	]\)@\1#\2define\3MALLOC_RET_VOID 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MALLOC_RET_VOID$@\1#\2define\3MALLOC_RET_VOID 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SYSTEM\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_SYSTEM\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SYSTEM\([ 	]\)@\1#\2define\3HAVE_SYSTEM 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SYSTEM$@\1#\2define\3HAVE_SYSTEM 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_NICE\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_NICE\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_NICE\([ 	]\)@\1#\2define\3HAVE_NICE 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_NICE$@\1#\2define\3HAVE_NICE 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_MEMORY_H\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_MEMORY_H\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_MEMORY_H\([ 	]\)@\1#\2define\3HAVE_MEMORY_H 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_MEMORY_H$@\1#\2define\3HAVE_MEMORY_H 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STRING_H\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_STRING_H\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STRING_H\([ 	]\)@\1#\2define\3HAVE_STRING_H 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STRING_H$@\1#\2define\3HAVE_STRING_H 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_LONG_DOUBLE\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_LONG_DOUBLE\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_LONG_DOUBLE\([ 	]\)@\1#\2define\3HAVE_LONG_DOUBLE 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_LONG_DOUBLE$@\1#\2define\3HAVE_LONG_DOUBLE 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_LONG_LONG_INT\([ 	][ 	]*\)[^ 	]*@\1#\2HAVE_LONG_LONG_INT\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_LONG_LONG_INT\([ 	]\)@\1#\2define\3HAVE_LONG_LONG_INT 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_LONG_LONG_INT$@\1#\2define\3HAVE_LONG_LONG_INT 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MPI_ADI2\([ 	][ 	]*\)[^ 	]*@\1#\2MPI_ADI2\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPI_ADI2\([ 	]\)@\1#\2define\3MPI_ADI2 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPI_ADI2$@\1#\2define\3MPI_ADI2 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MPIR_MEMDEBUG\([ 	][ 	]*\)[^ 	]*@\1#\2MPIR_MEMDEBUG\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPIR_MEMDEBUG\([ 	]\)@\1#\2define\3MPIR_MEMDEBUG 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPIR_MEMDEBUG$@\1#\2define\3MPIR_MEMDEBUG 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)MPIR_OBJDEBUG\([ 	][ 	]*\)[^ 	]*@\1#\2MPIR_OBJDEBUG\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPIR_OBJDEBUG\([ 	]\)@\1#\2define\3MPIR_OBJDEBUG 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)MPIR_OBJDEBUG$@\1#\2define\3MPIR_OBJDEBUG 1@g
s@^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)DEBUG_TRACE\([ 	][ 	]*\)[^ 	]*@\1#\2DEBUG_TRACE\31@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)DEBUG_TRACE\([ 	]\)@\1#\2define\3DEBUG_TRACE 1\4@g
s@^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)DEBUG_TRACE$@\1#\2define\3DEBUG_TRACE 1@g

s,^[ 	]*#[ 	]*undef[ 	][ 	]*[a-zA-Z_][a-zA-Z_0-9]*,/* & */,
