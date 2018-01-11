#! /bin/sh
#
if [ "$MPIR_HOME" = "" ] ; then 
    MPIR_HOME=/home/gropp/mpich
fi
if [ "$MPIR_HOME" = "#""MPIR_HOME""#" ] ; then
    MPIR_HOME=`pwd`/..
fi
MPIRUN_HOME=$MPIR_HOME/bin
if [ "$argsset" = "" ] ; then
   . $MPIRUN_HOME/mpirun.args
   argsset=1
fi
#
$Show $progname -np $np $cmdLineArgs
