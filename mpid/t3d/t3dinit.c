/*
 *  $Id: t3dinit.c,v 1.6 1995/06/07 06:34:26 bright Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: t3dinit.c,v 1.6 1995/06/07 06:34:26 bright Exp $";
#endif

#include "mpid.h"


/* 
 * ADI Initialization
 *
 *
 * Interface Description
 * ---------------------
 *
 * This file contains the routines that provide the basic information
 * on the device, and initialize it.  
 *
 * Currently, the following ADI functions are provided to the API:
 *
 *   void T3D_Set_space_debug_flag( int flag )
 *      Sets the debug flag for runtime memory debugging information.
 *
 *   void T3D_Mysize( int *size )
 *      Sets "size" to the number of processes in the world.
 *
 *   void T3D_Myrank( int *rank )
 *      Sets "rank" to my rank the in the world.
 *
 *   void *T3D_Init( int *argc, char ***argv )
 *      Initializes the device.  Returns an ADI context if the device
 *      uses them (for multiprotocol versions).
 *
 *   void T3D_Abort( int code )
 *      Prints abort message and causes code to quit/exit
 *
 *   void T3D_End()
 *      Frees device resources.  Called when MPI environment is being
 *      destroyed.
 *
 *   void T3D_Node_name( char *name, int len )
 *      Returns the name of the current processor in "name".
 *
 *   void T3D_Version_name( char *name )
 *      Returns version information about the device in "name".
 *
 *   double T3D_Wtime()
 *   double T3D_Wtick()
 *      Clock routines that mirror the functionality prescribed
 *      in the MPI document.
 * 

/****************************************************************************
  Include files
 ***************************************************************************/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "t3drecv.h"
#include <signal.h>

/****************************************************************************
  Global variables
 ***************************************************************************/
char  t3d_hostname[T3D_HOSTNAME_LEN];
int   t3d_myid;
int   t3d_num_pes;
long *t3d_heap_limit;

/****************************************************************************
  Local variables
 ***************************************************************************/
static double t3d_reference_time = T3D_UNDEFINED;


/***************************************************************************
   T3D_Set_init_debug_flag
 ***************************************************************************/
static int DebugFlag = 0;
void T3D_Set_init_debug_flag( flag )
int flag;
{
    DebugFlag = flag;
}

/***************************************************************************
   T3D_Set_space_debug_flag
 ***************************************************************************/
static int DebugSpace = 0;
void T3D_Set_space_debug_flag( flag )
int flag;
{
    DebugSpace = flag;
}


/***************************************************************************
   T3D_Myrank
 ***************************************************************************/
void T3D_Myrank( rank )
int *rank;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Myrank\n");
    }
#   endif

    /* Set the value of the current processes rank in the world of
       processes. */
    (*rank) = T3D_MY_PE;
}


/***************************************************************************
   T3D_Mysize
 ***************************************************************************/
void T3D_Mysize( size )
int *size;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Mysize\n");
    }
#   endif

    /* Set the size of the world; i.e., how many processes exist */
    (*size) = T3D_NUM_PES;
}


/***************************************************************************
  T3D_Wtime

  Description:
     Return a time in "seconds"
 ***************************************************************************/
double T3D_Wtime()
{
    double current_time;
    double wtime;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Wtime\n");
    }
#   endif


    current_time = (double)rtclock();

    wtime = (current_time - t3d_reference_time) * 6.6e-3 * 0.000001 ;

    return wtime;
}


/***************************************************************************
  T3D_Wtick
 ***************************************************************************/
double T3D_Wtick()
{

   static double tickval = -1.0;
   double t1, t2;
   int    cnt;
   int    icnt;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT)
    if (DebugFlag) {
      T3D_Printf("T3D_Wtick\n");
    }
#   endif

   if (tickval < 0.0) {
     tickval = 1.0e6;
     for (icnt=0; icnt<10; icnt++) {
       cnt = 1000;
       t1  = T3D_Wtime();
       while (cnt-- && (t2 = T3D_Wtime()) <= t1) ;
         if (cnt && t2 - t1 < tickval)
           tickval = t2 - t1;
       }
   }

   return tickval;
}


/***************************************************************************
   T3D_Initenv

   Initialize global variables.
 ***************************************************************************/
void T3D_Initenv(argc,argv)
int   *argc;
char **argv;
{
    char temp_t3d_lcp[100];

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Initenv\n");
    }
#   endif

    /* Get the host name */
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT)
    if (gethostname(t3d_hostname, T3D_HOSTNAME_LEN) != 0) {
        T3D_Error(T3D_ERR_UNKNOWN, "unable to get host name\n");
        t3d_hostname[0] = '\0';
        return;
    }
#   else
    (void)gethostname(t3d_hostname, T3D_HOSTNAME_LEN);
#   endif

}


/***************************************************************************
   T3D_Init
 ***************************************************************************/
void *T3D_Init( argc, argv )
int  *argc;
char ***argv;
{
    int        i, numprocs;
    int        size;
    T3D_PKT_T *pkt;

#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Init\n");
    }
#   endif

    /* Set the clocks initial time */
    t3d_myid           = _my_pe();
    t3d_num_pes        = _num_pes();
    t3d_reference_time = (double)rtclock();
    t3d_heap_limit     = sbreak( 0 );

    /* Look for a subset number */
    numprocs = t3d_num_pes;
    for (i=1; i<*argc; i++) {
	if (strcmp( (*argv)[i], "-np" ) == 0) {
	    /* Need to remove both args and check for missing value for -np */
	    if (i + 1 == *argc) {
		fprintf( stderr, 
		    "Missing argument to -np for number of processes\n" );
		exit( 1 );
		}
	    numprocs = atoi( (*argv)[i+1] );
	    (*argv)[i] = 0;
	    (*argv)[i+1] = 0;
	    MPIR_ArgSqueeze( argc, *argv );
	    break;
	    }
	}
    if (numprocs <= 0 || numprocs > t3d_num_pes) {
	fprintf( stderr, 
		"Invalid number of processes (%d) invalid\n", numprocs );
	exit( 1 );
	}
    t3d_num_pes = numprocs;

    shmem_set_cache_inv();

    size = sizeof( T3D_PKT_T ) * t3d_num_pes;

    t3d_recv_bufs = (T3D_PKT_T *)shmalloc( size );

    size = sizeof( T3D_Buf_Status ) * t3d_num_pes;

    t3d_dest_bufs = (T3D_Buf_Status *)shmalloc( size );

    for ( i=0; i<t3d_num_pes; i++ ) 
      t3d_dest_bufs[i] = t3d_recv_bufs[i].head.status = T3D_BUF_AVAIL;

    barrier();

    if (t3d_myid >= numprocs) {
	/* Turn off these processes */
	T3D_End();
	exit(0);
	}

    return ((void *)0);
}


/***************************************************************************
  T3D_Abort
 ***************************************************************************/
void T3D_Abort( code )
int code;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Abort\n");
    }
#   endif

    /* Print abort message */
    /* Clean up device if possible and abort */
    (void) T3D_Printf("Aborting with error code = %d\n",code);
    /* See MPIBUGS #1010 */
    globalexit(1);
    /* Just in case ... */
    abort();
}

/***************************************************************************
  T3D_End
 ***************************************************************************/
void T3D_End()
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_End\n");
    }
#   endif

    /* Free resources used by the device. This function is called when the
       MPI environment is being destroyed.  */

    /* For debugging/profiling versions, this is where calls to dump
       statistics might be placed */

    shfree( (void *)t3d_recv_bufs );

    shfree( (void *)t3d_dest_bufs );

    shmem_clear_cache_inv();
}


/***************************************************************************
  T3D_Node_name
 ***************************************************************************/
void T3D_Node_name( name, len )
char *name;
int   len;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Node_name\n");
    }
#   endif

    /* Get the host name */
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT)
    if (gethostname(name, len) != 0) {
        T3D_Error(T3D_ERR_UNKNOWN, "unable to get host name\n");
        name[0] = '\0';
        return;
    }
#   else
    (void)gethostname(name, len);
#   endif
}


/***************************************************************************
  T3D_Version_name
 ***************************************************************************/
void T3D_Version_name( name )
    char *name;
{
#   if defined(MPID_DEBUG_ALL) || defined(MPID_DEBUG_INIT) 
    if (DebugFlag) {
      T3D_Printf("T3D_Version_name\n");
    }
#   endif

    /* Insert appropriate information into "name".  "name" is currently
       defined in mpich/src/init.c to be 128 characters long */
    (void)sprintf( name, "T3D Device Driver, Version 0.0" );
}

