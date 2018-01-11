/*
   Event stuff for Upshot.
*/


#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "expandingList.h"
/*
#include "idx.h"
*/

typedef struct eventDefInfo_ {
  char *name;
} eventDefInfo;

typedef struct eventDefData_ {
  xpandList /*eventDefInfo*/ list;
} eventDefData;

typedef struct eventInfo_ {
  double time;
  int type, proc;
} eventInfo;

typedef struct eventData {
  xpandList /*eventInfo*/ list;
    /* master list of all events */
  xpandList /*int*/ idx;          /* index of events, sorted by time */
  xpandList /*int*/ *idx_proc;    /* indexed by process #, then by time */
    /* index of events, one list per process */
  eventDefData defs;		/* event definitions */
  int np;			/* number of processes */
} eventData;

typedef struct eventsVisible_ {
  int *list, n;
} eventsVisible;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif


eventData *Event_Create();
  /* call before adding any event definitions */

  /* call before adding any event instances */
int Event_DataInit ARGS(( eventData *event_data, int np ));

  /* Return the number of event type */
int Event_Ndefs ARGS(( eventData *event_data ));

  /* add an event definition */
int Event_AddDef ARGS(( eventData *event_data, char *desc ));

  /* retrieve an event definition */
int Event_GetDef ARGS(( eventData *event_data, int n, char **desc ));

  /* set an event definition */
int Event_SetDef ARGS(( eventData *event_data, int n, char *desc ));

  /* log one event */
int Event_Add ARGS(( eventData *event_data, int type,
		     int proc, double time ));

  /* get the number of events stored */
int Event_N ARGS(( eventData *event_data ));

  /* return info on one particular event */
int Event_Get ARGS(( eventData *event_data, int n, int *type, int *proc,
		     double *time ));

  /* finished adding events, maybe make an index or two */
int Event_DoneAdding ARGS(( eventData *event_data ));
  /* call after logging all event instances */
  /* in this call the incices will be created */

  /* release memory used by event data structures */
int Event_Close ARGS(( eventData * ));


#endif
