c
c  This program solves Laplace's equation on a two-dimensional rectangle
c  using the successive over-relaxation method with red/black ordering.
c
c  The number of particles per processor in each direction must be even.
c
c  This is the MPI version.
c
c  Author: David W. Walker
c  Date:   August 18, 1995
c
c  http://www.epm.ornl.gov/~walker/mpi/examples/sor.html 
c  David W. Walker, Oak Ridge National Laboratory / (walkerdw@ornl.gov) 
c  Last Modified April 7, 1995 
c
        program sor
        implicit none
        include 'mpif.h'
        integer myrank, ierr, nprocs, nprocx, nprocy, pseudohost
        integer nptx, npty
        integer MAXX, MAXY
        parameter (pseudohost = 0)
        parameter (MAXX = 299, MAXY=299)
        real phi(0:MAXY+1,0:MAXX+1)
        logical mask(0:MAXY+1,0:MAXX+1)
        integer color(0:MAXY+1,0:MAXX+1)
        integer i, j, k, ix, iy, ipx, ipy, jpx, jpy
        integer RED, BLACK
        parameter (RED = 0, BLACK = 1)
        integer maxiter
        parameter (maxiter = 500)
        double precision timebegin, timeend
        integer insize, pos, src, m, n, status(MPI_STATUS_SIZE)
        parameter (insize = 1000)
        integer input(insize)
        real rnput(insize)
        integer nfixed, indx(1000), indy(1000)
        real omega, omega4, omegam, fixval(1000), buf(MAXY)
        integer ldest, rdest, udest, ddest
        integer lsrc, rsrc, usrc, dsrc
        integer dims(2), coords(2), from(2)
        logical periods(2)
        logical reorder
        integer comm2d
        integer typex, typey
c
c  Initialize MPI, find rank of each process, and the number of processes
c
        call mpi_init (ierr)
        call mpi_comm_rank (MPI_COMM_WORLD, myrank, ierr)
        call mpi_comm_size (MPI_COMM_WORLD, nprocs, ierr)
c
c
c  One process acts as the host and reads in the input
c
        if (myrank .eq. pseudohost) call get_input (input, rnput)
c
c  The input is broadcast to all processes
c
        call mpi_bcast (input, 5, mpi_integer, pseudohost, 
     #                  MPI_COMM_WORLD, ierr)
        nprocx = input(1)
        nprocy = input(2)
        nptx   = input(3)
        npty   = input(4)
        nfixed = input(5)
        call mpi_bcast (input(6), 2*nfixed, mpi_integer, pseudohost, 
     #                  MPI_COMM_WORLD, ierr)
        call mpi_bcast (rnput, 1+nfixed, mpi_real, pseudohost, 
     #                  MPI_COMM_WORLD, ierr)
        omega  = rnput(1)
        do i=1,nfixed
           indx(i)   = input(4+2*i)
           indy(i)   = input(5+2*i)
           fixval(i) = rnput(1+i)
        end do
c
c  Abort if incorrect.
c
        if (nprocx .lt. 0 .or. nprocy .lt. 0) then
           print *, 'Number of processes in each direction must be >0'
           goto 999
        else if (nprocx*nprocy .ne. nprocs) then
           print *, 'nprocx*nprocy must = ',nprocs
           goto 999
        endif

        if (nptx .gt. MAXX) then
           print *, 'Number of points per process must be <= ',MAXX
           goto 999
        end if
        if (npty .gt. MAXY) then
           print *, 'Number of points per process must be <= ',MAXY
           goto 999
        end if

        if (mod(nptx,2) .ne. 0 .or. mod(npty,2) .ne. 0) then
           print *, 'Number of points per process must be even'
           goto 999
        end if
c
c  Start timer
c
        timebegin = mpi_wtime ()
c
c  Set up 2D application topology
c
        periods(1) = .false.
        periods(2) = .false.
        dims(1)    = nprocy
        dims(2)    = nprocx
        reorder    = .false.
        call mpi_cart_create (MPI_COMM_WORLD, 2, dims, periods, reorder,
     #                        comm2d, ierr)
        call mpi_cart_coords (comm2d, myrank, 2, coords, ierr)
        call mpi_cart_shift (comm2d, 0,  1, dsrc, ddest, ierr)
        call mpi_cart_shift (comm2d, 0, -1, usrc, udest, ierr)
        call mpi_cart_shift (comm2d, 1,  1, rsrc, rdest, ierr)
        call mpi_cart_shift (comm2d, 1, -1, lsrc, ldest, ierr)
c
c  Set up the grid mask and grid arrays
c
        call setup_grid (phi, fixval, color, nptx, npty, nfixed,
     #                   indx, indy, coords, nprocx, nprocy,
     #                   mask, MAXY+1)
c
c  Set up general datatypes
c
        call mpi_type_contiguous (npty/2, mpi_real, typex, ierr)
        call mpi_type_vector (nptx/2, 1, MAXY+2, mpi_real, typey, ierr) 
        call mpi_type_commit (typex, ierr)
        call mpi_type_commit (typey, ierr)
c
c  Start main loop
c
        omega4 = 0.25*omega
        omegam = 1.0 - omega
        do k=1,maxiter
c
c  Exchange black boundary data
c
           call mpi_sendrecv (phi(1,1),      1, typex, ldest,  14,
     #                        phi(1,nptx+1), 1, typex, lsrc,   14,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(npty/2+1,nptx), 1, typex, rdest, 15,
     #                        phi(npty/2+1,0),    1, typex, rsrc,  15,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(1,1),      1, typey, udest, 16,
     #                        phi(npty+1,1), 1, typey, usrc,  16,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(npty,nptx/2+1), 1, typey, ddest, 17,
     #                        phi(0,nptx/2+1),    1, typey, dsrc,  17,
     #                        comm2d, status, ierr)
c
c  Update red points
c
           do j=1,nptx
              do i=1,npty
                 if (mask(i,j) .and. color(i,j) .eq. RED) phi(i,j) =
     #              omega4*(phi(i,j-1)+phi(i-1,j)+phi(i,j+1)+phi(i+1,j))
     #              + omegam*phi(i,j)
              end do
           end do
c
c  Exchange red boundary data
c
           call mpi_sendrecv (phi(npty/2+1,1),      1, typex, ldest, 10,
     #                        phi(npty/2+1,nptx+1), 1, typex, lsrc,  10,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(1,nptx), 1, typex, rdest, 11,
     #                        phi(1,0),    1, typex, rsrc,  11,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(1,nptx/2+1),      1, typey, udest, 12,
     #                        phi(npty+1,nptx/2+1), 1, typey, usrc,  12,
     #                        comm2d, status, ierr)
           call mpi_sendrecv (phi(npty,1), 1, typey, ddest, 13,
     #                        phi(0,1),    1, typey, dsrc,  13,
     #                        comm2d, status, ierr)
c
c  Update black points
c
           do j=1,nptx
              do i=1,npty
                 if (mask(i,j) .and. color(i,j) .eq. BLACK) phi(i,j) =
     #              omega4*(phi(i,j-1)+phi(i-1,j)+phi(i,j+1)+phi(i+1,j))
     #              + omegam*phi(i,j)
              end do
           end do
        end do
c
c  End main loop
c
        call mpi_type_free (typex, ierr)
        call mpi_type_free (typey, ierr)
c
c  Stop clock and write out timings
c
        timeend = mpi_wtime()
        print *,'Node', myrank,'  Elapsed time: ',
     #          timeend-timebegin,' seconds'
c
c  Print out results
c
        if (myrank .eq. pseudohost) open (7,file='sor.output')
        do m=0,nprocx-1
           do i=1,nptx
              do n=0,nprocy-1
                 if (coords(1) .eq. m .and. coords(2) .eq. n) then
                    if (myrank .eq. pseudohost) then
                       write (7,100) (phi(j,i),j=1,npty)
                    else
                       call mpi_send (phi(1,i), npty, mpi_real,
     #                                pseudohost, 20,
     #                                MPI_COMM_WORLD, ierr)
                    end if
                 else if (myrank .eq. pseudohost) then
                    from(1) = n
                    from(2) = m
                    call mpi_cart_rank (comm2d, from, src, ierr)
                    call mpi_recv (buf, npty, mpi_real, src, 20,
     #                             MPI_COMM_WORLD, status, ierr)
                    write (7,100) (buf(j),j=1,npty)
                 end if
              end do
           end do
        end do

        call mpi_comm_free (comm2d, ierr)
  999   call mpi_finalize (ierr)

        stop
  100   format(5e15.6)
        end

        subroutine get_input (input, rnput)
        implicit none
        integer input(*)
        real    rnput(*)
        integer i

        open(4,file='sor.input')
c
c  Read in number of processes in each direction of process mesh
c
        read (4,*) input(1)
        read (4,*) input(2)
c
c  Read in number of points per process in each direction
c
        read (4,*) input(3)
        read (4,*) input(4)
c
c  Enter the value of the SOR parameter omega
c
        read (4,*) rnput(1)
c
c  Input information on fixed interior points
c
        read (4,*) input(5)
        do i=1,input(5)
           read (4,*) input(4+2*i)
           read (4,*) input(5+2*i)
           read (4,*) rnput(1+i)
        end do

        return
        end

        subroutine setup_grid (phi, fixval, color, nptx, npty, nfixed,
     #                         indx, indy, coords, nprocx, nprocy,
     #                         mask, ldim)
        implicit none
        integer ldim
        real phi(0:ldim,0:*), fixval(*)
        integer color(0:ldim,0:*), nptx, npty, nfixed
        integer indx(*), indy(*), coords(2), nprocx, nprocy
        logical mask(0:ldim,0:*)
        integer i, j, ix, iy, ipx, ipy, jpx, jpy 
        integer RED, BLACK
        parameter (RED = 0, BLACK = 1)
       
        do j=1,nptx
           do i=1,npty
              mask(i,j)    = .true.
              phi(i,j)     = 0.0
              if ((j .le. nptx/2 .and. i .gt. npty/2) .or.
     #            (j .gt. nptx/2 .and. i .le. npty/2)) then
                  color(i,j) = RED
              else
                  color(i,j) = BLACK
              end if
           end do
        end do
        if (coords(2) .eq. 0) then
           do i=1,npty
              mask(i,1) = .false.
              phi(i,1)  = 0.0
           end do
        end if
        if (coords(2) .eq. nprocx-1) then
           do i=1,npty
              mask(i,nptx) = .false.
              phi(i,nptx)  = 0.0
           end do
        end if
        if (coords(1) .eq. 0) then
           do i=1,nptx
              mask(1,i) = .false.
              phi(1,i)  = 0.0
           end do
        end if
        if (coords(1) .eq. nprocy-1) then
           do i=1,nptx
              mask(npty,i) = .false.
              phi(npty,i)  = 0.0
           end do
        end if

        do i=1,nfixed
           ix  = indx(i) - 1
           iy  = indy(i) - 1
           ipx = ix/nptx
           ipy = iy/npty
           if (coords(1) .eq. ipy .and. coords(2) .eq. ipx) then
              jpx = mod(ix, nptx) + 1
              jpy = mod(iy, npty) + 1
              mask(jpy,jpx) = .false.
              phi(jpy,jpx)  = fixval(i)
           end if
        end do

        return
        end
