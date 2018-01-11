dnl
dnl rcsid = "$Header: /nfs/globus1/src/master/configuration/old-aclocal.m4,v 1.2 1996/10/21 05:56:26 carl Exp $"
dnl
dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                         AC_PROG_HPCC                                ----
dnl ----                                                                     ----
dnl ----                 Set compiler options based on the                   ----
dnl ----                 selected threads package                            ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
dnl -- First get what the system thinks is CC
dnl -- The look at the threads package and platform to find anything
dnl -- that looks to be a special case
dnl -- but try cc before gcc
AC_DEFUN(AC_PROG_HPCC,
[case $target in
    sparc-sun-sunos* ) ;;
    * ) AC_CHECK_PROGS(CC, cc gcc, cc) ;;
esac
AC_PROG_CC
AC_CACHE_VAL(ac_cv_prog_usercc, [dnl
AC_ARG_WITH(cc,
	[--with-cc=comp             use comp as the compiler],
	[ac_cv_prog_usercc="$withval"])
])
AC_CACHE_VAL(ac_cv_prog_userccfull, [dnl
AC_ARG_WITH(cc-full,
	[--with-cc-full=comp        use comp as the compiler for threaded builds],
	[ac_cv_prog_userccfull="$withval"],
	[if test "x$ac_cv_prog_usercc" != "x"; then
	  ac_cv_prog_userccfull="$ac_cv_prog_usercc"
	 fi])
])
AC_CACHE_VAL(ac_cv_prog_usercclite, [dnl
AC_ARG_WITH(cc-lite,
	[--with-cc-lite=comp        use comp as the compiler for lite builds],
	[ac_cv_prog_usercclite="$withval"],
	[if test "x$ac_cv_prog_usercc" != "x"; then
	  ac_cv_prog_usercclite="$ac_cv_prog_usercc"
	 fi])
])
case "$target--$ac_cv_sys_threads" in
  rs6000-ibm-aix3*--pthreads )
    dnl -- at this point I am assuming pthreads is the IBM DCE threads package
    if test "x$ac_cv_prog_userccfull" = "x"; then
      CCfull="/usr/bin/xlc_r"
    else
      CCfull="$ac_cv_prog_userccfull"
    fi
    dnl -- to prevent compiling half the system with xlc_r and the other half
    dnl -- with gcc, I am going to set CClite if the user has not specified it.
    if test "x$ac_cv_prog_usercclite" = "x"; then
      CClite="/usr/bin/xlc"
    else
      CClite="$ac_cv_prog_usercclite"
    fi
  ;;
  *-ibm-aix4*--pthreads )
    if test "x$ac_cv_prog_userccfull" = "x"; then
      CCfull="/usr/bin/xlc_r"
    else
      CCfull="$ac_cv_prog_userccfull"
    fi
    if test "x$ac_cv_prog_usercclite" = "x"; then
      CClite="/usr/bin/xlc"
    else
      CClite="$ac_cv_prog_usercclite"
    fi
  ;;
  *-hp-hpux9*--* )
    dnl -- The HPUX 9 C compiler requires -Aa when compiling ANSI code
    if test "x$ac_cv_prog_userccfull" = "x"; then
      CCfull="$CC"
    else
      CCfull="$ac_cv_prog_userccfull"
    fi
    if test "x$ac_cv_prog_usercclite" = "x"; then
      CClite="$CC"
    else
      CClite="$ac_cv_prog_usercclite"
    fi

    case "$CCfull" in
      *gcc ) ;;
      *cc )
        CCOPTS_full="-Aa $CCOPTS_full"
        LDOPTS_full="$LDOPTS_full -Wl,-a,archive"
      ;;
    esac
    case "$CClite" in
      *gcc ) ;;
      *cc )
        CCOPTS_lite="-Aa $CCOPTS_lite"
      ;;
    esac
    dnl -- For HP's that just gotta have -Aa -D_HPUX_SOURCE even for
    dnl -- our simple feature test programs to compile, we have to
    dnl -- set CC and CFLAGS otherwise things like the flock test bomb
    CC="$CClite"
    CFLAGS="-Aa -D_HPUX_SOURCE"
  ;;
  sparc-sun-solaris2*--solaristhreads )
    dnl -- The Solaris C compiler needs the -mt flag when compiling
    dnl -- and linking against the Solaris threads library.
    if test "x$ac_cv_prog_userccfull" = "x"; then
      CCfull="$CC"
    else
      CCfull="$ac_cv_prog_userccfull"
    fi
    if test "x$ac_cv_prog_usercclite" = "x"; then
      CClite="$CC"
    else
      CClite="$ac_cv_prog_usercclite"
    fi

    case "$CCfull" in
      *gcc ) ;;
      *cc )
        CCOPTS_full="-mt $CCOPTS_full"
        LDOPTS_full="-mt $LDOPTS_full"
      ;;
    esac
    case "$CClite" in
      *gcc ) ;;
      *cc )
        CCOPTS_lite="-mt $CCOPTS_lite"
        LDOPTS_lite="-mt $LDOPTS_lite"
      ;;
    esac
  ;;
  mips-sgi-irix5* )
    if test "x$ac_cv_prog_userccfull" = "x"; then
      CCfull="ncc"
      CCOPTS_full="$CCOPTS_full -woff 3262"
    fi
    if test "x$ac_cv_prog_usercclite" = "x"; then
      CClite="ncc"
      CCOPTS_lite="$CCOPTS_lite -woff 3262"
    fi
  ;;
  * )
    CCfull="$CC"
    CClite="$CC"
  ;;
esac
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                         AC_PROG_CC_SHARED                           ----
dnl ----                                                                     ----
dnl ----                 Determine what flags and commands are               ----
dnl ----                 needed to generate shared libraries                 ----
dnl ----                                                                     ----
dnl ----                 Because almost every different vendor               ----
dnl ----                 use a completely non-portable set of                ----
dnl ----                 flags and procedures, this macro                    ----
dnl ----                 pretty much has to end up being a                   ----
dnl ----                 big case statement.                                 ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PROG_CC_SHARED,
[CC_SHARED_OPTS=""
MK_SHARED_LIB=""
link_makefile="makefile.lnk.shr"
AC_MSG_CHECKING(how to make shared libraries)
echo "$target--$CC"
case "$target--$CC" in
    hppa*-hp-hpux*--*gcc )
      CC_SHARED_OPTS='-fPIC'
      MK_SHARED_LIB='ld -b -o $[@]'
    ;;
    hppa*-hp-hpux*--*cc )
      CC_SHARED_OPTS='+Z'
      MK_SHARED_LIB='ld -b -o $[@]'
    ;;
    mips-sgi-irix5*--*gcc )
      CC_SHARED_OPTS='-fPIC'
      MK_SHARED_LIB='ld -shared -rdata_shared -soname `basename $[@]` -o $[@]'
    ;;
    mips-sgi-irix5*--*cc )
      CC_SHARED_OPTS='-KPIC'
      MK_SHARED_LIB='ld -shared -rdata_shared -soname `basename $[@]` -o $[@]'
    ;;
    *-*-linux*--*gcc )
      CC_SHARED_OPTS='-fPIC'
      MK_SHARED_LIB="gcc -o \$[@].\$(REL_VERSION) -shared -Wl,-soname,\`basename \$[@].\$(ABI_VERSION)\`,-stats"
dnl   LOCAL_LDFLAGS='-Wl,-rpath,../lib'
    ;;
    *-*-netbsd*--*gcc )
      CC_SHARED_OPTS='-fpic -DPIC'
      MK_SHARED_LIB='ld -Bshareable -o $[@]'
    ;;
    *-*-osf1*--* )
      CC_SHARED_OPTS=''
      MK_SHARED_LIB="ld -o \$[@].\$(REL_VERSION) -shared -soname \`basename \$[@].\$(ABI_VERSION)\`"
dnl   LOCAL_LDFLAGS='-Wl,-rpath,../lib'
    ;;
    rs6000-ibm-aix3*--*gcc )
    ;;
    rs6000-ibm-aix3*--*cc )
      link_makefile="makefile.lnk.ibm"
      AC_PATH_PROG(NM,"nm")
      AC_SUBST(NM)
      AC_PROG_AWK
      dnl -- use the lite compiler since the -lpthreads is in the makefile.lnk.ibm
      dnl      MK_SHARED_LIB="$CClite"
      dnl -- the above does not work for the SP so use CCfull
      MK_SHARED_LIB="$CCfull"
    ;;
    sparc-sun-sunos5*--*gcc | sparc-sun-solaris2*--*gcc )
      CC_SHARED_OPTS="-fpic"
      MK_SHARED_LIB="ld -d y -G -o \$[@].\$(REL_VERSION)"
    ;;
    sparc-sun-sunos5*--*cc | sparc-sun-solaris2*--*cc )
      LIBFILE="so"
      CC_SHARED_OPTS="-KPIC"
      MK_SHARED_LIB="ld -d y -G -o \$[@]"
    ;;
    sparc-sun-sunos4*--*gcc | sparc-sun-solaris1*--*gcc )
      CC_SHARED_OPTS="-fpic"
      MK_SHARED_LIB="ld -o \$[@].\$(REL_VERSION)"
    ;;
    sparc-sun-sunos4*--*cc | sparc-sun-solaris1*--*cc )
      CC_SHARED_OPTS="-pic"
      MK_SHARED_LIB="ld -o \$[@].\$(REL_VERSION)"
    ;;
    *-*-sysv4--* )
      CC_SHARED_OPTS='-KPIC'
      MK_SHARED_LIB='ld -d y -G -o $[@]'
    ;;
    * )
      link_makefile="makefile.lnk"
    ;;
  esac
  if test "x$link_makefile" = "xmakefile.lnk"; then
    AC_MSG_RESULT(unknown)
  else
    AC_MSG_RESULT(found)
  fi
  dnl -- if the default "makefile.lnk" was not chosen, nor was a special link file
  dnl -- such as makefile.lnk.ibm, then we should use the standard makefile.lnk.shr
  if test "x$link_makefile" = "x"; then
    link_makefile="makefile.lnk.shr"
  fi
  AC_SUBST(CC_SHARED_OPTS)
  AC_SUBST(MK_SHARED_LIB)
])

dnl -----------------------------------------------------------------
dnl ---  The options matrix for what to build is indicated below
dnl ---  F=Full L=Lite X=invalid
dnl ---
dnl ---                                with_threads
dnl ---                      	Yes	No	Unknown
dnl ---
dnl ---                Yes	FL	L	L
dnl ---  with_lite     No	F	X	F
dnl ---                Unknown	FL	L	FL
dnl ---
dnl ---
dnl -----------------------------------------------------------------
AC_DEFUN(AC_BUILD_SETS,
[ac_cv_build_full="no"
ac_cv_build_lite="no"
case "$ac_cv_acarg_threads-$ac_cv_acarg_lite" in
  yes-no )		ac_cv_build_full="yes" ;;
  unknown-no )		ac_cv_build_full="yes" ;;
  no-no )		AC_MSG_ERROR(nothing to build specify full or lite) ;;
  yes-unknown )		ac_cv_build_full="yes"; ac_cv_build_lite="yes" ;;
  unknown-unknown )	ac_cv_build_full="yes"; ac_cv_build_lite="yes" ;;
  no-unknown )		ac_cv_build_lite="yes" ;;
  yes-yes )		ac_cv_build_full="yes"; ac_cv_build_lite="yes" ;;
  unknown-yes )		ac_cv_build_lite="yes" ;;
  no-yes )		ac_cv_build_lite="yes" ;;
  *-yes )		ac_cv_build_full="yes"; ac_cv_build_lite="yes" ;;
  *-unknown )		ac_cv_build_full="yes"; ac_cv_build_lite="yes" ;;
  *-no )		ac_cv_build_full="yes" ;;
esac
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_TCP                                ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports TCP/IP            ----
dnl ----                 communication.                                      ----
dnl ----                 Add pr_tcp to the build if it does                  ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_SYS_TCP,
[case "$target" in
  sparc-sun-sunos5* | sparc-sun-solaris2* )
    LIBS="$LIBS -lsocket -lnsl"
  ;;
esac
AC_CHECK_FUNCS(socket connect listen bind accept select getsockname,
[filegoing="yes"],
[proto_tcp="no"])
if test "$proto_tcp" != "no"; then
  proto_tcp="yes"
  AC_DEFINE(HAVE_TCP)
  if test "$start_ss" != "no"; then
    start_ss="yes"
    AC_DEFINE(HAVE_SS)
  fi
fi
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_SHM                                ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports System V           ----
dnl ----                 shared memory communication.                        ----
dnl ----                 Add pr_shm to the build if it does                  ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_SYS_SHM,
[dnl currently don't automatically install shared memory--make the user
dnl specify it from the command line.
proto_shm="no"
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_FORK                               ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports                   ----
dnl ----                 creating processes with the fork                    ----
dnl ----                 system call                                         ----
dnl ----                 Add st_fork to the build if it does                 ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_SYS_FORK,
[AC_CHECK_FUNCS(fork1 fork,
[start_fork="yes"])  dnl  -- we found at least one version of a fork
if test "$start_fork" != "yes"; then
  start_fork="no"
fi
])
dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_PROG_RSH                               ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports                   ----
dnl ----                 creating processes with the rsh                     ----
dnl ----                 program                                             ----
dnl ----                 Add st_rsh to the build if it does                  ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PROG_RSH,
[AC_CHECK_PROGS(rsh_prog, remsh rsh,no)
if test "$rsh_prog" != "no"; then
  AC_PATH_PROG(RSH,"$rsh_prog")
  AC_DEFINE_UNQUOTED(NEXUS_RSH,"$RSH")
  AC_DEFINE(HAVE_RSH)
  start_rsh="yes"
else
  start_rsh="no"
fi
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_MPL                                ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports                   ----
dnl ----                 the IBM MPL protocol (used in SP/2's)               ----
dnl ----                                                                     ----
dnl ----                 Add pr_mpl and st_mpl to the build if it does       ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------

AC_DEFUN(AC_SYS_MPL,
[AC_MSG_CHECKING(for MPL)
AC_CACHE_VAL(ac_cv_sys_mpl, [dnl
if test "$target_os" = "aix3.2.5" || test "$target_os" = "aix4.1.1"; then
  poepackage="`/usr/bin/lslpp -clq | grep \"/usr/lib/objrepos.*Parallel Operating Environment\" | wc -l`"
  case "$poepackage" in
    *1 )
      ac_cv_sys_mpl="yes"
      AC_DEFINE(NEXUS_DONT_CATCH_SIGSEGV)
      AC_DEFINE(NEXUS_LISTENER_EXEC_HACK)
      AC_DEFINE(HAVE_MPL)
    ;;
    * )
      ac_cv_sys_mpl="no"
    ;;
  esac
else
  ac_cv_sys_mpl="no"
fi])
AC_MSG_RESULT($ac_cv_sys_mpl)
proto_mpl="$ac_cv_sys_mpl"
start_mpl="$ac_cv_sys_mpl"
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_INX                                ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports                   ----
dnl ----                 the Intel NX protocol (used in Paragons)            ----
dnl ----                                                                     ----
dnl ----                 Add pr_inx and st_inx to the build if it does       ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------

AC_DEFUN(AC_SYS_INX,
[AC_MSG_CHECKING(for NX)
AC_CACHE_VAL(ac_cv_sys_inx, [dnl
if test "$target" = "i860-intel-osf1"; then
  ac_cv_sys_inx="yes"
  AC_DEFINE(HAVE_INX)
else
  ac_cv_sys_inx="no"
fi])
AC_MSG_RESULT($ac_cv_sys_inx)
proto_inx="$ac_cv_sys_inx"
start_inx="$ac_cv_sys_inx"
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_IBMDS                              ----
dnl ----                                                                     ----
dnl ----                 Determine if this system supports                   ----
dnl ----                 data segment cloning under AIX 3.2.5                ----
dnl ----                                                                     ----
dnl ----                 Add st_ibmds to the full build if it does           ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_SYS_IBMDS,
[AC_MSG_CHECKING(for IBMDS)
AC_CACHE_VAL(ac_cv_sys_ibmds, [dnl
if test "$target_os" = "aix3.2.5"; then
  ac_cv_sys_ibmds="yes"
else
  ac_cv_sys_ibmds="no"
fi
])
AC_MSG_RESULT($ac_cv_sys_ibmds)
if test "$ac_cv_sys_ibmds" = "yes"; then
  AC_DEFINE(HAVE_IBMDS)
fi
start_ibmds="$ac_cv_sys_ibmds"
])

AC_DEFUN(AC_SYS_X_ACC,
[AC_MSG_CHECKING(for X_ACC)
    AC_CACHE_VAL(ac_cv_sys_x_acc, [dnl
	AC_EGREP_CPP(zowie,
[#include <unistd.h>
#ifdef X_ACC
    zowie
#endif
], ac_cv_sys_x_acc="yes", ac_cv_sys_x_acc="no") dnl --End AC_EGREP_CPP()
]) dnl --End AC_CACHE_VAL()
    if test "$ac_cv_sys_x_acc" = "yes"; then
	AC_DEFINE(HAVE_X_ACC)
    fi
    AC_MSG_RESULT($ac_cv_sys_x_acc)
]) dnl --End AC_SYS_ACC()

AC_DEFUN(AC_SYS_X_OK,
[AC_MSG_CHECKING(for X_OK)
AC_CACHE_VAL(ac_cv_sys_x_ok, [dnl
AC_EGREP_CPP(zowie,
[#include <unistd.h>
#ifdef X_OK
  zowie
#endif
], ac_cv_sys_x_ok="yes", ac_cv_sys_x_ok="no")
dnl
dnl --Of course gcc on the NeXT is braindead, so it doesn't include the
dnl --right header files, so we have to check special for it!!!
dnl
ac_cv_sys_x_ok="yes"
])
if test "$ac_cv_sys_x_ok" = "yes"; then
  AC_DEFINE(HAVE_X_OK)
fi
AC_MSG_RESULT($ac_cv_sys_x_ok)
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_C_FLAGS                                ----
dnl ----                                                                     ----
dnl ----                 Determine the flags used to specify                 ----
dnl ----                 debugging and optimization                          ----
dnl ----                                                                     ----
dnl ----                 Set the variables CFLAGS_O and CFLAGS_G             ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_C_FLAGS,
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(for optimization flag)
AC_CACHE_VAL(ac_cv_c_flags_o, [dnl
if test "$ac_cv_prog_gcc" = "yes"; then
  ac_cv_c_flags_o="-O3"
else
  case "$target" in
    sparc-sun-sunos5* | sparc-sun-solaris2* )
      ac_cv_c_flags_o="-xO3"
    ;;
    *c90* )
      ac_cv_c_flags_o="-O3"
    ;;
    * )
      ac_cv_c_flags_o="-O"
    ;;
  esac
fi
])
CFLAGS_O="$ac_cv_c_flags_o"
AC_MSG_RESULT($ac_cv_c_flags_o)
AC_MSG_CHECKING(for debugging flag)
AC_CACHE_VAL(ac_cv_c_flags_g, [dnl
if test "$ac_cv_prog_gcc" = "yes"; then
  ac_cv_c_flags_g="-g -Wall"
else
  ac_cv_c_flags_g="-g"
fi
])
CFLAGS_G="$ac_cv_c_flags_g"
AC_MSG_RESULT($ac_cv_c_flags_g)
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                           AC_SYS_CLASSTYPE                          ----
dnl ----                                                                     ----
dnl ----                 Determine the wordsize and endian layout            ----
dnl ----                 of the target platform                              ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_SYS_CLASSTYPE,
[case "$target" in
  *c90* )
    AC_DEFINE(SYS_CLASS_TYPE,CLASS_TYPE_CRAYC90)
  ;;
  * )
    case "$ac_cv_c_bigendian-$ac_cv_sizeof_int_p" in
    yes-4 )
      AC_DEFINE(SYS_CLASS_TYPE,CLASS_TYPE_32BIT_BE)
    ;;
    yes-8 )
      AC_DEFINE(SYS_CLASS_TYPE,CLASS_TYPE_64BIT_BE)
    ;;
    no-4 )
      AC_DEFINE(SYS_CLASS_TYPE,CLASS_TYPE_32BIT_LE)
    ;;
    no-8 )
      AC_DEFINE(SYS_CLASS_TYPE,CLASS_TYPE_64BIT_LE)
    ;;
    * )
      AC_MSG_ERROR(Class type could not be determined)
    ;;
    esac
  ;;
esac
])


AC_DEFUN(AC_FIND_USER_INCLUDE,[
AC_MSG_CHECKING([for include directory for $1])
ac_find_inc_dir=""
for dir in $2 \
	/usr \
	/usr/include \
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
	    ac_find_inc_dir=$dir
	    break
	fi
	if test -r $dir/include/$1.h ; then
	    ac_find_inc_dir=$dir/include
	    break
	fi
dnl	if test -r $dir/lib/lib$1.a ; then
dnl	    ac_find_lib_file=$dir/lib/lib$1.a
dnl	    break
dnl	fi
done
if test -n "$ac_find_inc_dir" ; then
  AC_MSG_RESULT(found $ac_find_inc_dir)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])

AC_DEFUN(AC_FIND_USER_LIB,[
AC_MSG_CHECKING([for library $1])
ac_find_lib_file=""
for dir in $2 \
	/usr \
	/usr/lib \
	/usr/local \
	/usr/local/$1 \
	/usr/contrib \
	/usr/contrib/$1 \
	$HOME/$1 \
	/opt/$1 \
	/opt/local \
	/opt/local/$1 \
	/local/encap/$1 ; do
  for ext in a so; do
	if test -r $dir/$1.$ext ; then
	    ac_find_lib_file=$dir/$1.$ext
	    ac_find_lib_dir=$dir
	    break
	fi
	if test -r $dir/lib$1.$ext ; then
	    ac_find_lib_file=$dir/lib$1.$ext
	    ac_find_lib_dir=$dir
	    break
	fi
	if test -r $dir/lib/$1.$ext ; then
	    ac_find_lib_file=$dir/lib/$1.$ext
	    ac_find_lib_dir=$dir/lib
	    break
	fi
	if test -r $dir/lib/lib$1.$ext ; then
	    ac_find_lib_file=$dir/lib/lib$1.$ext
	    ac_find_lib_dir=$dir/lib
	    break
	fi
  done
done
if test -n "$ac_find_lib_file" ; then
  AC_MSG_RESULT(found $ac_find_lib_file)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])


dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                     AC_PATH_THREADS                                 ----
dnl ----                                                                     ----
dnl ----   Read any thread based arguments, determine build sets,            ----
dnl ----   then determine thread package to use.                             ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
dnl -----------------------------------------------------------------
dnl --- if no --with-threads option is specified, we mark it with
dnl --- the value "unknown" and will determine whether or not
dnl --- we need to build the threaded version
dnl -----------------------------------------------------------------
dnl -----------------------------------------------------------------
dnl --- if no --with-lite option is specified, we mark it with
dnl --- the value "unknown" and will determine whether or not
dnl --- we need to build the lite version
dnl -----------------------------------------------------------------
dnl -----------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS,
[AC_MSG_CHECKING(for threads arg)
AC_CACHE_VAL(ac_cv_acarg_threads, [dnl
AC_ARG_WITH(threads,
	[  --with-threads[=package]  use default or specify package],
[],
[ with_threads="unknown" ])
ac_cv_acarg_threads="$with_threads"
])
AC_MSG_RESULT($ac_cv_acarg_threads)
dnl -----------------------------------------------------------------
AC_MSG_CHECKING(for lite arg)
AC_CACHE_VAL(ac_cv_acarg_lite, [dnl
AC_ARG_WITH(lite,
	[  --with-lite             build Lite version also],
[],
[ with_lite="unknown" ])
ac_cv_acarg_lite="$with_lite"
])
AC_MSG_RESULT($ac_cv_acarg_lite)
dnl -----------------------------------------------------------------

AC_MSG_CHECKING(for full build)
AC_CACHE_VAL(ac_cv_build_full, [AC_BUILD_SETS])
AC_MSG_RESULT($ac_cv_build_full)

AC_MSG_CHECKING(for lite build)
AC_CACHE_VAL(ac_cv_build_lite, [AC_BUILD_SETS])
AC_MSG_RESULT($ac_cv_build_lite)

test "$ac_cv_build_full" = "yes" && buildset_list="full"
test "$ac_cv_build_lite" = "yes" && buildset_list="$buildset_list lite"
dnl -----------------------------------------------------------------
if test "$ac_cv_build_full" = "yes"; then
  AC_MSG_CHECKING(for thread paths parameter)
  AC_CACHE_VAL(ac_cv_acarg_path_threads, [dnl
    AC_ARG_WITH(threads-path,
            [  --with-threads-path=PATH  path to threads package],
    [],
    [ with_threads_path="NONE" ])
    ac_cv_acarg_path_threads="$with_threads_path"
  ])
  AC_MSG_RESULT($ac_cv_acarg_path_threads)
dnl -----------
  AC_MSG_CHECKING(for thread include path parameter)
  AC_CACHE_VAL(ac_cv_acarg_path_threads_inc, [dnl
    AC_ARG_WITH(threads-inc,
            [  --with-threads-inc=PATH   path to thread include files],
    [],
    [ with_threads_inc="NONE" ])
    ac_cv_acarg_path_threads_inc="$with_threads_inc"
  ])
  AC_MSG_RESULT($ac_cv_acarg_path_threads_inc)
dnl -----------
  AC_MSG_CHECKING(for thread library path parameter)
  AC_CACHE_VAL(ac_cv_acarg_path_threads_lib, [dnl
    AC_ARG_WITH(threads-lib,
            [  --with-threads-lib=PATH   path to thread library files],
    [],
    [ with_threads_lib="NONE" ])
    ac_cv_acarg_path_threads_lib="$with_threads_lib"
  ])
  AC_MSG_RESULT($ac_cv_acarg_path_threads_lib)
dnl -----------
fi
dnl -----------------------------------------------------------------
dnl AC_PATH_THREADS_blah macros have side effects so they can't go inside
dnl of a cache_val macro
dnl -----------
if test "$ac_cv_build_full" = "yes"; then
  ac_sys_threads="$ac_cv_sys_threads"
  AC_PATH_THREADS_SOLARISTHREADS
  AC_PATH_THREADS_PTHREADS
  AC_PATH_THREADS_CTHREADS
  AC_PATH_THREADS_QUICKTHREADS
  AC_MSG_CHECKING(for threads)
  AC_CACHE_VAL(ac_cv_sys_threads, [
    ac_cv_sys_threads="$ac_sys_threads"
  ])
  AC_MSG_RESULT($ac_cv_sys_threads)
  AC_DEFINE_UNQUOTED(TH_MODULE, $ac_th_module)
  SYS_TH_MODULE="$ac_th_module"
  AC_SUBST(SYS_TH_MODULE)
fi
])


dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                         AC_PATH_THREADS_PTHREADS                    ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS_PTHREADS,
[if test "x$ac_sys_threads" = "xpthreads" || test "x$ac_sys_threads" = "xunknown" || test "x$ac_sys_threads" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
AC_FIND_USER_INCLUDE(pthread,$ac_cv_acarg_path_threads_inc $ac_cv_acarg_path_threads /usr/local/fsu-pthreads,
[found_inc="yes"
ac_cv_path_pthreads_inc="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
AC_FIND_USER_LIB(pthreads,$ac_cv_acarg_path_threads_lib $ac_cv_acarg_path_threads /usr/local/fsu-pthreads,
[found_lib="yes"
dnl
dnl  --Old versions of fsu-pthreads installed to libpthreads.a.  Check
dnl  --for the Sun arch and act appropriately.
dnl
case "$target" in
    sparc-sun-sunos4* | sparc-sun-solaris1* )
	lib_type="fsu-old"
    ;;
    * )
	lib_type="dce"
    ;;
esac
ac_cv_path_pthreads_lib="$ac_find_lib_dir"
ac_cv_path_pthreads_lib_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
AC_FIND_USER_LIB(gthreads,$ac_cv_acarg_path_threads_lib $ac_cv_acarg_path_threads /usr/local/fsu-pthreads,
[found_lib="yes"
lib_type="fsu"
ac_cv_path_pthreads_lib="$ac_find_lib_dir"
ac_cv_path_pthreads_lib_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
AC_FIND_USER_LIB(dce,$ac_cv_acarg_path_threads_lib $ac_cv_acarg_path_threads /usr/local/fsu-pthreads,
[found_lib="yes"
lib_type="dce"
ac_cv_path_pthreads_lib="$ac_find_lib_dir"
ac_cv_path_pthreads_lib_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
dnl ----- set defines, flags, libs
  if test "$found_inc" = "yes" && test "$found_lib" = "yes"; then
    AC_DEFINE(HAVE_PTHREADS)
    ac_sys_threads="pthreads"
    ac_th_module="p0_th_p"
    case "$lib_type" in
      dce )
        AC_DEFINE(_ALL_SOURCE)
        case "$target" in
          rs6000-ibm-aix3* )
            AC_DEFINE(HAVE_PTHREADS_DRAFT_4)
          ;;
          rs6000-ibm-aix4* )
            AC_DEFINE(HAVE_PTHREADS_DRAFT_10)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
          ;;
          hppa*-hp-hpux9* )
            CCOPTS_full="$CCOPTS_full -I/usr/include/reentrant"
            LDOPTS_full="$LDOPTS_full -ldce -lm -lc_r"
            AC_DEFINE(_REENTRANT)
            AC_DEFINE(_HPUX_SOURCE)
            AC_DEFINE(_CMA_REENTRANT_CLIB_)
            AC_DEFINE(HAVE_PTHREADS_DRAFT_4)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
          ;;
        esac
      ;;
      dnl
      dnl --Note the only difference in the next two is the LDOPTS_full
      dnl --link line
      dnl
      fsu )
        CCOPTS_full="$CCOPTS_full -I$ac_cv_path_pthreads_inc"
        LDOPTS_full="$LDOPTS_full -L$ac_cv_path_pthreads_lib -lgthreads -lmalloc"
        AC_DEFINE(HAVE_PTHREADS_DRAFT_6)
	AC_DEFINE(HAVE_PTHREADS_INIT_FUNC)
        AC_DEFINE(__USE_FIXED_PROTOTYPES__)
	AC_CACHE_VAL(ac_cv_other_pthreads_lib, [dnl
	    ac_cv_other_pthreads_lib=malloc
	])
      ;;
      fsu-old )
	CCOPTS_full="$CCOPTS_full -I$ac_cv_path_pthreads_inc"
	LDOPTS_full="$LDOPTS_full -L$ac_cv_path_pthreads_lib -lpthreads -lmalloc"
        AC_DEFINE(HAVE_PTHREADS_DRAFT_6)
	AC_DEFINE(HAVE_PTHREADS_INIT_FUNC)
	AC_DEFINE(__USE_FIXED_PROTOTYPES__)
	AC_CACHE_VAL(ac_cv_other_pthreads_lib, [dnl
	    ac_cv_other_pthreads_lib=malloc
        ])
      ;;
    esac
  fi
fi
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                   AC_PATH_THREADS_SOLARISTHREADS                    ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS_SOLARISTHREADS,
[if test "x$ac_sys_threads" = "xsolaristhreads" || test "x$ac_sys_threads" = "xunknown" || test "x$ac_sys_threads" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
AC_FIND_USER_INCLUDE(thread,$ac_cv_acarg_path_threads_inc $ac_cv_acarg_path_threads,
[found_inc="yes"
ac_cv_path_solthreads_inc="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
AC_FIND_USER_LIB(thread,$ac_cv_acarg_path_threads_lib $ac_cv_acarg_path_threads,
[found_lib="yes"
ac_cv_path_solthreads_lib="$ac_find_lib_dir"
ac_cv_path_solthreads_lib_file="$ac_find_lib_file"
])
dnl %%%%%%%%%%%%%%
  if test "$found_inc" = "yes" && test "$found_lib" = "yes"; then
    AC_DEFINE(HAVE_SOLARISTHREADS)
    ac_sys_threads="solaristhreads"
    ac_th_module="p0_th_sol"
    case "$target" in
      sparc-sun-sunos5* | sparc-sun-solaris2* )
        AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
      ;;
    esac
  fi
fi
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                         AC_PATH_THREADS_CTHREADS                    ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS_CTHREADS,
[if test "x$ac_sys_threads" = "xcthreads" || test "x$ac_sys_threads" = "xunknown" || test "x$ac_sys_threads" = "x"; then
    found_inc="no"
    AC_FIND_USER_INCLUDE(cthreads,$ac_cv_acarg_path_threads_inc $ac_cv_acarg_path /usr/include/mach,
[found_inc="yes"
ac_cv_path_cthreads_inc="$ac_find_inc_dir"
])
    if test "$found_inc" = "yes"; then
	AC_DEFINE(HAVE_CTHREADS)
	ac_sys_threads="cthreads"
	ac_th_module="p0_th_c"
    fi
fi
])

dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                   AC_PATH_THREADS_QUICKTHREADS                      ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS_QUICKTHREADS,
[if test "x$ac_sys_threads" = "xquickthreads" || test "x$ac_sys_threads" = "xunknown" || test "x$ac_sys_threads" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
AC_FIND_USER_INCLUDE(qt,$ac_cv_acarg_path_threads_inc $ac_cv_acarg_path_threads,
[found_inc="yes"
ac_cv_path_quickthreads_inc="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
AC_FIND_USER_LIB(qt,$ac_cv_acarg_path_threads_lib $ac_cv_acarg_path_threads,
[found_lib="yes"
ac_cv_path_quickthreads_lib="$ac_find_lib_dir"
])
dnl %%%%%%%%%%%%%%
  dnl  *** quickthreads is the default package.. it will always succeed
  AC_DEFINE(HAVE_QUICKTHREADS)
  ac_sys_threads="quickthreads"
  ac_th_module="p0_th_qt"
  if test "$found_inc" != "yes" || test "$found_lib" != "yes"; then
    dnl *** use quickthreads that came with this package
    ac_cv_path_quickthreads_inc="../../quickthreads/"
    ac_cv_path_quickthreads_lib="../quickthreads"
  fi
  case "$target" in
    hppa*-hp-hpux9* )
      CCOPTS_full="$CCOPTS_full -I$ac_cv_path_quickthreads_inc"
      LDOPTS_full="$LDOPTS_full -L$ac_cv_path_quickthreads_lib -lbb -lqt"
    ;;
    rs6000-ibm-aix3* )
      CCOPTS_full="$CCOPTS_full -I$ac_cv_path_quickthreads_inc"
      LDOPTS_full="$LDOPTS_full -L$ac_cv_path_quickthreads_lib -lqt"
    ;;
    sparc-sun-sunos4* | sparc-sun-solaris1* )
      CCOPTS_full="$CCOPTS_full -I$ac_cv_path_quickthreads_inc"
      LDOPTS_full="$LDOPTS_full -L$ac_cv_path_quickthreads_lib -lqt"
    ;;
    sparc-sun-sunos5* | sparc-sun-solaris2* )
      AC_SYS_LWP
    ;;
    mips-sgi-irix5* )
      CCOPTS_full="$CCOPTS_full -I$ac_cv_path_quickthreads_inc"
      LDOPTS_full="$LDOPTS_full -L$ac_cv_path_quickthreads_lib -lqt -lmpc"
      AC_CHECK_FUNCS(sproc)
      AC_CHECK_HEADERS(limits.h ulocks.h)
      AC_DEFINE(P0_QT_MULTIPROCESSOR)
      AC_DEFINE(_SGI_MP_SOURCE)
    ;;
  esac
fi
])


dnl
dnl  --HP seems to think the prototype for select() should be:
dnl  --
dnl  --   int select(int, int *, int *, int *, struct timeval *);
dnl  --
dnl  --by putting the "correct" prototype in the source, it causes an
dnl  --error on any machine with a different prototype.  The strange
dnl  --thing is that the HP man pages show two examples; one using an
dnl  --int * and the other an fd_set *.  Best be safe and use what the
dnl  --compiler is expecting.
dnl  --
dnl  --IBM is even more strange.  They have the prototype for select()
dnl  --in the info pages as:
dnl  --
dnl  --   int select(int,
dnl  --		     struct sellist *,
dnl  --		     struct sellist *,
dnl  --		     struct sellist *,
dnl  --		     struct timeval *);
dnl  --
dnl  --but the header files have it as (and xlc takes):
dnl  --
dnl  --   int select(unsigned long, void *, void *, void *, struct timeval *);
dnl  --
dnl  --This is very strange indeed.
dnl
AC_DEFUN(AC_CHECK_SELECT_PROTOTYPE,
[AC_MSG_CHECKING(what type to use for select())
    AC_CACHE_VAL(ac_cv_fd_set_cast, [dnl
	AC_TRY_COMPILE(
#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>

int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
,
    fd_set read_values;
    int rc;

    rc = select(0, &read_values, NULL, NULL, NULL);
,
    ac_cv_fd_set_cast="(fd_set *)"
,
    [case "$target" in
	rs6000-ibm-aix* )
	    ac_cv_fd_set_cast="(void *)"
	;;
	*hpux* )
	    ac_cv_fd_set_cast="(int *)"
	;;
	* )
	    ac_cv_fd_set_cast="unknown"
	    echo "Please notify nexus@mcs.anl.gov that autoconf could"
	    echo "not detect the type for your select() system call."
	    echo "Please include your system type (OS, manufacturer, chip)"
	    echo "as well as a copy of /usr/include/sys/types.h and any"
	    echo "other file that is used for the select() call."
	    AC_MSG_ERROR(cannot compile without the correct information)
	;;
    esac]
	) dnl --End of AC_TRY_COMPILE()
    ]) dnl --End of AC_CACHE_VAL()

    AC_DEFINE_UNQUOTED(NEXUS_FD_SET_CAST,$ac_cv_fd_set_cast)
    AC_SUBST(NEXUS_FD_SET_CAST)
    AC_MSG_RESULT($ac_cv_fd_set_cast)
]) dnl --End of AC_CHECK_SELECT_PROTOTYPE()

AC_DEFUN(AC_CHECK_POSIX_SIGNALS,
[found_ps_funcs="yes"
    AC_MSG_CHECKING(for posix signals in cache)
    AC_CACHE_VAL(ac_cv_check_posix_signals, [dnl
        AC_MSG_RESULT(no, checking manually)
        AC_CHECK_FUNCS(sigaction sigemptyset sigaddset sigprocmask waitpid,
	# do nothing if it finds the functions
	,
	    found_ps_funcs="no"
	    break) dnl --End of AC_CHECK_FUNCS()
    ]) dnl --End of AC_CACHE_VAL

    if test "$found_ps_funcs" = "yes" ; then
        ac_cv_check_posix_signals="yes"
        AC_DEFINE(HAVE_POSIX_SIGNALS)
    fi
    AC_MSG_RESULT($ac_cv_check_posix_signals)
]) dnl --End of AC_CHECK_POSIX_SIGNALS()

dnl
dnl  --This is almost a direct copy of the file that John Garnett wrote
dnl  --for the CC++ installation.
dnl
AC_DEFUN(AC_FUNC_FILE_LOCK,
[AC_MSG_CHECKING(for preferred file locking method in cache)
  AC_CACHE_VAL(ac_cv_func_file_lock, [dnl
    dnl
    dnl --Don't use AC_CHECK_FUNCS() because we don't want to have
    dnl --these functions recognized unless the program says they work.
    dnl
    AC_MSG_RESULT(no)
    AC_CHECK_FUNC(flock, AC_DEFINE(USE_FLOCK))
    AC_CHECK_FUNC(fcntl, AC_DEFINE(USE_FCNTL))
    AC_CHECK_FUNC(lockf, AC_DEFINE(USE_LOCKF))
    dnl  AC_CHECK_FUNCS(flock fcntl lockf)
    AC_MSG_CHECKING(which lock method to use)
    AC_TRY_RUN(
[#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

char filename[32];
int fd;
FILE *output;

void cleanup(ec)
int ec;
{
        close(fd);
        unlink(filename);
        fclose(output);
        exit(ec);
}

void do_lseek(which)
char* which;
{
        while (lseek(fd, 0, SEEK_SET) == -1) {
                if (errno == EINTR) {
                        fprintf(stderr, "lseek %s failed with errno = %d\n", which, errno);
                        break;
                }
        }
}

#ifdef USE_FLOCK
int do_flock()
{
        while (flock(fd, LOCK_EX) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "flock failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}


int do_unflock()
{
        while (flock(fd, LOCK_UN) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "unflock failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}
#endif /* USE_FLOCK */

#ifdef USE_LOCKF
int do_lockf()
{
        do_lseek("lockf");
        while (lockf(fd, F_LOCK, 1L) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "lockf failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}

int do_unlockf()
{
        do_lseek("unlockf");
        while (lockf(fd, F_ULOCK, 1L) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "unlockf failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}
#endif /* USE_LOCKF */

#ifdef USE_FCNTL
int do_fcntl()
{
        struct flock theLock;

        theLock.l_type = F_WRLCK;
        theLock.l_whence = 0;
        theLock.l_start = 0;
        theLock.l_len = 0;

        while (fcntl(fd, F_SETLKW, (int) &theLock) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "fcntl failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}

int do_unfcntl()
{
        struct flock theLock;

        theLock.l_type = F_UNLCK;
        theLock.l_whence = 0;
        theLock.l_start = 0;
        theLock.l_len = 0;

        while (fcntl(fd, F_SETLKW, (int) &theLock) == -1) {
                if (errno != EINTR) {
                        fprintf(stderr, "fcntl failed with errno = %d\n", errno);
                        return 0;
                }
        }
        return 1;
}
#endif /* USE_FCNTL */

int main(int argc, char **argv)
{
        int suffix;
        int success = 0;

        if (argc == 2) {
                suffix = atoi(argv[1]);
        } else {
                suffix = getpid();
        }
        sprintf(filename, "conftest.res", suffix);
        output = fopen(filename, "w");
        sprintf(filename, "/tmp/lock.%d", suffix);
        fd = open(filename, O_RDWR|O_CREAT|O_EXCL, 0600);
        if (fd == -1) {
                fprintf(stderr, "Open of %s failed.\n", filename);
                exit(1);
        }
#ifdef USE_LOCKF
        if (do_lockf()) {
                if (do_unlockf()) {
                    success = 1;
                    fprintf(output, "LOCKF");
                }
        }
#endif /* USE_LOCKF */
#ifdef USE_FCNTL
        if (!success && do_fcntl()) {
                if (do_unfcntl()) {
                    success = 2;
                    fprintf(output, "FCNTL");
                }
        }
#endif /* USE_FCNTL */
#ifdef USE_FLOCK
        if (!success && do_flock()) {
                if (do_unflock()) {
                    success = 3;
                    fprintf(output, "FLOCK");
                }
        }
#endif /* USE_FLOCK */
        if (!success) {
            fprintf(output, "none");
        }
        cleanup(0);
        return (!success);
}
    ],[
	ac_cv_func_file_lock=`cat conftest.res`
    ],[
	ac_cv_func_file_lock=none
	AC_MSG_RESULT(none)
    ],[
	ac_cv_func_file_lock=LOCKF
    ]) dnl --End of AC_TRY_RUN()

    if test "$ac_cv_func_file_lock" = "none" ; then
	AC_MSG_ERROR(You must have at least one locking method: flock() fcntl() lockf())
    fi
  ]) dnl --End of AC_CACHE_VAL()
  AC_MSG_RESULT($ac_cv_func_file_lock)
  AC_DEFINE_UNQUOTED(HAVE_$ac_cv_func_file_lock)
]) dnl --End of AC_FUNC_FILE_LOCK()

AC_DEFUN(AC_FUNC_MALLOCERR,
[ AC_MSG_CHECKING(for malloc error descriptors (NeXT))
  AC_CACHE_VAL(ac_cv_func_mallocerr, [dnl
    result=0
    AC_CHECK_FUNC(malloc_debug, result=`expr $result + 1`)
    AC_CHECK_FUNC(malloc_error, result=`expr $result + 1`)
    if test "$result" = "2"; then
      ac_cv_func_mallocerr="yes"
    else
      ac_cv_func_mallocerr="no"
    fi
  ])
  AC_MSG_RESULT($ac_cv_func_mallocerr)
  if test "$ac_cv_func_mallocerr" = "yes"; then
    AC_DEFINE(HAVE_MALLOCERR)
  fi
])

AC_DEFUN(AC_SYS_LWP,
[ AC_MSG_CHECKING(for light-weight processes)
  AC_CACHE_VAL(ac_cv_sys_lwp, [dnl
  result=0
  AC_CHECK_FUNC(_lwp_getprivate, result=`expr $result + 1`)
  AC_CHECK_FUNC(_lwp_setprivate, result=`expr $result + 1`)
  if test "$result" = "2"; then
    ac_cv_sys_lwp="yes"
  else
    ac_cv_sys_lwp="no"
  fi
  ])
  AC_MSG_RESULT($ac_cv_sys_lwp)
  if test "$ac_cv_sys_lwp" = "yes"; then
    AC_DEFINE(HAVE_LWP)
  fi
])

AC_DEFUN(AC_SYS_SIGNAL_WAITING,
[ AC_MSG_CHECKING(for SIGWAITING)
  AC_CACHE_VAL(ac_cv_sys_signal_waiting, [dnl
  AC_EGREP_CPP(zowie,
[#include <signal.h>
#ifdef SIGWAITING
 zowie
#endif
], ac_cv_sys_signal_waiting="yes",
ac_cv_sys_signal_waiting="no")
])
  AC_MSG_RESULT($ac_cv_sys_signal_waiting)
  if test "$ac_cv_sys_signal_waiting" = "yes"; then
    AC_DEFINE(HAVE_SYS_SIGWAITING)
  fi
])

AC_DEFUN(AC_STRUCT_TBTABLE,
[ AC_MSG_CHECKING(for struct tbtable tracebacks)
  AC_CACHE_VAL(ac_cv_struct_tbtable, [dnl
    ac_cv_struct_tbtable="no"
    AC_CHECK_HEADER(sys/debug.h,
      AC_EGREP_HEADER(tbtable, sys/debug.h, ac_cv_struct_tbtable="yes"))
  ])
  AC_MSG_RESULT($ac_cv_struct_tbtable)
  if test "$ac_cv_struct_tbtable" = "yes"; then
    AC_DEFINE(HAVE_STRUCT_TBTABLE)
  fi
])

dnl
dnl  --This is code that was adopted from the PAC_CHECK_COMPILER()
dnl  --macro in the mpich distriubtion
dnl  --
dnl  --I would like to use this once I check it works correctly as it
dnl  --would dramically reduce the amount of C code in this file.  And
dnl  --it would make it very easy to add/delete a test.
dnl  --
AC_DEFUN(AC_CHECK_PROGS,
[   if test "x$check_progs_dir" = "x" ; then
        $check_progs_dir="../config/test/nexus"
    fi
    for file in $check_progs_dir/*.c ; do
	base=`basename $file .c`
	AC_MSG_CHECKING(`cat $check_progs_dir/$base.check`)
	cp $file conftest.c
	rm -f conftest.cout conftest.rout
	if eval $CC $CFLAGS -o conftest conftest.c $LIBS > conftest.cout 2>&1 ; then
	    if test -s conftest ; then
		./conftest 2>&1 1> conftest.rout
		if test -s conftest.result ; then
		    AC_MSG_RESULT(`cat conftest.res`)
		    result_unknown="no"
		else
		    AC_MSG_RESULT(unknown)
		    result_unknown="yes"
		fi
		if test $? = 0 ; then
		    # it worked
		    if test "$result_unknow" = "yes" ; then
			echo "Program appeared to work, however"
		    fi
		else
		    # it didn't
		    cat $base.msg
		    if test -s conftest.cout ; then
			echo "Output from compile step was:"
			cat conftest.cout
		    fi
		    if test -s conftest.rout ; then
			echo "Output from run step was:
			cat conftest.rout
		    fi
		fi
	    fi
	else
	    echo "Program failed to compile."
	    cat $base.msg
	    if test -s conftest.cout ; then
		echo "Output from compile step was:"
		cat conftest.cout
	    fi
	fi
    done
]) dnl --End of AC_CHECK_PROGS()
