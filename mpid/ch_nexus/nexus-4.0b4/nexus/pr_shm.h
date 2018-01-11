/*
 * Nexus
 * Authors:     Cheryl DeMatteis
 *              The Aerospace Corporation
 *
 * pr_shm.	- header file for shared memory protocol module
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/param.h>
#include "mn_Malloc.h"

#ifndef NEXUS_SHM_KEY
#define NEXUS_SHM_KEY   100
#endif /* NEXUS_SHM_KEY */

#ifndef NEXUS_SHM_SIZE
#define NEXUS_SHM_SIZE  PAGESIZE
#endif /* NEXUS_SHM_SIZE */

#ifndef SHM_R
#define SHM_R 0400
#endif

#ifndef SHM_W
#define SHM_W 0200
#endif


extern int shmid;
extern void* shmptr;

void     _nx_shm_usage_message(void);
