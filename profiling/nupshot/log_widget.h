/*
   Tcl wrappers for unsuspecting logfile routines

   Ed Karrels
   Argonne National Laboratory
*/

#ifndef _LOG_WIDGET_H_
#define _LOG_WIDGET_H_

#include "tcl.h"
#include "log.h"

typedef struct logDataAndFile_ {
  logData data;
  logFile file;
} logDataAndFile;


#ifdef __STDC__

  /* register the "logfile" Tcl command */
int logfileAppInit( Tcl_Interp *interp );

  /* convert "log%d" to the pointer is represents */
logDataAndFile *LogToken2Ptr( Tcl_Interp *interp, char *str );

  /* the "logfile" commands */
int LogFileCmd( ClientData, Tcl_Interp *, int argc, char *argv[] );

#else
int logfileAppInit();
logDataAndFile *LogToken2Ptr();
int LogFileCmd();
#endif

#endif
