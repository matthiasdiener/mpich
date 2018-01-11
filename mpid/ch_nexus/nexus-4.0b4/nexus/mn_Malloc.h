#ifndef	mn_Malloc_h
#define	mn_Malloc_h

/*
 *	Project:	High Speed Network ASR
 *	Organization:	The Aerospace Corporation
 *	Programmer:	Robert Lindell
 *	File:		$RCSfile: mn_Malloc.h,v $
 *	Date:		$Date: 1996/10/07 04:43:46 $
 *	Version:	$Revision: 1.3 $
 */

#include <stdlib.h>
#include <unistd.h>
/* #include <sysent.h> */
#include <memory.h>
#include <sys/types.h>

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 30

struct mn_Malloc {
	int realloc_srchlen;
	caddr_t buf;
	caddr_t top;
	unsigned len;
	union overhead *nextf[NBUCKETS];
	int pagesz;			/* page size */
	int pagebucket;			/* page size bucket */
};

typedef struct mn_Malloc Malloc;


void                    construct_mn_Malloc( Malloc**, caddr_t, unsigned );
void                    destruct_mn_Malloc( Malloc* );
char*			mn_malloc( Malloc*, unsigned );
char*			mn_calloc( Malloc*, unsigned, unsigned );
char*			mn_realloc( Malloc*, char*, unsigned );
void			mn_free( Malloc*, char* );
caddr_t			get_mn_buffer( Malloc* );
void			set_mn_buffer( Malloc*, caddr_t );
unsigned		mn_get_length( Malloc* );
void			mn_set_length( Malloc*, unsigned );
void                    mn_mstats( Malloc* );
void			mn_morecore( Malloc*, int );
int			mn_findbucket( Malloc*, union overhead*, int );
caddr_t			mn_sbrk( Malloc*, int );


#endif	/* mn_Malloc_h */
