      integer function mpir_iargc()
C
      mpir_iargc = iargc()
      return
      end
c     
      subroutine mpir_getarg( i, s )
C
      integer       i
      character*(*) s
      call getarg(i,s)
      return
      end
