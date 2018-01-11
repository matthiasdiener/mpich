dnl
dnl Additional macros for using autoconf to build configure scripts
dnl
dnl NOTES ON ADDING TO THIS
dnl It is important to end ALL definitions with "dnl" to insure that 
dnl there are NO blank lines before the "/bin/sh" in the configure script.
dnl
dnl
dnl PAC_TEST_PROGRAM is like AC_TEST_PROGRAM, except that it makes it easier
dnl to find out what failed.
dnl
define(PAC_TEST_PROGRAM,
[AC_PROVIDE([$0])
AC_REQUIRE([AC_CROSS_CHECK])
if test $cross_compiling = 1 -a -z "$TESTCC" ; then
    ifelse([$4], , ,$4)
    Pac_CV_NAME=0
else
    if test -n "$TESTCC" ; then
      CCsav="$CC"
      CC="$TESTCC"
    fi
    cat > conftest.c <<EOF
#include "confdefs.h"
[$1]
EOF
    eval $compile
    if test ! -s conftest ; then
      echo "Could not build executable program:"
      echo "${CC-cc} $CFLAGS conftest.c -o conftest $LIBS"
      ${CC-cc} $CFLAGS conftest.c -o conftest $LIBS 
    ifelse([$3], , , [$3
])
    else
      /bin/rm -f conftestout
      if test -s conftest && (./conftest; exit) 2>conftestout; then
          ifelse([$2], , :, [$2
])
      else
        echo "Execution of test program failed"
        ifelse([$3], , , [$3
])
        if test -s conftestout ; then
            cat conftestout
        fi
      fi
    if test -n "$TESTCC" ; then
        CC="$CCsav"
    fi
  fi
  rm -fr conftest*
fi
])dnl
dnl
dnl Some systems (particularly parallel systems) do not return correct
dnl values for exit codes; for this reason, it is better to get the
dnl sizes by running the programs and then comparing them
dnl
dnl PAC_GET_TYPE_SIZE(typename,var_for_size)
dnl
dnl sets var_for_size to the size.  Ignores if the size cannot be determined
dnl
define(PAC_GET_TYPE_SIZE,
[AC_MSG_CHECKING([for size of $1])
/bin/rm -f conftestval
PAC_TEST_PROGRAM([#include <stdio.h>
main() { 
  FILE *f=fopen("conftestval","w");
  if (!f) exit(1);
  fprintf( f, "%d\n", sizeof($1));
  exit(0);
}],Pac_CV_NAME=`cat conftestval`,Pac_CV_NAME="")
if test -n "$Pac_CV_NAME" ; then
    AC_MSG_RESULT($Pac_CV_NAME)
else
    AC_MSG_RESULT(unavailable)
fi
$2=$Pac_CV_NAME
])dnl
dnl
dnl
dnl Define test for 64-bit pointers
dnl
define(PAC_POINTER_64_BITS,
[
pointersize=""
PAC_GET_TYPE_SIZE(void *,pointersize)
AC_MSG_CHECKING([for pointers greater than 32 bits])
if test -z "$pointersize" ; then
    AC_MSG_RESULT(can not determine; assuming not)
elif test $pointersize -gt 4 ; then
    ifelse($1,,AC_DEFINE(POINTER_64_BITS),AC_DEFINE($1))
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
])dnl
dnl
define(PAC_INT_LT_POINTER,[
intsize=""
PAC_GET_TYPE_SIZE(int,intsize)
if test -z "$pointersize" ; then
    PAC_GET_TYPE_SIZE(void *,pointersize)
fi
AC_MSG_CHECKING([for int large enough for pointers])
if test -n "$pointersize" -a -n "$intsize" ; then
    if test $pointersize -le $intsize ; then
       AC_MSG_RESULT(yes)
    else
       AC_DEFINE(INT_LT_POINTER)
       AC_MSG_RESULT(no)
    fi
else
    AC_MSG_RESULT(can not determine; assuming it is)
fi
])dnl
dnl
dnl Define the test for the long long int type
define(PAC_LONG_LONG_INT,
[AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING([for long long int])
AC_TEST_PROGRAM([int main() {
/* See long double test; this handles the possibility that long long int 
   has the same problem on some systems */
exit(sizeof(long long int) < sizeof(long)); }],
AC_DEFINE(HAVE_LONG_LONG_INT)AC_MSG_RESULT(yes),AC_MSG_RESULT(no))
])dnl
dnl
dnl PAC_HAVE_VOLATILE
dnl 
dnl Defines HAS_VOLATILE if the C compiler accepts "volatile" 
dnl
define(PAC_HAVE_VOLATILE,
[AC_MSG_CHECKING([for volatile])
AC_COMPILE_CHECK(,[volatile int a;],main();,
AC_DEFINE(HAS_VOLATILE)AC_MSG_RESULT(yes),AC_MSG_RESULT(no))
])dnl
dnl
dnl
dnl
define(PAC_WORDS_BIGENDIAN,
[AC_MSG_CHECKING([byte ordering])
AC_TEST_PROGRAM([main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
}], ,pac_r=1)
if test -n "$pac_r" ; then
    AC_MSG_RESULT(little endian)
else
    AC_MSG_RESULT(big endian)
    ifelse($1,,AC_DEFINE(WORDS_BIGENDIAN),AC_DEFINE($1))
fi
])dnl
dnl
dnl Define the test to look for wish (the tcl/tk windowing shell)
dnl This is under development and is derived from the FINDX command
dnl FIND_WISH looks in the path and in places that we've found wish.
dnl It sets "wishloc" to the location that it found, or to the
dnl empty string if it can't find wish.
dnl Note that we need tk version 3.3 or later, so we don't check for the 
dnl earlier versions
define(PAC_FIND_WISH,[wishloc=""
AC_MSG_CHECKING([for wish])
# Look for wish in the path
IFS="${IFS= 	}"; saveifs="$IFS"; IFS="${IFS}:"
for dir in $PATH ; do 
    if test -x $dir/wish ; then
	wishloc=$dir/wish
        break
    fi
done
IFS="$saveifs"
# Look for wish elsewhere
if test -z "$wishloc" ; then
for dir in \
    /usr/local/bin \
    /usr/local/tk-3.3/bin \
    /usr/local/tcl7.3-tk3.6/bin \
    /usr/local/tcl7.0/bin \
    /usr/local/tcl7.0-tk3.3/bin \
    /usr/contrib/bin \
    /usr/contrib/tk3.6/bin \
    /usr/contrib/tcl7.3-tk3.6/bin \
    /usr/contrib/tk3.3/bin \
    /usr/contrib/tcl7.0-tk3.3/bin \
    $HOME/tcl/bin \
    $HOME/tcl7.3/bin \
    /opt/Tcl/bin \
    /opt/bin \
    /usr/unsupported \
    /usr/unsupported/bin \
    /usr/bin \
    /bin \
    /local/encap/tcl-7.1/bin ; do
    if test -x $dir/wish ; then
	wishloc=$dir/wish
        break
    fi
done
fi
if test -n "$wishloc" ; then 
  AC_MSG_RESULT(found $wishloc)
else
  AC_MSG_RESULT(no)
fi])dnl
define(PAC_FIND_TCL,[
# Look for Tcl
if test -z "$TCL_DIR" ; then
AC_MSG_CHECKING([for Tcl])
for dir in \
    /usr \
    /usr/local \
    /usr/local/tcl7.3 \
    /usr/local/tcl7.3-tk3.6 \
    /usr/local/tcl7.0 \
    /usr/local/tcl7.0-tk3.3 \
    /usr/contrib \
    /usr/contrib/tk3.6 \
    /usr/contrib/tcl7.3-tk3.6 \
    /usr/contrib/tk3.3 \
    /usr/contrib/tcl7.0-tk3.3 \
    $HOME/tcl \
    $HOME/tcl7.3 \
    /opt/Tcl \
    /opt/local \
    /opt/local/tcl7.0 \
    /local/encap/tcl-7.1 ; do
    if test -r $dir/include/tcl.h -a -r $dir/lib/libtcl.a ; then
	TCL_DIR=$dir
	break
    fi
done
fi
if test -n "$TCL_DIR" ; then 
  AC_MSG_RESULT(found $TCL_DIR/include/tcl.h and $TCL_DIR/lib/libtcl.a)
else
  AC_MSG_RESULT(no)
fi
# Look for Tk (look in tcl dir if the code is nowhere else)
if test -z "$TK_DIR" ; then
AC_MSG_CHECKING([for Tk])
for dir in \
    /usr \
    /usr/local \
    /usr/local/tk3.6 \
    /usr/local/tcl7.3-tk3.6 \
    /usr/local/tk3.3 \
    /usr/local/tcl7.0-tk3.3 \
    /usr/contrib \
    /usr/contrib/tk3.6 \
    /usr/contrib/tcl7.3-tk3.6 \
    /usr/contrib/tk3.3 \
    /usr/contrib/tcl7.0-tk3.3 \
    $HOME/tcl \
    $HOME/tcl7.3 \
    /opt/Tcl \
    /opt/local \
    /opt/local/tk3.6 \
    /local/encap/tk-3.4 $TCL_DIR ; do
    if test -r $dir/include/tk.h -a -r $dir/lib/libtk.a ; then
	TK_DIR=$dir
	break
    fi
done
fi
if test -n "$TK_DIR" ; then 
  AC_MSG_RESULT(found $TK_DIR/include/tk.h and $TK_DIR/lib/libtk.a)
else
  AC_MSG_RESULT(no)
fi
])dnl
dnl
dnl Look for a non-standard library by looking in some named places.
dnl Check for both foo.a and libfoo.a
dnl 
dnl PAC_FIND_USER_LIB(LIB-NAME[,LIB-LIST,ACTION-IF-FOUND,ACTION-IF-NOT-FOUND])
dnl (use foo to check for foo.a and libfoo.a)
dnl Checks the usual places, as well as /usr/local/LIBNAME and
dnl /usr/local/LIBNAME/lib .
dnl The location of the library may be found in pac_lib_file.
dnl The DIRECTORY of the library may be found in pac_lib_dir
dnl
define(PAC_FIND_USER_LIB,[
AC_MSG_CHECKING([for library $1])
pac_lib_file=""
for dir in $2 \
    /usr \
    /usr/local \
    /usr/local/$1 \
    /usr/contrib \
    /usr/contrib/$1 \
    $HOME/$1 \
    /opt/$1 \
    /opt/local \
    /opt/local/$1 \
    /local/encap/$1 ; do
    if test -r $dir/$1.a ; then
	pac_lib_file=$dir/$1.a
        pac_lib_dir=$dir
	break
    fi
    if test -r $dir/lib$1.a ; then
	pac_lib_file=$dir/lib$1.a
        pac_lib_dir=$dir
	break
    fi
    if test -r $dir/lib/$1.a ; then
	pac_lib_file=$dir/lib/$1.a
        pac_lib_dir=$dir/lib
	break
    fi
    if test -r $dir/lib/lib$1.a ; then
	pac_lib_file=$dir/lib/lib$1.a
        pac_lib_dir=$dir/lib
	break
    fi
done
if test -n "$pac_lib_file" ; then 
  AC_MSG_RESULT(found $pac_lib_file)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])dnl
dnl
dnl Look for a non-standard include by looking in some named places.
dnl Check for foo.h
dnl 
dnl PAC_FIND_USER_INCLUDE(FILE-NAME[,DIR-LIST,ACTION-IF-FOUND,ACTION-IF-NOT-FOUND])
dnl (use foo to check for foo.h)
dnl Checks the usual places, as well as /usr/local/FILENAME and
dnl /usr/local/FILENAME/include .
dnl The location of the include directory library may be found in pac_inc_dir.
dnl
define(PAC_FIND_USER_INCLUDE,[
AC_MSG_CHECKING([for include directory for $1])
pac_inc_dir=""
for dir in $2 \
    /usr \
    /usr/local \
    /usr/local/$1 \
    /usr/contrib \
    /usr/contrib/$1 \
    $HOME/$1 \
    /opt/$1 \
    /opt/local \
    /opt/local/$1 \
    /local/encap/$1 ; do
    if test -r $dir/$1.h ; then
	pac_inc_dir=$dir
	break
    fi
    if test -r $dir/include/$1.h ; then
	pac_inc_dir=$dir/include
	break
    fi
    if test -r $dir/lib/lib$1.a ; then
	pac_lib_file=$dir/lib/lib$1.a
	break
    fi
done
if test -n "$pac_inc_dir" ; then 
  AC_MSG_RESULT(found $pac_inc_dir)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])dnl
dnl
dnl The AC_CHECK_HEADER assumes that you can use cpp to check for 
dnl headers for the (CROSS!) compiler.  This is ridiculous.
dnl I've FIXED the code from version 2
dnl
dnl ### Printing messages
dnl
dnl
dnl Check whether to use -n, \c, or newline-tab to separate
dnl checking messages from result messages.
dnl Idea borrowed from dist 3.0.
dnl Internal use only.
define(AC_PROG_ECHO_N,
ac_echo_n=yes
[if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi
ac_echo_test=`echo foo 1>&1`
if test -z "$ac_echo_test" ; then
     print_error "Your sh shell does not handle the output redirection"
     print_error "1>&1 correctly.  Configure will work around this problem,"
     print_error "but you should report the problem to your vendor."
fi
])dnl
dnl AC_MSG_CHECKING(FEATURE-DESCRIPTION)
define(AC_FD_MSG,1)dnl
define(AC_MSG_CHECKING,[dnl
if test -z "$ac_echo_n" ; then
AC_PROG_ECHO_N
fi
if test -z "$ac_echo_test" -a AC_FD_MSG = 1 ; then
echo $ac_n "checking $1""... $ac_c"
else
echo $ac_n "checking $1""... $ac_c" 1>&AC_FD_MSG
fi])dnl
dnl
dnl AC_CHECKING(FEATURE-DESCRIPTION)
define(AC_CHECKING,dnl
[echo "checking $1" 1>&AC_FD_MSG])dnl
dnl
dnl AC_MSG_RESULT(RESULT-DESCRIPTION)
define(AC_MSG_RESULT,dnl
if test -z "$ac_echo_test" -a AC_FD_MSG = 1 ; then
[echo "$ac_t""$1"]
else
[echo "$ac_t""$1" 1>&AC_FD_MSG]
fi)dnl
dnl
dnl PAC_CHECK_HEADER(HEADER-FILE, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND],
dnl PRE-REQ-HEADERS )
dnl
dnl BUG: AIX 4.1 can't handle a \055 (octal for -) in a tr string (sometimes;
dnl it works from the shell but not within a file)
dnl I've removed that and hoped that no header will include a - in the
dnl name
dnl
dnl This can fail if the header needs OTHER headers for the compile
dnl to succeed.  Those headers should be specified in the "pre-req-headers"
dnl For example 
dnl PAC_CHECK_HEADER(sys/vfs.h,AC_DEFINE(HAVE_SYS_VFS_H),,
dnl                  [#include <sys/types.h>])
dnl
define(PAC_CHECK_HEADER,dnl
[dnl Do the transliteration at runtime so arg 1 can be a shell variable.
changequote(,)dnl
ac_safe=`echo "$1" | tr '[a-z]./' '[A-Z]__'`
changequote([,])dnl
AC_MSG_CHECKING([for $1])
dnl AC_CACHE_VAL(ac_cv_header_$ac_safe,[dnl
AC_COMPILE_CHECK(,[$4]
[#include <$1>],main();,eval "ac_cv_header_$ac_safe=yes",
  eval "ac_cv_header_$ac_safe=no")dnl])dnl
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no)
ifelse([$3], , , [$3
])dnl
fi
])dnl
dnl
dnl PAC_CHECK_HEADERS(HEADER-FILE... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
define(PAC_CHECK_HEADERS,[for ac_hdr in $1
do
PAC_CHECK_HEADER($ac_hdr,
[changequote(, )dnl
  ac_tr_hdr=HAVE_`echo $ac_hdr | tr '[a-z]./' '[A-Z]__'`
changequote([, ])dnl
  AC_DEFINE($ac_tr_hdr) $2], $3)dnl
done
])dnl
dnl
dnl WARNING: AC_HEADER_STDC uses CPP instead of CC!
dnl
dnl
dnl Check to see if malloc is declared as char *malloc or void *malloc
dnl If stdlib.h is not defined, then this will choose char*malloc.
dnl
define(PAC_MALLOC_RETURNS_VOID,
[AC_MSG_CHECKING(for malloc return type)
AC_COMPILE_CHECK(,[#include <stdlib.h>],[extern void *malloc();],
eval "ac_cv_malloc=void",eval "ac_cv_malloc=char")
AC_MSG_RESULT($ac_cv_malloc)
if test "$ac_cv_malloc" = void ; then
    AC_DEFINE(MALLOC_RET_VOID)
fi
])dnl
dnl
dnl Check that the compile actually runs.  Perform first arg is yes,
dnl second if false
dnl PAC_CHECK_COMPILER_OK(true-action, false-action)
dnl
define(PAC_CHECK_COMPILER_OK,[
AC_MSG_CHECKING(that the compiler $CC runs)
AC_COMPILE_CHECK(,,return 0;,eval "ac_cv_ccworks=yes",eval "ac_cv_ccworks=no")
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
# Generate output from failed test.  See COMPILE_CHECK code
# It really would be better if the compile tests put the output into
# a file for later analysis, like conftest.out
#
cat > conftest.c <<EOF
#include "confdefs.h"
int main() { exit(0); }
int t() { return 0; }
EOF
$CC $CFLAGS conftest.c -o conftest $LIBS
rm -f conftest* 
#
# End of output
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl Check that the compile accepts ANSI prototypes.  Perform first arg if yes,
dnl second if false
dnl PAC_CHECK_CC_PROTOTYPES(true-action, false-action)
dnl
define(PAC_CHECK_CC_PROTOTYPES,[
AC_MSG_CHECKING(that the compiler $CC accepts ANSI prototypes)
AC_COMPILE_CHECK(,[int f(double a){return 0;}],,eval "ac_cv_ccworks=yes",eval "ac_cv_ccworks=no")
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl Test the compiler to see if it actually works.  First, check to see
dnl if the compiler works at all
dnl Uses TESTCC, not CC
dnl 
dnl The test directory is ccbugs by default, but can be overridded with 
dnl CCBUGS
dnl
define(PAC_CORRECT_COMPILER,[
if test -z "$CCBUGS" ; then CCBUGS=ccbugs ; fi
if test -d $CCBUGS ; then 
    # Use "LTESTCC" as "local Test CC"
    if test -z "$TESTCC" ; then LTESTCC="$CC" ; else LTESTCC="$TESTCC" ; fi
    for file in $CCBUGS/ccfail*.c ; do
        CFILE=`basename $file .c`
        AC_MSG_CHECKING(`cat $CCBUGS/$CFILE.title`)
        cp $file conftest.c
        broken=1
        rm -f conftest.out conftest.rout
        if eval $LTESTCC $CFLAGS -o conftest conftest.c $LIBS >conftest.out 2>&1 ; then
	    if test -s conftest ; then
                ./conftest 2>&1 1>conftest.rout
                if test $? = 0 ; then
  	            broken=0
                fi
	    fi
        fi
        if test $broken = 1 ; then 
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g' 
	    if test -s conftest.out ; then
	        echo "Output from compile step was:"
		cat conftest.out
	    fi
	    if test -s conftest.rout ; then
	        echo "Output from run step was:"
		cat conftest.rout
	    fi
	else
	    AC_MSG_RESULT(yes)
        fi
	/bin/rm -f conftest conftest.c conftest.o conftest.out conftest.rout
    done
    #
    # These are non-fatal, but must be run
    for file in $CCBUGS/ccnfail*.c ; do
        CFILE=`basename $file .c`
        AC_MSG_CHECKING(`cat $CCBUGS/$CFILE.title`)
        cp $file conftest.c
        nbroken=1
	rm -f conftest.out conftest.rout
        if eval $LTESTCC $CFLAGS -o conftest conftest.c $LIBS >conftest.out 2>&1 ; then
	    if test -s conftest ; then
                ./conftest 2>&1 1>conftest.rout
                if test $? = 0 ; then
  	            nbroken=0
                fi
	    fi
        fi
        if test $nbroken = 1 ; then 
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g' 
	    if test -s conftest.out ; then
	        echo "Output from compile step was:"
		cat conftest.out
	    fi
	    if test -s conftest.rout ; then
	        echo "Output from run step was:"
		cat conftest.rout
	    fi
	else
	    AC_MSG_RESULT(yes)
        fi
	/bin/rm -f conftest conftest.c conftest.o conftest.out conftest.rout
    done

    # 
    # Now, try the warnings.  Note that this just does compiles, not runs
    for file in $CCBUGS/ccwarn*.c ; do
        CFILE=`basename $file .c`
        AC_MSG_CHECKING(`cat $CCBUGS/$CFILE.title`)
        cp $file conftest.c
        if eval $CC $CFLAGS \
	    -DCONFIGURE_ARGS_CLEAN="'"'"'-A -B'"'"'" -c \
	    conftest.c $LIBS > /dev/null 2>&1 ; then
	    AC_MSG_RESULT(yes)
	    true 
	else
	    AC_MSG_RESULT(no)
	    cat $CCBUGS/$CFILE.txt | sed 's/^/\*\#/g' 
	    if test "$CFILE" = "ccwarn1" ; then
	       CONFIGURE_ARGS_CLEAN="`echo $CONFIGURE_ARGS_CLEAN | tr ' ' '_'`"
            fi
        fi
	# set +x
	/bin/rm -f conftest conftest.[co]
    done
    # 
    # After everything, see if there are any problems
    if test $broken = 1 ; then 
        if test -z "$FAILMSG" ; then
	    echo "Compiler $CC appears broken; aborting configure..."
        else
	    eval echo "$FAILMSG"
        fi
        exit 1
    fi
fi
])dnl
dnl
dnl Check that the Fortran compiler works.  We needed this first for LINUX
dnl Perform first arg is yes, second if false
dnl PAC_CHECK_F77_COMPILER_OK(true-action, false-action)
dnl The name of the compiler is F77
dnl
define(PAC_CHECK_F77_COMPILER_OK,[
AC_MSG_CHECKING(that the compiler $F77 runs)
cat >conftest.f <<EOF
          program main
          end
EOF
/bin/rm -f conftest.out
$F77 $FFLAGS -c conftest.f > conftest.out 2>&1
if test $? != 0 ; then
    AC_MSG_RESULT(no)
    echo "Fortran compiler returned non-zero return code"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
elif test ! -s conftest.o ; then
    AC_MSG_RESULT(no)
    echo "Fortran compiler did not produce object file"
    if test -s conftest.out ; then
	echo "Output from test was"
        cat conftest.out
    fi
    ifelse([$2],,:,[$2])
else    
    AC_MSG_RESULT(yes)
    ifelse([$1],,:,[$1])
fi
rm -f conftest* 
])dnl
dnl
dnl
dnl PAC_PROGRAM_CHECK(VARIABLE, PROG-TO-CHECK-FOR, VALUE-IF-FOUND
dnl               [, VALUE-IF-NOT-FOUND [,FULL-PATH-IF-FOUND])
dnl
dnl The variable named by FULL-PATH-IF-FOUND will be set to the
dnl full path for the program
dnl
dnl A fault with the routine in autoconf is that if PROG-TO-CHECK-FOR
dnl already has a path, it will FAIL!
dnl
dnl An UNDOCUMENTED FEATURE is that if VARIABLE is already set, this
dnl routine DOES NOTHING!
dnl
define(PAC_PROGRAM_CHECK,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
dnl AC_CACHE_VAL(ac_cv_prog_$1,[
ac_prog_where=""
if test -n "[$]$1"; then
  ac_cv_prog_$1="[$]$1" # Let the user override the test.
else
  ac_first_char=`expr "$2" : "\(.\)"`
  if test "$ac_first_char" = "/" -a -x "$2" ; then
       ac_cv_prog_$1="$3"
       ac_prog_where=$2
  else
      IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
      for ac_dir in $PATH; do
        test -z "$ac_dir" && ac_dir=.
        if test -f $ac_dir/$ac_word; then
          ac_cv_prog_$1="$3"
          ac_prog_where=$ac_dir/$ac_word
          break
        fi
      done
      IFS="$ac_save_ifs"
  fi
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_CHECK_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_prog_$1" && ac_cv_prog_$1="$4"
])dnl
fi;dnl])dnl
$1="$ac_cv_prog_$1"
if test -n "$ac_prog_where" ; then
  AC_MSG_RESULT(found $ac_prog_where ([$]$1))
  ifelse([$5], , , [ $5=$ac_prog_where ] )
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])dnl
dnl
dnl PAC_PROGRAMS_CHECK is like PAC_PROGRAM_CHECK, but with
dnl a list of programs.
dnl
define(PAC_PROGRAMS_CHECK,
[for p in $2
do
PAC_PROGRAM_CHECK($1, [$]p, [$]p, )
test -n "[$]$1" && break
done
ifelse([$3],,, [test -n "[$]$1" || $1="$3"
])])dnl
dnl
dnl PAC_CHECK_SIZEOF(TYPE)
define(PAC_CHECK_SIZEOF,
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(sizeof_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_sizeof_$1, [ *], [_p]))dnl
changequote([, ])dnl
dnl Can only do this test if not cross-compiling (and TESTCC not defined)
if test $cross_compiling = 1 -a -z "$TESTCC" ; then
    echo "Cannot check for size of $1 when cross-compiling"
    AC_CV_NAME=0
else
AC_MSG_CHECKING(size of $1)
if test -n "$TESTCC" ; then
    CCsav="$CC"
    CC="$TESTCC"
fi
AC_TEST_PROGRAM([#include <stdio.h>
main()
{
  FILE *f=fopen("cftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", sizeof($1));
  exit(0);
}], AC_CV_NAME=`cat cftestval`,AC_CV_NAME=0)
rm -f cftestval
if test -n "$TESTCC" ; then
    CC="$CCsav"
fi

if test "$AC_CV_NAME" = 0 ; then
AC_MSG_RESULT($1 unsupported)
else
AC_MSG_RESULT($AC_CV_NAME)
fi
$2=$AC_CV_NAME
dnl AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME)
fi
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])dnl
dnl
dnl We need routines to check that make works.  Possible problems with
dnl make include
dnl
dnl It is really gnumake, and contrary to the documentation on gnumake,
dnl it insists on screaming everytime a directory is changed.  The fix
dnl is to add the argument --no-print-directory to the make
dnl
dnl It is really BSD 4.4 make, and can't handle 'include'.  For some
dnl systems, this can be fatal; there is no fix (other than removing this
dnl aleged make).
dnl
dnl It is the OSF V3 make, and can't handle a comment in a block of targe
dnl code.  There is no acceptable fix.
dnl
dnl This assumes that "MAKE" holds the name of the make program.  If it
dnl determines that it is an improperly built gnumake, it adds
dnl --no-print-directorytries to the symbol MAKE.
define(PAC_MAKE_IS_GNUMAKE,[
AC_MSG_CHECKING(gnumake)
/bin/rm -f conftest
cat > conftest <<.
SHELL=/bin/sh
ALL:
	@(dir=`pwd` ; cd .. ; $MAKE -f \$\$dir/conftest SUB)
SUB:
	@echo "success"
.
str=`$MAKE -f conftest 2>&1`
if test "$str" != "success" ; then
    str=`$MAKE --no-print-directory -f conftest 2>&1`
    if test "$str" = "success" ; then
        MAKE="$MAKE --no-print-directory"
	AC_MSG_RESULT(yes using --no-print-directory)
    else
	AC_MSG_RESULT(no)
    fi
else
    AC_MSG_RESULT(no)
fi
/bin/rm -f conftest
str=""
])dnl
dnl
dnl PAC_MAKE_IS_BSD44([true text])
dnl
define(PAC_MAKE_IS_BSD44,[
AC_MSG_CHECKING(BSD 4.4 make)
/bin/rm -f conftest
cat > conftest <<.
ALL:
	@echo "success"
.
cat > conftest1 <<.
include conftest
.
str=`$MAKE -f conftest1 2>&1`
/bin/rm -f conftest conftest1
if test "$str" != "success" ; then
    AC_MSG_RESULT(Found BSD 4.4 so-called make)
    echo "The BSD 4.4 make is INCOMPATIBLE with all other makes."
    echo "Using this so-called make may cause problems when building programs."
    echo "You should consider using gnumake instead."
    ifelse([$1],,[$1])
else
    AC_MSG_RESULT(no - whew)
fi
str=""
])dnl
dnl
dnl PAC_MAKE_IS_OSF([true text])
dnl
define(PAC_MAKE_IS_OSF,[
AC_MSG_CHECKING(OSF V3 make)
/bin/rm -f conftest
cat > conftest <<.
SHELL=/bin/sh
ALL:
	@# This is a valid comment!
	@echo "success"
.
str=`$MAKE -f conftest 2>&1`
/bin/rm -f conftest 
if test "$str" != "success" ; then
    AC_MSG_RESULT(Found OSF V3 make)
    echo "The OSF V3 make does not allow comments in target code."
    echo "Using this make may cause problems when building programs."
    echo "You should consider using gnumake instead."
    ifelse([$1],,[$1])
else
    AC_MSG_RESULT(no)
fi
str=""
])dnl
dnl
dnl Here begins macros for setting defaults for specific systems.
dnl These handle things like C compilers with funny names and special
dnl options.
dnl
dnl
dnl These make use of the GLOBALS; see each definition for which ones
dnl are used:
dnl
dnl (Not yet present)
dnl
dnl PAC_GET_CC(arch)
dnl Uses USERCC, CC, USERCLINKER, CLINKER, LIB_LIST.  
dnl Looks for special versions
dnl of C compilers, particularly cross compilers.  May also set some
dnl compile flags.  Clears GCC if it sets CC.  Calls "print_error" for
dnl error messages
dnl
define(PAC_GET_CC,[
if test -z "$USERCC" ; then
case $1 in 
   intelnx|paragon) CC=icc ; GCC="" 
	  # If this version of the intel compiler accepts the -nx flag, use it.
  	  if icc -nx > /dev/null 2>&1 ; then
	    # For some reason the lib list was commented out; we need
	    # it to link with if we use the NX routines
	    LIB_LIST="$LIB_LIST -nx"
	    CFLAGS="$CFLAGS -nx"
	  fi
	;;
   cm5) CC=cc ; GCC="" ;   if test -z "$USERCLINKER" ; then
		      CLINKER="cmmd-ld -comp $CC"
		  fi ;;
   cray_t3d)        CC=/mpp/bin/cc ; CFLAGS="$CFLAGS -Tcray-t3d" ; GCC="" 
                    if test -z "$USERCLINKER" ; then 
	            CLINKER="$CC -Tcray-t3d" ; fi ;;
   hpux) if test "`which ${CC-cc}`" = "/usr/convex/bin/cc" ; then 
        CFLAGS="$CFLAGS -or none -U_REENTRANT -D_POSIX_SOURCE -D_HPUX_SOURCE -DMPI_cspp"
         elif test "$CC" != "gcc" ; then
            CFLAGS="$CFLAGS -Aa -D_POSIX_SOURCE -D_HPUX_SOURCE"
	    # Alternate...
	    # -Ae is extended -Aa (only on some PA RISC systems)
	    #CFLAGS="$CFLAGS -Ae +Olibcalls"
	    #CLINKER="cc -Ae"
	    # fort77 is the POSIX-compliant version of f77; fort77 can use 
	    # -Ldirectory
	    #F77=fort77
	    #FLINKER=fort77
	    #FFLAGS="$FFLAGS +ppu"
	    # Try and see that this works
	    AC_COMPILE_CHECK(Checking that HP compiler has ANSI option...,,,
		hpux_ansi=1,hpux_ansi=0)
	    if test $hpux_ansi = 0 ; then
	       print_error "HPUX C compiler does not support ANSI mode!"
	       print_error "This mode is required because severe bugs in HPUX CPP"
	       print_error "cause problems.  Configuration continuing BUT	"
	       print_error "if you have trouble, consider using the GNU C"
	       print_error "compiler gcc instead."
	    else
              print_error "HPUX C compiler is being forced into ANSI mode so that"
              print_error "severe bugs in HPUX CPP do not cause problems"
	    fi
         fi 
	 ;;
    alpha)   ;;
    convex_spp)  CC="/usr/convex/bin/cc" ;;
    ibmpoe)
         dnl This is intended for the Standard POE/MPL version
         dnl This version REQUIRES you to have either mpCC or mpcc.
         dnl ??? is this safe ??
	 dnl An additional problem is that some sites will install mpCC
         dnl even though xlC is not available (!).  This forces us
         dnl to test for both mpCC and xlC, then mpcc and xlc.
	 CCval=
         PAC_PROGRAM_CHECK(CCval,xlC,xlC)
         if test -n "$CCval" ; then
             TESTCC="$CCval"
	     CCval=""
             PAC_PROGRAM_CHECK(CCval,mpCC,mpCC)
         else
	     PAC_PROGRAM_CHECK(CCval,xlc,xlc)
             if test -n "$CCval" ; then
                 TESTCC="$CCval"
	         CCval=""
                 PAC_PROGRAM_CHECK(CCval,mpcc,mpcc)
             fi
         fi
         if test -z "$CCval" ; then
            print_error "Could not find mpCC or mpcc!"
            print_error "Make sure that you path is set correctly."
	    exit 1
         fi
         CC="$CCval"
    ;;
    meiko) 
      dnl /opt/SUNWspro/bin/cc,/opt/apogee/bin/apcc,/opt/PGI/bin/cc,
      dnl /opt/gcc/bin/gcc
      CCval=''
      PAC_PROGRAMS_CHECK(CCval,cc apcc pgcc gcc)
      if test -z "$CCval" ; then
          print_error "Could not find a C compiler"
	  exit 1
      elif test "$CCVal" = "cc" ; then
          CC="cc -g -xcg92"
      else
	  CC=$CCval
      fi
	;;
    ncube)   CC=ncc ;;
    rs6000)
      CCval=""
      PAC_PROGRAMS_CHECK(CCval,xlC xlc cc)
      if test -n "$CCval" ; then
	 CC=$CCval
         GCC=""
      fi
      ;;
    *)
      CCval=""
      # Pick the vendor's cc ahead of gcc.
      PAC_PROGRAMS_CHECK(CCval,cc gcc)
      if test -n "$CCval" ; then
	 CC=$CCval
      fi
      ;;
esac
fi
if test -z "$USERCLINKER" -a -z "$CLINKER" ; then
    CLINKER="$CC"
fi
])dnl
dnl
dnl PAC_GET_ANSI_CC(...) is like PAC_GET_CC, but it checks that CC
dnl accepts ANSI-style prototypes.  If not, it then tries to use gcc
dnl
define(PAC_GET_ANSI_CC,[
PAC_GET_CC($1)
PAC_CHECK_CC_PROTOTYPES(,noproto=1)
if test -n "$noproto" ; then
    print_error "The compiler $CC does not accept ANSI prototypes"
    CC=""
    PAC_PROGRAM_CHECK(CC,gcc,gcc)
    if test -n "$CC" ; then
	PAC_CHECK_COMPILER_OK(,exit 1)
	PAC_CHECK_CC_PROTOTYPES(,exit 1)
        print_error "Using $CC as the C compiler instead"
	CLINKER=$CC
    else
	CC=""
        CLINKER=""
    fi
fi
])dnl
dnl
dnl
dnl May also set F77GETARG (routine to get commandline arguments)
dnl Also sets HAS_F77 (makes sure that the chosen compiler is present!)
dnl
define(PAC_GET_F77,[
if test -z "$USERF77" ; then
case $1 in 
   intelnx|paragon|i860) F77=if77 ;;
   cm5) # TMC Told us this should be f77
        F77=f77 ; if test -z "$USERFLINKER" ; then
		      FLINKER="cmmd-ld -comp $F77"
		  fi ;;
   CRAY)
   # The Fortran compiler might be cf77 or f77
   # This wierd function uses the VALUE of the first argument before
   # the second!
   F77=	
   PAC_PROGRAMS_CHECK(F77,cf77 f77)
   ;;
   cray_t3d)        
# The dp switch on the following lines allows compilation of "double precision"
# at the cost of violating the Fortran standard
   print_error Setting double precision to be the same as real, in violation 
   print_error of the Fortran standard.  This is needed because Cray Fortran 
   print_error for the T3D does not support double precision and hence is 
   print_error not actually Fortran.
   F77="/mpp/bin/cf77"
   FFLAGS="$FFLAGS -Ccray-t3d -dp"
   F77GETARG="call pxfgetarg(i,s,len(s),ierr)"
   if test -z "$USERFLINKER" ; then
       FLINKER="$F77 -Ccray-t3d"
   fi

   ;;
   hpux) 
    # This may eliminate the need for +U77 ....
    if test "`which $F77`" != "/usr/convex/bin/fc" ; then 
        F77GETARG=["call igetarg(i,s,len(s))"]
	# mgates reports that the +T option makes MPICH work on some HPUX
	# platforms.  The documentation for +T is truely scary; it seems
	# to imply that without it, a Fortran program will NOT run correctly.
	FFLAGS="$FFLAGS +T"
    else
        # The Convex compiler needs to have optimization messages suppressed
        FFLAGS="$FFLAGS -or none"
    fi
    ;;
    convex_spp)  F77="/usr/convex/bin/fc" ;;
    ibmpoe)
         dnl This is intended for the Standard POE/MPL version
	 F77=mpxlf
    ;;
    meiko) 
      PAC_PROGRAMS_CHECK(FCval,f77 apf77 pgf77)
      if test -n "$FCval" -a "$FCval" = f77 ; then
	F77="f77 -g -cg92"
      else
        F77="$FCval"
      fi
      ;;
    ncube)   F77=nf77 ;;
    rs6000)  F77=xlf ;;
esac
fi
if test -z "$USERFLINKER" -a -z "$FLINKER" ; then
    FLINKER="$F77"
fi
#
# Check that the Fortran compiler is actually available:
HAS_F77=
if test -n "$F77" ; then
    PAC_PROGRAM_CHECK(HAS_F77,$F77,1,0)
fi
])dnl
dnl
dnl Fortran runtime for Fortran/C linking
dnl On suns, try
dnl FC_LIB          =/usr/local/lang/SC2.0.1/libM77.a \ 
dnl              /usr/local/lang/SC2.0.1/libF77.a -lm \
dnl              /usr/local/lang/SC2.0.1/libm.a \
dnl              /usr/local/lang/SC2.0.1/libansi.a
dnl
dnl AIX requires -bI:/usr/lpp/xlf/lib/lowsys.exp
dnl ------------------------------------------------------------------------
dnl
dnl Get the format of Fortran names.  Uses F77, FFLAGS, and sets WDEF.
dnl If the test fails, sets NOF77 to 1, HAS_FORTRAN to 0
dnl
define(PAC_GET_FORTNAMES,[
   # Check for strange behavior of Fortran.  For example, some FreeBSD
   # systems use f2c to implement f77, and the version of f2c that they 
   # use generates TWO (!!!) trailing underscores
   # Currently, WDEF is not used but could be...
   #
   # Eventually, we want to be able to override the choices here and
   # force a particular form.  This is particularly useful in systems
   # where a Fortran compiler option is used to force a particular
   # external name format (rs6000 xlf, for example).
   cat > confftest.f <<EOF
       subroutine mpir_init_fop( a )
       integer a
       a = 1
       return
       end
EOF
   $F77 $FFLAGS -c confftest.f > /dev/null 2>&1
   if test ! -s confftest.o ; then
        print_error "Unable to test Fortran compiler"
        print_error "(compiling a test program failed to produce an "
        print_error "object file)."
	NOF77=1
        HAS_FORTRAN=0
   elif test -n "$FORTRANNAMES" ; then
	WDEF="-D$FORTRANNAMES"
   else
    # We have to be careful here, since the name may occur in several
    # forms.  We try to handle this by testing for several forms
    # directly.
    if test $arch_CRAY ; then
     # Cray doesn't accept -a ...
     nameform1=`strings confftest.o | grep mpir_init_fop_  | head -1`
     nameform2=`strings confftest.o | grep MPIR_INIT_FOP   | head -1`
     nameform3=`strings confftest.o | grep mpir_init_fop   | head -1`
     nameform4=`strings confftest.o | grep mpir_init_fop__ | head -1`
    else
     nameform1=`strings -a confftest.o | grep mpir_init_fop_  | head -1`
     nameform2=`strings -a confftest.o | grep MPIR_INIT_FOP   | head -1`
     nameform3=`strings -a confftest.o | grep mpir_init_fop   | head -1`
     nameform4=`strings -a confftest.o | grep mpir_init_fop__ | head -1`
    fi
    /bin/rm -f confftest.f confftest.o
    if test -n "$nameform4" ; then
	echo "Fortran externals are lower case and have 1 or 2 trailing underscores"
	WDEF=-DFORTRANDOUBLEUNDERSCORE
    elif test -n "$nameform1" ; then
        # We don't set this in CFLAGS; it is a default case
        echo "Fortran externals have a trailing underscore and are lowercase"
	WDEF=-DFORTRANUNDERSCORE
    elif test -n "$nameform2" ; then
	echo "Fortran externals are uppercase"     
	WDEF=-DFORTRANCAPS 
    elif test -n "$nameform3" ; then
	echo "Fortran externals are lower case"
	WDEF=-DFORTRANNOUNDERSCORE 
    else
	print_error "Unable to determine the form of Fortran external names"
	print_error "Make sure that the compiler $F77 can be run on this system"
#	print_error "If you have problems linking, try using the -nof77 option"
#        print_error "to configure and rebuild MPICH."
	print_error "Turning off Fortran (-nof77 being assumed)."
	NOF77=1
        HAS_FORTRAN=0
    fi
#   case $nameform in 
#       MPIR_INIT_FOP | _MPIR_INIT_FOP)
#	echo "Fortran externals are uppercase"     
#	WDEF=-DFORTRANCAPS 
#	;;
#       mpir_init_fop_ | _mpir_init_fop_)   
#	 # We don't set this in CFLAGS; it is a default case
#        echo "Fortran externals have a trailing underscore and are lowercase"
#	WDEF=-DFORTRANUNDERSCORE ;;
#
#       mpir_init_fop | _mpir_init_fop)     
#	echo "Fortran externals are lower case"
#	WDEF=-DFORTRANNOUNDERSCORE 
#	;;
#
#           # Fortran no underscore is the "default" case for the wrappers; 
#	   # having this defined allows us to have an explicit test, 
#	   # particularly since the most common UNIX case is FORTRANUNDERSCORE
#       mpir_init_fop__ | _mpir_init_fop__)  
#	echo "Fortran externals are lower case and have 1 or 2 trailing underscores"
#	WDEF=-DFORTRANDOUBLEUNDERSCORE
#        ;;
#
#       *)
#	print_error "Unable to determine the form of Fortran external names"
#	print_error "If you have problems linking, try using the -nof77 option"
#        print_error "to configure and rebuild MPICH."
#	NOF77=1
#        HAS_FORTRAN=0
#	;;
#   esac
    fi])dnl
dnl
dnl ------------------------------------------------------------------------
dnl
define(PAC_GET_AR,[
if test -z "$USERAR" ; then
case $1 in 
   intelnx|paragon|i860) AR="ar860 crl" ;;
   cm5) AR="ar cr"
   ;;
   meiko|solaris) AR="ar cr"
   ;;
   ncube) AR="nar cr" ;;
esac
fi
if test -z "$AR" ; then 
    AR="ar cr$ARLOCAL"
fi
])dnl
dnl --------------------------------------------------------
dnl Find X11.  This is more careful than the AC version
dnl Uses USERXLIB; sets x_includes, X11INC, x_libraries, X11LIB, no_x
dnl
define(PAC_FIND_X11,[
   # FIND_X doesn't always work correctly when cross compiling, so we
   # try to be more careful and conservative
   if test -z "$USERXLIB" ; then 
    # The user has specified the libraries/include paths; pick them up 
    # below....
    if test $cross_compiling = 0 ; then 
       AC_FIND_X()
       if test -n "$no_x" ; then
	  print_error "Did not find X11 libraries and/or include files"
       fi
    else
       # Try to compile a program with an include file.
       # I didn't use HEADER_CHECK because I want to insist that the 
       # code try to compile with the header
       no_x=true
       AC_COMPILE_CHECK([X11 headers],[#include <X11/Xlib.h>],,no_x="")
       if test -z "$no_x" ; then 
          # Try to link a simple X program
          AC_HAVE_LIBRARY(X11,no_x="",no_x="true")
       fi
       if test -n "$no_x" ; then
         print_error " " 
         print_error "X11 is not used when cross compiling (because of the"
         print_error "difficulties in finding the correct libraries)"
         print_error " "
       fi
     fi
   else
	# Pick up the paths from the user if possible
	if test -z "$x_includes" -a -n "$X11INC" ; then 
	    x_includes="$X11INC"
            XINCLUDES="-Ix_includes"
	fi
	if test -z "$x_libraries" -a -n "$X11LIB" ; then 
	    x_libraries="$X11LIB"
	fi
   fi
   if test -n "$x_includes" ; then
       XINCLUDES="-I$x_includes"
   fi
])dnl
dnl --------------------------------------------------------
dnl Test for the VERSION of tk.  There are major changes between 3.6 and 4.0
dnl (in particular, the type Tk_ColorModel disappeared
dnl  Put result into TK_VERSION (as, e.g., 3.6 or 4.0).  Should test version
dnl as STRING, since we don't control the changes between versions, and 
dnl only versions that we know should be tested.
dnl Note that this may be important ONLY if you include tk.h .
dnl
dnl TK_LIB and XINCLUDES must be defined
dnl
define(PAC_TK_VERSION,[
AC_MSG_CHECKING(for version of TK)
/bin/rm -f conftestval
CFLAGSsave="$CFLAGS"
CFLAGS="$CFLAGS -I$TK_DIR/include $XINCLUDES"
PAC_TEST_PROGRAM([#include "tk.h"
#include <stdio.h>
main() { FILE *fp = fopen( "conftestval", "w" ); 
fprintf( fp, "%d.%d", TK_MAJOR_VERSION, TK_MINOR_VERSION );
return 0; }],
TK_VERSION=`cat conftestval`,TK_VERSION="unavailable")
CFLAGS="$CFLAGSsave"
AC_MSG_RESULT($TK_VERSION)
])dnl
dnl
dnl Redefine these to use msg_checking/result
dnl Also, test for broken LINUX shells
dnl
define([AC_COMPILE_CHECK],
[AC_PROVIDE([$0])dnl
ifelse([$1], , , [AC_MSG_CHECKING(for $1)]
)dnl
if test ! -f confdefs.h ; then
    AC_MSG_RESULT("!! SHELL ERROR !!")
    echo "The file confdefs.h created by configure has been removed"
    echo "This may be a problem with your shell; some versions of LINUX"
    echo "have this problem.  See the Installation guide for more"
    echo "information."
    exit 1
fi
cat > conftest.c <<EOF
#include "confdefs.h"
[$2]
int main() { exit(0); }
int t() { [$3] }
EOF
dnl Don't try to run the program, which would prevent cross-configuring.
if eval $compile; then
  ifelse([$1], , , [AC_MSG_RESULT(yes)])
  ifelse([$4], , :, [rm -rf conftest*
  $4
])
ifelse([$5], , , [else
  rm -rf conftest*
  $5
])dnl
   ifelse([$1], , , ifelse([$5], ,else) [AC_MSG_RESULT(no)])
fi
rm -f conftest*]
)dnl
dnl
dnl
dnl checks for compiler characteristics
dnl
dnl
define([AC_CROSS_CHECK],
[AC_PROVIDE([$0])AC_MSG_CHECKING(whether cross-compiling)
# If we cannot run a trivial program, we must be cross compiling.
AC_TEST_PROGRAM([main(){exit(0);}], AC_MSG_RESULT(no), cross_compiling=1;AC_MSG_RESULT(yes))
])dnl
dnl
dnl Append SH style definitions to a file
dnl To generate a site file (for MAKE), use PAC_APPEND_FILE.  This allows
dnl you to use configure to create a likely site file.
dnl
dnl PAC_APPEND_FILE(varname,varvalue,file)
dnl Example: PAC_APPEND_FILE("CC",$CC,"make.site")
dnl
define(PAC_APPEND_FILE,[
if test "$3" = "-" ; then echo "$1=$2" ; else echo "$1=$2" >> $3 ; fi
])dnl
dnl
dnl See if Fortran compiler accepts -Idirectory flag
dnl 
dnl PAC_FORTRAN_HAS_INCDIR(true-action,false-action)
dnl
dnl Fortran compiler is F77 and is passed FFLAGS
dnl
define(PAC_FORTRAN_HAS_INCDIR,[
AC_MSG_CHECKING([for Fortran include argument])
cat > conftest.f <<EOF
       program main
       include 'mpif.h'
       end
EOF
if $F77 $FFLAGS -c -Iinclude conftest.f > /dev/null 2>&1 ; then
    ifelse($1,,true,$1)
    AC_MSG_RESULT([supports -I])
else
    ifelse($2,,true,$2)
    AC_MSG_RESULT([does NOT support -I])
fi
/bin/rm -f conftest.f
])dnl
dnl
dnl Check that signal semantics work correctly
dnl
define(PAC_SIGNALS_WORK,[
AC_MSG_CHECKING([that signals work correctly])
cat >conftest.c <<EOF
#include <signal.h>
static int rc = 0, irc = 1, maxcnt=5;
void handler( sig )
int sig;
{
void (*oldsig)();
oldsig = signal( SIGUSR1, handler );
if (oldsig != handler) rc = 1;
irc = 0;
}
int main(argc, argv)
int argc;
char **argv;
{
(void)signal( SIGUSR1, handler );
kill( getpid(), SIGUSR1 );
while (irc && maxcnt) { sleep(1); maxcnt--;}
return rc;
}
EOF
if eval $CC $CFLAGS -o conftest conftest.c > /dev/null 2>&1 ; then
    if ./conftest ; then
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(Signals reset when used!)
    fi
else
    AC_MSG_RESULT(Could not compile test program!)
fi
/bin/rm -f conftest conftest.c conftest.o
])dnl
dnl
dnl
dnl record top-level directory (this one)
dnl A problem.  Some systems use an NFS automounter.  This can generate
dnl paths of the form /tmp_mnt/... . On SOME systems, that path is
dnl not recognized, and you need to strip off the /tmp_mnt. On others, 
dnl it IS recognized, so you need to leave it in.  Grumble.
dnl The real problem is that OTHER nodes on the same NFS system may not
dnl be able to find a directory based on a /tmp_mnt/... name.
dnl
dnl It is WRONG to use $PWD, since that is maintained only by the C shell,
dnl and if we use it, we may find the 'wrong' directory.  To test this, we
dnl try writing a file to the directory and then looking for it in the 
dnl current directory.  Life would be so much easier if the NFS automounter
dnl worked correctly.
dnl
dnl PAC_GETWD(varname [, filename ] )
dnl 
dnl Set varname to current directory.  Use filename (relative to current
dnl directory) if provided to double check.
dnl
dnl Need a way to use "automounter fix" for this.
dnl
define(PAC_GETWD,[
AC_MSG_CHECKING(for current directory name)
$1=$PWD
if test "${$1}" != "" -a -d "${$1}" ; then 
    if test -r ${$1}/.foo$$ ; then
        /bin/rm -f ${$1}/.foo$$
	/bin/rm -f .foo$$
    fi
    if test -r ${$1}/.foo$$ -o -r .foo$$ ; then
	$1=
    else
	echo "test" > ${$1}/.foo$$
	if test ! -r .foo$$ ; then
	    $1=
	fi
	/bin/rm -f ${$1}/.foo$$
    fi
fi
if test "${$1}" = "" ; then
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
fi
dnl
dnl First, test the PWD is sensible
ifelse($2,,,
if test ! -r ${$1}/$2 ; then
    dnl PWD must be messed up
    $1=`pwd`
    if test ! -r ${$1}/$2 ; then
	print_error "Cannot determine the root directory!" 
        exit 1
    fi
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
    if test ! -d ${$1} ; then 
        print_error "Warning: your default path uses the automounter; this may"
        print_error "cause some problems if you use other NFS-connected systems."
        $1=`pwd`
    fi
fi)
if test -z "${$1}" ; then
    $1=`pwd | sed -e 's%/tmp_mnt/%/%g'`
    if test ! -d ${$1} ; then 
        print_error "Warning: your default path uses the automounter; this may"
        print_error "cause some problems if you use other NFS-connected systems."
        $1=`pwd`
    fi
fi
AC_MSG_RESULT(${$1})
])dnl
dnl
dnl
dnl 
define(PAC_GET_SPECIAL_SYSTEM_INFO,[
#
# We should provide a way to specify a particular IRIX version, rather 
# than requiring the this code to figure everything out.
# In particular, there are IRIX-like systems that do not have the 'hinv'
# command.
#
if test -n "$arch_IRIX"; then
   AC_MSG_CHECKING(for IRIX OS version)
   dnl Every version and machine under IRIX is incompatible with every other
   dnl version.  This block of code replaces a generic "IRIX" arch value 
   dnl with 
   dnl  IRIX_<version>_<chip>
   dnl  For example
   dnl  IRIX_5_4400 (IRIX 5.x, using MIPS 4400)
   osversion=`uname -r | sed 's/\..*//'`
   dnl Note that we need to allow brackets here, so we briefly turn off 
   dnl the macro quotes
   changequote(,)dnl
   dnl Get the second field (looking for 6.1)
   osvminor=`uname -r | sed 's/[0-9]\.\([0-9]*\)\..*/\1/'`
   AC_MSG_RESULT($osversion)
   dnl Get SGI processor count by quick hack
   dnl 7/13/95, bri@sgi.com
   AC_MSG_CHECKING(for IRIX cpucount)
   cpucount=`hinv | grep '[0-9]* [0-9]* MHZ IP[0-9]* Proc' | cut -f 1 -d' '`
   if test "$cpucount" = "" ; then
     cpucount=`hinv | grep 'Processor [0-9]*:' | wc -l | sed -e 's/ //g'`
   fi
   changequote([,])dnl
   if test "$cpucount" = "" ; then
     print_error "Could not determine cpucount."
     print_error "Please send "
     hinv
     print_error "to mpi-bugs@mcs.anl.gov"
     exit 1
   fi
   AC_MSG_RESULT($cpucount)

   dnl 
   dnl Check for fast SGI device
   if test -d mpid/sgi -a "$osversion" -ge 6 -a "$osvminor" -ge 1 -a \
	`uname -s` = "IRIX64" ; then
	if test -z "$device_sgi" ; then
	    echo "Consider using -device=sgi for SGI arrays"
	fi
   elif test -n "$device_sgi" ; then
	print_error "The sgi device requires IRIX64 and version 6.1 or later"
        exit 1
   fi
   dnl
   dnl Set -comm=shared if IRIX MP & COMM=ch_p4 & COMM not explicitly set
   dnl 7/13/95 bri@sgi.com
   if test $cpucount -gt 1 ; then
     if test "$COMM" = "ch_p4" ; then
       if test "$default_comm" = "1" ; then
         echo "IRIX multiprocessor & p4, setting -comm=shared"
         echo "  (configure with -comm=ch_p4 to disable shared memory)"
         COMM="shared"
       fi
     fi
   fi

   AC_MSG_CHECKING(for IRIX cputype)
   dnl The tail -1 is necessary for multiple processor SGI boxes
   dnl We might use this to detect SGI multiprocessors and recommend
   dnl -comm=shared
   cputype=`hinv -t cpu | tail -1 | cut -f 3 -d' '`
   if test -z "$cputype" ; then
	print_error "Could not get cputype from hinv -t cpu command."
	print_error "Please send "
	hinv -t cpu 2>&1
	hinv -t cpu | cut -f 3 -d' ' 2>&1
	print_error "to mpi-bugs@mcs.anl.gov" 
	exit 1
   fi
   AC_MSG_RESULT($cputype)
   dnl echo "checking for osversion and cputype"
   dnl cputype may contain R4400, R2000A/R3000, or something else.  
   dnl We may eventually need to look at it.
   if test -z "$osversion" ; then
	print_error "Could not determine OS version.  Please send" 
        print_error " " 
	uname -a
	print_error "to mpi-bugs@mcs.anl.gov" 
        exit 1
   elif test $osversion = 4 ; then
	dnl Nathan told us that things worked for IRIX 4 as well; 
	dnl however, we need 'ar ts libname' (ranlib) on version 4 but 
	dnl not the others
        true
   elif test $osversion = 5 ; then
	true
   elif test $osversion = 6 ; then
	true
   else 
       print_error "Could not recognize the version of IRIX (got $osversion)"
       print_error "MPICH knows about versions 4, 5 and 6; the version being"
       print_error "returned from uname -r is $osversion."
       print_error "Please send"
       uname -a 2>&1
       hinv 2>&1
       print_error "to mpi-bugs@mcs.anl.gov"
       exit 1
   fi
   AC_MSG_RESULT(getting cputype)
   OLD_ARCH=IRIX
   IRIXARCH="$ARCH_$osversion"
   dnl Now, handle the chip set
   changequote(,)dnl
   cputype=`echo $cputype | sed -e 's%.*/%%' -e 's/R//' | tr -d "[A-Z]"`
   changequote([,])dnl
   case $cputype in 
	3000) ;;
	4000) ;;
	4400) ;;
	4600) ;;
	8000) ;;
        *)
	print_error "Unexpected IRIX/MIPS chipset $cputype.  Please send the output"
	print_error " "
        uname -a 2>&1
        hinv 2>&1 
 	print_error " " 
        print_error "to mpi-bugs@mcs.anl.gov" 
	print_error "MPICH will continue and assume that the cputype is"
        print_error "compatible with a MIPS 4400 processor."
 	print_error " " 
        cputype=4400
	;;
   esac
   AC_MSG_RESULT($cputype)
   IRIXARCH="$IRIXARCH_$cputype"
   echo "IRIX-specific architecture is $IRIXARCH"
fi
])dnl
dnl
dnl Check that ranlib works, and is not just a noisy stub
dnl We do this by creating a small object file
dnl and a trial library, and then ranlib the result.
dnl
dnl Requires that CC, AR, and RANLIB already be defined.
dnl
define(PAC_RANLIB_WORKS,[
AC_MSG_CHECKING(that ranlib works)
cat <<EOF >conftest.c
int a(){return 1;}
EOF
compileonly='$CC -c $CFLAGS conftest.c >/dev/null 2>&1'
if eval $compileonly ; then 
    broken=0;
else
    broken=1;
fi
if test $broken = 1 ; then
    AC_MSG_RESULT(no)
    print_error "Error in creating test object for ranlib!"
else
    arcmd='$AR foo.a conftest.o >/dev/null 2>&1'
    eval $arcmd
    ranlibtest='$RANLIB foo.a >/dev/null 2>&1'
    if eval $ranlibtest ; then
        broken=0;
	AC_MSG_RESULT(yes)
    else
        broken=1;
	AC_MSG_RESULT(no)
    fi
    /bin/rm -f foo.a
    if test $broken = 1 ; then
        print_error "RANLIB ($RANLIB) failed!"
        print_error "Assuming that ranlib is a stub returning non-zero"
        print_error "condition code"
        RANLIB=':'
    fi
fi
rm -f conftest.o conftest.c
])dnl
dnl
dnl PAC_OUTPUT_EXEC(files[,mode]) - takes files (as shell script or others),
dnl and applies configure to the them.  Basically, this is what AC_OUTPUT
dnl should do, but without adding a comment line at the top.
dnl Must be used ONLY after AC_OUTPUT (it needs config.status, which 
dnl AC_OUTPUT creates).
dnl Optionally, set the mode (+x, a+x, etc)
dnl
define(PAC_OUTPUT_EXEC,[
CONFIG_FILES="$1"
export CONFIG_FILES
./config.status
CONFIG_FILES=""
for pac_file in $1 ; do 
    /bin/rm -f .pactmp
    sed -e '1d' $pac_file > .pactmp
    /bin/rm -f $pac_file
    mv .pactmp $pac_file
    ifelse($2,,,chmod $2 $pac_file)
done
])dnl
dnl
dnl PAC_FIND_FCLIB( [carch] )
dnl
dnl Find the libraries needed to link Fortran routines with C main programs
dnl This is ONLY an approximation but DOES handle some simple cases.
dnl Sets FCLIB if it can.  Fortran compiler FULL PATH would help.
dnl
define(PAC_FIND_FCLIB,[
if test -n "$F77" ; then
PAC_PROGRAM_CHECK(FCVal,$F77,,,FCFULLPATH)
AC_MSG_CHECKING([for Fortran libraries to link C programs with])
case $1 in 
    sun4)
    if test "$FCFULLPATH" = /usr/lang/f77 ; then
	# Look for /usr/lang/SC... .   This is tricky, because 
	# we want to find the LATEST versions first
	for dir in /usr/lang/SC2*.*.* /usr/lang/SC2.* /usr/lang/SC2* \
	 	 /usr/lang/SC1.*.* /usr/lang/SC1.* /usr/lang/SC1* ; do
	    if test -d $dir ; then
		if test -s $dir/libF77.a ; then
		    FCLIB="$dir/libF77.a -lm"
		    if test -s $dir/libm.a ; then
			FCLIB="$FCLIB $dir/libm.a"
		    fi
	            break
	        fi
            fi
        done
    fi
    ;;
    solaris)
	# /opt/SUNWspro/SC*/lib/name
	for file in libF77.a libM77.a libsunmath.a ; do
	    for dir in /opt/SUNWspro/SC3.* /opt/SUNWspro/SC3* \
		       /opt/SUNWspro/SC2.* /opt/SUNWspro/SC2* ; do
  	        if test -d $dir ; then
		    if test -s $dir/$file ; then
			FCLIB="$FCLIB $dir/$file"
			break
		    fi
                fi
            done
        done
    ;;

    rs6000)
	for file in /usr/lib/libxlf.a /usr/lib/libxlf90.a ; do
	    if test -s $file ; then
		FCLIB="$FCLIB $file"
            fi
	done
	if test -s /usr/lpp/xlf/lib/lowsys.exp ; then
	    FCLIB="$FCLIB /usr/lpp/xlf/lib/lowsys.exp"
	fi
	;;
    IRIX64|IRIX)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	for lib in fpe sun F77 U77 I77 isam ; do
	    AC_HAVE_LIBRARY($lib,FCLIB="$FCLIB -l$lib")
        done
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    alpha)
	for file in libfor.a libutil.a libFutil.a libots.a ; do
	    if test -s /usr/lib/$file ; then
		FCLIB="$FCLIB /usr/lib/$file"
            fi
	done
    ;;
    freebsd|linux)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	for lib in f2c m ; do
	    AC_HAVE_LIBRARY($lib,FCLIB="$FCLIB -l$lib")
        done
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    hpux)
	for file in libf.a libf.sl ; do
	    if test -s /usr/lib/$file ; then
		FCLIB="$FCLIB /usr/lib/$file"
            fi
	done
    ;;
    paragon)
	AC_MSG_RESULT()
        SaveDEFS="$DEFS"
        SaveLIBS="$LIBS"
	for lib in f ; do
	    AC_HAVE_LIBRARY($lib,FCLIB="$FCLIB -l$lib")
        done
        DEFS="$SaveDEFS"
	LIBS="$SaveLIBS"
    ;;
    *)
    :
    ;;
esac
if test -n "$FCLIB" ; then
    AC_MSG_RESULT(found $FCLIB)
else
    AC_MSG_RESULT(none)
fi
fi
])dnl
