      subroutine mpir_init_fcm( world, self, groupempty )
      integer world, self, groupempty
      include '../../include/mpif.h'
      MPI_COMM_WORLD  = world
      MPI_COMM_SELF   = self
      MPI_GROUP_EMPTY = groupempty
C
C     Tell C where MPI_BOTTOM is
      call mpir_init_bottom( MPI_BOTTOM )
      return
      end
      subroutine mpir_init_fop( maxf, minf, sumf, prodf, landf, bandf,
     $                          lorf, borf, lxorf, bxorf, maxlocf, 
     $                          minlocf, err_fatal, err_ret )
      integer maxf, minf, sumf, prodf, landf, bandf,
     $        lorf, borf, lxorf, bxorf, maxlocf, 
     $        minlocf, err_fatal, err_ret
      include '../../include/mpif.h'
C
      MPI_MAX    = maxf
      MPI_MIN    = minf
      MPI_SUM    = sumf
      MPI_PROD   = prodf
      MPI_LAND   = landf
      MPI_BAND   = bandf
      MPI_LOR    = lorf
      MPI_BOR    = borf
      MPI_LXOR   = lxorf
      MPI_BXOR   = bxorf
      MPI_MAXLOC = maxlocf
      MPI_MINLOC = minlocf
C
C     This works since an OP is given to Fortran as a pointer to a
C     structure containing the function; using 0 works as long as
C     (int)((void *)0) == 0.
C
      MPI_OP_NULL = 0
C
C     Error handlers
      MPI_ERRORS_ARE_FATAL = err_fatal
      MPI_ERRORS_RETURN    = err_ret
C
      return
      end
C
      subroutine mpir_init_fattr( tag_ub, host, io, wtime_g )
      integer tag_ub, host, io, wtime_g
      include '../../include/mpif.h'
C
      MPI_TAG_UB          = tag_ub
      MPI_HOST            = host
      MPI_IO              = io
      MPI_WTIME_IS_GLOBAL = wtime_g
      return
      end
C
      subroutine mpir_init_flog( itrue, ifalse )
      logical itrue, ifalse
C
      itrue  = .TRUE.
      ifalse = .FALSE.
      return
      end
