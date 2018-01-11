
      program main
      Include 'mpif.h'
      integer st(3)
*
      call MPI_init (icode)
      call MPI_comm_size (mpi_comm_world,nproc,icode)
      write (*,*) icode
      call MPI_comm_rank (mpi_comm_world,ipme,icode)
      write (*,*) icode
      i = 1
      call MPI_send (i,1,MPI_integer,0,0,mpi_comm_world,icode)
      write (*,*) icode
      call MPI_recv (i,1,MPI_integer,0,0,mpi_comm_world,st,icode)
      write (*,*) icode
      call MPI_FINALIZE(icode)
      end
