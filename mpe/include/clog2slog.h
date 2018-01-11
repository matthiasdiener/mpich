#ifndef _CLOG2SLOG
#define _CLOG2SLOG

#include "clogimpl.h" 
#include "clog.h"

/* clog2slog structure */

/**** state_info
      A structure defined as a node in a list of CLOG-type state definitions.
      Contains all relevant information to convert a CLOG-type state into
      an SLOG-type interval.
      This list remains constant after all states definitions have been 
      initialized when the first pass through the clog file is made.
****/
struct state_info {
  int state_id;
  int start_event_num;
  int end_event_num;
  CLOG_CNAME color;
  CLOG_DESC description;
  struct state_info *next;
};

/**** list_elemnt
      A structure defined as a node in a list of CLOG-type events. Only the
      start events of states are added to this list and whenever a matching 
      end event is found that start event is removed from the list and an 
      slog interval logged. The list keeps growing and shrinking - its maximum
      size is the total number of processes in the logged parallel program 
      if there were no threads in the program. 
****/
struct list_elemnt {
  int state_id;
  int data;
  int process_id;
  int rectype;
  double start_time;
  struct list_elemnt *next;
};

/* clog2slog constants */
#define MSG_STATE      9999     /* for state_info list - not for SLOG */
#define MSG_RECORD     SLOG_RECTYPE_STATIC_OFFDIAG
#define NON_MSG_RECORD SLOG_RECTYPE_STATIC_DIAG
#define SLOG_PREVIEW_NAME "/dev/null"
#define C2S_ERROR      0
#define C2S_SUCCESS    1

#define CLASS_TYPE     "state"
#define FORWARD_MSG    10001
#define BACKWARD_MSG   10002
#define FORWARD_MSG_CLASSTYPE  "message"
#define BACKWARD_MSG_CLASSTYPE "message"
#define FORWARD_MSG_LABEL      "forward arrow"
#define BACKWARD_MSG_LABEL     "backward arrow"
#define FORWARD_MSG_COLOR      "white"
#define BACKWARD_MSG_COLOR     "grey"

#define EXTRA_STATES   40 
#define C2S_NUM_FRAMES     0
#define C2S_FRAME_BYTE_SIZE 64

/* clog2slog prototyping begins */ 

/** initializations **/

/*  void checkForBigEndian ( void );  */
int  CLOG_init_state_defs ( double * );
int  CLOG_init_all_mpi_state_defs( void );
int  init_SLOG ( long, long, char * );
int  init_SLOG_TTAB ( void );
int  init_SLOG_PROF_RECDEF ( void );
int  init_clog2slog ( char*, char** );
void CLOG_init_essential_values ( long, int );

/** actual logging */

#ifdef FOO
int  logEvent ( CLOG_HEADER * ,CLOG_RAW * ); 
int  writeSLOGInterval ( CLOG_HEADER*, CLOG_RAW*, struct list_elemnt ));
int  handleStartEvent ( int, CLOG_HEADER*, CLOG_RAW* );
int  handle_extra_state_defs ( CLOG_STATE * );

/** state_info and list_elemnt linked lists accessors **/

int  addState ( int ,int ,int ,CLOG_CNAME ,CLOG_DESC );
int  replace_state_in_list ( int, int, CLOG_CNAME, CLOG_DESC );
int  findState_strtEvnt ( int );
int  findState_endEvnt ( int );
int  get_new_state_id ( void );
int  addToList ( int ,int ,int ,double );
int  find_elemnt ( int ,int ,int ,struct list_elemnt* );
int  addToMsgList ( int ,int ,int ,int, double );
int  find_msg_elemnt ( int ,int ,int ,int, struct list_elemnt* );
void freeList ( void );
void freeMsgList ( void );
void freeStateInfo ( void );
void printEventList ( void );
void printMsgEventList ( void );
#endif
void CLOG_freeStateInfo(void);
int  CLOG_makeSLOG ( double * ); 
void CLOG_free_resources ( void );
void CLOG_printHelp ( void );
void CLOG_printStateInfo ( void );

#endif









