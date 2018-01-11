#ifndef MPID_BIND
#define MPID_BIND
/* These are the bindings of the ADI2 routines */

void MPID_Init ANSI_ARGS(( int *, char ***, void *, int *));
void MPID_End  ANSI_ARGS((void));
void MPID_Abort ANSI_ARGS(( MPI_Comm, int, char *, char * ));
int  MPID_DeviceCheck ANSI_ARGS(( MPID_BLOCKING_TYPE ));
void MPID_Node_name ANSI_ARGS(( char *, int ));
int  MPID_WaitForCompleteSend ANSI_ARGS((MPIR_SHANDLE *));
int  MPID_WaitForCompleteRecv ANSI_ARGS((MPIR_RHANDLE *));
void MPID_Version_name ANSI_ARGS((char *));

/* SetPktSize is used by util/cmnargs.c */
void MPID_SetPktSize ANSI_ARGS(( int ));

void MPID_RecvContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, 
				 MPI_Status *, int * ));
void MPID_IrecvContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, 
				  MPI_Request, int * ));
void MPID_RecvComplete ANSI_ARGS(( MPI_Request, MPI_Status *, int *));
int  MPID_RecvIcomplete ANSI_ARGS(( MPI_Request, MPI_Status *, int *));

void MPID_SendContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, int, 
				 MPID_Msgrep_t, int * ));
void MPID_BsendContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, 
				  int, MPID_Msgrep_t, int * ));
void MPID_SsendContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int,
				  int, MPID_Msgrep_t, int * ));
void MPID_IsendContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, int, 
				 MPID_Msgrep_t, MPI_Request, int * ));
void MPID_IssendContig ANSI_ARGS(( MPI_Comm, void *, int, int, int, int, int, 
				   MPID_Msgrep_t, MPI_Request, int * ));
void MPID_SendComplete ANSI_ARGS(( MPI_Request, int *));
int  MPID_SendIcomplete ANSI_ARGS(( MPI_Request, int *));

void MPID_Probe ANSI_ARGS(( MPI_Comm, int, int, int, int *, MPI_Status * ));
void MPID_Iprobe ANSI_ARGS(( MPI_Comm, int, int, int, int *, int *, 
			     MPI_Status * ));

void MPID_SendCancel ANSI_ARGS(( MPI_Request, int * ));
void MPID_RecvCancel ANSI_ARGS(( MPI_Request, int * ));

/* General MPI Datatype routines  */
void MPID_SendDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				   int, int, int, int, int * ));
void MPID_SsendDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				    int, int, int, int, int * ));
void MPID_IsendDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				    int, int, int, int, MPI_Request, int * ));
void MPID_IssendDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				     int, int, int, int, MPI_Request, int * ));
void MPID_RecvDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				   int, int, int, MPI_Status *, int * ));
void MPID_IrecvDatatype ANSI_ARGS(( MPI_Comm, void *, int, MPI_Datatype, 
				   int, int, int, MPI_Request, int * ));

/* Pack and unpack support */
void MPID_Msg_rep ANSI_ARGS(( MPI_Comm, int, MPI_Datatype, MPID_Msgrep_t *, 
			      MPID_Msg_pack_t * ));
void MPID_Msg_act ANSI_ARGS(( MPI_Comm, int, MPI_Datatype, MPID_Msgrep_t, 
			      MPID_Msg_pack_t * ));
void MPID_Pack_size ANSI_ARGS(( int, MPI_Datatype, MPID_Msg_pack_t, int * ));
void MPID_Pack ANSI_ARGS(( void *, int, MPI_Datatype, 
			   void *, int, int *, MPI_Comm, int, 
			   MPID_Msgrep_t, MPID_Msg_pack_t, int * ));
void MPID_Unpack ANSI_ARGS(( void *, int, MPID_Msgrep_t, int *, 
			     void *, int, MPI_Datatype, int *, 
			     MPI_Comm, int, int * ));

/* Requests */
void MPID_Request_free ANSI_ARGS((MPI_Request));

/* 
 * These are debugging commands; they are exported so that the command-line
 * parser and other routines can control the debugging output
 */
void MPID_SetDebugFile ANSI_ARGS(( char * ));
void MPID_Set_tracefile ANSI_ARGS(( char * ));
void MPID_SetSpaceDebugFlag ANSI_ARGS(( int ));
void MPID_SetDebugFlag ANSI_ARGS(( int ));
#endif