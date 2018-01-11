#ifndef __T3DLONG_H__
#define __T3DLONG_H__

#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "packets.h"
#include "t3dpriv.h"

#define T3D_IS_8BYTE_ALIGNED(addr)  (! ( (int)(addr) & 0x7 ) )
#define T3D_IS_4BYTE_ALIGNED(addr)  (! ( (int)(addr) & 0x3 ) )
#define T3D_IS_8BYTE_LENGTH(len)    (! ( len % 8 ) )
#define T3D_IS_4BYTE_LENGTH(len)    (! ( len % 4 ) )


void MPID_T3D_Check_Target(void *target,int length);
void MPID_T3D_Reset_stack();
int MPID_T3D_Progress_long_sends();
int MPID_T3D_Eagerb_send_long(void *buf,
			      int   len,
			      int   src_lrank,
			      int   tag,
			      int   context_id,
			      int   dest,
			      MPID_Msgrep_t msgrep);
int MPID_T3D_Eagern_isend_long(void *buf,
			       int  len,
			       int  src_lrank,
			       int  tag,
			       int  context_id,
			       int  dest,
			       MPID_Msgrep_t msgrep,
			       MPIR_SHANDLE *shandle);
int MPID_T3D_Eagerb_recv_long(MPIR_RHANDLE *rhandle,
			      int           from,
			      void         *in_pkt);
int MPID_T3D_Eagerb_save_long(MPIR_RHANDLE *rhandle,
			      int           from,
			      void         *in_pkt);
int MPID_T3D_Eagerb_unxrecv_start_long(MPIR_RHANDLE *rhandle,
				       void         *in_runex);
void MPID_T3D_Eagerb_long_delete(MPID_Protocol *p);
MPID_Protocol *MPID_T3D_Long_setup();




#endif
