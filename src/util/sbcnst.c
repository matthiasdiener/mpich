/*
 *  $Id: sbcnst.c,v 1.10 1994/12/11 16:53:05 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char SCCSid[] = "%W% %G%";
#endif

#include <stdio.h>
#define _SBCNSTDEF
#include "mpiimpl.h"
#include "mpisys.h"

/* #define DEBUG */

/*
   This file contains routines for allocating a number of fixed-sized blocks.
   This is often a good way to improve the performance of dynamic-memory
   codes, at the expense of some additional space.  However, unlike a purely
   static allocation (a fixed maximum), this mechanism allows space to grow.

   The basic interface is

  sb = MPIR_SBinit( blocksize, initialnumber, incrementnumber );
  ptr = MPIR_SBalloc( sb );
  ...
  MPIR_SBfree( sb, ptr );
  ...
  MPIR_SBdestroy( sb );
 */

#if defined(MPIR_DEBUG_MEM) || defined(MPIR_MEMDEBUG)
#undef MPIR_SBinit
#undef MPIR_SBalloc
#undef MPIR_SBfree
#undef MPIR_SBdestroy
#endif

/* This is the allocation unit. */
typedef struct _sbialloc {
    struct _sbialloc *next;
    int              nbytes, nballoc;
    int              nbinuse;
    } MPIR_SBiAlloc;

/* Blocks are linked together; they are (much) larger than this */
#ifdef DEBUG
typedef struct {
    long sentinal_1;
    char *next;
    long sentinal_2;
    } MPIR_SBblock;
#else
typedef struct {
    char *next;
    } MPIR_SBblock;
#endif

/* Context for fixed-block allocator */
typedef struct {
    MPIR_SBiAlloc *blocks;	         /* allocated storage */
    MPIR_SBblock  *avail;             /* fixed blocks (of size sizeb) to provide */
    int     nbfree, nballoc,     /* blocks free and in use */
            sizeb,               /* sizes in bytes */
            sizeincr;            /* # of blocks to allocate when more needed */
    } MPIR_SBHeader;

void MPIR_SBiAllocate();

MPIR_SBHeader *MPIR_SBinit( bsize, nb, nbincr )
int bsize, nb, nbincr;
{
MPIR_SBHeader *head;

/* Make sure that the blocksizes are multiples of pointer size */
if (bsize < sizeof(MPIR_SBblock)) bsize = sizeof(MPIR_SBblock);

head           = NEW(MPIR_SBHeader);  
if (!head) {
   MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, "Not enough space" );
   return 0;
   }
head->nbfree   = 0;
head->nballoc  = 0;
head->sizeb    = bsize;
head->sizeincr = nbincr;
head->avail    = 0;
head->blocks   = 0;
if (nb > 0) {
    MPIR_SBiAllocate( head, bsize, nb );
    if (!head->avail) {
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, 
		    "Failed to allocate space" );
	head = 0;
	}
    }

return head;
}

/* 
    MPIR_SBfree - return a fixed-sized block to the allocator
 */    
void MPIR_SBfree( sb, ptr )
MPIR_SBHeader *sb;
void     *ptr;
{
((MPIR_SBblock *)ptr)->next = (char *)(sb->avail);
sb->avail              = (MPIR_SBblock *)ptr;
#ifdef DEBUG
((MPIR_SBblock *)ptr)->sentinal_1 = 0xdeadbeef;
((MPIR_SBblock *)ptr)->sentinal_2 = 0xbeeffeed;
#endif
sb->nbfree++;
sb->nballoc--;
}

/*
    Internal routine to allocate space
 */
void MPIR_SBiAllocate( sb, bsize, nb )
MPIR_SBHeader *sb;
int      bsize, nb;
{
char     *p, *p2;
int      i, headeroffset, n;
MPIR_SBiAlloc *header;

/* printf( "Allocating %d blocks of size %d\n", nb, bsize ); */
/* Double-align block */
headeroffset    = (sizeof(MPIR_SBiAlloc) + sizeof(double) - 1) / sizeof(double);
headeroffset    *= sizeof(double);

sb->avail       = 0;
p               = (char *) MALLOC( bsize * nb + headeroffset );
if (!p) {
   MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, "Not enough space" );
   return;
   }
/* zero the data FOR DEBUGGING
   LATER CHANGE THIS TO SOME INVALID POINTER VALUE */
n = bsize * nb + headeroffset;
for (i=0; i<n; i++) 
    p[i] = 0;

header          = (MPIR_SBiAlloc *)p;
/* Place at header for list of allocated blocks */
header->next    = sb->blocks;
sb->blocks      = header;
header->nbytes  = bsize * nb;
header->nballoc = nb;
header->nbinuse = nb;

/* Link the list together */
p2 = p + headeroffset;
for (i=0; i<nb-1; i++) {
    ((MPIR_SBblock *)p2)->next = p2 + bsize;
#ifdef DEBUG
    ((MPIR_SBblock *)p2)->sentinal_1 = 0xdeadbeef;
    ((MPIR_SBblock *)p2)->sentinal_2 = 0xbeeffeed;
#endif
    p2 += bsize;
    }
((MPIR_SBblock *)p2)->next = (char *)sb->avail;
#ifdef DEBUG
((MPIR_SBblock *)p2)->sentinal_1 = 0xdeadbeef;
((MPIR_SBblock *)p2)->sentinal_2 = 0xbeeffeed;
#endif
sb->avail  = (MPIR_SBblock *)(p + headeroffset);
sb->nbfree += nb;
}

/* 
    MPIR_SBalloc - Gets a block from the fixed-block allocator.

    Input Parameter:
.   sb - Block context (from MPIR_SBinit)

    Returns:
    Address of new block.  Allocates more blocks if required.
 */
void *MPIR_SBalloc( sb )
MPIR_SBHeader *sb;
{
MPIR_SBblock *p;
	
if (!sb->avail) {
    MPIR_SBiAllocate( sb, sb->sizeb, sb->sizeincr );   /* nbincr instead ? */
    if (!sb->avail) {
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_EXHAUSTED, "Not enough space" );
	return 0;
	}
    }
p         = sb->avail;
#ifdef DEBUG
if (p->sentinal_1 != 0xdeadbeef) {
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, "Corrupted memory (1)!" );
    }
if (p->sentinal_2 != 0xbeeffeed) {
    MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, "Corrupted memory (2)!" );
    }
#endif
sb->avail = (MPIR_SBblock *)(p->next);
sb->nballoc++;
sb->nbfree--;
/* printf( "Allocating a block at address %x\n", (char *)p ); */
return (void *)p;
}	

/* 
    MPIR_SBPrealloc - Insure that at least nb blocks are available

    Input Parameters:
.   sb - Block header
.   nb - Number of blocks that should be preallocated

    Notes:
    This routine insures that nb blocks are available, not that an
    additional nb blocks are allocated.  This is appropriate for the common
    case where the preallocation is being used to insure that enough space
    is available for a new object (e.g., a sparse matrix), reusing any
    available blocks.
 */
void MPIR_SBPrealloc( sb, nb )
MPIR_SBHeader *sb;
int      nb;
{
if (sb->nbfree < nb) {
    MPIR_SBiAllocate( sb, sb->sizeb, nb - sb->nbfree );
    }	
}

/* 
    MPIR_SBdestroy - Destroy a fixed-block allocation context

 */
void MPIR_SBdestroy( sb )
MPIR_SBHeader *sb;
{
MPIR_SBiAlloc *p, *pn;

p = sb->blocks;
while (p) {
    pn = p->next;
    FREE( p );
    p = pn;
    }
FREE( sb );
}

/* Decrement the use count for the block containing p */
void MPIR_SBrelease( sb, ptr )
MPIR_SBHeader *sb;
void     *ptr;
{
char *p = (char *)ptr;
MPIR_SBiAlloc *b = sb->blocks;
char *first, *last;

/* printf( "Releasing a block at address %x\n", (char *)ptr ); */
while (b) {
    first = ((char *)b) + sizeof(MPIR_SBiAlloc) - 1;
    last  = first + b->nbytes - 1;
    if (p >= first && p <= last) {
	b->nbinuse--;
	break;
	}
    b = b->next;
    }
}

/* Release any unused chuncks */
void MPIR_SBFlush( sb )
MPIR_SBHeader *sb;
{
MPIR_SBiAlloc *b, *bnext, *bprev = 0;

b = sb->blocks;
while (b) {
    bnext = b->next;
    if (b->nbinuse == 0) {
	if (bprev) bprev->next = bnext;
	else       sb->blocks  = bnext;
	sb->nballoc -= b->nballoc;
	FREE( b );
	}
    else 
	bprev = b;
    b = bnext;
    }
}

/* Print the allocated blocks */
void MPIR_SBDump( fp, sb )
FILE     *fp;
MPIR_SBHeader *sb;
{
MPIR_SBiAlloc *b = sb->blocks;

while (b) {
    fprintf( fp, "Block %x of %d bytes and %d chuncks in use\n", 
	     (char *)b, b->nbytes, b->nbinuse );
    b = b->next;
    }
}

void MPIR_SBReleaseAvail( sb )
MPIR_SBHeader *sb;
{
MPIR_SBblock *p, *pnext;
	
p         = sb->avail;
while (p) {
    pnext = (MPIR_SBblock *)(p->next);
    sb->avail = pnext;
    sb->nbfree--;
    MPIR_SBrelease( sb, p );
    p     = pnext;
    }
}

#ifdef DEBUG
/* Check that the sb space remains valid ... */
void MPIR_SBvalid( sb )
MPIR_SBHeader *sb;
{
MPIR_SBblock *p;
	
p         = sb->avail;
while (p) {
    if (p->sentinal_1 != 0xdeadbeef) {
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, "Corrupted memory (3)!" );
	}
    if (p->sentinal_2 != 0xbeeffeed) {
	MPIR_ERROR( MPI_COMM_WORLD, MPI_ERR_OTHER, "Corrupted memory (4)!" );
	}
    p     = p->next;
    }
#endif
