#include "p4.h"
#include "p4_sys.h"

#ifdef CAN_DO_XDR
int xdr_send(type, from, to, msg, len, data_type, ack_req)
char *msg;
int type, from, to, len, data_type, ack_req;
{
    int done = 0;
    int nbytes_written = 0;
    int flag, sent, fd, myid, rc, n, nfds;
    struct p4_net_msg_hdr nmsg;
    XDR *xdr_enc;
    xdrproc_t xdr_proc;
    char *xdr_buff;
    int xdr_elsize, els_per_buf, xdr_numels;
    int xdr_len, xdr_len1, len_bytes;

    p4_dprintfl(20, "sending msg of type %d from %d to %d via xdr\n",
		type,from,to);

    myid = p4_get_my_id();
    fd = p4_local->conntab[to].port;

    nmsg.msg_type = p4_i_to_n(type);
    nmsg.to = p4_i_to_n(to);
    nmsg.from = p4_i_to_n(from);
    switch (data_type)
    {
      case P4INT:
	xdr_proc = xdr_int;
	xdr_elsize = XDR_INT_LEN;
	break;
      case P4LNG:
	xdr_proc = xdr_long;
	xdr_elsize = XDR_LNG_LEN;
	break;
      case P4FLT:
	xdr_proc = xdr_float;
	xdr_elsize = XDR_FLT_LEN;
	break;
      case P4DBL:
	xdr_proc = xdr_double;
	xdr_elsize = XDR_DBL_LEN;
	break;
      default:
	p4_dprintf("xdr_send: invalid data type %d\n", data_type);
	return (-1);
    }
    xdr_numels = len / xdr_elsize;
    nmsg.msg_len = p4_i_to_n(xdr_numels);
    nmsg.ack_req = p4_i_to_n(ack_req);
    nmsg.data_type = p4_i_to_n(data_type);

    flag = (myid < to) ? TRUE : FALSE;
    net_send(fd, &nmsg, sizeof(struct p4_net_msg_hdr), flag);

    xdr_enc = &(p4_local->xdr_enc);
    xdr_buff = p4_local->xdr_buff;
    els_per_buf = (XDR_BUFF_LEN - XDR_PAD) / xdr_elsize;
    while (xdr_numels > 0)
    {
	if (xdr_numels > els_per_buf)
	    xdr_len = els_per_buf;
	else
	    xdr_len = xdr_numels;
	xdr_len1 = xdr_len;	/* remember xdr_len */
	if (!xdr_setpos(xdr_enc, 0))
	{
	    p4_dprintf("xdr_send: xdr_setpos failed\n");
	    return (-1);
	}
	if (!xdr_array(xdr_enc, &msg, &xdr_len, XDR_BUFF_LEN,
		       xdr_elsize, xdr_proc))
	{
	    p4_dprintf("xdr_send: xdr_array failed\n");
	    return (-1);
	}
	len_bytes = xdr_getpos(xdr_enc);

	net_send(fd, xdr_buff, len_bytes, flag);

	nbytes_written += len_bytes;
	xdr_numels -= xdr_len1;
	msg = msg + len_bytes - XDR_PAD;
    }

    if (ack_req & P4_ACK_REQ_MASK)
    {
	wait_for_ack(fd);
    }
    p4_dprintfl(10, "sent msg of type %d from %d to %d via xdr\n",type,from,to);
    return (nbytes_written);
}
#endif

int socket_send(type, from, to, msg, len, data_type, ack_req)
int type, from, to, len, data_type, ack_req;
char *msg;
{
    int fd, flag;
    int sent = 0;
    int done = 0;
    int nleft, rc, n, nfds;
    struct p4_net_msg_hdr nmsg;

    p4_dprintfl(20, "sending msg of type %d from %d to %d via socket\n",type,from,to);

    if (CHECKNODE(to) || CHECKNODE(from))
	p4_error("socket_send: bad header: to/from node is out of range",
		 to * 10000 + from);

    fd = p4_local->conntab[to].port;

    nmsg.msg_type = p4_i_to_n(type);
    nmsg.to = p4_i_to_n(to);
    nmsg.from = p4_i_to_n(from);
    nmsg.msg_len = p4_i_to_n(len);
    nmsg.ack_req = p4_i_to_n(ack_req);
    nmsg.data_type = p4_i_to_n(data_type);

    flag = (from < to) ? TRUE : FALSE;
    net_send(fd, &nmsg, sizeof(struct p4_net_msg_hdr), flag);
    p4_dprintfl(20, "sent hdr for type %d from %d to %d via socket\n",type,from,to);

    while (sent < len)
    {
	if ((len - sent) > SOCK_BUFF_SIZE)
	    nleft = SOCK_BUFF_SIZE;
	else
	    nleft = len - sent;
	n = net_send(fd, ((char *) msg) + sent, nleft, flag);
	sent += n;
    }

    if (ack_req & P4_ACK_REQ_MASK)
    {
	wait_for_ack(fd);
    }

    p4_dprintfl(10, "sent msg of type %d from %d to %d via socket %d\n",type,from,to,fd);
    return (sent);
}


struct p4_msg *socket_recv()
{
    int i, fd, nfds;
    struct p4_msg *tmsg = NULL;
    P4BOOL found = FALSE;
    struct timeval tv;
    fd_set read_fds;

    while (!found)
    {
	tv.tv_sec = 9;
	tv.tv_usec = 0;  /* RMB */
	FD_ZERO(&read_fds);
	for (i = 0; !tmsg && i < p4_global->num_in_proctable; i++)
	{
	    if (p4_local->conntab[i].type == CONN_REMOTE_EST)
	    {
		fd = p4_local->conntab[i].port;
		FD_SET(fd, &read_fds);
	    }
	}
	SYSCALL_P4(nfds, select(p4_global->max_connections, &read_fds, 0, 0, &tv));
	if (nfds)
	{
	    for (i = 0; !tmsg && i < p4_global->num_in_proctable; i++)
	    {
		if (p4_local->conntab[i].type == CONN_REMOTE_EST)
		{
		    fd = p4_local->conntab[i].port;
		    if (FD_ISSET(fd,&read_fds)  &&  sock_msg_avail_on_fd(fd))
		    {
			tmsg = socket_recv_on_fd(fd);
			found = TRUE;
			if (tmsg->ack_req & P4_ACK_REQ_MASK)
			{
			    send_ack(fd, tmsg->from);
			}
		    }
		}
	    }
	}
    }
    return (tmsg);
}

struct p4_msg *socket_recv_on_fd(fd)
int fd;
{
    int n, data_type, msg_len;
    struct p4_msg *tmsg;
    struct p4_net_msg_hdr nmsg;

    n = net_recv(fd, &nmsg, sizeof(struct p4_net_msg_hdr));

    data_type = p4_n_to_i(nmsg.data_type);
    if (data_type == P4NOX)
	msg_len = p4_n_to_i(nmsg.msg_len);
    else
    {
	switch (data_type)
	{
	  case P4INT:
	    msg_len = p4_n_to_i(nmsg.msg_len) * XDR_INT_LEN;
	    break;
	  case P4LNG:
	    msg_len = p4_n_to_i(nmsg.msg_len) * XDR_LNG_LEN;
	    break;
	  case P4FLT:
	    msg_len = p4_n_to_i(nmsg.msg_len) * XDR_FLT_LEN;
	    break;
	  case P4DBL:
	    msg_len = p4_n_to_i(nmsg.msg_len) * XDR_DBL_LEN;
	    break;
	  default:
	    p4_error("socket_recv_on_fd: invalid data type %d\n", data_type);
	}
    }

    tmsg = alloc_p4_msg(msg_len);
    tmsg->type = p4_n_to_i(nmsg.msg_type);
    tmsg->from = p4_n_to_i(nmsg.from);
    tmsg->to = p4_n_to_i(nmsg.to);
    tmsg->len = p4_n_to_i(nmsg.msg_len);	/* chgd by xdr_recv below */
    tmsg->data_type = p4_n_to_i(nmsg.data_type);
    tmsg->ack_req = p4_n_to_i(nmsg.ack_req);
    if (tmsg->data_type == P4NOX || p4_local->conntab[tmsg->from].same_data_rep)
    {
	n = net_recv(fd, (char *) &(tmsg->msg), tmsg->len);
    }
    else
    {
#       ifdef CAN_DO_XDR
	n = xdr_recv(fd, tmsg);
#       else
	p4_error("cannot do xdr recvs\n",0);
#       endif
    }
    return (tmsg);
}

P4BOOL socket_msgs_available()
{
    int i, fd;

    for (i = 0; i < p4_global->num_in_proctable; i++)
    {
	if (p4_local->conntab[i].type == CONN_REMOTE_EST)
	{
	    fd = p4_local->conntab[i].port;
	    if (sock_msg_avail_on_fd(fd))
	    {
		return (TRUE);
	    }
	}
    }
    return (FALSE);
}

P4BOOL sock_msg_avail_on_fd(fd)
int fd;
{
    int i, rc, nfds;
    struct timeval tv;
    fd_set read_fds;
    char tempbuf[2];

    rc = FALSE;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    SYSCALL_P4(nfds, select(p4_global->max_connections, &read_fds, 0, 0, &tv));

    if (nfds == -1)
    {
	p4_error("sock_msg_avail_on_fd select", nfds);
    }
    if (nfds)			/* true even for eof */
    {
	/* see if data is on the socket or merely an eof condition */
	/* this should not loop long because the select succeeded */
	while ((rc = recv(fd, tempbuf, 1, MSG_PEEK)) == -1)
	    ;	

	if (rc == 0)	/* if eof */
	{
	    /* eof; a process has closed its socket; may have died */
	    for (i = 0; i < p4_global->num_in_proctable; i++)
		if (p4_local->conntab[i].port == fd)
		{
		    p4_local->conntab[i].type = CONN_REMOTE_DYING;
		    /*
		    p4_error("tried to read from dead process",-1);
		    */
		}
	}
	else
	    rc = TRUE;
    }
    return (rc);
}

#ifdef CAN_DO_XDR
int xdr_recv(fd, rmsg)
int fd;
struct p4_msg *rmsg;
{
    xdrproc_t xdr_proc;
    XDR *xdr_dec;
    char *xdr_buff, *msg;
    int i, n;
    int msg_len = 0, nbytes_read = 0;
    int xdr_elsize, els_per_buf, xdr_numels;
    int xdr_len, xdr_len1, len_bytes;

    msg = (char *) &(rmsg->msg);

    xdr_dec = &(p4_local->xdr_dec);
    xdr_buff = p4_local->xdr_buff;
    switch (rmsg->data_type)
    {
      case P4INT:
	xdr_proc = xdr_int;
	xdr_elsize = XDR_INT_LEN;
	break;
      case P4LNG:
	xdr_proc = xdr_long;
	xdr_elsize = XDR_LNG_LEN;
	break;
      case P4FLT:
	xdr_proc = xdr_float;
	xdr_elsize = XDR_FLT_LEN;
	break;
      case P4DBL:
	xdr_proc = xdr_double;
	xdr_elsize = XDR_DBL_LEN;
	break;
      default:
	p4_dprintf("xdr_recv: invalid data type %d\n", rmsg->data_type);
	return (-1);
    }
    xdr_numels = rmsg->len;
    els_per_buf = (XDR_BUFF_LEN - XDR_PAD) / xdr_elsize;
    while (xdr_numels > 0)
    {
	if (xdr_numels > els_per_buf)
	    xdr_len = els_per_buf;
	else
	    xdr_len = xdr_numels;
	xdr_len1 = xdr_len;	/* remember xdr_len */

	len_bytes = (xdr_len * xdr_elsize) + XDR_PAD;
    	p4_dprintfl(90, "xdr_recv: reading %d bytes from %d\n", len_bytes, fd);
	n = net_recv(fd, xdr_buff, len_bytes);
	p4_dprintfl(90, "xdr_recv: read %d bytes \n", n);

	if (n < 0)
	{
	    p4_error("xdr_recv net_recv", n);
	}

	if (!xdr_setpos(xdr_dec, 0))
	{
	    p4_dprintf("xdr_recv: xdr_setpos failed\n");
	    return (-1);
	}

	if (!xdr_array(xdr_dec, &msg, &xdr_len, XDR_BUFF_LEN,
		       xdr_elsize, xdr_proc))
	{
	    p4_dprintf("xdr_recv: xdr_array failed\n");
	    return (-1);
	}

	nbytes_read += len_bytes;
	xdr_numels -= xdr_len1;
	msg = msg + len_bytes - XDR_PAD;
	msg_len = msg_len + len_bytes - XDR_PAD;
    }
    rmsg->len = msg_len;
    return (msg_len);
}
#endif

P4VOID wait_for_ack(fd)
int fd;
{
    struct p4_msg *ack;

    p4_dprintfl(30, "waiting for ack \n");
    ack = socket_recv_on_fd(fd);
    while (!(ack->ack_req & P4_ACK_REPLY_MASK))
    {
	queue_p4_message(ack, p4_local->queued_messages);
	ack = socket_recv_on_fd(fd);
    }
    ack->msg_id = (-1);
    free_p4_msg(ack);
    p4_dprintfl(30, "received ack from %d\n", ack->from);
}

P4VOID send_ack(fd, to)
int fd, to;
{
    struct p4_net_msg_hdr ack;

    p4_dprintfl(30, "sending ack to %d\n", to);
    ack.from = p4_i_to_n(p4_get_my_id());
    ack.data_type = p4_i_to_n(P4NOX);
    ack.msg_len = p4_i_to_n(0);
    ack.to = p4_i_to_n(to);
    ack.ack_req = p4_i_to_n(P4_ACK_REPLY_MASK);
    net_send(fd, &ack, sizeof(ack), FALSE);
    p4_dprintfl(30, "sent ack to %d\n", to);
}

P4VOID shutdown_p4_socks()
/*
  Shutdown all sockets we know about discarding info
  in either direction.
  */
{
    int i;

    if (!p4_local)		/* Need info to be defined */
	return;
    if (!p4_local->conntab)
	return;
    if (p4_local->my_id == LISTENER_ID)
	return;

    for (i = 0; i < p4_num_total_ids(); i++)
	if (p4_local->conntab[i].type == CONN_REMOTE_EST)
	{
	    (P4VOID) shutdown(p4_local->conntab[i].port, 2);
	    (P4VOID) close(p4_local->conntab[i].port);
	}
}
