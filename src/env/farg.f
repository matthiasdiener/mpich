      integer function MPIR_iargc()
      MPIR_iargc = iargc()
      return
      end
c     
      subroutine MPIR_getarg( i, s )
      integer       i
      character*(*) s
      call getarg( i, s )
      return
      end
