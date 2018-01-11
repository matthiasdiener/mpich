/*
 *  $Id: ptrcvt.c,v 1.12 1996/06/07 15:12:34 gropp Exp $
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

/* 
   This file contains routines to convert to and from pointers

   There is another approach that can be used on some systems.  This is
   to identify the 32bit range that is used by pointers, and to do the
   appropriate masks/shifts to make these valid integers.  This requires
   that the pointers actually lie in some 4 GB part of a 64 bit 
   integer, and that this segment is known in advance.  Because ensuring
   that the conditions are upheld requires friendly relations with the
   OS and runtime library developers, we cannot make use of this 
   in the portable system, but it may be valuable for specific ports.

   Thanks to Jim Cownie for this suggestion.

   In order to improve the ability of this code to handle large numbers
   of conversions, I'm switching to the following strategy:

   There is an array PtrToIdxPtr[] of pointers to blocks of size 2**k
   (k = 10 for now).  In addition, the initial block is preallocated.
   When a block runs out of room, we add one to the PtrToIdxPtr[];
   given an index, we use the high bits to find the entry in the array and
   to low bits to index into the block.

   To avoid the extra indirection, we could test for a 0 index and go 
   directly to the preallocated first block.
 */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
/* Else we should define malloc somehow... */
#endif
#include "mpiimpl.h"
#ifdef MPI_ADI2
#include "tr2.h"
#else
#include "mpisys.h"
#endif

typedef struct _PtrToIdx {
    int idx;
    void *ptr;
    struct _PtrToIdx *next;
    } PtrToIdx;
/* These 4 go together and must be changed consistently */
#define MAX_PTRS 1024
#define PTR_MASK (0x3ff)
#define PTR_IDX(i) ((i) >> 10)
#define BLOCK_IDX(i) ((i) << 10)

#define MAX_BLOCKS 256
/* #define MAX_PTRS 10000 */

static PtrToIdx *(PtrBlocks[MAX_BLOCKS]);
static PtrToIdx PtrArray[MAX_PTRS];
static PtrToIdx *avail=0;
static int      DoInit = 1;
/* Perm_In_use lets us know how many permanent, system owned indices there are.
  */
static int      Perm_In_Use = 0;
static int      PermPtr = 0;

void MPIR_PointerPerm( flag )
int flag;
{
    PermPtr = flag;
}

static void MPIR_InitPointer ANSI_ARGS((void))
{
    int  i;

    for (i=0; i<MAX_PTRS-1; i++) {
	PtrArray[i].next = PtrArray + i + 1;
	PtrArray[i].idx  = i;
    }
    PtrArray[MAX_PTRS-1].next = 0;
    for (i=1; i<MAX_BLOCKS; i++) 
	PtrBlocks[i] = 0;
    PtrBlocks[0] = PtrArray;
    
    /* Don't start with the first one, whose index is 0. That could
       break some code. */
    avail   = PtrArray + 1;
}

void *MPIR_ToPointer( idx )
int idx;
{
    int blockidx, blocknum;

    if (DoInit) {
	DoInit = 0;
	MPIR_InitPointer();
    }

    blocknum = PTR_IDX(idx);
    blockidx = idx & PTR_MASK;
    if (blocknum < 0 || blocknum >= MAX_BLOCKS ||
	blockidx < 0 || blockidx >= MAX_PTRS || !PtrBlocks[blocknum]) {
	MPIR_ERROR_PUSH_ARG(&idx);
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_BAD_INDEX, "Error in MPI object" );
	return (void *)0;
    }
    if (blocknum == 0 && blockidx == 0) return (void *)0;

    return PtrBlocks[blocknum][idx].ptr;
}

/* Create an index from a pointer */
int MPIR_FromPointer( ptr )
void *ptr;
{
    int      idx, blocknum, i;
    PtrToIdx *new;

    if (DoInit) {
	DoInit = 0;
	MPIR_InitPointer();
    }
    if (PermPtr) Perm_In_Use++;

    if (!ptr) return 0;
    if (avail) {
	new		  = avail;
	avail		  = avail->next;
	new->next	  = 0;
	idx		  = new->idx;
	PtrArray[idx].ptr = ptr;
	return idx;
    }

    /* Allocate a new block and add it to the PtrBlock array */
    for (blocknum=1; blocknum<MAX_BLOCKS; blocknum++) {
	if (!PtrBlocks[blocknum]) break;
    }
    if (blocknum == MAX_BLOCKS) {
	/* This isn't the right thing to do, but it isn't too bad */
	(void) MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INDEX_EXHAUSTED, 
			   "Error in MPI object" );
    }
    PtrBlocks[blocknum] = (PtrToIdx *)MALLOC( sizeof(PtrToIdx) * MAX_PTRS );
    if (!PtrBlocks[blocknum]) {
	(void) MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INDEX_EXHAUSTED, 
			   "Error in MPI object" );
    }
    for (i=0; i<MAX_PTRS-1; i++) {
	PtrBlocks[blocknum][i].next = &PtrBlocks[blocknum][i+1];
	PtrBlocks[blocknum][i].idx  = BLOCK_IDX(blocknum) | i;
    }
    PtrBlocks[blocknum][MAX_PTRS-1].next = 0;
    new = &PtrBlocks[blocknum][0];
    new->next	  = 0;
    idx		  = new->idx;
    new->ptr      = ptr;
    avail = &PtrBlocks[blocknum][1];

    return idx;
}

void MPIR_RmPointer( idx )
int idx;
{
    int      blocknum, blockidx;
    PtrToIdx *ptridx;

    if (DoInit) {
	DoInit = 0;
	MPIR_InitPointer();
    }

    blocknum = PTR_IDX(idx);
    blockidx = idx & PTR_MASK;
    if (blocknum < 0 || blocknum >= MAX_BLOCKS ||
	blockidx < 0 || blockidx >= MAX_PTRS || !PtrBlocks[blocknum]) {
	MPIR_ERROR_PUSH_ARG(&idx);
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_BAD_INDEX, "Error in MPI object" );
	return;
    }
    if (blocknum == 0 && blockidx == 0) return;

    ptridx = PtrBlocks[blocknum];
    if (ptridx[blockidx].next) {
	/* In-use pointers NEVER have next set */
	MPIR_ERROR_PUSH_ARG(&idx);
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_INDEX_FREED, 
		    "Error in MPI object" );
	return;
    }
    ptridx[blockidx].next = avail;
    ptridx[blockidx].ptr  = 0;
    avail		  = PtrArray + blockidx;
}

int MPIR_UsePointer( fp )
FILE *fp;
{
    int      count, perm_in_use;
    int      in_use, allocated_blocks;
    PtrToIdx *p;
    
    if (DoInit) return 0;

    /* Find the number of blocks */
    for (allocated_blocks=1; allocated_blocks < MAX_BLOCKS; 
	 allocated_blocks++) 
	if (!PtrBlocks[allocated_blocks]) break;
   
   /* Find out how many are not in use */
    count	= 0;
    perm_in_use	= 0;
    p		= avail;
    while (p) {
	count++;
	p = p->next;
	/* This test protects against a corrupted list */
	if (count > MAX_PTRS * allocated_blocks + 1) break;
    }
    /* The number in use is MAX_PTRS * allocated_blocks - count - 1 
       (The -1 is because index 0 is never made available) */
    if (count > MAX_PTRS * allocated_blocks) {
	FPRINTF( fp, "# Pointer conversions corrupted!\n" );
	return count;
    }
    in_use = MAX_PTRS * allocated_blocks - count - 1 - Perm_In_Use;
    if (in_use > 0 && fp) {
	FPRINTF( fp, 
	 "# There are %d pointer conversions in use (used for Fortran)\n", 
		 in_use );
    }
    return in_use;
}
