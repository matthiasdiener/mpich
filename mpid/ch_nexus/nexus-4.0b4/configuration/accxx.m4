dnl Check the type of the C++ compiler
AC_DEFUN(LAC_CXX_TYPE,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_MSG_CHECKING(C++ compiler type)
 AC_CACHE_VAL(CXX_TYPE, [dnl
     LAC_EGREP_CPP_RESULT(type,
[#if defined(__CLCC__) || defined(CENTERLINE_CLPP)
  type centerline
#elif (defined(sgi) && defined(mips) && defined(_LANGUAGE_C_PLUS_PLUS))
  type sgiCC
#elif __GNUG__
  type gnu
#elif __DECCXX
  type deccxx
#elif __xlC__
  type xlC
#elif __SUNPRO_CC
  type sunCC4
#elif __PGC__
  type pgc
#endif], CXX_TYPE=`sed -e '/type/ {s/ *type[ ]*//;s/ *//g;}' conftest.out`,
  # The compiler doen't seem to identify itself.  Lets try some other methods...
  echo "int main() { return 0; }" > conftest.$ac_ext
  if `($CXX -F conftest.$ac_ext | egrep 'HP C++' > /dev/null) 2> /dev/null` ; then
  # HP C++ puts a comment into the output file
    CXX_TYPE=HPCC
  elif `($CXX -F conftest.$ac_ext | egrep 'USL C++' > /dev/null) 2> /dev/null` ; then
  # USL Cfront puts a comment into the output file
    CXX_TYPE=USL
  # See if it is a pre version 4 Sun compiler by looking at the banner
  elif `$CXX -V -dryrun conftest.$ac_ext 2>&1 | egrep 'Sun C++' > /dev/null` ; then
    CXX_TYPE=sunCC
  else
    CXX_TYPE=unknown
  fi 
  rm conftest*
     ) dnl -- End of AC_EGREP_CPP
 ]) dnl --End AC_CACHE_VAL()
 AC_MSG_RESULT($CXX_TYPE)
 AC_SUBST(CXX_TYPE)dnl
 AC_PROVIDE([AC_CXX_TYPE])dnl 
]) dnl --End AC_CXX_TYPE()

dnl Check the type of the C++ compiler
AC_DEFUN(LAC_CXX_VERSION,
[AC_REQUIRE([LAC_CXX_TYPE])dnl
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_REQUIRE_CPP 
 AC_MSG_CHECKING(C++ compiler version)
 AC_CACHE_VAL(CXX_VERSION, [dnl
     LAC_EGREP_CPP_RESULT(version,
[#if defined(__CLCC__) || defined(CENTERLINE_CLPP)
  type centerline
#elif __GNUG__
  version __VERSION__
#elif __xlC__
version __xlC__
#elif __SUNPRO_CC
version __SUNPRO_CC
#endif], CXX_VERSION=`sed -e '/version/ {s/ *version[ ]*//;s/ *//;}' conftest.out`,[
  # No predefined version macro, got to poke around looking for it...
  case "${CXX_TYPE}" in
  sunCC)
	# The version number of this compiler is printed by the -V option on stderr.
	# look for the string Sun C++ x.y.z
	echo "int main() { return 0; }" > conftest.$ac_ext
	CXX_VERSION=`$CXX -V -E $conftest.$ac_ext 2>/dev/null | grep version | \
		sed -e '{s/.*Sun C++ *//;}'`
	rm -f conftest*
	;;
  USL)
	# USL Cfront puts the version number in a comment in 
	# output file.
	echo "int main() { return 0; }" > conftest.$ac_ext
	cpp_version=`$CXX -F conftest.$ac_ext | grep USL | \
		sed -e '/USL/ {s/.*<//; s/>.*$//;}'`
	rm -f conftest*
	;;
  HPCC)
	# HP C++ puts the version number in a comment in output file.
  	# String looks like: /* <<HP C++ B2402  A.03.50>> */
	echo "int main() { return 0; }" > conftest.$ac_ext
	CXX_VERSION=`$CXX -F conftest.$ac_ext | grep HP | \
	    sed -e '/HP/ {s/.* A\.//;s/>.*//;}'`
	rm -f conftest*
	;;
  centerline)
	# Centerline Cfront puts the version number in a comment in output file.
	echo "int main() { return 0; }" > conftest.$ac_ext
	CXX_VERSION=`$CXX -F conftest.$ac_ext.C | grep 'ObjectCenter Version' | \
	    sed -e 's/.*ObjectCenter Version *\([0-9.]*\).*/\1/'`
	rm -f conftest*
	;;
  deccxx)
	CXX_VERSION=`$CXX -V 2>/dev/null | grep "DEC C++ V" |
	    $AWK '{ print $3; }'`
    	;;
  *) 
	CXX_VERSION='0'
	;;
  esac]dnl
     ) dnl -- End of AC_EGREP_CPP
 ]) dnl --End AC_CACHE_VAL()
 # We use the hex type version string rather than the x.y.z
 # version string.  This way we can test for version within the preprocessor.
 # Remove dots hyphens and double quotes and spaces.
 CXX_VERSION="`echo $CXX_VERSION | \
   sed -e 's/\.//g' -e 's/\-//g' -e 's/\"//g' -e 's/ //g'"
 case $CXX_VERSION in
   0x???) CXX_VERSION=`echo $CXX_VERSION | sed -e 's/0x/0x0/'` 	;;
   0x*)	  ;;
   ??) 	  CXX_VERSION="0x00$CXX_VERSION";;
   ???)	  CXX_VERSION="0x0$CXX_VERSION"	;;
   *)     CXX_VERSION="0x$CXX_VERSION"  ;;
 esac
 AC_MSG_RESULT($CXX_VERSION)
 AC_SUBST(CXX_VERSION)dnl
]) dnl --End AC_CXX_VERSION()

AC_DEFUN(LAC_CXX_PREDEFINED_INCLUDES,
[AC_REQUIRE([AC_PROG_AWK])dnl
 AC_REQUIRE([LAC_CXX_VERSION])
 AC_REQUIRE([LAC_CXX_TYPE])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 echo "int main() { return 0; }" > conftest.$ac_ext
 cxx_cmd="$CXX -v"
 case "$target_alias-$CXX_TYPE_$CXX_VERSION" in
 *) ac_cxx_predefined_includes="`$cxx_cmd -v conftest.$ac_ext 2>&1 | \
      ${AWK} '{for(i=1;i<=NF;i++) if ($i ~ /-I/) printf(\"%s \",$i);}'`"
 esac
 rm -f conftest.* a.out
 AC_LANG_RESTORE
])

AC_DEFUN(LAC_CXX_PREDEFINED_DEFINES,
[AC_REQUIRE([AC_PROG_AWK])dnl
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 echo "int main() { return 0; }" > conftest.$ac_ext
 cxx_cmd="$CXX -v"
 ac_cxx_predefined_defines="`$cxx_cmd -v conftest.$ac_ext 2>&1 | \
    ${AWK} '{for(i=1;i<=NF;i++) if ($i ~ /-D/) printf(\"%s \",$i);}'`"
 rm -f conftest.* 
 AC_LANG_RESTORE
])

AC_DEFUN(LAC_CXX_INCLUDE_PATHS,
[AC_REQUIRE([AC_PROG_AWK])dnl
 AC_MSG_CHECKING(C++ system includes)
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 LAC_CXX_PREDEFINED_INCLUDES
 ac_config_includedirs=$srcdir/includedirs.awk
cat > conftest.$ac_ext <<EOF
 ac_predefined_includes "$ac_cxx_predefined_includes"
 ac_include_file_list "$1"
EOF
for ac_hdr in $1
do
echo "#include <$ac_hdr>" >> conftest.$ac_ext
done
dnl eval is necessary to expand ac_cpp.
dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
CXX_INCLUDE_PATHS=`(eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC |
  $AWK -f $ac_config_includedirs` 
rm -rf conftest*
AC_LANG_RESTORE
AC_MSG_RESULT($CXX_INCLUDE_PATHS)
AC_SUBST(CXX_INCLUDE_PATHS)
])

AC_DEFUN(LAC_CXX_DEFINES,
[AC_REQUIRE([AC_PROG_AWK])dnl
 AC_REQUIRE([AC_CANONICAL_SYSTEM]) 
 AC_REQUIRE([LAC_CXX_INCLUDE_PATHS])
 AC_MSG_CHECKING(C++ compiler defines)
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 LAC_CXX_PREDEFINED_DEFINES
 ac_config_ifnames=$srcdir/ifnames

 case "$target_alias-$CXX_TYPE-$CXX_VERSION" in
 sparc-sun-solaris2.5-sunCC4-0x0410)
   CXX_DEFINES="-Dsun=1 -D__SUNPRO_CC=0x410 -Dsparc=1 -D__SunOS_5_5=1 -D__STDC__=0 -D_WCHAR_T -D__unix=1 -D__sparc=1 -Dunix=1 -D__cplusplus=1 -D__SVR4=1 -D__BUILTIN_VA_ARG_INCR=1 -D__sun=1"
    ;;
 sparc-sun-solaris2.3-sunCC4-0x0401)
   CXX_DEFINES="-D__SUNPRO_CC=0x401 -Dsun=1 -Dsparc=1 -D__STDC__=0 -D_WCHAR_T -D__unix=1 -D__sparc=1 -Dunix=1 -D__cplusplus=1 -D__SVR4=1 -D__BUILTIN_VA_ARG_INCR=1 -D__sun=1"  
    ;;
 *-ibm-aix3.2.5-xlC-0x0201)
   CXX_DEFINES="-D_AIX=1 -D__STDC__=1 -D__MATH__=1 -D_IBMR2=1 -D__cplusplus=1 -D__xlC__=0x0201 -D__STR__=1"  
    ;;
 *)	
   echo $ac_cxx_predefined_defines | ${AWK} '
    { gsub("-D",""); gsub("=[[^ ]]+",""); split([$]0,sym);
      for (i in sym) 
        printf("#if defined(%s)\n define\"%s\" %s\n#endif\n", 
	  sym[[i]],sym[[i]],sym[[i]]);
    }' > conftest.$ac_ext
   AWK=${AWK} $ac_config_ifnames $CXX_INCLUDE_PATHS >> conftest.$ac_ext
   CXX_DEFINES=`(eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC |
     ${AWK} '/^define/ {sym=[$]2; gsub("\"","",sym); syms[[sym]]=[$]3;}
	END {for (sym in syms) {
	  printf("-D" sym);
	  if (syms[[sym]] != "" ) printf("=" syms[[sym]]);
	  printf(" ");}}'`
 esac 
 rm -rf conftest*
 AC_LANG_RESTORE
 AC_MSG_RESULT($CXX_DEFINES)
 AC_SUBST(CXX_DEFINES)
])


AC_DEFUN(LAC_CXX_WCHAR_T,
[AC_CACHE_CHECK(for C++ wchar, ac_cv_cxx_wchar_t_double,
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(
	[wchar_t foo = 0.0;]
	[exit(sizeof(wchar_t) < sizeof(char));],
	ac_cv_cxx_wchar_t=yes, ac_cv_cxx_wchar_t=no)
if test $ac_cv_c_wchar_t = yes; then
  AC_DEFINE(HAVE_CXX_WCHAR_T)
fi
AC_LANG_RESTORE
])

AC_DEFUN(LAC_NEXUS_PATH,
[  AC_MSG_CHECKING(path to nexus configuration directory....)
if test -z "$NEXUS_PATH"; then
  for lac_nexus_path in nexus_src ../nexus_src ../../nexus_src; do
    if test -d $lac_nexus_path; then
       NEXUS_PATH=$lac_nexus_path
       break
    fi
  done
fi
AC_SUBST(NEXUS_PATH)
test -z "$NEXUS_PATH" && AC_MSG_ERROR([not found])
])
