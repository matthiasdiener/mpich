#! /bin/sh
#
if [ "$MPIR_HOME" = "" ] ; then 
    MPIR_HOME=/home/MPI/mpich
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
if [ $just_testing = 1 ] ; then
  echo $progname -np $np $cmdLineArgs
else
  $progname -np $np $cmdLineArgs
fi
