dnl
dnl Define the test to look for wish (the tcl/tk windowing shell)
dnl This is under development and is derived from the FINDX command
dnl FIND_WISH looks in the path and in places that we've found wish.
dnl It sets "wishloc" to the location that it found, or to the
dnl empty string if it can't find wish.
dnl Note that we need tk version 3.3 or later, so we don't check for the 
dnl earlier versions
dnl
dnl Some systems are now (probably wisely, given the number of 
dnl incompatibilities) using names like "wish4.2" or "wish-3.6" and the like.
define(PAC_FIND_WISH,[wishloc=""
AC_MSG_CHECKING([for wish])
# Look for wish in the path
IFS="${IFS= 	}"; saveifs="$IFS"; IFS="${IFS}:"
for dir in $PATH ; do 
    if test -x $dir/wish ; then
	wishloc=$dir/wish
        break
    elif test -x $dir/tcl7.3-tk3.6/bin/wish ; then
	wishloc=$dir/tcl7.3-tk3.6/bin/wish
	break
    elif test -x $dir/tcl7.4-tk4.0/bin/wish ; then
        wishloc=$dir/tcl7.4-tk4.0/bin/wish
	break
    else
	for file in $dir/wish3.? $dir/wish-3.? $dir/wish4.? $dir/wish* ; do
	    if test -x $file ; then
		wishloc=$file
		break
	    fi
	if test -n "$wishloc" ; then break ; fi
	done
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
    /usr/sgitcl \
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
dnl
dnl We can use wish to find tcl and tk libraries with
dnl puts stdout $tk_library
dnl tclsh can be used with 
dnl puts stdout $tcl_library
dnl
dnl
define(PAC_FIND_TCL,[
# Look for Tcl
if test -z "$TCL_DIR" ; then
PAC_PROGRAM_CHECK(TCLSH,tclsh,1,,tclshloc)
AC_MSG_CHECKING([for Tcl])
# See if tclsh is in the path
# If there is a tclsh, it MAY provide tk.
if test -n "$tclshloc" ; then
    cat >conftest <<EOF
puts stdout [\$]tcl_library
EOF
    tcllibloc=`$tclshloc conftest 2>/dev/null`
    # The tcllibloc is the directory containing the .tcl files.  
    # The .a files may be one directory up
    if test -n "$tcllibloc" ; then
        tcllibloc=`dirname $tcllibloc`
        # and the lib directory one above that
        tcllibs="$tcllibloc `dirname $tcllibloc`"
    fi
    /bin/rm -f conftest   
fi
for dir in $tcllibs \
    /usr \
    /usr/local \
    /usr/local/tcl7.5 \
    /usr/local/tcl7.3 \
    /usr/local/tcl7.3-tk3.6 \
    /usr/local/tcl7.0 \
    /usr/local/tcl7.0-tk3.3 \
    /usr/local/tcl7.* \
    /usr/contrib \
    /usr/contrib/tk3.6 \
    /usr/contrib/tcl7.3-tk3.6 \
    /usr/contrib/tk3.3 \
    /usr/contrib/tcl7.0-tk3.3 \
    $HOME/tcl \
    $HOME/tcl7.3 \
    $HOME/tcl7.5 \
    /opt/Tcl \
    /opt/local \
    /opt/local/tcl7.5 \
    /opt/local/tcl7.* \
    /usr/bin \
    /Tools/tcl \
    /usr/sgitcl \
    /local/encap/tcl-7.1 ; do
    if test -r $dir/include/tcl.h ; then 
	# Check for correct version
	changequote(,)
	tclversion=`grep 'TCL_MAJOR_VERSION' $dir/include/tcl.h | \
		sed -e 's/^.*TCL_MAJOR_VERSION[^0-9]*\([0-9]*\).*$/\1/'`
	changequote([,])
	if test "$tclversion" != "7" ; then
	    # Skip if it is the wrong version
	    continue
	fi
        if test -r $dir/lib/libtcl.a -o -r $dir/lib/libtcl.so ; then
 	    TCL_DIR=$dir
	    break
        fi
	for file in $dir/lib/libtcl*.a ; do
	    if test -r $file ; then 
                TCL_DIR_W="$TCL_DIR_W $file"
	    fi
	done
    fi
done
fi
if test -n "$TCL_DIR" ; then 
  AC_MSG_RESULT(found $TCL_DIR/include/tcl.h and $TCL_DIR/lib/libtcl)
else
  if test -n "$TCL_DIR_W" ; then
    AC_MSG_RESULT(found $TCL_DIR_W but need libtcl.a)
  else
    AC_MSG_RESULT(no)
  fi
fi
# Look for Tk (look in tcl dir if the code is nowhere else)
if test -z "$TK_DIR" ; then
AC_MSG_CHECKING([for Tk])
if test -n "$wishloc" ; then
    # Originally, we tried to run wish and get the tkversion from it
    # unfortunately, this sometimes hung, probably waiting to get a display
#    cat >conftest <<EOF
#puts stdout [\$]tk_library
#exit
#EOF
#    tklibloc=`$wishloc -file conftest 2>/dev/null`
    tklibloc=`strings $wishloc | grep 'lib/tk'`
    # The tklibloc is the directory containing the .tclk files.  
    # The .a files may be one directory up
    # There may be multiple lines in tklibloc now.  Make sure that we only
    # test actual directories
    if test -n "$tklibloc" ; then
	for tkdirname in $tklibloc ; do
    	    if test -d $tkdirname ; then
                tkdirname=`dirname $tkdirname`
                # and the lib directory one above that
                tklibs="$tkdirname `dirname $tkdirname`"
	    fi
	done
    fi
    /bin/rm -f conftest   
fi
for dir in $tklibs \
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
    /usr/bin \
    /Tools/tk \
    /usr/sgitcl \
    /local/encap/tk-3.4 $TCL_DIR ; do
    if test -r $dir/include/tk.h ; then 
	# Check for correct version
	changequote(,)
	tkversion=`grep 'TK_MAJOR_VERSION' $dir/include/tk.h | \
		sed -e 's/^.*TK_MAJOR_VERSION[^0-9]*\([0-9]*\).*$/\1/'`
	changequote([,])
	if test "$tkversion" != "3" ; then
	    # Skip if it is the wrong version
	    continue
	fi
        if test -r $dir/lib/libtk.a -o -r $dir/lib/libtk.so ; then
	    TK_DIR=$dir
	    break
	fi
	for file in $dir/lib/libtk*.a ; do
	    if test -r $file ; then 
                TK_DIR_W="$TK_DIR_W $file"
	    fi
	done
    fi
done
fi
if test -n "$TK_DIR" ; then 
  AC_MSG_RESULT(found $TK_DIR/include/tk.h and $TK_DIR/lib/libtk)
else
  if test -n "$TK_DIR_W" ; then
    AC_MSG_RESULT(found $TK_DIR_W but need libtk.a (and version 3.6) )
  else
    AC_MSG_RESULT(no)
  fi
  AC_MSG_RESULT(no)
fi
])dnl
dnl
dnl --------------------------------------------------------
dnl Test for the VERSION of tk.  There are major changes between 3.6 and 4.0
dnl (in particular, the type Tk_ColorModel disappeared
dnl  Put result into TK_VERSION (as, e.g., 3.6 or 4.0).  Should test version
dnl as STRING, since we don't control the changes between versions, and 
dnl only versions that we know should be tested.
dnl Note that this may be important ONLY if you include tk.h .
dnl TKINCDIR may also be defined if the include files are not where the
dnl architecture-dependant library files are
dnl
dnl TK_LIB and XINCLUDES must be defined (and no_x must NOT be true)
dnl
define(PAC_TK_VERSION,[
AC_MSG_CHECKING(for version of TK)
/bin/rm -f conftestval
#
# Some systems have a separate tcl dir; since we need both tcl and tk
# we include both directories
# Tk is going to load X11; if no X11, skip this step
if test -z "$no_x" -a -n "$TK_DIR" -a -n "$TCL_DIR" ; then
  CFLAGSsave="$CFLAGS"
  CFLAGS="$CFLAGS -I$TK_DIR/include -I$TCL_DIR/include $XINCLUDES"
  if test -n "$TKINCDIR" ; then
      CFLAGS="$CFLAGS -I$TKINCDIR/include"
  fi
  PAC_TEST_PROGRAM([#include "tk.h"
#include <stdio.h>
main() { FILE *fp = fopen( "conftestval", "w" ); 
fprintf( fp, "%d.%d", TK_MAJOR_VERSION, TK_MINOR_VERSION );
return 0; }],
  TK_VERSION=`cat conftestval`,TK_VERSION="unavailable")
  CFLAGS="$CFLAGSsave"
elif test -n "$wishloc" ; then
  # It is possible to use a wish program with
  # set tk_version [ string range $tk_patchLevel 0 2 ]
  # puts stdout $tk_version
  TK_VERSION="unavailable"
else
  TK_VERSION="unavailable"
fi
AC_MSG_RESULT($TK_VERSION)
])dnl
