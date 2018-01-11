
      program main
      integer argc
      integer argv
      include 'mpif.h'
*
      call MPI_INIT( ierr )
      call MPI_COMM_RANK( MPI_COMM_WORLD, myid, ierr )
      call MPI_COMM_SIZE( MPI_COMM_WORLD, numnodes, ierr )
      call MPI_SEND(2.,1,MPI_real,myid,0,MPI_COMM_WORLD,rc)
*
      call MPI_FINALIZE(ierr)
      end

