/*
   This file contains routines that are private and unique to the ch_shmem
   implementation
 */


#ifndef lint
static char vcid[] = "$Id: shmempriv.c,v 1.4 1995/03/07 22:34:26 gropp Exp $";
#endif

#include "mpid.h"
#include <stdlib.h>

/* MPID_shmem is not volatile but its contents are */
MPID_SHMEM_globmem *MPID_shmem = 0;
MPID_SHMEM_ptrmem   MPID_pktPtr; 
int                 MPID_myid = -1;
int                 MPID_numids = 0;
VOLATILE MPID_PKT_T *MPID_local = 0, **MPID_incoming = 0;
unsigned int procNode[MPID_MAX_PROCS];

void MPID_SHMEM_init( int  *argc, char **argv )
{
  int numprocs, i;
  int cnt, j, pkts_per_proc;
  int memsize;
  char *envVarBuf;
  int allowOversub;
  unsigned int mmapRound;
  unsigned int numNodes;
  cnx_node_t myNode;
  unsigned int numCPUs[MPID_MAX_NODES];
  unsigned int totalCPUs;
  unsigned int curNode, numCurNode;
  MPID_SHMEM_pktmem  *MPID_pktshmemPtr[MPID_MAX_NODES];
  
  allowOversub = 0;
  if (envVarBuf = getenv("MPI_OVERSUB"))
    allowOversub = (envVarBuf[0] == 'Y') || (envVarBuf[0] == 'y');
  
  MPID_SPP_getSCTopology(&myNode, &numNodes, &totalCPUs, numCPUs);
  printf("my node id: %d\n", myNode);
  printf("number of nodes = %d\n", numNodes);
  printf("total number of cpus = %d\n", totalCPUs);
  for (i=0; i<numNodes; i++)
  {
    printf("number of cpus in node[%d]: %d\n", i, numCPUs[i]);
  }
  fflush(stdout);

  numprocs = 0;
  for (i=1; i<*argc; i++)
  {
    if (strcmp( argv[i], "-np" ) == 0)
    {
      numprocs = atoi( (const char *)argv[i+1] );
      if (numprocs <= 0)
      {
        fprintf( stderr, "Invalid number of processes (%d) specified with -np option\n", numprocs);
        exit(-1);
      }
      break;
    }
  }

  if (envVarBuf = getenv("MPI_TOPOLOGY")) 
    MPID_SPP_processTopologyInfo(envVarBuf, &numprocs, numNodes, numCPUs, allowOversub);

  if (numprocs == 0)
  {
    fprintf( stderr, "Number of processes not specified \n" );
    exit( -1 );
  }
  if (numprocs > totalCPUs && !allowOversub)
  {
    fprintf(stderr, "Current subcomplex is oversubscribed:\
 %d processors for %d processes\n", totalCPUs, numprocs);
    exit( -1 );
  }

  if (numCPUs[myNode] == 0)
    myNode = p2p_migrateInitialProcess(myNode, numNodes, numCPUs);
   
  for (i=0; i<numNodes; i++)
  {
    printf("number of cpus in node[%d]: %d\n", i, numCPUs[i]);
  }
  fflush(stdout);
  
  memsize = MPID_MAX_SHMEM;
  if (envVarBuf = getenv("MPI_GLOBMEMSIZE")) memsize = atoi(envVarBuf);
  if (memsize < sizeof(MPID_SHMEM_pktmem) + numprocs * 65536)
      memsize = sizeof(MPID_SHMEM_pktmem) + numprocs * 65536;
       /* round up to size of page * number of nodes */
  mmapRound = sysconf(_SC_PAGE_SIZE) * numNodes;
  memsize = (memsize + mmapRound - 1) / mmapRound * mmapRound;
  
/*   printf("shared memory size = %d\n", memsize); */
  p2p_init( memsize, numNodes );
  
  MPID_shmem = p2p_shmalloc( sizeof( MPID_SHMEM_globmem ), myNode );
  if (!MPID_shmem)
  {
    fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
             sizeof( MPID_SHMEM_globmem ) );
    exit(1);
  }
  
  /* Initialize the shared memory */
  MPID_shmem->globid = 0;
  p2p_lock_init( &MPID_shmem->globlock );
  
  for (i=0; i<numNodes; i++)
  {
    MPID_pktshmemPtr[i] = p2p_shmalloc( sizeof( MPID_SHMEM_pktmem ), i );
    if (!MPID_pktshmemPtr) {
      fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
               sizeof( MPID_SHMEM_pktmem ) );
      exit(1);
    }
  }
  
  cnt = 0;    /* allocated packets */
  /* The following is rough if numprocs doesn't divide the MAX_PKTS */
  pkts_per_proc = MPID_SHMEM_MAX_PKTS_PER_NODE / MPID_MAX_PROCS_PER_NODE;

  for (curNode = myNode, numCurNode = 0, i = 0; i < numprocs; i++)
  {
    procNode[i] = curNode;
/* printf("process %d to run on node%d\n", i, procNode[i] ); */

    MPID_pktPtr.availlockPtr[i] = 
      &MPID_pktshmemPtr[curNode]->availlock[numCurNode];
    MPID_pktPtr.incominglockPtr[i] = 
      &MPID_pktshmemPtr[curNode]->incominglock[numCurNode];
    MPID_pktPtr.availPtr[i] = 
      &MPID_pktshmemPtr[curNode]->avail[numCurNode];
    MPID_pktPtr.incomingPtr[i] = 
      &MPID_pktshmemPtr[curNode]->incoming[numCurNode];
    MPID_pktPtr.incomingtailPtr[i] = 
      &MPID_pktshmemPtr[curNode]->incomingtail[numCurNode];

    *MPID_pktPtr.incomingPtr[i] = 0;
    *MPID_pktPtr.incomingtailPtr[i] = 0;
  
      /* Setup the avail list of packets */
    *MPID_pktPtr.availPtr[i] = MPID_pktshmemPtr[curNode]->pool + cnt;
    for (j=0; j<pkts_per_proc-1; j++)
      MPID_pktshmemPtr[curNode]->pool[cnt+j].head.next = 
        ((MPID_PKT_T *)MPID_pktshmemPtr[curNode]->pool) + cnt + j + 1;
    MPID_pktshmemPtr[curNode]->pool[cnt+pkts_per_proc-1].head.next = 0;
    cnt += pkts_per_proc;
  
    p2p_lock_init( MPID_pktPtr.availlockPtr[i] );
    p2p_lock_init( MPID_pktPtr.incominglockPtr[i] );
    if (++numCurNode == numCPUs[curNode])
    {
      curNode = (curNode + 1) % numNodes;
      numCurNode = 0;
      cnt = 0;
    }
  }
  
  MPID_shmem->globid = MPID_numids = numprocs;
  p2p_create_procs( numprocs, procNode,  &MPID_myid );
fprintf(stderr, "MPID_myid = %d\n", MPID_myid);
  
  MPID_incoming = MPID_pktPtr.incomingPtr[MPID_myid];
}

void MPID_SHMEM_finalize()
{
  fflush(stdout);
  fflush(stderr);
  /* Wait for everyone to finish */
  p2p_lock( &MPID_shmem->globlock );
  MPID_shmem->globid--;
printf("MPID_myid = %d MPID_shmem->globid = %d\n", MPID_myid, MPID_shmem->globid);
fflush(stdout);
  p2p_unlock( &MPID_shmem->globlock );
  while (MPID_shmem->globid) ;
  
  p2p_cleanup();
}

/* 
  Read an incoming control message.

  NOTE:
  This routine maintains an internal list of elements; this allows it to
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
int MPID_SHMEM_ReadControl( MPID_PKT_T **pkt, int size, int *from)
{
  MPID_PKT_T *inpkt;
  int        backoff, cnt;

  if (MPID_local)
  {
    inpkt = (MPID_PKT_T *)MPID_local;
    MPID_local = MPID_local->head.next;
  } else
  {
    if (!*MPID_pktPtr.incomingPtr[MPID_myid])
    {
    /* This code tries to let other processes get to run.  If there
       are more physical processors than processes, then a simple
       while (!*MPID_pktPtr.incomingPtr[MPID_myid]);
       might be better */
      backoff = 1;
      while (!*MPID_pktPtr.incomingPtr[MPID_myid])
      {
        cnt = backoff;
        while (cnt--) ;
        backoff = 2 * backoff;
        if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
        if (*MPID_pktPtr.incomingPtr[MPID_myid]) break;
        p2p_yield();
      }
    }
      /* This code could drain the ENTIRE list into a local list */
    p2p_lock( MPID_pktPtr.incominglockPtr[MPID_myid] );
    inpkt = (MPID_PKT_T *) *MPID_incoming;
    MPID_local = inpkt->head.next;
    *MPID_incoming = 0;
    *MPID_pktPtr.incomingtailPtr[MPID_myid] = 0;
    p2p_unlock( MPID_pktPtr.incominglockPtr[MPID_myid] );
  }
  
  /* Deliver this packet to the caller */
  *pkt  = inpkt;
  
  *from = (*inpkt).head.src;
  
  MPID_TRACE_CODE_PKT("Readpkt",*from,inpkt);
  
  return MPI_SUCCESS;
}
void MPID_SHMEM_FreeRecvPkt( MPID_PKT_T *pkt )
{
  int        src;
  
  src = (*pkt).head.src;
  
  MPID_TRACE_CODE_PKT("Freepkt",-1,pkt);
  p2p_lock( MPID_pktPtr.availlockPtr[src] );
  pkt->head.next = (MPID_PKT_T *)*MPID_pktPtr.availPtr[src];
  *MPID_pktPtr.availPtr[src] = pkt;
  p2p_unlock( MPID_pktPtr.availlockPtr[src] );
}

#else
int MPID_SHMEM_ReadControl( MPID_PKT_T *pkt, int size, int *from )
{
  MPID_PKT_T *inpkt;
  int        src;
  int        backoff, cnt;
  
  if (MPID_local)
  {
    inpkt = (MPID_PKT_T *)MPID_local;
    MPID_local = MPID_local->head.next;
  } else
  {
    if (!*MPID_pktPtr.incomingPtr[MPID_myid])
    {
    /* This code tries to let other processes get to run.  If there
       are more physical processors than processes, then a simple
       while (!*MPID_pktPtr.incomingPtr[MPID_myid]);
       might be better */
      backoff = 1;
      while (!*MPID_pktPtr.incomingPtr[MPID_myid])
      {
        cnt = backoff;
        while (cnt--) ;
        backoff = 2 * backoff;
        if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;
        if (*MPID_pktPtr.incomingPtr[MPID_myid]) break;
        p2p_yield();
      }
    }
      /* This code could drain the ENTIRE list into a local list */
    p2p_lock( MPID_pktPtr.incominglockPtr[MPID_myid] );
    inpkt = (MPID_PKT_T *) *MPID_incoming;
    MPID_local = inpkt->head.next;
    *MPID_incoming = 0;
    *MPID_pktPtr.incomingtailPtr[MPID_myid] = 0;
    p2p_unlock( MPID_pktPtr.incominglockPtr[MPID_myid] );
  }
  
  /* Eventually, we'll arrange to deliver the address of the packet */
  MEMCPY( pkt, inpkt, (inpkt->head).pkt_len ); 
  
  src = (*inpkt).head.src;
  
  MPID_TRACE_CODE_PKT("Read/freepkt",src,inpkt);
  
  p2p_lock( MPID_pktPtr.availlockPtr[src] );
  inpkt->head.next = (MPID_PKT_T *)*MPID_pktPtr.availPtr[src];
  *MPID_pktPtr.availPtr[src] = inpkt;
  p2p_unlock( MPID_pktPtr.availlockPtr[src] );
  
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
  
  if (localavail)
  {
    inpkt = localavail;
  }
  else
  {
      /* WARNING: THIS CODE DOES A SLEEP 
         IF THERE ARE NO AVAILABLE LOCAL PACKETS */
    do
    {
      p2p_lock( MPID_pktPtr.availlockPtr[MPID_myid] );
      inpkt = 
        (MPID_PKT_T *)*MPID_pktPtr.availPtr[MPID_myid];
      *MPID_pktPtr.availPtr[MPID_myid] = 0;
      p2p_unlock( MPID_pktPtr.availlockPtr[MPID_myid] );
#ifdef MPID_DEBUG_ALL
      if (!inpkt)
        MPID_TRACE_CODE("No freePkt",-1);
#endif
    } while (!inpkt && (sleep(1),1));
  }
  localavail = inpkt->head.next;
  inpkt->head.next = 0;
  
  MPID_TRACE_CODE_PKT("Allocsendpkt",-1,inpkt);
  
  return inpkt;
}

int MPID_SHMEM_SendControl( MPID_PKT_T *pkt, int size, int dest )
{
  MPID_PKT_T *tail;
  
  /* Place the actual length into the packet */
  MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt);
  
  pkt->head.pkt_len = size;
  pkt->head.src = MPID_myid;
  pkt->head.next = 0;           /* Should already be true */
  
  p2p_lock( MPID_pktPtr.incominglockPtr[dest] );
  tail = (MPID_PKT_T *)*MPID_pktPtr.incomingtailPtr[dest];
  if (tail) 
    tail->head.next = pkt;
  else
    *MPID_pktPtr.incomingPtr[dest] = pkt;
  
  *MPID_pktPtr.incomingtailPtr[dest] = pkt;
  p2p_unlock( MPID_pktPtr.incominglockPtr[dest] );
  
  return MPI_SUCCESS;
}
#else
int MPID_SHMEM_SendControl( MPID_PKT_T *pkt, int size, dest )
{
  MPID_PKT_T *inpkt;
  MPID_PKT_T *tail;
  static MPID_PKT_T *localavail = 0;
  
  
  /* Place the actual length into the packet */
  pkt->head.pkt_len = size;
  
  if (localavail)
  {
    inpkt = localavail;
    localavail = inpkt->head.next;
  } else
  {
      /* WARNING: THIS CODE DOES A SLEEP 
         IF THERE ARE NO AVAILABLE LOCAL PACKETS */
    do
    {
      p2p_lock( MPID_pktPtr.availlockPtr[MPID_myid] );
      inpkt = (MPID_PKT_T *)*MPID_pktPtr.availPtr[MPID_myid];
      localavail = inpkt->head.next;
      *MPID_pktPtr.availPtr[MPID_myid] = 0;
      p2p_unlock( MPID_pktPtr.availlockPtr[MPID_myid] );
    } while (!inpkt && (sleep(1),1));
  }
  MPID_TRACE_CODE_PKT("Sendpkt",dest,inpkt);
  MEMCPY( inpkt, pkt, size );
  inpkt->head.next = 0;
  inpkt->head.src = MPID_myid;
  
  p2p_lock( MPID_pktPtr.incominglockPtr[dest] );
  tail = (MPID_PKT_T *)*MPID_pktPtr.incomingtailPtr[dest];
  if (tail) 
    tail->head.next = inpkt;
  else
    *MPID_pktPtr.incomingPtr[dest] = inpkt;
  
  *MPID_pktPtr.incomingtailPtr[dest] = inpkt;
  p2p_unlock( MPID_pktPtr.incominglockPtr[dest] );
  
  return MPI_SUCCESS;
}
#endif

#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif
#ifdef HAVE_SYSINFO
#include <sys/systeminfo.h>
#endif
void SY_GetHostName( int nlen, char *name )
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
  if (!strchr(name,'.'))
  {
    int l, rc;
    l = strlen(name);
    name[l++] = '.';
    name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
    /* Note that IRIX does not support SI_SRPC_DOMAIN */
    rc = -1;
#if defined(solaris) || defined(HAVE_SYSINFO)
    rc = sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#endif
#if defined(HAVE_GETDOMAINNAME)
    if (rc == -1) getdomainname( name+l, nlen - l );
#endif
  }
}

/* 
   Return the address the destination (dest) should use for getting the 
   data at in_addr.  len is INOUT; it starts as the length of the data
   but is returned as the length available, incase all of the data can 
   not be transfered 
 */
void * MPID_SetupGetAddress( void *in_addr, int *len, int dest )
{
  void *new;
  int  tlen = *len;
  
  /* To test, just comment out the first line and set new to null */
  new = p2p_shmalloc( tlen, procNode[dest]);
  /* new = 0; */
  if (!new)
  {
    tlen = tlen / 2; 
    while(tlen > 0 && !(new = p2p_shmalloc(tlen, procNode[dest])))
	   tlen = tlen / 2;
    if (tlen == 0)
    {
      fprintf( stderr, "Could not get any shared memory for long message!" );
      exit(1);
    }
      /* fprintf( stderr, "Message too long; sending partial data\n" ); */
    *len = tlen;
  }
  /*
  if (MPID_DEBUG_FILE)
  {
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

void MPID_FreeGetAddress( void *addr )
{
  p2p_shfree( addr );
}
