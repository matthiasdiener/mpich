/*
 *  $Id: ptrcvt.c,v 1.6 1995/05/09 18:56:56 gropp Exp $
 *
 *  (C) 1994 by Argonne National Laboratory and Mississipi State University.
 *      See COPYRIGHT in top-level directory.
 */

/* 
   This file contains routines to convert to and from pointers
 */

#include <stdio.h>
#include "mpiimpl.h"
#include "mpisys.h"

typedef struct _PtrToIdx {
    int idx;
    void *ptr;
    struct _PtrToIdx *next;
    } PtrToIdx;
#define MAX_PTRS 10000

static PtrToIdx PtrArray[MAX_PTRS];
static PtrToIdx *avail=0;
static int      DoInit = 1;

static void MPIR_InitPointer()
{
int  i;

for (i=0; i<MAX_PTRS-1; i++) {
    PtrArray[i].next = PtrArray + i + 1;
    PtrArray[i].idx  = i;
    }
PtrArray[MAX_PTRS-1].next = 0;
/* Don't start with the first one, whose index is 0. That could
   break some code. */
avail   = PtrArray + 1;
}

void *MPIR_ToPointer( idx )
int idx;
{
if (DoInit) {
    DoInit = 0;
    MPIR_InitPointer();
    }
if (idx < 0 || idx >= MAX_PTRS) {
    fprintf( stderr, "Could not convert index %d into a pointer\n", idx );
    exit(1);
    }
if (idx == 0) return (void *)0;
return PtrArray[idx].ptr;
}

int MPIR_FromPointer( ptr )
void *ptr;
{
int      idx;
PtrToIdx *new;
if (DoInit) {
    DoInit = 0;
    MPIR_InitPointer();
    }
if (!ptr) return 0;
if (avail) {
    new		      = avail;
    avail	      = avail->next;
    new->next	      = 0;
    idx		      = new->idx;
    PtrArray[idx].ptr = ptr;
    return idx;
    }
/* This isn't the right thing to do, but it isn't too bad */
fprintf( stderr, "Pointer conversions exhausted\n" );
exit(1);
/* Some systems may complain here about the lack of a return.  */
return 0;
}

void MPIR_RmPointer( idx )
int idx;
{
int myrank;
if (DoInit) {
    DoInit = 0;
    MPIR_InitPointer();
    }
if (idx < 0 || idx >= MAX_PTRS) {
    fprintf( stderr, "Could not convert index %d into a pointer\n", idx );
    exit(1);
    }
if (idx == 0) return;
if (PtrArray[idx].next) {
    /* In-use pointers NEVER have next set */
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    fprintf( stderr, 
	    "[%d] Error in recovering Fortran pointer; already freed\n", 
	    myrank );
    return;
    }
PtrArray[idx].next = avail;
PtrArray[idx].ptr  = 0;
avail              = PtrArray + idx;
}

int MPIR_UsePointer( fp )
FILE *fp;
{
int count;
int in_use;
PtrToIdx *p;

if (DoInit) return 0;

/* Find out how many are not in use */
count = 0;
p = avail;
while (p) {
    count++;
    p = p->next;
    if (count > MAX_PTRS + 1) break;
    }
/* The number in use is MAX_PTRS - count - 1 
   (The -1 is because index 0 is never made available) */
if (count > MAX_PTRS) {
    fprintf( fp, "# Pointer conversions corrupted!\n" );
    return count;
    }
in_use = MAX_PTRS - count - 1;
if (in_use && fp) {
    fprintf( fp, 
	    "# There are %d pointer conversions in use (used for Fortran)\n", 
	    in_use );
    }
return in_use;
}
