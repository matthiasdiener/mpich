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
dnl     if test -r $dir/lib/lib$1.a ; then
dnl         ac_find_lib_file=$dir/lib/lib$1.a
dnl         break
dnl     fi
done
if test -n "$ac_find_inc_dir" ; then
  AC_MSG_RESULT(found $ac_find_inc_dir)
  ifelse([$3],,,[$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4],,,[$4])
fi
])
