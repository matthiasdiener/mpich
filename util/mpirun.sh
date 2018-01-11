#!/bin/sh 

# MPIRUN
# This script tries to start jobs on whatever kind of machine you're on.
# Strategy - This program is built with a default device it uses in
# certain ways. The user can override this default from the command line.
#
# This sh script is designed to use other scripts to provide the commands
# to run each system, using the . filename.sh mechanism

#set verbose
MPIR_HOME=/home/MPI/mpich
if [ "$MPIR_HOME" = "#""MPIR_HOME""#" ] ; then
    MPIR_HOME=`pwd`/..
fi
MPIRUN_HOME=$MPIR_HOME/util
#
# Local routines
#
# End of routine

#
#
# Special, system specific values
#
# polling_mode is for systems that can select between polling and 
# interrupt-driven operation.  Currently, only IBM POE is so supported
# (TMC CMMD has some support for this choice of mode)
polling_mode=1

# Parse command line arguments
# The ultimate goal is to determine what kind of parallel machine this
# is we are running on. Then we know how to start jobs...
#
# Process common arguments (currently does ALL, but should pass unrecognized
# ones to called files)
#
hasprinthelp=1
. $MPIRUN_HOME/mpirun.args
argsset=1

#
# Jump to the correct code for the device (by pseudo machine)
#
case $machine in
    ch_cmmd)
	. $MPIRUN_HOME/mpirun.ch_cmmd
	;;
    ibmspx|ch_eui|ch_mpl)
	. $MPIRUN_HOME/mpirun.ch_mpl
	;;
    anlspx)
	. $MPIRUN_HOME/mpirun.anlspx
	;;
    ch_meiko|meiko)
	. $MPIRUN_HOME/mpirun.meiko
	;;
    ch_nc)
	. $MPIRUN_HOME/mpirun.ch_nc
	;;
    paragon|ch_nx)
	. $MPIRUN_HOME/mpirun.paragon
	;;
    inteldelta)
	. $MPIRUN_HOME/mpirun.delta
	;;
    i860|ipsc860)
	. $MPIRUN_HOME/mpirun.i860
	;;
    p4|ch_p4)
	. $MPIRUN_HOME/mpirun.ch_p4
	;;
    execer)
	. $MPIRUN_HOME/mpirun.execer
	;;
    ch_shmem)
	. $MPIRUN_HOME/mpirun.ch_shmem
    	;;
    ksr|sgi_mp|symm_ptx)
	. $MPIRUN_HOME/mpirun.p4shmem
	;;
    ch_tcp)
	. $MPIRUN_HOME/mpirun.execer
	;;
    chameleon)
	. $MPIRUN_HOME/mpirun.ch
	;;
    *)    
	#
	# This allows us to add a device without changing the base mpirun
	# code
	if [ -x $MPIRUN_HOME/mpirun.$device ] ; then
	    . $MPIRUN_HOME/mpirun.$device
	else
	    echo "Cannot find MPIRUN machine file for $arch,$machine"
	    echo "(Looking for $MPIRUN_HOME/mpirun.$device)"
	    # . $MPIRUN_HOME/mpirun.default
	    exit 1
	fi
  	;;
esac
exit 0

