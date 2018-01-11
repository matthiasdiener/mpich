/*
 *  $Id: ptrcvt.c,v 1.5 1994/11/01 19:42:19 gropp Exp $
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
static PtrToIdx *avail=0, *head=0;
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
avail = PtrArray + 1;
head  = 0;
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
return PtrArray[idx].ptr;
}

int MPIR_FromPointer( ptr )
void *ptr;
{
int idx;
if (DoInit) {
    DoInit = 0;
    MPIR_InitPointer();
    }
if (avail) {
    idx               = avail->idx;
    avail             = avail->next;
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
if (DoInit) {
    DoInit = 0;
    MPIR_InitPointer();
    }
if (idx < 0 || idx >= MAX_PTRS) {
    fprintf( stderr, "Could not convert index %d into a pointer\n", idx );
    exit(1);
    }
PtrArray[idx].next = avail;
avail              = PtrArray + idx;
}
