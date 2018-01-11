      PROGRAM MAIN

      include '../../../include/mpif.h'

C. Data layout
C. Number of tests
      integer PM_MAX_TESTS
      parameter (PM_MAX_TESTS=3)
C. Test data
      integer PM_TEST_INTEGER, fuzzy, Error, FazAttr
      integer PM_RANK_SELF
      integer Faz_World
      parameter (PM_TEST_INTEGER=12345)
      logical FazFlag
      external FazCreate, FazDelete
C
C. Initialize MPI
      call MPI_INIT(PM_GLOBAL_ERROR)

      PM_GLOBAL_ERROR = MPI_SUCCESS
C. Find out the number of processes
      call MPI_COMM_SIZE (MPI_COMM_WORLD,PM_NUM_NODES,PM_GLOBAL_ERROR)
      call MPI_COMM_RANK (MPI_COMM_WORLD,PM_RANK_SELF,PM_GLOBAL_ERROR)

      
      call MPI_keyval_create ( FazCreate, FazDelete, FazTag,
     &                         fuzzy, Error )

      call MPI_attr_get (MPI_COMM_WORLD, FazTag, FazAttr, 
     &                   FazFlag, Error)

      FazAttr = 120
      call MPI_attr_put (MPI_COMM_WORLD, FazTag, FazAttr, Error)
   
      print 1, ' Proc=',PM_Rank_self, ' ATTR=', FazAttr

C. Duplicate the Communicator and it's cached attributes

      call MPI_Comm_Dup (MPI_COMM_WORLD, Faz_WORLD, Error)


      call MPI_Attr_Get ( Faz_WORLD, FazTag, FazAttr, 
     &                    FazFlag, Error)

      if (FazFlag) then
        print 1, ' T-Flag, Proc=',PM_Rank_self,' ATTR=', FazAttr
      else
         print 1, ' F-Flag, Proc=',PM_Rank_self,' ATTR=',FazAttr
      end if
 1    format( a, i5, a, i5 )

C. Clean up MPI
      call MPI_Comm_free( Faz_WORLD, Error )
      call MPI_FINALIZE (PM_GLOBAL_ERROR)

      end


      INTEGER FUNCTION FazCreate (comm, keyval, fuzzy, 
     &                    attr_in, attr_out, flag)
      INTEGER comm, keyval, fuzzy, attr_in, attr_out
      LOGICAL flag
      attr_out = attr_in + 1
      flag = .true.
      FazCreate = 0
      END

      INTEGER FUNCTION FazDelete (comm, keyval, attr, extra)
      INTEGER comm, keyval, attr, extra
      include '../../../include/mpif.h'
      if (keyval .ne. MPI_KEYVAL_INVALID)then
         attr = attr -  1
      end if 
      FazDelete = 0
      END
