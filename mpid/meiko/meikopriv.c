/*
 * This implements the Meiko device for MPICH.
 * It sits on top of a single TPORT, and packs the context
 * and sender into the tag field.
 *
 * Updated to have ANSI style code, since I know that it is only compiled
 * for the CS2, and there is definitely and ANSI compiler available !
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/systeminfo.h>
#include <ew/ew.h>
#include "mpid.h"

#define MEIKO_VERSION (MPIDTRANSPORT MPIDPATCHLEVEL)
#define TXFLAGS(len)  (((len) < 64 * 1024) ? 0 : EW_TPORT_TXSYNC)
#define MEIKO2VP(x)   (ew_groupMember(meiko_state.group,(x)))

typedef struct
{
   int             proc;           /* Who am I ?                      */
   int             nproc;          /* How many TOTAL processes ?      */
   EW_TPORT       *tport;          /* What we actually use for moving messages */
   double          clockDelta;     /* add to local clock for the epoch */
   EW_GROUP       *group;          /* The group with everyone in      */
} MEIKO_STATE;

static MEIKO_STATE meiko_state;

void *MPID_MEIKO_Init ( int * Argc, char *** Argv )
{
    ELAN_TIMEVAL tv;
    EW_GROUP    *meikoGroup;
   
    if (meiko_state.tport) {	/* already fully initialised */
	fprintf( stderr, "Already initialized!\n" );
	return 0;
    }

    /* Make 100% sure the state is clean before we start */
    memset(&meiko_state, 0, sizeof(meiko_state));

    /* Connect with the rest of the parallel program */
    ew_baseInit ();

    /* Ensure we're linked with compatible software */
    if (!elan_checkVersion (ELAN_VERSION))
	ew_exception (EW_EINIT, "'%s' incompatible with '%s' ('%s' expected)",
		      MEIKO_VERSION, elan_version (), ELAN_VERSION);

    if (!ew_checkVersion (EW_VERSION))
	ew_exception (EW_EINIT, "'%s' incompatible with '%s' ('%s' expected)",
		      MEIKO_VERSION, ew_version (), EW_VERSION);

    if (!meiko_checkVersion (MEIKO_VERSION))
	ew_exception (EW_EINIT, "meiko_checkVersion(self)");

#ifdef DEBUG
    atexit((void (*)(void))meiko_fini);
#endif

    /* Hostless style is all we support for now, don't include prun.
     */
    meikoGroup         = ew_base.myGroup;
    meiko_state.proc   = meikoGroup->g_self;
    meiko_state.nproc  = meikoGroup->g_size;
    meiko_state.group  = meikoGroup;

    if (!(meiko_state.tport = (EW_TPORT *) ew_allocate (ew_base.alloc, EW_ALIGN,
							ew_tportSize (ew_base.tport_nattn))))
	ew_exception (EW_EINIT, "MPI Can't allocate tport");

    ew_tportInit (meiko_state.tport, ew_base.tport_nattn, meiko_state.proc,
		  ew_base.tport_smallmsg,
		  ew_base.waitType,
		  ew_base.dmaType); 

    /* NB the tport's tag == the index in comm_world
     * => source envelope info needs no translation, provided that 
     * we always work in the all group.
     */

    /* Remember the tport for the debugger, and associate the group  */
    ew_tagObject("TPORT","MPI",  meiko_state.tport);
    ew_tagObject("GROUP","MPI_g",meiko_state.group);
    /* Describe to the debugger how we slice the tag 
     * We don't display the source field at all.
     */
    ew_tagObject("TAG",  "0xff000000:Context", (void *)(((int)meiko_state.tport)+1));
    ew_tagObject("TAG",  "0x0000ffff:Tag",     (void *)(((int)meiko_state.tport)+2));

    ew_tportGrowDescs (meiko_state.tport, 0, 0); /* allocate min # tport rx/tx descs */

    ew_gsync (meikoGroup);      /* sync the nodes */

    /* get local-to-global clock delta, referenced to NOW on meikoGroup member 0 */

    elan_t_clock (ew_ctx, &tv);
    meiko_state.clockDelta = ew_getDeltaT (meikoGroup, -(tv.tv_sec + tv.tv_nsec * 1.0e-9));

    return (void *)&meiko_state;
}

char *meiko_version (void)
{
    return (MEIKO_VERSION);
}

int meiko_checkVersion (char *version)
{
    return (!strcmp (version, MEIKO_VERSION));
}

void MPID_MEIKO_End (void)
{
#ifdef DEBUG
    /* Only run once even if called by the user, and then by atexit */
    static int exited = FALSE;

    if (!exited)
    {
 	MEIKO_DESC * d;
 
	exited = TRUE;

	/* Check the tport for quiescence */

	/* First, pending non blocking receive operations */
	ew_tportRxIterator(meiko_state.tport, dumpRx, NULL);

 	/* Check that there are no pending Tx operations left */
	ew_tportTxIterator(meiko_state.tport, dumpTx, NULL);

	/* And finally any messages that were sent to us, but we never collected */
	ew_tportBufIterator(meiko_state.tport, dumpBuf, NULL);

#if 0	
	gsync();		/* Don't leave until we all get here */
#endif
    } 
#endif

    return;
}

void MPID_MEIKO_Mysize( int *size )
{
    *size = meiko_state.nproc;
}

void MPID_MEIKO_Myrank( int *rank )
{
    *rank = meiko_state.proc;
}

double MPID_MEIKO_Wtime()
{
    ELAN_TIMEVAL	   tv;

    elan_t_clock (ew_ctx, &tv);
    
    return (tv.tv_sec + tv.tv_nsec * 1.0e-9 + meiko_state.clockDelta);
}

void MPID_MEIKO_Abort( int errno  )
{
    abort();
}


/****************************************************************/
/*                                                              */
/*                 Message Passing                              */
/*                                                              */
/****************************************************************/

int MPID_MEIKO_Probe(int tag, int source, int context_id, MPI_Status *status )
{
    ew_tportRxWait (ew_tportRxStart (meiko_state.tport, EW_TPORT_RXPROBE,
				     (source < 0) ? 0 : -1, source, 
				     (tag    < 0) ? MPID_CTX_MASK : MPID_CTX_MASK | MPID_TAG_MASK,
				     (context_id << MPID_CTX_SHFT) | (tag & MPID_TAG_MASK),
				     NULL, 0),
		    &status->MPI_SOURCE, &status->MPI_TAG, &status->count );

    status->MPI_SOURCE  = (status->MPI_TAG >> MPID_SRC_SHFT) &(MPID_SRC_MASK >> MPID_SRC_SHFT);
    status->MPI_TAG    &= MPID_TAG_MASK;

    return 0;
}


int MPID_MEIKO_Iprobe(int tag, int source, int context_id, 
		      int *flag, MPI_Status *status )
{
    *flag = ew_tportRxPoll (meiko_state.tport, 
			    (source < 0) ? 0 : -1, source, 
			    (tag    < 0) ? MPID_CTX_MASK : MPID_CTX_MASK | MPID_TAG_MASK,
			    (context_id << MPID_CTX_SHFT) | (tag & MPID_TAG_MASK),
			    &status->MPI_SOURCE, &status->MPI_TAG, &status->count );
   
    status->MPI_SOURCE  = (status->MPI_TAG >> MPID_SRC_SHFT) &(MPID_SRC_MASK >> MPID_SRC_SHFT);
    status->MPI_TAG &= MPID_TAG_MASK;

    return *flag;
}

int MPID_MEIKO_blocking_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
    int tag	   = dmpi_recv_handle->tag;
    int context_id = dmpi_recv_handle->contextid;
    int source     = dmpi_recv_handle->source;   
    
    ew_tportRxWait (ew_tportRxStart (meiko_state.tport, 0,
				     (source < 0) ? 0 : -1, source, 
				     (tag    < 0) ? MPID_CTX_MASK : MPID_CTX_MASK | MPID_TAG_MASK,
				     (context_id << MPID_CTX_SHFT) | (tag & MPID_TAG_MASK),
				     dmpi_recv_handle->dev_rhandle.start, 
				     dmpi_recv_handle->dev_rhandle.bytes_as_contig ),
		    &dmpi_recv_handle->source, 
		    &dmpi_recv_handle->tag,
		    &dmpi_recv_handle->totallen );

    dmpi_recv_handle->dev_rhandle.bytes_as_contig = dmpi_recv_handle->totallen;
    dmpi_recv_handle->source = (dmpi_recv_handle->tag >> MPID_SRC_SHFT) &
	(MPID_SRC_MASK >> MPID_SRC_SHFT);
    dmpi_recv_handle->tag &= MPID_TAG_MASK;
    return (0);
}

int MPID_MEIKO_post_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
    int tag        = dmpi_recv_handle->tag;
    int context_id = dmpi_recv_handle->contextid;
    int source     = dmpi_recv_handle->source;   
    
    dmpi_recv_handle->dev_rhandle.id = 
	ew_tportRxStart (meiko_state.tport, 0,
			 (source < 0) ? 0 : -1, source, 
			 (tag    < 0) ? MPID_CTX_MASK : MPID_CTX_MASK | MPID_TAG_MASK,
			 (context_id << MPID_CTX_SHFT) | (tag & MPID_TAG_MASK),
			 dmpi_recv_handle->dev_rhandle.start, 
			 dmpi_recv_handle->dev_rhandle.bytes_as_contig );

    return 0;
}


int MPID_MEIKO_Blocking_send(MPIR_SHANDLE *dmpi_send_handle )
{
    int tag	  = dmpi_send_handle->tag;
    int context_id = dmpi_send_handle->contextid;
    int dest       = dmpi_send_handle->dest;
    int len        = dmpi_send_handle->dev_shandle.bytes_as_contig;
    int source     = dmpi_send_handle->lrank;

    ew_tportTxWait (ew_tportTxStart (meiko_state.tport, TXFLAGS(len),
				     MEIKO2VP (dest), meiko_state.tport,
				     (context_id << MPID_CTX_SHFT) | 
				     (source     << MPID_SRC_SHFT) |
				     (tag & MPID_TAG_MASK),
				     dmpi_send_handle->dev_shandle.start, len ) );
   
    return (0);
}

int MPID_MEIKO_post_send(MPIR_SHANDLE *dmpi_send_handle, int flags )
{
    int tag        = dmpi_send_handle->tag;
    int context_id = dmpi_send_handle->contextid;
    int dest       = dmpi_send_handle->dest;
    int source     = dmpi_send_handle->lrank;
   
    dmpi_send_handle->dev_shandle.id = 
	ew_tportTxStart (
			 meiko_state.tport, 
			 flags | TXFLAGS(dmpi_send_handle->dev_shandle.bytes_as_contig),
			 MEIKO2VP (dest), meiko_state.tport,
			 (context_id << MPID_CTX_SHFT) |
			 (source     << MPID_SRC_SHFT) |
			 (tag & MPID_TAG_MASK),
			 dmpi_send_handle->dev_shandle.start, 
			 dmpi_send_handle->dev_shandle.bytes_as_contig );
    return 0;
}

int MPID_MEIKO_Test_send(MPIR_SHANDLE *dmpi_send_handle )
{
    ELAN_EVENT *d = dmpi_send_handle->dev_shandle.id;

    if (d == NULL)
	return 1;
    if (EVENT_READY (d))
    {
	MPID_MEIKO_complete_send(dmpi_send_handle);
	return 1;
    } else
	return 0;
}


int MPID_MEIKO_Test_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
    ELAN_EVENT *d = dmpi_recv_handle->dev_rhandle.id;

    if (d == NULL)
	return 1;
    if (EVENT_READY (d))
    {
	MPID_MEIKO_complete_recv(dmpi_recv_handle);
	return 1;
    } else
	return 0;
}

int MPID_MEIKO_complete_recv(MPIR_RHANDLE *dmpi_recv_handle )
{
    ELAN_EVENT *d = dmpi_recv_handle->dev_rhandle.id;

    if (d) 
    {
	ew_tportRxWait (d, &dmpi_recv_handle->source, &dmpi_recv_handle->tag,
			&dmpi_recv_handle->dev_rhandle.bytes_as_contig );
	/* Clean the tag, it's not the whole field... */
	dmpi_recv_handle->source = (dmpi_recv_handle->tag >> MPID_SRC_SHFT) &
	    (MPID_SRC_MASK >> MPID_SRC_SHFT);
	dmpi_recv_handle->tag           &= MPID_TAG_MASK;
	dmpi_recv_handle->dev_rhandle.id = NULL;
	dmpi_recv_handle->totallen = 
	    dmpi_recv_handle->dev_rhandle.bytes_as_contig; 
	MPID_Set_completed( ctx, dmpi_recv_handle );
    }
   
    return (0);
}

int MPID_MEIKO_complete_send(MPIR_SHANDLE *dmpi_send_handle )
{
    ELAN_EVENT *d = dmpi_send_handle->dev_shandle.id;

    if (d) 
    {
	ew_tportTxWait (d);
	dmpi_send_handle->dev_shandle.id = NULL;
	MPID_Set_completed( ctx, dmpi_send_handle);
    }
   
    return (0);
}

int MPID_MEIKO_Cancel(MPIR_COMMON *dmpi_handle )
{
    fprintf( stderr, "CANCEL Not implemented (yet)!\n" );

    abort();
}

/*
 *  Miscellaneous stuff
 */
void MPID_MEIKO_Version_name( name )
char *name;
{
    sprintf( name, "ADI version %s - transport %s", MPIDPATCHLEVEL, 
	    MPIDTRANSPORT );
}

double MPID_MEIKO_Wtick()
{
    return 1.0 / 40.0e6;
}

void MPID_MEIKO_Node_name( name, len )
char *name;
int  len;
{
    sysinfo(SI_HOSTNAME, name, len);
}
 
#ifdef DEBUG
static char * selectString(char * res, const int mask, const int match)
{
    if (mask == 0)
	strcpy(res, "ANY");
    else
	sprintf(res,"%d",match);

    return res;
}

static void dumpRx(void * usrArgs, void * desc, int sendmask, int sender, 
		   int tagmask,  int tag, 
		   int size,     int bufSize, void * buffer, int done)
{
    char sendString[8];
    char tagString[8];

    fprintf(stderr,"MEIKO warning @ %d: %d (%s: receive)\n"
	    "             @ %d: %s sender %s tag %s buffer 0x%x size %d\n",
	    meiko_state.proc, MEIKO_WLEFTOVEROP, meiko_errlist[MEIKO_WLEFTOVEROP-MEIKO_EBASE],
	    meiko_state.proc, 
	    done ? "completed":"active",
	    selectString(sendString, sendmask, sender),
	    selectString(tagString, tagmask, tag),
	    buffer, bufSize);
    fflush(stderr);
}

static void dumpTx(void * usrArgs, void * desc, int targetVp, int tag, int size, void * buf, int done)
{
    int i;

    /* Translate the target VP back into MEIKO space.
     * This is unpleasant, and slow, but doesn't matter here.
     * By definition these groups are one to one mappings.
     */
    for (i=0; i<ew_base.allGroup->g_size; i++)
	if (ew_groupMember(ew_base.allGroup,i) == targetVp)
	    break;
    
    fprintf(stderr,"MEIKO warning @ %d: %d (%s: send)\n" 
	    "             @ %d: %s target %d tag %d buffer 0x%x size %d\n",
	    meiko_state.proc, MEIKO_WLEFTOVEROP, meiko_errlist[MEIKO_WLEFTOVEROP-MEIKO_EBASE],
	    meiko_state.proc, done ? "completed":"active",i, tag, buf, size); 
    fflush(stderr);
}

static void dumpBuf(void * usrArgs, void * desc, int source, int tag, int size, int done)
{
    fprintf(stderr,"MEIKO warning @ %d: %d (%s)\n"
	    "             @ %d: %s sender %d tag %d size %d\n",
	    meiko_state.proc, MEIKO_WLEFTOVERMSG,  meiko_errlist[MEIKO_WLEFTOVERMSG-MEIKO_EBASE],
	    meiko_state.proc, done ? "completed":"active",source, tag, size);
    fflush(stderr);
}
#endif
