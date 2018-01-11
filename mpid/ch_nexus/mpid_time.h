#ifndef MPID_Wtime
#include "nexus.h"

/* Special clock for Nexus */

#define MPID_Wtime(t) *(t) = nexus_wallclock()

#define MPID_Wtick(t) MPID_CH_Wtick(t)

void MPID_CH_Wtick(double *t);

#endif /* MPID_Wtime */
