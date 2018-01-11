#ifndef CHCONFIG
#define CHCONFIG

/* Special configuration information goes here */

#define MPID_CommInit(oldcomm,newcomm) MPI_SUCCESS
#define MPID_CommFree(comm)            MPI_SUCCESS

#endif
