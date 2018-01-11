dnl
dnl Define test for 64-bit pointers
define(PAC_POINTER_64_BITS,
[echo checking for pointers greater than 32 bits
AC_TEST_PROGRAM([main() { exit(sizeof(void *) <=4); }],
 AC_DEFINE(POINTER_64_BITS)AC_MSG_RESULT(yes))
])dnl
define(PAC_INT_LT_POINTER,
[echo checking for int large enough for pointers
AC_TEST_PROGRAM([main() { exit(sizeof(int) >= sizeof(void*)); }],
 AC_DEFINE(INT_LT_POINTER)AC_MSG_RESULT(no))
])dnl
dnl
dnl Define the test for the long long int type
define(PAC_LONG_LONG_INT,
[AC_REQUIRE([AC_PROG_CC])dnl
echo checking for long long int
AC_TEST_PROGRAM([int main() {
/* See long double test; this handles the possibility that long long int 
   has the same problem on some systems */
exit(sizeof(long long int) < sizeof(long)); }],
AC_DEFINE(HAVE_LONG_LONG_INT)AC_MSG_RESULT(yes))
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
echo "checking for wish"
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
    /opt/bin \
    /usr/unsupported \
    /usr/bin \
    /bin ; do
    if test -x $dir/wish ; then
	wishloc=$dir/wish
        break
    fi
done
fi
if test -n "$wishloc" ; then 
  AC_MSG_RESULT(found $wishloc)
fi])dnl
define(PAC_FIND_TCL,[
# Look for Tcl
if test -z "$TCL_DIR" ; then
echo checking for Tcl
for dir in \
    /usr \
    /usr/local \
    /usr/local/tcl7.3 \
    /usr/local/tcl7.3-tk3.6 \
    /usr/local/tcl7.0 \
    /usr/local/tcl7.0-tk3.3 \
    ~/tcl \
    ~/tcl7.3 \
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
  AC_MSG_RESULT(found $TCL_DIR)
fi
# Look for Tk
if test -z "$TK_DIR" ; then
echo checking for Tk
for dir in \
    /usr \
    /usr/local \
    /usr/local/tk3.6 \
    /usr/local/tcl7.3-tk3.6 \
    /usr/local/tk3.3 \
    /usr/local/tcl7.0-tk3.3 \
    ~/tcl \
    ~/tcl7.3 \
    /opt/local \
    /opt/local/tk3.6 \
    /local/encap/tk-3.4 ; do
    if test -r $dir/include/tk.h -a -r $dir/lib/libtk.a ; then
	TK_DIR=$dir
	break
    fi
done
fi
if test -n "$TK_DIR" ; then 
  AC_MSG_RESULT(found $TK_DIR)
fi
])
dnl
dnl The AC_CHECK_HEADER assumes that you can use cpp to check for 
dnl headers for the (CROSS!) compiler.  This is ridiculous.
dnl I've FIXED the code from version 2
dnl
dnl ### Printing messages
dnl
dnl
dnl AC_MSG_CHECKING(FEATURE-DESCRIPTION)
define(AC_FD_MSG,1)
define(AC_MSG_CHECKING,dnl
[echo $ac_n "checking $1""... $ac_c" 1>&AC_FD_MSG])
dnl
dnl AC_CHECKING(FEATURE-DESCRIPTION)
define(AC_CHECKING,dnl
[echo "checking $1" 1>&AC_FD_MSG])
dnl
dnl AC_MSG_RESULT(RESULT-DESCRIPTION)
define(AC_MSG_RESULT,dnl
[echo "$ac_t""$1" 1>&AC_FD_MSG])
dnl
dnl PAC_CHECK_HEADER(HEADER-FILE, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
dnl
dnl BUG: AIX 4.1 can't handle a \055 (octal for -) in a tr string (sometimes;
dnl it works from the shell but not within a file)
dnl I've removed that an hoped that no header will include a - in the
dnl name
dnl
define(PAC_CHECK_HEADER,dnl
[dnl Do the transliteration at runtime so arg 1 can be a shell variable.
changequote(,)dnl
ac_safe=`echo "$1" | tr '[a-z]./' '[A-Z]__'`
changequote([,])dnl
AC_MSG_CHECKING([for $1])
dnl AC_CACHE_VAL(ac_cv_header_$ac_safe,[dnl
AC_COMPILE_CHECK(,[#include <$1>],main();,eval "ac_cv_header_$ac_safe=yes",
  eval "ac_cv_header_$ac_safe=no")dnl])dnl
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no)
ifelse([$3], , , [$3
])dnl
fi
])
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
])
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
])
dnl
dnl Check that the compile actually runs.  Perform first arg is yes,
dnl second if false
dnl PAC_CHECK_COMPILER_OK(true-action, false-action)
dnl
define(PAC_CHECK_COMPILER_OK,[
AC_MSG_CHECKING(that the compiler $CC runs)
AC_COMPILE_CHECK(,,,eval "ac_cv_ccworks=yes",eval "ac_cv_ccworks=no")
AC_MSG_RESULT($ac_cv_ccworks)
if test $ac_cv_ccworks = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])

dnl Test the compiler to see if it actually works.  First, check to see
dnl if the compiler works at all
dnl Uses TESTCC, not CC
dnl
define(PAC_CORRECT_COMPILER,[
if test -d ccbugs ; then 
    if test -z "$TESTCC" ; then TESTCC="$CC" ; fi
    for file in ccbugs/ccfail*.c ; do
        CFILE=`basename $file .c`
        echo `cat ccbugs/$CFILE.title`
        cp $file conftest.c
        broken=1
        if eval $TESTCC $CFLAGS -o conftest conftest.c $LIBS >/dev/null 2>&1 ; then
	    if test -s conftest && (./conftest ; exit) 2>/dev/null ; then
  	        broken=0
	    fi
        fi
        if test $broken = 1 ; then 
	    cat ccbugs/$CFILE.txt | sed 's/^/\*\#/g' 
        fi
	/bin/rm -f conftest conftest.[co]
    done
    # 
    # Now, try the warnings.  Note that this just does compiles, not runs
    for file in ccbugs/ccwarn*.c ; do
        CFILE=`basename $file .c`
        echo `cat ccbugs/$CFILE.title`
        cp $file conftest.c
        if eval $CC $CFLAGS \
	    -DCONFIGURE_ARGS_CLEAN="'"'"'-A -B'"'"'" -c \
	    conftest.c $LIBS > /dev/null 2>&1 ; then
	    true 
	else
	    cat ccbugs/$CFILE.txt | sed 's/^/\*\#/g' 
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
	echo "Compiler $CC appears broken; aborting configure..."
        exit 1
    fi
fi
])
dnl
dnl
dnl PAC_PROGRAM_CHECK(VARIABLE, PROG-TO-CHECK-FOR, VALUE-IF-FOUND
dnl               [, VALUE-IF-NOT-FOUND])
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
  if test "$ac_first_char" = "/" -a -x $2 ; then
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
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

dnl
dnl PAC_CHECK_SIZEOF(TYPE)
define(PAC_CHECK_SIZEOF,
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(sizeof_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_sizeof_$1, [ *], [_p]))dnl
changequote([, ])dnl
dnl Can only do this test if not cross-compiling
if test $cross_compiling = 1 ; then
    echo "Can not check for size of $1 when cross-compiling"
    AC_CV_NAME=0
else
AC_MSG_CHECKING(size of $1)
AC_TEST_PROGRAM([#include <stdio.h>
main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", sizeof($1));
  exit(0);
}], AC_CV_NAME=`cat conftestval`,AC_CV_NAME=0)
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
])
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
    fi
fi
/bin/rm -f conftest
str=""
])
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
fi
str=""
])
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
dnl PAC_GET_CC
dnl Uses USERCC, CC, CLINKER, ARCH, arch_xxx.  Looks for special versions
dnl of C compilers, particularly cross compilers.  May also set some
dnl compile flags.  Clears GCC if it sets CC.  Calls "print_error" for
dnl error messages
dnl
define(PAC_GET_CC,[
if test -z "$USERCC" ; then
case $ARCH in 
   intelnx|paragon) CC=icc ; CLINKER=$CC ; GCC="" ;;
   cray_t3d)        CC=/mpp/bin/cc ; CFLAGS="$CFLAGS -Tcray-t3d" 
	            CLINKER="$CC -Tcray-t3d" ;;
   hpux) if test "$CC != "gcc" ; then
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
    alpha)   CFLAGS="$CFLAGS -DDBX_IS_OSF" ;;
esac
fi
])
dnl
dnl Fortran runtime for Fortran/C linking
dnl On suns, try
dnl FC_LIB          =/usr/local/lang/SC2.0.1/libM77.a \ 
dnl              /usr/local/lang/SC2.0.1/libF77.a -lm \
dnl              /usr/local/lang/SC2.0.1/libm.a \
dnl              /usr/local/lang/SC2.0.1/libansi.a
dnl
