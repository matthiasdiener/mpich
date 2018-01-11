
dnl -------------------------------------------------------------------------
dnl ----                                                                   ----
dnl ----                       AC_PROG_CC_SHARED                           ----
dnl ----                                                                   ----
dnl ----               Determine what flags and commands are               ----
dnl ----               needed to generate shared libraries                 ----
dnl ----                                                                   ----
dnl ----               Because almost every different vendor               ----
dnl ----               use a completely non-portable set of                ----
dnl ----               flags and procedures, this macro                    ----
dnl ----               pretty much has to end up being a                   ----
dnl ----               big case statement.                                 ----
dnl ----                                                                   ----
dnl ---------------------------------------------------------------------------
AC_DEFUN(LAC_SHARED_LIB,
[SHARED_CCFLAGS=""
MK_SHARED_LIB=""
AC_MSG_CHECKING(how to make shared libraries)
echo "$target--$CC"
case "$target--$CC" in
    hppa*-hp-hpux*--*gcc )
      SHARED_CCFLAGS='-fPIC'
      MK_SHARED_LIB='ld -b -o $[@]'
    ;;
    hppa*-hp-hpux*--*cc )
      SHARED_CCFLAGS='+Z'
      MK_SHARED_LIB='ld -b -o $[@]'
    ;;
    mips-sgi-irix*--*gcc )
      CC_SHARED_OPTS='-fPIC'
      MK_SHARED_LIB='ld -shared -rdata_shared -soname `basename $[@]` -o $[@]'
    ;;
    mips-sgi-irix5*--*cc )
      CC_SHARED_OPTS='-KPIC'
      MK_SHARED_LIB='ld -shared -rdata_shared -soname `basename $[@]` -o $[@]'
    ;;
    mips-sgi-irix6*--*cc )
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
    *-ibm-aix3*--*gcc )
    ;;
    *-ibm-aix3*--*cc )
      link_makefile="makefile.lnk.ibm"
      AC_PATH_PROG(NM,"nm")
      AC_SUBST(NM)
      AC_PROG_AWK
      dnl -- use the lite compiler since the -lpthreads is in the makefile.lnk.ibm
      dnl      MK_SHARED_LIB="$CClite"
      dnl -- the above does not work for the SP so use CCfull
      MK_SHARED_LIB="$CCfull"
    ;;
    *sunos5*--*gcc | *solaris2*--*gcc )
      SOEXT="so"
      SHARED_CFLAGS="-fpic -shared"
      SHARED_LDFLAGS="-shared"
      link_makefile="makefile.lnk"
    ;;
    *sunos5*--*cc | *solaris2*--*cc )
      SOEXT="so"
      SHARED_CFLAGS="-KPIC"
      SHARED_LDFLAGS="-G"
      link_makefile="makefile.lnk"
    ;;
    *sunos4*--*gcc | *solaris1*--*gcc )
      SOEXT="so"
      SHARED_CFLAGS="-fpic -shared"
      SHARED_LDFLAGS="-shared"
      link_makefile="makefile.lnk"
    ;;
    *-*-sysv4--* )
      CC_SHARED_OPTS='-KPIC'
      MK_SHARED_LIB='ld -d y -G -o $[@]'
      SOEXT="so"
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
  link_makefile=$srcdir/$link_makefile
  AC_SUBST(SHARED_CFLAGS)	
  AC_SUBST(SHARED_LDFLAGS)
  AC_SUBST(SOEXT)
  AC_SUBST_FILE(link_makefile)
])
