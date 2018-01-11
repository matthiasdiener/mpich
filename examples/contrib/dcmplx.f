       program test1
c      messing around with mpi
c
      include '../../include/mpif.h'
c      include 'emfd.par'
      complex*16 y0,y0sum    
      complex*16 k0sum,k0,z0,cbuff
      complex*16 source
      real*8 ay,aysum,recvbuf,pi,sum
      integer itot(3),status(3),request
      integer ierror,Proc,Num_Proc,icmplx,ireal
      pi=3.1415927d0
      c1=dcmplx(0.0,1.0)
c
c  INITIALIZE MPI AND DETERMINE BOTH INDIVIDUAL PROCESSOR #
C   AND THE TOTAL NUMBER OF PROCESSORS
c
       call mpi_init(ierror)

       call MPI_COMM_RANK(MPI_COMM_WORLD,Proc,IERROR)
       write(6,*)'Proccessor # = ',Proc

       call MPI_COMM_SIZE(MPI_COMM_WORLD,Num_Proc,IERROR)
       write(6,*)'num. of proc. = ',Num_Proc
       
c
      ay=dfloat(proc)+1.0d0
      k0=dcmplx(dfloat(proc),pi)*c1 
c
c  TEST DOUBLE PRECISION SEND AND RECEIVE    \
c
      do i=1,Num_Proc
      npsend=i-1
      if(Proc.eq.npsend) then
      recvbuf=ay
        call MPI_send(recvbuf,1,MPI_DOUBLE_PRECISION,0,2001,
     1  MPI_COMM_WORLD,ierror)
      endif
      if(Proc.eq.0) then
        call MPI_RECV(recvbuf,1,MPI_DOUBLE_PRECISION,npsend,
     1  MPI_ANY_TAG,MPI_COMM_WORLD,status,ierror)
      sum=sum+recvbuf
      endif
      call MPI_BARRIER(MPI_COMM_WORLD,ierror)
      enddo
      call MPI_BCAST(sum,1,MPI_DOUBLE_PRECISION,0,MPI_COMM_WORLD,
     1ierror)
 
      write(6,*)'Proc=',proc,'  REAL SUM=',sum
c
c  NOW TEST DOUBLE PRECISION ALLREDUCE
c
      ay=pi*dfloat(proc)
      call MPI_ALLREDUCE(ay,aysum,1,MPI_DOUBLE_PRECISION,MPI_SUM,
     1  MPI_COMM_WORLD,ierror)
      write(6,*)'Proc=',proc,'  REDUCE SUM=',aysum 
c
c   NOW TEST COMPLEX SEND AND RECEIVE 
c
      do i=1,Num_Proc
      npsend=i-1
      if(Proc.eq.npsend) then
      cbuf=k0
        call MPI_send(cbuf,1,MPI_DOUBLE_COMPLEX,0,2001,
     1  MPI_COMM_WORLD,ierror)
      endif
      if(Proc.eq.0) then
        call MPI_RECV(cbuf,1,MPI_DOUBLE_COMPLEX,npsend,
     1  MPI_ANY_TAG,MPI_COMM_WORLD,status,ierror)
      k0sum=k0sum+cbuf
      endif
      call MPI_BARRIER(MPI_COMM_WORLD,ierror)
      enddo
      call MPI_BCAST(k0sum,1,MPI_DOUBLE_COMPLEX,0,MPI_COMM_WORLD,
     1ierror)
      write(6,*)'Proc=',proc,'  COMPLEX SUM=',k0sum

c   EXIT MPI
c
        call MPI_Finalize( ierr )
      stop
      end
