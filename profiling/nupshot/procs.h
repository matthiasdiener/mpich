/*
   Process information for Upshot.
   Thanks to dennis@rats-b.nosc.mil (Dennis Cottel) for this file
*/


#ifndef _PROCS_H_
#define _PROCS_H_

#include "expandingList.h"
/*
#include "idx.h"
*/

typedef struct processDefInfo_ {
  char *name;
} processDefInfo;

typedef struct processDefData_ {
  xpandList /*processDefInfo*/ list;
} processDefData;

typedef struct processData {
  processDefData defs;		/* process definitions */
} processData;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif


  /* call before adding any process definitions */
processData *Process_Create();

  /* call before adding any process definitions after # processes is known */
int Process_DataInit ARGS(( processData *process_data, int np ));

  /* retrieve a process definition */
int Process_GetDef ARGS(( processData *process_data, int n, char **name ));

  /* set a process definition */
int Process_SetDef ARGS(( processData *process_data, int n, char *name ));

  /* release memory used by process data structures */
int Process_Close ARGS(( processData * ));


#endif
