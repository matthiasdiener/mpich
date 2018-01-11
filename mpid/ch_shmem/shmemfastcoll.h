/*
 *	Convex SPP
 *	Copyright 1995 Convex Computer Corp.
 *	$CHeader: shmemfastcoll.h 1.1 1995/11/08 14:00:00 $
 *
 *	Function:	- fast collective calls header file
 */

#ifndef FASTCOLL_INCL
#define FASTCOLL_INCL

/*
 * constants
 */
#define CACHELINESIZE	64

/*
 * structures
 */
typedef struct {
	int		*flag;
	int		*ival;
	void		**addr;
	double		*dval;
	float		*rval;
} MPID_Barf;

typedef struct {
	void		*barrier;
	int		np;
	int		mypid;
	int		nc;
	int		myhypernode;
	int		same_node;
	MPID_Barf	*barf;
} MPID_Fastbar;

typedef struct {
	int		bsize;
	int		*blockcounts;
	int		*displs;
} MPID_Fastcoll;

/*
 * functions
 */

extern int		MPID_SHMEM_Wait_barrier();
extern void		*MPID_SHMEM_Init_barrier();
extern void		MPID_SHMEM_Free_barrier();
extern void		MPID_SHMEM_Free_collect();
extern int		MPID_SHMEM_First_barrier();

#endif
