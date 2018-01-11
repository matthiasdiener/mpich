/*
   This file contains routines that are private and unique to the ch_shmem
   implementation
 */

#include "mpid.h"
#include "mpiddev.h"

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;
/* LOCAL copy of some of MPID_shmem */
MPID_SHMEM_lglobmem MPID_lshmem;
int                 MPID_myid = -1;
int                 MPID_numids = 0;
MPID_PKT_T          *MPID_local = 0;
VOLATILE MPID_PKT_T **MPID_incoming = 0;
static int	    MPID_pktflush;

#if defined (MPI_cspp)
/* These are special fields for the Convex SPP NUMA */
unsigned int			procNode[MPID_MAX_PROCS];
unsigned int			numCPUs[MPID_MAX_NODES];
unsigned int			numNodes;
int				MPID_myNode;
int                 		masterid;
extern int			cnx_yield;
extern int			cnx_debug;
extern char			*cnx_exec;
#endif

void				MPID_SHMEM_lbarrier  ANSI_ARGS((void));
void                            MPID_SHMEM_FreeSetup ANSI_ARGS((void));

/*
   Get an integer from the environment; otherwise, return defval.
 */
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

#if defined (MPI_cspp)

    extern char *getenv();
    char *envVarBuf;
    unsigned int mmapRound;
    cnx_node_t myNode;
    unsigned int totalCPUs;
    unsigned int curNode, numCurNode;

    MPID_SHMEM_setflags();

    MPID_SHMEM_getSCTopology(&myNode, &numNodes, &totalCPUs, numCPUs);
    if (cnx_debug) {
	printf("CNXDB: %d nodes, %d CPUs\n", numNodes, totalCPUs);
	printf("CNXDB: root node = %d\n", myNode);
	for (i = 0; i < numNodes; ++i) {
	    printf("CNXDB: node %d -> %d CPUs\n", i, numCPUs[i]);
	}
    }

#endif

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
	    MPID_ArgSqueeze( argc, argv );
	    break;
	}
    }

#if defined (MPI_cspp)

    envVarBuf = getenv("MPI_TOPOLOGY");
    MPID_SHMEM_processTopologyInfo(envVarBuf, myNode,
				   &numprocs, numNodes, numCPUs, 1);

    if (numprocs == 0) {
	fprintf(stderr, "no processes specified\n");
	exit(1);
    }

/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );

    if (memsize < (sizeof(MPID_SHMEM_globmem) + numprocs * 65536))
	memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 65536;
    mmapRound = sysconf(_SC_PAGE_SIZE) * numNodes;
    memsize = ((memsize + mmapRound - 1) / mmapRound) * mmapRound;

#else

    if (numprocs <= 0 || numprocs > MPID_MAX_PROCS) {
	fprintf( stderr, "Invalid number of processes (%d) invalid\n", numprocs );
	exit( 1 );
    }

/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );

    if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 128)
	memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 128;

#endif

    p2p_init( numprocs, memsize );

    MPID_shmem = p2p_shmalloc(sizeof(MPID_SHMEM_globmem));

    if (!MPID_shmem) {
	fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
		 sizeof( MPID_SHMEM_globmem ) );
	exit(1);
    }

/* Initialize the shared memory */

    MPID_shmem->barrier.phase = 1;
    MPID_shmem->barrier.cnt1  = numprocs;
    MPID_shmem->barrier.cnt2  = 0;
    MPID_shmem->barrier.size  = numprocs;

    p2p_lock_init( &MPID_shmem->globlock );
    cnt	      = 0;    /* allocated packets */

#if defined (MPI_cspp)
    for (i = j = 0; i < numNodes; ++i) {
	MPID_shmem->globid[i] = j;
	j += numCPUs[i];
	p2p_lock_init(&(MPID_shmem->globid_lock[i]));
    }
#else
    MPID_shmem->globid = 0;
#endif

/* The following is rough if numprocs doesn't divide the MAX_PKTS */
    pkts_per_proc = MPID_SHMEM_MAX_PKTS / numprocs;

    pkts_per_proc = 4;
/*
 * Determine the packet flush count at runtime.
 * (delay the harsh reality of resource management :-) )
 */
    MPID_pktflush = (pkts_per_proc > numprocs) ? pkts_per_proc / numprocs : 1;

#if defined (MPI_cspp)
    if (cnx_debug) printf("CNXDB: packet flush count = %d\n", MPID_pktflush);
#endif

    for (i=0; i<numprocs; i++) {
	/* setup the local copy of the addresses of objects in MPID_shmem */
	MPID_lshmem.availlockPtr[i]	   = &MPID_shmem->availlock[i];
	MPID_lshmem.incominglockPtr[i] = &MPID_shmem->incominglock[i];
	MPID_lshmem.incomingPtr[i]	   = &MPID_shmem->incoming[i];
	MPID_lshmem.availPtr[i]	   = &MPID_shmem->avail[i];

	/* Initialize the shared memory data structures */
	MPID_shmem->incoming[i].head     = 0;
	MPID_shmem->incoming[i].tail     = 0;

    /* Setup the avail list of packets */
	MPID_shmem->avail[i].head = MPID_shmem->pool + cnt;
	for (j=0; j<pkts_per_proc; j++) {
	    MPID_shmem->pool[cnt+j].head.next = 
		((MPID_PKT_T *)MPID_shmem->pool) + cnt + j + 1;
/*	    MPID_shmem->pool[cnt+j].head.src = i; */
	    MPID_shmem->pool[cnt+j].head.owner = i;
	}
	/* Clear the last "next" pointer */
	MPID_shmem->pool[cnt+pkts_per_proc-1].head.next = 0;
	cnt += pkts_per_proc;

	p2p_lock_init( MPID_shmem->availlock + i );
	p2p_lock_init( MPID_shmem->incominglock + i );
    }

#if defined (MPI_cspp)
/*
 * Place processes on nodes.
 */
    for (i = 0, curNode = numCurNode = 0; i < numprocs; ++i) {

	while (numCurNode >= numCPUs[curNode]) {
	    if (++curNode == numNodes) {
		fprintf(stderr,
			"Cannot place proc %d (out of %) on a node!\n",
			i, numprocs);
		exit(1);
	    }
	    numCurNode = 0;
	}

	procNode[i] = curNode;
	if (cnx_debug) printf("CNXDB: rank %d -> node %d\n", i, curNode);
	++numCurNode;
    }

#endif

    MPID_numids = numprocs;
    MPID_MyWorldSize = numprocs;
/* Above this point, there was a single process.  After the p2p_create_procs
   call, there are more */
    p2p_setpgrp();

#if defined (MPI_cspp)
    p2p_create_procs( numprocs, *argc, argv );

    MPID_myNode = myNode = (int) MPID_SHMEM_getNodeId();
    p2p_lock(&(MPID_shmem->globid_lock[myNode]));
    MPID_myid = (MPID_shmem->globid[myNode])++;
    p2p_unlock(&(MPID_shmem->globid_lock[myNode]));

#else
    p2p_create_procs( numprocs - 1, *argc, argv );

    p2p_lock( &MPID_shmem->globlock );
    MPID_myid = MPID_shmem->globid++;
    p2p_unlock( &MPID_shmem->globlock );
#endif

    MPID_MyWorldRank = MPID_myid;
    MPID_SHMEM_FreeSetup();

    MPID_incoming = &MPID_shmem->incoming[MPID_myid].head;

#if defined (MPI_cspp)
    if (cnx_exec) cnx_start_tool(cnx_exec, argv[0]);
#endif
}

void MPID_SHMEM_lbarrier()
{
    VOLATILE int *cnt, *cntother;

/* Figure out which counter to decrement */
    if (MPID_shmem->barrier.phase == 1) {
	cnt	     = &MPID_shmem->barrier.cnt1;
	cntother = &MPID_shmem->barrier.cnt2;
    }
    else {
	cnt	     = &MPID_shmem->barrier.cnt2;
	cntother = &MPID_shmem->barrier.cnt1;
    }

/* Decrement it atomically */
    p2p_lock( &MPID_shmem->globlock );
    *cnt = *cnt - 1;
    p2p_unlock( &MPID_shmem->globlock );
    
/* Wait for everyone to to decrement it */
    while ( *cnt ) p2p_yield();

/* If process 0, change phase. Reset the OTHER counter*/
    if (MPID_myid == 0) {
	MPID_shmem->barrier.phase = ! MPID_shmem->barrier.phase;
	*cntother = MPID_shmem->barrier.size;
    }
    else 
	while (! *cntother) p2p_yield();
}

void MPID_SHMEM_finalize()
{
    VOLATILE int *globid;

    fflush(stdout);
    fflush(stderr);

/* There is a potential race condition here if we want to catch
   exiting children.  We should probably have each child indicate a successful
   termination rather than this simple count.  To reduce this race condition,
   we'd like to perform an MPI barrier before clearing the signal handler.

   However, in the current code, MPID_xxx_End is called after most of the
   MPI system is deactivated.  Thus, we use a simple count-down barrier.
   Eventually, we the fast barrier routines.
 */
/* MPI_Barrier( MPI_COMM_WORLD ); */
    MPID_SHMEM_lbarrier();
    p2p_clear_signal();

/* Wait for everyone to finish 
   We can NOT simply use MPID_shmem->globid here because there is always the 
   possibility that some process is already exiting before another process
   has completed starting (and we've actually seen this behavior).
   Instead, we perform an additional MPI Barrier.
*/
    MPID_SHMEM_lbarrier();
/* MPI_Barrier( MPI_COMM_WORLD ); */
#ifdef FOO
    globid = &MPID_shmem->globid;
    p2p_lock( &MPID_shmem->globlock );
    MPID_shmem->globid--;
    p2p_unlock( &MPID_shmem->globlock );
/* Note that this forces all jobs to spin until everyone has exited */
    while (*globid) p2p_yield(); /* MPID_shmem->globid) ; */
#endif

    p2p_cleanup();
}

/* 
  Read an incoming control message.

  NOTE:
  This routine maintains and internal list of elements; this allows it to
  read from that list without locking it.
 */
/* #define BACKOFF_LMT 1048576 */
#define BACKOFF_LMT 1024
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
	if (!MPID_lshmem.incomingPtr[MPID_myid]->head) {
	    /* This code tries to let other processes run.  If there
	       are more physical processors than processes, then a simple
	       while (!MPID_shmem->incoming[MPID_myid].head);
	       might be better.
	       We might also want to do

	       VOLATILE MPID_PKT_T *msg_ptr = 
	       &MPID_shmem->incoming[MPID_myid].head;
	       while (!*msg_ptr) { .... }

	       This code should be tuned with vendor help, since it depends
	       on fine details of the hardware and system.
	       */
#if defined(MPI_cspp)
	    if (cnx_yield) {
#endif
		backoff = 1;
		while (!MPID_lshmem.incomingPtr[MPID_myid]->head) {
		    cnt	    = backoff;
		    while (cnt--) ;
		    backoff = 2 * backoff;
		    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
		    if (MPID_lshmem.incomingPtr[MPID_myid]->head) break;
		    p2p_yield();
		}
#if defined(MPI_cspp)
	    } else {
		while (!MPID_lshmem.incomingPtr[MPID_myid]->head);
	    }
#endif
	}
	/* This code drains the ENTIRE list into a local list */
	p2p_lock( MPID_lshmem.incominglockPtr[MPID_myid] );
	inpkt          = (MPID_PKT_T *) *MPID_incoming;
	MPID_local     = inpkt->head.next;
	*MPID_incoming = 0;
	MPID_lshmem.incomingPtr[MPID_myid]->tail = 0;
	p2p_unlock( MPID_lshmem.incominglockPtr[MPID_myid] );
    }

/* Deliver this packet to the caller */
    *pkt  = inpkt;

    *from = (*inpkt).head.src;

    MPID_TRACE_CODE_PKT("Readpkt",*from,(*inpkt).head.mode);

    return MPI_SUCCESS;
}


/*
   Rather than free recv packets every time, we accumulate a few
   and then return them in a group.  

   This is useful when a processes sends several messages to the same
   destination.
   
   This keeps a list for each possible processor, and returns them
   all when MPID_pktflush are available FROM ANY SOURCE.
 */
static MPID_PKT_T *FreePkts[MPID_MAX_PROCS];
static MPID_PKT_T *FreePktsTail[MPID_MAX_PROCS];
static int to_free = 0;

void MPID_SHMEM_FreeSetup()
{
    int i;
    for (i=0; i<MPID_numids; i++) FreePkts[i] = 0;
}
void MPID_SHMEM_FreeRecvPkt( pkt )
MPID_PKT_T *pkt;
{
    int        src, i;
    MPID_PKT_T *tail;

    MPID_TRACE_CODE_PKT("Freepkt",pkt->head.owner,(pkt->head.mode));

    src	       = pkt->head.owner;
/*
    if (src == MPID_MyWorldRank) {
	pkt->head.next = MPID_localavail;
	MPID_localavail = pkt;
	return;
    }
    */
    pkt->head.next = FreePkts[src];
/* Set the tail if we're the first */
    if (!FreePkts[src])
	FreePktsTail[src] = pkt;
    FreePkts[src]  = pkt;
    to_free++;

    if (to_free >= MPID_pktflush) {
	for (i=0; i<MPID_numids; i++) {
	    if ((pkt = FreePkts[i])) {
		tail			  = FreePktsTail[i];
		p2p_lock( MPID_lshmem.availlockPtr[i] );
		tail->head.next		  = 
		    (MPID_PKT_T *)MPID_lshmem.availPtr[i]->head;
		MPID_lshmem.availPtr[i]->head = pkt;
		p2p_unlock( MPID_lshmem.availlockPtr[i] );
		FreePkts[i] = 0;
	    }
	}
	to_free = 0;
    }
}

/* 
   If this is set, then packets are allocated, then set, then passed to the
   sendcontrol routine.  If not, packets are created on the call stack and
   then copied to a shared-memory packet.

   We should probably make "localavail" global, then we can use a macro
   to allocate packets as long as there is a local supply of them.

   For example, just
       extern MPID_PKT_T *MPID_localavail = 0;
   and the
   #define ...GetSendPkt(inpkt) \
   {if (MPID_localavail) {inpkt= MPID_localavail; \
   MPID_localavail=inpkt->head.next;}else inpkt = routine();\
   inpkt->head.next = 0;}
 */
MPID_PKT_T *MPID_SHMEM_GetSendPkt(nonblock)
int nonblock;
{
    MPID_PKT_T *inpkt;
    static MPID_PKT_T *localavail = 0;
    int   freecnt=0;

    if (localavail) {
	inpkt      = localavail;
    }
    else {
	/* If there are no available packets, this code does a yield */
	while (1) {
	    p2p_lock( MPID_lshmem.availlockPtr[MPID_myid] );
	    inpkt			     = 
		(MPID_PKT_T *)MPID_lshmem.availPtr[MPID_myid]->head;
	    MPID_lshmem.availPtr[MPID_myid]->head = 0;
	    p2p_unlock( MPID_lshmem.availlockPtr[MPID_myid] );
	    /* If we found one, exit the loop */
	    if (inpkt) break;

	    /* No packet.  Wait a while (if possible).  If we do this
	       several times without reading a packet, try to drain the
	       incoming queues 
	     */
#ifdef MPID_DEBUG_ALL
	    if (!freecnt) {
		    MPID_TRACE_CODE("No freePkt",MPID_myid);
		}
#endif
	    /* If not blocking, just return a null packet.  Not used 
	       currently (?) */
	    if (nonblock) return(inpkt);
	    freecnt++;
	    p2p_yield();
	    if ((freecnt % 8) == 0) {
		MPID_DeviceCheck( MPID_NOTBLOCKING );
	    }

        }
    }
    localavail	 = inpkt->head.next;
    inpkt->head.next = 0;

    MPID_TRACE_CODE_PKT("Allocsendpkt",-1,inpkt->head.mode);

    return inpkt;
}

int MPID_SHMEM_SendControl( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
    MPID_PKT_T *tail;

/* Place the actual length into the packet */
    MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt->head.mode);

    pkt->head.src     = MPID_myid;
    pkt->head.next    = 0;           /* Should already be true */

    p2p_lock( MPID_lshmem.incominglockPtr[dest] );
    tail = (MPID_PKT_T *)MPID_lshmem.incomingPtr[dest]->tail;
    if (tail) 
	tail->head.next = pkt;
    else
	MPID_lshmem.incomingPtr[dest]->head = pkt;

    MPID_lshmem.incomingPtr[dest]->tail = pkt;
    p2p_unlock( MPID_lshmem.incominglockPtr[dest] );

    return MPI_SUCCESS;
}

#if 0 && defined(MPI_cspp)
#include <sys/cnx_sysinfo.h>
#include <sys/cnx_pattr.h>
void SY_GetHostName( name, nlen )
int  nlen;
char *name;
{
    cnx_is_target_t target;
    char  p[1024], *addr = p;
    struct pattributes pattrib;

/* This sets the target to get process info for my process */
    cnx_sysinfo_target_process( &target, getpid() );

/* An id of CNX_IS_PROCESS_BASIC_INFO gets information on the process, but 
   not the location of the process */
    cnx_sysinfo( id, target, addr, nel, lel, elavail );

/* Get the subcomplex id into pattrib.pattr_scid */
    cnx_getpattr( getpid(), CNX_PATTR_SCID, &pattrib );

/* Now that we have the subcomplex id, we use sysinfo to get more info about 
   it (but can we get the information about a SINGLE node, or does this
   tell us about the whole subcomplex? 
   */
    cnx_sysinfo_target_( &target, pattrib.pattr_scid );
    cnx_sysinfo( CNS_IS_SCNODE_BASIC_INFO, &scnode_info, .... );
    scnode_info.node , .physical_node, cpus[CNX_MAX_CPUS_PER_NODE]
	}
#else

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
#endif
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

    MPID_TRACE_CODE("Allocating shared space",len);
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
#ifdef FOO
/* If this mapped the address space, we wouldn't need to copy anywhere */
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
#endif

    MPID_TRACE_CODE("Allocated space at",(long)new );
    return new;
}

void MPID_FreeGetAddress( addr )
void *addr;
{
    MPID_TRACE_CODE("Freeing space at",(long)addr );
    p2p_shfree( addr );
}

