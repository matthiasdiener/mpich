/*
 *  $Id: mpiuser.h,v 1.3 1994/06/07 21:30:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* %W% %G% */

/* This should be the "user" mpi.h file, containing
    error values
    prototypes
    enums
 */    
#ifndef __MPI_USER
#define __MPI_USER

#include "mpi_errno.h"
#define MPI_BOTTOM ((void *)0)
#define MPI_PROC_NULL (-1)
#define MPI_ANY_SOURCE (-2)
#define MPI_ANY_TAG (-1)
#define MPI_UNDEFINED
#define MPI_UB
#define MPI_LB

typedef struct MPIR_DATATYPE *MPI_Datatype;
extern MPI_Datatype MPI_CHAR, MPI_SHORT, MPI_INT, MPI_LONG, MPI_UNSIGNED_CHAR,
       MPI_UNSIGNED_SHORT, MPI_UNSIGNED, MPI_UNSIGNED_LONG, MPI_FLOAT, 
       MPI_DOUBLE, MPI_LONG_DOUBLE, MPI_BYTE, MPI_PACKED;
extern MPI_Datatype MPI_FLOAT_INT, MPI_DOUBLE_INT, MPI_LONG_INT, MPI_2INT;

typedef struct MPIR_COMMUNICATOR *MPI_Comm;
extern MPI_Comm MPI_COMM_WORLD, MPI_COMM_SELF;

#define MPI_IDENT
#define MPI_CONGRUENT
#define MPI_SIMILAR
#define MPI_UNEQUAL

extern int MPI_TAG_UB, MPI_IO, MPI_HOST;

extern MPI_Op MPI_MAX, MPI_MIN, MPI_SUM, MPI_PROD, MPI_MAXLOC, MPI_MINLOC, 
       MPI_BAND, MPI_BOR, MPI_BXOR, MPI_LAND, MPI_LOR, MPI_LXOR;

typedef struct MPIR_GROUP *MPI_Group;

#define MPI_GROUP_NULL ((MPI_Group)0)
#define MPI_COMM_NULL  ((MPI_Comm)0)
#define MPI_DATATYPE_NULL ((MPI_Datatype)0)
#define MPI_REQUEST_NULL ((MPI_Request)0)
#define MPI_OP_NULL      ((MPI_Op)0)
#define MPI_ERRHANDLER_NULL ((MPI_Errhandler)0)

extern MPI_Group MPI_GROUP_EMPTY;

#define MPI_GRAPH 0
#define MPI_CART  1

typedef long MPI_Aint;

typedef struct { 
    int count;
    int MPI_SOURCE;
    int MPI_TAG;
    int count_in_bytes;     /* This is needed to implement MPI_Get_count
			       and MPI_Get_elements, particularly after
			       a probe */
} MPI_Status;

typedef struct MPIR_HANDLE *MPI_Request;
typedef struct MPIR_OP *MPI_Op;

#include "bindings.h"

#endif
