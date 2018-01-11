dnl Check to make sure that compiler runs
dnl AC_CHECK_COMPILER_OK(true-action, false-action)
AC_DEFUN(LAC_CHECK_COMPILER_OK,[
AC_CACHE_CHECK(that the compiler works, lac_cv_ccworks_$1, 
AC_TRY_LINK( , , lac_cv_ccworks_$1=yes, lac_cv_ccworks_$1=no))

if test "$lac_cv_ccworks_$1" = yes; then
  $2
else
  $3
fi
])

dnl Check that the compiler accepts ANSI prototypes.
dnl AC_CHECK_CC_PROTOTYPES(true-action, false-action)
dnl
AC_DEFUN(LAC_CHECK_CC_PROTOTYPES,[
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

AC_DEFUN(LAC_PROG_SEARCH_CC,
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_PROGS(CC, [$1], gcc)
test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])

AC_CACHE_CHECK(whether we are using GNU C, ac_cv_prog_gcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if AC_TRY_COMMAND(${CC-cc} -E conftest.c) | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi])
if test $ac_cv_prog_gcc = yes; then
  GCC=yes
  if test "${CFLAGS+set}" != set; then
    AC_CACHE_CHECK(whether ${CC-cc} accepts -shared, ac_cv_prog_gcc_shared,
[echo 'void f(){}' > conftest.c
if test -z "`${CC-cc} -shared -c conftest.c 2>&1`"; then
  ac_cv_prog_gcc_shared=yes
else
  ac_cv_prog_gcc_shared=no
fi
rm -f conftest*
])
    AC_CACHE_CHECK(whether ${CC-cc} accepts -g, ac_cv_prog_gcc_g,
[echo 'void f(){}' > conftest.c
if test -z "`${CC-cc} -g -c conftest.c 2>&1`"; then
  ac_cv_prog_gcc_g=yes
else
  ac_cv_prog_gcc_g=no
fi
rm -f conftest*
])
    if test $ac_cv_prog_gcc_g = yes; then
      CFLAGS="-g"
    fi
  fi
else
  GCC=no
fi
])

AC_DEFUN(LAC_PROG_CC,
[AC_PROVIDE([AC_PROG_CC])
if test -z "$CC"; then
set_opt_flag="no"
case $target--$lac_threads_type in
    *sunos4* ) 
	LAC_PROG_SEARCH_CC(gcc) 
	;;
    *solaris2*--solaristhreads)
	LAC_PROG_SEARCH_CC(/opt/SUNWspro/bin/cc cc gcc)
	if test "x$GCC" != "xyes"; then
	    CFLAGS="-mt $CFLAGS"
	    if test "x$lac_cv_debug" != "xyes"; then
	        CFLAGS="-xO3 $CFLAGS"
	        set_opt_flag="yes"
	    fi
	fi
	;;
    *solaris2*)
	LAC_PROG_SEARCH_CC(/opt/SUNWspro/bin/cc cc gcc)
	if test "x$GCC" != "xyes"; then
	    if test "x$lac_cv_debug" != "xyes"; then
	        CFLAGS="-xO3 $CFLAGS"
	        set_opt_flag="yes"
	    fi
	fi
	;;
    *-hp-hpux9* )
	LAC_PROG_SEARCH_CC(cc gcc)
        if test "x${GCC}" != "xyes"; then
	    CFLAGS="-Aa -D_HPUX_SOURCE $CFLAGS"
	fi
	LDFLAGS="-Wl -a,archive"
	;;
    mips-sgi-irix5* )
        LAC_PROG_SEARCH_CC(cc gcc)
	if test "x$GCC" != "xyes"; then
            CFLAGS="-woff 3262 $CFLAGS"
	fi
      ;;	
    mips-sgi-irix6* )
        LAC_PROG_SEARCH_CC(cc)
        if test "x$lac_cv_build_64bit" = "xyes"; then
	    CFLAGS="-64 $CFLAGS"
	fi
      ;;	
    *-ibm-aix*--pthreads )
        LAC_PROG_SEARCH_CC(xlc_r gcc)
      ;;
    *-ibm-aix*--none )
        LAC_PROG_SEARCH_CC(xlc gcc)
      ;;
    * ) 
        LAC_PROG_SEARCH_CC(cc gcc)
      ;;
esac
test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi
if test "x$lac_cv_debug" = "xyes"; then
  if test "x$GCC" != "xyes"; then
    CFLAGS="-g $CFLAGS"
  fi
else
    if test "$set_opt_flag" = "no"; then
        CFLAGS="-O $CFLAGS"
    fi
fi
dnl LAC_CHECK_COMPILER_OK($CC,true,exit)
LAC_CHECK_CC_PROTOTYPES(,noproto=1)
if test -n "$noproto" ; then
    AC_MSG_ERROR(The compiler $CC does not accept ANSI prototypes)
fi
CFLAGS="$THREADSAFE_CFLAGS $CFLAGS"
LDFLAGS="$THREADSAFE_LDFLAGS $LDFLAGS"
AC_SUBST(THREADSAFE_CFLAGS)
AC_SUBST(THREADSAFE_LDFLAGS)
])

dnl AC_PROG_ANSI_CC(...) is like AC_PROG_CC, but it checks that CC
dnl accepts ANSI-style prototypes.
AC_DEFUN(LAC_PROG_ANSI_CC,[dnl
LAC_PROG_CC
LAC_CHECK_CC_PROTOTYPES(,noproto=1)
if test -n "$noproto" ; then
    AC_MSG_ERROR(The compiler $CC does not accept ANSI prototypes)
fi
])

dnl Identify the target C compiler.  This will be different in the case
dnl of cross compilation or things like the SP where we use a different
dnl version of the basic compiler.
AC_DEFUN(LAC_PROG_ANSI_TARGET_CC,[
AC_REQUIRE([LAC_PROG_ANSI_CC])
TARGET_CC=$CC
AC_SUBST(TARGET_CC)
])

AC_DEFUN(LAC_PROG_CXX,
[AC_BEFORE([$0], [AC_PROG_CXXCPP])dnl
AC_PROVIDE([AC_PROG_CXX])
if test -n "$CCC"; then 
  CXX=CCC
fi
if test -z "$CXX"; then
  AC_CHECK_PROGS(CXX, CC xlC c++ g++ gcc cxx gcc)
  test -z "$CXX" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
LAC_CHECK_COMPILER_OK(CXX,true,exit)
AC_LANG_RESTORE
])

dnl Identify the target C++ compiler.  This will be different in the case
dnl of cross compilation or things like the SP where we use a different
dnl version of the basic compiler.
AC_DEFUN(LAC_PROG_TARGET_CXX,[
TARGET_CXX=$CXX
AC_SUBST(TARGET_CXX)
])

dnl LAC_LANG_TARGET_C()
AC_DEFUN(LAC_LANG_TARGET_C,
[define([AC_LANG], [C])dnl
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$TARGET_CPP $TARGET_CPPFLAGS'
ac_compile='${TARGET_CC-cc} -c $TARGET_CFLAGS $TARGET_CPPFLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${TARGET_CC-cc} -o conftest $TARGET_CFLAGS $TARGET_CPPFLAGS $TARGET_LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
])


dnl LAC_LANG_THREADSAFE_TARGET_C()
AC_DEFUN(LAC_LANG_THREADSAFE_TARGET_C,
[define([AC_LANG], [C])dnl
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$THREADSAFE_TARGET_CPP $THREADSAFE_TARGET_CPPFLAGS'
ac_compile='${THREADSAFE_TARGET_CC-cc} -c $THREADSAFE_TARGET_CFLAGS $THREADSAFE_TARGET_CPPFLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${CC-cc} -o conftest $THREADSAFE_TARGET_CFLAGS $THREADSAFE_TARGET_CPPFLAGS $THREADSAFE_TARGET_LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
])

dnl LAC_LANG_TARGET_CPLUSPLUS()
AC_DEFUN(LAC_LANG_TARGET_CPLUSPLUS,
[define([AC_LANG], [CPLUSPLUS])dnl
ac_ext=C
# CXXFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$TARGET_CXXCPP $CPPFLAGS'
ac_compile='${TARGET_CXX-g++} -c $TARGET_CXXFLAGS $TARGET_CPPFLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${TARGET_CXX-g++} -o conftest $TARGET_CXXFLAGS $TARGET_CPPFLAGS $TARGET_LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
])

AC_DEFUN(LAC_PROG_TARGET_CPP,
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
AC_SUBST(TARGET_CPP)dnl
])


