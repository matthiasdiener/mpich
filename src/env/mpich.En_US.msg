$ This is the file defining the error messages for MPICH in US English
$ Make sure that you change the default messages in errorstring.c if
$ you change messages here.
$ We use one set for each MPI error CLASS, and individual numbers in each
$ class for the message.
$ (Note some systems allow using just a leading blank for comments.
$ At least HP-UX does not.)
$set 1 MPI_ERR_BUFFER
1 Invalid buffer pointer
2 Can not attach buffer when a buffer already exists
3 Insufficent space available in user-defined buffer
4 Invalid buffer pointer: Arguments must specify different buffers (no aliasing)
5 Buffer size (%d) is illegal
$set 2 MPI_ERR_COUNT
1 Invalid count argument
2 Invalid count argument: count[%d] is %d
$set 3 MPI_ERR_TYPE
1 Invalid datatype argument
2 Invalid datatype argument: Datatype has not been committed
3 Invalid datatype argument: Datatype is MPI_TYPE_NULL
4 Invalid datatype argument: Datatype argument is not a valid datatype\n\
Special bit pattern %x in datatype is incorrect.  May indicate an \n\
out-of-order argument or a deleted datatype
5 Invalid datatype argument: Can not free permanent data type
$set 4 MPI_ERR_TAG
1 Invalid message tag %d
$set 5 MPI_ERR_COMM
1 Invalid communicator
2 Invalid communicator: Null communicator
3 Invalid communicator: Intercommunicator is not allowed
4 Invalid communicator: Intracommunicator is not allowed
5 Communicator argument is not a valid communicator\n\
Special bit pattern %x in communicator is incorrect.  May indicate an \n\
out-of-order argument or a freed communicator
6 Error setting communicator name
$set 6 MPI_ERR_RANK
1 Invalid rank %d
$set 7 MPI_ERR_ROOT
1 Invalid root %d
$set 8 MPI_ERR_GROUP
1 Invalid group passed to function 
2 Invalid group passed to function: Null group
3 Group argument is not a valid group\n\
Special bit pattern %x in group is incorrect.  May indicate an \n\
out-of-order argument or a freed group
$set 9 MPI_ERR_OP
1 Invalid operation
2 Operation not defined for this datatype 
3 Invalid operation: Null MPI_Op
$set 10 MPI_ERR_TOPOLOGY
1 Invalid topology
$set 11 MPI_ERR_DIMS
1 Illegal dimension argument %d
$set 12 MPI_ERR_ARG
1 Invalid argument
2 Invalid error code
3 Null parameter
5 Can not free permanent attribute key
7 Can not free permanent MPI_Op
8 Address of location given to MPI_ADDRESS (%lx) does not fit in Fortran integer
9 Invalid keyval
10 Null MPI_Errhandler
11 MPI_Errhandler argument is not a valid errorhandler\n\
Special bit pattern %x in errhandler is incorrect.  May indicate an \n\
out-of-order argument or a deleted error handler
12 Illegal use of MPI_STATUS_IGNORE or MPI_STATUSES_IGNORE
$set 13 MPI_ERR_UNKNOWN
1 Unknown error
$set 14 MPI_ERR_TRUNCATE
1 Message truncated
$set 15 MPI_ERR_OTHER
2 System resource limit exceeded
3 Ready send had no matching receive
4 Can not call MPI_INIT twice!
5 MPI_INIT must be called before other MPI routines
6 MPIRUN chose the wrong device %s; program needs device %s
7 Could not convert index %d into a pointer\n\
The index may be an incorrect argument.\n\
Possible sources of this problem are a missing "include 'mpif.h'",\n\
a misspelled MPI object (e.g., MPI_COM_WORLD instead of MPI_COMM_WORLD)\n\
or a misspelled user variable for an MPI object (e.g., \n\
com instead of comm).
8 Pointer conversions exhausted\n\
Too many MPI objects may have been passed to/from Fortran\n\
without being freed
9 Error in recovering Fortran pointer (%d); already freed
10 Specified buffer is smaller than MPI_BSEND_OVERHEAD
$set 16 MPI_ERR_INTERN
1 Internal MPI error!
2 Internal MPI error! Out of internal memory
3 Internal MPI error! Cray restriction: Either both or neither buffers must be of type character
4 Internal MPI error! WARNING - sender format not ready!
5 Internal MPI error! WARNING - unrecognized msgrep %d
$set 17 MPI_ERR_IN_STATUS
1 Error code is in status
$set 18 MPI_ERR_PENDING
1 Pending request (no error)
$set 19 MPI_ERR_REQUEST
1 Illegal mpi_request handle
2 Illegal mpi_request handle: Null request
