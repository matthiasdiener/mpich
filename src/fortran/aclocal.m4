dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl
dnl/*D
dnl AC_CACHE_LOAD - Replacement for autoconf cache load 
dnl
dnl Notes:
dnl Caching in autoconf is broken.  The problem is that the cache is read
dnl without any check for whether it makes any sense to read it.
dnl A common problem is a build on a shared file system; connecting to 
dnl a different computer and then building within the same directory will
dnl lead to at best error messages from configure and at worse a build that
dnl is wrong but fails only at run time (e.g., wrong datatype sizes used).
dnl 
dnl This fixes that by requiring the user to explicitly enable caching 
dnl before the cache file will be loaded.
dnl
dnl To use this version of 'AC_CACHE_LOAD', you need to include
dnl 'aclocal_cache.m4' in your 'aclocal.m4' file.  The sowing 'aclocal.m4'
dnl file includes this file.
dnl
dnl If no --enable-cache or --disable-cache option is selected, the
dnl command causes configure to keep track of the system being configured
dnl in a config.system file; if the current system matches the value stored
dnl in that file (or there is neither a config.cache nor config.system file),
dnl configure will enable caching.  In order to ensure that the configure
dnl tests make sense, the values of CC, F77, F90, and CXX are also included 
dnl in the config.system file.
dnl
dnl See Also:
dnl PAC_ARG_CACHING
dnlD*/
define([AC_CACHE_LOAD],
[if test "X$cache_system" = "X" ; then
    if test "$cache_file" != "/dev/null" ; then
        # Get the directory for the cache file, if any
	changequote(,)
        cache_system=`echo $cache_file | sed -e 's%^\(.*/\)[^/]*%\1/config.system%'`
	changequote([,])
        test "x$cache_system" = "x$cache_file" && cache_system="config.system"
    fi
fi
dnl
dnl The "action-if-not-given" part of AC_ARG_ENABLE is not executed until
dnl after the AC_CACHE_LOAD is executed (!).  Thus, the value of 
dnl enable_cache if neither --enable-cache or --disable-cache is selected
dnl is null.  Just in case autoconf ever fixes this, we test both cases.
if test "X$enable_cache" = "Xnotgiven" -o "X$enable_cache" = "X" ; then
    # check for valid cache file
    if uname -srm >/dev/null 2>&1 ; then
	dnl cleanargs=`echo "$*" | tr '"' ' '`
	cleanargs=`echo "$CC $F77 $CXX $F90" | tr '"' ' '`
        testval="`uname -srm` $cleanargs"
        if test -f "$cache_system" -a -n "$testval" ; then
	    if test "$testval" = "`cat $cache_system`" ; then
	        enable_cache="yes"
	    fi
        elif test ! -f "$cache_system" -a -n "$testval" ; then
	    echo "$testval" > $cache_system
	    # remove the cache file because it may not correspond to our
	    # system
	    rm -f $cache_file
	    enable_cache="yes"
        fi
    fi
fi
if test "X$enable_cache" = "Xyes" ; then
  if test -r "$cache_file" ; then
    echo "loading cache $cache_file"
    . $cache_file
  else
    echo "creating cache $cache_file"
    > $cache_file
    rm -f $cache_system
    cleanargs=`echo "$CC $F77 $CXX" | tr '"' ' '`
    testval="`uname -srm` $cleanargs"
    echo "$testval" > $cache_system
  fi
else
  cache_file="/dev/null"
fi
])
dnl
dnl/*D 
dnl PAC_ARG_CACHING - Enable caching of results from a configure execution
dnl
dnl Synopsis:
dnl PAC_ARG_CACHING
dnl
dnl Output Effects:
dnl Adds '--enable-cache' and '--disable-cache' to the command line arguments
dnl accepted by 'configure'.  
dnl
dnl See Also:
dnl AC_CACHE_LOAD
dnlD*/
dnl Add this call to the other ARG_ENABLE calls.  Note that the values
dnl set here are redundant; the LOAD_CACHE call relies on the way autoconf
dnl initially processes ARG_ENABLE commands.
AC_DEFUN(PAC_ARG_CACHING,[
AC_ARG_ENABLE(cache,
[--enable-cache  - Turn on configure caching],
enable_cache="$enableval",enable_cache="notgiven")
])

dnl
dnl/*D 
dnl PAC_LIB_MPI - Check for MPI library
dnl
dnl Synopsis:
dnl PAC_LIB_MPI([action if found],[action if not found])
dnl
dnl Output Effect:
dnl
dnl Notes:
dnl Currently, only checks for lib mpi and mpi.h.  Later, we will add
dnl MPI_Pcontrol prototype (const int or not?).  
dnl
dnl If PAC_ARG_MPICH_BUILDING is included, this will work correctly 
dnl when MPICH is being built.
dnl
dnl Prerequisites:
dnl autoconf version 2.13 (for AC_SEARCH_LIBS)
dnlD*/
dnl Other tests to add:
dnl Version of MPI
dnl MPI-2 I/O?
dnl MPI-2 Spawn?
dnl MPI-2 RMA?
dnl PAC_LIB_MPI([found text],[not found text])
AC_DEFUN(PAC_LIB_MPI,[
AC_PREREQ(2.13)
if test "X$pac_lib_mpi_is_building" != "Xyes" ; then
  # Use CC if TESTCC is defined
  if test "X$pac_save_level" != "X" ; then
     pac_save_TESTCC="${TESTCC}"
     pac_save_TESTCPP="${TESTCPP}"
     CC="$pac_save_CC"
     if test "X$pac_save_CPP" != "X" ; then
         CPP="$pac_save_CPP"
     fi
  fi
  # Look for MPILIB first if it is defined
  AC_SEARCH_LIBS(MPI_Init,$MPILIB mpi mpich)
  if test "$ac_cv_search_MPI_Init" = "no" ; then
    ifelse($2,,
    AC_MSG_ERROR([Could not find MPI library]),[$2])
  fi
  AC_CHECK_HEADER(mpi.h,pac_have_mpi_h="yes",pac_have_mpi_h="no")
  if test $pac_have_mpi_h = "no" ; then
    ifelse($2,,
    AC_MSG_ERROR([Could not find mpi.h include file]),[$2])
  fi
  if test "X$pac_save_level" != "X" ; then
     CC="$pac_save_TESTCC"
     CPP="$pac_save_TESTCPP"
  fi
fi
ifelse($1,,,[$1])
])
dnl
dnl
dnl/*D
dnl PAC_ARG_MPICH_BUILDING - Add configure command-line argument to indicated
dnl that MPICH is being built
dnl
dnl Output Effect:
dnl Adds the command-line switch '--with-mpichbuilding' that may be used to
dnl indicate that MPICH is building.  This allows a configure to work-around
dnl the fact that during a build of MPICH, certain commands, particularly the
dnl compilation commands such as 'mpicc', are not yet functional.  The
dnl variable 'pac_lib_mpi_is_building' is set to 'yes' if in an MPICH build,
dnl 'no' otherwise.
dnl
dnl See Also:
dnl PAC_LIB_MPI
dnlD*/
AC_DEFUN(PAC_ARG_MPICH_BUILDING,[
AC_ARG_WITH(mpichbuilding,
[--with-mpichbuilding - Assume that MPICH is being built],
pac_lib_mpi_is_building=$withval,pac_lib_mpi_is_building="no")
])
dnl
dnl
dnl This should also set MPIRUN.
dnl
dnl/*D
dnl PAC_ARG_MPI_TYPES - Add command-line switches for different MPI 
dnl environments
dnl
dnl Synopsis:
dnl PAC_ARG_MPI_TYPES([default])
dnl
dnl Output Effects:
dnl Adds the following command line options to configure
dnl+ \-\-with\-mpich[=path] - MPICH.  'path' is the location of MPICH commands
dnl. \-\-with\-ibmmpi - IBM MPI
dnl- \-\-with\-sgimpi - SGI MPI
dnl If no type is selected, and a default ("mpich", "ibmmpi", or "sgimpi")
dnl is given, that type is used as if '--with-<default>' was given.
dnl
dnl Sets 'CC', 'F77', 'TESTCC', 'TESTF77', and 'MPILIBNAME'.  Does `not`
dnl perform an AC_SUBST for these values.
dnl
dnl See also:
dnl PAC_LANG_PUSH_COMPILERS, PAC_LIB_MPI
dnlD*/
AC_DEFUN(PAC_ARG_MPI_TYPES,[
AC_PROVIDE([AC_PROG_CC])
AC_SUBST(CC)
AC_SUBST(CXX)
AC_SUBST(F77)
AC_SUBST(F90)
AC_ARG_WITH(mpich,
[--with-mpich=path  - Assume that we are building with MPICH],
ac_mpi_type=mpich)
AC_ARG_WITH(ibmmpi,
[--with-ibmmpi    - Use the IBM SP implementation of MPI],
ac_mpi_type=ibmmpi)
AC_ARG_WITH(sgimpi,
[--with-sgimpi    - Use the SGI implementation of MPI],
ac_mpi_type=sgimpi)
if test "X$ac_mpi_type" = "X" ; then
    if test "X$1" != "X" ; then
        ac_mpi_type=$1
    else
        ac_mpi_type=unknown
    fi
fi
case $ac_mpi_type in
	mpich)
        dnl 
        dnl This isn't correct.  It should try to get the underlying compiler
        dnl from the mpicc and mpif77 scripts or mpireconfig
        if test "X$pac_lib_mpi_is_building" != "Xyes" ; then
            save_PATH="$PATH"
            if test "$with_mpich" != "yes" -a "$with_mpich" != "no" ; then 
		# Look for commands; if not found, try adding bin to the
		# path
		if test ! -x $with_mpich/mpicc -a -x $with_mpich/bin/mpicc ; then
			with_mpich="$with_mpich/bin"
		fi
                PATH=$with_mpich:${PATH}
            fi
            AC_PATH_PROG(MPICC,mpicc)
            TESTCC=${CC-cc}
            CC="$MPICC"
            AC_PATH_PROG(MPIF77,mpif77)
            TESTF77=${F77-f77}
            F77="$MPIF77"
            AC_PATH_PROG(MPIF90,mpif90)
            TESTF90=${F90-f90}
            F90="$MPIF90"
            AC_PATH_PROG(MPICXX,mpiCC)
            TESTCXX=${CXX-CC}
            CXX="$MPICXX"
	    PATH="$save_PATH"
  	    MPILIBNAME="mpich"
        else 
	    # All of the above should have been passed in the environment!
	    :
        fi
	;;
	ibmmpi)
	TESTCC=${CC-xlC}; TESTF77=${F77-xlf}; CC=mpcc; F77=mpxlf
	# There is no mpxlf90, but the options langlvl and free can
	# select the F90 version of xlf
	TESTF90=${F90-xlf90}; F90="mpxlf -qlanglvl=90ext -qfree=f90"
	MPILIBNAME=""
	;;
	sgimpi)
	TESTCC=${CC:=cc}; TESTF77=${F77:=f77}; 
	TESTCXX=${CXX:=CC}; TESTF90=${F90:=f90}
	AC_CHECK_LIB(mpi,MPI_Init)
	if test "$ac_cv_lib_mpi_MPI_Init" = "yes" ; then
	    MPILIBNAME="mpi"
	fi	
	;;
	*)
	;;
esac
])
dnl
dnl/*D
dnl PAC_MPI_F2C - Determine if MPI has the MPI-2 functions MPI_xxx_f2c and
dnl   MPI_xxx_c2f
dnl
dnl Output Effect:
dnl Define 'HAVE_MPI_F2C' if the routines are found.
dnl
dnl Notes:
dnl Looks only for 'MPI_Request_c2f'.
dnlD*/
AC_DEFUN(PAC_MPI_F2C,[
AC_CACHE_CHECK([for MPI F2C and C2F routines],
pac_cv_mpi_f2c,
[
AC_TRY_LINK([#include "mpi.h"],
[MPI_Request request;MPI_Fint a;a = MPI_Request_c2f(request);],
pac_cv_mpi_f2c="yes",pac_cv_mpi_f2c="no")
])
if test "$pac_cv_mpi_f2c" = "yes" ; then 
    AC_DEFINE(HAVE_MPI_F2C) 
fi
])

dnl
dnl This is a replacement for AC_PROG_CC that does not prefer gcc and
dnl that does not mess with CFLAGS.  See acspecific.m4 for the original defn.
dnl
dnl/*D
dnl PAC_PROG_CC - Find a working C compiler
dnl
dnl Synopsis:
dnl PAC_PROG_CC
dnl
dnl Output Effect:
dnl   Sets the variable CC if it is not already set
dnl
dnl Notes:
dnl   Unlike AC_PROG_CC, this does not prefer gcc and does not set CFLAGS.
dnl   It does check that the compiler can compile a simple C program.
dnl   It also sets the variable GCC to yes if the compiler is gcc.  It does
dnl   not yet check for some special options needed in particular for 
dnl   parallel computers, such as -Tcray-t3e, or special options to get
dnl   full ANSI/ISO C, such as -Aa for HP.
dnl
dnlD*/
AC_DEFUN(PAC_PROG_CC,[
AC_PROVIDE([AC_PROG_CC])
AC_CHECK_PROGS(CC, cc xlC xlc pgcc gcc )
test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
PAC_PROG_CC_WORKS
AC_PROG_CC_GNU
if test $ac_cv_prog_gcc = yes; then
  GCC=yes
else
  GCC=
fi
])
dnl
dnl/*D
dnl PAC_C_CHECK_COMPILER_OPTION - Check that a compiler option is accepted
dnl without warning messages
dnl
dnl Synopsis:
dnl PAC_C_CHECK_COMPILER_OPTION(optionname,action-if-ok,action-if-fail)
dnl
dnl Output Effects:
dnl
dnl If no actions are specified, a working value is added to 'COPTIONS'
dnl
dnl Notes:
dnl This is now careful to check that the output is different, since 
dnl some compilers are noisy.
dnl 
dnl We are extra careful to prototype the functions in case compiler options
dnl that complain about poor code are in effect.
dnl
dnl Because this is a long script, we have ensured that you can pass a 
dnl variable containing the option name as the first argument.
dnlD*/
AC_DEFUN(PAC_C_CHECK_COMPILER_OPTION,[
AC_MSG_CHECKING([that C compiler accepts option $1])
save_CFLAGS="$CFLAGS"
CFLAGS="$1 $CFLAGS"
rm -f conftest.out
echo 'int try(void);int try(void){return 0;}' > conftest2.c
echo 'int main(void);int main(void){return 0;}' > conftest.c
if ${CC-cc} $save_CFLAGS $CPPFLAGS -o conftest conftest.c >conftest.bas 2>&1 ; then
   if ${CC-cc} $CFLAGS $CPPFLAGS -o conftest conftest.c >conftest.out 2>&1 ; then
      if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
         AC_MSG_RESULT(yes)
         AC_MSG_CHECKING([that routines compiled with $1 can be linked with ones compiled  without $1])       
         /bin/rm -f conftest.out
         /bin/rm -f conftest.bas
         if ${CC-cc} -c $save_CFLAGS $CPPFLAGS conftest2.c >conftest2.out 2>&1 ; then
            if ${CC-cc} $CFLAGS $CPPFLAGS -o conftest conftest2.o conftest.c >conftest.bas 2>&1 ; then
               if ${CC-cc} $CFLAGS $CPPFLAGS -o conftest conftest2.o conftest.c >conftest.out 2>&1 ; then
                  if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
	             AC_MSG_RESULT(yes)	  
		     CFLAGS="$save_CFLAGS"
                     ifelse($2,,COPTIONS="$COPTIONS $1",$2)
                  elif test -s conftest.out ; then
	             cat conftest.out >&AC_FD_CC
	             AC_MSG_RESULT(no)
                     CFLAGS="$save_CFLAGS"
	             $3
                  else
                     AC_MSG_RESULT(no)
                     CFLAGS="$save_CFLAGS"
	             $3
                  fi  
               else
	          if test -s conftest.out ; then
	             cat conftest.out >&AC_FD_CC
	          fi
                  AC_MSG_RESULT(no)
                  CFLAGS="$save_CFLAGS"
                  $3
               fi
	    else
               # Could not link with the option!
               AC_MSG_RESULT(no)
            fi
         else
            if test -s conftest2.out ; then
               cat conftest.out >&AC_FD_CC
            fi
	    AC_MSG_RESULT(no)
            CFLAGS="$save_CFLAGS"
	    $3
         fi
      else
         cat conftest.out >&AC_FD_CC
         AC_MSG_RESULT(no)
         $3
         CFLAGS="$save_CFLAGS"         
      fi
   else
      AC_MSG_RESULT(no)
      $3
      if test -s conftest.out ; then cat conftest.out >&AC_FD_CC ; fi    
      CFLAGS="$save_CFLAGS"
   fi
else
    # Could not compile without the option!
    AC_MSG_RESULT(no)
fi
rm -f conftest*
])
dnl
dnl/*D
dnl PAC_C_OPTIMIZATION - Determine C options for producing optimized code
dnl
dnl Synopsis
dnl PAC_C_OPTIMIZATION([action if found])
dnl
dnl Output Effect:
dnl Adds options to 'COPTIONS' if no other action is specified
dnl 
dnl Notes:
dnl This is a temporary standin for compiler optimization.
dnl It should try to match known systems to known compilers (checking, of
dnl course), and then falling back to some common defaults.
dnl Note that many compilers will complain about -g and aggressive
dnl optimization.  
dnlD*/
AC_DEFUN(PAC_C_OPTIMIZATION,[
    for copt in "-O4 -Ofast" "-Ofast" "-fast" "-O3" "-xO3" "-O" ; do
        PAC_C_CHECK_COMPILER_OPTION($copt,found_opt=yes,found_opt=no)
        if test $found_opt = "yes" ; then
	    ifelse($1,,COPTIONS="$COPTIONS $copt",$1)
	    break
        fi
    done
])
dnl
dnl/*D
dnl PAC_C_DEPENDS - Determine how to use the C compiler to generate 
dnl dependency information
dnl
dnl Synopsis:
dnl PAC_C_DEPENDS
dnl
dnl Output Effects:
dnl Sets the following shell variables and call AC_SUBST for them:
dnl+ C_DEPEND_OPT - Compiler options needed to create dependencies
dnl. C_DEPEND_OUT - Shell redirection for dependency file (may be empty)
dnl. C_DEPEND_PREFIX - Empty (null) or true; this is used to handle
dnl  systems that do not provide dependency information
dnl- C_DEPEND_MV - Command to move created dependency file
dnl Also creates a Depends file in the top directory (!).
dnl
dnl In addition, the variable 'C_DEPEND_DIR' must be set to indicate the
dnl directory in which the dependency files should live.  
dnl
dnl Notes:
dnl A typical Make rule that exploits this macro is
dnl.vb
dnl #
dnl # Dependency processing
dnl .SUFFIXES: .dep
dnl DEP_SOURCES = ${SOURCES:%.c=.dep/%.dep}
dnl C_DEPEND_DIR = .dep
dnl Depends: ${DEP_SOURCES}
dnl         @-rm -f Depends
dnl         cat .dep/*.dep >Depends
dnl .dep/%.dep:%.c
dnl	    @if [ ! -d .dep ] ; then mkdir .dep ; fi
dnl         @@C_DEPEND_PREFIX@ ${C_COMPILE} @C_DEPEND_OPT@ $< @C_DEPEND_OUT@
dnl         @@C_DEPEND_MV@
dnl
dnl depends-clean:
dnl         @-rm -f *.dep ${srcdir}/*.dep Depends ${srcdir}/Depends
dnl         @-touch Depends
dnl.ve
dnl
dnl For each file 'foo.c', this creates a file 'foo.dep' and creates a file
dnl 'Depends' that contains all of the '*.dep' files.
dnl
dnl For your convenience, the autoconf variable 'C_DO_DEPENDS' names a file 
dnl that may contain this code (you must have `dependsrule` or 
dnl `dependsrule.in` in the same directory as the other auxillery configure 
dnl scripts (set with dnl 'AC_CONFIG_AUX_DIR').  If you use `dependsrule.in`,
dnl you must have `dependsrule` in 'AC_OUTPUT' before this `Makefile`.
dnl 
dnlD*/
dnl 
dnl Eventually, we can add an option to the C_DEPEND_MV to strip system
dnl includes, such as /usr/xxxx and /opt/xxxx
dnl
AC_DEFUN(PAC_C_DEPENDS,[
AC_SUBST(C_DEPEND_OPT)AM_IGNORE(C_DEPEND_OPT)
AC_SUBST(C_DEPEND_OUT)AM_IGNORE(C_DEPEND_OUT)
AC_SUBST(C_DEPEND_MV)AM_IGNORE(C_DEPEND_MV)
AC_SUBST(C_DEPEND_PREFIX)AM_IGNORE(C_DEPEND_PREFIX)
AC_SUBST_FILE(C_DO_DEPENDS) 
dnl set the value of the variable to a 
dnl file that contains the dependency code, such as
dnl ${top_srcdir}/maint/dependrule 
if test -n "$ac_cv_c_depend_opt" ; then
    AC_MSG_RESULT([Option $ac_cv_c_depend_opt creates dependencies (cached)])
    C_DEPEND_OUT="$ac_cv_c_depend_out"
    C_DEPEND_MV="$ac_cv_c_depend_mv"
    C_DEPEND_OPT="$ac_cv_c_depend_opt"
    C_DEPEND_PREFIX="$ac_cv_c_depend_prefix"
    C_DO_DEPENDS="$ac_cv_c_do_depends"
else
   # Determine the values
rm -f conftest*
dnl
dnl Some systems (/usr/ucb/cc on Solaris) do not generate a dependency for
dnl an include that doesn't begin in column 1
dnl
cat >conftest.c <<EOF
    #include "confdefs.h"
    int f(void) { return 0; }
EOF
dnl -xM1 is Solaris C compiler (no /usr/include files)
dnl -MM is gcc (no /usr/include files)
dnl -MMD is gcc to .d
dnl .u is xlC (AIX) output
for copt in "-xM1" "-c -xM1" "-xM" "-c -xM" "-MM" "-M" "-c -M"; do
    AC_MSG_CHECKING([whether $copt option generates dependencies])
    rm -f conftest.o conftest.u conftest.d conftest.err conftest.out
    dnl also need to check that error output is empty
    if $CC $CFLAGS $copt conftest.c >conftest.out 2>conftest.err && \
	test ! -s conftest.err ; then
        dnl Check for dependency info in conftest.out
        if test -s conftest.u ; then 
	    C_DEPEND_OUT=""
	    C_DEPEND_MV='mv $[*].u ${C_DEPEND_DIR}/$[*].dep'
            pac_dep_file=conftest.u 
        elif test -s conftest.d ; then
	    C_DEPEND_OUT=""
	    C_DEPEND_MV='mv $[*].d ${C_DEPEND_DIR}/$[*].dep'
            pac_dep_file=conftest.d 
        else
	    dnl C_DEPEND_OUT='>${C_DEPEND_DIR}/$[*].dep'
	    dnl This for is needed for VPATH.  Perhaps the others should match.
	    C_DEPEND_OUT='>$@'
	    C_DEPEND_MV=:
            pac_dep_file=conftest.out
        fi
        if grep 'confdefs.h' $pac_dep_file >/dev/null 2>&1 ; then
            AC_MSG_RESULT(yes)
	    C_DEPEND_OPT="$copt"
	    AC_MSG_CHECKING([whether .o file created with dependency file])
	    if test -s conftest.o ; then
	        AC_MSG_RESULT(yes)
	    else
                AC_MSG_RESULT(no)
		echo "Output of $copt option was" >&AC_FD_CC
		cat $pac_dep_file >&AC_FD_CC
            fi
	    break
        else
	    AC_MSG_RESULT(no)
        fi
    else
	echo "Error in compiling program with flags $copt" >&AC_FD_CC
	cat conftest.out >&AC_FD_CC
	if test -s conftest.err ; then cat conftest.err >&AC_FD_CC ; fi
	AC_MSG_RESULT(no)
    fi
    copt=""
done
    if test -f $CONFIG_AUX_DIR/dependsrule -o \
	    -f $CONFIG_AUX_DIR/dependsrule.in; then
	C_DO_DEPENDS="$CONFIG_AUX_DIR/dependsrule"
    else 
	C_DO_DEPENDS="/dev/null"
    fi
    if test "X$copt" = "X" ; then
        C_DEPEND_PREFIX="true"
    else
        C_DEPEND_PREFIX=""
    fi
    ac_cv_c_depend_out="$C_DEPEND_OUT"
    ac_cv_c_depend_mv="$C_DEPEND_MV"
    ac_cv_c_depend_opt="$C_DEPEND_OPT"
    ac_cv_c_depend_prefix="$C_DEPEND_PREFIX"
    ac_cv_c_do_depends="$C_DO_DEPENDS"
fi
])
dnl
dnl/*D 
dnl PAC_C_PROTOTYPES - Check that the compiler accepts ANSI prototypes.  
dnl
dnl Synopsis:
dnl PAC_C_PROTOTYPES([action if true],[action if false])
dnl
dnlD*/
AC_DEFUN(PAC_C_PROTOTYPES,[
AC_CACHE_CHECK([if $CC supports function prototypes],
pac_cv_c_prototypes,[
AC_TRY_COMPILE([int f(double a){return 0;}],[return 0];,
pac_cv_c_prototypes="yes",pac_cv_c_prototypes="no")])
if test "$pac_cv_c_prototypes" = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl/*D
dnl PAC_FUNC_SEMCTL - Check for semctl and its argument types
dnl
dnl Synopsis:
dnl PAC_FUNC_SEMCTL
dnl
dnl Output Effects:
dnl Sets 'HAVE_SEMCTL' if semctl is available.
dnl Sets 'HAVE_UNION_SEMUN' if 'union semun' is available.
dnl Sets 'SEMCTL_NEEDS_SEMUN' if a 'union semun' type must be passed as the
dnl fourth argument to 'semctl'.
dnlD*/ 
dnl Check for semctl and arguments
AC_DEFUN(PAC_FUNC_SEMCTL,[
AC_CHECK_FUNC(semctl)
if test "$ac_cv_func_semctl" = "yes" ; then
    AC_CACHE_CHECK([for union semun],
    pac_cv_type_union_semun,[
    AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>],[union semun arg;arg.val=0;],
    pac_cv_type_union_semun="yes",pac_cv_type_union_semun="no")])
    if test "$pac_cv_type_union_semun" = "yes" ; then
        AC_DEFINE(HAVE_UNION_SEMUN)
        #
        # See if we can use an int in semctl or if we need the union
        AC_CACHE_CHECK([whether semctl needs union semun],
        pac_cv_func_semctl_needs_semun,[
        AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>],[
int arg = 0; semctl( 1, 1, SETVAL, arg );],
        pac_cv_func_semctl_needs_semun="yes",
        pac_cv_func_semctl_needs_semun="no")
        ])
        if test "$pac_cv_func_semctl_needs_semun" = "yes" ; then
            AC_DEFINE(SEMCTL_NEEDS_SEMUN)
        fi
    fi
fi
])
dnl
dnl/*D
dnl PAC_C_VOLATILE - Check if C supports volatile
dnl
dnl Synopsis:
dnl PAC_C_VOLATILE
dnl
dnl Output Effect:
dnl Defines 'volatile' as empty if volatile is not available.
dnl
dnlD*/
AC_DEFUN(PAC_C_VOLATILE,[
AC_CACHE_CHECK([for volatile],
pac_cv_c_volatile,[
AC_TRY_COMPILE(,[volatile int a;],pac_cv_c_volatile="yes",
pac_cv_c_volatile="no")])
if test "$pac_cv_c_volatile" = "no" ; then
    AC_DEFINE(volatile,)
fi
])dnl
dnl
dnl/*D
dnl PAC_C_CPP_CONCAT - Check whether the C compiler accepts ISO CPP string
dnl   concatenation
dnl
dnl Synopsis:
dnl PAC_C_CPP_CONCAT([true-action],[false-action])
dnl
dnl Output Effects:
dnl Invokes the true or false action
dnl
dnlD*/
AC_DEFUN(PAC_C_CPP_CONCAT,[
pac_pound="#"
AC_CACHE_CHECK([that the compiler $CC accepts $ac_pound$ac_pound for concatenation in cpp],
pac_cv_c_cpp_concat,[
AC_TRY_COMPILE([
#define concat(a,b) a##b],[int concat(a,b);return ab;],
pac_cv_cpp_concat="yes",pac_cv_cpp_concat="no")])
if test $pac_cv_c_cpp_concat = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])dnl
dnl
dnl/*D
dnl PAC_FUNC_GETTIMEOFDAY - Check whether gettimeofday takes 1 or 2 arguments
dnl
dnl Synopsis
dnl  PAC_IS_GETTIMEOFDAY_OK(ok_action,failure_action)
dnl
dnl Notes:
dnl One version of Solaris accepted only one argument.
dnl
dnlD*/
AC_DEFUN(PAC_FUNC_GETTIMEOFDAY,[
AC_CACHE_CHECK([whether gettimeofday takes 2 arguments],
pac_cv_func_gettimeofday,[
AC_TRY_COMPILE([#include <sys/time.h>],[struct timeval tp;
gettimeofday(&tp,(void*)0);return 0;],pac_cv_func_gettimeofday="yes",
pac_cv_func_gettimeofday="no")
])
if test "$pac_cv_func_gettimeofday" = "yes" ; then
     ifelse($1,,:,$1)
else
     ifelse($2,,:,$2)
fi
])
dnl
dnl/*D
dnl PAC_C_RESTRICT - Check if C supports restrict
dnl
dnl Synopsis:
dnl PAC_C_RESTRICT
dnl
dnl Output Effect:
dnl Defines 'restrict' if some version of restrict is supported; otherwise
dnl defines 'restrict' as empty.  This allows you to include 'restrict' in 
dnl declarations in the same way that 'AC_C_CONST' allows you to use 'const'
dnl in declarations even when the C compiler does not support 'const'
dnl
dnlD*/
AC_DEFUN(PAC_C_RESTRICT,[
AC_CACHE_CHECK([for restrict],
pac_cv_c_restrict,[
AC_TRY_COMPILE(,[int * restrict a;],pac_cv_c_restrict="restrict",
pac_cv_c_restrict="no")
if test "$pac_cv_c_restrict" = "no" ; then
   AC_TRY_COMPILE(,[int * _Restrict a;],pac_cv_c_restrict="_Restrict",
   pac_cv_c_restrict="no")
fi
if test "$pac_cv_c_restrict" = "no" ; then
   AC_TRY_COMPILE(,[int * __restrict a;],pac_cv_c_restrict="__restrict",
   pac_cv_c_restrict="no")
fi
])
if test "$pac_cv_c_restrict" = "no" ; then
  AC_DEFINE(restrict, )
elif test "$pac_cv_c_restrict" != "restrict" ; then
  AC_DEFINE_UNQUOTED(restrict,$pac_cv_c_restrict)
fi
])dnl
dnl
dnl/*D
dnl PAC_HEADER_STDARG - Check whether standard args are defined and whether
dnl they are old style or new style
dnl
dnl Synopsis:
dnl PAC_HEADER_STDARG(action if works, action if oldstyle, action if fails)
dnl
dnl Output Effects:
dnl Defines HAVE_STDARG_H if the header exists.
dnl defines 
dnl
dnl Notes:
dnl It isn't enough to check for stdarg.  Even gcc doesn't get it right;
dnl on some systems, the gcc version of stdio.h loads stdarg.h `with the wrong
dnl options` (causing it to choose the `old style` 'va_start' etc).
dnl
dnl The original test tried the two-arg version first; the old-style
dnl va_start took only a single arg.
dnl This turns out to be VERY tricky, because some compilers (e.g., Solaris) 
dnl are quite happy to accept the *wrong* number of arguments to a macro!
dnl Instead, we try to find a clean compile version, using our special
dnl PAC_C_TRY_COMPILE_CLEAN command.
dnl
dnlD*/
AC_DEFUN(PAC_HEADER_STDARG,[
AC_CHECK_HEADER(stdarg.h)
dnl Sets ac_cv_header_stdarg_h
if test "$ac_cv_header_stdarg_h" = "yes" ; then
    dnl results are yes,oldstyle,no.
    AC_CACHE_CHECK([whether stdarg is oldstyle],
    pac_cv_header_stdarg_oldstyle,[
PAC_C_TRY_COMPILE_CLEAN([#include <stdio.h>
#include <stdarg.h>],
[int func( int a, ... ){
int b;
va_list ap;
va_start( ap );
b = va_arg(ap, int);
printf( "%d-%d\n", a, b );
va_end(ap);
fflush(stdout);
return 0;
}
int main() { func( 1, 2 ); return 0;}],pac_check_compile)
case $pac_check_compile in 
    0)  pac_cv_header_stdarg_oldstyle="yes"
	;;
    1)  pac_cv_header_stdarg_oldstyle="may be newstyle"
	;;
    2)  pac_cv_header_stdarg_oldstyle="no"   # compile failed
	;;
esac
])
if test "$pac_cv_header_stdarg_oldstyle" = "yes" ; then
    ifelse($2,,:,[$2])
else
    AC_CACHE_CHECK([whether stdarg works],
    pac_cv_header_stdarg_works,[
    PAC_C_TRY_COMPILE_CLEAN([
#include <stdio.h>
#include <stdarg.h>],[
int func( int a, ... ){
int b;
va_list ap;
va_start( ap, a );
b = va_arg(ap, int);
printf( "%d-%d\n", a, b );
va_end(ap);
fflush(stdout);
return 0;
}
int main() { func( 1, 2 ); return 0;}],pac_check_compile)
case $pac_check_compile in 
    0)  pac_cv_header_stdarg_works="yes"
	;;
    1)  pac_cv_header_stdarg_works="yes with warnings"
	;;
    2)  pac_cv_header_stdarg_works="no"
	;;
esac
])
fi   # test on oldstyle
if test "$pac_cv_header_stdarg_works" = "no" ; then
    ifelse($3,,:,[$3])
else
    ifelse($1,,:,[$1])
fi
else 
    ifelse($3,,:,[$3])
fi  # test on header
])
dnl/*D
dnl PAC_C_TRY_COMPILE_CLEAN - Try to compile a program, separating success
dnl with no warnings from success with warnings.
dnl
dnl Synopsis:
dnl PAC_C_TRY_COMPILE_CLEAN(header,program,flagvar)
dnl
dnl Output Effect:
dnl The 'flagvar' is set to 0 (clean), 1 (dirty but success ok), or 2
dnl (failed).
dnl
dnlD*/
AC_DEFUN(PAC_C_TRY_COMPILE_CLEAN,[
$3=2
dnl Get the compiler output to test against
if test -z "$pac_TRY_COMPLILE_CLEAN" ; then
    rm -f conftest*
    echo 'int try(void);int try(void){return 0;}' > conftest.c
    if ${CC-cc} $CFLAGS -c conftest.c >conftest.bas 2>&1 ; then
	if test -s conftest.bas ; then 
	    pac_TRY_COMPILE_CLEAN_OUT=`cat conftest.bas`
        fi
        pac_TRY_COMPILE_CLEAN=1
    else
	AC_MSG_WARN([Could not compile simple test program!])
	if test -s conftest.bas ; then 	cat conftest.bas >> config.log ; fi
    fi
fi
dnl
dnl Create the program that we need to test with
rm -f conftest*
cat >conftest.c <<EOF
#include "confdefs.h"
[$1]
[$2]
EOF
dnl
dnl Compile it and test
if ${CC-cc} $CFLAGS -c conftest.c >conftest.bas 2>&1 ; then
    dnl Success.  Is the output the same?
    if test "$pac_TRY_COMPILE_CLEAN_OUT" = "`cat conftest.bas`" ; then
	$3=0
    else
        cat conftest.c >>config.log
	if test -s conftest.bas ; then 	cat conftest.bas >> config.log ; fi
        $3=1
    fi
else
    dnl Failure.  Set flag to 2
    cat conftest.c >>config.log
    if test -s conftest.bas ; then cat conftest.bas >> config.log ; fi
    $3=2
fi
rm -f conftest*
])
dnl
dnl/*D
dnl PAC_PROG_C_UNALIGNED_DOUBLES - Check that the C compiler allows unaligned
dnl doubles
dnl
dnl Synopsis:
dnl   PAC_PROG_C_UNALIGNED_DOUBLES(action-if-true,action-if-false,
dnl       action-if-unknown)
dnl
dnl Notes:
dnl 'action-if-unknown' is used in the case of cross-compilation.
dnlD*/
AC_DEFUN(PAC_PROG_C_UNALIGNED_DOUBLES,[
AC_CACHE_CHECK([whether C compiler allows unaligned doubles],
pac_cv_prog_c_unaligned_doubles,[
AC_TRY_RUN([
void fetch_double( v )
double *v;
{
*v = 1.0;
}
int main( argc, argv )
int argc;
char **argv;
{
int p[4];
double *p_val;
fetch_double( (double *)&(p[0]) );
p_val = (double *)&(p[0]);
if (*p_val != 1.0) return 1;
fetch_double( (double *)&(p[1]) );
p_val = (double *)&(p[1]);
if (*p_val != 1.0) return 1;
return 0;
}
],pac_cv_prog_c_unaligned_doubles="yes",pac_cv_prog_c_unaligned_doubles="no",
pac_cv_prog_c_unaligned_doubles="unknown")])
ifelse($1,,,if test "X$pac_cv_prog_c_unaligned_doubles" = "yes" ; then 
$1
fi)
ifelse($2,,,if test "X$pac_cv_prog_c_unaligned_doubles" = "no" ; then 
$2
fi)
ifelse($3,,,if test "X$pac_cv_prog_c_unaligned_doubles" = "unknown" ; then 
$3
fi)
])
dnl
dnl/*D 
dnl PAC_PROG_C_WEAK_SYMBOLS - Test whether C supports weak symbols.
dnl
dnl Synopsis
dnl PAC_PROG_C_WEAK_SYMBOLS(action-if-true,action-if-false)
dnl
dnl Output Effect:
dnl Defines one of the following if a weak symbol pragma is found:
dnl.vb
dnl    HAVE_PRAGMA_WEAK - #pragma weak
dnl    HAVE_PRAGMA_HP_SEC_DEF - #pragma _HP_SECONDARY_DEF
dnl    HAVE_PRAGMA_CRI_DUP) - #pragma _CRI duplicate x as y
dnl.ve
dnl 
dnlD*/
AC_DEFUN(PAC_PROG_C_WEAK_SYMBOLS,[
AC_CACHE_CHECK([for type of weak symbol support],
pac_cv_prog_c_weak_symbols,[
# Test for weak symbol support...
# We can't put # in the message because it causes autoconf to generate
# incorrect code
AC_TRY_LINK([
#pragma weak PFoo = Foo
int Foo(a) { return a; }
],[return PFoo(1);],pac_cv_prog_c_weak_symbols="pragma weak")
dnl
if test -z "$pac_cv_prog_c_weak_symbols" ; then 
    AC_TRY_LINK([
#pragma _HP_SECONDARY_DEF Foo  PFoo
int Foo(a) { return a; }
],[return PFoo(1);],pac_cv_prog_c_weak_symbols="pragma _HP_SECONDARY_DEF")
fi
dnl
if test -z "$pac_cv_prog_c_weak_symbols" ; then
    AC_TRY_LINK([
#pragma _CRI duplicate PFoo as Foo
int Foo(a) { return a; }
],[return PFoo(1);],pac_cv_prog_c_weak_symbols="pragma _CRI duplicate x as y")
fi
dnl
if test -z "$pac_cv_prog_c_weak_symbols" ; then
    pac_cv_prog_c_weak_symbols="no"
fi
])
dnl
if test "$pac_cv_prog_c_weak_symbols" = "no" ; then
    ifelse([$2],,:,[$2])
else
    case "$pac_cv_prog_c_weak_symbols" in
	"pragma weak") AC_DEFINE(HAVE_PRAGMA_WEAK) 
	;;
	"pragma _HP")  AC_DEFINE(HAVE_PRAGMA_HP_SEC_DEF)
	;;
	"pragma _CRI") AC_DEFINE(HAVE_PRAGMA_CRI_DUP)
	;;
    esac
    ifelse([$1],,:,[$1])
fi
])
#
# This is a replacement that checks that FAILURES are signaled as well
# (later configure macros look for the .o file, not just success from the
# compiler, but they should not HAVE to
#
AC_DEFUN(PAC_PROG_CC_WORKS,
[AC_PROG_CC_WORKS
AC_MSG_CHECKING([whether the C compiler sets its return status correctly])
AC_LANG_SAVE
AC_LANG_C
AC_TRY_COMPILE(,[int a = bzzzt;],notbroken=no,notbroken=yes)
AC_MSG_RESULT($notbroken)
if test "$notbroken" = "no" ; then
    AC_MSG_ERROR([installation or configuration problem: C compiler does not
correctly set error code when a fatal error occurs])
fi
])

AC_DEFUN(AM_IGNORE,[])

dnl
dnl Macros for Fortran 90
dnl
dnl We'd like to have a PAC_LANG_FORTRAN90 that worked with AC_TRY_xxx, but
dnl that would require too many changes to autoconf macros.
AC_DEFUN(PAC_LANG_FORTRAN90,
[AC_REQUIRE([PAC_PROG_F90])
define([AC_LANG], [FORTRAN90])dnl
ac_ext=$pac_cv_f90_ext
ac_compile='${F90-f90} -c $F90FLAGS conftest.$ac_ext 1>&AC_FD_CC'
ac_link='${F90-f90} -o conftest${ac_exeext} $F90FLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&AC_FD_CC'
cross_compiling=$pac_cv_prog_f90_cross
])
dnl
dnl This is an addition for AC_TRY_COMPILE, but for f90.  If the current 
dnl language is not f90, it does a save/restore
AC_DEFUN(PAC_TRY_F90_COMPILE,
[AC_REQUIRE([PAC_LANG_FORTRAN90])
ifelse(AC_LANG, FORTRAN90,,[AC_LANG_SAVE
PAC_LANG_FORTRAN90
define([NEED_POP],yes)])
cat > conftest.$ac_ext <<EOF
      program main
[$2]
      end
EOF
if AC_TRY_EVAL(ac_compile); then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
ifelse([$4], , , [  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*
ifelse(NEED_POP,yes,[
undefine([NEED_POP])
AC_LANG_RESTORE)]
])
dnl
dnl PAC_F90_MODULE_EXT(action if found,action if not found)
dnl
AC_DEFUN(PAC_F90_MODULE_EXT,
[AC_CACHE_CHECK([for Fortran 90 module extension],
pac_cv_f90_module_ext,[
pac_cv_f90_module_case="unknown"
cat >conftest.$ac_f90ext <<EOF
	module conftest
        integer n
        parameter (n=1)
        end module conftest
EOF
if AC_TRY_EVAL(ac_f90compile) ; then
   dnl Look for module name
   pac_MOD=`ls conftest* 2>&1 | grep -v conftest.$ac_f90ext | grep -v conftest.o`
   pac_MOD=`echo $pac_MOD | sed -e 's/conftest\.//g'`
   pac_cv_f90_module_case="lower"
   if test "X$pac_MOD" = "X" ; then
	pac_MOD=`ls CONFTEST* 2>&1 | grep -v CONFTEST.f | grep -v CONFTEST.o`
        pac_MOD=`echo $pac_MOD | sed -e 's/CONFTEST\.//g'`
	if test -n "$pac_MOD" ; then
	    testname="CONFTEST"
	    pac_cv_f90_module_case="upper"
	fi
    fi
    if test -z "$pac_MOD" ; then 
	pac_cv_f90_module_ext="unknown"
    else
	pac_cv_f90_module_ext=$pac_MOD
    fi
else
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.$ac_f90ext >&AC_FD_CC
    pac_cv_f90_module_ext="unknown"
fi
rm -f conftest*
])
AC_SUBST(F90MODEXT)
if test "$pac_cv_f90_module_ext" = "unknown" ; then
    ifelse($2,,:,[$2])
else
    ifelse($1,,F90MODEXT=$pac_MOD,[$1])
fi
])
dnl
dnl PAC_F90_MODULE_INCFLAG
AC_DEFUN(PAC_F90_MODULE_INCFLAG,[
AC_CACHE_CHECK([for Fortran 90 module include flag],
pac_cv_f90_module_incflag,[
AC_REQUIRE([PAC_F90_MODULE_EXT])
cat >conftest.$ac_f90ext <<EOF
	module conf
        integer n
        parameter (n=1)
        end module conf
EOF
pac_madedir="no"
if test ! -d conf ; then mkdir conf ; pac_madedir="yes"; fi
if test "$pac_cv_f90_module_case" = "upper" ; then
    pac_module="CONF.$pac_cv_f90_module_ext"
else
    pac_module="conf.$pac_cv_f90_module_ext"
fi
if AC_TRY_EVAL(ac_f90compile) ; then
    cp $pac_module conf
else
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.$ac_f90ext >&AC_FD_CC
fi
rm -f conftest.$ac_f90ext
cat >conftest.$ac_f90ext <<EOF
    program main
    use conf
    end
EOF
if ${F90-f90} -c $F90FLAGS -Iconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test -s conftest.o ; then
    pac_cv_f90_module_incflag="-I"
elif ${F90-f90} -c $F90FLAGS -Mconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test-s conftest.o ; then
    pac_cv_f90_module_incflag="-M"
elif ${F90-f90} -c $F90FLAGS -pconf conftest.$ac_f90ext 1>&AC_FD_CC && \
	test -s conftest.o ; then
    pac_cv_f90_module_incflag="-p"
else
    pac_cv_f90_module_incflag="unknown"
fi
if test "$pac_madedir" = "yes" ; then rm -rf conf ; fi
rm -f conftest*
])
AC_SUBST(F90MODINCFLAG)
F90MODINCFLAG=$pac_cv_f90_module_incflag
])
AC_DEFUN(PAC_F90_MODULE,[
PAC_F90_MODULE_EXT
PAC_F90_MODULE_INCFLAG
])
AC_DEFUN(PAC_F90_EXT,[
AC_CACHE_CHECK([whether Fortran 90 accepts f90 suffix],
pac_cv_f90_ext_f90,[
save_ac_f90ext=$ac_f90ext
ac_f90ext="f90"
PAC_TRY_F90_COMPILE(,,pac_cv_f90_ext_f90="yes",pac_cv_f90_ext_f90="no")
ac_f90ext=$save_ac_f90ext
])
])
dnl
dnl/*D 
dnl PAC_PROG_F90_INT_KIND - Determine kind parameter for an integer with
dnl the specified number of bytes.
dnl
dnl Synopsis:
dnl  PAC_PROG_F90_INT_KIND(variable-to-set,number-of-bytes,[cross-size])
dnl
dnlD*/
AC_DEFUN(PAC_PROG_F90_INT_KIND,[
# Set the default
$1=-1
if test "$pac_cv_prog_f90_cross" = "yes" ; then
    $1="$3"
else
if test -n "$ac_f90compile" ; then
    AC_MSG_CHECKING([for Fortran 90 integer kind for $2-byte integers])
    # Convert bytes to digits
    case $2 in 
	1) sellen=2 ;;
	2) sellen=4 ;;
	4) sellen=8 ;;
	8) sellen=16 ;;
       16) sellen=30 ;;
	*) sellen=8 ;;
    esac
    # Check for cached value
    eval testval=\$"pac_cv_prog_f90_int_kind_$sellen"
    if test -n "$testval" ; then 
        AC_MSG_RESULT([$testval (cached)])
	$1=$testval
    else
        # must compute
        rm -f conftest*
        cat <<EOF > conftest.$ac_f90ext
      program main
      integer i
      i = selected_int_kind($sellen)
      open(8, file="conftest1.out", form="formatted")
      write (8,*) i
      close(8)
      stop
      end
EOF
        KINDVAL="unavailable"
        eval "pac_cv_prog_f90_int_kind_$sellen"=-1
        if AC_TRY_EVAL(ac_f90link) && test -s conftest ; then
            ./conftest >>config.log 2>&1
            if test -s conftest1.out ; then
	        # Because of write, there may be a leading blank.
                KINDVAL=`cat conftest1.out | sed 's/ //g'`
 	        eval "pac_cv_prog_f90_int_kind_$sellen"=$KINDVAL
	        $1=$KINDVAL
            fi
        fi
        rm -f conftest*
	AC_MSG_RESULT($KINDVAL)
    fi # not cached
fi # Has Fortran 90
fi # is not cross compiling
])dnl
dnl
dnl
dnl Note: This checks for f95 before f90, since F95 is the more recent
dnl revision of Fortran 90.
AC_DEFUN(PAC_PROG_F90,[
if test -z "$F90" ; then
    AC_CHECK_PROGS(F90,f95 f90 xlf90 pgf90)
    test -z "$F90" && AC_MSG_WARN([no acceptable Fortran 90 compiler found in \$PATH])
fi
if test -n "$F90" ; then
     PAC_PROG_F90_WORKS
fi
dnl Cache these so we don't need to change in and out of f90 mode
ac_f90ext=$pac_cv_f90_ext
ac_f90compile='${F90-f90} -c $F90FLAGS conftest.$ac_f90ext 1>&AC_FD_CC'
ac_f90link='${F90-f90} -o conftest${ac_exeext} $F90FLAGS $LDFLAGS conftest.$ac_f90ext $LIBS 1>&AC_FD_CC'
])
dnl Internal routine for testing F90
dnl PAC_PROG_F90_WORKS()
AC_DEFUN(PAC_PROG_F90_WORKS,
[AC_MSG_CHECKING([for extension for Fortran 90 programs])
pac_cv_f90_ext="f90"
cat > conftest.$pac_cv_f90_ext <<EOF
      program conftest
      end
EOF
ac_compile='${F90-f90} -c $F90FLAGS conftest.$pac_cv_f90_ext 1>&AC_FD_CC'
if AC_TRY_EVAL(ac_compile) ; then
    AC_MSG_RESULT([f90])
else
    rm -f conftest*
    pac_cv_f90_ext="f"
    cat > conftest.$pac_cv_f90_ext <<EOF
      program conftest
      end
EOF
    if AC_TRY_EVAL(ac_compile) ; then
	AC_MSG_RESULT([f])
    else
        AC_MSG_RESULT([unknown!])
    fi
fi
AC_MSG_CHECKING([whether the Fortran 90 compiler ($F90 $F90FLAGS $LDFLAGS) works])
AC_LANG_SAVE
PAC_LANG_FORTRAN90
cat >conftest.$ac_ext <<EOF
      program conftest
      end
EOF
if AC_TRY_EVAL(ac_link) && test -s conftest${ac_exeect} ; then
    pac_cv_prog_f90_works="yes"
    if (./conftest; exit) 2>/dev/null ; then
        pac_cv_prog_f90_cross="no"
    else
        pac_cv_prog_f90_cross="yes"
    fi
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  pac_cv_prog_f90_works="no"
fi
rm -f conftest*
AC_LANG_RESTORE
AC_MSG_RESULT($pac_cv_prog_f90_works)
if test $pac_cv_prog_f90_works = no; then
  AC_MSG_WARN([installation or configuration problem: Fortran 90 compiler cannot create executables.])
fi
AC_MSG_CHECKING([whether the Fortran 90 compiler ($F90 $F90FLAGS $LDFLAGS) is a cross-compiler])
AC_MSG_RESULT($pac_cv_prog_f90_cross)
cross_compiling=$pac_cv_prog_f90_cross
])
dnl
dnl The following looks for F90 options to enable th specified f90 compiler
dnl to work with the f77 compiler, particularly for accessing command-line
dnl arguments

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
dnl alleged make).
dnl
dnl It is the OSF V3 make, and can't handle a comment in a block of target
dnl code.  There is no acceptable fix.
dnl
dnl
dnl
dnl
dnl Find a make program if none is defined.
AC_DEFUN(PAC_PROG_MAKE_PROGRAM,[true
if test "X$MAKE" = "X" ; then
   AC_CHECK_PROGS(MAKE,make gnumake nmake pmake smake)
fi
])dnl
dnl/*D
dnl PAC_PROG_MAKE_ECHOS_DIR - Check whether make echos all directory changes
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_ECHOS_DIR
dnl
dnl Output Effect:
dnl  If make echos directory changes, append '--no-print-directory' to the 
dnl  symbol 'MAKE'.  If 'MAKE' is not set, chooses 'make' for 'MAKE'.
dnl
dnl See also:
dnl PAC_PROG_MAKE
dnlD*/
dnl
AC_DEFUN(PAC_PROG_MAKE_ECHOS_DIR,[
AC_CACHE_CHECK([whether make echos directory changes],
pac_cv_prog_make_echos_dir,
[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
/bin/rm -f conftest
cat > conftest <<.
SHELL=/bin/sh
ALL:
	@(dir=`pwd` ; cd .. ; \$(MAKE) -f \$\$dir/conftest SUB)
SUB:
	@echo "success"
.
str=`$MAKE -f conftest 2>&1`
if test "$str" != "success" ; then
    str=`$MAKE --no-print-directory -f conftest 2>&1`
    if test "$str" = "success" ; then
	pac_cv_prog_make_echos_dir="yes using --no-print-directory"
    else
	pac_cv_prog_make_echos_dir="no"
    fi
else
    pac_cv_prog_make_echos_dir="no"
fi
/bin/rm -f conftest
str=""
])
if test "$pac_cv_prog_make_echos_dir" = "yes using --no-print-directory" ; then
    MAKE="$MAKE --no-print-directory"
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE_INCLUDE - Check whether make supports include
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_INCLUDE([action if true],[action if false])
dnl
dnl Output Effect:
dnl   None
dnl
dnl Notes:
dnl  This checks for makes that do not support 'include filename'.  Some
dnl  versions of BSD 4.4 make required '#include' instead; some versions of
dnl  'pmake' have the same syntax.
dnl
dnl See Also:
dnl  PAC_PROG_MAKE
dnl
dnlD*/
dnl
AC_DEFUN(PAC_PROG_MAKE_INCLUDE,[
AC_CACHE_CHECK([whether make supports include],pac_cv_prog_make_include,[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
/bin/rm -f conftest
cat > conftest <<.
ALL:
	@echo "success"
.
cat > conftest1 <<.
include conftest
.
pac_str=`$MAKE -f conftest1 2>&1`
/bin/rm -f conftest conftest1
if test "$pac_str" != "success" ; then
    pac_cv_prog_make_include="no"
else
    pac_cv_prog_make_include="yes"
fi
])
if test "$pac_cv_prog_make_include" = "no" ; then
    ifelse([$2],,:,[$2])
else
    ifelse([$1],,:,[$1])
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE_ALLOWS_COMMENTS - Check whether comments are allowed in 
dnl   shell commands in a makefile
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_ALLOWS_COMMENTS([false text])
dnl
dnl Output Effect:
dnl Issues a warning message if comments are not allowed in a makefile.
dnl Executes the argument if one is given.
dnl
dnl Notes:
dnl Some versions of OSF V3 make do not all comments in action commands.
dnl
dnl See Also:
dnl  PAC_PROG_MAKE
dnlD*/
dnl
AC_DEFUN(PAC_PROG_MAKE_ALLOWS_COMMENTS,[
AC_CACHE_CHECK([whether make allows comments in actions],
pac_cv_prog_make_allows_comments,[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
/bin/rm -f conftest
cat > conftest <<.
SHELL=/bin/sh
ALL:
	@# This is a valid comment!
	@echo "success"
.
pac_str=`$MAKE -f conftest 2>&1`
/bin/rm -f conftest 
if test "$pac_str" != "success" ; then
    pac_cv_prog_make_allows_comments="no"
else
    pac_cv_prog_make_allows_comments="yes"
fi
])
if test "$pac_cv_prog_make_allows_comments" = "no" ; then
    AC_MSG_WARN([Your make does not allow comments in target code.
Using this make may cause problems when building programs.
You should consider using gnumake instead.])
    ifelse([$1],,[$1])
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE_VPATH - Check whether make supports source-code paths.
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_VPATH
dnl
dnl Output Effect:
dnl Sets the variable 'VPATH' to either
dnl.vb
dnl VPATH = .:${srcdir}
dnl.ve
dnl or
dnl.vb
dnl .PATH: . ${srcdir}
dnl.ve
dnl 
dnl Notes:
dnl The test checks that the path works with implicit targets (some makes
dnl support only explicit targets with 'VPATH' or 'PATH').
dnl
dnl NEED TO DO: Check that $< works on explicit targets.
dnl
dnl See Also:
dnl PAC_PROG_MAKE
dnl
dnlD*/
dnl
AC_DEFUN(PAC_PROG_MAKE_VPATH,[
AC_SUBST(VPATH)AM_IGNORE(VPATH)
AC_CACHE_CHECK([for virtual path format],
pac_cv_prog_make_vpath,[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
rm -rf conftest*
mkdir conftestdir
cat >conftestdir/a.c <<EOF
A sample file
EOF
cat > conftest <<EOF
all: a.o
VPATH=.:conftestdir
.c.o:
	@echo \$<
EOF
ac_out=`$MAKE -f conftest 2>&1 | grep 'conftestdir/a.c'`
if test -n "$ac_out" ; then 
    pac_cv_prog_make_vpath="VPATH"
else
    rm -f conftest
    cat > conftest <<EOF
all: a.o
.PATH: . conftestdir
.c.o:
	@echo \$<
EOF
    ac_out=`$MAKE -f conftest 2>&1 | grep 'conftestdir/a.c'`
    if test -n "$ac_out" ; then 
        pac_cv_prog_make_vpath=".PATH"
    else
	pac_cv_prog_make_vpath="neither VPATH nor .PATH works"
    fi
fi
rm -rf conftest*
])
if test "$pac_cv_prog_make_vpath" = "VPATH" ; then
    VPATH='VPATH=.:${srcdir}'
elif test "$pac_cv_prog_make_vpath" = ".PATH" ; then
    VPATH='.PATH: . ${srcdir}'
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE_SET_CFLAGS - Check whether make sets CFLAGS
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_SET_CFLAGS([action if true],[action if false])
dnl
dnl Output Effects:
dnl Executes the first argument if 'CFLAGS' is set by 'make'; executes
dnl the second argument if 'CFLAGS' is not set by 'make'.
dnl
dnl Notes:
dnl If 'CFLAGS' is set by make, you may wish to override that choice in your
dnl makefile.
dnl
dnl See Also:
dnl PAC_PROG_MAKE
dnlD*/
AC_DEFUN(PAC_PROG_MAKE_SET_CFLAGS,[
AC_CACHE_CHECK([whether make sets CFLAGS],
pac_cv_prog_make_set_cflags,[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
/bin/rm -f conftest
cat > conftest <<EOF
SHELL=/bin/sh
ALL:
	@echo X[\$]{CFLAGS}X
EOF
pac_str=`$MAKE -f conftest 2>&1`
/bin/rm -f conftest 
if test "$pac_str" = "XX" ; then
    pac_cv_prog_make_set_cflags="no"
else
    pac_cv_prog_make_set_cflags="yes"
fi
])
if test "$pac_cv_prog_make_set_cflags" = "no" ; then
    ifelse([$2],,:,[$2])
else
    ifelse([$1],,:,[$1])
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE_HAS_PATTERN_RULES - Determine if the make program supports
dnl pattern rules
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE_HAS_PATTERN_RULES([action if true],[action if false])
dnl
dnl Output Effect:
dnl Executes the first argument if patterns of the form
dnl.vb
dnl   prefix%suffix: prefix%suffix
dnl.ve
dnl are supported by make (gnumake and Solaris make are known to support
dnl this form of target).  If patterns are not supported, executes the
dnl second argument.
dnl
dnl See Also:
dnl PAC_PROG_MAKE
dnl 
dnlD*/
AC_DEFUN(PAC_PROG_MAKE_HAS_PATTERN_RULES,[
AC_CACHE_CHECK([whether make has pattern rules],
pac_cv_prog_make_has_patterns,[
AC_REQUIRE([PAC_PROG_MAKE_PROGRAM])
rm -f conftest*
cat > conftestmm <<EOF
# Test for pattern rules
.SUFFIXES:
.SUFFIXES: .dep .c
conftest%.dep: %.c
	@cat \[$]< >\[$]@
EOF
date > conftest.c
if ${MAKE} -f conftestmm conftestconftest.dep 1>&AC_FD_CC 2>&1 ; then
    pac_cv_prog_make_has_patterns="yes"
else
    pac_cv_prog_make_has_patterns="no"
fi
rm -f conftest*
])
if test "$pac_cv_prog_make_has_patterns" = "no" ; then
    ifelse([$2],,:,[$2])
else
    ifelse([$1],,:,[$1])
fi
])dnl
dnl
dnl/*D
dnl PAC_PROG_MAKE - Checks for the varieties of MAKE, including support for 
dnl VPATH
dnl
dnl Synopsis:
dnl PAC_PROG_MAKE
dnl
dnl Output Effect:
dnl Sets 'MAKE' to the make program to use if 'MAKE' is not already set.
dnl Sets the variable 'SET_CFLAGS' to 'CFLAGS =' if make sets 'CFLAGS'.
dnl
dnl Notes:
dnl This macro uses 'PAC_PROG_MAKE_ECHOS_DIR', 'PAC_PROG_MAKE_INCLUDE',
dnl 'PAC_PROG_MAKE_ALLOWS_COMMENTS', 'PAC_PROG_MAKE_VPATH', and
dnl 'PAC_PROG_MAKE_SET_CFLAGS'.  See those commands for details about their
dnl actions.
dnl 
dnl It may call 'AC_PROG_MAKE_SET', which sets 'SET_MAKE' to 'MAKE = @MAKE@'
dnl if the make program does not set the value of make, otherwise 'SET_MAKE'
dnl is set to empty; if the make program echos the directory name, then 
dnl 'SET_MAKE' is set to 'MAKE = $MAKE'.
dnlD*/
dnl
AC_DEFUN(PAC_PROG_MAKE,[
PAC_PROG_MAKE_PROGRAM
PAC_PROG_MAKE_ECHOS_DIR
PAC_PROG_MAKE_INCLUDE
PAC_PROG_MAKE_ALLOWS_COMMENTS
PAC_PROG_MAKE_VPATH
dnl 
dnl We're not using patterns any more, and Compaq/DEC OSF-1 sometimes hangs
dnl at this test
dnl PAC_PROG_MAKE_HAS_PATTERN_RULES
AC_SUBST(SET_CFLAGS)AM_IGNORE(SET_CFLAGS)
PAC_PROG_MAKE_SET_CFLAGS([SET_CFLAGS='CFLAGS='])
if test "$pac_cv_prog_make_echos_dir" = "no" ; then
    AC_PROG_MAKE_SET
else
    SET_MAKE="MAKE=${MAKE-make}"
fi
])

dnl
dnl/*D
dnl PAC_LANG_PUSH_COMPILERS - Replace all compilers with test versions 
dnl
dnl Synopsis:
dnl PAC_LANG_PUSH_COMPILERS
dnl
dnl Output Effects:
dnl The values of 'CC', 'CXX', 'F77', 'F90', and 'CPP' are replaced with
dnl the values of 'TESTCC' etc.  The old values are saved (see 
dnl 'PAC_LANG_POP_COMPILERS').
dnl 
dnl Calls to this macro may be nested, but only the outer-most calls have
dnl any effect.
dnl
dnl See also:
dnl PAC_LANG_POP_COMPILERS
dnlD*/
dnl
dnl These two name allow you to use TESTCC for CC, etc, in all of the 
dnl autoconf compilation tests.  This is useful, for example, when the
dnl compiler needed at the end cannot be used to build programs that can 
dnl be run, for example, as required by some parallel computing systems.
dnl Instead, define TESTCC, TESTCXX, TESTF77, and TESTF90 as the "local"
dnl compilers.  Because autoconf insists on calling cpp for the header 
dnl checks, we use TESTCPP for the CPP test as well.  And if no TESTCPP 
dnl is defined, we create one using TESTCC.
dnl
AC_DEFUN(PAC_LANG_PUSH_COMPILERS,[
if test "X$pac_save_level" = "X" ; then
    pac_save_CC="$CC"
    pac_save_CXX="$CXX"
    pac_save_F77="$F77"
    pac_save_F90="$F90"
    pac_save_prog_cc_cross="$ac_cv_prog_cc_cross"
    pac_save_prog_f77_cross="$ac_cv_prog_f77_cross"
    pac_save_prog_cxx_cross="$ac_cv_prog_cxx_cross"
    pac_save_prog_f90_cross="$pac_cv_prog_f90_cross"
    if test "X$CPP" = "X" ; then
	AC_PROG_CPP
    fi
    pac_save_CPP="$CPP"
    CC="${TESTCC:=$CC}"
    CXX="${TESTCXX:=$CXX}"
    F77="${TESTF77:=$F77}"
    F90="${TESTF90:=$F90}"
    if test -z "$TESTCPP" ; then
        PAC_PROG_TESTCPP
    fi
    CPP="${TESTCPP:=$CPP}"
    pac_save_level="0"
    # Recompute cross_compiling values and set for the current language
    # This is just:
    AC_LANG_SAVE
    AC_LANG_C
    AC_TRY_COMPILER([main(){return(0);}], ac_cv_prog_cc_works, ac_cv_prog_cc_cross)
    AC_LANG_RESTORE
    # Ignore Fortran if we aren't using it.
    if test -n "$F77" ; then
        AC_LANG_SAVE
        AC_LANG_FORTRAN77
        AC_TRY_COMPILER(dnl
[      program conftest
      end
], ac_cv_prog_f77_works, ac_cv_prog_f77_cross)
        AC_LANG_RESTORE
    fi
    # Ignore C++ if we aren't using it.
    if test -n "$CXX" ; then
        AC_LANG_SAVE
        AC_LANG_CPLUSPLUS
        AC_TRY_COMPILER([int main(){return(0);}], ac_cv_prog_cxx_works, ac_cv_prog_cxx_cross)
        AC_LANG_RESTORE
    fi
    # Ignore Fortran 90 if we aren't using it.
    if test -n "$F90" ; then
        AC_LANG_SAVE
        PAC_LANG_FORTRAN90
	dnl We can't use AC_TRY_COMPILER because it doesn't know about 
        dnl Fortran 90
        cat > conftest.$ac_ext << EOF
      program conftest
      end
EOF
        if { (eval echo configure:2324: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
          ac_cv_prog_f90_works=yes
          # If we can't run a trivial program, we are probably using a cross compiler.
          if (./conftest; exit) 2>/dev/null; then
              ac_cv_prog_f90_cross=no
          else
              ac_cv_prog_f90_cross=yes
          fi
        else
          echo "configure: failed program was:" >&5
          cat conftest.$ac_ext >&5
          ac_cv_prog_f90_works=no
        fi
	pac_cv_prog_f90_cross="$ac_cv_prog_f90_cross"
	pac_cv_prog_f90_works="$ac_cv_prog_f90_works"
        rm -fr conftest*
        AC_LANG_RESTORE
    fi
fi
pac_save_level=`expr $pac_save_level + 1`
])
dnl/*D
dnl PAC_LANG_POP_COMPILERS - Restore compilers that were displaced by
dnl PAC_LANG_PUSH_COMPILERS
dnl
dnl Synopsis:
dnl PAC_LANG_POP_COMPILERS
dnl
dnl Output Effects:
dnl The values of 'CC', 'CXX', 'F77', 'F90', and 'CPP' are replaced with
dnl their original values from the outermost call to 'PAC_LANG_PUSH_COMPILERS'.
dnl 
dnl Calls to this macro may be nested, but only the outer-most calls have
dnl any effect.
dnl
dnl See also:
dnl PAC_LANG_PUSH_COMPILERS
dnlD*/
AC_DEFUN(PAC_LANG_POP_COMPILERS,[
pac_save_level=`expr $pac_save_level - 1`
if test "X$pac_save_level" = "X0" ; then
    CC="$pac_save_CC"
    CXX="$pac_save_CXX"
    F77="$pac_save_F77"
    F90="$pac_save_F90"
    CPP="$pac_save_CPP"
    ac_cv_prog_cc_cross="$pac_save_prog_cc_cross"
    ac_cv_prog_f77_cross="$pac_save_prog_f77_cross"
    ac_cv_prog_cxx_cross="$pac_save_prog_cxx_cross"
    pac_cv_prog_f90_cross="$pac_save_prog_f90_cross"
    pac_save_level=""
fi
])
AC_DEFUN(PAC_PROG_TESTCPP,[
if test -z "$TESTCPP"; then
  AC_CACHE_VAL(pac_cv_prog_TESTCPP,[
  rm -f conftest.*
  cat > conftest.c <<EOF
  #include <assert.h>
  Syntax Error
EOF
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp.
  TESTCPP="${TESTCC-cc} -E"
  ac_try="$TESTCPP conftest.c >/dev/null 2>conftest.out"
  if AC_TRY_EVAL(ac_try) ; then
      pac_cv_prog_TESTCPP="$TESTCPP"
  fi
  if test "X$pac_cv_prog_TESTCPP" = "X" ; then
      TESTCPP="${TESTCC-cc} -E -traditional-cpp"
      ac_try="$TESTCPP conftest.c >/dev/null 2>conftest.out"
      if AC_TRY_EVAL(ac_try) ; then
          pac_cv_prog_TESTCPP="$TESTCPP"
      fi
  fi
  if test "X$pac_cv_prog_TESTCPP" = "X" ; then
      TESTCPP="${TESTCC-cc} -nologo -E"
      ac_try="$TESTCPP conftest.c >/dev/null 2>conftest.out"
      if AC_TRY_EVAL(ac_try) ; then
          pac_cv_prog_TESTCPP="$TESTCPP"
      fi
  fi
  if test "X$pac_cv_prog_TESTCPP" = "X" ; then
      AC_PATH_PROG(TESTCPP,cpp)
  fi
  rm -f conftest.*
  ])
else
  pac_cv_prog_TESTCPP="$TESTCPP"
fi
])

dnl
dnl/*D
dnl PAC_PROG_F77_NAME_MANGLE - Determine how the Fortran compiler mangles
dnl names 
dnl
dnl Synopsis:
dnl PAC_PROG_F77_NAME_MANGLE([action])
dnl
dnl Output Effect:
dnl If no action is specified, one of the following names is defined:
dnl.vb
dnl If fortran names are mapped:
dnl   lower -> lower                  F77_NAME_LOWER
dnl   lower -> lower_                 F77_NAME_LOWER_USCORE
dnl   lower -> UPPER                  F77_NAME_UPPER
dnl   lower_lower -> lower__          F77_NAME_LOWER_2USCORE
dnl   mixed -> mixed                  F77_NAME_MIXED
dnl   mixed -> mixed_                 F77_NAME_MIXED_USCORE
dnl.ve
dnl If an action is specified, it is executed instead.
dnl 
dnl Notes:
dnl We assume that if lower -> lower (any underscore), upper -> upper with the
dnl same underscore behavior.  Previous versions did this by 
dnl compiling a Fortran program and running strings -a over it.  Depending on 
dnl strings is a bad idea, so instead we try compiling and linking with a 
dnl C program, since that is why we are doing this anyway.  A similar approach
dnl is used by FFTW, though without some of the cases we check (specifically, 
dnl mixed name mangling)
dnl
dnlD*/
dnl
AC_DEFUN(PAC_PROG_F77_NAME_MANGLE,[
AC_CACHE_CHECK([for Fortran 77 name mangling],
pac_cv_prog_f77_name_mangle,
[
   # Check for strange behavior of Fortran.  For example, some FreeBSD
   # systems use f2c to implement f77, and the version of f2c that they 
   # use generates TWO (!!!) trailing underscores
   # Currently, WDEF is not used but could be...
   #
   # Eventually, we want to be able to override the choices here and
   # force a particular form.  This is particularly useful in systems
   # where a Fortran compiler option is used to force a particular
   # external name format (rs6000 xlf, for example).
   rm -f conftest*
   cat > conftest.f <<EOF
       subroutine MY_name( a )
       return
       end
EOF
   # This is the ac_compile line used if LANG_FORTRAN77 is selected
   if test "X$ac_fcompile" = "X" ; then
       ac_fcompile='${F77-f77} -c $FFLAGS conftest.f 1>&AC_FD_CC'
   fi
   if AC_TRY_EVAL(ac_fcompile) && test -s conftest.o ; then
	mv conftest.o fconftestf.o
   else 
	echo "configure: failed program was:" >&AC_FD_CC
        cat conftest.f >&AC_FD_CC
   fi

   AC_LANG_SAVE
   AC_LANG_C   
   save_LIBS="$LIBS"
   LIBS="fconftestf.o $LIBS"
   AC_TRY_LINK(,my_name();,pac_cv_prog_f77_name_mangle="lower")
   if test  "X$pac_cv_prog_f77_name_mangle" = "X" ; then
     AC_TRY_LINK(,my_name_();,pac_cv_prog_f77_name_mangle="lower underscore")
   fi
   if test  "X$pac_cv_prog_f77_name_mangle" = "X" ; then
     AC_TRY_LINK(,MY_NAME();,pac_cv_prog_f77_name_mangle="upper")
   fi
   if test  "X$pac_cv_prog_f77_name_mangle" = "X" ; then
     AC_TRY_LINK(,my_name__();,
       pac_cv_prog_f77_name_mangle="lower doubleunderscore")
   fi
   if test  "X$pac_cv_prog_f77_name_mangle" = "X" ; then
     AC_TRY_LINK(,MY_name();,pac_cv_prog_f77_name_mangle="mixed")
   fi
   if test  "X$pac_cv_prog_f77_name_mangle" = "X" ; then
     AC_TRY_LINK(,MY_name_();,pac_cv_prog_f77_name_mangle="mixed underscore")
   fi
   LIBS="$save_LIBS"
   AC_LANG_RESTORE
   rm -f fconftest*
])
# Make the actual definition
pac_namecheck=`echo X$pac_cv_prog_f77_name_mangle | sed 's/ /-/g'`
ifelse([$1],,[
case $pac_namecheck in
    X) AC_MSG_WARN([Cannot determine Fortran naming scheme]) ;;
    Xlower) AC_DEFINE(F77_NAME_LOWER) ;;
    Xlower-underscore) AC_DEFINE(F77_NAME_LOWER_USCORE) ;;
    Xlower-doubleunderscore) AC_DEFINE(F77_NAME_LOWER_2USCORE) ;;
    Xupper) AC_DEFINE(F77_NAME_UPPER) ;;
    Xmixed) AC_DEFINE(F77_NAME_MIXED) ;;
    Xmixed-underscore) AC_DEFINE(F77_NAME_MIXED_USCORE) ;;
    *) AC_MSG_WARN([Unknown Fortran naming scheme]) ;;
esac
],[$1])
])
dnl
dnl/*D
dnl PAC_PROG_F77_CHECK_SIZEOF - Determine the size in bytes of a Fortran
dnl type
dnl
dnl Synopsis:
dnl PAC_PROG_F77_CHECK_SIZEOF(type,[cross-size])
dnl
dnl Output Effect:
dnl Sets SIZEOF_F77_uctype to the size if bytes of type.
dnl If type is unknown, the size is set to 0.
dnl If cross-compiling, the value cross-size is used (it may be a variable)
dnl For example 'PAC_PROG_F77_CHECK_SIZEOF(real)' defines
dnl 'SIZEOF_F77_REAL' to 4 on most systems.  The variable 
dnl 'pac_cv_sizeof_f77_<type>' (e.g., 'pac_cv_sizeof_f77_real') is also set to
dnl the size of the type. 
dnl If the corresponding variable is already set, that value is used.
dnl If the name has an '*' in it (e.g., 'integer*4'), the defined name 
dnl replaces that with an underscore (e.g., 'SIZEOF_F77_INTEGER_4').
dnl
dnl Notes:
dnl If the 'cross-size' argument is not given, 'autoconf' will issue an error
dnl message.  You can use '0' to specify undetermined.
dnl
dnlD*/
AC_DEFUN(PAC_PROG_F77_CHECK_SIZEOF,[
changequote(<<, >>)dnl
dnl The name to #define.
dnl If the arg value contains a variable, we need to update that
define(<<PAC_TYPE_NAME>>, translit(sizeof_f77_$1, [a-z *], [A-Z__]))dnl
dnl The cache variable name.
define(<<PAC_CV_NAME>>, translit(pac_cv_f77_sizeof_$1, [ *], [__]))dnl
changequote([, ])dnl
AC_CACHE_CHECK([for size of Fortran type $1],PAC_CV_NAME,[
AC_REQUIRE([PAC_PROG_F77_NAME_MANGLE])
/bin/rm -f conftest*
cat <<EOF > conftest.f
      subroutine isize( )
      $1 i(2)
      call cisize( i(1), i(2) )
      end
EOF
if test "X$ac_fcompile" = "X" ; then
    ac_fcompile='${F77-f77} -c $FFLAGS conftest.f 1>&AC_FD_CC'
fi
if AC_TRY_EVAL(ac_fcompile) && test -s conftest.o ; then
    mv conftest.o conftestf.o
    AC_LANG_SAVE
    AC_LANG_C
    save_LIBS="$LIBS"
    LIBS="conftestf.o $LIBS"
    AC_TRY_RUN([#include <stdio.h>
#ifdef F77_NAME_UPPER
#define cisize_ CISIZE
#define isize_ ISIZE
#elif defined(F77_NAME_LOWER) || defined(F77_NAME_MIXED)
#define cisize_ cisize
#define isize_ isize
#endif
static int isize_val;
void cisize_(char *i1p, char *i2p)
{ 
   isize_val = (int)(i2p - i1p);
}
main()
{
    FILE *f = fopen("conftestval", "w");
    if (!f) exit(1);
    isize_();
    fprintf(f,"%d\n", isize_val );
    exit(0);
}], eval PAC_CV_NAME=`cat conftestval`,eval PAC_CV_NAME=0,
ifelse([$2],,,eval PAC_CV_NAME=$2))
    LIBS="$save_LIBS"
    AC_LANG_RESTORE
else 
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.f >&AC_FD_CC
    ifelse([$2],,eval PAC_CV_NAME=0,eval PAC_CV_NAME=$2)
fi
])
AC_DEFINE_UNQUOTED(PAC_TYPE_NAME,$PAC_CV_NAME)
undefine([PAC_TYPE_NAME])
undefine([PAC_CV_NAME])
])
dnl
dnl/*D
dnl PAC_PROG_F77_EXCLAIM_COMMENTS
dnl
dnl Synopsis:
dnl PAC_PROG_F77_EXCLAIM_COMMENTS([action-if-true],[action-if-false])
dnl
dnl Notes:
dnl Check whether '!' may be used to begin comments in Fortran.
dnl
dnl This macro requires a version of autoconf `after` 2.13; the 'acgeneral.m4'
dnl file contains an error in the handling of Fortran programs in 
dnl 'AC_TRY_COMPILE' (fixed in our local version).
dnl
dnlD*/
AC_DEFUN(PAC_PROG_F77_EXCLAIM_COMMENTS,[
AC_CACHE_CHECK([whether Fortran accepts ! for comments],
pac_cv_prog_f77_exclaim_comments,[
AC_LANG_SAVE
AC_LANG_FORTRAN77
AC_TRY_COMPILE(,[
!      This is a comment
],pac_cv_prog_f77_exclaim_comments="yes",
pac_cv_prog_f77_exclaim_comments="no")
AC_LANG_RESTORE
])
if test "$pac_cv_prog_f77_exclaim_comments" = "yes" ; then
    ifelse([$1],,:,$1)
else
    ifelse([$2],,:,$2)
fi
])dnl
dnl
dnl/*D
dnl PAC_F77_CHECK_COMPILER_OPTION - Check that a compiler option is accepted
dnl without warning messages
dnl
dnl Synopsis:
dnl PAC_F77_CHECK_COMPILER_OPTION(optionname,action-if-ok,action-if-fail)
dnl
dnl Output Effects:
dnl
dnl If no actions are specified, a working value is added to 'FOPTIONS'
dnl
dnl Notes:
dnl This is now careful to check that the output is different, since 
dnl some compilers are noisy.
dnl 
dnl We are extra careful to prototype the functions in case compiler options
dnl that complain about poor code are in effect.
dnl
dnl Because this is a long script, we have ensured that you can pass a 
dnl variable containing the option name as the first argument.
dnlD*/
AC_DEFUN(PAC_F77_CHECK_COMPILER_OPTION,[
AC_MSG_CHECKING([that Fortran 77 compiler accepts option $1])
ac_result="no"
save_FFLAGS="$FFLAGS"
FFLAGS="$1 $FFLAGS"
rm -f conftest.out
cat >conftest2.f <<EOF
        subroutine try()
        end
EOF
cat >conftest.f <<EOF
        program main
        end
EOF
dnl It is important to use the AC_TRY_EVAL in case F77 is not a single word
dnl but is something like "f77 -64" (where the switch has changed the
dnl compiler)
ac_fscompilelink='${F77-f77} $save_FFLAGS -o conftest conftest.f >conftest.bas 2>&1'
ac_fscompilelink2='${F77-f77} $FFLAGS -o conftest conftest.f >conftest.out 2>&1'
if AC_TRY_EVAL(ac_fscompilelink) && test -x conftest ; then
   if AC_TRY_EVAL(ac_fscompilelink2) && test -x conftest ; then
      if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
         AC_MSG_RESULT(yes)
         AC_MSG_CHECKING([that routines compiled with $1 can be linked with ones compiled  without $1])       
         /bin/rm -f conftest2.out
         /bin/rm -f conftest.bas
	 ac_fscompile3='${F77-f77} -c $save_FFLAGS conftest2.f >conftest2.out 2>&1'
	 ac_fscompilelink4='${F77-f77} $FFLAGS -o conftest conftest2.o conftest.f >conftest.bas 2>&1'
         if AC_TRY_EVAL(ac_fscompile3) && test -s conftest2.o ; then
            if AC_TRY_EVAL(ac_fscompilelink4) && test -x conftest ; then
               if diff -b conftest.out conftest.bas >/dev/null 2>&1 ; then
	          ac_result="yes"
	       else 
		  echo "configure: Compiler output differed in two cases" >&AC_FD_CC
                  diff -b conftest.out conftest.bas >&AC_FD_CC
	       fi
	    else
	       echo "configure: failed program was:" >&AC_FD_CC
	       cat conftest.f >&AC_FD_CC
	    fi
	  else
	    echo "configure: failed program was:" >&AC_FD_CC
	    cat conftest2.f >&AC_FD_CC
	  fi
      else
	# diff
        echo "configure: Compiler output differed in two cases" >&AC_FD_CC
        diff -b conftest.out conftest.bas >&AC_FD_CC
      fi
   else
      # try_eval(fscompilelink2)
      echo "configure: failed program was:" >&AC_FD_CC
      cat conftest.f >&AC_FD_CC
   fi
   if test "$ac_result" != "yes" -a -s conftest.out ; then
	cat conftest.out >&AC_FD_CC
   fi
else
    # Could not compile without the option!
    echo "configure: Could not compile program" >&AC_FD_CC
    cat conftest.f >&AC_FD_CC
    cat conftest.bas >&AC_FD_CC
fi
if test "$ac_result" = "yes" ; then
     AC_MSG_RESULT(yes)	  
     ifelse([$2],,FOPTIONS="$FOPTIONS $1",$2)
else
     AC_MSG_RESULT(no)
     $3
fi
FFLAGS="$save_FFLAGS"
rm -f conftest*
])
dnl
dnl/*D
dnl PAC_PROG_F77_CMDARGS - Determine how to access the command line from
dnl Fortran 77
dnl
dnl Output Effects:
dnl  The following variables are set:
dnl.vb
dnl    F77_GETARG         - Statement to get an argument i into string s
dnl    F77_IARGC          - Routine to return the number of arguments
dnl    FXX_MODULE         - Module command when using Fortran 90 compiler
dnl    F77_GETARGDECL     - Declaration of routine used for F77_GETARG
dnl    F77_GETARG_FFLAGS  - Flags needed when compiling/linking
dnl    F77_GETARG_LDFLAGS - Flags needed when linking
dnl.ve
dnl If 'F77_GETARG' has a value, then that value and the values for these
dnl other symbols will be used instead.  If no approach is found, all of these
dnl variables will have empty values.
dnl If no other approach works and a file 'f77argdef' is in the directory, 
dnl that file will be sourced for the values of the above four variables.
dnl
dnl In most cases, you should add F77_GETARG_FFLAGS to the FFLAGS variable
dnl and F77_GETARG_LDFLAGS to the LDFLAGS variable, to ensure that tests are
dnl performed on the compiler version that will be used.
dnl
dnl 'AC_SUBST' is called for all six variables.
dnl
dnl f77argdef
dnlD*/
AC_DEFUN(PAC_PROG_F77_CMDARGS,[
found_cached="yes"
AC_MSG_CHECKING([for routines to access the command line from Fortran 77])
AC_CACHE_VAL(pac_cv_prog_f77_cmdarg,
[
    AC_MSG_RESULT([searching...])
    found_cached="no"
    # Grumph.  Here are a bunch of different approaches
    # We have several axes the check:
    # Library to link with (none, -lU77 (HPUX), -lg2c (LINUX f77))
    # The first line is a dummy
    # (we experimented with using a <space>, but this caused other 
    # problems because we need <space> in the IFS)
trial_LIBS="0
-lU77
-lg2c"
    # Discard libs that are not availble:
    save_IFS="$IFS"
    # Make sure that IFS includes a space, or the tests that run programs
    # may fail
    IFS=" 
"
    save_trial_LIBS="$trial_LIBS"
    trial_LIBS=""
    cat > conftest.f <<EOF
        program main
        end
EOF
    ac_fcompilelink_test='${F77-f77} -o conftest $FFLAGS conftest.f $libs $LIBS 1>&AC_FD_CC'
    for libs in $save_trial_LIBS ; do
	if test "$libs" = "0" ; then
	    lib_ok="yes"
        else
	    AC_MSG_CHECKING([whether Fortran 77 links with $libs])
	    if AC_TRY_EVAL(ac_fcompilelink_test) && test -x conftest ; then
		AC_MSG_RESULT([yes])
	        lib_ok="yes"
	    else
		AC_MSG_RESULT([no])
	        lib_ok="no"
	    fi
	fi
	if test "$lib_ok" = "yes" ; then
	    trial_LIBS="$trial_LIBS
$libs"
        fi
    done

    # Options to use when compiling and linking
    # +U77 is needed by HP Fortran to access getarg etc.
    # The -N109 was used for getarg before we realized that GETARG
    # was necessary with the (non standard conforming) Absoft compiler
    # (Fortran is monocase; Absoft uses mixedcase by default)
    # The -f is used by Absoft and is the compiler switch that folds 
    # symbolic names to lower case.  Without this option, the compiler
    # considers upper- and lower-case letters to be unique.
    # The -YEXT_NAMES=LCS will cause external names to be output as lower
    # case letter for Absoft F90 compilers (default is upper case)
    # The first line is "<space><newline>, the space is important
    # To make the Absoft f77 and f90 work together, we need to prefer the
    # upper case versions of the arguments.  They also require libU77.
    # -YCFRL=1 causes Absoft f90 to work with g77 and similar (f2c-based) 
    # Fortran compilers
    #
trial_FLAGS="000
-N109
-f
-YEXT_NAMES=UCS
-YEXT_NAMES=LCS
-YCFRL=1
+U77"
    # Discard options that are not available:
    save_IFS="$IFS"
    IFS=" 
"
    save_trial_FLAGS="$trial_FLAGS"
    trial_FLAGS=""
    for flag in $save_trial_FLAGS ; do
	if test "$flag" = " " -o "$flag" = "000" ; then
	    opt_ok="yes"
        else
            PAC_F77_CHECK_COMPILER_OPTION($flag,opt_ok=yes,opt_ok=no)
        fi
	if test "$opt_ok" = "yes" ; then
	    if test "$flag" = " " -o "$flag" = "000" ; then 
		fflag="" 
	    else 
		fflag="$flag" 
	    fi
	    # discard options that don't allow mixed-case name matching
	    cat > conftest.f <<EOF
        program main
        call aB()
        end
        subroutine Ab()
        end
EOF
	    if test -n "$fflag" ; then flagval="with $fflag" ; else flagval="" ; fi
	    AC_MSG_CHECKING([that Fortran 77 routine names are case-insensitive $flagval])
	    dnl we can use double quotes here because all is already
            dnl evaluated
            ac_fcompilelink_test="${F77-f77} -o conftest $fflag $FFLAGS
conftest.f $LIBS 1>&AC_FD_CC"
	    if AC_TRY_EVAL(ac_fcompilelink_test) && test -x conftest ; then
	        AC_MSG_RESULT(yes)
	    else
	        AC_MSG_RESULT(no)
	        opt_ok="no"
            fi
        fi
        if test "$opt_ok" = "yes" ; then
	    trial_FLAGS="$trial_FLAGS
$flag"
        fi
    done
    IFS="$save_IFS"
    # Name of routines.  Since these are in groups, we use a case statement
    # and loop until the end (accomplished by reaching the end of the
    # case statement
    # For one version of Nag F90, the names are 
    # call f90_unix_MP_getarg(i,s) and f90_unix_MP_iargc().
    trial=0
    while test -z "$pac_cv_prog_f77_cmdarg" ; do
        case $trial in 
	0) # User-specified values, if any
	   if test -z "$F77_GETARG" -o -z "$F77_IARGC" ; then 
	       trial=`expr $trial + 1`
	       continue 
           fi
           MSG="Using environment values of F77_GETARG etc."
	   ;;
	1) # Standard practice, uppercase (some compilers are case-sensitive)
	   FXX_MODULE=""
	   F77_GETARGDECL="external GETARG"
	   F77_GETARG="call GETARG(i,s)"
	   F77_IARGC="IARGC()"
	   MSG="GETARG and IARGC"
	   ;;
	2) # Standard practice, lowercase
	   FXX_MODULE=""
           F77_GETARGDECL="external getarg"
	   F77_GETARG="call getarg(i,s)"
	   F77_IARGC="iargc()"
	   MSG="getarg and iargc"
	   ;;
	3) # Posix alternative
	   FXX_MODULE=""
	   F77_GETARGDECL="external pxfgetarg"
	   F77_GETARG="call pxfgetarg(i,s,l,ier)"
	   F77_IARGC="ipxfiargc()"
	   MSG="pxfgetarg and ipxfiargc"
	   ;;
	4) # Nag f90_unix_env module
	   FXX_MODULE="        use f90_unix_env"
	   F77_GETARGDECL=""
	   F77_GETARG="call getarg(i,s)"
	   F77_IARGC="iargc()"
	   MSG="f90_unix_env module"
	   ;;
        5) # Nag f90_unix module
	   FXX_MODULE="        use f90_unix"
	   F77_GETARGDECL=""
	   F77_GETARG="call getarg(i,s)"
	   F77_IARGC="iargc()"
	   MSG="f90_unix module"
	   ;;
	6) # user spec in a file
	   if test -s f77argdef ; then
		. ./f77argdef
	       MSG="Using definitions in the file f77argdef"
	   else
	        trial=`expr $trial + 1`
		continue
	   fi
	   ;;
        *) # exit from while loop
	   FXX_MODULE=""
	   F77_GETARGDECL=""
	   F77_GETARG=""
	   F77_IARGC=""
           break
	   ;;
	esac
	# Create the program
        cat > conftest.f <<EOF
        program main
$FXX_MODULE
        integer i
        character*20 s

        $F77_GETARGDECL
        $F77_GETARG
        i=$F77_IARGC
        end
EOF
    #
    # Now, try to find some way to compile and link that program, looping 
    # over the possibilities of options and libraries
        save_IFS="$IFS"
        IFS=" 
"
        for libs in $trial_LIBS ; do
            if test -n "$pac_cv_prog_f77_cmdarg" ; then break ; fi
	    if test "$libs" = " " -o "$libs" = "0" ; then libs="" ; fi
            for flags in $trial_FLAGS ; do
	        if test "$flags" = " " -o "$flags" = "000"; then flags="" ; fi
                AC_MSG_CHECKING([if ${F77-f77} $flags $libs works with $MSG])
		IFS="$save_IFS"
		dnl We need this here because we've fiddled with IFS
	        ac_fcompilelink_test="${F77-f77} -o conftest $FFLAGS $flags conftest.f $libs $LIBS 1>&AC_FD_CC"
                if AC_TRY_EVAL(ac_fcompilelink_test) && test -x conftest ; then
	            AC_MSG_RESULT([yes])
		    pac_cv_prog_f77_cmdarg="$MSG"
		    pac_cv_prog_f77_cmdarg_fflags="$flags"
		    pac_cv_prog_f77_cmdarg_ldflags="$libs"
		    break
	        else
                    AC_MSG_RESULT([no])
		    echo "configure: failed program was:" >&AC_FD_CC
                    cat conftest.f >&AC_FD_CC
	        fi
		IFS=" 
"
            done
        done
        IFS="$save_IFS"   
	rm -f conftest.*
        trial=`expr $trial + 1`   
    done
pac_cv_F77_GETARGDECL="$F77_GETARGDECL"
pac_cv_F77_IARGC="$F77_IARGC"
pac_cv_F77_GETARG="$F77_GETARG"
pac_cv_FXX_MODULE="$FXX_MODULE"
])
if test "$found_cached" = "yes" ; then 
    AC_MSG_RESULT([$pac_cv_prog_f77_cmdarg])
elif test -z "$pac_cv_F77_IARGC" ; then
    AC_MSG_WARN([Could not find a way to access the command line from Fortran 77])
fi
# Set the variable values based on pac_cv_prog_xxx
F77_GETARGDECL="$pac_cv_F77_GETARGDECL"
F77_IARGC="$pac_cv_F77_IARGC"
F77_GETARG="$pac_cv_F77_GETARG"
FXX_MODULE="$pac_cv_FXX_MODULE"
F77_GETARG_FFLAGS="$pac_cv_prog_f77_cmdarg_fflags"
F77_GETARG_LDFLAGS="$pac_cv_prog_f77_cmdarg_ldflags"
AC_SUBST(F77_GETARGDECL)
AC_SUBST(F77_IARGC)
AC_SUBST(F77_GETARG)
AC_SUBST(FXX_MODULE)
AC_SUBST(F77_GETARG_FFLAGS)
AC_SUBST(F77_GETARG_LDFLAGS)
])
dnl/*D
dnl PAC_PROG_F77_LIBRARY_DIR_FLAG - Determine the flag used to indicate
dnl the directories to find libraries in
dnl
dnl Notes:
dnl Many compilers accept '-Ldir' just like most C compilers.  
dnl Unfortunately, some (such as some HPUX Fortran compilers) do not, 
dnl and require instead either '-Wl,-L,dir' or something else.  This
dnl command attempts to determine what is accepted.  The flag is 
dnl placed into 'F77_LIBDIR_LEADER'.
dnl
dnlD*/
AC_DEFUN(PAC_PROG_F77_LIBRARY_DIR_FLAG,[
if test "X$F77_LIBDIR_LEADER" = "X" ; then
AC_CACHE_CHECK([for Fortran 77 flag for library directories],
pac_cv_prog_f77_library_dir_flag,
[
    rm -f conftest.*
    cat > conftest.f <<EOF
        program main
        end
EOF
    ac_fcompileldtest='${F77-f77} -o conftest $FFLAGS ${ldir}. conftest.f 1>&AC_FD_CC'
    for ldir in "-L" "-Wl,-L," ; do
        if AC_TRY_EVAL(ac_fcompileldtest) && test -s conftest ; then
	    pac_cv_prog_f77_library_dir_flag="$ldir"
	    break
       fi
    done
    rm -f conftest*
])
    AC_SUBST(F77_LIBDIR_LEADER)
    if test "X$pac_cv_prog_f77_library_dir_flag" != "X" ; then
        F77_LIBDIR_LEADER="$pac_cv_prog_f77_library_dir_flag"
    fi
fi
])
dnl/*D 
dnl PAC_PROG_F77_HAS_INCDIR - Check whether Fortran accepts -Idir flag
dnl
dnl Syntax:
dnl   PAC_PROG_F77_HAS_INCDIR(directory,action-if-true,action-if-false)
dnl
dnl Output Effect:
dnl  Sets 'F77_INCDIR' to the flag used to choose the directory.  
dnl
dnl Notes:
dnl This refers to the handling of the common Fortran include extension,
dnl not to the use of '#include' with the C preprocessor.
dnl If directory does not exist, it will be created.  In that case, the 
dnl directory should be a direct descendant of the current directory.
dnl
dnlD*/ 
AC_DEFUN(PAC_PROG_F77_HAS_INCDIR,[
checkdir=$1
AC_CACHE_CHECK([for include directory flag for Fortran],
pac_cv_prog_f77_has_incdir,[
if test ! -d $checkdir ; then mkdir $checkdir ; fi
cat >$checkdir/conftestf.h <<EOF
       call sub()
EOF
cat >conftest.f <<EOF
       program main
       include 'conftestf.h'
       end
EOF

ac_fcompiletest='${F77-f77} -c $FFLAGS ${idir}$checkdir conftest.f 1>&AC_FD_CC'
pac_cv_prog_f77_has_incdir="none"
# SGI wants -Wf,-I
for idir in "-I" "-Wf,-I" ; do
    if AC_TRY_EVAL(ac_fcompiletest) && test -s conftest.o ; then
        pac_cv_prog_f77_has_incdir="$idir"
	break
    fi
done
rm -f conftest*
rm -f $checkdir/conftestf.h
])
AC_SUBST(F77_INCDIR)
if test "X$pac_cv_prog_f77_has_incdir" != "Xnone" ; then
    F77_INCDIR="$pac_cv_prog_f77_has_incdir"
fi
])
dnl
dnl/*D
dnl PAC_PROG_F77_ALLOWS_UNUSED_EXTERNALS - Check whether the Fortran compiler
dnl allows unused and undefined functions to be listed in an external 
dnl statement
dnl
dnl Syntax:
dnl   PAC_PROG_F77_ALLOWS_UNUSED_EXTERNALS(action-if-true,action-if-false)
dnl
dnlD*/
AC_DEFUN(PAC_PROG_F77_ALLOWS_UNUSED_EXTERNALS,[
AC_CACHE_CHECK([whether Fortran allows unused externals],
pac_cv_prog_f77_allows_unused_externals,[
AC_LANG_SAVE
AC_LANG_FORTRAN77
dnl We can't use TRY_LINK, because it wants a routine name, not a 
dnl declaration.  The following is the body of TRY_LINK, slightly modified.
cat > conftest.$ac_ext <<EOF
       program main
       external bar
       end
EOF
if AC_TRY_EVAL(ac_link) && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  pac_cv_prog_f77_allows_unused_externals="yes"
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  rm -rf conftest*
  pac_cv_prog_f77_allows_unused_externals="no"
  $4
fi
rm -f conftest*
#
AC_LANG_RESTORE
])
if test "X$pac_cv_prog_f77_allows_unused_externals" = "Xyes" ; then
   ifelse([$1],,:,[$1])
else
   ifelse([$2],,:,[$2])
fi
])
dnl/*D 
dnl PAC_PROG_F77_HAS_POINTER - Determine if Fortran allows pointer type
dnl
dnl Synopsis:
dnl   PAC_PROG_F77_HAS_POINTER(action-if-true,action-if-false)
dnlD*/
AC_DEFUN(PAC_PROG_F77_HAS_POINTER,[
AC_CACHE_CHECK([whether Fortran has pointer declaration],
pac_cv_prog_f77_has_pointer,[
AC_LANG_SAVE
AC_LANG_FORTRAN77
AC_TRY_COMPILE(,[
        integer M
        pointer (MPTR,M)
        data MPTR/0/
],pac_cv_prog_f77_has_pointer="yes",pac_cv_prog_f77_has_pointer="no")
AC_LANG_RESTORE
])
if test "$pac_cv_prog_f77_has_pointer" = "yes" ; then
    ifelse([$1],,:,[$1])
else
    ifelse([$2],,:,[$2])
fi
])
dnl
dnl pac_prog_f77_run_proc_from_c( c main program, fortran routine, 
dnl                               action-if-works, action-if-fails, 
dnl                               cross-action )
dnl Fortran routine MUST be named ftest unless you include code
dnl to select the appropriate Fortran name.
dnl 
AC_DEFUN(PAC_PROG_F77_RUN_PROC_FROM_C,[
/bin/rm -f conftest*
cat <<EOF > conftest.f
$2
EOF
dnl
if test "X$ac_fcompile" = "X" ; then
    ac_fcompile='${F77-f77} -c $FFLAGS conftest.f 1>&AC_FD_CC'
fi
if AC_TRY_EVAL(ac_fcompile) && test -s conftest.o ; then
    mv conftest.o conftestf.o
    AC_LANG_SAVE
    AC_LANG_C
    save_LIBS="$LIBS"
    LIBS="conftestf.o $LIBS"
    AC_TRY_RUN([#include <stdio.h>
#ifdef F77_NAME_UPPER
#define ftest_ FTEST
#elif defined(F77_NAME_LOWER) || defined(F77_NAME_MIXED)
#define ftest_ ftest
#endif
$1
], [$3], [$4], [$5] )
    LIBS="$save_LIBS"
    AC_LANG_RESTORE
else 
    echo "configure: failed program was:" >&AC_FD_CC
    cat conftest.f >&AC_FD_CC
fi
rm -f conftest*
])
