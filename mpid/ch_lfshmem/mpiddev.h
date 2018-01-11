/* Definitions for the device only 
   This is an example that can can be used by channel codes
 */
#ifndef MPID_DEV_H
#define MPID_DEV_H

#if !defined(VOLATILE)
#if (HAS_VOLATILE || defined(__STDC__))
#define VOLATILE volatile
#else
#define VOLATILE
#endif
#endif

#include "dev.h"

/* Globals - For the device */
extern int          MPID_n_pending;
extern MPID_DevSet *MPID_devset;
extern MPID_INFO   *MPID_tinfo;

#include "shpackets.h"
#include "mpid_debug.h"

#define MPIDTRANSPORT "ch_lfshmem"
#define MPIDPATCHLEVEL 2.0

#include "shdef.h"

#ifdef FOO
/* LOCAL copy of some of MPID_shmem */
int                 MPID_myid = -1;
int                 MPID_numids = 0;
static int	    MPID_pktflush;
extern MPID_SHMEM_globmem  *MPID_shmem;
extern MPID_PKT_T          *MPID_local;
extern VOLATILE MPID_PKT_T **MPID_incoming;
extern MPID_SHMEM_lglobmem MPID_lshmem;
#endif

/* 
   Common macro for checking the actual length (msglen) against the
   declared max length in a handle (dmpi_recv_handle).  
   Resets msglen if it is too long; also sets err to MPI_ERR_TRUNCATE.
   This will set the error field to be added to a handle "soon" 
   (Check for truncation)

   This does NOT call the MPID_ErrorHandler because that is for panic
   situations.
 */
#define MPID_CHK_MSGLEN(rhandle,msglen,err) \
if ((rhandle)->len < (msglen)) {\
    err = MPI_ERR_TRUNCATE;\
    rhandle->s.MPI_ERROR = MPI_ERR_TRUNCATE;\
    msglen = (rhandle)->len;\
    }
#define MPID_CHK_MSGLEN2(actlen,msglen,err) \
if ((actlen) < (msglen)) {\
    err = MPI_ERR_TRUNCATE;\
    msglen = (actlen);\
    }

/* Function prototypes for routines known only to the device */
extern MPID_Device *MPID_SHMEM_InitMsgPass ANSI_ARGS(( int *, char ***, 
						    int, int ));
extern MPID_Protocol *MPID_SHMEM_Short_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Eagerb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Rndvb_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Eagern_setup ANSI_ARGS((void));
extern MPID_Protocol *MPID_SHMEM_Rndvn_setup ANSI_ARGS((void));
extern int MPID_SHMEM_Check_incoming ANSI_ARGS(( MPID_Device *, 
					      MPID_BLOCKING_TYPE));
extern int  MPID_CH_Init_hetero ANSI_ARGS(( int *, char *** ));
extern MPID_PKT_T *MPID_SHMEM_GetSendPkt ANSI_ARGS((int));
extern void *MPID_SetupGetAddress ANSI_ARGS(( void *, int *, int ));
extern void MPID_FreeGetAddress ANSI_ARGS(( void * ));
extern int MPID_PackMessageFree ANSI_ARGS((MPIR_SHANDLE *));
extern void MPID_PackMessage ANSI_ARGS((void *, int, MPI_Datatype, 
					MPI_Comm, int, int, MPID_Msg_pack_t, 
					void **, int *, int *));
extern void MPID_UnpackMessageSetup ANSI_ARGS(( int, MPI_Datatype, MPI_Comm,
						int, int, void **, int *,
						int * ));
extern int MPID_UnpackMessageComplete ANSI_ARGS(( MPIR_RHANDLE * ));
#endif
