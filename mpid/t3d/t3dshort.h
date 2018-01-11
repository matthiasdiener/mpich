#ifndef __T3DSHORT_H__
#define __T3DSHORT_H__


#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "reqalloc.h"
#include "packets.h"
#include "t3dpriv.h"

/* Prototypes */
int MPID_T3D_Eagerb_send_short(void*,int,int,int,int,int,MPID_Msgrep_t);
int MPID_T3D_Eagerb_isend_short(void*,int,int,int,int,int,MPID_Msgrep_t,MPIR_SHANDLE*);
int MPID_T3D_Eagerb_recv_short(MPIR_RHANDLE*,int,void*);
int MPID_T3D_Eagerb_save_short(MPIR_RHANDLE*,int,void*);
int MPID_T3D_Eagerb_unxrecv_start_short(MPIR_RHANDLE*,void*);
void MPID_T3D_Eagerb_short_delete(MPID_Protocol*);
MPID_Protocol *MPID_T3D_Short_setup();

#endif
