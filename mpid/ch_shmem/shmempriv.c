/*
   This file contains routines that are private and unique to the ch_shmem
   implementation
 */


#ifndef lint
static char vcid[] = "$Id: shmempriv.c,v 1.8 1995/07/26 16:56:53 gropp Exp $";
#endif

#include "mpid.h"

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;
int                 MPID_myid = -1;
int                 MPID_numids = 0;
MPID_PKT_T          *MPID_local = 0;
VOLATILE MPID_PKT_T **MPID_incoming = 0;

int MPID_GetIntParameter( name, defval )
char *name;
int  defval;
{
extern char *getenv();
char *p = getenv( name );

if (p) 
    return atoi(p);
return defval;
}

void MPID_SHMEM_init( argc, argv )
int  *argc;
char **argv;
{
int numprocs, i;
int cnt, j, pkts_per_proc;
int memsize;

/* Make one process the default */
numprocs = 1;
for (i=1; i<*argc; i++) {
    if (strcmp( argv[i], "-np" ) == 0) {
	/* Need to remove both args and check for missing value for -np */
	if (i + 1 == *argc) {
	    fprintf( stderr, 
		    "Missing argument to -np for number of processes\n" );
	    exit( 1 );
	    }
	numprocs = atoi( argv[i+1] );
	argv[i] = 0;
	argv[i+1] = 0;
	MPIR_ArgSqueeze( argc, argv );
	break;
	}
    }
if (numprocs <= 0 || numprocs > MPID_MAX_PROCS) {
    fprintf( stderr, "Invalid number of processes (%d) invalid\n", numprocs );
    exit( 1 );
    }
/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );
/*
if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 65536)
    memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 65536;
 */
if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 128)
    memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 128;
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
    MPID_shmem->incoming[i].head     = 0;
    MPID_shmem->incoming[i].tail     = 0;

    /* Setup the avail list of packets */
    MPID_shmem->avail[i].head = MPID_shmem->pool + cnt;
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

/* Above this point, there was a single process.  After the p2p_create_procs
   call, there are more */
p2p_create_procs( numprocs - 1 );

p2p_lock( &MPID_shmem->globlock );
MPID_myid = MPID_shmem->globid++;
p2p_unlock( &MPID_shmem->globlock );

MPID_incoming = &MPID_shmem->incoming[MPID_myid].head;
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
   This version assumes that the packets are dynamically allocated (not off of
   the stack).  This lets us use packets that live in shared memory.

   NOTE THE DIFFERENCES IN BINDINGS from the usual versions.
 */
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
    if (!MPID_shmem->incoming[MPID_myid].head) {
	/* This code tries to let other processes get to run.  If there
	   are more physical processors than processes, then a simple
	   while (!MPID_shmem->incoming[MPID_myid].head);
	   might be better */
	backoff = 1;
	while (!MPID_shmem->incoming[MPID_myid].head) {
	    cnt	    = backoff;
	    while (cnt--) ;
	    backoff = 2 * backoff;
	    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
	    if (MPID_shmem->incoming[MPID_myid].head) break;
	    p2p_yield();
	    }
	}
    /* This code drains the ENTIRE list into a local list */
    p2p_lock( &MPID_shmem->incominglock[MPID_myid] );
    inpkt          = (MPID_PKT_T *) *MPID_incoming;
    MPID_local     = inpkt->head.next;
    *MPID_incoming = 0;
    MPID_shmem->incoming[MPID_myid].tail = 0;
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
pkt->head.next       = (MPID_PKT_T *)MPID_shmem->avail[src].head;
MPID_shmem->avail[src].head = pkt;
p2p_unlock( &MPID_shmem->availlock[src] );
}

/* 
   If this is set, then packets are allocated, then set, then passed to the
   sendcontrol routine.  If not, packets are created on the call stack and
   then copied to a shared-memory packet.

   We should probably make "localavail" global, then we can use a macro
   to allocate packets as long as there is a local supply of them.
 */
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
	  (MPID_PKT_T *)MPID_shmem->avail[MPID_myid].head;
        MPID_shmem->avail[MPID_myid].head = 0;
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
tail = (MPID_PKT_T *)MPID_shmem->incoming[dest].tail;
if (tail) 
    tail->head.next = pkt;
else
    MPID_shmem->incoming[dest].head = pkt;

MPID_shmem->incoming[dest].tail = pkt;
p2p_unlock( &MPID_shmem->incominglock[dest] );

return MPI_SUCCESS;
}

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

