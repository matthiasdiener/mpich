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

acquire_lock(ip)
#ifdef USE_VOL
	int * volatile ip;
#else
	int *ip;
#endif
{
	register int count = 0;

/* printf("acquire_lock, ip %x *ip %x\n", ip, *ip); */
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
		while (!__read32(ip)) count++;
#endif
		if (__ldcws32(ip))
			break;
	}
	if (debugmask & SEM) {
printf("Lock %x acquired after %d polls\n", ip, count);
	}
	return 0;
}

release_lock(ip)
	int *ip;
{

	if (debugmask & SEM) {
printf("Releasing lock %x\n", ip);
	}
	*ip = 1;
	return 0;
}
