#define POST_SHORT 0
#define POST_LONG 1
#define POST_SYNC_SHORT 2
#define POST_SYNC_LONG 3
#define CMPL_SYNC 4
#define CMPL_SEND 5
#define ISEND_WAIT 6
#define TEST_SEND 7
#define CMPL_PEND 8
#define POST_SEND 9

#define SHORT_SET 10
#define SHORT_PACK 11
#define SHORT_MEMCPY 12
#define SHORT_DRAIN 13
#define SHORT_SEND 14
#define SHORT_MARK 15

#define BLOCKING_SEND 16
#define BLOCKING_RECV 17

#define TOP_LEVEL 18
#define TOP_LEVEL_RECV 24
#define CHK_CONT 19

#define SEND_CONTROL 20
#define SEND_INIT 21
#define SEND_PACK 22
#define SEND_SEND 23

#define SEND_DATA 25

#define GET_CONT 27
#define GET_UNPACK 28
#define GET_COND 29

#define RECV_SEARCH 31
#define RECV_PROCESS 32
#define RECV_WAIT 33
#define RECV_OTHER 34
#define RECV_FINISH 35
#define NEXUS_WAIT 36
#define RECV_NEXUS 37
#define FIND_CONTROL 38
#define FIND_DATA 39

#define CRITICAL_PATH 41
#define SEND_CRITICAL 42
#define RECV_CRITICAL 43

#define NX_SND_CRTCL 45
#define MPI_SND_CRTCL 46
#define NX_RCV_CRTCL 47
#define MPI_RCV_CRTCL 48

#define N_TIMERS 50
#define TIMER_FILE "/sphome/geisler/test/timers.utp"

extern int critical;
extern int send_started;
extern int recv_started;
