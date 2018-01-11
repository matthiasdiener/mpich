/*
 *  $Id: dmpiatom.h,v 1.40 1995/12/21 22:25:06 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* Atomic types */

#ifndef _DMPIATOM_INCLUDE
#define _DMPIATOM_INCLUDE

/* SGI change... */
#include "mpi.h"

/*****************************************************************************
*                           MPI ATOMIC DATA STRUCTURES                       *
*****************************************************************************/

/*****************************************************************************
*  We place "cookies" into the data structures to improve error detection and*
*  reporting of invalid objects.  In order to make this flexible, the        *
*  cookies are defined as macros.                                            *
*  If MPIR_HAS_COOKIES is not defined, then the "cookie" fields are not      *
*  set or tested                                                             *
*****************************************************************************/

#define MPIR_HAS_COOKIES
#ifdef MPIR_HAS_COOKIES
#define MPIR_COOKIE unsigned long cookie;
#define MPIR_SET_COOKIE(obj,value) (obj)->cookie = (value);
#else
#define MPIR_COOKIE
#define MPIR_SET_COOKIE(obj,value)
#endif
/****************************************************************************/

/* Ensure that this is at least 32 bits */
typedef unsigned long MPIR_uint32;

typedef struct _MPIR_HBT_node {
  MPIR_COOKIE                    /* Cookie to help detect valid items */
  void *value;
  int   keyval;
  short balance;
  struct _MPIR_HBT_node *left;
  struct _MPIR_HBT_node *right;
} MPIR_HBT_node;
#define MPIR_HBT_NODE_COOKIE 0x03b740de

typedef struct _MPIR_HBT {
  MPIR_COOKIE                    /* Cookie to help detect valid items */
  unsigned int   height;
  int            ref_count;
  MPIR_HBT_node *root;
} MPIR_HBT;
#define MPIR_HBT_COOKIE 0x03b7c007

typedef struct  {
  MPIR_COOKIE                    /* Cookie to help detect valid items */
  int  (*copy_fn)();
  int  (*delete_fn)();
  void  *extra_state;
  int    FortranCalling;        /* Used to indicate whether Fortran or
				   C calling conventions are used for
				   copy_fn (attribute_in is passed by 
				   value in C, must be passed by reference
				   in Fortran); the underlying code
				   must also understand what a 
				   Fortran logical looks like */
  int    ref_count;
  int    permanent;             /* Used to mark the permanent attributes of
				   MPI_COMM_WORLD */
} MPIR_Attr_key;
#define MPIR_ATTR_COOKIE 0xa774c003

extern int MPIR_TOPOLOGY_KEYVAL;  /* Keyval for topology information */

#define MPIR_GRAPH_TOPOL_COOKIE 0x0101beaf
typedef struct {
  int type;
  MPIR_COOKIE
  int nnodes;
  int nedges;
  int *index;
  int *edges;
} MPIR_GRAPH_TOPOLOGY;

#define MPIR_CART_TOPOL_COOKIE 0x0102beaf
typedef struct {
  int type;
  MPIR_COOKIE
  int nnodes;
  int ndims;
  int *dims;
  int *periods;
  int *position;
} MPIR_CART_TOPOLOGY;

typedef union {
  int type;
  MPIR_GRAPH_TOPOLOGY  graph;
  MPIR_CART_TOPOLOGY   cart;
} MPIR_TOPOLOGY;

#define MPIR_GROUP_COOKIE 0xea01beaf
struct MPIR_GROUP {
    MPIR_COOKIE             /* Cookie to help detect valid item */
    int np;			        /* Number of processes in group */
    int local_rank;         /* My rank in the group (if I belong) */
    int ref_count;          /* Number of references to this group */
    int N2_next;            /* Next power of 2 from np */
    int N2_prev;            /* Previous power of 2 from np */
    int permanent;          /* Permanent group */
    int *lrank_to_grank;    /* Mapping from local to "global" ranks */
    int *set_mark;          /* Used for set marking/manipulation on groups */
};

/* 
   Error handlers must survive being deleted and set to MPI_ERRHANDLER_NULL,
   the reference count is for knowing how many communicators still have this
   error handler active 
 */
struct MPIR_Errhandler {
    MPIR_COOKIE                    /* Cookie to help detect valid items */
    MPI_Handler_function *routine;
    int                  ref_count;
    };
#define MPIR_ERRHANDLER_COOKIE 0xe443a2dd


typedef unsigned long MPIR_CONTEXT;
#define  MPIR_CONTEXT_TYPE MPI_UNSIGNED_LONG

#define  MPIR_WORLD_PT2PT_CONTEXT 0
#define  MPIR_WORLD_COLL_CONTEXT  1
#define  MPIR_SELF_PT2PT_CONTEXT  2
#define  MPIR_SELF_COLL_CONTEXT   3
#define  MPIR_FIRST_FREE_CONTEXT  4

typedef enum { MPIR_INTRA=1, MPIR_INTER } MPIR_COMM_TYPE;

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif

#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

/*
 * Collective operations (this allows choosing either an implementation
 * in terms of point-to-point, or a special version exploiting special
 * facilities, in a communicator by communicator fashion).
 */
typedef struct MPIR_COLLOPS {
    int (*Barrier) ANSI_ARGS((MPI_Comm comm ));
    int (*Bcast) ANSI_ARGS((void* buffer, int count, MPI_Datatype datatype, int root, 
		 MPI_Comm comm ));
    int (*Gather) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		  void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		  int root, MPI_Comm comm)); 
    int (*Gatherv) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		   void* recvbuf, int *recvcounts, int *displs, 
		   MPI_Datatype recvtype, int root, MPI_Comm comm)); 
    int (*Scatter) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		   void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		   int root, MPI_Comm comm));
    int (*Scatterv) ANSI_ARGS((void* sendbuf, int *sendcounts, int *displs, 
		    MPI_Datatype sendtype, void* recvbuf, int recvcount, 
		    MPI_Datatype recvtype, int root, MPI_Comm comm));
    int (*Allgather) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		     void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		     MPI_Comm comm));
    int (*Allgatherv) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		      void* recvbuf, int *recvcounts, int *displs, 
		      MPI_Datatype recvtype, MPI_Comm comm));
    int (*Alltoall) ANSI_ARGS((void* sendbuf, int sendcount, MPI_Datatype sendtype, 
		    void* recvbuf, int recvcount, MPI_Datatype recvtype, 
		    MPI_Comm comm));
    int (*Alltoallv) ANSI_ARGS((void* sendbuf, int *sendcounts, int *sdispls, 
		     MPI_Datatype sendtype, void* recvbuf, int *recvcounts, 
		     int *rdispls, MPI_Datatype recvtype, MPI_Comm comm));
    int (*Reduce) ANSI_ARGS((void* sendbuf, void* recvbuf, int count, 
		  MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm));
    int (*Allreduce) ANSI_ARGS((void* sendbuf, void* recvbuf, int count, 
		     MPI_Datatype datatype, MPI_Op op, MPI_Comm comm));
    int (*Reduce_scatter) ANSI_ARGS((void* sendbuf, void* recvbuf, int *recvcounts, 
			  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm));
    int (*Scan) ANSI_ARGS((void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, 
		MPI_Op op, MPI_Comm comm ));
    int ref_count;     /* So we can share it */
} MPIR_COLLOPS;

/*
   The local_rank field is used to reduce unnecessary memory references
   when doing send/receives.  It must equal local_group->local_rank.

   lrank_to_grank is group->lrank_to_grank; this is also used to 
   reduce memory refs.  (it is IDENTICAL, not just a copy; the "group"
   owns the array.)

   These have been ordered so that the most common elements are 
   near the top, in hopes of improving cache utilization.

   For a normal intra-communicator the group and local_group are identical
   The group differs from the local_group only in an inter-communicator
 */
#define MPIR_COMM_COOKIE 0xea02beaf
struct MPIR_COMMUNICATOR {
    MPIR_COOKIE                   /* Cookie to help detect valid item */
    int           np;             /* size of (remote) group */
    int           local_rank;     /* rank in local_group of this process */
    int           *lrank_to_grank;/* mapping for group */
    MPIR_CONTEXT   send_context;  /* context to send messages */
    MPIR_CONTEXT   recv_context;  /* context to recv messages */
    void          *ADIctx;        /* Context (if any) for abstract device */

    MPIR_COMM_TYPE comm_type;	  /* inter or intra */
    MPI_Group     group;	  /* group associated with communicator */
    MPI_Group     local_group;    /* local group */
    MPI_Comm      comm_coll;      /* communicator for collective ops */

    int            ref_count;     /* number of references to communicator */
    void          *comm_cache;	  /* Hook for communicator cache */
    MPIR_HBT      *attr_cache;    /* Hook for attribute cache */
    struct MPIR_Errhandler *error_handler;  /* Error handler structure */
    int            permanent;      /* Is this a permanent object? */
    void          *mutex;          /* Local for threaded versions */

    /*** BEGIN HETEROGENEOUS ONLY ***/
    int           is_homogeneous; /* True if communicator's members
				     are homogeneous in data representation
				     (not used yet) */
    int           msgrep;         /* Message representation form for 
				     this communicator.  This is either
				     0 == MPIR_MSGREP_RECEIVER (all the same)
				     or != 0 (sender or XDR) */
    /* Note that point-to-point information on message representations
       is managed directly by the device and is not duplicated in the
       communicator */
    /*** END HETEROGENEOUS ONLY ***/

    /* This would be a good place to cache data that is often needed, 
       such as the rank of the process that holds this communicator in
       the local group 
     */

    /* Needed for ADI-supported collective operations. These are ignored
       if the ADI doesn't support the operation; otherwise, they are
       null if the operation is not supported on this communicator and 
       non-null otherwise.  */
    void          *ADIBarrier;
    void          *ADIReduce;
    void          *ADIScan;
    void          *ADIBcast;
    void          *ADICollect;

    void          *adiCollCtx;
    MPIR_COLLOPS  *collops;
};

typedef enum {
  MPIR_NO = 0, MPIR_YES
} MPIR_BOOL;

/* 
   MPIR_FORT_INT is a special integer type for systems where sizeof(int) !=
   sizeof(Fortran REAL).  See the Fortran standard for why this is required.

   MPIR_LOGICAL is required for Fortran, since while the length of a 
   Fortran LOGICAL == Fortran INTEGER, the true/false values are 
   unspecified 
 */
typedef enum {
    MPIR_INT, MPIR_FLOAT, MPIR_DOUBLE, MPIR_COMPLEX, MPIR_LONG, MPIR_SHORT,
    MPIR_CHAR, MPIR_BYTE, MPIR_UCHAR, MPIR_USHORT, MPIR_ULONG, MPIR_UINT,
    MPIR_CONTIG, MPIR_VECTOR, MPIR_HVECTOR, MPIR_INDEXED,
    MPIR_HINDEXED, MPIR_STRUCT, MPIR_DOUBLE_COMPLEX, MPIR_PACKED, 
	MPIR_UB, MPIR_LB, MPIR_LONGDOUBLE, MPIR_LONGLONGINT, 
    MPIR_LOGICAL, MPIR_FORT_INT 
} MPIR_NODETYPE;

/* These are used by some of the datatype routines */
#ifndef MPIR_TRUE
#define MPIR_TRUE  1
#define MPIR_FALSE 0
#endif

#define MPIR_UNMARKED 0
#define MPIR_MARKED   1

/* #include "mpi_bc.h" */
#endif
