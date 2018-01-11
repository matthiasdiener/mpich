#! /bin/sh
#
if [ "$MPIR_HOME" = "" ] ; then 
    MPIR_HOME=/tmp_mnt/Net/alriga/alriga9/MPI/mpich
fi
if [ "$MPIR_HOME" = "#""MPIR_HOME""#" ] ; then
    MPIR_HOME=`pwd`/..
fi
MPIRUN_HOME=$MPIR_HOME/util
if [ "$argsset" = "" ] ; then
   . $MPIRUN_HOME/mpirun.args
   argsset=1
fi
#
$Show $progname -np $np $cmdLineArgs
