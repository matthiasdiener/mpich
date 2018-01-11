/*	$CHeader: mem.c 1.13 94/11/17 12:09:55 $
 *	Copyright 1992 Convex Computer Corp.
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "mem.h"
#include "global.h"
#include "ddpro.h"
#include "tdpro.h"
#include <assert.h>
 */
#define SEM 0

static int debugmask;

MPID_SPP__acquire_lock(ip)
#ifdef USE_VOL
	int * volatile ip;
#else
	int *ip;
#endif
{
	register int count = 0;

/* printf("%d: acquire_lock, ip %x *ip %x\n", getpid(), ip, *ip); */
/*
	if ((int)ip & 0x000000f) {
printf("acquire_lock, bad ip %x\n", ip);
return -1;
	}
*/
	if (debugmask & SEM) {
printf("trying to acquire lock %x\n", ip);
	}
	while (1) {
#ifdef USE_VOL
		while (!(*ip)) count++;
#else
		while (!MPID_SPP__read32(ip)) count++;
#endif
		if (MPID_SPP__ldcws32(ip))
			break;
	}
	if (debugmask & SEM) {
printf("Lock %x acquired after %d polls\n", ip, count);
	}
/* printf("%d: acquired_lock, ip %x *ip %x\n", getpid(), ip, *ip); */
	return 0;
}
