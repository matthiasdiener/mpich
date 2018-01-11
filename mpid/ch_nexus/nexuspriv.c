/*
 * This file contains code private to the Nexus implementation of the 
 * ADI device.  Primarily, this contains the code to setup the intial
 * environment and terminate the program.
 */
#include "mpid.h"
#include "dev.h"
#include "mpimem.h"
#include "../util/queue.h"
#include "nexuspriv.h"

nexus_startpoint_t *Nexus_nodes;
nexus_endpointattr_t default_ep_attr;
int *remote_formats;

static nexus_barrier_t node_barrier;
static nexus_barrier_t format_barrier;
static int my_argc;
static char **my_argv;
static nexus_endpoint_t default_ep;

static void setup_nodes(nexus_node_t *orig_nodes, int n_nodes);
static void publicize_nodes(int *argc, char ***argv);
static void publicize_formats(void);
static void wait_for_sps(int *argc, char ***argv);
static void wait_for_formats(void);

static void send_contig_handler(nexus_endpoint_t *ep,
				nexus_buffer_t *recv_buf,
			   	nexus_bool_t called_from_non_threaded_handler);
static void Ssend_done_handler(nexus_endpoint_t *ep,
			       nexus_buffer_t *recv_buf,
			       nexus_bool_t called_from_non_threaded_handler);
static void send_datatype_handler(nexus_endpoint_t *ep,
			          nexus_buffer_t *recv_buf,
			          nexus_bool_t called_from_non_threaded_handler);
static void Ssend_datatype_handler(nexus_endpoint_t *ep,
			           nexus_buffer_t *recv_buf,
			           nexus_bool_t called_from_non_threaded_handler);
static void initial_nodes_handler(nexus_endpoint_t *ep,
				  nexus_buffer_t *recv_buf,
			   	  nexus_bool_t called_from_non_threaded_handler);
static void send_formats_handler(nexus_endpoint_t *ep,
				 nexus_buffer_t *recv_buf,
			   	 nexus_bool_t called_from_non_threaded_handler);
static void receive_formats_handler(nexus_endpoint_t *ep,
				    nexus_buffer_t *recv_buf,
			   	    nexus_bool_t called_from_non_threaded_handler);

static nexus_handler_t default_handler_table[] =
{ \
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)send_contig_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)Ssend_done_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)send_datatype_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)Ssend_datatype_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)initial_nodes_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)send_formats_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t)receive_formats_handler},
};

int NexusBoot(nexus_startpoint_t *sp)
{
    nexus_endpointattr_init(&default_ep_attr);
    nexus_endpointattr_set_handler_table(&default_ep_attr,
					 default_handler_table,
					 HANDLER_TABLE_SIZE);
    nexus_endpoint_init(&default_ep, &default_ep_attr);
    nexus_startpoint_bind(sp, &default_ep);

    return NEXUS_SUCCESS;
}

int NexusExit(void)
{
    /* do nothing */
    return NEXUS_SUCCESS;
}

int NexusAcquiredAsNode(nexus_startpoint_t *sp)
{
    /*
     * No one should currently be trying to attach to us, so this is
     * a serious security problem.  FAIL big time!
     */
    return NEXUS_FAILURE;
}

void MPID_Nexus_Init(int *argc, char ***argv)
{
    nexus_node_t *orig_nodes;
    int n_orig_nodes;

    MPID_InitQueue();

    nexus_mutex_init(&node_barrier.mutex, (nexus_mutexattr_t *)NULL);
    nexus_cond_init(&node_barrier.cond, (nexus_condattr_t *)NULL);
    node_barrier.count = 1;

    nexus_mutex_init(&format_barrier.mutex, (nexus_mutexattr_t *)NULL);
    nexus_cond_init(&format_barrier.cond, (nexus_condattr_t *)NULL);
    format_barrier.count = 1;

    nexus_init(argc, argv,
	       "MPI_ARGS", "mpi",
	       NULL, NULL, NULL, NULL,
	       &orig_nodes, &n_orig_nodes);
    nexus_start_nonblocking();

#ifdef DEBUG
    nexus_printf("orig_nodes = %x\tn_orig_nodes = %d\n",
    		     orig_nodes, n_orig_nodes);
#endif

    if (orig_nodes)
    {
	/*
	 * This is the first node in the computation, and is the only
	 * one with nodes.  It must pass them to the others.
	 */
#ifdef DEBUG
    nexus_printf("first node setting up nodes.\n");
#endif
	setup_nodes(orig_nodes, n_orig_nodes);
#ifdef DEBUG
    nexus_printf("first node publicizing nodes.\n");
#endif
	publicize_nodes(argc, argv);

#ifdef DEBUG
    nexus_printf("first node awaiting formats from other nodes.\n");
#endif
	wait_for_formats();
#ifdef DEBUG
    nexus_printf("first node publishing formats to other nodes.\n");
#endif
	publicize_formats();
    }
    else
    {
#ifdef DEBUG
    nexus_printf("node awaiting sps.\n");
#endif
        wait_for_sps(argc, argv);
#ifdef DEBUG
    nexus_printf("node awaiting formats.\n");
#endif
	wait_for_formats();
    }

#ifdef DEBUG
   nexus_printf("done with initialization\n");
#endif

#ifdef JGG
/* Is this necessary? */
#ifdef HAVE_MPL
    signal(SIGALRM, SIG_IGN);
#endif
#endif
}

void MPID_Nexus_End(void)
{
    nexus_context_destroy(NEXUS_FALSE);
}

static void setup_nodes(nexus_node_t *orig_nodes, int n_orig_nodes)
{
    int i, j;
/* HACK */
    nexus_startpoint_bind(&orig_nodes[0].startpoint, &default_ep);
/* HACK */

    /*
     * I would like to have only one for loop where I allocate a node
     * and copy it, but ports0_realloc doesn't seem to be working
     * properly.
     */
    for (i = 0; i < n_orig_nodes; i++)
    {
	if (orig_nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    MPID_MyWorldSize++;
	}
    }

    NexusMalloc(setup_nodes(),
		Nexus_nodes,
		nexus_startpoint_t *,
		sizeof(nexus_startpoint_t) * MPID_MyWorldSize);
    
    for (i = 0, j = 0; i < n_orig_nodes; i++)
    {
	if (orig_nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    nexus_startpoint_copy(&Nexus_nodes[j++],
	    			  &orig_nodes[i].startpoint);
	}
	nexus_startpoint_destroy(&orig_nodes[i].startpoint);
    }
    nexus_free(orig_nodes);

    NexusMalloc(setup_nodes(),
		remote_formats,
		int *,
		sizeof(int) * MPID_MyWorldSize);
    remote_formats[0] = NEXUS_DC_FORMAT_LOCAL;
} /* setup_nodes() */

/*
 * If we didn't include the node number in these messages, we would only
 * have to construct the buffer once because all the other data remains
 * the same for all the nodes.
 */
static void publicize_nodes(int *argc, char ***argv)
{
    nexus_buffer_t send_buf;
    int buf_size;
    int arg_len;
    int i, j;

    buf_size = nexus_sizeof_startpoint(Nexus_nodes, MPID_MyWorldSize);
    for (j = 0; j < *argc; j++)
    {
	buf_size += nexus_sizeof_char(strlen((*argv)[j]));
    }

    /* since the master node is designated as 0, skip it */
    for (i = 1; i < MPID_MyWorldSize; i++)
    {
/* GKT: You cannot declare a variable length array in any C compiler, except
 *      gcc to my knowledge. So I have replaced that code with this:
 */
        nexus_startpoint_t *tmp_nodes;
        NexusMalloc(initial_nodes_handler(),
		tmp_nodes,
		nexus_startpoint_t *,
		sizeof(nexus_startpoint_t) * MPID_MyWorldSize);


        nexus_buffer_init(&send_buf, buf_size, 0);

	/* node id */
        nexus_put_int(&send_buf, &i, 1);
	/* num nodes */
        nexus_put_int(&send_buf, &MPID_MyWorldSize, 1);
	/* node startpoints */
	for (j = 0; j < MPID_MyWorldSize; j++)
	{
	    nexus_startpoint_copy(&tmp_nodes[j], &Nexus_nodes[j]);
	}
	nexus_put_startpoint_transfer(&send_buf, tmp_nodes, MPID_MyWorldSize);

	/* argc */
	nexus_put_int(&send_buf, argc, 1);
	for (j = 0; j < *argc; j++)
	{
	    arg_len = strlen((*argv)[j]);
	    /* length of argv[j] */
	    nexus_put_int(&send_buf, &arg_len, 1);
	    /* argv[j] */
	    nexus_put_char(&send_buf, (*argv)[j], arg_len);
	}
	
	nexus_send_rsr(&send_buf,
		       &Nexus_nodes[i],
		       INITIAL_NODES_HANDLER_ID,
		       NEXUS_TRUE,
		       NEXUS_FALSE);
/*
 * GKT: It seems kind of foolish to re-allocate this again, but I am trying
 *      to preserve the original spirit of JGG's code. We'll optimize these
 *      sorts of problems/comments later, when it works.
 */

        NexusFree(tmp_nodes);
    }

    my_argc = *argc;
    my_argv = *argv;
} /* publicize_nodes() */

static void publicize_formats(void)
{
    nexus_buffer_t send_buf;
    int buf_size;
    int i;

    buf_size = nexus_sizeof_int(MPID_MyWorldSize);
    nexus_buffer_init(&send_buf, buf_size, 0);
    nexus_put_int(&send_buf, remote_formats, MPID_MyWorldSize);

    /* Master node already has this, so skip it */
    for (i = 1; i < MPID_MyWorldSize; i++)
    {
	nexus_send_rsr(&send_buf,
		       &Nexus_nodes[i],
		       RECEIVE_FORMATS_ID,
		       NEXUS_FALSE,
		       NEXUS_FALSE);
    }
    nexus_buffer_destroy(&send_buf);
} /* publicize_formats() */

static void wait_for_sps(int *argc, char ***argv)
{
    /* wait for the master node to send the sps to the other nodes */
    nexus_mutex_lock(&node_barrier.mutex);
    while (node_barrier.count)
    {
	nexus_cond_wait(&node_barrier.cond,
			&node_barrier.mutex);
    }
    nexus_mutex_unlock(&node_barrier.mutex);

    *argc = my_argc;
    *argv = my_argv;
} /* wait_for_sps() */

static void wait_for_formats(void)
{
    /* wait for the master node to send the formats to the other nodes */
    nexus_mutex_lock(&format_barrier.mutex);
#ifdef DEBUG
    nexus_printf("waiting for formats\n");
#endif
    while (format_barrier.count < MPID_MyWorldSize)
    {
#ifdef DEBUG
    nexus_printf("format barrier count = %d\n" , format_barrier.count);
#endif
    while (format_barrier.count < MPID_MyWorldSize)
	nexus_cond_wait(&format_barrier.cond,
			&format_barrier.mutex);
    }
    nexus_mutex_unlock(&format_barrier.mutex);
} /* wait_for_formats() */

static void initial_nodes_handler(nexus_endpoint_t *ep,
				  nexus_buffer_t *recv_buf,
			   	  nexus_bool_t called_from_non_threaded_handler)
{
    nexus_buffer_t send_buf;
    int buf_size;
    int tmp_int;
    int arg_len;
    int i;

#ifdef DEBUG

    nexus_printf("initial nodes handler called\n");
#endif
    nexus_get_int(recv_buf, &MPID_MyWorldRank, 1);
    nexus_get_int(recv_buf, &MPID_MyWorldSize, 1);
    NexusMalloc(initial_nodes_handler(),
		Nexus_nodes,
		nexus_startpoint_t *,
		sizeof(nexus_startpoint_t) * MPID_MyWorldSize);
    NexusMalloc(initial_nodes_handler(),
		remote_formats,
		int *,
		sizeof(int) * MPID_MyWorldSize);
    nexus_get_startpoint(recv_buf, Nexus_nodes, MPID_MyWorldSize);

    nexus_get_int(recv_buf, &my_argc, 1);
    NexusMalloc(initial_nodes_handler(),
		my_argv,
		char **,
		sizeof(char *) * (my_argc + 1 /* '\0' */));
    for (i = 0; i < my_argc; i++)
    {
	nexus_get_int(recv_buf, &arg_len, 1);
	NexusMalloc(initial_nodes_handler(),
		    my_argv[i],
		    char *,
		    sizeof(char) * arg_len);
	nexus_get_char(recv_buf, my_argv[i], arg_len);
	/* make sure there is a terminating zero at the end */
	my_argv[i][arg_len] = '\0';
    }

    /* send master node our format */
    buf_size = nexus_sizeof_int(1) * 2;
    nexus_buffer_init(&send_buf, buf_size, 0);
    nexus_put_int(&send_buf, &MPID_MyWorldRank, 1);
    tmp_int = NEXUS_DC_FORMAT_LOCAL;
    nexus_put_int(&send_buf, &tmp_int, 1);
    nexus_send_rsr(&send_buf,
    		   &Nexus_nodes[0],
		   SEND_FORMATS_ID,
		   NEXUS_TRUE,
		   called_from_non_threaded_handler);
    /*
     * we have now received the sps for the other nodes, so we can
     * signal the thread waiting for them
     */
    nexus_mutex_lock(&node_barrier.mutex);
    node_barrier.count = 0;
    nexus_cond_signal(&node_barrier.cond);
    nexus_mutex_unlock(&node_barrier.mutex);
} /* initial_nodes_handler() */

static void send_formats_handler(nexus_endpoint_t *ep,
				 nexus_buffer_t *recv_buf,
				 nexus_bool_t called_from_non_threaded_handler)
{
    int node_id;

#ifdef DEBUG

    nexus_printf("send formats handler called\n");
#endif
    nexus_get_int(recv_buf, &node_id, 1);
    nexus_get_int(recv_buf, &remote_formats[node_id], 1);

    nexus_mutex_lock(&format_barrier.mutex);
    format_barrier.count++;
    nexus_cond_signal(&format_barrier.cond);
    nexus_mutex_unlock(&format_barrier.mutex);
} /* send_formats_handler() */

static void receive_formats_handler(nexus_endpoint_t *ep,
				    nexus_buffer_t *recv_buf,
				    nexus_bool_t called_from_non_threaded_handler)
{
#ifdef DEBUG
    nexus_printf("Receive formats handler called \n");
#endif
    nexus_get_int(recv_buf, remote_formats, MPID_MyWorldSize);

    nexus_mutex_lock(&format_barrier.mutex);
    format_barrier.count = MPID_MyWorldSize;
    nexus_cond_signal(&format_barrier.cond);
    nexus_mutex_unlock(&format_barrier.mutex);
} /* receive_formats_handler() */

static void Ssend_done_handler(nexus_endpoint_t *ep,
			       nexus_buffer_t *recv_buf,
			       nexus_bool_t called_from_non_threaded_handler)
{
    MPIR_RHANDLE *rhandle;

    rhandle = (MPIR_RHANDLE *)nexus_endpoint_get_user_pointer(ep);
    nexus_endpoint_destroy(&(rhandle->endpoint));

    nexus_mutex_lock(&message_queue_lock);
    rhandle->is_complete = NEXUS_TRUE;
    if (rhandle->cond)
    {
	nexus_cond_signal(&rhandle->cond);
    }
    nexus_mutex_unlock(&message_queue_lock);
} /* Ssend_done_handler() */

static void send_datatype_handler(nexus_endpoint_t *ep,
			          nexus_buffer_t *recv_buf,
			          nexus_bool_t called_from_non_threaded_handler)
{
    MPIR_RHANDLE *rhandle;
    int sender;
    int tag;
    int context_id;
    int found;
    int count;

    nexus_get_int(recv_buf, &sender, 1);
    nexus_get_int(recv_buf, &tag, 1);
    nexus_get_int(recv_buf, &context_id, 1);
    nexus_get_int(recv_buf, &count, 1);

#ifdef DEBUG2
    nexus_printf(
	"+send_datatype_handler send = %d tag = %d context = %d ct = %d\n",
	sender, tag, context_id, count);
#endif

    nexus_mutex_lock(&message_queue_lock);
    MPID_Msg_arrived(sender,
		     tag,
		     context_id,
		     &rhandle,
		     &found);
    if (found)
    {
	rhandle->s.MPI_ERROR = 0;

	if (rhandle->count < count)
	{
#ifdef DEBUG
            nexus_printf("%s:%d ", __FILE__, __LINE__);
	    nexus_printf(" send_datatype_handler: truncation detected\n");
            nexus_printf("%s:%d ", __FILE__, __LINE__);
	    nexus_printf(" send_datatype_handler: rhandle->count = %d count = %d",
		rhandle->count, count);
#endif
	    rhandle->s.MPI_ERROR = MPI_ERR_TRUNCATE;
	}
	else
	{
            rhandle->count = rhandle->s.count = count;

#ifdef DEBUG
            nexus_printf("send_datatype_handler rhandle->count = %d\n",
                         rhandle->count);
#endif
            MPID_Unpack_buffer(rhandle->buf,
			   rhandle->count / rhandle->type->size,
			   rhandle->type,
			   recv_buf);

	}
	rhandle->is_complete = NEXUS_TRUE;
	if (rhandle->cond)
	{
	    nexus_cond_signal(rhandle->cond);
	}
    }
    else
    {
	/* add message to unexpected queue */
	rhandle->count = rhandle->s.count = count;
        rhandle->is_complete = NEXUS_FALSE;
#ifdef DEBUG2
	nexus_printf("send_datatype_handler(): setting rhandle->count to %d\n", count);
#endif
	nexus_buffer_save(recv_buf);
        rhandle->recv_buf = *recv_buf;
    }
    nexus_mutex_unlock(&message_queue_lock);
} /* send_datatype_handler() */

static void Ssend_datatype_handler(nexus_endpoint_t *ep,
			           nexus_buffer_t *recv_buf,
			           nexus_bool_t called_from_non_threaded_handler)
{
    nexus_buffer_t send_buf;
    MPIR_RHANDLE *rhandle;
    int found;
    int sender;
    int tag;
    int context_id;
    int count;

    nexus_get_int(recv_buf, &sender, 1);
    nexus_get_int(recv_buf, &tag, 1);
    nexus_get_int(recv_buf, &context_id, 1);
    nexus_get_int(recv_buf, &count, 1);
    nexus_mutex_lock(&message_queue_lock);
    MPID_Msg_arrived(sender,
		     tag,
		     context_id,
		     &rhandle,
		     &found);
    nexus_get_startpoint(recv_buf, &(rhandle->sp), 1);
    rhandle->sptr = &(rhandle->sp);
     /* Note: field "sptr" is set to NULL after any call to MPID_RecvAlloc() */
    if (found)
    {

	rhandle->s.MPI_ERROR = 0;

	if (rhandle->count < count)
	{
#ifdef DEBUG
            nexus_printf("%s:%d ", __FILE__, __LINE__);
	    nexus_printf(" Ssend_datatype_handler: truncation detected\n");
            nexus_printf("%s:%d ", __FILE__, __LINE__);
	    nexus_printf(" Ssend_datatype_handler: rhandle->count = %d count = %d\n",
		rhandle->count, count);
#endif
	    rhandle->s.MPI_ERROR = MPI_ERR_TRUNCATE;
	}
	else
	{
	    rhandle->count = rhandle->s.count = count;
#ifdef DEBUG
            nexus_printf("Ssend datatype handler. rhandle->count = %d\n",
                         rhandle->count);
#endif
            
            
	    MPID_Unpack_buffer(rhandle->buf,
			   rhandle->count / rhandle->type->size,
			   rhandle->type,
			   recv_buf);

	
	}
	/* put message into user space */
	/* tell sender message has been received */
	nexus_buffer_init(&send_buf, 0, 0);
	nexus_send_rsr(&send_buf,
		       &(rhandle->sp),
		       SSEND_DONE_ID,
		       NEXUS_TRUE,
		       called_from_non_threaded_handler);
        rhandle->is_complete = NEXUS_TRUE;
        if (rhandle->cond)
	{
	    nexus_cond_signal(rhandle->cond);
	}
    }
    else
    {

	rhandle->count = rhandle->s.count = count;
#ifdef DEBUG2
	nexus_printf("Ssend_datatype_handler(): setting rhandle->count to %d\n", count);
#endif
	nexus_buffer_save(recv_buf);
	rhandle->recv_buf = *recv_buf;
    }
    nexus_mutex_unlock(&message_queue_lock);
} /* Ssend_datatype_handler() */

/*
 * This needs to be filled in later
 */
void send_contig_handler(nexus_endpoint_t *ep,
			 nexus_buffer_t *recv_buf,
			 nexus_bool_t called_from_non_threaded_handler)
{
} /* send_contig_handler() */

