dnl Get things in a better order than default  
AC_DEFUN(LAC_INIT,
[AC_CONFIG_AUX_DIR(lac_config_path)
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PATH_PROGS(SH, sh bash)
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(SED, sed)
AC_PATH_PROG(AR, ar)
AC_PROG_RANLIB
AC_PROG_MAKE_SET
AC_AIX
configuration_dir=lac_config_path
AC_SUBST(configuration_dir)
for i in lac_config_files; do
  configuration_files="$configuration_files ${srcdir}/$i"
done
AC_SUBST(configuration_files)
])

dnl The BUILD_LIBDIR is a common intermediate
dnl directory for library files.  If this value
dnl is not in the cache, assume the build directory
dnl is in the current subdirectory.
AC_DEFUN(LAC_BUILD_LIBDIR,
[AC_CACHE_VAL(lac_cv_build_dir_lib,
 [ if test -n "$build_dir_lib"; then
      lac_cv_build_dir_lib=$build_dir_lib
   else
      lac_cv_build_dir_lib=`pwd`/lib
   fi ])
AC_CACHE_VAL(lac_cv_build_dir_inc,
 [ if test -n "$build_dir_inc"; then
       lac_cv_build_dir_inc=$build_dir_inc
   else
       lac_cv_build_dir_inc=`pwd`/include
   fi ])

build_dir_lib=$lac_cv_build_dir_lib
build_dir_inc=$lac_cv_build_dir_inc
AC_SUBST(build_dir_lib)
AC_SUBST(build_dir_inc)
])

dnl Use list of makefiles to include a common makefile_header
dnl into each makefile.
AC_DEFUN(LAC_MAKEFILES,
[define(lac_makefile_header,$1)dnl
 lac_makefiles=`echo '$2' | sed 's/\n/ /g'`
 lac_makefile_headers="`echo '$2' | sed 's|Makefile|makefile_header:lac_config_path/$1.in |g'`"
])


dnl Fix up definintion of PROG_LEX so that it can find
dnl lex library.
AC_DEFUN(LAC_PROG_LEX,
[AC_PROVIDE([AC_PROG_LEX])
AC_CHECK_PROG(LEX, flex, flex, lex)
AC_PATH_PROG(ac_lex_path, $LEX)
ac_lex_path=`echo $ac_lex_path | sed s/$LEX//`
if test -z "$LEXLIB"
then
  case "$LEX" in
  flex*) ac_lib=fl
         case "$target" in
	    *solaris2*) LEX_CXXFLAGS="-D__EXTERN_C__" ;;
	 esac
  ;;
  *) ac_lib=l ;;
  esac
  ac_save_LDFLAGS="$LDFLAGS"
  for ac_extension in a so sl; do
    for ac_dir in \
        /usr/local/lib \
        /usr/local/gnu/lib \
        /usr/gnu/lib \
        $ac_lex_path \
        $ac_lex_path/lib \
        ; \
    do 
      if test -r "$ac_dir/lib${ac_lib}.$ac_extension"; then
        ac_lex_lib=$ac_dir
	LDFLAGS="-L$ac_dir $LDFLAGS"
        break
      fi
    done
  done
  AC_CHECK_LIB($ac_lib, yywrap, LEXLIB="-l$ac_lib")
  LDFLAGS=$ac_save_LDFLAGS
fi
if test -n "$ac_lex_lib"; then 
   LEXLIB="-L$ac_lex_lib $LEXLIB"
fi
AC_SUBST(LEXLIB)
AC_SUBST(LEXDEFS)])

dnl Search for a library in a list of possible locations.  
dnl lex library.
AC_DEFUN(LAC_FIND_AND_CHECK_LIB,
[ac_link_save=$ac_link
 for ac_extension in a so sl; do
    for ac_dir in $3; do
      if test -r "$ac_dir/lib${ac_lib}.$ac_extension"; then
        ac_lib_dir=$ac_dir
        ac_link="$ac_link -L$ac_lib_dir"   
        break
      fi
    done
  done
  AC_CHECK_LIB($1, $2, [lac_lib_found=yes],[lac_lib_found=no]) 
  LIBS="-l$$1 $LIBS"
  if test ac_lib_found=yes -a -n "$ac_lib_dir"; then
      LIBS=-L$ac_lib_dir
 fi
])

dnl Same as AC version, but saves result of egrep in file conftest.out
dnl Because this macro is used by AC_PROG_GCC_TRADITIONAL, which must
dnl come early, it is not included in AC_BEFORE checks.
dnl LAC_EGREP_CPP(PATTERN, PROGRAM, ACTION-IF-FOUND [,
dnl              ACTION-IF-NOT-FOUND])
AC_DEFUN(LAC_EGREP_CPP_RESULT,
[AC_REQUIRE_CPP()dnl
cat > conftest.$ac_ext <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
[$2]
EOF
dnl eval is necessary to expand ac_cpp.
dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
if (eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC |
  egrep "$1" >conftest.out 2>&1; then
  ifelse([$3], , :, [$3
 rm -rf conftest*])
ifelse([$4], , , [else
  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*
])

dnl Insert a standard header into a makefile.  The tricky
dnl bit is we have to run variable substitutions over the
dnl header before we insert it.  We also need to get
dnl reletive directories correct. 
AC_DEFUN(LAC_INSERT_MAKEFILE_HEADERS,[
lac_headers=`echo $lac_makefiles | sed 's/Makefile/lac_makefile_header/g'`
 while test -n "$lac_makefiles"; do
   set $lac_headers; lac_header=[$]1; shift; lac_headers=[$]*
   set $lac_makefiles; lac_makefile=[$]1; shift; lac_makefiles=[$]*
   sed "/@lac_makefile_header@/r $lac_header
        s/@lac_makefile_header@//g" $lac_makefile > conf.tmp
   mv conf.tmp $lac_makefile
 done
])
