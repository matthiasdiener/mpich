        MODULE MPI2__<type>_s
        IMPLICIT NONE
        PRIVATE
        PUBLIC :: MPI_FILE_IREAD
        INTERFACE MPI_FILE_IREAD
          MODULE PROCEDURE MPI_FILE_IREAD_T
        END INTERFACE ! MPI_FILE_IREAD

        PUBLIC :: MPI_FILE_IREAD_AT
        INTERFACE MPI_FILE_IREAD_AT
          MODULE PROCEDURE MPI_FILE_IREAD_AT_T
        END INTERFACE ! MPI_FILE_IREAD_AT

        PUBLIC :: MPI_FILE_IREAD_SHARED
        INTERFACE MPI_FILE_IREAD_SHARED
          MODULE PROCEDURE MPI_FILE_IREAD_SHARED_T
        END INTERFACE ! MPI_FILE_IREAD_SHARED

        PUBLIC :: MPI_FILE_IWRITE
        INTERFACE MPI_FILE_IWRITE
          MODULE PROCEDURE MPI_FILE_IWRITE_T
        END INTERFACE ! MPI_FILE_IWRITE

        PUBLIC :: MPI_FILE_IWRITE_AT
        INTERFACE MPI_FILE_IWRITE_AT
          MODULE PROCEDURE MPI_FILE_IWRITE_AT_T
        END INTERFACE ! MPI_FILE_IWRITE_AT

        PUBLIC :: MPI_FILE_IWRITE_SHARED
        INTERFACE MPI_FILE_IWRITE_SHARED
          MODULE PROCEDURE MPI_FILE_IWRITE_SHARED_T
        END INTERFACE ! MPI_FILE_IWRITE_SHARED

        PUBLIC :: MPI_FILE_READ
        INTERFACE MPI_FILE_READ
          MODULE PROCEDURE MPI_FILE_READ_T
        END INTERFACE ! MPI_FILE_READ

        PUBLIC :: MPI_FILE_READ_ALL
        INTERFACE MPI_FILE_READ_ALL
          MODULE PROCEDURE MPI_FILE_READ_ALL_T
        END INTERFACE ! MPI_FILE_READ_ALL

        PUBLIC :: MPI_FILE_READ_ALL_BEGIN
        INTERFACE MPI_FILE_READ_ALL_BEGIN
          MODULE PROCEDURE MPI_FILE_READ_ALL_BEGIN_T
        END INTERFACE ! MPI_FILE_READ_ALL_BEGIN

        PUBLIC :: MPI_FILE_READ_ALL_END
        INTERFACE MPI_FILE_READ_ALL_END
          MODULE PROCEDURE MPI_FILE_READ_ALL_END_T
        END INTERFACE ! MPI_FILE_READ_ALL_END

        PUBLIC :: MPI_FILE_READ_AT
        INTERFACE MPI_FILE_READ_AT
          MODULE PROCEDURE MPI_FILE_READ_AT_T
        END INTERFACE ! MPI_FILE_READ_AT

        PUBLIC :: MPI_FILE_READ_AT_ALL
        INTERFACE MPI_FILE_READ_AT_ALL
          MODULE PROCEDURE MPI_FILE_READ_AT_ALL_T
        END INTERFACE ! MPI_FILE_READ_AT_ALL

        PUBLIC :: MPI_FILE_READ_AT_ALL_BEGIN
        INTERFACE MPI_FILE_READ_AT_ALL_BEGIN
          MODULE PROCEDURE MPI_FILE_READ_AT_ALL_BEGIN_T
        END INTERFACE ! MPI_FILE_READ_AT_ALL_BEGIN

        PUBLIC :: MPI_FILE_READ_AT_ALL_END
        INTERFACE MPI_FILE_READ_AT_ALL_END
          MODULE PROCEDURE MPI_FILE_READ_AT_ALL_END_T
        END INTERFACE ! MPI_FILE_READ_AT_ALL_END

        PUBLIC :: MPI_FILE_READ_ORDERED
        INTERFACE MPI_FILE_READ_ORDERED
          MODULE PROCEDURE MPI_FILE_READ_ORDERED_T
        END INTERFACE ! MPI_FILE_READ_ORDERED

        PUBLIC :: MPI_FILE_READ_ORDERED_BEGIN
        INTERFACE MPI_FILE_READ_ORDERED_BEGIN
          MODULE PROCEDURE MPI_FILE_READ_ORDERED_BEGIN_T
        END INTERFACE ! MPI_FILE_READ_ORDERED_BEGIN

        PUBLIC :: MPI_FILE_READ_ORDERED_END
        INTERFACE MPI_FILE_READ_ORDERED_END
          MODULE PROCEDURE MPI_FILE_READ_ORDERED_END_T
        END INTERFACE ! MPI_FILE_READ_ORDERED_END

        PUBLIC :: MPI_FILE_READ_SHARED
        INTERFACE MPI_FILE_READ_SHARED
          MODULE PROCEDURE MPI_FILE_READ_SHARED_T
        END INTERFACE ! MPI_FILE_READ_SHARED

        PUBLIC :: MPI_FILE_WRITE
        INTERFACE MPI_FILE_WRITE
          MODULE PROCEDURE MPI_FILE_WRITE_T
        END INTERFACE ! MPI_FILE_WRITE

        PUBLIC :: MPI_FILE_WRITE_ALL
        INTERFACE MPI_FILE_WRITE_ALL
          MODULE PROCEDURE MPI_FILE_WRITE_ALL_T
        END INTERFACE ! MPI_FILE_WRITE_ALL

        PUBLIC :: MPI_FILE_WRITE_ALL_BEGIN
        INTERFACE MPI_FILE_WRITE_ALL_BEGIN
          MODULE PROCEDURE MPI_FILE_WRITE_ALL_BEGIN_T
        END INTERFACE ! MPI_FILE_WRITE_ALL_BEGIN

        PUBLIC :: MPI_FILE_WRITE_ALL_END
        INTERFACE MPI_FILE_WRITE_ALL_END
          MODULE PROCEDURE MPI_FILE_WRITE_ALL_END_T
        END INTERFACE ! MPI_FILE_WRITE_ALL_END

        PUBLIC :: MPI_FILE_WRITE_AT
        INTERFACE MPI_FILE_WRITE_AT
          MODULE PROCEDURE MPI_FILE_WRITE_AT_T
        END INTERFACE ! MPI_FILE_WRITE_AT

        PUBLIC :: MPI_FILE_WRITE_AT_ALL
        INTERFACE MPI_FILE_WRITE_AT_ALL
          MODULE PROCEDURE MPI_FILE_WRITE_AT_ALL_T
        END INTERFACE ! MPI_FILE_WRITE_AT_ALL

        PUBLIC :: MPI_FILE_WRITE_AT_ALL_BEGIN
        INTERFACE MPI_FILE_WRITE_AT_ALL_BEGIN
          MODULE PROCEDURE MPI_FILE_WRITE_AT_ALL_BEGIN_T
        END INTERFACE ! MPI_FILE_WRITE_AT_ALL_BEGIN

        PUBLIC :: MPI_FILE_WRITE_AT_ALL_END
        INTERFACE MPI_FILE_WRITE_AT_ALL_END
          MODULE PROCEDURE MPI_FILE_WRITE_AT_ALL_END_T
        END INTERFACE ! MPI_FILE_WRITE_AT_ALL_END

        PUBLIC :: MPI_FILE_WRITE_ORDERED
        INTERFACE MPI_FILE_WRITE_ORDERED
          MODULE PROCEDURE MPI_FILE_WRITE_ORDERED_T
        END INTERFACE ! MPI_FILE_WRITE_ORDERED

        PUBLIC :: MPI_FILE_WRITE_ORDERED_BEGIN
        INTERFACE MPI_FILE_WRITE_ORDERED_BEGIN
          MODULE PROCEDURE MPI_FILE_WRITE_ORDERED_BEGIN_T
        END INTERFACE ! MPI_FILE_WRITE_ORDERED_BEGIN

        PUBLIC :: MPI_FILE_WRITE_ORDERED_END
        INTERFACE MPI_FILE_WRITE_ORDERED_END
          MODULE PROCEDURE MPI_FILE_WRITE_ORDERED_END_T
        END INTERFACE ! MPI_FILE_WRITE_ORDERED_END

        PUBLIC :: MPI_FILE_WRITE_SHARED
        INTERFACE MPI_FILE_WRITE_SHARED
          MODULE PROCEDURE MPI_FILE_WRITE_SHARED_T
        END INTERFACE ! MPI_FILE_WRITE_SHARED

        CONTAINS
!
        SUBROUTINE MPI_FILE_IREAD_T(FH, BUF, COUNT, DATATYPE, REQUEST, &
      &   IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR
        EXTERNAL MPI_FILE_IREAD
        CALL MPI_FILE_IREAD(FH, BUF, COUNT, DATATYPE, REQUEST, IERROR)
        END SUBROUTINE MPI_FILE_IREAD_T
!
        SUBROUTINE MPI_FILE_IREAD_AT_T(FH, OFFSET, BUF, COUNT,         &
      &   DATATYPE, REQUEST, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_IREAD_AT
        CALL MPI_FILE_IREAD_AT(FH, OFFSET, BUF, COUNT, DATATYPE,       &
      &   REQUEST, IERROR) 
        END SUBROUTINE MPI_FILE_IREAD_AT_T
!
        SUBROUTINE MPI_FILE_IREAD_SHARED_T(FH, BUF, COUNT, DATATYPE,   &
      &   REQUEST, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR
        EXTERNAL MPI_FILE_IREAD_SHARED
        CALL MPI_FILE_IREAD_SHARED(FH, BUF, COUNT, DATATYPE, REQUEST,  &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_IREAD_SHARED_T
!
        SUBROUTINE MPI_FILE_IWRITE_T(FH, BUF, COUNT, DATATYPE,         &
      &   REQUEST, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR
        EXTERNAL MPI_FILE_IWRITE
        CALL MPI_FILE_IWRITE(FH, BUF, COUNT, DATATYPE, REQUEST,        &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_IWRITE_T
!
        SUBROUTINE MPI_FILE_IWRITE_AT_T(FH, OFFSET, BUF, COUNT,        &
      &   DATATYPE, REQUEST, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_IWRITE_AT
        CALL MPI_FILE_IWRITE_AT(FH, OFFSET, BUF, COUNT, DATATYPE,      &
      &   REQUEST, IERROR) 
        END SUBROUTINE MPI_FILE_IWRITE_AT_T
!
        SUBROUTINE MPI_FILE_IWRITE_SHARED_T(FH, BUF, COUNT, DATATYPE,  &
      &   REQUEST, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, REQUEST, IERROR
        EXTERNAL MPI_FILE_IWRITE_SHARED
        CALL MPI_FILE_IWRITE_SHARED(FH, BUF, COUNT, DATATYPE, REQUEST, &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_IWRITE_SHARED_T
!
        SUBROUTINE MPI_FILE_READ_T(FH, BUF, COUNT, DATATYPE, STATUS,   &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ
        CALL MPI_FILE_READ(FH, BUF, COUNT, DATATYPE, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_READ_T
!
        SUBROUTINE MPI_FILE_READ_ALL_T(FH, BUF, COUNT, DATATYPE,       &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ_ALL
        CALL MPI_FILE_READ_ALL(FH, BUF, COUNT, DATATYPE, STATUS,       &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_READ_ALL_T
!
        SUBROUTINE MPI_FILE_READ_ALL_BEGIN_T(FH, BUF, COUNT, DATATYPE, &
      &   IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR
        EXTERNAL MPI_FILE_READ_ALL_BEGIN
        CALL MPI_FILE_READ_ALL_BEGIN(FH, BUF, COUNT, DATATYPE, IERROR)
        END SUBROUTINE MPI_FILE_READ_ALL_BEGIN_T
!
        SUBROUTINE MPI_FILE_READ_ALL_END_T(FH, BUF, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ_ALL_END
        CALL MPI_FILE_READ_ALL_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_READ_ALL_END_T
!
        SUBROUTINE MPI_FILE_READ_AT_T(FH, OFFSET, BUF, COUNT,          &
      &   DATATYPE, STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE, MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_READ_AT
        CALL MPI_FILE_READ_AT(FH, OFFSET, BUF, COUNT, DATATYPE,        &
      &   STATUS, IERROR) 
        END SUBROUTINE MPI_FILE_READ_AT_T
!
        SUBROUTINE MPI_FILE_READ_AT_ALL_T(FH, OFFSET, BUF, COUNT,      &
      &   DATATYPE, STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE, MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_READ_AT_ALL
        CALL MPI_FILE_READ_AT_ALL(FH, OFFSET, BUF, COUNT, DATATYPE,    &
      &   STATUS, IERROR) 
        END SUBROUTINE MPI_FILE_READ_AT_ALL_T
!
        SUBROUTINE MPI_FILE_READ_AT_ALL_BEGIN_T(FH, OFFSET, BUF,       &
      &   COUNT, DATATYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_READ_AT_ALL_BEGIN
        CALL MPI_FILE_READ_AT_ALL_BEGIN(FH, OFFSET, BUF, COUNT,        &
      &   DATATYPE, IERROR) 
        END SUBROUTINE MPI_FILE_READ_AT_ALL_BEGIN_T
!
        SUBROUTINE MPI_FILE_READ_AT_ALL_END_T(FH, BUF, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ_AT_ALL_END
        CALL MPI_FILE_READ_AT_ALL_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_READ_AT_ALL_END_T
!
        SUBROUTINE MPI_FILE_READ_ORDERED_T(FH, BUF, COUNT, DATATYPE,   &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ_ORDERED
        CALL MPI_FILE_READ_ORDERED(FH, BUF, COUNT, DATATYPE, STATUS,   &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_READ_ORDERED_T
!
        SUBROUTINE MPI_FILE_READ_ORDERED_BEGIN_T(FH, BUF, COUNT,       &
      &   DATATYPE, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR
        EXTERNAL MPI_FILE_READ_ORDERED_BEGIN
        CALL MPI_FILE_READ_ORDERED_BEGIN(FH, BUF, COUNT, DATATYPE,     &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_READ_ORDERED_BEGIN_T
!
        SUBROUTINE MPI_FILE_READ_ORDERED_END_T(FH, BUF, STATUS,        &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_READ_ORDERED_END
        CALL MPI_FILE_READ_ORDERED_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_READ_ORDERED_END_T
!
        SUBROUTINE MPI_FILE_READ_SHARED_T(FH, BUF, COUNT, DATATYPE,    &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR 
        EXTERNAL MPI_FILE_READ_SHARED
        CALL MPI_FILE_READ_SHARED(FH, BUF, COUNT, DATATYPE, STATUS,    &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_READ_SHARED_T
!
        SUBROUTINE MPI_FILE_WRITE_T(FH, BUF, COUNT, DATATYPE, STATUS,  &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE
        CALL MPI_FILE_WRITE(FH, BUF, COUNT, DATATYPE, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_WRITE_T
!
        SUBROUTINE MPI_FILE_WRITE_ALL_T(FH, BUF, COUNT, DATATYPE,      &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_ALL
        CALL MPI_FILE_WRITE_ALL(FH, BUF, COUNT, DATATYPE, STATUS,      &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_ALL_T
!
        SUBROUTINE MPI_FILE_WRITE_ALL_BEGIN_T(FH, BUF, COUNT,          &
      &   DATATYPE, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR
        EXTERNAL MPI_FILE_WRITE_ALL_BEGIN
        CALL MPI_FILE_WRITE_ALL_BEGIN(FH, BUF, COUNT, DATATYPE,        &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_ALL_BEGIN_T
!
        SUBROUTINE MPI_FILE_WRITE_ALL_END_T(FH, BUF, STATUS, IERROR)
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_ALL_END
        CALL MPI_FILE_WRITE_ALL_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_WRITE_ALL_END_T
!
        SUBROUTINE MPI_FILE_WRITE_AT_T(FH, OFFSET, BUF, COUNT,         &
      &   DATATYPE, STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE, MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_WRITE_AT
        CALL MPI_FILE_WRITE_AT(FH, OFFSET, BUF, COUNT, DATATYPE,       &
      &   STATUS, IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_AT_T
!
        SUBROUTINE MPI_FILE_WRITE_AT_ALL_T(FH, OFFSET, BUF, COUNT,     &
      &   DATATYPE, STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE, MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_WRITE_AT_ALL
        CALL MPI_FILE_WRITE_AT_ALL(FH, OFFSET, BUF, COUNT, DATATYPE,   &
      &   STATUS, IERROR) 
        END SUBROUTINE     MPI_FILE_WRITE_AT_ALL_T
!
        SUBROUTINE MPI_FILE_WRITE_AT_ALL_BEGIN_T(FH, OFFSET, BUF,      &
      &   COUNT, DATATYPE, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_OFFSET_KIND
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR 
        INTEGER(KIND=MPI_OFFSET_KIND) OFFSET
        EXTERNAL MPI_FILE_WRITE_AT_ALL_BEGIN
        CALL MPI_FILE_WRITE_AT_ALL_BEGIN(FH, OFFSET, BUF, COUNT,       &
      &   DATATYPE, IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_AT_ALL_BEGIN_T
!
        SUBROUTINE MPI_FILE_WRITE_AT_ALL_END_T(FH, BUF, STATUS,        &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_AT_ALL_END
        CALL MPI_FILE_WRITE_AT_ALL_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_WRITE_AT_ALL_END_T
!
        SUBROUTINE MPI_FILE_WRITE_ORDERED_T(FH, BUF, COUNT, DATATYPE,  &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_ORDERED
        CALL MPI_FILE_WRITE_ORDERED(FH, BUF, COUNT, DATATYPE, STATUS,  &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_ORDERED_T
!
        SUBROUTINE MPI_FILE_WRITE_ORDERED_BEGIN_T(FH, BUF, COUNT,      &
      &   DATATYPE, IERROR) 
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, IERROR
        EXTERNAL MPI_FILE_WRITE_ORDERED_BEGIN
        CALL MPI_FILE_WRITE_ORDERED_BEGIN(FH, BUF, COUNT, DATATYPE,    &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_ORDERED_BEGIN_T
!
        SUBROUTINE MPI_FILE_WRITE_ORDERED_END_T(FH, BUF, STATUS,       &
      &   IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_ORDERED_END
        CALL MPI_FILE_WRITE_ORDERED_END(FH, BUF, STATUS, IERROR)
        END SUBROUTINE MPI_FILE_WRITE_ORDERED_END_T
!
        SUBROUTINE MPI_FILE_WRITE_SHARED_T(FH, BUF, COUNT, DATATYPE,   &
      &   STATUS, IERROR) 
        USE MPI_CONSTANTS,ONLY: MPI_STATUS_SIZE
        <type> BUF
        INTEGER FH, COUNT, DATATYPE, STATUS(MPI_STATUS_SIZE), IERROR
        EXTERNAL MPI_FILE_WRITE_SHARED
        CALL MPI_FILE_WRITE_SHARED(FH, BUF, COUNT, DATATYPE, STATUS,   &
      &   IERROR) 
        END SUBROUTINE MPI_FILE_WRITE_SHARED_T
        END MODULE MPI2__<type>_s
