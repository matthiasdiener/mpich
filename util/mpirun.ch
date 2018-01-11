#! /bin/sh
#
if [ "$MPIR_HOME" = "" ] ; then 
    MPIR_HOME=/Net/antireo/antireo9/MPI/mpich
fi
if [ "$MPIR_HOME" = "#""MPIR_HOME""#" ] ; then
    MPIR_HOME=`pwd`/..
fi
if [ "lib/sun4/ch_p4" = "#""MPIRUN_BIN""#" ] ; then 
    MPIRUN_HOME=$MPIR_HOME/bin
else
    MPIRUN_HOME=$MPIR_HOME/lib/sun4/ch_p4
fi
if [ "$argsset" = "" ] ; then
   . $MPIRUN_HOME/mpirun.args
   argsset=1
fi
#
$Show $progname -np $np $cmdLineArgs
