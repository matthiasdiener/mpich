#ifndef CHCONFIG
#define CHCONFIG

/* Special configuration information for ch_shmem device goes here */

/* Turn on flow control */
#define MPID_NO_FLOW_CONTROL
#ifndef MPID_NO_FLOW_CONTROL
#define MPID_FLOW_CONTROL
/* chflow uses just SendControl */
#define MPID_SendControl MPID_SHMEM_SendControl
/* shmem is homogeneous */
#define MPID_PKT_PACK(pkt,size,dest) 
#define MPID_PKT_UNPACK(pkt,size,src) 
#endif

#define MPID_CommInit(oldcomm,newcomm) MPI_SUCCESS
#define MPID_CommFree(comm)            MPI_SUCCESS

/* Hook for debuggers (totalview) on created processes */
#define MPID_HAS_PROC_INFO
#define MPID_getpid(i,n,e) p2p_proc_info((i),(n),(e))

#endif
