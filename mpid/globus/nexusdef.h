/* 
 * These are the definitions particular to the Nexus implementation.
 */

#ifndef __commNexus
#define __commNexus

#include "globus_gram_myjob.h"
#include "globus_nexus.h"

/* Convert generic datatype to Nexus version */

#define PI_NO_NSEND
#define PI_NO_NRECV

#define MPIDTRANSPORT "ch_nexus"

#define MPID_CH_InitMsgPass(argc, argv, short, long) \
    MPID_Nexus_InitMsgPass(argc, argv, short, long)

void MPID_Globus_Init(int *argc, char ***argv);

#endif /* __commNexus */