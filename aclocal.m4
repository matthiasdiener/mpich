dnl
dnl Define test for 64-bit pointers
define(PAC_POINTER_64_BITS,
[AC_MSG_CHECKING([for pointers greater than 32 bits])
AC_TEST_PROGRAM([main() { exit(sizeof(void *) <=4); }],
 AC_DEFINE(POINTER_64_BITS)AC_MSG_RESULT(yes),AC_MSG_RESULT(no))
])dnl
define(PAC_INT_LT_POINTER,
[AC_MSG_CHECKING([for int large enough for pointers])
AC_TEST_PROGRAM([main() { exit(sizeof(int) >= sizeof(void*)); }],
 AC_DEFINE(INT_LT_POINTER)AC_MSG_RESULT(no),AC_MSG_RESULT(yes))
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
])
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
])
dnl AC_MSG_CHECKING(FEATURE-DESCRIPTION)
define(AC_FD_MSG,1)
define(AC_MSG_CHECKING,dnl
if test -z "$ac_echo_n" ; then
AC_PROG_ECHO_N
fi
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
        AC_MSG_CHECKING(`cat ccbugs/$CFILE.title`)
        cp $file conftest.c
        broken=1
        if eval $TESTCC $CFLAGS -o conftest conftest.c $LIBS >/dev/null 2>&1 ; then
	    if test -s conftest && (./conftest ; exit) 2>/dev/null ; then
  	        broken=0
	    fi
        fi
        if test $broken = 1 ; then 
	    AC_MSG_RESULT(no)
	    cat ccbugs/$CFILE.txt | sed 's/^/\*\#/g' 
	else
	    AC_MSG_RESULT(yes)
        fi
	/bin/rm -f conftest conftest.c conftest.o 
    done
    # 
    # Now, try the warnings.  Note that this just does compiles, not runs
    for file in ccbugs/ccwarn*.c ; do
        CFILE=`basename $file .c`
        AC_MSG_CHECKING(`cat ccbugs/$CFILE.title`)
        cp $file conftest.c
        if eval $CC $CFLAGS \
	    -DCONFIGURE_ARGS_CLEAN="'"'"'-A -B'"'"'" -c \
	    conftest.c $LIBS > /dev/null 2>&1 ; then
	    AC_MSG_RESULT(yes)
	    true 
	else
	    AC_MSG_RESULT(no)
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
    echo "Cannot check for size of $1 when cross-compiling"
    AC_CV_NAME=0
else
AC_MSG_CHECKING(size of $1)
AC_TEST_PROGRAM([#include <stdio.h>
main()
{
  FILE *f=fopen("cftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", sizeof($1));
  exit(0);
}], AC_CV_NAME=`cat cftestval`,AC_CV_NAME=0)
rm -f cftestval
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
else
    AC_MSG_RESULT(no - whew)
fi
str=""
])
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
	    dnl LIB_LIST="$LIB_LIST -nx"
	    CFLAGS="$CFLAGS -nx"
	  fi
	;;
   cm5) CC=cc ; GCC="" ;   if test -z "$USERCLINKER" ; then
		      CLINKER="cmmd-ld -comp $CC"
		  fi ;;
   cray_t3d)        CC=/mpp/bin/cc ; CFLAGS="$CFLAGS -Tcray-t3d" ; GCC="" 
                    if test -z "$USERCLINKER" ; then 
	            CLINKER="$CC -Tcray-t3d" ; fi ;;
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
    convex_spp)  CC="/usr/convex/bin/cc" ;;
    ibmpoe)
         dnl This is intended for the Standard POE/MPL version
	 CCval=
         AC_PROGRAMS_CHECK(CCval,mpCC mpcc "$CC")
         if test -n "$CCval" ; then
	    CC=$CCval
	    if test "$CC" = mpcc ; then 
	        TESTCC=xlc
	    elif test "$CC" = mpCC ; then
        	TESTCC=xlC
	    fi
         else
            printerror "Could not find mpCC or mpcc!"
	    exit 1
         fi
    ;;
    meiko) 
      dnl /opt/SUNWspro/bin/cc,/opt/apogee/bin/apcc,/opt/PGI/bin/cc,
      dnl /opt/gcc/bin/gcc
      CCval=''
      AC_PROGRAMS_CHECK(CCval,cc apcc pgcc gcc)
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
esac
fi
if test -z "$USERCLINKER" -a -z "$CLINKER" ; then
    CLINKER="$CC"
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
dnl AIX requires -bI:/usr/lpp/xlf/lib/lowsys.exp
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
define(PAC_TK_VERSION,
AC_MSG_CHECKING(for version of TK)
[/bin/rm -f conftestval
CFLAGSsave="$CFLAGS"
CFLAGS="$CFLAGS -I$TK_DIR/include $XINCLUDES"
AC_TEST_PROGRAM([#include "tk.h"
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
dnl Append SH style definitions to a file
dnl To generate a site file (for MAKE), use PAC_APPEND_FILE.  This allows
dnl you to use configure to create a likely site file.
dnl
dnl PAC_APPEND_FILE(varname,varvalue,file)
dnl Example: PAC_APPEND_FILE("CC",$CC,"make.site")
dnl
define(PAC_APPEND_FILE,[
if test "$3" = "-" ; then echo "$1=$2" ; else echo "$1=$2" >> $3 ; fi
])
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
])
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
	print_error Signals reset when used!
    fi
else
    AC_MSG_RESULT(Could not compile test program!)
fi
/bin/rm -f conftest conftest.c conftest.o
])

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
    if test -r ${$1}/.foo ; then
        /bin/rm -f ${$1}/.foo
	/bin/rm -f .foo
    fi
    if test -r ${$1}/.foo -o -r .foo ; then
	$1=
    else
	echo "test" > ${$1}/.foo
	if test ! -r .foo ; then
	    $1=
	fi
	/bin/rm -f ${$1}/.foo
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
])
dnl
dnl
dnl 
define(PAC_GET_SPECIAL_SYSTEM_INFO,[
dnl
dnl We should provide a way to specify a particular IRIX version, rather 
dnl than requiring the this code to figure everything out.
dnl In particular, there are IRIX-like systems that do not have the 'hinv'
dnl command.
dnl
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
])
