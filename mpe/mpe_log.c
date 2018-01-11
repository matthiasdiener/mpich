/**\ --MPE_Log--
*  * mpe_log.c - the externally callable functions in MPE_Log
*  *
*  * MPE_Log currently represents some code written by Dr. William
*  * Gropp, stolen from Chameleon's 'blog' logging package and
*  * modified by Ed Karrels, as well as some fresh code written
*  * by Ed Karrels.
*  *
*  * All work funded by Argonne National Laboratory
\**/

#include <stdio.h>
#include "mpi.h"

#define DEBUG 0
#define DEBUG_FORMATRECORD 0
#define DEBUG_RELOAD 0
#define DEBUG_LINKS 0

/* 
   Include a definition of MALLOC and FREE to allow the use of Chameleon
   memory debug code 
*/
#ifdef DEVICE_CHAMELEON
#include "mpisys.h"
#else
#define MALLOC(a) malloc(a)
#define FREE(a)   free(a)
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#endif

#ifdef USE_PMPI
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
#endif

#include "mpe_log.h"
#include "mpe_log_genproc.h"

#include "mpe_log_genproc.c"
#include "mpe_log_adjusttime.c"
#include "mpe_log_merge.c"

/*@
    MPE_Initlog - Initialize for logging
@*/
int MPE_Init_log()
{
  char fn[40];

  if (!MPE_Log_hasBeenInit || MPE_Log_hasBeenClosed) {
    MPI_Comm_rank( MPI_COMM_WORLD, &MPE_Log_procid ); /* get process ID */
#if DEBUG
    sprintf( fn, "mpe_log_debug%d.out", MPE_Log_procid );
    debug_file = fopen( fn, "w" );
#endif
    if (!MPE_Log_Flush()) {	/* get the first block for data */
      fprintf(stderr, "MPE_Log: **** trace buffer creation failure ****\n");
      return MPE_Log_NO_MEMORY;
    }
    MPE_Log_hasBeenInit = 1;	/* set MPE_Log as being initialized */
    MPE_Log_hasBeenClosed = 0;
    MPE_Log_isLockedOut = 0;
      /* set MPE_Log as not being not closed, Startlog is not required */
    MPI_Barrier( MPI_COMM_WORLD );
    MPE_Log_init_clock();
    MPE_Log_event(MPE_Log_EVENT_SYNC,0,0);	/* log a 'sync' event */
  }
  return MPE_Log_OK;
}





/*@
    MPE_Start_log - Begin logging of events
@*/
int MPE_Start_log()
{
  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  MPE_Log_isLockedOut = 0;
  return MPE_Log_OK;
}



/*@
    MPE_Stop_log - Stop logging events
@*/
int MPE_Stop_log()
{
  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  MPE_Log_isLockedOut = 1;
  return MPE_Log_OK;
}




/*@
    MPE_Describe_state - Create log record describing a state

    Notes:
    Adds string containing a state def to the logfile.  The format of the
    def is (LOG_STATE_DEF) 0 sevent eevent 0 0 "color" "name".
@*/
int MPE_Describe_state( start, end, name, color )
int start, end;
char *name, *color;
{
  int  lnc, lnn, ln4;
  char buf[150];
  MPE_Log_HEADER *b;
  MPE_Log_VFIELD *v;
  int         i[2];
  

  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
#if DEBUG
  fprintf( debug_file, "State :%s: %d %d defined.\n", name, start, end );
      fflush( debug_file );
#endif

  strncpy( buf, color, 150 );
  strncat( buf, " ", 150 );
  strncat( buf, name, 150 );
  if (MPE_Log_i + MPE_Log_HEADERSIZE + MPE_Log_VFIELDSIZE(2) + 
      MPE_Log_VFIELDSIZE((strlen(buf)+sizeof(int)-1)/sizeof(int))>
      MPE_Log_size)
    if (!MPE_Log_Flush()) return MPE_Log_NO_MEMORY;

  b = (MPE_Log_HEADER *)((int *)(MPE_Log_thisBlock+1) + MPE_Log_i);
  MPE_Log_ADDHEADER(b,LOG_STATE_DEF);
  i[0] = start;
  i[1] = end;
  MPE_Log_ADDINTS(b,v,2,i);
  MPE_Log_ADDSTRING(b,v,buf);
#if DEBUG
  fprintf( debug_file, "add state %s, record length %d, buffer size %d\n",
	   name, b->len, MPE_Log_i);
#endif
  
  return MPE_Log_OK;
}



/*@
    MPE_Describe_event - Create log record describing an event type
@*/
int MPE_Describe_event( event, name )
int event;
char *name;
{
  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
#if DEBUG
  fprintf( debug_file, "Event :%s: %d defined.\n", name, event );
      fflush( debug_file );
#endif
  MPE_Log_def (event, name);
  return MPE_Log_OK;
}



/*@
    MPE_Log_message - log the sending or receiving of a message
@*/
static int MPE_Log_message( type, otherParty, tag, size )
int type, otherParty, tag, size;
{
  MPE_Log_HEADER *b;
  MPE_Log_VFIELD *v;

  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  if (MPE_Log_isLockedOut) return MPE_Log_LOCKED_OUT;
  
#if DEBUG>1
  fprintf( debug_file, "[%d] %s %d bytes with tag %d %s %d at %10.5lf\n",
	  MPE_Log_procid, type==LOG_MESG_SEND?"send":"receive", size, tag,
	  type==LOG_MESG_SEND?"to":"from", otherParty, MPI_Wtime() );
  fflush( debug_file );
#endif

  /*              header (w/event)     data                 string */
  if (MPE_Log_i + MPE_Log_HEADERSIZE + MPE_Log_VFIELDSIZE(1)*3 >
      MPE_Log_size) {
    /* fprintf(stderr,"allocate more memory\n"); */
    if (!MPE_Log_Flush()) return MPE_Log_NO_MEMORY;
      /* cannot allocate more memory */
  }
  
  MPE_Log_ADDHEADER( b, type );
  MPE_Log_ADDINTS (b, v, 1, &otherParty); /* store otherParty */
  MPE_Log_ADDINTS (b, v, 1, &tag);	  /* store tag */
  MPE_Log_ADDINTS (b, v, 1, &size);	  /* store size */

#if DEBUG
  fprintf( debug_file, "add event, record length %d, buffer size %d\n", b->len,
	   MPE_Log_i);
  fflush( debug_file );
#endif

  return MPE_Log_OK;
}


/*@
    MPE_Log_send - Logs the sending of a message
@*/
int MPE_Log_send( otherParty, tag, size )
int otherParty, tag, size;
{
  MPE_Log_message( LOG_MESG_SEND, otherParty, tag, size );
  return MPE_Log_OK;
}



/*@
    MPE_Log_receive - log the sending of a message
@*/
int MPE_Log_receive( otherParty, tag, size )
int otherParty, tag, size;
{
  MPE_Log_message( LOG_MESG_RECV, otherParty, tag, size );
  return MPE_Log_OK;
}



/*@
    MPE_Log_event - Logs an event
@*/
int MPE_Log_event(event,data,string)
int event, data;
char *string;
{
  MPE_Log_HEADER *b;
  MPE_Log_VFIELD *v;
  char c;
  int str_len,	    /* length of the string */
      str_store;    /* # of ints needed to store the string */

  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  if (MPE_Log_isLockedOut) return MPE_Log_LOCKED_OUT;

  if (!string) {
    string	= "";
    str_len	= 0;
    str_store	= 0;
  }
  else {
    str_len	= strlen(string);
    str_store	= str_len / sizeof(int) + 1;
  }

#if DEBUG_LINKS
PrintBlockLinks( debug_file );
      fflush( debug_file );
#endif

#if DEBUG>1
fprintf( debug_file, "[%d] event %d at %10.5lf\n", MPE_Log_procid, event,
	 MPI_Wtime() );
      fflush( debug_file );
#endif
  /*              header (w/event)     data                 string */
  if (MPE_Log_i + MPE_Log_HEADERSIZE + MPE_Log_VFIELDSIZE(1) +
      MPE_Log_VFIELDSIZE(str_store) > MPE_Log_size) {
    /* fprintf(stderr,"allocate more memory\n"); */
    if (!MPE_Log_Flush()) return MPE_Log_NO_MEMORY;
      /* cannot allocate more memory */
  }
  
  MPE_Log_ADDHEADER (b, event);
  MPE_Log_ADDINTS (b, v, 1, &data);	/* add one int to the definition */
  
  if (MPE_Log_i + MPE_Log_VFIELDSIZE(str_store) > MPE_Log_size) {
    

    /* fprintf(stderr,"long string cut, %d %d %d ints used for headers\n", 
       MPE_Log_HEADERSIZE, MPE_Log_VFIELDSIZE(1), MPE_Log_VFIELDSIZE(0)); */
    
    v           = (MPE_Log_VFIELD *)((int *)(MPE_Log_thisBlock+1) + MPE_Log_i);
    str_store   = MPE_Log_size - MPE_Log_i - MPE_Log_VFIELDSIZE(0);
    str_len     = str_store*sizeof( int )-1;
    v->len      = MPE_Log_VFIELDSIZE(str_store);  /* size of 'v' in ints */
    b->len      += v->len;
    v->dtype    = MPE_Log_CHAR;
    memcpy( v->other, string, str_len);
    ((char *)(v->other))[str_len]=0;   /* terminate string */
    MPE_Log_i   += v->len;
  } else {
    v           = (MPE_Log_VFIELD *)((int *)(MPE_Log_thisBlock+1) + MPE_Log_i);
    v->len      = MPE_Log_VFIELDSIZE(str_store);
    b->len      += v->len;
    v->dtype    = MPE_Log_CHAR;
    memcpy( v->other, string, str_len+1 );
    MPE_Log_i   += v->len;
  }
#if DEBUG
  fprintf( debug_file, "add event, record length %d, buffer size %d\n", b->len,
	   MPE_Log_i);
  fflush( debug_file );
#endif
  return MPE_Log_OK;
}


/*@
    MPE_Finish_log - Send log to master, who writes it out

    Notes:
    This routine dumps a logfile in alog format
@*/
int MPE_Finish_log( filename )
char *filename;
{
  int returnStatus;

  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  if (!MPE_Log_hasBeenClosed) {
    MPE_Log_isLockedOut = 1;
    MPI_Barrier(MPI_COMM_WORLD );
    MPE_Log_isLockedOut = 0;
    MPE_Log_event(MPE_Log_EVENT_SYNC,0,0);	/* log a 'sync' event */
    MPE_Log_isLockedOut = 1;
    MPE_Log_thisBlock->size = MPE_Log_i;
#if DEBUG
    fprintf( debug_file, "adjusting times\n" );
    PrintBlockChain( debug_file, MPE_Log_firstBlock );
      fflush( debug_file );
#endif
    MPE_Log_adjusttimes();
#if DEBUG
    fprintf( debug_file, "parallel merge\n" );
      fflush( debug_file );
#endif
    if (returnStatus=MPE_Log_ParallelMerge( filename )) 
      return returnStatus;
    MPE_Log_hasBeenClosed = 1;
  }
  return MPE_Log_OK;
}

#undef DEBUG
#undef DEBUG_FORMATRECORD
#undef DEBUG_RELOAD
#undef DEBUG_LINKS
