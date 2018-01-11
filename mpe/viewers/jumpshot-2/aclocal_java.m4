dnl
dnl PAC_JAVA_TRY_COMPILE - test the compilation of java program
dnl
dnl PAC_JAVA_TRY_COMPILE( JC, JFLAGS, IMPORTS, PROGRAM-BODY,
dnl                       [ ACTION-IF-WORKING [ , ACTION-IF-NOT-WORKING ] ] )
dnl JC            - java compiler
dnl JFLAGS        - java compiler flags, like options: -d and -classpath, ...
dnl IMPORTS       - java import statements, besides top level "class" statement
dnl PROGRAM_BODY  - java program body
dnl
AC_DEFUN(PAC_JAVA_TRY_COMPILE,[
dnl - set internal JC and JFLAGS variables
pac_JC="$1"
pac_JFLAGS="$2"
dnl - set the testing java program
changequote(,)
    rm -f conftest*
    cat > conftest.java <<EOF
$3
class conftest {
$4
}
EOF
changequote([, ])
dnl
    pac_jcompile='${pac_JC} ${pac_JFLAGS} conftest.java 1>&AC_FD_CC'
    if AC_TRY_EVAL(pac_jcompile) && test -s conftest.class ; then
        ifelse([$5],,:,[rm -rf conftest* ; $5])
    else
        ifelse([$6],,:,[rm -rf conftest* ; $6])
    fi
])dnl
dnl
dnl PAC_JAVA_TRY_RMIC - test the rmic program
dnl
dnl PAC_JAVA_TRY_RMIC( RMIC, JRFLAGS, JC, JFLAGS
dnl                    [ ACTION-IF-WORKING [ , ACTION-IF-NOT-WORKING ] ] )
dnl RMIC          - rmic compiler
dnl JRFLAGS       - rmic compiler flags, like options: -d and -classpath, ...
dnl JC            - java compiler
dnl JFLAGS        - java compiler flags, like options: -d and -classpath, ...
dnl
AC_DEFUN(PAC_JAVA_TRY_RMIC,[
dnl - set internal RMIC and JRFLAGS variables
pac_RMIC="$1"
pac_JRFLAGS="$2"
dnl - set internal JC and JFLAGS variables
pac_JC="$3"
pac_JFLAGS="$4"
dnl - set the testing java program
changequote(,)
    rm -f conftest*
dnl
    cat > conftest_remote.java <<EOF
import java.rmi.*;
public interface conftest_remote extends Remote
{
    public void remote_interface() throws RemoteException;
}
EOF
dnl
    cat > conftest_rmic.java <<EOF
import java.rmi.*;
import java.rmi.server.*;
public class conftest_rmic extends UnicastRemoteObject
                           implements conftest_remote
{
    public conftest_rmic() throws RemoteException
    { super(); }
    public void remote_interface() throws RemoteException
    {}
}
EOF
changequote([, ])
dnl
    pac_jcompile='${pac_JC} ${pac_JFLAGS} conftest_remote.java conftest_rmic.java 1>&AC_FD_CC'
    if AC_TRY_EVAL(pac_jcompile) && test -s conftest_rmic.class ; then
        pac_jrmic='${pac_RMIC} ${pac_JRFLAGS} conftest_rmic 1>&AC_FD_CC'
        if AC_TRY_EVAL(pac_jrmic) && test -s conftest_rmic_Stub.class ; then
            ifelse([$5],,:,[rm -rf conftest* ; $5])
        else
            ifelse([$6],,:,[rm -rf conftest* ; $6])
        fi
    else
        ifelse([$6],,:,[rm -rf conftest* ; $6])
    fi
])dnl
dnl
dnl PAC_FIND_JAVA(varname) - locate Java in standard location
dnl
dnl where varname is the returned variable name of Java home directory
dnl
AC_DEFUN([PAC_FIND_JAVA],[
$1=""
# Determine the system type
subdir=""
AC_CANONICAL_HOST
case "$host" in
    mips-sgi-irix*)
        if test -d "/software/irix" ; then
            subdir="irix"
        elif test -d "/software/irix-6" ; then
            subdir="irix-6"
        fi
        ;;
    *linux*)
        if test -d "/software/linux" ; then
            subdir="linux"
        fi
        ;;
   *solaris*)
        if test -d "/software/solaris" ; then
            subdir="solaris"
        elif test -d "/software/solaris-2" ; then
            subdir="solaris-2"
        fi
        ;;
    *sun4*)
        if test -d "/software/sun4" ; then
            subdir="sun4"
        fi
        ;;
   *aix*)
        if test -d "/software/aix-4" ; then
            subdir="aix-4"
        fi
        ;;
   *rs6000*)
        if test -d "/software/aix-4" ; then
            subdir="aix-4"
        fi
        ;;
   *freebsd*)
        if test -d "/software/freebsd" ; then
            subdir="freebsd"
   	    fi
esac
#
if test -z "$subdir" ; then
    if test -d "/software/common" ; then
       subdir="common"
    fi
fi
#
AC_MSG_CHECKING(for Java in known locations)
for dir in \
    /usr \
    /usr/jdk* \
    /usr/java* \
    /usr/local \
    /usr/local/jdk* \
    /usr/local/java* \
    /usr/contrib \
    /usr/contrib/jdk* \
    /usr/contrib/java* \
    $HOME/jdk* \
    $HOME/java* \
    /opt/jdk* \
    /opt/java* \
    /opt/local \
    /opt/local/jdk* \
    /opt/local/java* \
    /usr/bin \
    /Tools/jdk* \
    /software/$subdir/apps/packages/jdk* \
    /software/$subdir/apps/packages/java* \
    /software/$subdir/com/packages/jdk* \
    /software/$subdir/com/packages/java* \
    /local/encap/jdk* \
    /local/encap/java* ; do
    if test -d $dir ; then
        case "$dir" in
            *java-workshop* )
                if test -d "$dir/JDK/bin" ; then
                    if test -x "$dir/JDK/bin/java" \
                         -a -x "$dir/JDK/bin/javac" ; then
                        $1="$dir/JDK"
                    fi
                fi
                ;;
            *java* | *jdk* )
                if test -x "$dir/bin/java" -a -x "$dir/bin/javac" ; then
                    $1="$dir"
                fi
                ;;
        esac
dnl
        # Not all releases work.  Try a simple program
        if test -n "${$1}" ; then
            AC_MSG_RESULT([found ${$1}])
            AC_MSG_CHECKING([if ${$1}/bin/javac compiles])
            PAC_JAVA_TRY_COMPILE( [${$1}/bin/javac], , , [
    public static void main( String args[] )
    {
        System.out.println( "Hello world" );
    }
            ], [ pac_java_working=yes ], [ pac_java_working=no ] )
            if test "$pac_java_working" = "yes" ; then
                AC_MSG_RESULT(yes)
                break
            else
                AC_MSG_RESULT(no)
                AC_MSG_CHECKING([for working Java in known locations])
                $1=""
            fi
        fi
dnl
    fi
done
if test -z "${$1}" ; then
    AC_MSG_RESULT(not found)
fi
])dnl
dnl
dnl PAC_PATH_JAVA(varname) - locate Java in User's $PATH
dnl
dnl where varname is the returned variable name of Java home directory
dnl
AC_DEFUN([PAC_PATH_JAVA],[
AC_MSG_CHECKING(for Java in user's PATH)
if test -n "$PATH" ; then
    $1=""
    Pac_USER_PATH=`echo $PATH | sed 's/:/ /g'`
    for dir in ${Pac_USER_PATH} ; do
        if test -d $dir ; then
            case "$dir" in
                *java-workshop* )
                    if test -x "$dir/java" -a -x "$dir/javac" ; then
                        Pac_JAVA_HOME="`echo $dir | sed -e 's%/JDK/bin/*$%%'`"
                        if test -d ${Pac_JAVA_HOME} ; then
                            $1=${Pac_JAVA_HOME}
                        else
                            $1=$dir/..
                        fi
                    fi
                    ;;
                *java* | *jdk* )
                    if test -x "$dir/java" -a -x "$dir/javac" ; then
                        Pac_JAVA_HOME="`echo $dir | sed -e 's%/bin/*$%%'`"
                        if test -d ${Pac_JAVA_HOME} ; then
                            $1=${Pac_JAVA_HOME}
                        else
                            $1=$dir/..
                        fi
                    fi
                    ;;
            esac
dnl
            # Not all releases work.  Try a simple program
            if test -n "${$1}" ; then
                AC_MSG_RESULT([found ${$1}])
                AC_MSG_CHECKING([if ${$1}/bin/javac compiles])
                PAC_JAVA_TRY_COMPILE( [${$1}/bin/javac], , , [
    public static void main( String args[] )
    {
        System.out.println( "Hello world" );
    }
                ], [ pac_java_working=yes ], [ pac_java_working=no ] )
                if test "$pac_java_working" = "yes" ; then
                    AC_MSG_RESULT(yes)
                    break
                else
                    AC_MSG_RESULT(no)
                    AC_MSG_CHECKING([for working Java in user's PATH])
                    $1=""
                fi
            fi
dnl
        fi
    done
fi
if test -z "${$1}" ; then
    AC_MSG_RESULT(not found)
fi
])dnl
