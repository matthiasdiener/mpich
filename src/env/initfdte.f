      subroutine mpir_init_fdtes( MPIR_int_dte, MPIR_float_dte,
     $     MPIR_double_dte, MPIR_complex_dte, MPIR_dcomplex_dte,
     $     MPIR_logical_dte,
     $     MPIR_char_dte, MPIR_byte_dte, MPIR_2int_dte, MPIR_2real_dte, 
     $     MPIR_2double_dte, MPIR_2complex_dte, MPIR_2dcomplex_dte,
     $     MPIR_int1_dte, MPIR_int2_dte, MPIR_int4_dte, MPIR_real4_dte,
     $     MPIR_real8_dte, MPIR_packed, MPIR_ub, MPIR_lb )
      integer MPIR_int_dte, MPIR_float_dte,
     $     MPIR_double_dte, MPIR_complex_dte, MPIR_dcomplex_dte,
     $     MPIR_logical_dte, MPIR_char_dte, MPIR_byte_dte,
     $     MPIR_2int_dte, 
     $     MPIR_2real_dte, MPIR_2double_dte, MPIR_2complex_dte,
     $     MPIR_2dcomplex_dte,
     $     MPIR_int1_dte, MPIR_int2_dte, MPIR_int4_dte, MPIR_real4_dte,
     $     MPIR_real8_dte, MPIR_packed, MPIR_ub, MPIR_lb
      include '../../include/mpif.h'
      MPI_INTEGER          = MPIR_int_dte
      MPI_REAL             = MPIR_float_dte
      MPI_DOUBLE_PRECISION = MPIR_double_dte
      MPI_COMPLEX          = MPIR_complex_dte
      MPI_DOUBLE_COMPLEX   = MPIR_dcomplex_dte
      MPI_LOGICAL          = MPIR_logical_dte
      MPI_CHARACTER        = MPIR_char_dte
      MPI_BYTE             = MPIR_byte_dte
      MPI_2REAL            = MPIR_2real_dte
      MPI_2DOUBLE_PRECISION= MPIR_2double_dte
      MPI_2INTEGER         = MPIR_2int_dte
      MPI_2COMPLEX         = MPIR_2complex_dte
      MPI_2DOUBLE_COMPLEX  = MPIR_2dcomplex_dte
      MPI_PACKED           = MPIR_packed
      MPI_UB               = MPIR_ub
      MPI_LB               = MPIR_lb
C
C     optional datatypes
      MPI_INTEGER1         = MPIR_int1_dte
      MPI_INTEGER2         = MPIR_int2_dte
      MPI_INTEGER4         = MPIR_int4_dte
      MPI_REAL4            = MPIR_real4_dte
      MPI_REAL8            = MPIR_real8_dte
      return
      end
C
      subroutine mpir_get_fsize()
      real r(2)
C     character c(2)
      double precision d(2)
      call mpir_init_fsize( r(1), r(2), d(1), d(2) )
      return
      end
