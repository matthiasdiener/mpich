dnl Check to make sure that compiler runs
dnl AC_CHECK_COMPILER_OK(compiler, true-action, false-action)
AC_DEFUN(AC_CHECK_COMPILER_OK,[
AC_MSG_CHECKING(that the compiler $CC runs)
AC_TRY_COMPILE(,[int main(){ exit(0);}],
  eval "ac_cv_ccworks=yes",
  eval "ac_cv_ccworks=no")
if test "$ac_cv_ccworks" = yes; then
  AC_MSG_RESULT(yes)
  $1
else
  AC_MSG_RESULT(no)
  $2
fi
])

dnl Check that the compiler accepts ANSI prototypes.
dnl AC_CHECK_CC_PROTOTYPES(true-action, false-action)
dnl
AC_DEFUN(AC_CHECK_CC_PROTOTYPES,[
AC_MSG_CHECKING(that the compiler $CC accepts ANSI prototypes)
AC_TRY_COMPILE([int f(double a){return 0;}],,
  eval "ac_cv_ccworks=yes",
  eval "ac_cv_ccworks=no")
if test "$ac_cv_ccworks" = yes; then
  AC_MSG_RESULT(yes)
  $1
else
  AC_MSG_RESULT(no)
  $2
fi
])

dnl AC_PROG_ANSI_CC(...) is like AC_PROG_CC, but it checks that CC
dnl accepts ANSI-style prototypes.  If not, it then tries to use gcc
dnl
AC_DEFUN(AC_PROG_ANSI_CC,[
AC_CHECK_PROGS(CC, $CC xlc cc gcc)
AC_PROG_CC
AC_CHECK_CC_PROTOTYPES(,noproto=1)
if test -n "$noproto" ; then
    AC_MSG_ERROR(The compiler $CC does not accept ANSI prototypes)
fi
])

dnl AC_LANG_TARGET_C()
AC_DEFUN(AC_LANG_TARGET_C,
[define([AC_LANG], [C])dnl
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$TARGET_CPP $TARGET_CPPFLAGS'
ac_compile='${TARGET_CC-cc} -c $TARGET_CFLAGS $TARGET_CPPFLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${TARGET_CC-cc} -o conftest $TARGET_CFLAGS $TARGET_CPPFLAGS $TARGET_LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
])

dnl AC_LANG_THREADSAFE_TARGET_C()
AC_DEFUN(AC_LANG_THREADSAFE_TARGET_C,
[define([AC_LANG], [C])dnl
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$THREADSAFE_TARGET_CPP $THREADSAFE_TARGET_CPPFLAGS'
ac_compile='${THREADSAFE_TARGET_CC-cc} -c $THREADSAFE_TARGET_CFLAGS $THREADSAFE_TARGET_CPPFLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${CC-cc} -o conftest $THREADSAFE_TARGET_CFLAGS $THREADSAFE_TARGET_CPPFLAGS $THREADSAFE_TARGET_LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
])

dnl Identify the target C compiler.  This will be different in the case
dnl of cross compilation or things like the SP where we use a different
dnl version of the basic compiler.
AC_DEFUN(AC_PROG_ANSI_TARGET_CC,[
AC_REQUIRE([AC_PROG_ANSI_CC])
TARGET_CC=$CC
AC_SUBST(TARGET_CC)
])

AC_DEFUN(AC_PROG_TARGET_CPP,
[AC_MSG_CHECKING(how to run the C preprocessor)
if test -z "$TARGET_CPP"; then
AC_CACHE_VAL(ac_cv_prog_TARGET_CPP,
[  # This must be in double quotes, not single quotes, because CPP may get
  # substituted into the Makefile and "${TARGET_CC-cc}" will confuse make.
  TARGET_CPP="${TARGET_CC-cc} -E"
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp.
dnl Use a header file that comes with gcc, so configuring glibc
dnl with a fresh cross-compiler works.
  AC_TRY_CPP([#include <assert.h>
Syntax Error], ,
  TARGET_CPP="${TARGET_CC-cc} -E -traditional-cpp"
  AC_TRY_CPP([#include <assert.h>
Syntax Error], , TARGET_CPP=/lib/cpp))
  ac_cv_prog_TARGET_CPP="$TARGET_CPP"])dnl
  TARGET_CPP="$ac_cv_prog_TARGET_CPP"
else
  ac_cv_prog_TARGET_CPP="$TARGET_CPP"
fi
AC_MSG_RESULT($TARGET_CPP)
AC_SUBSTC(TARGET_CPP)dnl
])

