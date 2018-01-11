#!/bin/sh
# script to test each datatype for 2 nodes

program=prism_test_bc_mc_ge

echo "outputs in subdirectories bc# where # = 0, 1, ....."
echo

for i in 0 1 2 3 4 5 6 7 8 9 10 11
do
  usedir=bc$i
  if [ ! -d $usedir ]
  then
    mkdir $usedir
  fi
  cd $usedir
  echo
  echo "doing datatype $i"
  ln -s ../$program .
  mpirun -np 2 $program $i 1 8192 100 1 1 0 0 0 0 1 2
  rm $program
  cd ..
done
