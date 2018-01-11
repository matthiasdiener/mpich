/*
 * This file contains code private to the Nexus implementation of the 
 * ADI device.  Primarily, this contains the code to setup the intial
 * environment and terminate the program.
 */

/* so global vars 'cond_list' (util.h) and 'message_queue_lock' (req.h) */
/* are defined once here and 'extern' everywhere else                   */
#define NEXUSPRIV

/* START NICK DUROC */
#include <stdio.h>
#include <assert.h>
#include <string.h>
/* END NICK DUROC */

#include "mpid.h"
#include "dev.h"
#include "mpimem.h"
#include "../util/queue.h"
#include "nexuspriv.h"

/* START NICK NEW DUROC */
/* #include "subjob_exchange.h" */

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "globus_common.h"
#include "globus_duroc_runtime.h"
#include "globus_duroc_bootstrap.h"
#include "globus_gram_myjob.h"
#include "globus_nexus.h"

#define MPIDGlobusMalloc(Func, Var, Type, Size) \
{ \
    size_t MPIDGlobusMalloc_size = (Size); \
    if (MPIDGlobusMalloc_size > 0) \
    { \
        if (((Var) = (Type) globus_malloc (MPIDGlobusMalloc_size)) == (Type) NULL) \
        { \
            printf("%s: malloc of size %d failed for %s %s in file %s line %d\n", \
                        #Func, MPIDGlobusMalloc_size, #Type, #Var, \
                        __FILE__, __LINE__); \
            exit(1); \
        } \
    } \
    else \
    { \
        (Var) = (Type) NULL; \
    } \
}
 
#define MPIDGlobusFree(Ptr) \
{ \
    if ((Ptr) != NULL) \
    { \
        globus_free(Ptr); \
    } \
}

globus_nexus_startpoint_t *Nexus_nodes;
globus_nexus_endpointattr_t default_ep_attr;
int *remote_formats;

static globus_barrier_t format_barrier;
static int my_argc;
static char **my_argv;
static globus_nexus_endpoint_t default_ep;
static globus_nexus_startpoint_t default_sp;

static void setup_nodes(globus_nexus_startpoint_t *sp_array, int n_nodes);

static void publicize_nodes(int *argc, char ***argv, int next_node,
			    globus_bool_t is_nonthreaded_handler);
static void publicize_formats(int next_node,
			      globus_bool_t is_nonthreaded_handler);
static void wait_for_formats(void);

static void send_contig_handler(globus_nexus_endpoint_t *ep,
				globus_nexus_buffer_t *recv_buf,
			   	globus_bool_t is_nonthreaded_handler);
static void Ssend_done_handler(globus_nexus_endpoint_t *ep,
			       globus_nexus_buffer_t *recv_buf,
			       globus_bool_t is_nonthreaded_handler);
static void send_datatype_handler(globus_nexus_endpoint_t *ep,
			          globus_nexus_buffer_t *recv_buf,
			          globus_bool_t is_nonthreaded_handler);
static void Ssend_datatype_handler(globus_nexus_endpoint_t *ep,
			           globus_nexus_buffer_t *recv_buf,
			           globus_bool_t is_nonthreaded_handler);
static void publicize_nodes_handler(globus_nexus_endpoint_t *ep,
				  globus_nexus_buffer_t *recv_buf,
			   	  globus_bool_t is_nonthreaded_handler);
static void publicize_nodes_reply_handler(globus_nexus_endpoint_t *ep,
				 globus_nexus_buffer_t *recv_buf,
			   	 globus_bool_t is_nonthreaded_handler);
static void publicize_formats_handler(globus_nexus_endpoint_t *ep,
				    globus_nexus_buffer_t *recv_buf,
			   	    globus_bool_t is_nonthreaded_handler);
static void publicize_formats_reply_handler(globus_nexus_endpoint_t *ep,
				    globus_nexus_buffer_t *recv_buf,
			   	    globus_bool_t is_nonthreaded_handler);
static void abort_handler(globus_nexus_endpoint_t *ep,
			    globus_nexus_buffer_t *recv_buf,
			    globus_bool_t is_nonthreaded_handler);

static globus_nexus_handler_t default_handler_table[] =
{
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     send_contig_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     Ssend_done_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     send_datatype_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     Ssend_datatype_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     publicize_nodes_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     publicize_nodes_reply_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     publicize_formats_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     publicize_formats_reply_handler},
    {GLOBUS_NEXUS_HANDLER_TYPE_NON_THREADED,
     abort_handler}
};

void MPID_Globus_Init(int *argc, char ***argv)
{
    int rc;
    globus_nexus_startpoint_t *sp_vector;
    int err;
    int n_orig_nodes;

    globus_module_activate(GLOBUS_NEXUS_MODULE);
    globus_nexus_enable_fault_tolerance(NULL, NULL);
    globus_module_activate(GLOBUS_DUROC_RUNTIME_MODULE);

    globus_duroc_runtime_barrier();

    globus_module_activate(GLOBUS_DUROC_BOOTSTRAP_MODULE);

    MPID_InitQueue();

    globus_mutex_init(&format_barrier.mutex, (globus_mutexattr_t *)NULL);
    globus_cond_init(&format_barrier.cond, (globus_condattr_t *)NULL);
    format_barrier.count = 1;

    globus_nexus_endpointattr_init(&default_ep_attr);
    globus_nexus_endpointattr_set_handler_table(&default_ep_attr,
                                         default_handler_table,
                                         HANDLER_TABLE_SIZE);
    globus_nexus_endpoint_init(&default_ep, &default_ep_attr);
    globus_nexus_startpoint_bind(&default_sp, &default_ep);

    globus_duroc_bootstrap_master_sp_vector(&default_sp, 
					    &n_orig_nodes, 
					    &sp_vector);

    globus_module_deactivate(GLOBUS_DUROC_BOOTSTRAP_MODULE);
    globus_module_deactivate(GLOBUS_DUROC_RUNTIME_MODULE);

/* #ifdef DEBUG_INIT */
    globus_nexus_printf("sp_vector = %x\tn_orig_nodes = %d\n",
    		     sp_vector, n_orig_nodes);
/* #endif */

    if (sp_vector)
    {
	/*
	 * This is the first node in the computation, and is the only
	 * one with nodes.  It must pass them to the others.
	 */
#ifdef DEBUG_INIT
	globus_nexus_printf("first node setting up nodes.\n");
#endif
	setup_nodes(sp_vector, n_orig_nodes);
	
#ifdef DEBUG_INIT
	globus_nexus_printf("first node publicizing nodes.\n");
#endif
	if (n_orig_nodes > 1)
	{
	    publicize_nodes(argc, argv, 1, GLOBUS_FALSE);

#ifdef DEBUG_INIT
	    globus_nexus_printf("first node awaiting formats last node.\n");
#endif
	    wait_for_formats();
	
#ifdef DEBUG_INIT
	    globus_nexus_printf("first node publishing formats to other nodes.\n");
#endif
	    format_barrier.count = 1;
	    publicize_formats(1, GLOBUS_FALSE);
	} /* endif */
    }
    else
    {
#ifdef DEBUG_INIT
	globus_nexus_printf("node awaiting formats.\n");
#endif
	wait_for_formats();
    }

#ifdef DEBUG_INIT
   globus_nexus_printf("done with initialization\n");
#endif

#ifdef JGG
/* Is this necessary? */
#ifdef HAVE_MPL
    signal(SIGALRM, SIG_IGN);
#endif
#endif
}

static void setup_nodes(globus_nexus_startpoint_t *sp_vector, int n_orig_nodes)
{
    int i, j;

    /* MPID_MyWorldSize += n_orig_nodes; */
    MPID_MyWorldSize = n_orig_nodes;

    MPIDGlobusMalloc(setup_nodes(),
		Nexus_nodes,
		globus_nexus_startpoint_t *,
		sizeof(globus_nexus_startpoint_t) * MPID_MyWorldSize);
    
    for (i = 0, j = 0; i < n_orig_nodes; i++)
    {
	globus_nexus_startpoint_copy(&Nexus_nodes[j++], &sp_vector[i]);
	globus_nexus_startpoint_destroy(&sp_vector[i]);
    } /* endfor */
    globus_free(sp_vector);

    MPIDGlobusMalloc(setup_nodes(),
		remote_formats,
		int *,
		sizeof(int) * MPID_MyWorldSize);
    remote_formats[0] = NEXUS_DC_FORMAT_LOCAL;

    MPID_MyWorldRank = 0;
} /* setup_nodes() */

/*
 * If we didn't include the node number in these messages, we would only
 * have to construct the buffer once because all the other data remains
 * the same for all the nodes.
 */
static void publicize_nodes(int *argc, char ***argv, int next_node,
			    globus_bool_t is_nonthreaded_handler)
{
    globus_nexus_buffer_t send_buf;
    globus_nexus_startpoint_t *tmp_nodes;
    int buf_size;
    int arg_len;
    int i, j;

#ifdef DEBUG_INIT
    globus_nexus_printf("%d: publicize_nodes(): entering\n", MPID_MyWorldRank);
#endif
    
    /* Copy the startpoints, so that they can be transferred */
    MPIDGlobusMalloc(publicize_nodes_handler(),
		tmp_nodes,
		globus_nexus_startpoint_t *,
		sizeof(globus_nexus_startpoint_t) * MPID_MyWorldSize);
    for (j = 0; j < MPID_MyWorldSize; j++)
    {
	/* NICK */
	/* globus_nexus_startpoint_copy(&tmp_nodes[j], &MPID_Nexus_nodes[j]); */
	globus_nexus_startpoint_copy(&tmp_nodes[j], &Nexus_nodes[j]);
    }

    /* Figure out the buffer size */
    buf_size = 2 * globus_nexus_sizeof_int(1);
    buf_size = globus_nexus_sizeof_startpoint(Nexus_nodes, MPID_MyWorldSize);
    buf_size += globus_nexus_sizeof_int(MPID_MyWorldSize);
    for (j = 0; j < *argc; j++)
    {
	buf_size += globus_nexus_sizeof_int(1);
	buf_size += globus_nexus_sizeof_char(strlen((*argv)[j]));
    }

    globus_nexus_buffer_init(&send_buf, buf_size, 0);

    /* node id */
    globus_nexus_put_int(&send_buf, &next_node, 1);
    /* num nodes */
    globus_nexus_put_int(&send_buf, &MPID_MyWorldSize, 1);
    /* node startpoints */
    globus_nexus_put_startpoint_transfer(&send_buf,tmp_nodes,MPID_MyWorldSize);
    globus_nexus_put_int(&send_buf, remote_formats, MPID_MyWorldSize);

    /* argc */
    globus_nexus_put_int(&send_buf, argc, 1);
    for (j = 0; j < *argc; j++)
    {
	arg_len = strlen((*argv)[j]);
	/* length of argv[j] */
	globus_nexus_put_int(&send_buf, &arg_len, 1);
	/* argv[j] */
	globus_nexus_put_char(&send_buf, (*argv)[j], arg_len);
    } /* endfor */
	
    globus_nexus_send_rsr(&send_buf,
		   &Nexus_nodes[next_node],
		   PUBLICIZE_NODES_HANDLER_ID,
		   GLOBUS_TRUE,
		   is_nonthreaded_handler);

    MPIDGlobusFree(tmp_nodes);

    my_argc = *argc;
    my_argv = *argv;
} /* publicize_nodes() */

static void publicize_nodes_handler(globus_nexus_endpoint_t *ep,
				    globus_nexus_buffer_t *recv_buf,
				    globus_bool_t is_nonthreaded_handler)
{
    globus_nexus_buffer_t send_buf;
    int buf_size;
    int tmp_int;
    int arg_len;
    int i;
    int next_node;

#ifdef DEBUG_INIT
    globus_nexus_printf("publicize_nodes_handler(): entering\n");
#endif

    globus_nexus_get_int(recv_buf, &MPID_MyWorldRank, 1);
    globus_nexus_get_int(recv_buf, &MPID_MyWorldSize, 1);
    MPIDGlobusMalloc(publicize_nodes_handler(),
		Nexus_nodes,
		globus_nexus_startpoint_t *,
		sizeof(globus_nexus_startpoint_t) * MPID_MyWorldSize);
    MPIDGlobusMalloc(publicize_nodes_handler(),
		remote_formats,
		int *,
		sizeof(int) * MPID_MyWorldSize);
    globus_nexus_get_startpoint(recv_buf, Nexus_nodes, MPID_MyWorldSize);
    globus_nexus_get_int(recv_buf, remote_formats, MPID_MyWorldSize);

    globus_nexus_get_int(recv_buf, &my_argc, 1);
    MPIDGlobusMalloc(publicize_nodes_handler(),
		my_argv,
		char **,
		sizeof(char *) * (my_argc + 1 /* '\0' */));
    for (i = 0; i < my_argc; i++)
    {
	globus_nexus_get_int(recv_buf, &arg_len, 1);
	MPIDGlobusMalloc(publicize_nodes_handler(),
		    my_argv[i],
		    char *,
		    (sizeof(char) * (arg_len + 1)));
	globus_nexus_get_char(recv_buf, my_argv[i], arg_len);
	/* make sure there is a terminating zero at the end */
	my_argv[i][arg_len] = '\0';
    }

    /* fillin my format */
    remote_formats[MPID_MyWorldRank] = NEXUS_DC_FORMAT_LOCAL;
    
    next_node = MPID_MyWorldRank + 1;
    if (next_node < MPID_MyWorldSize)
    {
	/* send startpoints and format on to next node */
	publicize_nodes(&my_argc, &my_argv, next_node, is_nonthreaded_handler);
    }
    else
    {
	/* I am last node, so send formats back to the node 0 */
	buf_size = globus_nexus_sizeof_int(MPID_MyWorldSize);
	globus_nexus_buffer_init(&send_buf, buf_size, 0);
	globus_nexus_put_int(&send_buf, remote_formats, MPID_MyWorldSize);
	globus_nexus_send_rsr(&send_buf,
		       /* NICK */
		       /* &MPID_Nexus_nodes[0], */
		       &Nexus_nodes[0],
		       PUBLICIZE_NODES_REPLY_HANDLER_ID,
		       GLOBUS_TRUE,
		       is_nonthreaded_handler);
    }
} /* publicize_nodes_handler() */

static void publicize_nodes_reply_handler(globus_nexus_endpoint_t *ep,
				 globus_nexus_buffer_t *recv_buf,
				 globus_bool_t is_nonthreaded_handler)
{
    int node_id;

#ifdef DEBUG_INIT
    globus_nexus_printf("%d: publicize_nodes_reply_handler(): entering\n", MPID_MyWorldRank);
#endif

    globus_nexus_get_int(recv_buf, remote_formats, MPID_MyWorldSize);

    globus_mutex_lock(&format_barrier.mutex);
    format_barrier.count--;
    globus_cond_signal(&format_barrier.cond);
    globus_mutex_unlock(&format_barrier.mutex);
} /* publicize_nodes_reply_handler() */

static void wait_for_formats(void)
{
    /* wait for the master node to send the formats to the other nodes */
    globus_mutex_lock(&format_barrier.mutex);
    while (format_barrier.count > 0)
    {
	globus_cond_wait(&format_barrier.cond,
			&format_barrier.mutex);
    }
    globus_mutex_unlock(&format_barrier.mutex);
} /* wait_for_formats() */

static void publicize_formats(int next_node,
			      globus_bool_t is_nonthreaded_handler)
{
    globus_nexus_buffer_t send_buf;
    int buf_size;
    int i;

#ifdef DEBUG_INIT
    globus_nexus_printf("%d: publicize_formats(): entering\n", MPID_MyWorldRank);
#endif
    
    buf_size = globus_nexus_sizeof_int(MPID_MyWorldSize);
    globus_nexus_buffer_init(&send_buf, buf_size, 0);
    globus_nexus_put_int(&send_buf, remote_formats, MPID_MyWorldSize);

    globus_nexus_send_rsr(&send_buf,
		   /* NICK */
		   /* &MPID_Nexus_nodes[next_node], */
		   &Nexus_nodes[next_node],
		   PUBLICIZE_FORMATS_HANDLER_ID,
		   GLOBUS_TRUE,
		   is_nonthreaded_handler);
} /* publicize_formats() */

static void publicize_formats_handler(globus_nexus_endpoint_t *ep,
				      globus_nexus_buffer_t *recv_buf,
				      globus_bool_t is_nonthreaded_handler)
{
    globus_nexus_buffer_t send_buf;
    int next_node;
    
#ifdef DEBUG_INIT
    globus_nexus_printf("publicize_formats_handler() called \n");
#endif
    
    globus_nexus_get_int(recv_buf, remote_formats, MPID_MyWorldSize);

    /* 
     * Send the formats on to the next node.
     * Wrap around to node 0 to signal completion.
     */
    next_node = MPID_MyWorldRank + 1;
    if (next_node < MPID_MyWorldSize)
    {
	/* send startpoints and format on to next node */
	publicize_formats(next_node, is_nonthreaded_handler);
    }
    else
    {
	/* I am last node, so send empty message node 0 */
	/* so that it can signal completion itself      */
	globus_nexus_buffer_init(&send_buf, 0, 0);
	globus_nexus_send_rsr(&send_buf,
		       /* NICK */
		       /* &MPID_Nexus_nodes[0], */
		       &Nexus_nodes[0],
		       PUBLICIZE_FORMATS_REPLY_HANDLER_ID,
		       GLOBUS_TRUE,
		       is_nonthreaded_handler);
    } /* endif */
    
    globus_mutex_lock(&format_barrier.mutex);
    format_barrier.count--;
    globus_cond_signal(&format_barrier.cond);
    globus_mutex_unlock(&format_barrier.mutex);

} /* publicize_formats_handler() */

static void publicize_formats_reply_handler(globus_nexus_endpoint_t *ep,
					  globus_nexus_buffer_t *recv_buf,
					  globus_bool_t is_nonthreaded_handler)
{
#ifdef DEBUG_INIT
    globus_nexus_printf("publicize_formats_reply_handler() called \n");
#endif
    
    globus_mutex_lock(&format_barrier.mutex);
    format_barrier.count--;
    globus_cond_signal(&format_barrier.cond);
    globus_mutex_unlock(&format_barrier.mutex);

} /* publicize_formats_reply_handler() */

static void abort_handler(globus_nexus_endpoint_t *ep,
			  globus_nexus_buffer_t *recv_buf,
			  globus_bool_t is_nonthreaded_handler)
{

    int exit_code;

    globus_nexus_get_int(recv_buf, &exit_code, 1);
    exit(exit_code);
} /* abort_handler() */

static void Ssend_done_handler(globus_nexus_endpoint_t *ep,
			       globus_nexus_buffer_t *recv_buf,
			       globus_bool_t is_nonthreaded_handler)
{
    MPIR_RHANDLE *rhandle;

    rhandle = (MPIR_RHANDLE *)globus_nexus_endpoint_get_user_pointer(ep);
    globus_nexus_endpoint_destroy(&(rhandle->endpoint));

    globus_mutex_lock(&message_queue_lock);
    rhandle->is_complete = GLOBUS_TRUE;
    if (rhandle->cond)
    {
	globus_cond_signal(rhandle->cond);
    }
    globus_mutex_unlock(&message_queue_lock);
} /* Ssend_done_handler() */

static void send_datatype_handler(globus_nexus_endpoint_t *ep,
			          globus_nexus_buffer_t *recv_buf,
			          globus_bool_t is_nonthreaded_handler)
{
    MPIR_RHANDLE *rhandle;
    int sender;
    int tag;
    int context_id;
    int found;
    int count;
    int dataorigin_unitnonpacksize;
    int dataorigin_maxnonpacksize;
    int dataorigin_nonpacksize;
    int dataorigin_format;
    int dummy;
    int recvd_nbytes;

    globus_nexus_get_int(recv_buf, &sender, 1);
    globus_nexus_get_int(recv_buf, &tag, 1);
    globus_nexus_get_int(recv_buf, &context_id, 1);
    globus_nexus_get_int(recv_buf, &dataorigin_nonpacksize, 1);
    globus_nexus_get_int(recv_buf, &dataorigin_format, 1);
/* globus_nexus_printf("NICK: send_datatype_handler() just pulled sender %d tag %d context_id %d dataorigin_nonpacksize %d dataorigin_format %d\n", sender, tag, context_id, dataorigin_nonpacksize, dataorigin_format); */

#ifdef DEBUG2
    globus_nexus_printf(
	"+send_datatype_handler send = %d tag = %d context = %d ct = %d\n",
	sender, tag, context_id, count);
#endif

    globus_mutex_lock(&message_queue_lock);
    MPID_Msg_arrived(sender,
		     tag,
		     context_id,
		     &rhandle,
		     &found);

    rhandle->dataorigin_format = rhandle->s.private_count = dataorigin_format;
    rhandle->dataorigin_nonpacksize = rhandle->s.count = dataorigin_nonpacksize;
    SET_STATUSCOUNT_ISDATAORIGIN(rhandle->s.private_count)
    if (found)
    {
	/* RECV already issued */

/* globus_nexus_printf("NICK: send_datatype_handler(): RCV fromrank %d tag %d context %d: dataorigin_nonpacksize %d: ... rcv already posted count %d type %d size %d source %d tag %d\n", sender, tag, context_id, dataorigin_nonpacksize, rhandle->count, rhandle->type->dte_type, rhandle->type->size, rhandle->s.MPI_SOURCE, rhandle->s.MPI_TAG); */
	rhandle->s.MPI_ERROR = 0;

/* #ifdef DEBUG */
	if (rhandle->count % rhandle->type->size)
	{
	globus_fatal("send_datatype_handler(): rhandle->count %d is NOT a multiple of rhandle->type->size %d\n", rhandle->count, rhandle->type->size);
	} /* endif */
/* #endif */

/* globus_nexus_printf("NICK: send_datatype_handler(): RCV fromrank %d tag %d context %d: dataorigin_nonpacksize %d: count %d type %d size %d: before MPID_extract_data()\n", sender, tag, context_id, dataorigin_nonpacksize, rhandle->count, rhandle->type->dte_type, rhandle->type->size); */
	MPID_extract_data(rhandle,
			recv_buf,
			rhandle->count / rhandle->type->size, /* req nelem */
			rhandle->type,
			rhandle->buf,
			NULL, /* optional error_code */
			&recvd_nbytes);
/* globus_nexus_printf("NICK: send_datatype_handler(): RCV fromrank %d tag %d context %d: dataorigin_nonpacksize %d: count %d type %d size %d: after MPID_extract_data() recvd_nbytes %d\n", sender, tag, context_id, dataorigin_nonpacksize, rhandle->count, rhandle->type->dte_type, rhandle->type->size, recvd_nbytes); */

	rhandle->count = rhandle->s.count = recvd_nbytes;
	SET_STATUSCOUNT_ISLOCAL(rhandle->s.private_count)
	rhandle->is_complete = GLOBUS_TRUE;
	if (rhandle->cond)
	{
	    globus_cond_signal(rhandle->cond);
	}
    }
    else
    {
	/* RECV not issued yet */

/* globus_nexus_printf("NICK: send_datatype_handler(): RCV fromrank %d tag %d context %d: dataorigin_nonpacksize %d type UNKNOWN: ... rcv not yet posted\n", sender, tag, context_id, dataorigin_nonpacksize); */

	/* add message to unexpected queue */
	/* NICK: these are now calculated in adi2hrecv.c:MPID_RecvDatatype() */
	/* rhandle->count = rhandle->s.count = count; */
        rhandle->is_complete = GLOBUS_FALSE;
#ifdef DEBUG2
	globus_nexus_printf("send_datatype_handler(): setting rhandle->count to %d\n", count);
#endif
	globus_nexus_buffer_save(recv_buf);
        rhandle->recv_buf = *recv_buf;
    }
    /* NICK */
    /* globus_mutex_unlock(&MPID_message_queue_lock); */
    globus_mutex_unlock(&message_queue_lock);
/* globus_nexus_printf("NICK: exiting send_datatype_handler()\n"); */
} /* send_datatype_handler() */

static void Ssend_datatype_handler(globus_nexus_endpoint_t *ep,
			           globus_nexus_buffer_t *recv_buf,
			           globus_bool_t is_nonthreaded_handler)
{
    globus_nexus_buffer_t send_buf;
    MPIR_RHANDLE *rhandle;
    int found;
    int sender;
    int tag;
    int context_id;
    int count;
    int dataorigin_unitnonpacksize;
    int dataorigin_maxnonpacksize;
    int dataorigin_nonpacksize;
    int dataorigin_format;
    int dummy;
    int recvd_nbytes;
/* globus_nexus_printf("NICK: enter Ssend_datatype_handler()\n"); */

    globus_nexus_get_int(recv_buf, &sender, 1);
    globus_nexus_get_int(recv_buf, &tag, 1);
    globus_nexus_get_int(recv_buf, &context_id, 1);
    globus_nexus_get_int(recv_buf, &dataorigin_nonpacksize, 1);
    globus_nexus_get_int(recv_buf, &dataorigin_format, 1);
    globus_mutex_lock(&message_queue_lock);
    MPID_Msg_arrived(sender,
		     tag,
		     context_id,
		     &rhandle,
		     &found);
    rhandle->dataorigin_format = rhandle->s.private_count = dataorigin_format;
    rhandle->dataorigin_nonpacksize = rhandle->s.count = dataorigin_nonpacksize;
    SET_STATUSCOUNT_ISDATAORIGIN(rhandle->s.private_count)
    globus_nexus_get_startpoint(recv_buf, &(rhandle->sp), 1);
    rhandle->sptr = &(rhandle->sp);
     /* Note: field "sptr" is set to NULL after any call to MPID_RecvAlloc() */
    if (found)
    {

	/* RECV already issued */
	rhandle->s.MPI_ERROR = 0;

/* #ifdef DEBUG */
	if (rhandle->count % rhandle->type->size)
	{
	globus_fatal("send_datatype_handler(): rhandle->count %d is NOT a multiple of rhandle->type->size %d\n", rhandle->count, rhandle->type->size);
	} /* endif */
/* #endif */

	MPID_extract_data(rhandle,
			recv_buf,
			rhandle->count / rhandle->type->size, /* req nelem */
			rhandle->type,
			rhandle->buf,
			NULL, /* optional error_code */
			&recvd_nbytes);
	rhandle->count = rhandle->s.count = recvd_nbytes;
	SET_STATUSCOUNT_ISLOCAL(rhandle->s.private_count)
		/* put message into user space */
	/* tell sender message has been received */
	globus_nexus_buffer_init(&send_buf, 0, 0);
	globus_nexus_send_rsr(&send_buf,
		       &(rhandle->sp),
		       SSEND_DONE_ID,
		       GLOBUS_TRUE,
		       is_nonthreaded_handler);
        rhandle->is_complete = GLOBUS_TRUE;
        if (rhandle->cond)
	{
	    globus_cond_signal(rhandle->cond);
	}
    }
    else
    {
	/* RECV not issued yet */

        /* NICK: these are now calculated in adi2hrecv.c:MPID_RecvDatatype() */
	/* rhandle->count = rhandle->s.count = count; */
#ifdef DEBUG2
	globus_nexus_printf("Ssend_datatype_handler(): setting rhandle->count to %d\n", count);
#endif
	globus_nexus_buffer_save(recv_buf);
	rhandle->recv_buf = *recv_buf;
    }
    globus_mutex_unlock(&message_queue_lock);
} /* Ssend_datatype_handler() */

/*
 * This needs to be filled in later
 */
void send_contig_handler(globus_nexus_endpoint_t *ep,
			 globus_nexus_buffer_t *recv_buf,
			 globus_bool_t is_nonthreaded_handler)
{
} /* send_contig_handler() */

