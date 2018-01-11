#include "globdev.h"
#include "reqalloc.h"
#include "queue.h" /* for queue traversal stuff in cancelling message  */
		   /* would prefer to have that code removed from here */
		   /* and not have to include queue.h here             */

/********************/
/* Global Variables */
/********************/
extern globus_io_handle_t Handle; 
extern globus_mutex_t     MessageQueuesLock;
extern volatile int       TcpOutstandingRecvReqs;
extern struct channel_t * CommworldChannels;
extern globus_size_t      Headerlen;
int			  MpichGlobus2TcpBufsz = 0;

static void data_arrived(struct tcp_rw_handle_t *rwhp);
static void send_cancel_result_over_tcp(int grank, 
					int result, 
					void *liba,
					int libasize,
					long msgid_sec,
					long msgid_usec,
					unsigned long msgid_ctr);

/**********************/
/* Callback functions */
/**********************/

/* called when a globus_io sends data here after reading off socket */
void read_callback(void *callback_arg, 
                    globus_io_handle_t *handle, 
                    globus_result_t result,
                    globus_byte_t *buff,   /* where date resides */
                    globus_size_t nbytes)  /* number of bytes read */
{
    struct tcp_rw_handle_t *rwhp = (struct tcp_rw_handle_t *) callback_arg;

/* globus_libc_printf("%d: enter read_callback: state=", MPID_MyWorldRank); switch (rwhp->state) { case await_instructions: globus_libc_printf("await_instructions\n"); break; case await_format: globus_libc_printf("await_format\n"); break; case await_header: globus_libc_printf("await_header\n"); break; case await_data: globus_libc_printf("await_data\n"); break; default: globus_libc_printf("unknown\n"); break; } fflush(stdout); */

    if (result != GLOBUS_SUCCESS)
    {
	globus_object_t* err = globus_error_get(result);

	/* ignoring EOF and cancellation (shutdown) errors */
	if (!globus_object_type_match(globus_object_get_type(err),
				      GLOBUS_IO_ERROR_TYPE_EOF) &&
	    !globus_object_type_match(globus_object_get_type(err),
				      GLOBUS_IO_ERROR_TYPE_IO_CANCELLED))
	{
	    /* things are very bad */
	    char *errstring = globus_object_printable_to_string(err);
	    char msg[1024];
	    
	    globus_libc_sprintf(
		msg,
		"read failure - %s, state=%s",
		errstring,
		(rwhp->state == await_instructions
		 ? "await_instructions"
		 : (rwhp->state == await_format 
		    ? "await_format" 
		    : (rwhp->state == await_header 
		       ? "await_header" 
		       : (rwhp->state == await_data 
			  ? "await_data" 
			  : "unknown")))));
	    MPID_Abort(NULL, 0, "MPICH-G2", msg);
	}
	/* endif */

	globus_object_free(err);
        return;
    } /* endif */

    switch (rwhp->state)
    {
        /**********************/
        /* await_instructions */
        /**********************/
        case await_instructions:
	    if (rwhp->instruction_buff[0] == FORMAT)
	    {
		/* 
		 * remote side called prime_the_line() connecting to me.
		 * set me their format.  i must send mine back.
		 */

		int dest_grank               = atoi((const char *) 
						 &(rwhp->instruction_buff[2]));
		globus_byte_t remote_format  = rwhp->instruction_buff[1];
		globus_byte_t my_format      = GLOBUS_DC_FORMAT_LOCAL;

		if (dest_grank < MPID_MyWorldRank 
		    || dest_grank > MPID_MyWorldSize)
		{
		    char err[1024];
		    globus_libc_sprintf(err,
			"ERROR: read_callback(): await_instructions FORMAT: "
			"proc %d extracted dest_grank %d which must be >= "
			"to my rank and < MPID_MyWorldSize %d",
			MPID_MyWorldRank, dest_grank, MPID_MyWorldSize);
		    MPID_Abort(NULL, 0, "MPICH-G2", err);
		}
		else if (!(CommworldChannels[dest_grank].selected_proto))
		{
		    globus_libc_fprintf(stderr,
			"ERROR: read_callback(): await_instructions FORMAT: "
			"proc %d: does not have selected proto for dest_grank "
			"%d\n",
			MPID_MyWorldRank, dest_grank); 
		    print_channels(MPID_MyWorldSize, CommworldChannels);
		    MPID_Abort(NULL, 0, "MPICH-G2", "");
		}
		else if (CommworldChannels[dest_grank].selected_proto->type 
			    != tcp)
		{
		    globus_libc_fprintf(stderr,
			"ERROR: read_callback(): await_instructions FORMAT: "
			"proc %d: called with selected protocol to dest_grank "
			"%d something other than TCP\n",
			MPID_MyWorldRank, dest_grank); 
		    print_channels(MPID_MyWorldSize, CommworldChannels);
		    MPID_Abort(NULL, 0, "MPICH-G2", "");
		}
		else
		{
		    struct tcp_miproto_t *tp = (struct tcp_miproto_t *) 
			    CommworldChannels[dest_grank].selected_proto->info;
		    globus_size_t nbytes_sent;

		    globus_mutex_lock(&(tp->connection_lock));
		    if (dest_grank == MPID_MyWorldRank)
		    {
			/* TCP connection to myself */

			/* in this case, assignment 
			 * tp->whandle = &(tp->to_self.handle) was done
			 * in prime_the_line.
			 */
			tp->handlep = rwhp;
		    }
		    else
		    {
			/* TCP connection to proc other than myself */

			if (tp->handlep)
			{
			    /* 
			     * already had a connection ... must use that one 
			     * and free the one allocated by listener_callback
			     * and passed to this function.
			     */
			    g_free((void *) rwhp);
			    rwhp = (struct tcp_rw_handle_t *) tp->handlep;
			} 
			else
			    tp->handlep = rwhp;

			tp->whandle = &(rwhp->handle);
		    } /* endif */
		    rwhp->remote_format = remote_format;
		    globus_mutex_unlock(&(tp->connection_lock));

		    /* sending my format back */

		    /* 
		     * in general, should use tp->whandle for all writes,
		     * but in this bootstrap situation we do not have
		     * access to tp->whandle when connecting back to
		     * ourselves, so will use rwhp->handle for this write only.
		     */
		    if (globus_io_write(&(rwhp->handle), 
					&(my_format), 
					globus_dc_sizeof_byte(1), 
					&nbytes_sent) != GLOBUS_SUCCESS)
		    {
			char err[1024];
			globus_libc_sprintf(
			    err,
			    "ERROR: read_callback(): await_instructions: "
			    "proc %d: write format failed",
			    MPID_MyWorldRank); 
			MPID_Abort(NULL, 0, "MPICH-G2", err);
		    } /* endif */

		    /* transition to await_header */
		    rwhp->state = await_header;
		    rwhp->incoming_header_len = 
			REMOTE_HEADER_LEN(rwhp->remote_format);
		    g_malloc(rwhp->incoming_header, 
			    globus_byte_t *, 
			    rwhp->incoming_header_len);
		    rwhp->libasize = 
			globus_dc_sizeof_remote_u_long(1, rwhp->remote_format);
		    g_malloc(rwhp->liba, void *, rwhp->libasize);
		    globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_header, /* data will sit */
					  /* here when callback is envoked */
				    rwhp->incoming_header_len, /* max nbytes */
				rwhp->incoming_header_len, /* wait for nbytes */
				    read_callback,
				    (void *) rwhp); /* optional callback arg */
		    /* 
		     * signalling my prime_the_line() just in case
		     * i started this whole thing by calling prime_the_line
		     * myself and was forced to instruct the other side
		     * to call his prime_the_line and wait.
		     */
		    globus_cond_signal(&(tp->connection_cond));

		} /* endif */
	    }
	    else if (rwhp->instruction_buff[0] == PRIME)
	    {
		/* 
		 * remote side called prime_the_line() but realizes
		 * that i must connect back to them, so they have
		 * instructed me to call prime_the_line().
		 */

		int dest_grank = atoi((const char *)
				    &(rwhp->instruction_buff[1]));
		if (dest_grank >= MPID_MyWorldRank)
		{
		    char err[1024];
		    globus_libc_sprintf(err,
			"ERROR: read_callback(): await_instructions CONNECT: "
			"proc %d extracted dest_grank %d which must be < "
			"my rank",
			MPID_MyWorldRank, dest_grank);
		    MPID_Abort(NULL, 0, "MPICH-G2", err);
		}
		else if (!(CommworldChannels[dest_grank].selected_proto))
		{
		    globus_libc_fprintf(stderr,
			"ERROR: read_callback(): await_instructions CONNECT: "
			"proc %d: does not have selected proto for dest_grank "
			"%d\n",
			MPID_MyWorldRank, dest_grank); 
		    print_channels(MPID_MyWorldSize, CommworldChannels);
		    MPID_Abort(NULL, 0, "MPICH-G2", "");
		}
		else if (CommworldChannels[dest_grank].selected_proto->type 
			    != tcp)
		{
		    globus_libc_fprintf(stderr,
			"ERROR: read_callback(): await_instructions CONNECT: "
			"proc %d: called with selected protocol to dest_grank "
			"%d something other than TCP\n",
			MPID_MyWorldRank, dest_grank); 
		    print_channels(MPID_MyWorldSize, CommworldChannels);
		    MPID_Abort(NULL, 0, "MPICH-G2", "");
		}
		else
		{
			/* don't need this handle anymore nor do i 
			 * need the tcp_rw_handle_t allocated in
			 * the listen_callback.  we only used that
			 * handle and struct to get this 'prime back
			 * to me' message.
			 */
			globus_io_close(&(rwhp->handle));
			g_free((void *) rwhp);

			prime_the_line((struct tcp_miproto_t *) 
			    CommworldChannels[dest_grank].selected_proto->info, 
			    dest_grank);
		} /* endif */
	    }
	    else 
	    {
		char err[1024];
		globus_libc_sprintf(err,
		    "ERROR: read_callback(): await_instructions: "
		    "proc %d: received unrecognizable instruction %c (%d)\n",
		    MPID_MyWorldRank, rwhp->instruction_buff[0], 
		    rwhp->instruction_buff[0]); 
		MPID_Abort(NULL, 0, "MPICH-G2", err);
	    } /* endif */
	    break;

        /****************/
        /* await_format */
        /****************/
        case await_format:
	    /* 
	     * signalling prime_the_line() that 
	     * remote format reply has arrived 
	     */
	    rwhp->recvd_format = GLOBUS_TRUE;
	    globus_cond_signal(&(rwhp->format_cond));

	    /* transition to await_header */
            rwhp->state = await_header;
            rwhp->incoming_header_len = REMOTE_HEADER_LEN(rwhp->remote_format);
            g_malloc(rwhp->incoming_header, 
		    globus_byte_t *, 
		    rwhp->incoming_header_len);
	    rwhp->libasize = 
		globus_dc_sizeof_remote_u_long(1, rwhp->remote_format);
	    g_malloc(rwhp->liba, void *, rwhp->libasize);
            globus_io_register_read(&(rwhp->handle),
                                    rwhp->incoming_header, /* data will sit */
                                          /* here when callback is envoked */
                                rwhp->incoming_header_len, /* max nbytes */
                                rwhp->incoming_header_len, /* wait for nbytes */
                                    read_callback,
                                    (void *) rwhp); /* optional callback arg */
            break;

        /****************/
        /* await_header */
        /****************/
        case await_header: /* header info */
            {
		int type;
                globus_byte_t *cp = rwhp->incoming_header;

		/* 
		 * unpacking header type: either user_data or ack
		 * type==user_data,src,tag,contextid,
		 *       dataoriginbuffsize,ssendflag,packed_flag,
		 *       msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr,liba
		 *       where (liba == remote addr and
		 *               ssendflag == -1 (no ack necessary)) 
		 *       or
		 *              (ssendflag = src_grank (ack necessary))
		 * OR 
		 * type==ack, liba
		 * OR 
		 * type==cancel_send,
		 *       msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr,
		 *       liba
		 * OR 
		 * type==cancel_result, cancel_success_flag, liba
		 */

                globus_dc_get_int(&cp, &type, 1, (int) (rwhp->remote_format));
		if (type == user_data)
		{
		    /* header for user data ... prepare for incoming payload */

		    /*
		     * unpacking rest of header: 
		     *      src,tag,contextid,
		     *      dataoriginbuffsize,ssendflag,packed_flag,
		     *      msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr,liba
		     */
		    globus_dc_get_int(&cp, &(rwhp->src), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->tag), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->context_id), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->dataorigin_bufflen), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->ssend_flag), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->packed_flag), 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(rwhp->msg_id_src_grank), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(rwhp->msg_id_sec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(rwhp->msg_id_usec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_u_long(&cp, &(rwhp->msg_id_ctr), 1,
					(int) (rwhp->remote_format));
		    memcpy(rwhp->liba, (void *) cp, rwhp->libasize);

		    /*
		     * NICK: for now unconditionally cache the message ... 
		     *       later detect if there is already is a recv 
		     *       pending and read it directly into user's memory.
		     */

		    if (rwhp->dataorigin_bufflen)
		    {
			g_malloc(rwhp->incoming_raw_data, 
				    globus_byte_t *, 
				    rwhp->dataorigin_bufflen);

			rwhp->state = await_data;
			globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_raw_data, /* data will sit */
					  /* here when callback is envoked */
				rwhp->dataorigin_bufflen, /* max nbytes */
				rwhp->dataorigin_bufflen, /* wait for nbytes */
						read_callback,
				    (void *) rwhp); /* optional callback arg */
		    }
		    else
		    {
			/* empty payload */
			rwhp->incoming_raw_data = (globus_byte_t *) NULL;
			data_arrived(rwhp);

			/* transition to 'await_header' state */
			rwhp->state = await_header;
			globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_header, /* data will sit */
					  /* here when callback is envoked */
				    rwhp->incoming_header_len, /* max nbytes */
				rwhp->incoming_header_len, /* wait for nbytes */
						read_callback,
				    (void *) rwhp); /* optional callback arg */
		    } /* endif */
		}
		else if (type == ack)
		{
		    /* header for ack ... signal waiting ssend */
		    MPIR_SHANDLE *sreq;

		    /* unpacking rest of header: liba */
		    memcpy(&sreq, (void *) cp, sizeof(MPIR_SHANDLE *));

		    if (!sreq)
		    {
			MPID_Abort(NULL, 0, "MPICH-G2",
			    "ERROR: read_callback(): await_header type=ack: "
			    "extracted NULL sreq");
		    } /* endif */

		    globus_mutex_lock(&(sreq->lock));
		    sreq->ack_arrived = GLOBUS_TRUE;
		    if (sreq->cancel_issued)
			sreq->is_complete = sreq->cancel_complete;
		    else
			sreq->is_complete = sreq->data_sent;
		    if (sreq->is_complete &&
			((MPI_Request) sreq)->chandle.ref_count <= 0)
		    {
			MPID_SendFree(sreq);
		    } /* endif */
		    globus_mutex_unlock(&(sreq->lock));

		    globus_mutex_lock(&MessageQueuesLock);
		    TcpOutstandingRecvReqs --;
		    globus_mutex_unlock(&MessageQueuesLock);

		    /* transition back to same state, 'await_header' */
		    rwhp->state = await_header;
		    globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_header, /* data will sit */
					  /* here when callback is envoked */
				rwhp->incoming_header_len, /* max nbytes */
				rwhp->incoming_header_len, /* wait for nbytes */
					    read_callback,
				    (void *) rwhp); /* optional callback arg */
		}
		else if (type == cancel_send)
		{
		    /* rcv side just received a request to cancel */
		    /* a previously sent message */
		    int result;
		    int msgid_src_grank;
		    long msgid_sec;
		    long msgid_usec;
		    unsigned long msgid_ctr;
		    MPIR_RHANDLE *rhandle;

		    /* unpacking rest of header: 
		     *     msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr, 
		     *     liba 
		     */
		    globus_dc_get_int(&cp, &(msgid_src_grank), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(msgid_sec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(msgid_usec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_u_long(&cp, &(msgid_ctr), 1,
					(int) (rwhp->remote_format));
		    memcpy(rwhp->liba, (void *) cp, rwhp->libasize);

		    /* search 'unexpected' queue for message */
		    /* if found, then found=TRUE and removed */
		    /* from 'unexpected' queue, else found=FALSE */
		    globus_mutex_lock(&MessageQueuesLock);
		    {
			MPID_QUEUE *queue = &MPID_recvs.unexpected;
			MPID_QEL   **pp = &(queue->first);
			MPID_QEL   *p = *pp;
			globus_bool_t found = GLOBUS_FALSE;

			while (!found && p)
			{
			    rhandle = p->ptr;
			    /* trying to order so most likely to fail first */
			    if (!(found = (rhandle->msg_id_ctr == msgid_ctr 
				&& rhandle->msg_id_usec == msgid_usec
				&& rhandle->msg_id_sec == msgid_sec
			    && rhandle->msg_id_src_grank == msgid_src_grank)))
			    {
				pp = &p->next;
				p = *pp;
			    } /* endif */
			} /* endwhile */

			if (found)
			{
			    result = 1;

			    if (MPID_Dequeue(&MPID_recvs.unexpected, rhandle))
			    {
				globus_libc_fprintf(stderr,
				    "ERROR: read_callback(): await_header: "
				    "cancel_send: proc %d: failed to dequeue "
				    "message from unexpected queue\n",
				MPID_MyWorldRank); 
			    } /* endif */
			}
			else
			    result = 0;
		    }
		    globus_mutex_unlock(&MessageQueuesLock);

		    if (result 
			&& ((MPI_Request) rhandle)->chandle.ref_count <= 0)
		    {
			MPID_RecvFree(rhandle);
		    } /* endif */

		    send_cancel_result_over_tcp(msgid_src_grank,
						result,
						rwhp->liba,
						rwhp->libasize,
						msgid_sec,
						msgid_usec,
						msgid_ctr);

		    /* transition back to same state, 'await_header' */
		    rwhp->state = await_header;
		    globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_header, /* data will sit */
					  /* here when callback is envoked */
				rwhp->incoming_header_len, /* max nbytes */
				rwhp->incoming_header_len, /* wait for nbytes */
					    read_callback,
				    (void *) rwhp); /* optional callback arg */
		}
		else if (type == cancel_result)
		{
		    /* send side just received a result from cancel request */
		    MPIR_SHANDLE *sreq;
		    int found;
		    int msgid_src_grank;
		    long msgid_sec;
		    long msgid_usec;
		    unsigned long msgid_ctr;

		    /* 
		     * unpacking rest of header: found, 
		     *          msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr,
		     *                           liba 
		     */
		    globus_dc_get_int(&cp, &found, 1, 
					(int) (rwhp->remote_format));
		    globus_dc_get_int(&cp, &(msgid_src_grank), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(msgid_sec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_long(&cp, &(msgid_usec), 1,
					(int) (rwhp->remote_format));
		    globus_dc_get_u_long(&cp, &(msgid_ctr), 1,
					(int) (rwhp->remote_format));
		    memcpy(&sreq, (void *) cp, sizeof(MPIR_SHANDLE *));

		    if (!sreq)
		    {
			MPID_Abort(NULL, 0, "MPICH-G2",
			    "ERROR: read_callback(): await_header "
			    "type=cancel_result: extracted NULL sreq");
		    } /* endif */

		    RC_mutex_lock(&(sreq->lock));
		    if (msgid_src_grank == MPID_MyWorldRank
			&& msgid_sec    == sreq->msg_id_sec
			&& msgid_usec   == sreq->msg_id_usec
			&& msgid_ctr    == sreq->msg_id_ctr)
		    {
			/* 
			 * result for this particular request ... otherwise
			 * a result for a previous request that was discarded
			 * before waiting for result.  in that case we simply
			 * throw this result away.  however, the liba from
			 * this message DOES match our sreq, so we update
			 * the cancel request result.
			 */
			sreq->cancel_complete = sreq->is_complete = GLOBUS_TRUE;
			if (sreq->is_cancelled = found)
			    sreq->s.MPI_TAG = MPIR_MSG_CANCELLED;
		    } /* endif */
		    RC_mutex_unlock(&(sreq->lock));

		    globus_mutex_lock(&MessageQueuesLock);
		    TcpOutstandingRecvReqs --;
		    globus_mutex_unlock(&MessageQueuesLock);

		    /* transition back to same state, 'await_header' */
		    rwhp->state = await_header;
		    globus_io_register_read(&(rwhp->handle),
				    rwhp->incoming_header, /* data will sit */
					  /* here when callback is envoked */
				rwhp->incoming_header_len, /* max nbytes */
				rwhp->incoming_header_len, /* wait for nbytes */
					    read_callback,
				    (void *) rwhp); /* optional callback arg */
		}
		else
		{
		    char err[1024];
		    globus_libc_sprintf(err,
			"ERROR: read_callback(): await_header: "
			"unrecognizable header type %d\n",
			type);
		    MPID_Abort(NULL, 0, "MPICH-G2", err);
		} /* endif */
	    }
            break;

        /**************/
        /* await_data */
        /**************/
        case await_data: /* data */
            {
                /* NICK: again, under the assumption that we unconditionally */
                /*       cache incoming data ... later we will optimize this */
                /*       to deal with the case that the data might have been */
                /*       read directly into the user's memory                */

		data_arrived(rwhp);

		/* transition to 'await_header' state */
                rwhp->state = await_header;
                globus_io_register_read(&(rwhp->handle),
                                    rwhp->incoming_header, /* data will sit */
                                          /* here when callback is envoked */
                                rwhp->incoming_header_len, /* max nbytes */
                                rwhp->incoming_header_len, /* wait for nbytes */
                                        read_callback,
                                    (void *) rwhp); /* optional callback arg */
                }
            break;

        default:
	    {
		char err[1024];
		globus_libc_sprintf(err,
		    "ERROR: read_callback(): unrecognizable state %d\n", 
		    rwhp->state);
		MPID_Abort(NULL, 0, "MPICH-G2", err);
	    }
            break;
    } /* end switch() */
/* globus_libc_printf("%d: exit read_callback: state=", MPID_MyWorldRank); switch (rwhp->state) { case await_instructions: globus_libc_printf("await_instructions\n"); break; case await_format: globus_libc_printf("await_format\n"); break; case await_header: globus_libc_printf("await_header\n"); break; case await_data: globus_libc_printf("await_data\n"); break; default: globus_libc_printf("unknown\n"); break; } fflush(stdout); */

}  /* end read_callback() */

static void data_arrived(struct tcp_rw_handle_t *rwhp)
{
    MPIR_RHANDLE *rhandle;
    int found;
    int rc;

    /* check 'posted' queue for posted request */
    /* if found in posted queue, then remove into var rhandle */
    /* if not found in posted queue, then alloc a req into   */
    /* var rhandle and place onto unexpected queue            */
    globus_mutex_lock(&MessageQueuesLock);
    MPID_Msg_arrived(rwhp->src,
		    rwhp->tag,
		    rwhp->context_id,
		    &rhandle,
		    &found);
    if (!found) 
    {
	rhandle->buf      = rwhp->incoming_raw_data;
	if (sizeof(unsigned long) < rwhp->libasize)
	{
	    char err[1024];
	    globus_libc_sprintf(err,
		"ERROR: read_callback(): await_data: detected sizeof("
		"unsigned long) %ld < size of incoming liba %d\n", 
		sizeof(unsigned long), 
		rwhp->libasize);
	    MPID_Abort(NULL, 0, "MPICH-G2", err);
	} /* endif */
	memcpy((void*) (&(rhandle->liba)), 
	    rwhp->liba, 
	    rwhp->libasize);
	rhandle->libasize = rwhp->libasize;

	/* copying msg id stuff */
	rhandle->msg_id_src_grank = rwhp->msg_id_src_grank;
	rhandle->msg_id_sec       = rwhp->msg_id_sec;
	rhandle->msg_id_usec      = rwhp->msg_id_usec;
	rhandle->msg_id_ctr       = rwhp->msg_id_ctr;
    } /* endif */
    rhandle->src_format   = rwhp->remote_format;
    rhandle->packed_flag  = rwhp->packed_flag;
    rhandle->s.count      = 
    rhandle->len          = rwhp->dataorigin_bufflen;
    STATUS_INFO_SET_COUNT_LOCAL(rhandle->s);
    rhandle->s.MPI_ERROR  = MPI_SUCCESS;
    rhandle->partner = rwhp->ssend_flag;

    if (found)
    {
	extern volatile int		TcpOutstandingRecvReqs;
	
	/* recv had already been posted ... */
	TcpOutstandingRecvReqs --;

#       if defined(VMPI)
        {
	    if (rhandle->req_src_proto == unknown)
	    {
		/* 
		 * this req has also been posted to the MpiPostedQueue 
		 * and must be removed from there also
		 */
		if (rhandle->my_mp)
		{
		    remove_and_free_mpircvreq(rhandle->my_mp);
		    rhandle->my_mp = (struct mpircvreq *) NULL;
		}
		else
		{
		    /*
		     * NICK: in single-threaded case i'm pretty sure
		     *       that this is a fatal error, but for now
		     *       simply printing warning and continuing.
		     */
		    globus_libc_fprintf(stderr,
			"WARNING: data_arrived: detected incoming "
			"message from unknown recv source over TCP but "
			"did NOT find request in MPI queue\n");
		} /* endif */
	    } /* endif */
	}
#       endif

	if (rwhp->ssend_flag != -1)
	    send_ack_over_tcp(rwhp->ssend_flag,
				rwhp->liba,
				rwhp->libasize);

	globus_mutex_lock(&(rhandle->lock));
	
	{
	    unsigned char *		buf;
	    int				len;
	    int				format;
	    
	    buf = rwhp->incoming_raw_data;
	    len = rwhp->dataorigin_bufflen;
	    format = rwhp->remote_format;
	    
	    if (rwhp->packed_flag &&
		rhandle->datatype->dte_type != MPIR_PACKED)
	    {
		format = buf[0];
		buf++;
		len--;
	    }
	    else if (rhandle->datatype->dte_type == MPIR_PACKED &&
		     !rwhp->packed_flag)
	    {
		g_malloc(buf, globus_byte_t *, len + 1);
		buf[0] = format;
		memcpy(buf + 1, rwhp->incoming_raw_data, len);
	    }
		
	    rc = extract_data_into_req(rhandle,
				       buf,
				       len,
				       format,
				       rwhp->src,
				       rwhp->tag);
	    
	    if (rhandle->datatype->dte_type == MPIR_PACKED &&
		!rwhp->packed_flag)
	    {
		g_free(buf);
	    }
	}
	
	if (rc)
	{
	    rhandle->s.MPI_ERROR = MPI_ERR_INTERN;
	}
	    
#       if defined(VMPI)
        {
	    if (rhandle->req_src_proto == unknown)
	    {
		MPI_Comm c = rhandle->comm->self;
		MPI_Comm_free(&c);
	    } /* endif */
	}
#       endif
	MPIR_Type_free(&(rhandle->datatype));
	rhandle->is_complete = GLOBUS_TRUE;
	if (((MPI_Request) rhandle)->chandle.ref_count <= 0)
	{
	    MPID_RecvFree(rhandle);
	}
	globus_mutex_unlock(&(rhandle->lock));

	g_free(rwhp->incoming_raw_data);
    } /* endif */
    globus_mutex_unlock(&MessageQueuesLock);

} /* end data_arrived() */

/* 
 * NICK THREAD: i don't it's possible for multiple threads
 * to be in this function simultaneously because we register
 * a callback at the end of this function.  we do need to know
 * that globus_io_{tcp_accept, register_read, register_listen}
 * are all thread-safe because _another_ handler might call
 * them while we're calling them.  the same is true for 
 * getsockopt().
 * note - globus_io_tcp_accept called only in this callback
 *      - globus_io_tcp_register_listen not possible to be
 *        called by two different threads?
 *      - getsockopt called in prime_the_line
 */
/* called when a client does a 'connect' to me */
void listen_callback(void *callback_arg,         /* unused */
                    globus_io_handle_t *handle, 
                    globus_result_t result)
{
    struct tcp_rw_handle_t *rwhp;

    if (result != GLOBUS_SUCCESS)
    {
        /* things are very bad */
        MPID_Abort(NULL, 0, "MPICH-G2",
	    "ERROR: listen_callback rcvd result != GLOBUS_SUCCESS");
    } /* endif */

    g_malloc(rwhp, struct tcp_rw_handle_t *, sizeof(struct tcp_rw_handle_t));
    /* intializing state as 'waiting for instructions' */
    rwhp->state = await_instructions;

    /* 'accept' connection which creates a new socket */
    /* and (therefore) new handle */
    globus_io_tcp_accept(handle,  /* should be handle passed to this callback */
                        (globus_io_attr_t *) GLOBUS_NULL, /* use atr passed */
                                        /* to globus_io_tcp_create_listener() */
                        &(rwhp->handle)); /* handle for new socket */
                                        /* created as a result of this accept */

    if (MpichGlobus2TcpBufsz > 0)
    {
	int size;
	int sizeof_size = sizeof(size);
	int rc;
	
	rc = getsockopt((rwhp->handle).fd,
			SOL_SOCKET,
			SO_RCVBUF,
			(char *) &size,
			&sizeof_size);
	if (rc != 0)
	{
	    char err[1024];
	    globus_libc_sprintf(err, 
		"ERROR: listen_callback(): received erroneous rc %d "
		"from getsockopt", rc);
	    MPID_Abort(NULL, 0, "MPICH-G2", err);
	} /* endif */

	if (size < (MpichGlobus2TcpBufsz - MpichGlobus2TcpBufsz / 10))
	{
	    globus_libc_fprintf(
		stderr,
		"%04d-WARNING: RCVBUF (%d) less than requested size (%d)\n",
		MPID_MyWorldRank,
		size,
		MpichGlobus2TcpBufsz);
	} /* endif */
    } /* endif */
	
    globus_io_register_read(&(rwhp->handle),
			rwhp->instruction_buff, /* data will */
                                       /* sit here when callback is envoked */
			sizeof(rwhp->instruction_buff), /* max nbytes */
			sizeof(rwhp->instruction_buff), /* wait for nbytes */
			read_callback,
			(void *) rwhp);             /* optional callback arg */

    /* registering another listen for next guy that wants to 'connect' to me */
    /* 
     * NICK THREAD: i think using Handle here is OK because until 
     * we register another listen, it's not possible for another 
     * thread to be in this function.
     */
    globus_io_tcp_register_listen(&Handle,
                                listen_callback,
                                (void *) NULL); /* optional user arg */
                                                /* to be passed to callback */

} /* end listen_callback() */

void prime_the_line(struct tcp_miproto_t *tp, int dest_grank)
{
    /* done first time only, connect and start TCP state machine */

    if (!(CommworldChannels[dest_grank].selected_proto))
    {
	globus_libc_fprintf(stderr,
            "ERROR: prime_the_line: proc %d does not have selected proto for"
            " dest_grank %d\n",
            MPID_MyWorldRank, dest_grank); 
        print_channels(MPID_MyWorldSize, CommworldChannels);
	MPID_Abort(NULL, 0, "MPICH-G2", "");
    }
    else if (CommworldChannels[dest_grank].selected_proto->type != tcp)
    {
        globus_libc_fprintf(stderr,
            "ERROR: prime_the_line: proc %d called with selected protocol to"
            " dest_grank %d something other than TCP\n",
            MPID_MyWorldRank, dest_grank); 
        print_channels(MPID_MyWorldSize, CommworldChannels);
	MPID_Abort(NULL, 0, "MPICH-G2", "");
    }
    else if (CommworldChannels[dest_grank].selected_proto->info != tp)
    {
        globus_libc_fprintf(
	    stderr,
            "ERROR: prime_the_line: proc %d encountered mismatch between"
            " info %lx and passed tp %lx ... they should be equal\n",
            MPID_MyWorldRank,
	    (unsigned long)
	    CommworldChannels[dest_grank].selected_proto->info, 
	    (unsigned long) tp); 
        print_channels(MPID_MyWorldSize, CommworldChannels);
	MPID_Abort(NULL, 0, "MPICH-G2", "");
    }
    else
    {
	globus_mutex_lock(&(tp->connection_lock));
	if (!(tp->handlep))
	{
	    /* connection has not been established yet */
	    struct tcp_rw_handle_t temp_rw;
	    struct tcp_rw_handle_t *rwp;
	    globus_size_t          nbytes_sent;

	    globus_io_tcpattr_init(&(tp->attr));
	    
	    /*
	     * We need to set the tcp send and receive buffer sizes to
	     * something large to deal with the large bandwidth - delay product
	     * associated with today's WAN.
	     */
	    if (MpichGlobus2TcpBufsz > 0)
	    {
		globus_io_attr_set_socket_sndbuf(&(tp->attr), 
						MpichGlobus2TcpBufsz);
		globus_io_attr_set_socket_rcvbuf(&(tp->attr), 
						MpichGlobus2TcpBufsz);
	    } /* endif */

	    /*
	     * Don't delay small messages; avoiding the extra latency 
	     * incurred by this delay is probably far more important 
	     * than saving the little bit of bandwidth eaten by an extra 
	     * TCP/IP header
	     */
	    globus_io_attr_set_tcp_nodelay(&(tp->attr), GLOBUS_TRUE);
	    
	    if (MPID_MyWorldRank >= dest_grank)
	    {
		/* establish the permanent socket */

		if (MPID_MyWorldRank == dest_grank)
		{
		    /* 
		     * special case, TCP connect to myself 
		     * need to use handle in 'to_self' field in tp 
		     * for writing and handle in tp->handlep (handlep
		     * malloc'd in listen_callback) for reading.
		     * assignment tp->rhanlde = &(handlep->handle)
		     * is done in read_callback:state==await_instructions:
		     * instruction==FORMAT.
		     */
		     rwp         = &(tp->to_self);
		     tp->whandle = &(tp->to_self.handle);
		}
		else
		{
		    /* general case, TCP connect to proc other than myself */
		    g_malloc(tp->handlep, 
			    struct tcp_rw_handle_t *, 
			    sizeof(struct tcp_rw_handle_t));
		    rwp = (struct tcp_rw_handle_t *) tp->handlep;
		    tp->whandle = &(rwp->handle);
		} /* endif */
	    }
	    else
		rwp = &temp_rw;

	    globus_cond_init(&(rwp->format_cond), 
			    (globus_condattr_t *) GLOBUS_NULL);
	    if (globus_io_tcp_connect(tp->hostname, 
				    tp->port, 
				    &(tp->attr), 
				    &(rwp->handle)) 
		!= GLOBUS_SUCCESS)
	    {
		MPID_Abort(NULL, 0, "MPICH-G2",
		    "ERROR: prime_the_line: connect failed");
	    } /* endif */
	    globus_io_tcpattr_destroy(&(tp->attr));

	    if (MpichGlobus2TcpBufsz > 0)
	    {
		int				size;
		int				sizeof_size = sizeof(size);
		int				rc;
		    
		rc = getsockopt((rwp->handle).fd,
				SOL_SOCKET,
				SO_SNDBUF,
				(char *) &size,
				&sizeof_size);
		if (rc != 0)
		{
		    char err[1024];
		    globus_libc_sprintf(err, 
			"ERROR: prime_the_line(): received erroneous rc %d "
			"from getsockopt", rc);
		    MPID_Abort(NULL, 0, "MPICH-G2", err);
		} /* endif */

		if (size < 
		    (MpichGlobus2TcpBufsz - MpichGlobus2TcpBufsz/10))
		{
		    globus_libc_fprintf(
			stderr,
			"%04d-WARNING: SNDBUF (%d) less than requested"
			" size (%d)\n",
			MPID_MyWorldRank,
			size,
			MpichGlobus2TcpBufsz);
		} /* endif */
	    } /* endif */
		    
	    if (MPID_MyWorldRank >= dest_grank)
	    {
		/* sending my format once */
		sprintf((char *) rwp->instruction_buff, 
		    "%c%c%d", 
		    FORMAT, 
		    GLOBUS_DC_FORMAT_LOCAL, 
		    MPID_MyWorldRank);
	    }
	    else
	    {
		/* telling other side to prime_the_line() back to me */
		sprintf((char *) rwp->instruction_buff, 
			"%c%d", 
			PRIME, 
			MPID_MyWorldRank);
	    } /* endif */

	    /* 
	     * generally we would use tp->whandle for all writes, but
	     * in this special bootstrapping situation we use rwp->handle
	     * to avoid having to incorrectly set 
	     * tp->whandle = &(temp_rw.handle) in the case where we're 
	     * just sending a PRIME message to the other side.
	     */
	    if (globus_io_write(&(rwp->handle), 
				rwp->instruction_buff, 
				sizeof(rwp->instruction_buff), 
				&nbytes_sent) != GLOBUS_SUCCESS)
	    {
		MPID_Abort(NULL, 0, "MPICH-G2",
		    "ERROR: prime_the_line: write format failed"); 
	    } /* endif */

	    if (MPID_MyWorldRank >= dest_grank)
	    {
		/* wait for other side's format */
		rwp->recvd_format = GLOBUS_FALSE;
		rwp->state        = await_format;
		/* 
		 * generally we would use tp->rhandle for all reads, but
		 * in this special bootstrapping situation we use rwp->handle
		 * to accomodate the situation when we're connecting
		 * to ourselves.
		 */
		globus_io_register_read(&(rwp->handle),
                            &(rwp->remote_format), /* data will */
                                       /* sit here when callback is envoked */
                            sizeof(rwp->remote_format), /* max nbytes */
                            sizeof(rwp->remote_format), /* wait for nbytes */
		    read_callback,
		    (void *) rwp);                /* optional callback arg */

		while (!(rwp->recvd_format))
		{
		    globus_cond_wait(&(rwp->connection_cond),
					    &(tp->connection_lock));
		} /* endwhile */
	    }
	    else
	    {
		/* tell the other side to establish the permanent socket */
		if (globus_io_close(&(rwp->handle)) != GLOBUS_SUCCESS)
		{
		    globus_libc_fprintf(
			stderr,
			"WARNING: prime_the_line: globus_io_close() failed\n"); 
		} /* endif */

		/* waiting for other side to call prime_the_line() */
		while (!(tp->handlep))
		{
		    globus_cond_wait(&(tp->connection_cond),
					&(tp->connection_lock));
		} /* endwhile */
	    } /* endif */

	} /* endif */
	globus_mutex_unlock(&(tp->connection_lock));
    } /* endif */

} /* end prime_the_line() */

/*
 * send_cancel_result_over_tcp
 *
 * assumed that messaging to grank is known to be TCP
 */

static void send_cancel_result_over_tcp(int grank, 
					int result, 
					void *liba,
					int libasize,
					long msgid_sec,
					long msgid_usec,
					unsigned long msgid_ctr)
{
    globus_size_t nbytes_sent;

    if (!(CommworldChannels[grank].selected_proto))
    {
        globus_libc_fprintf(stderr,
            "ERROR: send_cancel_result_over_tcp: proc %d does not have "
	    "selected proto for grank %d\n",
            MPID_MyWorldRank, grank); 
        print_channels(MPID_MyWorldSize, CommworldChannels);
	MPID_Abort(NULL, 0, "MPICH-G2", "");
    } 
    else if (CommworldChannels[grank].selected_proto->type == tcp)
    {
            struct tcp_miproto_t *tp = (struct tcp_miproto_t *) 
                CommworldChannels[grank].selected_proto->info;
            globus_byte_t *cp     = tp->header;
            enum header_type type = cancel_result;
	    struct tcpsendreq * sr;

            if (!(tp->handlep))
            {
                globus_libc_fprintf(stderr,
                    "ERROR: send_cancel_result_over_tcp: proc %d found NULL "
		    "handlep for grank %d\n",
                    MPID_MyWorldRank, grank);
                print_channels(MPID_MyWorldSize, CommworldChannels);
		MPID_Abort(NULL, 0, "MPICH-G2", "");
            } /* endif */

            /* 
	     * packing header: type=cancel_result,result,
	     *                  msgid_src_grank,msgid_sec,msgid_usec,msgid_ctr,
	     *                  liba 
	     */
            if (Headerlen-(globus_dc_sizeof_int(2)
			    +globus_dc_sizeof_long(2)
			    +globus_dc_sizeof_u_long(1)) < libasize)
            {
		char err[1024];

                globus_libc_sprintf(err,
                    "ERROR: send_cancel_result_over_tcp: deteremined that "
		    "Headerlen (%ld) - (2*sizeof(int) (%ld)"
		    "+2*sizeof(long) (%ld)+sizeof(ulong) (%ld))"
		    "< waiter for ack's "
		    "libasize %d and will therefore not fit into header\n", 
                    Headerlen, 
                    globus_dc_sizeof_int(2), 
                    globus_dc_sizeof_long(2), 
                    globus_dc_sizeof_u_long(1), 
                    libasize);
		MPID_Abort(NULL, 0, "MPICH-G2", err);
            } /* endif */

	    g_malloc(sr, struct tcpsendreq *, sizeof(struct tcpsendreq));
	    g_malloc(sr->liba, void *, libasize);
	    sr->type          = cancel_result;
	    sr->result        = result;
	    sr->dest_grank    = grank;
	    sr->msgid_sec     = msgid_sec;
	    sr->msgid_usec    = msgid_usec;
	    sr->msgid_ctr     = msgid_ctr;
	    sr->libasize      = libasize;
	    memcpy(sr->liba, liba, libasize);

	    enqueue_tcp_send(sr);
    }
    else
    {
        globus_libc_fprintf(stderr,
            "ERROR: send_cancel_result_over_tcp: proc %d called with "
	    "selected protocol to grank %d something other than TCP\n",
            MPID_MyWorldRank, grank); 
        print_channels(MPID_MyWorldSize, CommworldChannels);
	MPID_Abort(NULL, 0, "MPICH-G2", "");
    } /* endif */

} /* end send_cancel_result_over_tcp() */
