/* mpe_log.c - the externally callable functions in MPE_Log

   New version to use CLOG - Bill Gropp and Rusty Lusk

*/

#include "clog.h"
#include "clog_merge.h"
#include "mpi.h"		/* Needed for MPI routines */
#include "mpe.h"		/* why? */
#include "mpe_log.h"

#ifdef HAVE_STDLIB_H
/* Needed for getenv */
#include <stdlib.h>
#endif

/* why this? */
#ifdef USE_PMPI
#define MPI_BUILD_PROFILING
#include "mpiprof.h"
#endif

/* temporarily borrowed from mpe_log_genproc.h */ /* try to replace by CLOG */
int    MPE_Log_hasBeenInit = 0;
int    MPE_Log_hasBeenClosed = 0;
int    MPE_Log_clockIsRunning = 0;
int    MPE_Log_isLockedOut = 0;
int    MPE_Log_AdjustedTimes = 0;
#define MPE_HAS_PROCID
static int    MPE_Log_procid;
/* end of borrowing */

/*@
    MPE_Init_log - Initialize for logging

    Notes:
    Initializes the MPE logging package.  This must be called before any of
    the other MPE logging routines.

.seealso: MPE_Finish_log
@*/
int MPE_Init_log()
{
    if (!MPE_Log_hasBeenInit || MPE_Log_hasBeenClosed) {
	MPI_Comm_rank( MPI_COMM_WORLD, &MPE_Log_procid ); /* get process ID */
	CLOG_Init();
	CLOG_LOGCOMM(INIT, -1, (int) MPI_COMM_WORLD);

	MPE_Log_hasBeenInit = 1;	/* set MPE_Log as being initialized */
	MPE_Log_hasBeenClosed = 0;
	MPE_Log_isLockedOut = 0;
    }
    return MPE_Log_OK;
}

/*@
    MPE_Start_log - Begin logging of events
@*/
int MPE_Start_log()
{
  if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;
  CLOG_status = 0;
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
  CLOG_status = 1;
  return MPE_Log_OK;
}

/*@
    MPE_Describe_state - Create log record describing a state

    Notes:
    Adds string containing a state def to the logfile.  The format of the
    def is (LOG_STATE_DEF) 0 sevent eevent 0 0 "color" "name".

.seealso: MPE_Log_get_event_number 
@*/
int MPE_Describe_state( start, end, name, color )
int start, end;
char *name, *color;
{
    int stateid;

    if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;

    stateid = CLOG_get_new_state();
    CLOG_LOGSTATE( stateid, start, end, color, name);

    return MPE_Log_OK;
}

/*@
    MPE_Describe_event - Create log record describing an event type
    
    Input Parameters:
+   event - Event number
-   name  - String describing the event. 

.seealso: MPE_Log_get_event_number 
@*/
int MPE_Describe_event( event, name )
int event;
char *name;
{
    if (!MPE_Log_hasBeenInit) return MPE_Log_NOT_INITIALIZED;

    CLOG_LOGEVENT( event, name);

    return MPE_Log_OK;
}

/*@
  MPE_Log_get_event_number - Gets an unused event number

  Returns:
  A value that can be provided to MPE_Describe_event or MPE_Describe_state
  which will define an event or state not used before.  

  Notes: 
  This routine is provided to allow packages to ensure that they are 
  using unique event numbers.  It relies on all packages using this
  routine.
@*/
int MPE_Log_get_event_number( )

{
    return CLOG_get_new_event();
}

/*@
    MPE_Log_send - Logs the sending of a message
@*/
int MPE_Log_send( otherParty, tag, size )
int otherParty, tag, size;
{
    char comment[20];
    sprintf(comment, "%d %d", tag, size);
    CLOG_LOGRAW( LOG_MESG_SEND, otherParty, comment );
    return MPE_Log_OK;
}

/*@
    MPE_Log_receive - log the sending of a message
@*/
int MPE_Log_receive( otherParty, tag, size )
int otherParty, tag, size;
{
    char comment[20];
    sprintf(comment, "%d %d", tag, size);
    CLOG_LOGRAW( LOG_MESG_RECV, otherParty, comment );
    return MPE_Log_OK;
}

/*@
    MPE_Log_event - Logs an event

    Input Parameters:
+   event - Event number
.   data  - Integer data value
-   string - Optional string describing event
@*/
int MPE_Log_event(event,data,string)
int event, data;
char *string;
{
    CLOG_LOGRAW( event, data, string );
    return MPE_Log_OK;
}

/*@
    MPE_Finish_log - Send log to master, who writes it out

    Notes:
    This routine dumps a logfile in clog format.

@*/
int MPE_Finish_log( filename )
char *filename;
{
/*  The environment variable MPE_LOG_FORMAT may be set to CLOG to generate
    CLOG format log files (ALOG is the default). */
    char *env_log_format;
    int shift, log_format;
    int *is_globalp, flag;

    CLOG_Finalize();

    MPI_Attr_get( MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &is_globalp, &flag );
    if (!flag || (is_globalp && !*is_globalp))
	shift = CMERGE_SHIFT;
    else
        shift = CMERGE_NOSHIFT;

    log_format = ALOG_LOG;
    env_log_format = getenv("MPE_LOG_FORMAT");
 
    if ((env_log_format) && (strcmp(env_log_format,"CLOG") == 0)) 
	log_format = CLOG_LOG;

    CLOG_mergelogs(shift, filename, log_format); 
    return MPE_Log_OK;
}

