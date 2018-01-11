/* Message stuff */


#ifndef _MSGS_H_
#define _MSGS_H_

#include "expandingList.h"
/*
#include "idx.h"
*/

#define MSG_SEND 1
#define MSG_RECV 2

typedef struct msgDefInfo_ {
  int tag;
  char *name;
  char *color;
} msgDefInfo;


  /* type is the index of this message's definition */
typedef struct msgInfo_ {
  double sendTime, recvTime;
  int type, size, sender, recver;
} msgInfo;


typedef struct msgPost_ {
  int isUsed;
  int tag;
  double time;
} msgPost;


typedef struct msgData_ {
  xpandList /*msgDefInfo*/ defs;
    /* definitions for each tag */
  xpandList /*msgInfo*/ list;
    /* list of all messages, in the order they were encountered */
  xpandList /*msgPost*/ *sendq;
    /* an in/out queue for each process */
  xpandList /*msgPost*/ *recvq;
    /* and tag (array of lists of msgPosts */
  /* intIdx *tagList; */
    /* quick reference list of tag definitions */
  xpandList /*int*/ idx_send;
    /* index sorted by send time */
  xpandList /*int*/ idx_recv;
    /* index sorted by recv time */
  xpandList /*int*/ *idx_proc_send;
    /* indexed by process number, sorted by send time */
  xpandList /*int*/ *idx_proc_recv;
    /* indexed by process number, sorted by recv time */
  int np;			/* number of processes */
} msgData;
  /* As the messages are being read in, build list of tags.
     When all done, sort them. */

typedef struct msgsVisible_ {
  xpandList /*intIdx*/ list;
} msgsVisible;


#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif



  /* allocate memory for message data structure */
msgData *Msg_Create();

  /* describe a message */
int Msg_Def ARGS(( msgData *, int tag, char *name, char *color ));

  /* return the # of message descriptions */
int Msg_Ndefs ARGS(( msgData * ));

  /* get a message description */
int Msg_GetDef ARGS(( msgData *, int def_num, char **name, int *tag,
		      char **color ));

  /* set a message description */
int Msg_SetDef ARGS(( msgData *, int def_num, char *name, int tag,
		      char *color ));

  /* allocate memory for message data */
int Msg_DataInit ARGS(( msgData *msg_data, int np ));

  /* store the 'send' side of a message */
  /* return 1 if the message was a tachyon */
int Msg_Send ARGS(( msgData *msg_data, int sender, int recver,
	      double time, int tag, int size ));

int Msg_Recv ARGS(( msgData *msg_data, int recver, int sender,
	      double time, int tag, int size ));
  /* store the 'recv' side of a message */
  /* return 1 if the message was a tachyon */

int Msg_DoneAdding ARGS(( msgData *msg_data ));
  /* Shrinkwrap memory used by message data.  Sort messages.
     Might want to use an insertion
     sort since the data might already be well sorted. */

  /* Return the number of messages logged */
int Msg_N ARGS(( msgData *msg_data ));

int Msg_Get ARGS(( msgData *msg_data, int n, int *type,
		   int *sender, int *recver, double *sendTime,
		   double *recvTime, int *size ));

#if TESTING
  /* Prints a list of all messages read into memory. */
int Msg_PrintAll ARGS(( msgData *msg_data ));
#endif

int Msg_Close ARGS(( msgData * ));
  /* free all memory associated with messages */


#endif
