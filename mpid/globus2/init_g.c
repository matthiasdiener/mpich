#include <strings.h>   /* for index */
#include <sys/time.h> /* for gettimeofday() */

#include <globus_duroc_runtime.h>
#include <globus_duroc_bootstrap.h>
#include <globus_gram_client.h> /* for MPID_Abort */
#include "globdev.h"
#include "reqalloc.h"
#include "sendq.h"

#define MPIDPATCHLEVEL 2.0
#define MPIDTRANSPORT "globus"

/********************/
/* Global Variables */
/********************/

globus_mutex_t      MessageQueuesLock;
struct channel_t *  CommworldChannels;
extern volatile int TcpOutstandingRecvReqs;
extern volatile int TcpOutstandingSendReqs;
extern int          MpichGlobus2TcpBufsz;
static globus_byte_t *MyGlobusGramJobContact;
static globus_byte_t **GramJobcontactsVector;
/* 
 * for unique msg id's (used in conjunction with MPID_MyWorldRank) 
 * the NextMsgIdCtr must have at least enough bits so as not to rollover     
 * within the resolution of our clock (in our case usecs).  also, every time 
 * NextMsgIdCtr rolls over we have to call gettimeofday, so the more bits it
 * has, the fewer times we have to make that expensive system call.          
 */
struct timeval LastTimeILookedAtMyWatch;
unsigned long NextMsgIdCtr;

#if defined(VMPI)
extern struct mpi_posted_queue MpiPostedQueue;

int   				VMPI_MyWorldSize = 0;
int   				VMPI_MyWorldRank = -1;
int * 				VMPI_VGRank_to_GRank = NULL;
int * 				VMPI_GRank_to_VGRank = NULL;
void *				VMPI_Internal_Comm = NULL;
#endif

/* TCP proto Global Variables */
globus_size_t      Headerlen;
globus_io_handle_t Handle; 

/* Home for these globals ... required of all mpich devices */
int MPID_MyWorldSize, MPID_MyWorldRank;
int MPID_Print_queues = 0;
MPID_SBHeader MPIR_rhandles;
MPID_SBHeader MPIR_shandles;
int MPID_IS_HETERO = GLOBUS_FALSE;

/***************************************/
/* local utility function declarations */
/***************************************/

static int globus_init(int *argc, char ***argv);
static void create_my_miproto(globus_byte_t **my_miproto, int *nbytes);
static void build_channels(int nprocs,
			    globus_byte_t **mi_protos_vector,
			    int *mi_protos_vector_lengths,
			    struct channel_t **channels);
static void select_protocols(int myrank, 
                            int nprocs, 
                            struct channel_t *channels);
static void build_vmpi_maps();
static void free_vmpi_maps();

void MPID_Init(int *argc, char ***argv, void *config, int *error_code)
{
    /* required of all mpich devices */
    MPIR_shandles = MPID_SBinit(sizeof(MPIR_PSHANDLE), 100, 100);
    MPIR_rhandles = MPID_SBinit(sizeof(MPIR_PRHANDLE), 100, 100);
    MPID_InitQueue();

    *error_code = 0;
    if (globus_init(argc, argv))
    {
	*error_code = MPI_ERR_INTERN;
	globus_libc_fprintf(stderr, "ERROR: MPID_Init: failed globus_init()\n");
	goto fn_exit;
    } /* endif */

    /* Initialization for generating unique message id's */
    if (gettimeofday(&LastTimeILookedAtMyWatch, (void *) NULL))
    {
	*error_code = MPI_ERR_INTERN;
	globus_libc_fprintf(stderr, 
	    "ERROR: MPID_Init: failed gettimeofday()\n");
    } /* endif */
    NextMsgIdCtr = 0;

    /*
     * Call the vendor implementation of MPI_Init().  See pr_mp_g.c for a
     * discussion on startup and shutdown constraints.
     */
#   ifdef VMPI
    {
	if (mp_init(argc, argv))
	{
	    *error_code = MPI_ERR_INTERN;
	    globus_libc_fprintf(stderr, 
		"ERROR: MPID_Init: failed mp_init()\n");
	    goto fn_exit;
	} /* endif */

	VMPI_Internal_Comm = (void *) globus_libc_malloc(mp_comm_get_size());
	if (VMPI_Internal_Comm == NULL)
	{
	    *error_code = MPI_ERR_INTERN;
	    globus_libc_fprintf(stderr, "MPID_Init(): failed malloc\n");
	    goto fn_exit;
	} /* endif */

	if (mp_comm_dup(NULL, VMPI_Internal_Comm) != VMPI_SUCCESS)
	{
	    *error_code = MPI_ERR_INTERN;
	    globus_libc_fprintf(stderr, "MPID_Init(): failed mp_comm_dup()\n");
	    goto fn_exit;
	} /* endif */
    } /* endifdef */
#   endif

  fn_exit: 
    /* to supress compile warnings */ ;
} /* end MPID_Init() */

/* 
 * this about MPI_Abort from the MPI 1.1 standard:
 *   "This routine makes a ``best attempt'' to abort all tasks in the group 
 *    of comm. This function does not require that the invoking environment 
 *    take any action with the error code. However, a Unix or POSIX environment 
 *    should handle this as a return errorcode from the main program or an 
 *    abort(errorcode). 
 *    MPI implementations are required to define the behavior of MPI_ABORT at 
 *    least for a comm of MPI_COMM_WORLD. MPI implementations may ignore the 
 *    comm argument and act as if the comm was MPI_COMM_WORLD."
 *
 * we have chosen to ignore the comm arg and kill everything in MPI_COMM_WORLD.
 * also, we do NOT propogate the 
 */
void MPID_Abort(struct MPIR_COMMUNICATOR *comm,
		int error_code,
		char *facility, /* optional, used to indicate who called the
				   routine; for example, the user (MPI_Abort)
				   or the MPI implementation. */
		char *string) /* optional, use if provided else print default */
{
    int i;
    globus_byte_t *last_contact = (globus_byte_t *) NULL;

    if (facility && *facility)
	globus_libc_fprintf(stderr, "%s: ", facility); 

    if (string && *string)
	globus_libc_fprintf(stderr, "%s\n", string); 
    else
	/* default message */
	globus_libc_fprintf(stderr, "Aborting with code %d\n", error_code); 

    i = globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE);
    if (i != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "MPID_Abort: failed "
	    "globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE)");
	abort();
    } /* endif */

    /* loop to send ONE kill message to all gatekeepers OTHER than mine */
    for (i = 0; i < MPID_MyWorldSize; i ++)
    {
	if ((!last_contact || strcmp((const char *) last_contact, 
				    (const char *) GramJobcontactsVector[i]))
	    && strcmp((const char *) MyGlobusGramJobContact, 
			(const char *) GramJobcontactsVector[i]))
	{
	    last_contact = GramJobcontactsVector[i];
	    if (globus_gram_client_job_cancel((char *) GramJobcontactsVector[i])
		!= GLOBUS_SUCCESS)
	    {
		globus_libc_fprintf(stderr, 
		    "ERROR: MPID_Abort: failed remote "
		    "globus_gram_client_job_cancel to job contact >%s<\n", 
		    GramJobcontactsVector[i]);
	    } /* endif */
	} /* endif */
    } /* endfor */

    /* now killing MY subjob */
    if (globus_gram_client_job_cancel((char *) MyGlobusGramJobContact)
	    !=GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "ERROR: MPID_Abort: failed local globus_gram_client_job_cancel "
	    "to job contact >%s<\n", 
	    MyGlobusGramJobContact);
    } /* endif */

} /* end MPID_Abort() */

#undef DEBUG_FN_NAME
#define DEBUG_FN_NAME MPID_End
void MPID_End(void)
{
    struct miproto_t *mp;
    int i;

    DEBUG_FN_ENTRY(DEBUG_MODULE_INIT);

#   if defined(VMPI)
    {
	struct mpircvreq *mp;

	if (MpiPostedQueue.head)
	{
	    DEBUG_PRINTF(DEBUG_MODULE_INIT | DEBUG_MODULE_RECV,
			 DEBUG_INFO_WARNING,
			 ("WARNING: MPI_COMM_WORLD_RANK %d found residual "
			  "nodes in MpiPostedQueue\n",
			  MPID_MyWorldRank));
	} /* endif */

	while (mp = MpiPostedQueue.head)
	{
	    MpiPostedQueue.head = mp->next;
	    g_free(mp);
	} /* endwhile */
    }
#   endif
    globus_mutex_destroy(&MessageQueuesLock);

    /* freeing CommworldChannels */
    for (i = 0; i < MPID_MyWorldSize; i ++)
    {
        while (mp = CommworldChannels[i].proto_list)
        {
            CommworldChannels[i].proto_list = mp->next;
	    if (mp->type == tcp)
	    {
		struct tcp_miproto_t *tp = (struct tcp_miproto_t *) mp->info;
		struct tcpsendreq *tmp;

		if (tp->handlep)
		{
		    struct tcp_rw_handle_t *rwp;

		    rwp = (struct tcp_rw_handle_t *) tp->handlep;
		    globus_io_close(&(rwp->handle));
		    g_free(tp->handlep);
		}
		
		for (tmp = tp->cancel_head; tmp; tmp = tp->cancel_head)
		{
		    tp->cancel_head = tmp->next;
		    g_free((void *) tmp);
		} /* endfor */
		for (tmp = tp->send_head; tmp; tmp = tp->send_head)
		{
		    tp->send_head = tmp->next;
		    g_free((void *) tmp);
		} /* endfor */

		g_free((void *) tp->header);
	    } /* endif */
            g_free((void *) mp->info);
            g_free((void *) mp);
        } /* endwhile */
    } /* endfor */
    g_free((void *) CommworldChannels);

    /* freeing GramJobcontactsVector */
    for (i = 0; i < MPID_MyWorldSize; i ++)
	g_free((void *) GramJobcontactsVector[i]);
    g_free((void *) GramJobcontactsVector);

    free_vmpi_maps();
    
    globus_module_deactivate(GLOBUS_NEXUS_MODULE);
    globus_module_deactivate(GLOBUS_IO_MODULE);
    globus_module_deactivate(GLOBUS_COMMON_MODULE);

    /*
     * Call the vendor version of MPI_Finalize()
     */
#   if defined(VMPI)
    {
	mp_finalize();
    }
#   endif

    DEBUG_FN_EXIT(DEBUG_MODULE_INIT);

} /* end MPID_End() */

/*
 * MPID_DeviceCheck
 *
 * NICK: for now just call globus_poll and return 1.
 *       need to understand if it's OK to call globus_poll_blocking
 *       when 'is_blocking != 0'
 */
int MPID_DeviceCheck(MPID_BLOCKING_TYPE is_blocking)
{

#   if defined(VMPI)
    {
	/* nudge MPI */
	struct mpircvreq *curr, *next;

	/* 
	 * take one pass through the MpiPostedQueue trying to 
	 * satisfy each req.
	 */
	globus_mutex_lock(&MessageQueuesLock);
	for (curr = MpiPostedQueue.head; curr; curr = next)
	{
	    /* 
	     *if curr->req was satisfied, then mpi_recv_or_post
	     * removes it from MpiPostedQueue and sets curr->next=NULL
	     */
	    next = curr->next;
	    mpi_recv_or_post(curr->req, (int *) NULL);
	} /* endfor */
	globus_mutex_unlock(&MessageQueuesLock);
    }
#   endif

    /* nudge TCP */
    {
	globus_bool_t outstanding_tcp_reqs;
	RC_mutex_lock(&MessageQueuesLock);
	outstanding_tcp_reqs = ((TcpOutstandingRecvReqs > 0 
					|| TcpOutstandingSendReqs > 0)
					    ? GLOBUS_TRUE : GLOBUS_FALSE);
	RC_mutex_unlock(&MessageQueuesLock);
	if (outstanding_tcp_reqs)
	    globus_poll();
    }

    return 1;

} /* end MPID_DeviceCheck() */

int MPID_Complete_pending(void)
{
    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		"MPID_Complete_pending not implemented yet" );
    return 1;
} /* end MPID_Complete_pending() */

int MPID_WaitForCompleteSend(MPIR_SHANDLE *request)
{
    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		"MPID_WaitForCompleteSend not implemented yet" );
    return 1;
#if 0
    while (!request->is_complete)
    {
	MPID_DeviceCheck(MPID_BLOCKING);
    }
    return MPI_SUCCESS;
#endif
} /* end MPID_WaitForCompleteSend() */

int MPID_WaitForCompleteRecv(MPIR_RHANDLE *request)
{
    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
		"MPID_WaitForCompleteRecv not implemented yet" );
    return 1;
#if 0
    while (!request->is_complete)
    {
	MPID_DeviceCheck(MPID_BLOCKING);
    }
    return MPI_SUCCESS;
#endif
} /* end MPID_WaitForCompleteRecv() */

void MPID_SetPktSize(int len)
{
    /* do nothing */
} /* end MPID_SetPktSize() */

void MPID_Version_name(char *name)
{
    sprintf(name, "ADI version %4.2f - transport %s",
    		MPIDPATCHLEVEL, MPIDTRANSPORT);
} /* end MPID_Version_name() */

/*
 * this function gets called by MPI_Reqeust_free in 
 * mpich/src/pt2pt/commreq_free.c ONLY when the is_complete 
 * of request is FALSE.
 */
void MPID_Request_free(MPI_Request request)
{

    MPI_Request rq = request;
    int error_code;

    switch (rq->handle_type) 
    {
	case MPIR_SEND:
	    {
		MPIR_SHANDLE *sreq = (MPIR_SHANDLE *) rq;
#               if defined(VMPI)
		{
		    if (sreq->req_src_proto == mpi)
		    {
			/* MPI */
			error_code = vmpi_error_to_mpich_error(
				    mp_request_free(sreq->vmpi_req));
			MPIR_FORGET_SEND(&rq->shandle);
			MPID_SendFree(rq);
			return;
		    } /* endif */
		}
#               endif

		/* TCP */
		if (MPID_SendIcomplete(rq, &error_code)) 
		{
		    MPIR_FORGET_SEND(&rq->shandle);
		    MPID_SendFree(rq);
		    rq = 0;
		} /* endif */
	    }
	    break;
	case MPIR_RECV:
	    if (MPID_RecvIcomplete(rq, (MPI_Status *)0, &error_code)) 
	    {
		MPID_RecvFree(rq);
		rq = 0;
	    }
	    break;
	case MPIR_PERSISTENT_SEND:
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
			"Unimplemented operation - persistent send free" );
	    break;
	case MPIR_PERSISTENT_RECV:
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPI internal", 
			"Unimplemented operation - persistent recv free" );
	    break;
	default:
	    break;
    } /* end switch() */

    MPID_DeviceCheck( MPID_NOTBLOCKING );
    /* 
     * If we couldn't complete it, decrement it's reference count
     * and forget about it.  This requires that the device detect
     * orphaned requests when they do complete, and process them
     * independent of any wait/test.
     */
    if (rq)
    {
        rq->chandle.ref_count--;
    }
} /* end MPID_Request_free() */


void MPID_ZeroStatusCount(
    MPI_Status *			status)
{
    (status)->count = 0;
    STATUS_INFO_SET_COUNT_NONE(*status);
}

/****************************/
/* public utility functions */
/****************************/

void print_channels(int nprocs, struct channel_t *channels)
{

    int i;
    struct miproto_t *mp;

    globus_libc_fprintf(stderr, "*** START %d procs\n", nprocs);
    for (i = 0; i < nprocs; i ++)
    {
        globus_libc_fprintf(stderr, "proc %d", i); 
        for (mp = channels[i].proto_list; mp; mp = mp->next)
        {
            switch (mp->type)
            {
                case tcp:
                    globus_libc_fprintf(stderr, "\tTCP: host >%s< port %d", 
                        ((struct tcp_miproto_t *) (mp->info))->hostname, 
                        (int) ((struct tcp_miproto_t *) (mp->info))->port);
                    break;
                case mpi:
                    globus_libc_fprintf(stderr,
			"\tMPI: unique_string >%s< rank %d", 
                ((struct mpi_miproto_t *) (mp->info))->unique_session_string, 
                        ((struct mpi_miproto_t *) (mp->info))->rank);
                    break;
                default:
                    globus_libc_fprintf(stderr,
                        "\tERROR: encountered unrecognized proto type %d", 
                        mp->type);
                    break;
            } /* end switch() */
            if (mp == channels[i].selected_proto)
                globus_libc_fprintf(stderr, " (selected)");
            globus_libc_fprintf(stderr, "\n");
        } /* endfor */
    } /* endfor */
    globus_libc_fprintf(stderr, "*** END %d procs\n", nprocs); 

} /* end print_channels() */

/***************************/
/* Local Utility Functions */
/***************************/

static int globus_init(int *argc, char ***argv)
{
    int i;
    globus_byte_t *my_miproto;
    globus_byte_t **mi_protos_vector;
    int *mi_protos_vector_lengths;
    int nbytes;
    char * tmpstr;
    int rc;

    rc = globus_module_activate(GLOBUS_DUROC_RUNTIME_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "globus_init: failed "
	    "globus_module_activate(GLOBUS_DUROC_RUNTIME_MODULE)\n");
	abort();
    } /* endif */
	
    globus_duroc_runtime_barrier();
    
    rc = globus_module_deactivate(GLOBUS_DUROC_RUNTIME_MODULE);    
    if (rc != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "globus_init: failed "
	    "globus_module_deactivate(GLOBUS_DUROC_RUNTIME_MODULE)\n");
	abort();
    } /* endif */

    rc = globus_module_activate(GLOBUS_COMMON_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "globus_init: failed "
	    "globus_module_activate(GLOBUS_COMMON_MODULE)\n");
	abort();
    } /* endif */

    rc = globus_module_activate(GLOBUS_IO_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "globus_init: failed "
	    "globus_module_activate(GLOBUS_IO_MODULE)\n");
	abort();
    } /* endif */

    /* 
     * we have to activate the nexus and disable fault tolerance
     * because duroc bootstrap uses nexus AND insists on keeping
     * nexus activated during the entire computation (even though
     * duroc only uses nexus when we use duroc, that is, during
     * initialization).  the problem here is that when a remote
     * proc dies a bunch of nexus error messages gets generated
     * (because fd's in nexus ep's are being closed) AND our proc
     * is forced to abort because nexus_fatal gets called too.  
     * by registering NULL nexus error handlers we not only prevent
     * the annoying nexus error messages, but we also prevent our
     * proc terminating just becase a remote proc terminated.
     */
    rc = globus_module_activate(GLOBUS_NEXUS_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
	globus_libc_fprintf(stderr, 
	    "globus_init: failed "
	    "globus_module_activate(GLOBUS_NEXUS_MODULE)\n");
	abort();
    } /* endif */
    nexus_enable_fault_tolerance (NULL, NULL);

    /*
     * Find out if the user wants to increase the socket buffer size
     */
    tmpstr = globus_module_getenv("MPICH_GLOBUS2_TCP_BUFFER_SIZE");
    if (tmpstr != GLOBUS_NULL)
    {
	MpichGlobus2TcpBufsz = atoi(tmpstr);
	if (MpichGlobus2TcpBufsz < 0)
	{
	    MpichGlobus2TcpBufsz = 0;
	}
    }
    /****************************************************************/
    /* making sure there's enough room in a ulong to hold a pointer */
    /* ... this is REQUIRED for liba of our TCP headers.            */
    /****************************************************************/

    if (sizeof(MPIR_SHANDLE *) > globus_dc_sizeof_u_long(1))
    {
	globus_libc_fprintf(stderr, 
	    "ERROR: globus_init: detected that sizeof pointer %ld is greater "
	    "than sizeof(ulong) %ld ... cannot run\n", 
	    (long) sizeof(MPIR_SHANDLE *), 
	    (long) globus_dc_sizeof_u_long(1));
	return 1;
    } /* endif */

    /*********************************/
    /* initializing global variables */
    /*********************************/

    /* lock that must be acquired before accessing any of the MPID_ */
    /* queueing functions found in mpich/util/queue.c               */
    globus_mutex_init(&MessageQueuesLock, (globus_mutexattr_t *) GLOBUS_NULL);

#   if defined(VMPI)
    {
        MpiPostedQueue.head = MpiPostedQueue.tail = (struct mpircvreq *) NULL;
    }
#   endif

    /* initializing TCP proto global variables */
    /* tcphdr = src,tag,context,dataoriginbuffsize,ssend_flag,liba (ulong) */
    Headerlen = (globus_size_t) LOCAL_HEADER_LEN;

    /**************************************************/
    /* creating and all-to-all distributing mi_protos */
    /**************************************************/

    create_my_miproto(&my_miproto, &nbytes);
    globus_duroc_bootstrap_all_to_all_distribute_bytearray(
	my_miproto, 
	nbytes,
	&MPID_MyWorldSize, 
	&MPID_MyWorldRank,
	&mi_protos_vector,
	&mi_protos_vector_lengths);
    g_free((void *) my_miproto);

    mpich_globus2_debug_init();

    build_channels(MPID_MyWorldSize,
                    mi_protos_vector,
                    mi_protos_vector_lengths,
                    &CommworldChannels);

    g_free((void *) mi_protos_vector_lengths);
    for (i = 0; i < MPID_MyWorldSize; i ++)
    {
        g_free((void *) mi_protos_vector[i]);
    } /* endfor */
    g_free((void *) mi_protos_vector);

    select_protocols(MPID_MyWorldRank, MPID_MyWorldSize, CommworldChannels);

    build_vmpi_maps();
    
    /* print_channels(MPID_MyWorldSize, CommworldChannels); */

    /**********************************************************************/
    /* discovering and all-to-all distributing jobstrings (for MPI_Abort) */
    /**********************************************************************/

    {
	int nprocs, myrank;
	int *vector_lengths;

	if (!(MyGlobusGramJobContact = 
		(globus_byte_t *)globus_libc_getenv("GLOBUS_GRAM_JOB_CONTACT")))
	{
	    globus_libc_fprintf(stderr, 
		"ERROR: could not read env variable GLOBUS_GRAM_JOB_CONTACT\n");
	    return 1;
	} /* endif */

	globus_duroc_bootstrap_all_to_all_distribute_bytearray(
	    MyGlobusGramJobContact, 
	    strlen((const char *) MyGlobusGramJobContact)+1,
	    &nprocs, 
	    &myrank,
	    &GramJobcontactsVector,
	    &vector_lengths);
	g_free((void *) vector_lengths);
    }

    return 0;

} /* end globus_init() */

static void create_my_miproto(globus_byte_t **my_miproto, int *nbytes)
{
    char hostname[MAXHOSTNAMELEN];
    char s_port[10];
    char s_tcptype[10];
    char s_nprotos[10];
    globus_io_attr_t attr;
    unsigned short port;
    char *cp;
    int nprotos;
#ifdef VMPI
    int mpi_nbytes;
    char *mpi_miproto;
#endif

    *nbytes = 0;
    nprotos = 0;

#ifdef VMPI
    /*******/
    /* MPI */
    /*******/

    nprotos ++;
    mp_create_miproto(&mpi_miproto, &mpi_nbytes);
    (*nbytes) += mpi_nbytes; /* mpi_nbytes already included 1 for '\0' */
#endif

    /*******/
    /* TCP */
    /*******/

    nprotos ++;

    if (globus_libc_gethostname(hostname, MAXHOSTNAMELEN))
    {
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPICH-G2", 
		    "create_my_miproto() - failed globus_libc_gethostname()" );
    } /* endif */
    globus_io_tcpattr_init(&attr);
    
    /*
     * We need to set the tcp send and receive buffer sizes to something large
     * to deal with the large bandwidth - delay product associated with today's
     * WAN.
     */
    if (MpichGlobus2TcpBufsz > 0)
    {
	globus_io_attr_set_socket_sndbuf(&attr, MpichGlobus2TcpBufsz);
	globus_io_attr_set_socket_rcvbuf(&attr, MpichGlobus2TcpBufsz);
    }

    /*
     * Don't delay small messages; avoiding the extra latency incurred by this
     * delay is probably far more important than saving the little bit of
     * bandwidth eaten by an extra TCP/IP header
     */
    globus_io_attr_set_tcp_nodelay(&attr, GLOBUS_TRUE);
    
    port = 0; /* must be 0 so it will be assigned */
              /* by globus_io_tcp_create_listener */
    globus_io_tcp_create_listener(&port, /* gets assigned */
                                    -1,  /* backlog, same as for listen() */
                                         /* ... whatever that means?      */
                                         /* specifying -1 here results in */
                                         /* a backlog of SOMAXCONN.       */
                                &attr,
                                &Handle); /* gets assigned */
    globus_io_tcpattr_destroy(&attr);

    /* when client connects to socket specified by    */
    /* 'Handle', the callback function will be called */
    globus_io_tcp_register_listen(&Handle,
                                listen_callback,
                                (void *) NULL); /* optional user arg */
                                                /* to be passed to callback */
    globus_libc_sprintf(s_port, "%d", (int) port);
    globus_libc_sprintf(s_tcptype, "%d", tcp);
    (*nbytes) += ((size_t) (strlen(s_tcptype) + 1
                            + strlen(hostname) + 1
                            + strlen(s_port) + 1));

    /************************************************************************/
    /* acquire my_miproto info for other protos here ... increasing *nbytes */
    /************************************************************************/

    /**************************************/
    /* allocating and filling my_miproto */
    /**************************************/
    globus_libc_sprintf(s_nprotos, "%d", nprotos);
    (*nbytes) += ((size_t) (strlen(s_nprotos) + 1));

    g_malloc(*my_miproto, globus_byte_t *, *nbytes);

    cp = (char *) *my_miproto;
    globus_libc_sprintf(cp, "%s ", s_nprotos);
    cp += (strlen(s_nprotos) + 1);

    /* copying MPI */
#ifdef VMPI
    globus_libc_sprintf(cp, "%s ", mpi_miproto);
    cp += mpi_nbytes; /* mpi_nbytes already included 1 for '\0' */
    g_free(mpi_miproto);
#endif
    /* copying TCP */
    globus_libc_sprintf(cp, "%s %s %s", s_tcptype, hostname, s_port);
    cp += (strlen(s_tcptype) + 1  + strlen(hostname) + 1  + strlen(s_port) + 1);

    /*****************************************************************/
    /* appending miproto for other protos to end of *my_miproto here */
    /*****************************************************************/

} /* end create_my_miproto() */

static void build_channels(int nprocs,
                        globus_byte_t **mi_protos_vector,
                        int *mi_protos_vector_lengths,
                        struct channel_t **channels)
{
    int i, j;
    int dummy;
    int nprotos;
    char *cp;
    struct miproto_t *mp;

    g_malloc(*channels, struct channel_t *, nprocs*sizeof(struct channel_t));

    for (i = 0; i < nprocs; i ++)
    {
        (*channels)[i].selected_proto = (struct miproto_t *)  NULL;

        cp = (char *) mi_protos_vector[i];
        sscanf(cp, "%d ", &nprotos);
        cp = index(cp, ' ')+1;

        for (j = 0, mp = (struct miproto_t *) NULL; j < nprotos; j ++)
        {
            if (mp)
            {
                /* not the first proto for this proc */
                g_malloc(mp->next,
			 struct miproto_t *,
			 sizeof(struct miproto_t));
                mp = mp->next;
            }
            else
            {
                /* first proto for this proc */
		g_malloc((*channels)[i].proto_list, 
			struct miproto_t *, 
			sizeof(struct miproto_t));
                mp = (*channels)[i].proto_list;
            } /* endif */
            mp->next = (struct miproto_t *) NULL;
            sscanf(cp, "%d ", &(mp->type));
            cp = index(cp, ' ')+1;
            switch (mp->type)
            {
                case tcp:
                {
                    struct tcp_miproto_t *tp;

		    g_malloc(tp, 
			struct tcp_miproto_t *, 
			sizeof(struct tcp_miproto_t));
		    mp->info = tp;
                    tp->handlep = (struct tcp_rw_handle_t *) NULL;
                    tp->whandle = (globus_io_handle_t *) NULL;
		    globus_mutex_init(&(tp->connection_lock), 
			(globus_mutexattr_t *) GLOBUS_NULL);
		    globus_cond_init(&(tp->connection_cond), 
			(globus_condattr_t *) GLOBUS_NULL);
		    tp->cancel_head = tp->cancel_tail = 
			tp->send_head = tp->send_tail = 
			    (struct tcpsendreq *) NULL;
		    g_malloc(tp->header, globus_byte_t *, Headerlen);
                    sscanf(cp, "%s %d", tp->hostname, &dummy);
                    tp->port = (unsigned short) dummy;
                    cp = index(cp, ' ')+1;
                    cp = index(cp, ' ')+1;
                }
                    break;
                case mpi:
                    g_malloc(mp->info, void *, sizeof(struct mpi_miproto_t));
                    sscanf(cp, "%s %d", 
                ((struct mpi_miproto_t *) (mp->info))->unique_session_string, 
                        &(((struct mpi_miproto_t *) (mp->info))->rank));
                    cp = index(cp, ' ')+1;
                    cp = index(cp, ' ')+1;
                    break;
                default:
		    {
			char err[1024];
			sprintf(err, 
			    "ERROR: build_channles() - encountered "
			    "unrecognized proto type %d", 
			    mp->type);
			MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
				    1, "MPICH-G2", err);
		    }
                    break;
            } /* end switch() */
        } /* endfor */
    } /* endfor */
    
} /* end build_channels() */

static void select_protocols(int myrank, int nprocs, struct channel_t *channels)
{

    struct miproto_t *my_mp, *dest_mp;
    int i;

    for (i = 0; i < nprocs; i ++)
    {
        channels[i].selected_proto = (struct miproto_t *) NULL;

        for (my_mp = channels[myrank].proto_list; 
            !(channels[i].selected_proto) && my_mp; 
                my_mp = my_mp->next)
        {
            for (dest_mp = channels[i].proto_list; 
                !(channels[i].selected_proto) && dest_mp; 
                    dest_mp = dest_mp->next)
            {
                if (my_mp->type == dest_mp->type)
                {
                    switch (my_mp->type)
                    {
                        case tcp: /* auto-match */
                            channels[i].selected_proto = dest_mp; 
                        break; 
#ifdef VMPI
                        case mpi:
                            if (!strcmp(
            ((struct mpi_miproto_t *) (my_mp->info))->unique_session_string,
            ((struct mpi_miproto_t *) (dest_mp->info))->unique_session_string))
                            channels[i].selected_proto = dest_mp; 
                        break;
#endif
                        default:
			    {
				char err[1024];

				globus_libc_sprintf(err,
				    "select_protocols(): unrecognizable "
				    "proto type %d", 
				    my_mp->type);
				MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
					1, "MPICH-G2", err);
			    }
                        break;
                    } /* endif */
                } /* endif */
            } /* endfor */
        } /* endfor */
        if (!(channels[i].selected_proto))
        {
            globus_libc_fprintf(stderr,
		"ERROR: select_protocols(): proc %d could not select proto "
		"to proc %d\n", 
		myrank, 
		i);
            print_channels(MPID_MyWorldSize, channels);
	    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
				    1, "MPICH-G2", "");
        } /* endif */

    } /* endfor */

} /* end select_protocols() */

/*
 * build_vmpi_maps()
 *
 * Construct mapping between the global rank within the vendor MPI's MPI
 * MPI_COMM_WORLD and MPICH's MPI_COMM_WORLD, and vice versa.
 */
static void build_vmpi_maps()
{
#   if defined(VMPI)
    {
	struct channel_t *			channel;
	struct mpi_miproto_t *		mpi_miproto;
	struct miproto_t *			miproto;
	int					i;

	channel = &(CommworldChannels[MPID_MyWorldRank]);
	if ((channel->selected_proto) == NULL)
	{
	    globus_libc_fprintf(stderr, 
		"build_vmpi_maps: (channel->selected_proto) == NULL\n");
	    abort();
	} /* endif */
    
	mpi_miproto = NULL;
	if (channel->selected_proto->type == mpi)
	{
	    mpi_miproto = (struct mpi_miproto_t *)
		(channel->selected_proto->info);
	}
	else
	{
	    /*
	     * At this point we know that the selected proto to myself is NOT
	     * MPI.  In a world in which TCP and MPI are the only protos, this
	     * would be an error; however, later we may add other protos which
	     * are better than MPI (e.g., shm) in which case this would NOT be
	     * an error condition.
	     */
	    miproto = channel->proto_list;
	    while(miproto != NULL && miproto->type != mpi)
	    {
		miproto = miproto->next;
	    }

	    if (miproto != NULL)
	    {
		mpi_miproto = (struct mpi_miproto_t *) (miproto->info);
	    }
	}

	/*
	 * If we can't communicate using the vendor MPI, then we are done
	 */
	if (mpi_miproto == NULL)
	{
	    VMPI_MyWorldSize = 0;
	    VMPI_MyWorldRank = -1;
	    VMPI_GRank_to_VGRank = NULL;
	    VMPI_VGRank_to_GRank = NULL;

	    return;
	}
    
	VMPI_MyWorldRank = mpi_miproto->rank;
	VMPI_MyWorldSize = 0;

	/*
     * construct the mapping from MPICH to VMPI
     */
	g_malloc(VMPI_GRank_to_VGRank, int *, MPID_MyWorldSize * sizeof(int));
    
	for (i = 0; i < MPID_MyWorldSize; i++)
	{
	    struct mpi_miproto_t *		mpi_miproto;

	    miproto = CommworldChannels[i].selected_proto;

	    if (i == MPID_MyWorldRank)
	    {
		/* We might be using a different protocol for communicating
		   with ourself (such as a local buffer copy), but we still
		   need to be included in the map.  Actually, I'm not sure MPI
		   supports communication with oneself, but we include this for
		   consistency anyway */
		VMPI_GRank_to_VGRank[i] = VMPI_MyWorldRank;
		VMPI_MyWorldSize++;
	    }
	    else if (miproto->type == mpi)
	    {
		VMPI_GRank_to_VGRank[i] =
		    ((struct mpi_miproto_t *) (miproto->info))->rank;
		VMPI_MyWorldSize++;
	    }
	    else
	    {
		VMPI_GRank_to_VGRank[i] = -1;
	    }
	}

	/*
	 * construct the mapping from VMPI to MPICH
	 */
	g_malloc(VMPI_VGRank_to_GRank, int *, VMPI_MyWorldSize * sizeof(int));
    
	for (i = 0; i < MPID_MyWorldSize; i++)
	{
	    struct mpi_miproto_t *		mpi_miproto;

	    miproto = CommworldChannels[i].selected_proto;

	    if (VMPI_GRank_to_VGRank[i] >= 0)
	    {
		if (VMPI_GRank_to_VGRank[i] >= VMPI_MyWorldSize)
		{
		    globus_libc_fprintf(stderr, 
			"build_vmpi_maps: "
			"VMPI_GRank_to_VGRank[i] < VMPI_MyWorldSize\n");
		    abort();
		} /* endif */
		VMPI_VGRank_to_GRank[VMPI_GRank_to_VGRank[i]] = i;
	    }
	}
    }
#   endif /* defined(VMPI) */
} /* end build_vmpi_maps() */


void free_vmpi_maps()
{
#   if defined(VMPI)
    {
	g_free(VMPI_GRank_to_VGRank);
	g_free(VMPI_VGRank_to_GRank);
    }
#   endif /* defined(VMPI) */
} /* end free_vmpi_maps() */
