#!/bin/csh

# this script can be submitted by "qsub p4.sh"
#
# or if you'd like to see what you are requesting "qsub -verify p4.sh"
# and of course you can add other stuff on the command line


# make the CWD same as now
#$ -cwd

# join stdout and stderr 
#$ -j y

# we are going to be running a P4 job
#$ -par p4

# we want a total of 2 machines
#$ -l qty.eq.2,-exec.eq."/home/lusk/mpich/examples/test/pt2pt/third"

# now for the master
/home/lusk/mpich/examples/test/pt2pt/third $REMOTE_INFO

echo "bye"
