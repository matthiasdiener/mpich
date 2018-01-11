/* This is the code for NON dynamic receive */
/* 
   If this is set, then packets are not allocated and are just passed back 
   from the readcontrol routine.  If not, packets are created on the call 
   stack and copied from a shared-memory packet.

   NOTE THE DIFFERENCES IN BINDINGS
 */
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
    /* This code drains the ENTIRE list into a local list */
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

/* This is the code for NON dynamic sends */
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
