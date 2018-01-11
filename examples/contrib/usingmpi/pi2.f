c**********************************************************************
c   pi.f - compute pi by integrating f(x) = 4/(1 + x**2)     
c     
c   Each node: 
c    1) receives the number of rectangles used in the approximation.
c    2) calculates the areas of it's rectangles.
c    3) Synchronizes for a global summation.
c   Node 0 prints the result.
c
c  Variables:
c
c    pi  the calculated result
c    n   number of points of integration.  
c    x           midpoint of each rectangle's interval
c    f           function to integrate
c    sum,pi      area of rectangles
c    tmp         temporary scratch space for global summation
c    i           do loop index
c****************************************************************************
      program main

      include 'mpif.h'

      double precision  PI25DT
      parameter        (PI25DT = 3.141592653589793238462643d0)

      double precision  pi, h, sum, x, f, a, temp
      integer n, myid, numprocs, i, rc
      integer sumtype, sizetype, masternode
      integer status(MPI_STATUS_SIZE)

c                                 function to integrate
      f(a) = 4.d0 / (1.d0 + a*a)

      call MPI_INIT( ierr )
      call MPI_COMM_RANK( MPI_COMM_WORLD, myid, ierr )
      call MPI_COMM_SIZE( MPI_COMM_WORLD, numprocs, ierr )
      print *, "Process ", myid, " of ", numprocs, " is alive"

      sizetype   = 1
      sumtype    = 2
      masternode = 0
      
 10   if ( myid .eq. 0 ) then

         write(6,98)
 98      format('Enter the number of intervals: (0 quits)')
         read(5,99)n
 99      format(i10)

         do 15, i=1,numprocs-1
            call MPI_SEND(n,1,MPI_INTEGER,i,sizetype,MPI_COMM_WORLD,
     $           rc)
 15      continue   

      else
   
         call MPI_RECV(n,1,MPI_INTEGER,masternode,sizetype,
     $        MPI_COMM_WORLD,status,rc)

      endif

      print *, myid, ": ", "n = ", n

c                                 check for quit signal
      if ( n .le. 0 ) goto 30

c                                 calculate the interval size
      h = 1.0d0/n

      sum  = 0.0d0
      do 20 i = myid+1, n, numprocs
         x = h * (dble(i) - 0.5d0)
         sum = sum + f(x)
 20   continue
      pi = h * sum

c                                 collect all the partial sums
c     call p4globop(SUMTYPE,pi,1,DBLSIZ,p4dblsumop,P4DBL,rc)

      if (myid .ne. 0) then

         call MPI_SEND(pi,1,MPI_DOUBLE_PRECISION,0,sumtype,
     $        MPI_COMM_WORLD,rc)

      else

         do i=1,numprocs-1
           call MPI_RECV(temp,1,MPI_DOUBLE_PRECISION,i,sumtype,
     $          MPI_COMM_WORLD,status,rc)
           pi = pi + temp
        enddo
      endif

c                                 node 0 prints the answer.
      if (myid .eq. 0) then
         write(6, 97) pi, abs(pi - PI25DT)
 97      format('  pi is approximately: ', F18.16,
     +          '  Error is: ', F18.16)
      endif

      goto 10

 30   call MPI_FINALIZE(rc)
      end




