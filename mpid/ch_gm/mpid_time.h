/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef MPID_Wtime

/* Special clock for ch_gm */

/* #define MPID_Wtime(t) *(t) = gmpi_wtime() */
#define MPID_Wtime(t) MPID_CH_Wtime(t)
#define MPID_Wtick(t) MPID_CH_Wtick(t)
extern double gmpi_wtime (void);
void MPID_CH_Wtick ( double * );
#endif
