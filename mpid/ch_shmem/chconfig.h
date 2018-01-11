#ifndef CHCONFIG
#define CHCONFIG

/* Special configuration information goes here */

#define MPID_CommInit(oldcomm,newcomm) MPI_SUCCESS
#define MPID_CommFree(comm)            MPI_SUCCESS

/* Hook for debuggers (totalview) on created processes */
#define MPID_HAS_PROC_INFO
#define MPID_getpid(i,n,e) p2p_proc_info((i),(n),(e))

#endif
