c
c Check the handling of error strings from Fortran
c      

      program errstringsf

      include 'mpif.h'

      character*(MPI_MAX_ERROR_STRING) errs
      integer i, reslen, reserr, ierr

c      
c     Fill the string with 'x' to check that
c     blank padding happens correctly.
c
      call MPI_Init( ierr )
      do i = 1,MPI_MAX_ERROR_STRING
         errs(i:i) = 'x'
      end do

      call mpi_error_string(mpi_err_buffer, errs, reslen, reserr)

      if (errs(reslen+1:) .ne. ' ') then
         print *,' Fortran strings are not correctly blank padded'
      else
         print *,' Fortran strings are assigned ok'
      end if

c     Check that the length was right
      if (errs(reslen:reslen) .eq. ' ') then
         print *,' Length of result is wrong'
      end if

      call MPI_Finalize( ierr )

      end
