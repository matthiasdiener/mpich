/*
 *  $Id: mpi.h,v 1.12 1998/05/21 20:26:32 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* user include file for MPI programs */

#ifndef _MPI_INCLUDE
#define _MPI_INCLUDE

/* 
   NEW_POINTERS enables using ints for the MPI objects instead of addresses.
   This is required by MPI 1.1, particularly for Fortran (also for C, 
   since you can now have "constant" values for things like MPI_INT, and
   they are stronger than "constant between MPI_INIT and MPI_FINALIZE".
   For example, we might want to allow MPI_INT as a case label.

   NEW_POINTERS is the default; I'm purging all code that doesn't use
   NEW_POINTERS from the source.
 */
#define NEW_POINTERS

/* Keep C++ compilers from getting confused */
#if defined(__cplusplus)
extern "C" {
#endif

/* We require that the C compiler support prototypes */
#define MPIR_ARGS(a) a

 /* Results of the compare operations */
/* These should stay ordered */
#define MPI_IDENT     0
#define MPI_CONGRUENT 1
#define MPI_SIMILAR   2
#define MPI_UNEQUAL   3

/* Data types
 * A more aggressive yet homogeneous implementation might want to 
 * make the values here the number of bytes in the basic type, with
 * a simple test against a max limit (e.g., 16 for long double), and
 * non-contiguous structures with indices greater than that.
 * 
 * Note: Configure knows these values for providing the Fortran optional
 * types (like MPI_REAL8).  Any changes here must be matched by changes
 * in configure.in
 */
typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)1)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)2)
#define MPI_BYTE           ((MPI_Datatype)3)
#define MPI_SHORT          ((MPI_Datatype)4)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)5)
#define MPI_INT            ((MPI_Datatype)6)
#define MPI_UNSIGNED       ((MPI_Datatype)7)
#define MPI_LONG           ((MPI_Datatype)8)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)9)
#define MPI_FLOAT          ((MPI_Datatype)10)
#define MPI_DOUBLE         ((MPI_Datatype)11)
#define MPI_LONG_DOUBLE    ((MPI_Datatype)12)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)13)

#define MPI_PACKED         ((MPI_Datatype)14)
#define MPI_LB             ((MPI_Datatype)15)
#define MPI_UB             ((MPI_Datatype)16)

/* 
   The layouts for the types MPI_DOUBLE_INT etc are simply
   struct { 
       double var;
       int    loc;
   }
   This is documented in the man pages on the various datatypes.   
 */
#define MPI_FLOAT_INT      ((MPI_Datatype)17)
#define MPI_DOUBLE_INT     ((MPI_Datatype)18)
#define MPI_LONG_INT       ((MPI_Datatype)19)
#define MPI_SHORT_INT      ((MPI_Datatype)20)
#define MPI_2INT           ((MPI_Datatype)21)
#define MPI_LONG_DOUBLE_INT ((MPI_Datatype)22)

/* Fortran types */
#define MPI_COMPLEX        ((MPI_Datatype)23)
#define MPI_DOUBLE_COMPLEX ((MPI_Datatype)24)
#define MPI_LOGICAL        ((MPI_Datatype)25)
#define MPI_REAL           ((MPI_Datatype)26)
#define MPI_DOUBLE_PRECISION ((MPI_Datatype)27)
#define MPI_INTEGER        ((MPI_Datatype)28)
#define MPI_2INTEGER       ((MPI_Datatype)29)
#define MPI_2COMPLEX       ((MPI_Datatype)30)
#define MPI_2DOUBLE_COMPLEX   ((MPI_Datatype)31)
#define MPI_2REAL             ((MPI_Datatype)32)
#define MPI_2DOUBLE_PRECISION ((MPI_Datatype)33)
#define MPI_CHARACTER         ((MPI_Datatype)1)

/* Communicators */
typedef int MPI_Comm;
#define MPI_COMM_WORLD 91
#define MPI_COMM_SELF 92

/* Groups */
typedef int MPI_Group;
#define MPI_GROUP_EMPTY 90

/* Collective operations */
typedef int MPI_Op;

#define MPI_MAX    (MPI_Op)(100)
#define MPI_MIN    (MPI_Op)(101)
#define MPI_SUM    (MPI_Op)(102)
#define MPI_PROD   (MPI_Op)(103)
#define MPI_LAND   (MPI_Op)(104)
#define MPI_BAND   (MPI_Op)(105)
#define MPI_LOR    (MPI_Op)(106)
#define MPI_BOR    (MPI_Op)(107)
#define MPI_LXOR   (MPI_Op)(108)
#define MPI_BXOR   (MPI_Op)(109)
#define MPI_MINLOC (MPI_Op)(110)
#define MPI_MAXLOC (MPI_Op)(111)

/* Permanent key values */
/* C Versions (return pointer to value) */
#define MPI_TAG_UB 81
#define MPI_HOST 83
#define MPI_IO 85
#define MPI_WTIME_IS_GLOBAL 87
/* Fortran Versions (return integer value) */
#define MPIR_TAG_UB 80
#define MPIR_HOST 82
#define MPIR_IO 84
#define MPIR_WTIME_IS_GLOBAL 86

/* Define some null objects */
#define MPI_COMM_NULL      ((MPI_Comm)0)
#define MPI_OP_NULL        ((MPI_Op)0)
#define MPI_GROUP_NULL     ((MPI_Group)0)
#define MPI_DATATYPE_NULL  ((MPI_Datatype)0)
#define MPI_REQUEST_NULL   ((MPI_Request)0)
#define MPI_ERRHANDLER_NULL ((MPI_Errhandler )0)

/* These are only guesses; make sure you change them in mpif.h as well */
#define MPI_MAX_PROCESSOR_NAME 256
#define MPI_MAX_ERROR_STRING   512
#define MPI_MAX_NAME_STRING     63		/* How long a name do you need ? */

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
    int private_count;
} MPI_Status;
/* MPI_STATUS_SIZE is not strictly required in C; however, it should match
   the value for Fortran */
#define MPI_STATUS_SIZE 5

/* Must be able to hold any valid address.  64 bit machines may need
   to change this */
/* #if (defined(_SX) && !defined(_LONG64)) */
/* NEC SX-4 in some modes needs this */
/* typedef long long MPI_Aint; */
/* #else */
/* typedef long MPI_Aint; */
/* #endif */

/* MPI Error handlers.  Systems that don't support stdargs can't use
   this definition
 */
#if defined(USE_STDARG) 
typedef void (MPI_Handler_function) ( MPI_Comm *, int *, ... );
#else
typedef void (MPI_Handler_function) ();
#endif

#define MPI_ERRORS_ARE_FATAL ((MPI_Errhandler)119)
#define MPI_ERRORS_RETURN    ((MPI_Errhandler)120)
#define MPIR_ERRORS_WARN     ((MPI_Errhandler)121)
typedef int MPI_Errhandler;
/* Make the C names for the null functions all upper-case.  Note that 
   this is required for systems that use all uppercase names for Fortran 
   externals.  */
#define MPI_NULL_COPY_FN   MPIR_null_copy_fn
#define MPI_NULL_DELETE_FN MPIR_null_delete_fn
#define MPI_DUP_FN         MPIR_dup_fn

/* MPI request opjects */
typedef union MPIR_HANDLE *MPI_Request;

/* User combination function */
typedef void (MPI_User_function) ( void *, void *, int *, MPI_Datatype * ); 

/* MPI Attribute copy and delete functions */
typedef int (MPI_Copy_function) ( MPI_Comm, int, void *,
					    void *, void *, int * );
typedef int (MPI_Delete_function) ( MPI_Comm, int, void *, void * );

#define MPI_VERSION 1
#define MPI_SUBVERSION 1
#define MPICH_NAME 1

/********************** MPI-2 FEATURES BEGIN HERE ***************************/
#define MPICH_HAS_C2F

/* for the datatype decoders */
#define MPI_COMBINER_NAMED              2312
#define MPI_COMBINER_CONTIGUOUS         2313
#define MPI_COMBINER_VECTOR             2314
#define MPI_COMBINER_HVECTOR            2315
#define MPI_COMBINER_INDEXED            2316
#define MPI_COMBINER_HINDEXED           2317
#define MPI_COMBINER_STRUCT             2318

/* for info */
typedef struct MPIR_Info *MPI_Info;
# define MPI_INFO_NULL         ((MPI_Info) 0)
# define MPI_MAX_INFO_KEY       255
# define MPI_MAX_INFO_VAL      1024

/* for subarray and darray constructors */
#define MPI_ORDER_C             56
#define MPI_ORDER_FORTRAN       57
#define MPI_DISTRIBUTE_BLOCK    121
#define MPI_DISTRIBUTE_CYCLIC   122
#define MPI_DISTRIBUTE_NONE     123
#define MPI_DISTRIBUTE_DFLT_DARG -49767

/* mpidefs.h includes configuration-specific information, such as the 
   type of MPI_Aint or MPI_Fint, also mpio.h, if it was built */
#include "mpidefs.h"

/* Handle conversion types/functions */

/* Programs that need to convert types used in MPICH should use these */
#define MPI_Comm_c2f(comm) (MPI_Fint)(comm)
#define MPI_Comm_f2c(comm) (MPI_Comm)(comm)
#define MPI_Type_c2f(datatype) (MPI_Fint)(datatype)
#define MPI_Type_f2c(datatype) (MPI_Datatype)(datatype)
#define MPI_Group_c2f(group) (MPI_Fint)(group)
#define MPI_Group_f2c(group) (MPI_Group)(group)
/* MPI_Request_c2f is a routine in src/misc2 */
#define MPI_Request_f2c(request) (MPI_Request)MPIR_ToPointer(request)
#define MPI_Op_c2f(op) (MPI_Fint)(op)
#define MPI_Op_f2c(op) (MPI_Op)(op)
#define MPI_Errhandler_c2f(errhandler) (MPI_Fint)(errhandler)
#define MPI_Errhandler_f2c(errhandler) (MPI_Errhandler)(errhandler)

/* For new MPI-2 types */
#define MPI_Win_c2f(win)   (MPI_Fint)(win)
#define MPI_Win_f2c(win)   (MPI_Win)(win)

#define MPI_STATUS_IGNORE (MPI_Status *)0
#define MPI_STATUSES_IGNORE (MPI_Status *)0

/********************** MPI-2 FEATURES END HERE ***************************/

/* MPI's error classes */
#include "mpi_errno.h"

/* Bindings of the MPI routines */
#include "binding.h"

#if defined(__cplusplus)
}
#endif

#endif

