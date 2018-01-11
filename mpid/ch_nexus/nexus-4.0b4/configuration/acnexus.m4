dnl Various nexus build modes
AC_DEFUN(LAC_NEXUS_INIT,
[
AC_CANONICAL_SYSTEM
OFILE="o"
AC_SUBST(OFILE)

AC_ARG_ENABLE(macros,
	[  --disable-macros        use function calls instead of macros],
	[lac_cv_macros="$enableval"],
	[lac_cv_macros="yes"])

if test "$lac_cv_macros" = "yes"; then
  AC_DEFINE(BUILD_USING_MACROS)
fi

AC_ARG_ENABLE(sharedlib,
	[  --enable-sharedlib      build shared libraries],
	[lac_cv_build_shared="$enableval"],
	[lac_cv_build_shared="no"])

lac_build_shared=$lac_cv_build_shared

AC_ARG_ENABLE(debug,
	[  --enable-debug          compile in debugging features],
	[lac_cv_debug="$enableval"],
	[lac_cv_debug="no"])

nexus_flavor=""
if test "$lac_cv_debug" = "yes"; then
  nexus_flavor="d"
  AC_DEFINE(BUILD_DEBUG)
fi

AC_ARG_ENABLE(profile,
	[  --enable-profile        compile in profiling features],
	[lac_cv_profile="$enableval"],
	[lac_cv_profile="no"])

if test "$lac_cv_profile" = "yes"; then
  nexus_flavor="p${nexus_flavor}"
  AC_DEFINE(BUILD_PROFILE)
fi

if test -n "$nexus_flavor"; then
   nexus_flavor="_${nexus_flavor}" 
fi
AC_SUBST(nexus_flavor)

AC_ARG_ENABLE(64bit,
	[  --enable-64bit          build 64-bit objects (SGI Irix 6.x only)],
	[lac_cv_build_64bit="$enableval"],
	[lac_cv_build_64bit="no"])

dnl -----------------------------------------------------------------
dnl -- Protocols
dnl --- tcp, inx, mpinx, mpl, mn_udp, udp, atm,
dnl -- Startups
dnl --- fork, rsh, ss, ibmds, soldl
dnl -- Database
dnl --- file
dnl -----------------------------------------------------------------

proto_tcp="unknown"
proto_inx="unknown"
proto_mpl="unknown"
proto_shm="unknown"
start_inx="unknown"
start_fork="unknown"
start_rsh="unknown"
start_ss="unknown"
start_ibmds="unknown"
start_mpl="unknown"
db_file="yes"

AC_ARG_WITH(ss,
	[  --with-ss               include the secure server startup module],
	[start_ss="$withval"],
	[start_ss="no"])
AC_ARG_WITH(tcp,
	[  --with-tcp              include the TCP/IP protocol],
	[proto_tcp="$withval"],
	[AC_SYS_TCP])
AC_ARG_WITH(inx,
	[  --with-inx              include the Intel NX protocols],
	[proto_inx="$withval"; start_inx="$withval"],
	[AC_SYS_INX])
dnl AC_ARG_WITH(mpl,
dnl 	[  --with-mpl              include the SP/2 MPL protocols],
dnl 	[proto_mpl="$withval"; start_mpl="$withval"],
dnl	[AC_SYS_MPL])
AC_ARG_WITH(mpl,
	[  --with-mpl              include the SP/2 MPL protocols],
	[proto_mpl="$withval"; start_mpl="$withval"],
        [proto_mpl="no"; start_mpl="no"])
if test "x$start_mpl" = "xyes"; then
  proto_mpl="no"
  start_mpl="no"
  echo "MPL protocol module has been disabled"
fi
AC_ARG_WITH(shm,
	[  --with-shm              include the System V shared memory protocols],
	[proto_shm="$withval"],
	[AC_SYS_SHM])
AC_ARG_WITH(fork,
	[  --with-fork             include the fork startup module],
	[start_fork="$withval"],
	[AC_SYS_FORK])
AC_ARG_WITH(rsh,
	[  --with-rsh              include the rsh startup module],
	[start_rsh="$withval"],
	[AC_PROG_RSH])
dnl AC_ARG_WITH(ibmds,
dnl 	[  --with-ibmds            include the IBM AIX dynaload startup module],
dnl 	[start_ibmds="$withval"],
dnl 	[AC_SYS_IBMDS])
AC_ARG_WITH(ibmds,
	[  --with-ibmds            include the IBM AIX dynaload startup module],
	[start_ibmds="$withval"],
	[start_ibmds="no"])
if test "x$start_ibmds" = "xyes"; then
  start_ibmds="no"
  echo "IBM AIX dynaload (IBMDS) startup module has been disabled"
fi

case "$target" in
  *sunos4* | *solaris1* )
    ARFLAGS="ruv"
    AC_DEFINE(TARGET_ARCH_SUNOS41)
  ;;
  *sunos5* | *solaris2* )
    ARFLAGS="ruv"
    AC_DEFINE(TARGET_ARCH_SOLARIS)
  ;;
  *-ibm-aix3* )
    AC_DEFINE(TARGET_ARCH_AIX)
    AC_DEFINE(TARGET_ARCH_AIX3)
  ;;
  *-ibm-aix4* )
    AC_DEFINE(TARGET_ARCH_AIX)
    AC_DEFINE(TARGET_ARCH_AIX4)
  ;;
  *-*-hpux* )
    AC_DEFINE(TARGET_ARCH_HPUX)
  ;;
  i860-intel-osf* )
    AC_DEFINE(TARGET_ARCH_PARAGON)
  ;;
  mips-sgi-irix5* )
    AC_DEFINE(TARGET_ARCH_SGI)
    AC_DEFINE(TARGET_ARCH_IRIX)
    AC_DEFINE(TARGET_ARCH_IRIX5)
  ;;
  mips-sgi-irix6* )
    AC_DEFINE(TARGET_ARCH_SGI)
    AC_DEFINE(TARGET_ARCH_IRIX)
    AC_DEFINE(TARGET_ARCH_IRIX6)
  ;;
  *dec* )
    AC_DEFINE(TARGET_ARCH_AXP)
    AC_DEFINE(TARGET_ARCH_OSF1)
    dnl --- more stuff to go here.. need to check with John
  ;;
  *c90* )
    AC_DEFINE(TARGET_ARCH_CRAYC90)
  ;;
  *freebsd* )
    AC_DEFINE(TARGET_ARCH_FREEBSD)
  ;;
  *nextstep* )
    AC_DEFINE(TARGET_ARCH_NEXTSTEP)
  ;;
  * )
	echo "platform not configured"
  ;;
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
  *sunos5* | *solaris2* )
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


dnl include_file, path
AC_DEFUN(LAC_FIND_USER_INCLUDE,[
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

AC_DEFUN(LAC_FIND_USER_LIB,[
AC_MSG_CHECKING([for library $1])
ac_find_lib_file=""
for dir in $2 \
	/usr \
	/usr/lib \
	/usr/shlib \
	/usr/local \
	/usr/local/$1 \
	/usr/contrib \
	/usr/contrib/$1 \
	$HOME/$1 \
	/opt/$1 \
	/opt/local \
	/opt/local/$1 \
	/local/encap/$1 ; do
  for ext in so a; do
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
AC_DEFUN(LAC_PATH_THREADS,
[
  AC_ARG_WITH(threads,
	[  --without-threads       build nexus lite],
	[lac_with_threads="$withval"],
	[lac_with_threads="yes"])
  AC_ARG_WITH(thread-library,
        [  --with-thread-library=PATH    path to thread library files],
        [lac_thread_library_path="$withval"],
        [lac_thread_library_path=""])
  AC_ARG_WITH(thread-includes,
        [  --with-thread-includes=PATH   path to thread include files],
        [lac_thread_include_path="$withval"],
	[lac_thread_include_path=""])

  THREADSAFE_CFLAGS=
  THREADSAFE_LDFLAGS=

  if test "$lac_with_threads" = "no" ; then
     lac_nexus_lite="yes"
     if test -z "$nexus_flavor"; then
         nexus_flavor="_l"
     else
         nexus_flavor="${nexus_flavor}l"
     fi
     echo building lite version of library
     AC_DEFINE(BUILD_LITE)
     lac_threads_type=none
     ac_th_module=p0_th_null
  else
     lac_nexus_lite="no"
     lac_threads_type=$lac_with_threads
     if test $lac_with_threads = "yes"; then
         lac_threads_type=""
     fi   
     dnl AC_PATH_THREADS_blah macros have side effects so they can't go inside
     dnl of a cache_val macro
     AC_PATH_THREADS_SOLARISTHREADS
     AC_PATH_THREADS_PTHREADS
     AC_PATH_THREADS_CTHREADS
     AC_PATH_THREADS_QUICKTHREADS
     AC_MSG_CHECKING(for thread library)
     AC_MSG_RESULT($lac_threads_type)
  fi
AC_DEFINE_UNQUOTED(TH_MODULE, $ac_th_module)
THREAD_MODULE="$ac_th_module"
AC_SUBST(THREAD_MODULE)
AC_SUBST(THREADSAFE_CFLAGS)
AC_SUBST(THREADSAFE_LDFLAGS)
])


dnl -----------------------------------------------------------------------------
dnl ----                                                                     ----
dnl ----                         AC_PATH_THREADS_PTHREADS                    ----
dnl ----                                                                     ----
dnl -----------------------------------------------------------------------------
AC_DEFUN(AC_PATH_THREADS_PTHREADS,
[if test "x$lac_threads_type" = "xpthreads" || test "x$lac_threads_type" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
LAC_FIND_USER_INCLUDE(pthread,$lac_thread_include_path /usr/local/fsu-pthreads-3.1 /usr/local/fsu-pthreads,
[found_inc="yes"
lac_thread_include_path="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
LAC_FIND_USER_LIB(gthreads,$lac_thread_library_path /usr/local/fsu-pthreads-3.1 /usr/local/fsu-pthreads,
[found_lib="yes"
lib_type="fsu"
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
LAC_FIND_USER_LIB(pthread,$lac_thread_library_path,
[found_lib="yes"
lib_type="pthread"
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
LAC_FIND_USER_LIB(pthreads,$lac_thread_library_path /usr/local/fsu-pthreads-3.1 /usr/local/fsu-pthreads,
[found_lib="yes"
dnl
dnl  --Old versions of fsu-pthreads installed to libpthreads.a.  Check
dnl  --for the Sun arch and act appropriately.
dnl
case "$target" in
    *-ibm-aix4* )
	lib_type="pthread"
    ;;
    *sunos4* | *solaris1* )
	lib_type="fsu-old"
    ;;
    * )
	lib_type="dce"
    ;;
esac
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
if test "$found_lib" = "no"; then
LAC_FIND_USER_LIB(dce,$lac_thread_library_path,
[found_lib="yes"
lib_type="dce"
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
fi
dnl %%%%%%%%%%%%%%
dnl ----- set defines, flags, libs
  if test "$found_inc" = "yes" && test "$found_lib" = "yes"; then
    AC_DEFINE(HAVE_PTHREAD)
    lac_threads_type="pthreads"
    ac_th_module="p0_th_p"
    case "$lib_type" in
      pthread )
        case "$target" in
          mips-sgi-irix6* )
            AC_DEFINE(HAVE_PTHREAD_DRAFT_10)
            AC_DEFINE(HAVE_PTHREAD_PREEMPTIVE)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)	
            AC_DEFINE(_SGI_MP_SOURCE)	
            THREADSAFE_LDFLAGS="-lpthread"
          ;;
          *-ibm-aix4* )
            AC_DEFINE(HAVE_PTHREAD_DRAFT_8)
            AC_DEFINE(HAVE_PTHREAD_PREEMPTIVE)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
          ;;
          alpha-dec-osf4* )
            AC_DEFINE(HAVE_PTHREAD_DRAFT_10)
            AC_DEFINE(HAVE_PTHREAD_PREEMPTIVE)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)  
            THREADSAFE_CFLAGS="-pthread"
          ;;
	  * )
            AC_DEFINE(HAVE_PTHREAD_DRAFT_10)
            AC_DEFINE(HAVE_PTHREAD_PREEMPTIVE)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)	
            THREADSAFE_LDFLAGS="-lpthread"
          ;;
        esac
	;;
      dce )
        case "$target" in
          *-ibm-aix3* )
            AC_DEFINE(HAVE_PTHREAD_DRAFT_4)
            AC_DEFINE(HAVE_PTHREAD_SCHED)
          ;;
          hppa*-hp-hpux9* )
            THREADSAFE_CFLAGS="-I/usr/include/reentrant"
            THREADSAFE_LDFLAGS="-ldce -lm -lc_r"
            AC_DEFINE(_REENTRANT)
            AC_DEFINE(_CMA_REENTRANT_CLIB_)
            AC_DEFINE(HAVE_PTHREAD_DRAFT_4)
            AC_DEFINE(HAVE_PTHREAD_SCHED)
            AC_DEFINE(HAVE_THREAD_SAFE_STDIO)
            AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
          ;;
        esac
      ;;
      fsu )
        THREADSAFE_CFLAGS="-I$lac_thread_include_path"
        THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lgthreads -lmalloc"
        AC_DEFINE(HAVE_PTHREAD_DRAFT_6)
        AC_DEFINE(HAVE_PTHREAD_SCHED)
	AC_DEFINE(HAVE_PTHREAD_INIT_FUNC)
	AC_DEFINE(__USE_FIXED_PROTOTYPES__)
	AC_CACHE_VAL(ac_cv_other_pthreads_lib, [dnl
	    ac_cv_other_pthreads_lib=malloc
        ])
      ;;
      fsu-old )
        THREADSAFE_CFLAGS="-I$lac_thread_include_path"
        THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lpthreads -lmalloc"
        AC_DEFINE(HAVE_PTHREAD_DRAFT_6)
        AC_DEFINE(HAVE_PTHREAD_SCHED)
	AC_DEFINE(HAVE_PTHREAD_INIT_FUNC)
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
[if test "x$lac_threads_type" = "xsolaristhreads" || test "x$lac_threads_type" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
LAC_FIND_USER_INCLUDE(thread,$lac_thread_include_path,
[found_inc="yes"
lac_thread_include_path="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
LAC_FIND_USER_LIB(thread,$lac_thread_library_path,
[found_lib="yes"
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
dnl %%%%%%%%%%%%%%
  if test "$found_inc" = "yes" && test "$found_lib" = "yes"; then
    AC_DEFINE(HAVE_SOLARISTHREADS)
    lac_threads_type="solaristhreads"
    ac_th_module="p0_th_sol"
    case "$target" in
      *sunos5* | *solaris2* )
        AC_DEFINE(HAVE_THREAD_SAFE_SELECT)
        AC_DEFINE(_REENTRANT)
        THREADSAFE_LDFLAGS="-lthread"
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
[if test "x$lac_threads_type" = "xcthreads" || test "x$ac_threads_type" = "x"; then
    found_inc="no"
    LAC_FIND_USER_INCLUDE(cthreads,$lac_threads_include_path /usr/include/mach,
[found_inc="yes"
lac_threads_include_path="$ac_find_inc_dir"
])
    if test "$found_inc" = "yes"; then
	AC_DEFINE(HAVE_CTHREADS)
	ac_threads_type="cthreads"
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
[if test "x$lac_threads_type" = "xquickthreads" || test "x$lac_threads_type" = "x"; then
found_inc="no"
found_lib="no"
dnl %%%%%%%%%%%%%%
LAC_FIND_USER_INCLUDE(qt,$lac_thread_include_path quickthreads ../quickthreads ../../quickthreads,
[found_inc="yes"
lac_thread_include_path="$ac_find_inc_dir"
])
dnl %%%%%%%%%%%%%%
LAC_FIND_USER_LIB(qt,$lac_cv_thread_lib quickthreads ../quickthreads,
[found_lib="yes"
lac_thread_library_path="$ac_find_lib_dir"
lac_thread_library_file="$ac_find_lib_file"
])
dnl %%%%%%%%%%%%%%
  dnl  *** quickthreads is the default package.. it will always succeed
  AC_DEFINE(HAVE_QUICKTHREADS)
  lac_threads_type="quickthreads"
  ac_th_module="p0_th_qt"
  THREADSAFE_CFLAGS="-I$lac_thread_include_path"
  case "$target" in
    hppa*-hp-hpux9* )
      THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lbb -lqt"
    ;;
    *sunos5* | *solaris2* )
      AC_SYS_LWP
      THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lqt"
    ;;
    mips-sgi-irix5* )
      THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lqt -lmpc"
dnl      AC_CHECK_FUNCS(sproc)
dnl      AC_CHECK_HEADERS(limits.h ulocks.h)
      AC_DEFINE(P0_QT_MULTIPROCESSOR)
      AC_DEFINE(_SGI_MP_SOURCE)
      ;;
   * )
      THREADSAFE_LDFLAGS="-L$lac_thread_library_path -lqt"
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
	*-ibm-aix* )
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

AC_DEFUN(AC_CHECK_SSL,
[AC_MSG_CHECKING(for SSL in cache)
    found_ssl_in_cache="yes"
    AC_CACHE_VAL(ac_cv_sys_ssl, [dnl
	AC_MSG_RESULT(no. Checking manually)
	found_ssl_in_cache="no"
    ])
    if test "$found_ssl_in_cache" = "no"; then
	LAC_FIND_USER_INCLUDE(ssl,, ac_cv_sys_ssl="yes", ac_cv_sys_ssl="no")
    else
	AC_MSG_RESULT($ac_cv_sys_ssl)
    fi

    if test "$ac_cv_sys_ssl" = "yes"; then
	AC_DEFINE(HAVE_SSL)
    fi
])


AC_DEFUN(AC_CHECK_SSL,
[AC_MSG_CHECKING(for SSL in cache)
    found_ssl_in_cache="yes"
    AC_CACHE_VAL(ac_cv_sys_ssl, [dnl
	AC_MSG_RESULT(no. Checking manually)
	found_ssl_in_cache="no"
    ])
    if test "$found_ssl_in_cache" = "no"; then
	AC_FIND_USER_INCLUDE(ssl,, ac_cv_sys_ssl="yes", ac_cv_sys_ssl="no")
    else
	AC_MSG_RESULT($ac_cv_sys_ssl)
    fi

    if test "$ac_cv_sys_ssl" = "yes"; then
	AC_DEFINE(HAVE_SSL)
    fi
])


AC_DEFUN(LAC_NEXUS_ALIGN,[
case "$target" in
  *)
     AC_DEFINE(NEXUS_ALIGN,8)
     ;;	
esac
])
