#ifndef MPIR_ERROR

/* Generic error handling code.  This handles inserting the file and line
   number (in MPI) where the error occured.  In addition, it
   checks the error handler and calls the appropriate one.  Finally, 
   it returns the errorcode as its value.
 */
#define MPIR_ERROR(comm,code,string) \
    MPIR_Error( comm, code, string, __FILE__, __LINE__ )

/* 
 * This routine can be called to call an MPI function and call the
 * appropriate error handler.
 */
#define MPIR_CALL(fcn,comm,msg) {if (mpi_errno = fcn) \
				 return MPIR_ERROR(comm,mpi_errno,msg);}

/* Here we define some additional error information values.  These need to be
   or'ed into the appropriate MPI error class (from mpi_errno.h) 
 */
#define MPIR_ERR_CLASS_BITS 8
#define MPIR_ERR_CLASS_MASK 0xff

/* Here are error CODE bits, to be or'ed with the error CLASS.
   In addition, some MPI_ERR types are defined that are CODES, not CLASSES 
 */

/* These are all error CODES mapped onto some of the error CLASSES.
   The error CLASS is the low byte; the code is in the second byte 
 */

/* MPI_ERR_COMM */
#define MPIR_ERR_COMM_NULL    (1 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_COMM_NULL    (MPIR_ERR_COMM_NULL | MPI_ERR_COMM)
                                    /* NULL communicator argument 
				       passed to function */
#define MPIR_ERR_COMM_INTER   (2 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_COMM_INTER   (MPIR_ERR_COMM_INTER | MPI_ERR_COMM)
			            /* Intercommunicator is not allowed 
				       in function */
#define MPIR_ERR_COMM_INTRA   (3 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_COMM_INTRA   (MPIR_ERR_COMM_INTRA | MPI_ERR_COMM)      
                                    /* Intracommunicator is not allowed 
				       in function */

/* MPI_ERR_TYPE */
#define MPIR_ERR_UNCOMMITTED  (1 << MPIR_ERR_CLASS_BITS) 
                                    /* Uncommitted datatype */  

/* MPI_ERR_OP */
#define MPIR_ERR_NOT_DEFINED  (1 << MPIR_ERR_CLASS_BITS)
                                    /* Operation not defined for this 
				      datatype */

/* MPI_ERR_ARG */
#define MPIR_ERR_ERRORCODE    (1 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_ERRORCODE    (MPIR_ERR_ERRORCODE | MPI_ERR_ARG)
			            /* Invalid error code */
#define MPIR_ERR_NULL         (2 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_NULL         (MPIR_ERR_NULL | MPI_ERR_ARG)
                                    /* Null parameter */
#define MPIR_ERR_PERM_KEY     (4 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_PERM_KEY     (MPIR_ERR_PERM_KEY | MPI_ERR_ARG)
                                    /* Can't free a perm key */
#define MPIR_ERR_PERM_TYPE    (5 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_PERM_TYPE    (MPIR_ERR_PERM_TYPE | MPI_ERR_ARG)      
                                    /* Can't free a perm type */
#define MPIR_ERR_PERM_OP      (6 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_PERM_OP      (MPIR_ERR_PERM_OP | MPI_ERR_ARG)      
                                    /* Can't free a permanent operator */
#define MPIR_ERR_FORTRAN_ADDRESS_RANGE (7 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_FORTRAN_ADDRESS_RANGE \
                             (MPIR_ERR_FORTRAN_ADDRESS_RANGE | MPI_ERR_ARG)
           /* Address of location given to MPI_ADDRESS does not fit in 
	      Fortran int */

/* MPI_ERR_BUFFER */
#define MPIR_ERR_BUFFER_EXISTS (1 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_BUFFER_EXISTS (MPIR_ERR_BUFFER_EXISTS | MPI_ERR_BUFFER)

#define MPIR_ERR_USER_BUFFER_EXHAUSTED (2 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_USER_BUFFER_EXHAUSTED \
                            (MPIR_ERR_USER_BUFFER_EXHAUSTED | MPI_ERR_BUFFER)
                                    /* BSend with insufficent buffer space */
#define MPIR_ERR_BUFFER_ALIAS (3 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_BUFFER_ALIAS (MPIR_ERR_BUFFER_ALIAS | MPI_ERR_BUFFER)
                                    /* User has aliased an argument */


/* MPI_ERR_OTHER */
#define MPIR_ERR_LIMIT        (1 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_LIMIT        (MPIR_ERR_LIMIT | MPI_ERR_OTHER)
                                    /* limit reached */
#define MPIR_ERR_NOMATCH      (2 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_NOMATCH      (MPIR_ERR_NOMATCH | MPI_ERR_OTHER)
                                    /* no recv posted for ready send */
/* #define MPI_ERR_BAD_ARGS    24 */
#define MPIR_ERR_INIT         (3 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_INIT         (MPIR_ERR_INIT | MPI_ERR_OTHER)
                                    /* MPI_INIT already called */
#define MPIR_ERR_PRE_INIT     (4 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_PRE_INIT     (MPIR_ERR_PRE_INIT | MPI_ERR_OTHER)
                                    /* MPI_INIT has not been called */


/* MPI_ERR_INTERN */
#define MPIR_ERR_EXHAUSTED    (1 << MPIR_ERR_CLASS_BITS)
#define MPI_ERR_EXHAUSTED    (MPIR_ERR_EXHAUSTED | MPI_ERR_INTERN)
                                    /* Memory exhausted */

/* 
   Standardized error testing

   Many of the MPI routines take arguments of the same type.  These
   macros provide tests for these objects.

   It is intended that the tests for a valid opaque object such as 
   a communicator can check to insure that the object is both a communicator
   and that it is valid (hasn't been freed).  They can also test for
   null pointers.

   These are not used yet; we are still looking for the best ways to 
   define them.

   The intent is to use them in this manner:

   if (MPIR_TEST_...() || MPIR_TEST_... || ... ) 
        return MPIR_ERROR( comm, mpi_errno, "Error in MPI_routine" );

   The hope is, that in the NO_ERROR_CHECKING case, the optimizer will
   be smart enough to remove the code.
 */
#ifdef MPIR_NO_ERROR_CHECKING
#define MPIR_TEST_SEND_TAG(comm,tag)      0
#define MPIR_TEST_RECV_TAG(comm,tag)      0
#define MPIR_TEST_SEND_RANK(comm,rank)    0
#define MPIR_TEST_RECV_RANK(comm,rank)    0
#define MPIR_TEST_COUNT(comm,count)       0
#define MPIR_TEST_OP(comm,op)             0
#define MPIR_TEST_GROUP(comm,group)       0
#define MPIR_TEST_COMM(comm,comm1)        0
#define MPIR_TEST_REQUEST(comm,request)   0
#define MPIR_TEST_IS_DATATYPE(comm,datatype) 0
#define MPIR_TEST_DATATYPE(comm,datatype) 0
#define MPIR_TEST_ERRHANDLER(comm,errhandler) 0
#define MPIR_TEST_ALIAS(b1,b2)            0
#define MPIR_TEST_ARG(arg)                0

#else
#ifdef MPIR_HAS_COOKIES
#define MPIR_TEST_COOKIE(val,value) || ( ((val)->cookie != (value)) )
#else 
#define MPIR_TEST_COOKIE(val,value) 
#endif


#define MPIR_TEST_SEND_TAG(comm,tag) \
    ( ((tag) < 0 ) && (mpi_errno = MPI_ERR_TAG ))
    /* This requires MPI_ANY_TAG == -1 */
#define MPIR_TEST_RECV_TAG(comm,tag) \
    ( ((tag) < MPI_ANY_TAG) &&  (mpi_errno = MPI_ERR_TAG ))
    /* This exploits MPI_ANY_SOURCE==-2, MPI_PROC_NULL==-1 */
#define MPIR_TEST_SEND_RANK(comm,rank) \
    ( ((rank) < MPI_PROC_NULL || (rank) >= (comm)->np)\
           && (mpi_errno = MPI_ERR_RANK))
    /* This requires min(MPI_PROC_NULL,MPI_ANY_SOURCE)=-2 */
#define MPIR_TEST_RECV_RANK(comm,rank) \
    (((rank) < -2 || (rank) >= (comm)->np) && \
     (mpi_errno = MPI_ERR_RANK))
#define MPIR_TEST_COUNT(comm,count) ( ((count) < 0) && \
				     (mpi_errno = MPI_ERR_COUNT))
#define MPIR_TEST_OP(comm,op)       \
    ( (!(op) MPIR_TEST_COOKIE(op,MPIR_OP_COOKIE)) && (mpi_errno = MPI_ERR_OP ))
#define MPIR_TEST_GROUP(comm,group) \
    ( (!(group) MPIR_TEST_COOKIE(group,MPIR_GROUP_COOKIE)) && \
       (mpi_errno = MPI_ERR_GROUP ))
#define MPIR_TEST_COMM(comm,comm1)  \
    ( (!(comm1) MPIR_TEST_COOKIE(comm1,MPIR_COMM_COOKIE)) \
     && (mpi_errno = MPI_ERR_COMM ))
#define MPIR_TEST_REQUEST(comm,request) \
 ( (!(request) MPIR_TEST_COOKIE(&((request)->chandle),MPIR_REQUEST_COOKIE)) \
     && (mpi_errno = MPI_ERR_REQUEST))

#ifdef MPIR_HAS_COOKIES
#define MPIR_TEST_IS_DATATYPE(comm,datatype) \
    ( (!(datatype) || \
       (!MPIR_TEST_PREDEF_DATATYPE(datatype) && \
	((datatype)->cookie!=MPIR_DATATYPE_COOKIE))) \
     && (mpi_errno = MPI_ERR_TYPE ))
#else
#define MPIR_TEST_IS_DATATYPE(comm,datatype) \
    ( (!(datatype) ) && (mpi_errno = MPI_ERR_TYPE ))
#endif
#define MPIR_TEST_DATATYPE(comm,datatype) \
    (MPIR_TEST_IS_DATATYPE(comm,datatype) || \
  (!MPIR_TEST_PREDEF_DATATYPE(datatype) && !(datatype)->committed && \
   (mpi_errno = (MPI_ERR_TYPE | MPIR_ERR_UNCOMMITTED))))

#define MPIR_TEST_ERRHANDLER(comm,errhandler) \
    ( ( (!(errhandler) MPIR_TEST_COOKIE(errhandler,MPIR_ERRHANDLER_COOKIE)) \
       && (mpi_errno = MPI_ERR_ARG )))
#define MPIR_TEST_HBT_NODE(comm,node) \
    ( ( !(node) MPIR_TEST_COOKIE(node,MPIR_HBT_NODE_COOKIE)) \
      && (mpi_errno = MPI_ERR_INTERN))
#define MPIR_TEST_HBT(comm,hbt) \
    ( ( !(hbt) MPIR_TEST_COOKIE(hbt,MPIR_HBT_COOKIE)) \
      && (mpi_errno = MPI_ERR_INTERN))

#define MPIR_TEST_ALIAS(b1,b2)      \
    ( ((b1)==(b2)) && (mpi_errno = (MPI_ERR_BUFFER | MPIR_ERR_BUFFER_ALIAS) ))
#define MPIR_TEST_ARG(arg)  (!(arg) && (mpi_errno = MPI_ERR_ARG) )
#endif 

/* 
   Here are the definitions of the actual error messages; this is also needed
   by end-users (MPI error names are visible to all)
 */
#include "mpi_errno.h"

#endif
