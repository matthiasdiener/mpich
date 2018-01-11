/*
 *  $Id: finalize.c,v 1.25 1994/12/11 16:51:26 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpisys.h"

extern int MPIR_Print_queues;
#define DBG(a)

/*
  Function to un-initialize topology code 
 */
extern void MPIR_Topology_finalize();

/*@
   MPI_Finalize - Terminates MPI execution environment
@*/
int MPI_Finalize()
{

    void *ADIctx;
    DBG(fprintf( stderr, "Entering system finalize\n" ); fflush(stderr);)

    /*  Dump final status of queues */
    if (MPIR_Print_queues) {
	int i, np, rank;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &np );
	for (i=0 ; i<np; i++) {
	    MPI_Barrier( MPI_COMM_WORLD );
	    if (i == rank) {
		printf("[%d]Dumping recv queue...\n", rank );
		MPIR_dump_queue( &MPIR_posted_recvs );
		fflush( stdout );
		}
	    }
	for (i=0 ; i<np; i++) {
	    MPI_Barrier( MPI_COMM_WORLD );
	    if (i == rank) {
		printf("[%d]Dumping unexpected queue...\n", rank );
		MPIR_dump_queue( &MPIR_unexpected_recvs );
		fflush( stdout );
		}
	    }
	}

    /* Mark the MPI environment as having been destroyed.
       Note that the definition of MPI_Initialized returns only whether
       MPI_Init has been called; not if MPI_Finalize has also been called */
    MPIR_Has_been_initialized = 2;

    /* Un-initialize topology code */
    MPIR_Topology_finalize();

    ADIctx = MPI_COMM_WORLD->ADIctx;

    DBG(fprintf( stderr, "About to free operators\n" ); fflush( stderr );)
    MPI_Op_free( &MPI_MAX );
    MPI_Op_free( &MPI_MIN );
    MPI_Op_free( &MPI_SUM );
    MPI_Op_free( &MPI_PROD );
    MPI_Op_free( &MPI_LAND );
    MPI_Op_free( &MPI_BAND );
    MPI_Op_free( &MPI_LOR );
    MPI_Op_free( &MPI_BOR );
    MPI_Op_free( &MPI_LXOR );
    MPI_Op_free( &MPI_BXOR );
    MPI_Op_free( &MPI_MAXLOC );
    MPI_Op_free( &MPI_MINLOC );

    /* Free allocated space */
    DBG(fprintf( stderr, "About to free dtes\n" ); fflush( stderr );)
    MPI_Type_free( &MPI_INT );
    MPI_Type_free( &MPI_FLOAT );
    MPI_Type_free( &MPI_DOUBLE );
    MPI_Type_free( &MPI_LONG );
    MPI_Type_free( &MPIR_complex_dte );
    MPI_Type_free( &MPIR_dcomplex_dte );
    MPI_Type_free( &MPIR_logical_dte );
#ifndef MPID_NO_FORTRAN
#ifdef FOO
    /* Note that currently (see init.c), these are all copies of the
       existing C types, and so must not be freed (that will
       cause them to be freed twice) */
    if (MPIR_int1_dte)  MPI_Type_free( &MPIR_int1_dte );
    if (MPIR_int2_dte)  MPI_Type_free( &MPIR_int2_dte );
    if (MPIR_int4_dte)  MPI_Type_free( &MPIR_int4_dte );
    if (MPIR_real4_dte) MPI_Type_free( &MPIR_real4_dte );
    if (MPIR_real8_dte) MPI_Type_free( &MPIR_real8_dte );
#endif
#endif
    MPI_Type_free( &MPI_SHORT );
    MPI_Type_free( &MPI_CHAR );
    MPI_Type_free( &MPI_BYTE );
    MPI_Type_free( &MPI_UNSIGNED_CHAR );
    MPI_Type_free( &MPI_UNSIGNED_SHORT );
    MPI_Type_free( &MPI_UNSIGNED_LONG );
    MPI_Type_free( &MPI_UNSIGNED );
    MPI_Type_free( &MPI_PACKED );
    MPI_Type_free( &MPI_UB );
    MPI_Type_free( &MPI_LB );
    MPI_Type_free( &MPI_FLOAT_INT );
    MPI_Type_free( &MPI_DOUBLE_INT );
    MPI_Type_free( &MPI_LONG_INT );
    if (MPI_2INT != MPI_2INTEGER)
	MPI_Type_free( &MPI_2INTEGER );
    MPI_Type_free( &MPI_2INT );
    MPI_Type_free( &MPI_SHORT_INT );
    MPI_Type_free( &MPIR_2real_dte );
    MPI_Type_free( &MPIR_2double_dte );
    MPI_Type_free( &MPIR_2complex_dte );
    MPI_Type_free( &MPIR_2dcomplex_dte );

#if defined(HAVE_LONG_DOUBLE)
    MPI_Type_free( &MPI_LONG_DOUBLE );
    MPI_Type_free( &MPI_LONG_DOUBLE_INT );
#endif
#if defined(HAVE_LONG_LONG_INT)
    MPI_Type_free( &MPI_LONG_LONG_INT );
#endif

    DBG(fprintf( stderr, "About to free COMM_WORLD\n" ); fflush( stderr );)

    MPI_Comm_free ( &MPI_COMM_WORLD );

    DBG(fprintf( stderr, "About to free COMM_SELF\n" ); fflush( stderr );)

    MPI_Comm_free ( &MPI_COMM_SELF );

    DBG(fprintf( stderr, "About to free GROUP_EMPTY\n" ); fflush( stderr );)

    MPI_Group_free ( &MPI_GROUP_EMPTY );

    DBG(fprintf(stderr,"About to free permanent keyval's\n");fflush(stderr);)
	  
    MPI_Keyval_free( &MPI_TAG_UB );
    MPI_Keyval_free( &MPI_HOST );
    MPI_Keyval_free( &MPI_IO );
    MPI_Keyval_free( &MPIR_TAG_UB );
    MPI_Keyval_free( &MPIR_HOST );
    MPI_Keyval_free( &MPIR_IO );

    /* Tell device that we are done.  We place this here to allow
       the device to tell us about any memory leaks, since MPIR_SB... will
       free the storage even if it has not been deallocated by MPIR_SBfree. 
     */
    DBG(fprintf( stderr, "About to close device\n" ); fflush( stderr );)

    MPI_Errhandler_free( &MPI_ERRORS_RETURN );
    MPI_Errhandler_free( &MPI_ERRORS_ARE_FATAL );
    MPI_Errhandler_free( &MPIR_ERRORS_WARN );

    MPID_END( ADIctx );

    DBG(fprintf( stderr, "About to free SBstuff\n" ); fflush( stderr );)

    MPIR_SBdestroy( MPIR_dtes );
    MPIR_SBdestroy( MPIR_qels );
    MPIR_SBdestroy( MPIR_fdtels );
    MPIR_SBdestroy( MPIR_shandles );
    MPIR_SBdestroy( MPIR_rhandles );

    MPIR_SBdestroy( MPIR_hbts );
    MPIR_SBdestroy( MPIR_hbt_els );
    MPIR_SBdestroy( MPIR_topo_els );

#ifdef MPIR_MEMDEBUG
    MPIR_trdump( stdout );
#endif    
    /* barrier */
    return MPI_SUCCESS;
}





