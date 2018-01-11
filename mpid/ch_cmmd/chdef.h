/*
    These are the definitions particular to the TMC CMMD implementation.

    THESE ARE COMPLETELY UNTESTED!!!!!
 */

#ifndef __commcmmd
#define __commcmmd

extern int __NUMNODES, __MYPROCID;

#define PSAllProcs 0

#define MPIDTRANSPORT  "ch_cmmd"

#include <cm/cmmd.h>

/* Note that the send/recv id in a request is actually an integer ARRAY
   of 4 elements; we pick the first (0 origin) element to hold the
   CMMD id.  */

/* Prefer the nonblocking, but provide the blocking. */
#define PIbsend(tag,buffer,length,to,datatype) \
             {if(CMMD_send_noblock(to,tag,,(char*)(buffer),length)){\
    SYexitall("FATAL error in sending data!\n",1);}}
#define PInsend(tag,buffer,length,to,datatype,sid) \
 sid[0] = CMMD_send_async(to,tag,,(char*)(buffer),length,(void*(*)())0,(void*)0)
#define PInsendrr PInsend
#define PIwsend(tag,buffer,length,to,datatype,sid) \
        {CMMD_msg_wait(sid);CMMD_free_mcb(sid);}
#define PIwsendrr PIwsend

#define PIbrecv(tag,buffer,length,datatype)  \
        CMMD_receive_block(CMMD_ANY_NODE,tag,(char*)(buffer),length)
#define PInrecv(tag,buffer,length,datatype,rid) \
        rid[0] = CMMD_receive_async(CMMD_ANY_NODE,tag,(char*)(buffer),length,(void*(*)())0,(void*)0)
#define PInrecvrr PInrecv
#define PIwrecv(tag,buffer,length,datatype,rid) \
        {CMMD_msg_wait(rid);CMMD_free_mcb(rid);}

#define PIwrecvrr PIwrecv

#define PInprobe(tag) \
        CMMD_msg_pending(CMMD_ANY_NODE,tag)

#define PInstatus(rid) \
     CMMD_msg_done(rid[0])

#define PIsize() CMMD_bytes_received()
#define PIfrom() CMMD_msg_sender()

/* Global operation used ONLY in heterogeneous setup code so not needed here */

#define PInumtids  __NUMNODES
#define PImytid    __MYPROCID

/* Initialization routines */
#define PIiInit   MPID_CMMD_Init
#define PIiFinish MPID_CMMD_End
#define SYexitall(msg,code) {CMMD_error("Exiting...\n");exit(code);}

void MPID_CMMD_Init ANSI_ARGS(( int *, char *** ));
void MPID_CMMD_End  ANSI_ARGS((void));
#endif
