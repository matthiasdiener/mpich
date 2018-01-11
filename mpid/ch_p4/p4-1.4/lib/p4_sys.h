#ifdef P4BSD
#include <strings.h>
#endif
#ifdef P4SYSV
#include <string.h>
#endif

#include "p4_patchlevel.h"
#include "p4_sock_util.h"    

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

extern int errno;
/*****
extern char *sys_errlist[];
*****/

#define PRECV_EOF -1

#define LISTENER_ATTN_SIGNAL SIGUSR1

#define LISTENER_ID (-99)

#include "p4_sys_funcs.h"

#include "p4_defs.h"
#include "p4_macros.h"

#include "p4_globals.h"


#define BEGIN_USER      101
#define END_USER        102
#define BEGIN_SEND      103
#define END_SEND        104
#define BEGIN_RECV      105
#define END_RECV        106
#define BEGIN_WAIT      107
#define END_WAIT        108

#define UPDATE_NUM_SUBPROBS     200
#define REQUEST_MONITOR_ENTRY   201
#define ENTER_MONITOR           202
#define EXIT_MONITOR            203
#define OPEN_DOOR               204
#define ENTER_DELAY_QUEUE       205
#define EXIT_DELAY_QUEUE        206
#define SECRET_EXIT_MONITOR     207
#define PBDONE                  208
#define PGDONE                  209

