/*
 *  $Id: mpiimpl.h,v 1.20 1995/09/18 21:09:17 gropp Exp $
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
#include "dmpiatom.h"
#include "mpi_bc.h"
#include "dmpi.h"
#include "mpir.h"
#include "mpi_ad.h"
#include "mpid.h"

/* This handles the case of sizeof(int) < sizeof(void*). */
#ifdef INT_LT_POINTER
extern void *MPIR_ToPointer();
extern int  MPIR_FromPointer();
extern void MPIR_RmPointer();
#endif
/* 
   mpiprof contains the renamings for the profiling interface.  For it to
   be loaded, the symbol MPI_BUILD_PROFILING must be defined.  This 
   will be used in the makefiles for the construction of the MPI library
   routines to build the profiling interface
 */
#include "mpiprof.h"

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
#ifdef __STDC__
/* coll */

/* context */
int MPIR_Attr_copy_node( MPI_Comm, MPI_Comm, MPIR_HBT_node * );
int MPIR_Attr_copy_subtree ( MPI_Comm, MPI_Comm, MPIR_HBT *, MPIR_HBT_node * );
int MPIR_Attr_copy( MPI_Comm, MPI_Comm );
int MPIR_Attr_free_node( MPI_Comm, MPIR_HBT_node * );
int MPIR_Attr_free_subtree( MPI_Comm, MPIR_HBT_node * );
int MPIR_Attr_free_tree( MPI_Comm );
int MPIR_Attr_dup_tree( MPI_Comm, MPI_Comm );
int MPIR_Attr_create_tree( MPI_Comm );
int MPIR_Keyval_create ( MPI_Copy_function   *, MPI_Delete_function *, 
			int *, void *, int );
int MPIR_Comm_make_coll( MPI_Comm, MPIR_COMM_TYPE );
int MPIR_Comm_N2_prev( MPI_Comm, int * );
int MPIR_Dump_comm( MPI_Comm );
int MPIR_Intercomm_high( MPI_Comm, int * );
MPI_Group MPIR_CreateGroup( int );
void MPIR_FreeGroup( MPI_Group );
void MPIR_SetToIdentity( MPI_Group );
#ifndef MPIR_Group_dup
/* If it's not a macro, then it must be a function */
int MPIR_Group_dup( MPI_Group, MPI_Group * );
#endif
int MPIR_Dump_group( MPI_Group );
int MPIR_Dump_ranks( int, int * );
int MPIR_Dump_ranges( int, int * );
int MPIR_Powers_of_2( int, int *, int * );
int MPIR_Group_N2_prev( MPI_Group, int * );
int MPIR_Sort_split_table( int, int, int *, int *, int * );
int MPIR_Context_alloc( MPI_Comm, int, MPIR_CONTEXT * );
int MPIR_Context_dealloc( MPI_Comm, int, MPIR_CONTEXT );
int MPIR_dup_fn ( MPI_Comm *, int *, void *, void *, void *, int * );

/* pt2pt */
int MPIR_Pack ( MPI_Comm, void *, int, MPI_Datatype, void *, int, int *);
int MPIR_Pack_size ( int, MPI_Datatype, MPI_Comm, int *);
int MPIR_Unpack( MPI_Comm, void *, int, int, MPI_Datatype, int, 
		 void *, int *, int * );
int MPIR_UnPackMessage( char *, int, MPI_Datatype, int, MPI_Request, int * );
int MPIR_Type_free( MPI_Datatype * );
void MPIR_Type_free_struct( MPI_Datatype );
MPI_Datatype MPIR_Type_dup( MPI_Datatype );
int MPIR_Type_permanent( MPI_Datatype );
void MPIR_Type_get_limits( MPI_Datatype, MPI_Aint *, MPI_Aint *);
int MPIR_Send_init( void *, int, MPI_Datatype, int, int, MPI_Comm, 
		     MPI_Request, MPIR_Mode, int );

/* dmpi */
void MPIR_BSwap_N_inplace( unsigned char *, int, int );
void MPIR_BSwap_short_inplace( unsigned char *, int );
void MPIR_BSwap_int_inplace( unsigned char *, int );
void MPIR_BSwap_long_inplace( unsigned char *, int );
void MPIR_BSwap_float_inplace( unsigned char *, int );
void MPIR_BSwap_double_inplace( unsigned char *, int );
void MPIR_BSwap_long_double_inplace( unsigned char *, int );
void MPIR_BSwap_N_copy( unsigned char *, unsigned char *, int, int );
void MPIR_BSwap_short_copy( unsigned char *, unsigned char *, int );
void MPIR_BSwap_int_copy( unsigned char *, unsigned char *, int );
void MPIR_BSwap_long_copy( unsigned char *, unsigned char *, int );
void MPIR_BSwap_float_copy( unsigned char *, unsigned char *, int );
void MPIR_BSwap_double_copy( unsigned char *, unsigned char *, int );
void MPIR_BSwap_long_double_copy( unsigned char *, unsigned char *, int );

int MPIR_Type_swap_copy( unsigned char *, unsigned char *, MPI_Datatype, 
			  int, void * );
void MPIR_Type_swap_inplace( unsigned char *, MPI_Datatype, int );
int MPIR_Type_XDR_encode( unsigned char *, unsigned char *, MPI_Datatype, 
			  int, void * );
int MPIR_Type_XDR_decode( unsigned char *, int, MPI_Datatype, int, 
			  unsigned char *, int *, int *, void * );
int MPIR_Type_convert_copy( MPI_Comm, void *, int, void *, MPI_Datatype, 
			   int, int, int * );
int MPIR_Comm_needs_conversion( MPI_Comm );
int MPIR_Dest_needs_converstion( int );
void MPIR_Pack_Hvector( MPI_Comm, char *, int, MPI_Datatype, int, char * );
void MPIR_UnPack_Hvector( char *, int, MPI_Datatype, int, char * );
int MPIR_HvectorLen( int, MPI_Datatype );
int MPIR_PackMessage( char *, int, MPI_Datatype, int, MPI_Request );
int MPIR_EndPackMessage( MPI_Request );
int MPIR_SetupUnPackMessage( char *, int, MPI_Datatype, int, MPI_Request );
int MPIR_Receive_setup( MPI_Request * );
int MPIR_Send_setup( MPI_Request * );
int MPIR_SendBufferFree( MPI_Request );

void DMPI_msg_arrived( int, int, MPIR_CONTEXT, MPIR_RHANDLE **, int * );
void DMPI_free_unexpected( MPIR_RHANDLE      * );

/* env */
MPI_Datatype MPIR_Init_basic_datatype( MPIR_NODETYPE, int );

/* topol */
int MPIR_Topology_copy_fn( MPI_Comm *, int *, void *, void *, void *, int * );
int MPIR_Topology_delete_fn( MPI_Comm *, int *, void *, void * );

/* util */
/*
void *MPIR_SBalloc( void * );
void MPIR_SBfree( void *, void * );
 */
void MPIR_dump_rhandle( MPIR_RHANDLE );
void MPIR_dump_shandle( MPIR_SHANDLE );
void MPIR_dump_queue( MPIR_QHDR * );
int MPIR_enqueue( MPIR_QHDR *, MPIR_COMMON *, MPIR_QEL_TYPE );
int MPIR_dequeue( MPIR_QHDR *, void * );
int MPIR_search_posted_queue( int, int, MPIR_CONTEXT, int *, int, 
			     MPIR_RHANDLE ** );
int MPIR_search_unexpected_queue( int, int, MPIR_CONTEXT, int *, int, 
				 MPIR_RHANDLE ** );
int MPIR_dump_dte( MPI_Datatype, int );
int MPIR_flatten_dte( MPI_Datatype, MPIR_FDTEL **, MPIR_FDTEL ***, int * );
int MPIR_dump_flat_dte( MPIR_FDTEL * );
int MPIR_Tab( int );

int  MPIR_SetBuffer( void *, int );
void MPIR_FreeBuffer( void **, int *);
void MPIR_PrepareBuffer( MPIR_SHANDLE * );
int MPIR_GetBuffer( int, MPI_Request, void *, int, MPI_Datatype, void ** );
void MPIR_BufferFreeReq( MPIR_SHANDLE * );

#else
extern MPI_Group MPIR_CreateGroup();
extern MPI_Datatype MPIR_Type_dup();
#endif

#endif



