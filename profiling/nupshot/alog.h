/*
   Alog-specific stuff for upshot
*/

#ifndef _ALOG_H_
#define _ALOG_H_

#include "expandingList.h"

typedef struct alogStateDefInfo_ {
  int startEvt, endEvt;
} alogStateDefInfo;

typedef struct alogStateDefData_ {
  xpandList /*alogStateDefInfo*/ list;
  xpandList /*int*/ startEvents;
  xpandList /*int*/ endEvents;
} alogStateDefData;

typedef struct alogEventDefData_ {
  xpandList /*int*/ list;
} alogEventDefData;

typedef struct alogLineData_ {
  int type, process, task, data, cycle;
  double timestamp;
  char comment[100];
} alogLineData;


typedef struct alogData_ {
  int format;
  char *name;
  int lineNo;
  char *lastLine;
  double rolloverPt;
  alogStateDefData stateDefs;
  alogEventDefData eventDefs;
  alogLineData leftOverLine;
  FILE *leftOver_fp;
} alogData;

#include "log_common.h"


#define ALOG_MAX_COMMENT_LEN 100
#define ALOG_PROCESS_VS_PRE_RATIO 6

#define ALOG_MAX_HEADER_EVT    -1
#define ALOG_MIN_HEADER_EVT    -100
#define ALOG_CREATOR       -1
#define ALOG_NEVENTS       -2
#define ALOG_NP            -3
#define ALOG_NTASKS        -4
#define ALOG_NEVENT_TYPES  -5
#define ALOG_START_TIME    -6
#define ALOG_END_TIME      -7
#define ALOG_NTIMER_CYCLES -8
#define ALOG_EVT_DEF       -9
#define ALOG_EVT_FORMAT    -10
#define ALOG_ROLLOVER_PT   -11
#define ALOG_UNUSED0       -12
  /* what happened to -12? */
#define ALOG_STATE_DEF     -13
#define ALOG_SYNC          -100
#define ALOG_MESG_SEND     -101
#define ALOG_MESG_RECV     -102


#ifdef __STDC__
int AlogPreProcessLog( char *filename, logData *log_data,
			    alogData *alog_data );
int AlogProcessLog( logData *log_data, alogData *alog_data );
int AlogCloseLog( alogData *alog_data );
#else

int AlogPreProcessLog();
int AlogProcessLog();
int AlogCloseLog();

#endif


#endif
/* _ALOG_H_ */
