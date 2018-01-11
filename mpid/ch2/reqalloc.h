#ifndef MPIREQALLOC
#define MPIREQALLOC

/* Allocation of handles */
#include "sbcnst2.h"

extern MPID_SBHeader MPIR_shandles;
extern MPID_SBHeader MPIR_rhandles;
#ifdef MPIR_MEMDEBUG 
#define MPID_RecvAlloc() (MPIR_RHANDLE *)MALLOC(sizeof(MPIR_RHANDLE))
#define MPID_SendAlloc() (MPIR_SHANDLE *)MALLOC(sizeof(MPIR_SHANDLE))
#define MPID_PRecvAlloc() (MPIR_PRHANDLE *)MALLOC(sizeof(MPIR_PRHANDLE))
#define MPID_PSendAlloc() (MPIR_PSHANDLE *)MALLOC(sizeof(MPIR_PSHANDLE))
#define MPID_RecvFree( a ) FREE( a )
#define MPID_SendFree( a ) FREE( a )
#define MPID_PRecvFree( a ) FREE( a )
#define MPID_PSendFree( a ) FREE( a )
#else
#define MPID_RecvAlloc() (MPIR_RHANDLE *)MPID_SBalloc(MPIR_rhandles)
#define MPID_SendAlloc() (MPIR_SHANDLE *)MPID_SBalloc(MPIR_shandles)
#define MPID_PRecvAlloc() (MPIR_PRHANDLE *)MPID_SBalloc(MPIR_rhandles)
#define MPID_PSendAlloc() (MPIR_PSHANDLE *)MPID_SBalloc(MPIR_shandles)
#define MPID_RecvFree( a ) MPID_SBfree( MPIR_rhandles, a )
#define MPID_SendFree( a ) MPID_SBfree( MPIR_shandles, a )
#define MPID_PRecvFree( a ) MPID_SBfree( MPIR_rhandles, a )
#define MPID_PSendFree( a ) MPID_SBfree( MPIR_shandles, a )
#endif

#endif
