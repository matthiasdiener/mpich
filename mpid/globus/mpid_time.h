#ifndef MPID_Wtime
#include "globus_gram_myjob.h"
#include "globus_nexus.h"

/* Special clock for Nexus */

#define MPID_Wtime(t) *(t) = globus_libc_wallclock()

#define MPID_Wtick(t) MPID_CH_Wtick(t)

void MPID_CH_Wtick(double *t);

#endif /* MPID_Wtime */
