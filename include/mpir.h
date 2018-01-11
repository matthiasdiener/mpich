/*
 *  $Id: mpir.h,v 1.38 1994/12/21 14:40:15 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* include file for the MPIR implementation of MPI */

#include <stdio.h>

#ifndef _MPIR_INCLUDE
#define _MPIR_INCLUDE

#include "mpid.h"

/*****************************************************************************
*                           MPIR DATA STRUCTURES                             *
*****************************************************************************/


#define MPIR_DATATYPE_COOKIE 0xea31beaf
struct MPIR_DATATYPE {
    MPIR_NODETYPE dte_type; /* type of datatype element this is */
    MPIR_COOKIE             /* Cookie to help detect valid item */
    MPIR_BOOL    committed; /* whether committed or not */
    int          is_contig; /* whether entirely contiguous */
    int              basic; /* Is this a basic type */
    int          permanent; /* Is this a permanent type */
    int             ub, lb; /* upper/lower bound of type */
    int             extent; /* extent of this datatype */
    int               size; /* size of type */
    int           elements; /* number of basic elements */
    int          ref_count; /* nodes depending on this node */
    int              align; /* alignment needed for start of datatype */
    int                pad; /* padding for each element of type */
    int              *pads; /* padding for STRUCT types */
    int              count; /* replication count */
    MPI_Aint        stride; /* stride, for VECTOR and HVECTOR types */
    int           blocklen; /* blocklen, for VECTOR and HVECTOR types */
    int           *indices; /* array of indices, for (H)INDEXED */
    int         *blocklens; /* array of blocklens for (H)INDEXED */
    MPI_Datatype old_type;  /* type this type is built of, if 1 */
    MPI_Datatype *old_types;/* array of types, for STRUCT */
};

typedef enum {
    MPIR_SEND,
    MPIR_RECV
} MPIR_OPTYPE;

/* MPIR_COMMON is a subset of the handle structures that contain common
   elements.  MAKE SURE THAT ANY CHANGES IN THE HANDLES PRESERVES
   THIS ORDER! */
#define MPIR_REQUEST_COOKIE 0xe0a1beaf
typedef struct {
    MPIR_OPTYPE handle_type;
    MPIR_COOKIE                 /* Cookie to help detect valid item */
    int         contextid;
    int         partner;        /* source or destination, depending on type */
    int         tag;
    MPIR_BOOL   completed;
    MPI_Datatype datatype;
    MPI_Comm    comm;           /* The communicator this request is 
				   on. Needed for MPI_Start in
				   multiprotocol and heterogeneous
				   communication */
    int         persistent;     /* is handle persistent (created with 
				   MPI_..._init)? */
    int         active;         /* relavent only for persistent handles,
				   indicates that the handle is active */
    int         msgrep;         /* Message representation; used to indicate
				   XDR used */

    void *bufadd;       /* address of buffer */
    int buflen;         /* length of buffer at bufadd */
    int count;          /* requested count */

    char *bufpos;       /* position of next byte to be transferred */
    int  transfer_count;    /* count of bytes transferred to/from device */
    int  totallen;      /* length in local bytes of message */

    } MPIR_COMMON;

typedef struct {
    MPIR_OPTYPE handle_type;    /* send or receive */
    MPIR_COOKIE             /* Cookie to help detect valid item */
    int         contextid;  /* context id */
    int         dest;       /* destination process for message */
    int         tag;        /* tag */
    MPIR_BOOL   completed;  /* whether operation is completed or not */
    MPI_Datatype datatype;  /* basic or derived datatype */
    MPI_Comm    comm;
    int         persistent;
    int         active;
    int         msgrep;         /* Message representation; used to indicate
				   XDR used */
    void *bufadd;       /* address of buffer */
    int buflen;         /* length of buffer at bufadd */
    int count;          /* requested count */
    char *bufpos;       /* position of next byte to be transferred */
                        /* This is only used to point to a temporary buffer
			   to send out of. The buffer pointed to by this
			   is freed at the end of blocking sends and 
			   waits on non-blocking sends */
    int  transfer_count;    /* count of bytes transferred to/from device */
    int  totallen;      /* length in local bytes of message */

    MPIR_Mode mode;     /* mode: STANDARD, READY, SYNC, BUFFERED */
    int  lrank;                 /* Rank in sending group */
    void *bsend;        /* Pointer to structure managed for buffered sends */

    MPID_SHANDLE dev_shandle;   /* device's version of send handle */
} MPIR_SHANDLE;

typedef struct {
    MPIR_OPTYPE handle_type;    /* send or receive */
    MPIR_COOKIE             /* Cookie to help detect valid item */
    int         contextid;  /* context id */
    int         source;     /* source process message */
    int         tag;        /* tag */
    MPIR_BOOL   completed;  /* Whether this receive has been completed */
    MPI_Datatype datatype;  /* basic or derived datatype */
    MPI_Comm    comm;
    int         persistent;
    int         active;
    int         msgrep;         /* Message representation; used to indicate
				   XDR used */
    void *bufadd;       /* address of buffer */
    int buflen;         /* length of buffer at bufadd */
    int count;          /* requested count */
    char *bufpos;       /* position of next byte to be transferred */
    int  transfer_count;    /* count of bytes transferred to/from device */
    int  totallen;          /* length in local bytes of message */

    void *p;            /* Pointer to unexpected data */
    int  len;           /* Length of this data ? */
    int  actcount;      /* number of items actually read */

    MPID_RHANDLE dev_rhandle;   /* device's version of recv handle */
} MPIR_RHANDLE;

#define MPIR_HANDLES_DEFINED

union MPIR_HANDLE {
    MPIR_OPTYPE type;           /* send or receive */
    MPIR_COMMON  chandle;       /* common fields */
    MPIR_SHANDLE shandle;
    MPIR_RHANDLE rhandle;
};


/*
   MPIR manages queues of unexpected messages and posted receives.
   Originally, these were arranged as general queue elements, but 
   they have been modified to meet the needs of the MPIR implementation.

   The current implementation does not make use of the delivery-order
   fields.

   Because of the importance of this queue, we've added explicit support
   for MPI.  This is indicated by the fields:
   context_id
   tag, tagmask
   lsrc, srcmask

   (note that the last two may depart if we split the queues by source).
   The mask fields allow us to replace

   itag == tag || tag == MPI_ANY_TAG

   with 

   (itag & tagmask) == tag

   saving us a compare and branch at the cost of a load and bitwise-and.
   Generalizations of the tagmask could provide users with additional
   functionality (for MPICH EXTENSIONS).
 */
typedef enum {
    MPIR_QSHANDLE,
    MPIR_QRHANDLE
} MPIR_QEL_TYPE;

typedef struct _MPIR_QEL {  /* queue elements */
    void *ptr;              /* queues can contain anything  */
    int  context_id, 
         tag, tagmask,
         lsrc, srcmask;
    MPIR_QEL_TYPE qel_type; /* but we may need to know what */
    struct _MPIR_QEL *prev; /* previous queue element */
    struct _MPIR_QEL *next; /* next queue element */
    struct _MPIR_QEL *deliv_next;   /* next in delivery order */
} MPIR_QEL;

typedef struct {        /* header for queues of things like handles */
    MPIR_QEL *first;        /* first queue element */
    MPIR_QEL *last;         /* last queue element */
    MPIR_QEL *deliv_first;  /* first in delivery order */
    MPIR_QEL *deliv_last;   /* last in delivery order */
    int      currlen;       /* current length */
    int      maxlen;        /* maximum since beginning of run */
} MPIR_QHDR;

/* >>>> is this still in use <<<< */
typedef struct _MPIR_FDTEL {    /* flat datatype elements */
    MPIR_NODETYPE      type;    /* one of the elementary data types */
    int                disp;    /* displacement */
    struct _MPIR_FDTEL *next;   /* pointer to next element */
} MPIR_FDTEL;

/*****************************************************************************
*                                GLOBAL DATA                                 *
*****************************************************************************/

/* memory management for fixed-size blocks */
extern void *MPIR_shandles;     /* sbcnst MPIR_SHANDLES */
extern void *MPIR_rhandles;     /* sbcnst MPIR_RHANDLES */
extern void *MPIR_errhandlers;  /* sbcnst Error handlers */
extern void *MPIR_dtes;   /* sbcnst datatype elements */
extern void *MPIR_qels;   /* sbcnst queue elements */
extern void *MPIR_fdtels; /* sbcnst flat datatype elements */
extern void *MPIR_hbts;   /* sbcnst height balanced tree roots for cacheing */
extern void *MPIR_hbt_els;/* sbcnst height balanced tree nodes for cacheing */
extern void *MPIR_topo_els;/* sbcnst topology elements */

/* queues */
extern MPIR_QHDR MPIR_posted_recvs;
extern MPIR_QHDR MPIR_unexpected_recvs;

/* MPIR routines are defined in mpiimpl.h */

/* MPIR process id (from device) */
extern int MPIR_tid;

/* Fortran logical values */
extern int MPIR_F_TRUE, MPIR_F_FALSE;
#define MPIR_TO_FLOG(a) ((a) ? MPIR_F_TRUE : MPIR_F_FALSE)
/* 
   Note on true and false.  This code is only an approximation.
   Some systems define either true or false, and allow some or ALL other
   patterns for the other.  This is just like C, where 0 is false and 
   anything not zero is true.  Modify this test as necessary for your
   system.
 */
#define MPIR_FROM_FLOG(a) ( (a) == MPIR_F_TRUE ? 1 : 0 )

/* MPIR_F_MPI_BOTTOM is the address of the Fortran MPI_BOTTOM value */
extern void *MPIR_F_MPI_BOTTOM;

/* MPIR_F_PTR checks for the Fortran MPI_BOTTOM and provides the value 
   MPI_BOTTOM if found */
#define MPIR_F_PTR(a) (((a)==(MPIR_F_MPI_BOTTOM))?MPI_BOTTOM:a)

/* Message encodings - How messages are enecoded once they
   reach the device. MPI tries to put messages into the receiver's 
   format if the conversion is easy, i.e. switching bytes.
   If the conversion could be to difficult, then the messages use
   another form of encoding. (By default XDR) */

#define MPIR_MSGREP_UNKNOWN	-1
/* Encoded in the receiver's native format */
#define MPIR_MSGREP_RECEIVER	0
/* Encoded with XDR */
#define MPIR_MSGREP_XDR		1
/* Encoded in the sender's native format */
#define MPIR_MSGREP_SENDER	2

             

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
    ( ((rank) < MPI_PROC_NULL || (rank) >= (comm)->group->np)\
           && (mpi_errno = MPI_ERR_RANK))
    /* This requires min(MPI_PROC_NULL,MPI_ANY_SOURCE)=-2 */
#define MPIR_TEST_RECV_RANK(comm,rank) \
    (((rank) < -2 || (rank) >= (comm)->group->np) && \
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
#define MPIR_TEST_IS_DATATYPE(comm,datatype) \
    ( (!(datatype) MPIR_TEST_COOKIE(datatype,MPIR_DATATYPE_COOKIE)) \
     && (mpi_errno = MPI_ERR_TYPE ))
#define MPIR_TEST_DATATYPE(comm,datatype) \
    ( ( (!(datatype) MPIR_TEST_COOKIE(datatype,MPIR_DATATYPE_COOKIE)) \
    && (mpi_errno = MPI_ERR_TYPE )) || \
  (!(datatype)->committed && \
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

#endif
