/*
 *  $Id: mpiimpl.h,v 1.28 1996/07/17 18:06:07 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

/* user include file for MPI programs */

#ifndef _MPIRIMPL_INCLUDE
#define _MPIRIMPL_INCLUDE

/* mpi.h includes most of the definitions (all of the user-visible ones) */
#include "mpi.h"

/* The rest of these contain the details of the structures that are not 
   user-visible */ 
#include "patchlevel.h"

/* For debugging, use PRINTF, FPRINTF, SPRINTF.  This allows us to 
   grep for printf to find stray error messages that should be handled with
   the error message facility (errorstring/errmsg)
   */
#define PRINTF printf
#define FPRINTF fprintf
#define SPRINTF sprintf

#ifdef MPI_ADI2
/* The device knows a lot about communicators, requests, etc. */
#include "mpid.h"
#include "sendq.h"
/* Anything the device does NOT know about is included here */
/* FROM MPIR.H */
/* memory management for fixed-size blocks */
extern void *MPIR_errhandlers;  /* sbcnst Error handlers */

/* MPIR_F_MPI_BOTTOM is the address of the Fortran MPI_BOTTOM value */
extern void *MPIR_F_MPI_BOTTOM;

/* MPIR_F_PTR checks for the Fortran MPI_BOTTOM and provides the value 
   MPI_BOTTOM if found 
   See src/pt2pt/addressf.c for why MPIR_F_PTR(a) is just (a)
*/
/*  #define MPIR_F_PTR(a) (((a)==(MPIR_F_MPI_BOTTOM))?MPI_BOTTOM:a) */
#define MPIR_F_PTR(a) (a)

/* End of FROM MPIR.H */
/* FROM MPI_BC.H */
/* Value of tag in status for a cancelled message */
/* #define MPIR_MSG_CANCELLED (-3) */

/* This is the only global state in MPI */
extern int MPIR_Has_been_initialized;
/* End of FROM MPI_BC.H */

/* FROM DMPIATOM.H, used in group_diff, group_excl, group_inter,
   group_rexcl, group_union */
/* These are used in the Group manipulation routines */
#define MPIR_UNMARKED 0
#define MPIR_MARKED   1
/* End of FROM DMPIATOM.H */

/* Old-style thread macros */
#define MPID_THREAD_LOCK(a,b) MPID_THREAD_DS_LOCK(b)
#define MPID_THREAD_UNLOCK(a,b) MPID_THREAD_DS_UNLOCK(b)
#define MPID_THREAD_LOCK_INIT(a,b) MPID_THREAD_DS_LOCK_INIT(b)
#define MPID_THREAD_LOCK_FINISH(a,b) MPID_THREAD_DS_LOCK_FREE(b)

/* 
   Some code prototypes want FILE * .  We should limit these to
   the ones that need them. 
 */
#include <stdio.h>
#else
#include "dmpiatom.h"
#include "mpi_bc.h"
#include "dmpi.h"
#include "mpir.h"
#include "mpi_ad.h"
#include "mpid.h"
#endif

/* This handles the case of sizeof(int) < sizeof(void*). */
#if defined(INT_LT_POINTER) && !defined(MPI_ADI2)
extern void *MPIR_ToPointer ANSI_ARGS(());
extern int  MPIR_FromPointer ANSI_ARGS(());
extern void MPIR_RmPointer   ANSI_ARGS(());
#endif
/* 
   mpiprof contains the renamings for the profiling interface.  For it to
   be loaded, the symbol MPI_BUILD_PROFILING must be defined.  This 
   will be used in the makefiles for the construction of the MPI library
   routines to build the profiling interface
 */
#include "mpiprof.h"

/* These are special macros for interfacing with Fortran */
#ifdef _CRAY
#include <fortran.h>
#endif

/* 
   The wrapper generator now makes "universal" wrappers.
 */
#if (defined(MPI_rs6000) || defined(MPI_NO_FORTRAN_USCORE))
/* Nothing to do */
#elif defined(MPI_cray) || defined(MPI_ncube)
/* Fortran is uppercase, no trailing underscore */
#ifndef FORTRANCAPS
#define FORTRANCAPS
#endif

#else
/* Most common Unix case is FORTRANUNDERSCORE, so choose that unless
   FORTRANNOUNDERSCORE explicitly set */
#if !defined(FORTRANUNDERSCORE) && !defined(FORTRANNOUNDERSCORE)
#define FORTRANUNDERSCORE
#endif

#endif

#if 1

/* Provide a variety of macroed versions of communicator enquiry
 * functions for use inside the implementation. This should remove
 * a fair amount of overhead, given that we had already checked the 
 * communicator on entering the outermost MPI function.
 */
#define MPIR_Group_dup(s,r) ((*(r) = (s)), \
			     (((s) == MPI_GROUP_NULL) || (s)->ref_count++), \
			     MPI_SUCCESS)
#define MPIR_Comm_size(comm, size) ((*(size) = (comm)->local_group->np),MPI_SUCCESS)
#define MPIR_Comm_rank(comm, size) ((*(size) = (comm)->local_rank),MPI_SUCCESS)
#else
/* If you are really paranoid you may want the additional checking provided
 * by the externally callable routines. If so change the #if above, and
 * you'll get back to using the more expensive versions.
 */
#define MPIR_Comm_size MPI_Comm_size
#define MPIR_Comm_rank MPI_Comm_rank
#endif

/* Here are bindings for some of the INTERNAL MPICH routines.  These are
   used to help us ensure that the code has no obvious bugs (i.e., mismatched
   args) 
 */
/* coll */
extern void MPIR_MAXF ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_MINF ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_SUM ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_PROD ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_LAND ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_BAND ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_LOR ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_BOR ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_LXOR ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_BXOR ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_MAXLOC ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );
extern void MPIR_MINLOC ANSI_ARGS( ( void *, void *, int *, MPI_Datatype * ) );

/* context */
#ifdef FOO
int MPIR_Attr_copy_node ANSI_ARGS( ( MPI_Comm, MPI_Comm, MPIR_HBT_node * ) );
int MPIR_Attr_copy_subtree  ANSI_ARGS( ( MPI_Comm, MPI_Comm, MPIR_HBT *, 
					 MPIR_HBT_node * ) );
int MPIR_Attr_free_node ANSI_ARGS( ( MPI_Comm, MPIR_HBT_node * ) );
int MPIR_Attr_free_subtree ANSI_ARGS( ( MPI_Comm, MPIR_HBT_node * ) );
#endif
void MPIR_Attr_make_perm ANSI_ARGS(( int ));
int MPIR_Attr_copy ANSI_ARGS( ( MPI_Comm, MPI_Comm ) );
int MPIR_Attr_free_tree ANSI_ARGS( ( MPI_Comm ) );
int MPIR_Attr_dup_tree ANSI_ARGS( ( MPI_Comm, MPI_Comm ) );
int MPIR_Attr_create_tree ANSI_ARGS( ( MPI_Comm ) );
int MPIR_Keyval_create ANSI_ARGS( ( MPI_Copy_function *, 
				    MPI_Delete_function *, 
				    int *, void *, int ) );
int MPIR_Comm_make_coll ANSI_ARGS( ( MPI_Comm, MPIR_COMM_TYPE ) );
int MPIR_Comm_N2_prev ANSI_ARGS( ( MPI_Comm, int * ) );
int MPIR_Dump_comm ANSI_ARGS( ( MPI_Comm ) );
int MPIR_Intercomm_high ANSI_ARGS( ( MPI_Comm, int * ) );
MPI_Group MPIR_CreateGroup ANSI_ARGS( ( int ) );
void MPIR_FreeGroup ANSI_ARGS( ( MPI_Group ) );
void MPIR_SetToIdentity ANSI_ARGS( ( MPI_Group ) );
void MPIR_Comm_remember ANSI_ARGS( ( MPI_Comm ) );
void MPIR_Comm_forget   ANSI_ARGS( ( MPI_Comm ) );
int  MPIR_Comm_set_name ANSI_ARGS( ( MPI_Comm, char * ) );
int  MPIR_Comm_get_name ANSI_ARGS( ( MPI_Comm, char **) );
#ifndef MPIR_Group_dup
/* If it's not a macro, then it must be a function */
int MPIR_Group_dup ANSI_ARGS( ( MPI_Group, MPI_Group * ) );
#endif
int MPIR_Dump_group ANSI_ARGS( ( MPI_Group ) );
int MPIR_Dump_ranks ANSI_ARGS( ( int, int * ) );
int MPIR_Dump_ranges ANSI_ARGS( ( int, int * ) );
int MPIR_Powers_of_2 ANSI_ARGS( ( int, int *, int * ) );
int MPIR_Group_N2_prev ANSI_ARGS( ( MPI_Group, int * ) );
int MPIR_Sort_split_table ANSI_ARGS( ( int, int, int *, int *, int * ) );
int MPIR_Context_alloc ANSI_ARGS( ( MPI_Comm, int, MPIR_CONTEXT * ) );
int MPIR_Context_dealloc ANSI_ARGS( ( MPI_Comm, int, MPIR_CONTEXT ) );
int MPIR_dup_fn  ANSI_ARGS( ( MPI_Comm, int, void *, void *, void *, int * ) );
int MPIR_Comm_init  ANSI_ARGS( ( MPI_Comm, MPI_Comm, MPIR_COMM_TYPE ) );

/* pt2pt */
#ifndef MPI_ADI2
#include "mpipt2pt.h"
#endif

/* dmpi */
#ifndef MPI_ADI2
#include "mpidmpi.h"
#endif

/* env */
int MPIR_Init ANSI_ARGS( ( int *, char *** ) );
void MPIR_Op_setup ANSI_ARGS( ( MPI_User_function *, int, int, MPI_Op ) );
void MPIR_Breakpoint ANSI_ARGS(( void ));
int MPIR_GetErrorMessage ANSI_ARGS(( int, char *, char ** ));
void MPIR_Init_dtes ANSI_ARGS(( void ));
void MPIR_Free_dtes ANSI_ARGS(( void ));

/* topol */
void MPIR_Topology_Init ANSI_ARGS((void));
void MPIR_Topology_Free ANSI_ARGS((void));

/* util */
/*
void *MPIR_SBalloc ANSI_ARGS( ( void * ) );
void MPIR_SBfree ANSI_ARGS( ( void *, void * ) );
 */
#ifndef MPI_ADI2
void MPIR_dump_rhandle ANSI_ARGS( ( MPIR_RHANDLE ) );
void MPIR_dump_shandle ANSI_ARGS( ( MPIR_SHANDLE ) );
void MPIR_dump_queue ANSI_ARGS( ( MPIR_QHDR * ) );
int MPIR_enqueue ANSI_ARGS( ( MPIR_QHDR *, MPIR_COMMON *, MPIR_QEL_TYPE ) );
int MPIR_dequeue ANSI_ARGS( ( MPIR_QHDR *, void * ) );
int MPIR_search_posted_queue ANSI_ARGS( ( int, int, MPIR_CONTEXT, int *, int, 
			     MPIR_RHANDLE ** ) );
int MPIR_search_unexpected_queue ANSI_ARGS( ( int, int, MPIR_CONTEXT, int *, 
					      int, MPIR_RHANDLE ** ) );
int MPIR_flatten_dte ANSI_ARGS( ( MPI_Datatype, MPIR_FDTEL **, MPIR_FDTEL ***, int * ) );
int MPIR_dump_flat_dte ANSI_ARGS( ( MPIR_FDTEL * ) );
int MPIR_Tab ANSI_ARGS( ( int ) );
void MPIR_ArgSqueeze ANSI_ARGS( ( int *, char ** ) );
#else
int MPIR_dump_dte ANSI_ARGS(( MPI_Datatype, int ));
#endif

#ifdef MPI_ADI2
int MPIR_BsendInitBuffer ANSI_ARGS( ( void *, int ) );
void MPIR_BsendRelease ANSI_ARGS( ( void **, int * ) );
int MPIR_BsendBufferPrint ANSI_ARGS( ( void ) );
int MPIR_BsendAlloc ANSI_ARGS( ( int, MPI_Request, void ** ) );
void MPIR_BsendCopyData ANSI_ARGS(( MPIR_SHANDLE *, MPI_Comm, void *, int, 
				    MPI_Datatype, void **, int * ) );
void MPIR_BsendPersistent ANSI_ARGS( ( MPI_Request, int ) );
void MPIR_BsendFreeReq ANSI_ARGS( ( MPIR_SHANDLE * ) );
void MPIR_IbsendDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				     int, int, int, int, MPI_Request, int * ));
#else
int  MPIR_SetBuffer ANSI_ARGS( ( void *, int ) );
void MPIR_FreeBuffer ANSI_ARGS( ( void **, int *) );
void MPIR_PrepareBuffer ANSI_ARGS( ( MPIR_SHANDLE * ) );
int MPIR_GetBuffer ANSI_ARGS( ( int, MPI_Request, void *, int, MPI_Datatype, 
				void ** ) );
void MPIR_BufferFreeReq ANSI_ARGS( ( MPIR_SHANDLE * ) );
void MPIR_BsendPersistent ANSI_ARGS( ( MPI_Request, int ) );
int MPIR_DoBufferSend ANSI_ARGS( ( MPIR_SHANDLE * ) );
int MPIR_BufferPrint ANSI_ARGS( ( void ) );
#endif

void MPIR_HBT_Free ANSI_ARGS((void));
void MPIR_HBT_Init ANSI_ARGS((void));
#ifdef FOO
int MPIR_HBT_new_tree ANSI_ARGS( ( MPIR_HBT ** ) );
int MPIR_HBT_new_node ANSI_ARGS( ( int, void *, MPIR_HBT_node ** ) );
int MPIR_HBT_free_node ANSI_ARGS( ( MPIR_HBT_node * ) );
int MPIR_HBT_free_subtree ANSI_ARGS( ( MPIR_HBT_node * ) );
int MPIR_HBT_free_tree ANSI_ARGS( ( MPIR_HBT * ) );
int MPIR_HBT_lookup ANSI_ARGS( ( MPIR_HBT *, int, MPIR_HBT_node ** ) );
int MPIR_HBT_insert ANSI_ARGS( ( MPIR_HBT *, MPIR_HBT_node * ) );
int MPIR_HBT_delete ANSI_ARGS( ( MPIR_HBT *, int, MPIR_HBT_node ** ) );
#endif

/* We are switching to a single form for the prototypes, using the
   ANSI_ARGS forms */
void *MPIR_ToPointer ANSI_ARGS(( int ));
int  MPIR_FromPointer ANSI_ARGS((void *));
void MPIR_RmPointer ANSI_ARGS(( int ));
int MPIR_UsePointer ANSI_ARGS((FILE *));
void MPIR_PointerPerm ANSI_ARGS(( int ));
void MPIR_Comm_collops_init ANSI_ARGS(( MPI_Comm, MPIR_COMM_TYPE ));


#endif



