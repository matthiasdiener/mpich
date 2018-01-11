/*
   This file contains routines that are private and unique to the ch_shmem
   implementation
 */


#ifndef lint
static char vcid[] = "$Id: shmempriv.c,v 1.4 1995/03/07 22:34:26 gropp Exp $";
#endif

#include "mpid.h"

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;
int                 MPID_myid = -1;
int                 MPID_numids = 0;
VOLATILE MPID_PKT_T *MPID_local = 0, **MPID_incoming = 0;

void MPID_SHMEM_init( argc, argv )
int  *argc;
char **argv;
{
int numprocs, i;
int cnt, j, pkts_per_proc;
int memsize;

numprocs = 0;
for (i=1; i<*argc; i++) {
    if (strcmp( argv[i], "-np" ) == 0) {
	numprocs = atoi( argv[i+1] );
	break;
	}
    }
if (numprocs <= 0 || numprocs > MPID_MAX_PROCS) {
    fprintf( stderr, "Invalid number of processes or not specified with \n\
-np number\n" );
    exit( 1 );
    }
memsize = MPID_MAX_SHMEM;
if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 65536)
    memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 65536;
p2p_init( numprocs, memsize );

MPID_shmem = p2p_shmalloc( sizeof( MPID_SHMEM_globmem ) );
if (!MPID_shmem) {
    fprintf( stderr, "Could not allocated shared memory (%d bytes)!\n",
	     sizeof( MPID_SHMEM_globmem ) );
    exit(1);
    }

/* Initialize the shared memory */
MPID_shmem->globid = 0;
p2p_lock_init( &MPID_shmem->globlock );
cnt	      = 0;    /* allocated packets */
/* The following is rough if numprocs doesn't divide the MAX_PKTS */
pkts_per_proc = MPID_SHMEM_MAX_PKTS / numprocs;

for (i=0; i<numprocs; i++) {
    MPID_shmem->incoming[i]     = 0;
    MPID_shmem->incomingtail[i] = 0;

    /* Setup the avail list of packets */
    MPID_shmem->avail[i] = MPID_shmem->pool + cnt;
    for (j=0; j<pkts_per_proc-1; j++) {
	MPID_shmem->pool[cnt+j].head.next = 
	  ((MPID_PKT_T *)MPID_shmem->pool) + cnt + j + 1;
	}
    MPID_shmem->pool[cnt+pkts_per_proc-1].head.next = 0;
    cnt += pkts_per_proc;

    p2p_lock_init( MPID_shmem->availlock + i );
    p2p_lock_init( MPID_shmem->incominglock + i );
    }

MPID_numids = numprocs;
p2p_create_procs( numprocs - 1 );

p2p_lock( &MPID_shmem->globlock );
MPID_myid = MPID_shmem->globid++;
p2p_unlock( &MPID_shmem->globlock );

MPID_incoming = MPID_shmem->incoming + MPID_myid;
}

void MPID_SHMEM_finalize()
{
fflush(stdout);
fflush(stderr);
/* Wait for everyone to finish */
p2p_lock( &MPID_shmem->globlock );
MPID_shmem->globid--;
p2p_unlock( &MPID_shmem->globlock );
while (MPID_shmem->globid) ;

p2p_cleanup();
}

/* 
  Read an incoming control message.

  NOTE:
  This routine maintains and internal list of elements; this allows it to
  read from that list without locking it.
 */
#define BACKOFF_LMT 1048576
/* 
   If this is set, then packets are not allocated and are just passed back 
   from the readcontrol routine.  If not, packets are created on the call 
   stack and copied from a shared-memory packet.

   NOTE THE DIFFERENCES IN BINDINGS
 */
#ifdef MPID_PKT_DYNAMIC_RECV
int MPID_SHMEM_ReadControl( pkt, size, from )
MPID_PKT_T **pkt;
int        size, *from;
{
MPID_PKT_T *inpkt;
int        backoff, cnt;

if (MPID_local) {
    inpkt      = (MPID_PKT_T *)MPID_local;
    MPID_local = MPID_local->head.next;
    }
else {
    if (!MPID_shmem->incoming[MPID_myid]) {
	/* This code tries to let other processes get to run.  If there
	   are more physical processors than processes, then a simple
	   while (!MPID_shmem->incoming[MPID_myid]);
	   might be better */
	backoff = 1;
	while (!MPID_shmem->incoming[MPID_myid]) {
	    cnt	    = backoff;
	    while (cnt--) ;
	    backoff = 2 * backoff;
	    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
	    if (MPID_shmem->incoming[MPID_myid]) break;
	    p2p_yield();
	    }
	}
    /* This code could drain the ENTIRE list into a local list */
    p2p_lock( &MPID_shmem->incominglock[MPID_myid] );
    inpkt          = (MPID_PKT_T *) *MPID_incoming;
    MPID_local     = inpkt->head.next;
    *MPID_incoming = 0;
    MPID_shmem->incomingtail[MPID_myid] = 0;
    p2p_unlock( &MPID_shmem->incominglock[MPID_myid] );
    }

/* Deliver this packet to the caller */
*pkt  = inpkt;

*from = (*inpkt).head.src;

MPID_TRACE_CODE_PKT("Readpkt",*from,inpkt);

return MPI_SUCCESS;
}
void MPID_SHMEM_FreeRecvPkt( pkt )
MPID_PKT_T *pkt;
{
int        src;

src = (*pkt).head.src;

MPID_TRACE_CODE_PKT("Freepkt",-1,pkt);
p2p_lock( &MPID_shmem->availlock[src] );
pkt->head.next       = (MPID_PKT_T *)MPID_shmem->avail[src];
MPID_shmem->avail[src] = pkt;
p2p_unlock( &MPID_shmem->availlock[src] );
}

#else
int MPID_SHMEM_ReadControl( pkt, size, from )
MPID_PKT_T *pkt;
int        size, *from;
{
MPID_PKT_T *inpkt;
int        src;
int        backoff, cnt;

if (MPID_local) {
    inpkt      = (MPID_PKT_T *)MPID_local;
    MPID_local = MPID_local->head.next;
    }
else {
    if (!MPID_shmem->incoming[MPID_myid]) {
	/* This code tries to let other processes get to run.  If there
	   are more physical processors than processes, then a simple
	   while (!MPID_shmem->incoming[MPID_myid]);
	   might be better */
	backoff = 1;
	while (!MPID_shmem->incoming[MPID_myid]) {
	    cnt	    = backoff;
	    while (cnt--) ;
	    backoff = 2 * backoff;
	    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
	    if (MPID_shmem->incoming[MPID_myid]) break;
	    p2p_yield();
	    }
	}
    /* This code could drain the ENTIRE list into a local list */
    p2p_lock( &MPID_shmem->incominglock[MPID_myid] );
    inpkt          = (MPID_PKT_T *) *MPID_incoming;
    MPID_local     = inpkt->head.next;
    *MPID_incoming = 0;
    MPID_shmem->incomingtail[MPID_myid] = 0;
    p2p_unlock( &MPID_shmem->incominglock[MPID_myid] );
    }

/* Eventually, we'll arrange to deliver the address of the packet */
MEMCPY( pkt, inpkt, (inpkt->head).pkt_len ); 

src = (*inpkt).head.src;

MPID_TRACE_CODE_PKT("Read/freepkt",src,inpkt);

p2p_lock( &MPID_shmem->availlock[src] );
inpkt->head.next       = (MPID_PKT_T *)MPID_shmem->avail[src];
MPID_shmem->avail[src] = inpkt;
p2p_unlock( &MPID_shmem->availlock[src] );

*from = src;

return MPI_SUCCESS;
}
#endif

/* 
   If this is set, then packets are allocated, then set, then passed to the
   sendcontrol routine.  If not, packets are created on the call stack and
   then copied to a shared-memory packet
 */
#ifdef MPID_PKT_DYNAMIC_SEND
MPID_PKT_T *MPID_SHMEM_GetSendPkt()
{
MPID_PKT_T *inpkt;
static MPID_PKT_T *localavail = 0;

if (localavail) {
    inpkt      = localavail;
    }
else {
    /* WARNING: THIS CODE DOES A SLEEP 
       IF THERE ARE NO AVAILABLE LOCAL PACKETS */
    do {
        p2p_lock( &MPID_shmem->availlock[MPID_myid] );
        inpkt			     = 
	  (MPID_PKT_T *)MPID_shmem->avail[MPID_myid];
        MPID_shmem->avail[MPID_myid] = 0;
        p2p_unlock( &MPID_shmem->availlock[MPID_myid] );
#ifdef MPID_DEBUG_ALL
	if (!inpkt)
	    MPID_TRACE_CODE("No freePkt",-1);
#endif
        } while (!inpkt && (sleep(1),1));
    }
localavail	 = inpkt->head.next;
inpkt->head.next = 0;

MPID_TRACE_CODE_PKT("Allocsendpkt",-1,inpkt);

return inpkt;
}

int MPID_SHMEM_SendControl( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
MPID_PKT_T *tail;

/* Place the actual length into the packet */
MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt);

pkt->head.pkt_len = size;
pkt->head.src     = MPID_myid;
pkt->head.next    = 0;           /* Should already be true */

p2p_lock( &MPID_shmem->incominglock[dest] );
tail = (MPID_PKT_T *)MPID_shmem->incomingtail[dest];
if (tail) 
    tail->head.next = pkt;
else
    MPID_shmem->incoming[dest] = pkt;

MPID_shmem->incomingtail[dest] = pkt;
p2p_unlock( &MPID_shmem->incominglock[dest] );

return MPI_SUCCESS;
}
#else
int MPID_SHMEM_SendControl( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
MPID_PKT_T *inpkt;
MPID_PKT_T *tail;
static MPID_PKT_T *localavail = 0;


/* Place the actual length into the packet */
pkt->head.pkt_len = size;

if (localavail) {
    inpkt      = localavail;
    localavail = inpkt->head.next;
    }
else {
    /* WARNING: THIS CODE DOES A SLEEP 
       IF THERE ARE NO AVAILABLE LOCAL PACKETS */
    do {
        p2p_lock( &MPID_shmem->availlock[MPID_myid] );
        inpkt			     = 
	  (MPID_PKT_T *)MPID_shmem->avail[MPID_myid];
        localavail                   = inpkt->head.next;
        MPID_shmem->avail[MPID_myid] = 0;
        p2p_unlock( &MPID_shmem->availlock[MPID_myid] );
        } while (!inpkt && (sleep(1),1));
    }
MPID_TRACE_CODE_PKT("Sendpkt",dest,inpkt);
MEMCPY( inpkt, pkt, size );
inpkt->head.next = 0;
inpkt->head.src  = MPID_myid;

p2p_lock( &MPID_shmem->incominglock[dest] );
tail = (MPID_PKT_T *)MPID_shmem->incomingtail[dest];
if (tail) 
    tail->head.next = inpkt;
else
    MPID_shmem->incoming[dest] = inpkt;

MPID_shmem->incomingtail[dest] = inpkt;
p2p_unlock( &MPID_shmem->incominglock[dest] );

return MPI_SUCCESS;
}
#endif

#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif
#ifdef HAVE_SYSINFO
#include <sys/systeminfo.h>
#endif
void SY_GetHostName( name, nlen )
int  nlen;
char *name;
{
#if defined(HAVE_GETHOSTNAME)
  gethostname(name, nlen);
#elif defined(solaris) || defined(HAVE_UNAME)
  struct utsname utname;
  uname(&utname);
  strncpy(name,utname.nodename,nlen);
#else 
  sprintf( name, "%d", MPID_MyWorldRank );
#endif
/* See if this name includes the domain */
  if (!strchr(name,'.')) {
    int  l, rc;
    l = strlen(name);
    name[l++] = '.';
    name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
    /* Note that IRIX does not support SI_SRPC_DOMAIN */
    rc = -1;
#if defined(solaris) || defined(HAVE_SYSINFO)
    rc = sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#endif
#if defined(HAVE_GETDOMAINNAME)
    if (rc == -1) 
	getdomainname( name+l, nlen - l );
#endif
  }
}

/* 
   Return the address the destination (dest) should use for getting the 
   data at in_addr.  len is INOUT; it starts as the length of the data
   but is returned as the length available, incase all of the data can 
   not be transfered 
 */
void * MPID_SetupGetAddress( in_addr, len, dest )
void *in_addr;
int  *len, dest;
{
void *new;
int  tlen = *len;

/* To test, just comment out the first line and set new to null */
new = p2p_shmalloc( tlen );
/* new = 0; */
if (!new) {
    tlen = tlen / 2; 
    while(tlen > 0 && !(new = p2p_shmalloc(tlen))) 
	tlen = tlen / 2;
    if (tlen == 0) {
	fprintf( stderr, "Could not get any shared memory for long message!" );
	exit(1);
	}
    /* fprintf( stderr, "Message too long; sending partial data\n" ); */
    *len = tlen;
    }
/*
if (MPID_DEBUG_FILE) {
    fprintf( MPID_DEBUG_FILE, 
	    "[%d]R About to copy to %d from %d (n=%d) (%s:%d)...\n", 
	    MPID_MyWorldRank, new, in_addr, tlen, 
	    __FILE__, __LINE__ );
    fflush( MPID_DEBUG_FILE );
    }
 */

MEMCPY( new, in_addr, tlen );
return new;
}

void MPID_FreeGetAddress( addr )
void *addr;
{
p2p_shfree( addr );
}

