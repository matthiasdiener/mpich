#ifndef MPID_CHHETERO
#define MPID_CHHETERO

/* 
 * The following is for support of heterogeneous systems and can be ignored
 * by homogeneous implementations
 */
typedef enum { MPID_H_NONE = 0, 
		   MPID_H_LSB, MPID_H_MSB, MPID_H_XDR } MPID_H_TYPE;
/* 
   The MPID_INFO structure is acquired from each node and used to determine
   the format for data that is sent.
 */
typedef struct {
    MPID_H_TYPE byte_order;
    int         short_size, 
                int_size,
                long_size,
                float_size,
                double_size,
                float_type;
    } MPID_INFO;

extern MPID_INFO *MPID_procinfo;
extern MPID_H_TYPE MPID_byte_order;
extern int MPID_IS_HETERO;

#ifndef MPID_HAS_HETERO
/* Msgrep is simply OK for Homogeneous systems */
#define MPID_CH_Comm_msgrep( comm ) (comm)->msgform = MPID_MSG_OK;
#else
extern int MPID_CH_Comm_msgrep ANSI_ARGS(( MPI_Comm ));
#endif
extern int MPID_CH_Hetero_free ANSI_ARGS((void));

#ifdef MPID_DEVICE_CODE
#if defined(HAS_XDR) && defined(MPID_HAS_HETERO)
#include "rpc/rpc.h"
void MPID_Mem_XDR_Init ANSI_ARGS(( char *, int, enum xdr_op, XDR * ));
int MPID_Mem_XDR_len  ANSI_ARGS(( MPI_Datatype, int ));
void MPID_Mem_XDR_Free ANSI_ARGS(( XDR * ));
#endif
int MPID_Type_XDR_encode ANSI_ARGS((unsigned char *, unsigned char *, 
				    struct MPIR_DATATYPE *, int, void * ));
int MPID_Type_swap_copy ANSI_ARGS((unsigned char *, unsigned char *, 
				    struct MPIR_DATATYPE *, int, void * ));
#endif /* MPID_DEVICE_CODE */
#endif
