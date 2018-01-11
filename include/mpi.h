/*
 *  $Id: mpi.h,v 1.50 1996/01/08 19:52:29 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* user include file for MPI programs */

#ifndef _MPI_INCLUDE
#define _MPI_INCLUDE

/* Keep C++ compilers from getting confused */
#if defined(__cplusplus)
extern "C" {
#endif


 /* Results of the compare operations */
/* These should stay ordered */
#define MPI_IDENT     0
#define MPI_CONGRUENT 1
#define MPI_SIMILAR   2
#define MPI_UNEQUAL   3

/* Data types */
typedef struct MPIR_DATATYPE *MPI_Datatype;

#ifndef FOO
/* These "magic" values are typical for 4-byte-word machines; these 
   encode type and length.  
   Q: will a datatype need to be an int?  That is, will MPI_Datatype
   need to be an int?  gcc accepts these as case labels.  But icc and
   xlc do not.  Well, what if MPI_Datatype is MPI_Aint?  The slipperly slope?

   Grumble.  What to do about non-contiguous, predefined names (e.g.,
   MPI_DOUBLE_INT)?  What about MPI_UB/LB?
   For the moment, leave these as pointers to the structures
 */
#define MPI_CHAR           ((MPI_Datatype)0x10)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)0x20)
#define MPI_BYTE           ((MPI_Datatype)0x30)
#define MPI_SHORT          ((MPI_Datatype)0x11)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)0x21)
#define MPI_INT            ((MPI_Datatype)0x13)
#define MPI_UNSIGNED       ((MPI_Datatype)0x23)
#define MPI_LONG           ((MPI_Datatype)0x33)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)0x43)
#define MPI_FLOAT          ((MPI_Datatype)0x53)
#define MPI_DOUBLE         ((MPI_Datatype)0x17)
#define MPI_LONG_DOUBLE    ((MPI_Datatype)0x1f)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)0x2f)
#endif

/* 
   To allow compile-time use of MPI 'constants', they are declared in
   static storage.  This is incomplete at present. 

   Note that this will work for C but not for Fortran; in the Fortran
   case we will probably need predefined indices.  A C version that
   uses indices might have lower latency because of few cache misses
   while searching out the features of the basic datatypes.
 */
#ifdef FOO
extern struct MPIR_DATATYPE MPIR_I_CHAR, MPIR_I_SHORT, MPIR_I_INT, MPIR_I_LONG,
                            MPIR_I_UCHAR, MPIR_I_USHORT, MPIR_I_UINT, 
                            MPIR_I_ULONG, MPIR_I_FLOAT, MPIR_I_DOUBLE, 
                            MPIR_I_LONG_DOUBLE, MPIR_I_LONG_LONG_INT, 
                            MPIR_I_BYTE;
#endif
extern struct MPIR_DATATYPE MPIR_I_PACKED, MPIR_I_LONG_DOUBLE_INT,
                            MPIR_I_UB, MPIR_I_LB,
                            MPIR_I_2INTEGER, 
                            MPIR_I_FLOAT_INT, MPIR_I_DOUBLE_INT, 
                            MPIR_I_LONG_INT, MPIR_I_SHORT_INT, MPIR_I_2INT,
                            MPIR_I_REAL, MPIR_I_DOUBLE_PRECISION, 
                            MPIR_I_COMPLEX, MPIR_I_DCOMPLEX, 
                            MPIR_I_LONG_DOUBLE_INT, 
                            MPIR_I_LOGICAL;
#ifdef FOO
#define MPI_CHAR (&MPIR_I_CHAR)
#define MPI_BYTE (&MPIR_I_BYTE)
#define MPI_SHORT (&MPIR_I_SHORT)
#define MPI_INT (&MPIR_I_INT)
#define MPI_LONG (&MPIR_I_LONG)
#define MPI_FLOAT (&MPIR_I_FLOAT)
#define MPI_DOUBLE (&MPIR_I_DOUBLE)

#define MPI_UNSIGNED_CHAR  (&MPIR_I_UCHAR)
#define MPI_UNSIGNED_SHORT (&MPIR_I_USHORT)
#define MPI_UNSIGNED (&MPIR_I_UINT)
#define MPI_UNSIGNED_LONG (&MPIR_I_ULONG)
#endif

#define MPI_PACKED          (&MPIR_I_PACKED)
#define MPI_UB              (&MPIR_I_UB)
#define MPI_LB              (&MPIR_I_LB)

#define MPI_FLOAT_INT       (&MPIR_I_FLOAT_INT)
#define MPI_LONG_INT        (&MPIR_I_LONG_INT)
#define MPI_DOUBLE_INT      (&MPIR_I_DOUBLE_INT)
#define MPI_SHORT_INT       (&MPIR_I_SHORT_INT)
#define MPI_2INT            (&MPIR_I_2INT)

#define MPI_LONG_DOUBLE_INT (&MPIR_I_LONG_DOUBLE_INT)

/***********************************************************************/
/* The following datatypes are for Fortran and SHOULD NOT BE USED      */
/***********************************************************************/
extern MPI_Datatype MPI_2INTEGER;
extern MPI_Datatype MPIR_2real_dte, MPIR_2double_dte, 
                    MPIR_2complex_dte, MPIR_2dcomplex_dte, 
                    MPIR_int1_dte, 
                    MPIR_int2_dte, MPIR_int4_dte, MPIR_real4_dte, 
                    MPIR_real8_dte, MPI_REAL, MPI_DOUBLE_PRECISION;
/***********************************************************************/
/* 
   The layouts for the types MPI_DOUBLE_INT etc are simply
   struct { 
       double var;
       int    loc;
   }
   This is documented in the man pages on the various datatypes.   
 */

/* Communicators */
typedef struct MPIR_COMMUNICATOR *MPI_Comm;
extern MPI_Comm MPI_COMM_WORLD, MPI_COMM_SELF;

/* Groups */
typedef struct MPIR_GROUP *MPI_Group;
extern MPI_Group MPI_GROUP_EMPTY;

/* Collective operations */
typedef struct MPIR_OP *MPI_Op;
extern struct MPIR_OP MPIR_I_MAX, MPIR_I_MIN, MPIR_I_SUM, MPIR_I_PROD, 
              MPIR_I_LAND, MPIR_I_BAND, MPIR_I_LOR, MPIR_I_BOR, MPIR_I_LXOR, 
              MPIR_I_BXOR, MPIR_I_MINLOC, MPIR_I_MAXLOC;
#define MPI_MAX    (MPI_Op)(&(MPIR_I_MAX))
#define MPI_MIN    (MPI_Op)(&(MPIR_I_MIN))
#define MPI_SUM    (MPI_Op)(&(MPIR_I_SUM))
#define MPI_PROD   (MPI_Op)(&(MPIR_I_PROD))
#define MPI_LAND   (MPI_Op)(&(MPIR_I_LAND))
#define MPI_BAND   (MPI_Op)(&(MPIR_I_BAND))
#define MPI_LOR    (MPI_Op)(&(MPIR_I_LOR))
#define MPI_BOR    (MPI_Op)(&(MPIR_I_BOR))
#define MPI_LXOR   (MPI_Op)(&(MPIR_I_LXOR))
#define MPI_BXOR   (MPI_Op)(&(MPIR_I_BXOR))
#define MPI_MINLOC (MPI_Op)(&(MPIR_I_MINLOC))
#define MPI_MAXLOC (MPI_Op)(&(MPIR_I_MAXLOC))

/* Permanent key values */
/* C Versions (return pointer to value) */
extern int MPI_TAG_UB, MPI_HOST, MPI_IO, MPI_WTIME_IS_GLOBAL;
/* Fortran Versions (return value) */
extern int MPIR_TAG_UB, MPIR_HOST, MPIR_IO, MPIR_WTIME_IS_GLOBAL;

/* Define some null objects */
#define MPI_COMM_NULL      ((MPI_Comm)0)
#define MPI_OP_NULL        ((MPI_Op)0)
#define MPI_GROUP_NULL     ((MPI_Group)0)
#define MPI_DATATYPE_NULL  ((MPI_Datatype)0)
#define MPI_REQUEST_NULL   ((MPI_Request)0)
#define MPI_ERRHANDLER_NULL 0

/* These are only guesses; make sure you change them in mpif.h as well */
#define MPI_MAX_PROCESSOR_NAME 256
#define MPI_MAX_ERROR_STRING   256

/* Pre-defined constants */
#define MPI_UNDEFINED      (-32766)
#define MPI_UNDEFINED_RANK MPI_UNDEFINED
#define MPI_KEYVAL_INVALID 0

/* Upper bound on the overhead in bsend for each message buffer */
#define MPI_BSEND_OVERHEAD 512

/* Topology types */
#define MPI_GRAPH  1
#define MPI_CART   2

#define MPI_BOTTOM      (void *)0

#define MPI_PROC_NULL   (-1)
#define MPI_ANY_SOURCE 	(-2)
#define MPI_ANY_TAG	(-1)


/* 
   Status object.  It is the only user-visible MPI data-structure 
   The "count" field is PRIVATE; use MPI_Get_count to access it. 
 */
typedef struct { 
    int count;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;

/* Must be able to hold any valid address.  64 bit machines may need
   to change this */
typedef long MPI_Aint;


#if (defined(__STDC__) || defined(__cplusplus))
typedef void (MPI_Handler_function)( MPI_Comm *, int *, ... );
#else
typedef void (MPI_Handler_function)();
#endif

typedef struct MPIR_Errhandler *MPI_Errhandler;
extern MPI_Errhandler MPI_ERRORS_ARE_FATAL, MPI_ERRORS_RETURN, 
       MPIR_ERRORS_WARN;
/* Make the C names for the null functions all upper-case.  Note that 
   this is required for systems that use all uppercase names for Fortran 
   externals.  */
#define MPI_NULL_COPY_FN   MPIR_null_copy_fn
#define MPI_NULL_DELETE_FN MPIR_null_delete_fn
#define MPI_DUP_FN         MPIR_dup_fn

/* MPI request opjects */
typedef union MPIR_HANDLE *MPI_Request;

/* User combination function */
#if defined(__STDC__) || defined(__cplusplus)
typedef void (MPI_User_function)( void *invec, void *inoutvec, int *len,  
				   MPI_Datatype *datatype); 
#else
typedef void (MPI_User_function)();
#endif

/* MPI Attribute copy and delete functions */
#if defined(__STDC__) || defined(__cplusplus)
typedef int (MPI_Copy_function)( MPI_Comm oldcomm, int keyval, 
				 void *extra_state,
			         void *attr_in, void *attr_out, int *flag);
typedef int (MPI_Delete_function)( MPI_Comm comm, int keyval, void *attr_val,
			           void *extra_state );
#else
typedef int (MPI_Copy_function)( );
typedef int (MPI_Delete_function)( );
#endif

/* MPI's error classes */
#include "mpi_errno.h"

/* Bindings of the MPI routines */
#include "binding.h"

#if defined(__cplusplus)
}
#endif

#endif




