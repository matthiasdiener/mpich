AC_DEFUN(AC_CHECK_KERBEROS,
[AC_MSG_CHECKING(for Kerberos (/usr/kerberos))
    AC_CACHE_VAL(ac_cv_sys_kerberos, [dnl
	if test -d /usr/kerberos ; then
	    ac_cv_sys_kerberos="yes"
	else
	    ac_cv_sys_kerberos="no"
	fi
    ])
    if test "$ac_cv_sys_kerberos" = "yes"; then
	AC_DEFINE(HAVE_KERBEROS)
    fi
    AC_MSG_RESULT($ac_cv_sys_kerberos)
])

AC_DEFUN(AC_CHECK_AFS,
[AC_MSG_CHECKING(for AFS (/usr/afsws))
    AC_CACHE_VAL(ac_cv_sys_afs, [dnl
	if test -d /usr/afsws ; then
	    ac_cv_sys_afs="yes"
	else
	    ac_cv_sys_afs="no"
	fi
    ])
    if test "$ac_cv_sys_afs" = "yes"; then
	AC_DEFINE(HAVE_AFS)
    fi
    AC_MSG_RESULT($ac_cv_sys_afs)
])

AC_DEFUN(AC_CHECK_SSL,
[AC_MSG_CHECKING(for SSL in cache)
    found_ssl_in_cache="yes"
    AC_CACHE_VAL(ac_cv_sys_ssl, [dnl
	AC_MSG_RESULT(no. checking manually)
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

AC_DEFUN(AC_CHECK_IWAY,
[AC_MSG_CHECKING(for IWAY (/usr/local/iway))
    AC_CACHE_VAL(ac_cv_sys_iway, [dnl
	if test -d /usr/local/iway ; then
	    ac_cv_sys_iway="yes"
	else
	    ac_cv_sys_iway="no"
	fi
    ])
    dnl
    dnl Put any other possible tests for the IWAY here
    dnl
    if test "$ac_cv_sys_iway" = "yes"; then
	AC_DEFINE(IWAY)
    fi
    AC_MSG_RESULT($ac_cv_sys_iway)
])

AC_DEFUN(AC_CHECK_UNION_WAIT,
[AC_MSG_CHECKING(for union wait)
    AC_CACHE_VAL(ac_cv_type_union_wait, [dnl
	AC_TRY_COMPILE(

#include <sys/wait.h>
,

union wait status;
,
ac_cv_type_union_wait=yes
AC_DEFINE(HAVE_UNION_WAIT)
,
ac_cv_type_union_wait=no
) dnl --End of AC_TRY_COMPILE()
    ]) dnl--Endo fo AC_CACHE_VAL()
    AC_MSG_RESULT($ac_cv_type_union_wait)
])

dnl
dnl "Stolen" from the MPICH distribution
dnl
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

dnl
dnl "Stolen" from the MPICH distribution
dnl
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

AC_DEFUN(AC_CHECK_POSIX_SIGNAL,
[found_ps_funcs="yes"
    AC_MSG_CHECKING(for posix signal in cache)
    AC_CACHE_VAL(ac_cv_check_posix_signal, [dnl
        AC_MSG_RESULT(no. checking manually)
	found_ps_funcs="no"
    ])

    if test "$found_ps_funcs" = "no"; then
	found_ps_funcs="yes"
        AC_CHECK_FUNCS(sigaction sigemptyset sigaddset sigprocmask waitpid,
	# do nothing if it finds the functions
	,
	    found_ps_funcs="no"
	    break) dnl --End of AC_CHECK_FUNCS()
    else
	AC_MSG_RESULT($ac_cv_check_posix_signal);
    fi

    if test "$found_ps_funcs" = "yes" ; then
        ac_cv_check_posix_signal="yes"
        AC_DEFINE(HAVE_POSIX_SIGNAL)
    else
	ac_cv_check_posix_signal="no"
	AC_DEFINE(HAVE_BSD_SIGNAL)
    fi
]) dnl --End of AC_CHECK_POSIX_SIGNALS()

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
