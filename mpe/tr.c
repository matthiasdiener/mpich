#ifndef lint
static char vcid[] = "$Id: tr.c,v 1.3 1994/07/25 20:40:36 karrels Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include "tools.h"
#undef TRSPACE

#ifdef __MSDOS__
#ifdef __TURBOC__
#include <alloc.h>
#define malloc farmalloc
#ifndef MALLOC_DEFINED
#define MALLOC_DEFINED
#endif
#endif
#endif

#ifndef MALLOC_DEFINED
extern char *malloc();
#endif


/*D
    trspace - Routines for tracing space usage

    Description:
    trmalloc replaces malloc and trfree replaces free.  These routines
    have the same syntax and semantics as the routines that they replace,
    In addition, there are routines to report statistics on the memory
    usage, and to report the currently allocated space.  These routines
    are built on top of malloc and free, and can be used together with
    them as long as any space allocated with trmalloc is only freed with
    trfree.
 D*/

/* HEADER_DOUBLES is the number of doubles in a trSPACE header */
/* We have to be careful about alignment rules here */
#if defined(dec_alpha) || defined(cray) || defined(MPI_dec_alpha) || defined(MPI_cray)
#define TR_ALIGN_BYTES 8
#define TR_ALIGN_MASK  0x7
#define TR_FNAME_LEN   16
#define HEADER_DOUBLES 8
#else
#define TR_ALIGN_BYTES 4
#define TR_ALIGN_MASK  0x3
#define TR_FNAME_LEN   12
#define HEADER_DOUBLES 5
#endif

#define COOKIE_VALUE   0xf0e0d0c9
#define ALREADY_FREED  0x0f0e0d9c
/* we can make fname 16 on dec_alpha without taking any more space, because of
   the alignment rules */
typedef struct _trSPACE {
    unsigned long   size;
    int             id;
    int             lineno;
    char            fname[TR_FNAME_LEN];
    unsigned long   cookie;        
    struct _trSPACE *next, *prev;
    } TRSPACE;
/* This union is used to insure that the block passed to the user is
   aligned on a double boundary */
typedef union {
    TRSPACE sp;
    double  v[HEADER_DOUBLES];
    } TrSPACE;

static long allocated = 0, frags = 0;
static TRSPACE *TRhead = 0;
static int     TRid = 0;
static int     TRlevel = 0;
#define MAX_TR_STACK 20
static int     TRstack[MAX_TR_STACK];
static int     TRstackp = 0;
static int     TRdebugLevel = 0;
#define TR_MALLOC 0x1
#define TR_FREE   0x2

/* Used to keep track of allocations */
static long    TRMaxMem = 0;
static long    TRMaxMemId = 0;

/*
    trmalloc - Malloc with tracing

    Input Parameters:
.   a   - number of bytes to allocate
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    double aligned pointer to requested storage, or null if not
    available.
  */
void *trmalloc( a, lineno, fname )
unsigned int a;
int      lineno;
char     *fname;
{
TRSPACE          *head;
char             *new;
unsigned long    *nend;
unsigned int     nsize;
int              l;

if (TRdebugLevel > 0) {
    char buf[256];
    sprintf( buf, "Invalid MALLOC arena detected at line %d in %s\n", 
	     lineno, fname );
    if (trvalid( buf )) return 0;
    }

nsize = a;
if (nsize & TR_ALIGN_MASK) 
    nsize += (TR_ALIGN_BYTES - (nsize & TR_ALIGN_MASK));
new = malloc( (unsigned)( nsize + sizeof(TrSPACE) + sizeof(unsigned long) ) );
if (!new) return 0;

head = (TRSPACE *)new;
new  += sizeof(TrSPACE);

if (TRhead)
    TRhead->prev = head;
head->next     = TRhead;
TRhead         = head;
head->prev     = 0;
head->size     = nsize;
head->id       = TRid;
head->lineno   = lineno;
if ((l = strlen( fname )) > TR_FNAME_LEN-1 ) fname += (l - (TR_FNAME_LEN-1));
strncpy( head->fname, fname, (TR_FNAME_LEN-1) );
head->fname[TR_FNAME_LEN-1]= 0;
head->cookie   = COOKIE_VALUE;
nend           = (unsigned long *)(new + nsize);
nend[0]        = COOKIE_VALUE;

allocated += nsize;
if (allocated > TRMaxMem) {
    TRMaxMem   = allocated;
    TRMaxMemId = TRid;
    }

frags     ++;

if (TRlevel & TR_MALLOC) 
    fprintf( stderr, "Allocating %d bytes at %lx\n", a, new );
return (void *)new;
}

/*
   trfree - Free with tracing

   Input Parameters:
.  a    - pointer to a block allocated with trmalloc
.  line - line in file where called
.  file - Name of file where called
  */
void trfree( a, line, file )
char *a;
int  line;
char *file;
{
TRSPACE *head;
char    *ahead;
unsigned long *nend;

/* Don't try to handle empty blocks */
if (!a) return;

if (TRdebugLevel > 0) {
    if (trvalid( "Invalid MALLOC arena detected by FREE" )) return;
    }

ahead = a;
a     = a - sizeof(TrSPACE);
head  = (TRSPACE *)a;
if (head->cookie != COOKIE_VALUE) {
    /* Damaged header */
    fprintf( stderr, "Block at address %lx is corrupted; cannot free;\n\
may be block not allocated with trmalloc or MALLOC\n", a );
    SETERR(1);
    return;
    }
nend = (unsigned long *)(ahead + head->size);
if (*nend != COOKIE_VALUE) {
    if (*nend == ALREADY_FREED) {
	fprintf( stderr, 
  "Block [id=%d(%d)] at address %lx was already freed\n", 
		head->id, head->size, a + sizeof(TrSPACE) );
	head->fname[TR_FNAME_LEN-1]= 0;  /* Just in case */
	if (head->lineno > 0) 
	    fprintf( stderr, 
		    "Block freed in %s[%d]\n", head->fname, head->lineno );
	else
	    fprintf( stderr, 
	         "Block allocated at %s[%d]\n", head->fname, - head->lineno );
	SETERR(1);
	return;
	}
    else {
	/* Damaged tail */
	fprintf( stderr, 
  "Block [id=%d(%d)] at address %lx is corrupted (probably write past end)\n", 
		head->id, head->size, a );
	head->fname[TR_FNAME_LEN-1]= 0;  /* Just in case */
	fprintf( stderr, 
		"Block allocated in %s[%d]\n", head->fname, head->lineno );
	SETERR(1);
	}
    }
/* Mark the location freed */
*nend        = ALREADY_FREED;
/* Save location where freed.  If we suspect the line number, mark as 
   allocated location */
if (line > 0 && line < 5000) {
    head->lineno = line;
    strncpy( head->fname, file, (TR_FNAME_LEN-1) );
    }
else {
    head->lineno = - head->lineno;
    }

allocated -= head->size;
frags     --;
if (head->prev)
    head->prev->next = head->next;
else
    TRhead = head->next;

if (head->next)
    head->next->prev = head->prev;
if (TRlevel & TR_FREE)
    fprintf( stderr, "Freeing %d bytes at %lx\n", 
	             head->size, a + sizeof(TrSPACE) );
free( a );
}

/*
   trvalid - test the allocated blocks for validity.  This can be used to
   check for memory overwrites.

   Input Parameter:
.  str - string to write out only if an error is detected.

   Return value:
   The number of errors detected.
   
   Output Effect:
   Error messages are written to stdout.  These have the form of either

$   Block [id=%d(%d)] at address %lx is corrupted (probably write past end)
$   Block allocated in <filename>[<linenumber>]

   if the sentinal at the end of the block has been corrupted, and

$   Block at address %lx is corrupted

   if the sentinal at the begining of the block has been corrupted.

   The address is the actual address of the block.  The id is the
   value of TRID.

   No output is generated if there are no problems detected.
*/
int trvalid( str )
char *str;
{
TRSPACE *head;
char    *a;
unsigned long *nend;
int     errs = 0;

head = TRhead;
while (head) {
    if (head->cookie != COOKIE_VALUE) {
	if (!errs) fprintf( stderr, "%s\n", str );
	fprintf( stderr, "Block at address %lx is corrupted\n", head );
	return errs;
	}
    a    = (char *)(((TrSPACE*)head) + 1);
    nend = (unsigned long *)(a + head->size);
    if (nend[0] != COOKIE_VALUE) {
	if (!errs) fprintf( stderr, "%s\n", str );
	errs++;
	head->fname[TR_FNAME_LEN-1]= 0;  /* Just in case */
	fprintf( stderr, 
  "Block [id=%d(%d)] at address %lx is corrupted (probably write past end)\n", 
	     head->id, head->size, a );
	fprintf( stderr, 
		"Block allocated in %s[%d]\n", head->fname, head->lineno );
	}
    head = head->next;
    }
return errs;
}

/*
   trspace - Return space statistics
   
   Output parameters:
.   space - number of bytes currently allocated
.   frags - number of blocks currently allocated
  */
void trspace( space, fr )
int *space, *fr;
{
*space = allocated;
*fr    = frags;
}

/*
  trdump - Dump the allocated memory blocks to a file

  Input Parameter:
.  fp  - file pointer.  If fp is NULL, stderr is assumed.
 */
void trdump( fp )
FILE *fp;
{
TRSPACE *head;
int     id;

if (fp == 0) fp = stderr;
head = TRhead;
while (head) {
    fprintf( fp, "%d at [%lx], id = ", 
	     head->size, head + sizeof(TrSPACE) );
    if (head->id >= 0) {
	head->fname[TR_FNAME_LEN-1] = 0;
	fprintf( fp, "%d %s[%d]\n", head->id, head->fname, head->lineno );
	}
    else {
	/* Decode the package values */
	head->fname[TR_FNAME_LEN-1] = 0;
	id = head->id;
	if (id >= BCTRID)      { fprintf( fp, "BComm " ); id -= BCTRID; }
	else if (id >= SPTRID) { fprintf( fp, "Sparse " ); id -= SPTRID; }
	else if (id >= CMTRID) { fprintf( fp, "COMM " ); id -= CMTRID; }
	else if (id >= ITTRID) { fprintf( fp, "ITER " ); id -= ITTRID; }

	if (id >= TRIDPOOL)    { fprintf( fp, "Pool " ); id -= TRIDPOOL; }
	else if (id >= TRIDCREATE) { fprintf( fp, "Create " ); 
				     id -= TRIDCREATE; }
	fprintf( fp, "%d %s[%d]\n", id, head->fname, head->lineno );
	}
    head = head->next;
    }
fprintf( fp, "The maximum space allocated was %d bytes [%d]\n", 
	 TRMaxMem, TRMaxMemId );
}

#if !defined(__MSDOS__) && !defined(fx2800) && !defined(tc2000) && !defined(MPI_fx2800) && \
    !defined(MPI_tc2000) && !defined(NeXT) && !defined(c2mp) && !defined(intelnx) && \
    !defined(BSD386) && !defined(MPI_NeXT) && !defined(MPI_c2mp) && !defined(MPI_intelnx) && \
    !defined(MPI_BSD386)
/* The following routine uses the tsearch routines to summarize the
   memory heap by id */
/* rs6000 needs _XOPEN_SOURCE to use tsearch */
#if defined(rs6000) || defined(MPI_rs6000)
#define _NO_PROTO
#define _XOPEN_SOURCE
#endif
#if defined(HPUX) || defined(MPI_HPUX)
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <search.h>
typedef struct { int id, size, lineno; char *fname; } TRINFO;
static int IntCompare( a, b )
TRINFO *a, *b;
{
return a->id - b->id;
}
static FILE *TRFP;
/*ARGSUSED*/
static void PrintSum( a, order, level )
TRINFO **a;  
VISIT  order;
int    level;
{ 
if (order == postorder || order == leaf) 
    fprintf( TRFP, "[%d]%s[%d] has %d\n", 
	     (*a)->id, (*a)->fname, (*a)->lineno, (*a)->size );
}

/*
  trSummary - Summarize the allocate memory blocks by id

  Input Parameter:
.  fp  - file pointer

  Note:
  This routine is the same as trDump on those systems that do not include
  /usr/include/search.h .
 */
void trSummary( fp )
FILE *fp;
{
TRSPACE *head;
TRINFO  *root, *key, **fnd;
TRINFO  nspace[1000];

root = 0;
head = TRhead;
key  = nspace;
while (head) {
    key->id     = head->id;
    key->size   = 0;
    key->lineno = head->lineno;
    key->fname  = head->fname;
#if !defined(IRIX) && !defined(solaris) && !defined(HPUX) && \
    !defined(MPI_IRIX) && !defined(MPI_solaris) && !defined(MPI_HPUX) && \
    !defined(MPI_meiko)
    fnd    = (TRINFO **)tsearch( (char *) key, (char **) &root, IntCompare );
#else
    fnd    = (TRINFO **)tsearch( (void *) key, (void **) &root, IntCompare );
#endif
    if (*fnd == key) {
	key->size = 0;
	key++;
	}
    (*fnd)->size += head->size;
    head = head->next;
    }



/* Print the data */
TRFP = fp;

#if !defined(IRIX) && !defined(solaris) && !defined(HPUX) && \
    !defined(MPI_IRIX) && !defined(MPI_solaris) && !defined(MPI_HPUX) && \
    !defined(MPI_meiko)
twalk( (char *)root, PrintSum );
#else
twalk( (void *)root, PrintSum );
#endif
fprintf( fp, "The maximum space allocated was %d bytes [%d]\n", 
	 TRMaxMem, TRMaxMemId );
}
#else
void trSummary( fp )
FILE *fp;
{
fprintf( fp, "The maximum space allocated was %ld bytes [%ld]\n", 
	 TRMaxMem, TRMaxMemId );
}	
#endif

/*
  trid - set an "id" field to be used with each fragment
 */
void trid( id )
int id;
{
TRid = id;
}

/*
  trlevel - Set the level of output to be used by the tracing routines
 
  Input Parameters:
. level = 0 - notracing
. level = 1 - trace mallocs
. level = 2 - trace frees

  Note:
  You can add levels together to get combined tracing.
 */
void trlevel( level )
int level;
{
TRlevel = level;
}

/*
   trpush - Push an "id" value for the tracing space routines

   Input Parameters:
.  a      - value to push
*/
void trpush( a )
int a;
{
if (TRstackp < MAX_TR_STACK - 1)
    TRstack[++TRstackp] = a;
TRid = a;
}

/*
  trpop - Pop an "id" value for the tracing space routines
*/
void trpop()
{
if (TRstackp > 1) {
    TRstackp--;
    TRid = TRstack[TRstackp];
    }
else
    TRid = 0;
}

/*
    trDebugLevel - set the level of debugging for the space management routines

    Input Parameter:
.   level - level of debugging.  Currently, either 0 (no checking) or 1
    (use trvalid at each trmalloc or trfree).
*/
void trDebugLevel( level )
int level;
{
TRdebugLevel = level;
}

/*
    trcalloc - Calloc with tracing

    Input Parameters:
.   nelem  - number of elements to allocate
.   elsize - size of each element
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    Double aligned pointer to requested storage, or null if not
    available.
 */
void *trcalloc( nelem, elsize, lineno, fname )
unsigned nelem, elsize;
int      lineno;
char     *fname;
{
void *p;

p = trmalloc( (unsigned)(nelem*elsize), lineno, fname );
if (!p) {
    MEMSET(p,0,nelem*elsize);
    }
return p;
}

/*
    trrealloc - Realloc with tracing

    Input Parameters:
.   p      - pointer to old storage
.   size   - number of bytes to allocate
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    Double aligned pointer to requested storage, or null if not
    available.  This implementation ALWAYS allocates new space and copies 
    the contents into the new space.
 */
void *trrealloc( p, size, lineno, fname )
void *p;
int  size, lineno;
char *fname;
{
void    *pnew;
char    *pa;
int     nsize;
TRSPACE *head;

pnew = trmalloc( (unsigned)size, lineno, fname );
if (!pnew) return p;

/* We should really use the size of the old block... */
pa   = (char *)p;
head = (TRSPACE *)(pa - sizeof(TRSPACE));
if (head->cookie != COOKIE_VALUE) {
    /* Damaged header */
    fprintf( stderr, "Block at address %lx is corrupted; cannot realloc;\n\
may be block not allocated with trmalloc or MALLOC\n", pa );
    SETERR(1);
    return p;
    }
nsize = size;
if (head->size < nsize) nsize = head->size;
MEMCPY( pnew, p, nsize );
FREE( p );
return pnew;
}

#define TR_MAX_DUMP 100
/*
   The following routine attempts to give useful information about the
   memory usage when an "out-of-memory" error is encountered.  The rules are:
   If there are less than TR_MAX_DUMP blocks, output those.
   Otherwise, try to find multiple instances of the same routine/line #, and
   print a summary by number:
   file line number-of-blocks total-number-of-blocks

   We have to do a sort-in-place for this
 */

/*
  Sort by file/line number.  Do this without calling a system routine or
  allocating ANY space (space is being optimized here).

  We do this by first recursively sorting halves of the list and then
  merging them.  
 */

/* Merge two lists, returning the head of the merged list */
TRSPACE *trImerge( l1, l2 )
TRSPACE *l1, *l2;
{
TRSPACE *head = 0, *tail = 0;
int     sign;
while (l1 && l2) {
    sign = strcmp(l1->fname, l2->fname);
    if (sign > 0 || (sign == 0 && l1->lineno >= l2->lineno)) {
	if (head) tail->next = l1; 
	else      head = tail = l1;
	tail = l1;
	l1   = l1->next;
	}
    else {
	if (head) tail->next = l2; 
	else      head = tail = l2;
	tail = l2;
	l2   = l2->next;
	}
    }
/* Add the remaining elements to the end */
if (l1) tail->next = l1;
if (l2) tail->next = l2;

return head;
}
/* Sort head with n elements, returning the head */
TRSPACE *trIsort( head, n )
TRSPACE *head;
int     n;
{
TRSPACE *p, *l1, *l2;
int     m, i;

if (n <= 1) return head;

/* This guarentees that m, n are both > 0 */
m = n / 2;
p = head;
for (i=0; i<m-1; i++) p = p->next;
/* p now points to the END of the first list */
l2 = p->next;
p->next = 0;
l1 = trIsort( head, m );
l2 = trIsort( l2,   n - m );
return trImerge( l1, l2 );
}

void trSortBlocks()
{
TRSPACE *head, *next;
int     cnt;

head = TRhead;
cnt  = 0;
while (head) {
    cnt ++;
    head = head->next;
    }
TRhead = trIsort( TRhead, cnt );
}

/* Takes sorted input and dumps as an aggregate */
void trdumpGrouped( fp )
FILE *fp;
{
TRSPACE *head, *cur;
int     nblocks, nbytes;

if (fp == 0) fp = stderr;

trSortBlocks();
head = TRhead;
cur  = 0;
while (head) {
    cur     = head->next;
    nblocks = 1;
    nbytes  = head->size;
    while (cur && strcmp(cur->fname,head->fname) == 0 && 
	   cur->lineno == head->lineno ) {
	nblocks++;
	nbytes += cur->size;
	cur    = cur->next;
	}
    fprintf( fp, "File %13s line %5d: %d bytes in %d allocation%c\n", 
	     head->fname, head->lineno, nbytes, nblocks, 
	     (nblocks > 1) ? 's' : ' ' );
    head = cur;
    }
fflush( fp );
}
