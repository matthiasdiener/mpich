#ifndef CHCONFIG
#define CHCONFIG

#define MPID_HAS_HETERO
#define MPID_HAS_PROC_INFO

/* This makes chbrndv.c use memcpy for rendezvous messages to self */
#define MPID_RNDV_SELF

/* Turn on flow control */
/* #define MPID_NO_FLOW_CONTROL */
#ifndef MPID_NO_FLOW_CONTROL
#define MPID_FLOW_CONTROL
#endif

/* Put macro-definitions of routines here */
#define MPID_getpid(i,n) p4_proc_info((i),(n))

/* Communicator initialization routines */
/* Comm_msgrep determines the common representation format for 
   members of the new communicator */
#define MPID_CommInit(oldcomm,newcomm) MPID_CH_Comm_msgrep( newcomm )
#define MPID_CommFree(comm)            MPI_SUCCESS

#endif
