

        program ddpass
       
#include<mpif.h>

        parameter (nproc = 8)
        real*8 buf(0:2*nproc-1,0:nproc-1)
        real*8 rbuf(0:2*nproc-1,0:nproc-1)
c       integer procblk(0:nproc-1)
        integer rreqst,sreqst
        integer arreqst(0:nproc-1),asreqst(0:nproc-1)
c        integer istatus(MPI_STATUS_SIZE)
        dimension status(nproc)
        integer slice
 
        common /out/ lout
        data lout /20/
 
c  initialize mpi
 
        call MPI_INIT(ierror)
 
c  figure out who's who

        call MPI_COMM_RANK(MPI_COMM_WORLD, myrank,ierror)
        call MPI_COMM_SIZE(MPI_COMM_WORLD, isize, ierror)
       
c  check to see if size = nproc
       
        if(isize.ne.nproc) call MPI_ABORT(MPI_COMM_WORLD,ierrno,ierror)

c       call MPI_ATTR_PUT(MPI_COMM_WORLD,MPI_TAG_UB,100000,ierror)


        newend=2*nproc
        do 15 l = 0,nproc-1
        do 10 n=0,newend-1
            buf(n,l) = n+100*myrank
            rbuf(n,l) = -5
            print *, buf(n,l)
10      continue
c       procblk(n) = n+1
15      continue

c we're going to try a little something with derived datatypes now
c generate one and then commit it (for life)

        call MPI_Type_vector(newend,nproc,1,MPI_DOUBLE_PRECISION,
     +                       slice,ierror)
        call MPI_Type_commit(slice,ierror)

c now pass them around a little bit ...
            do 56 l = 0,nproc-1
            if(myrank.ne.nproc-1) then
              call MPI_ISEND(buf(0,l),1,slice,myrank+1,myrank,
     +                       MPI_COMM_WORLD,sreqst,ierror)
            else 
              call MPI_ISEND(buf(0,l),1,slice,0,myrank,MPI_COMM_WORLD,
     +                       sreqst,ierror)
            endif

            if(myrank.eq.0) then
              call MPI_IRECV(rbuf(0,l),1,slice,nproc-1,nproc-1,
     +                       MPI_COMM_WORLD,rreqst,ierror)
            else
              call MPI_IRECV(rbuf(0,l),1,slice,myrank-1,myrank-1,
     +                       MPI_COMM_WORLD,rreqst,ierror)
            endif
            arreqst(l) = rreqst
            asreqst(l) = sreqst
56          continue
            call MPI_WAITALL(nproc,asreqst,status,ierror)
            call MPI_WAITALL(nproc,arreqst,status,ierror)

         print *,'after passing'
         do 30 l = 0,nproc-1
           do 35 n = 0,newend-1
             print *,'myrank is',myrank
             print *,rbuf(n,l)
35         continue
30       continue

c send them all to node 0
         do 57 l = 1,nproc-1
          if(myrank.ne.0) then
            call MPI_ISEND(rbuf(0,l),1,slice,0,myrank,MPI_COMM_WORLD,
     +                     sreqst,ierror)
            asreqst(l-1) = sreqst
          endif
          if(myrank.eq.0) then  
            call MPI_IRECV(rbuf(0,l),1,slice,MPI_ANY_SOURCE,l,
     +                     MPI_COMM_WORLD,rreqst,ierror)
            arreqst(l-1) = rreqst
          endif
57       continue
            call MPI_WAITALL(nproc-1,asreqst,status,ierror)
            call MPI_WAITALL(nproc-1,arreqst,status,ierror)

c and print it out

        if(myrank.eq.0)then
	   open(unit=lout,file='ddpass.out',status='unknown')
           do 41 l = 0,nproc-1
	   do 40 n=0,newend-1
             write(lout,*) rbuf(n,l)
40         continue
41         continue
           close(lout)
        endif

c  Now we get to free the datatype

        call MPI_Type_free(slice,ierror)
           
        call MPI_FINALIZE(ierror)
 
        stop
        end
