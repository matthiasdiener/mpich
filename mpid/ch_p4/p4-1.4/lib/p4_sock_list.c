#include "p4.h"
#include "p4_sys.h"

P4VOID listener()
{
    struct listener_data *l = listener_info;
    P4BOOL done = FALSE;
    fd_set read_fds;
    int i, nfds, fd;

    p4_dprintfl(70, "enter listener \n");
    dump_listener(70);

    while (!done)
    {
	FD_ZERO(&read_fds);
	FD_SET(l->listening_fd, &read_fds);
	FD_SET(l->slave_fd, &read_fds);

	SYSCALL_P4(nfds, select(p4_global->max_connections, &read_fds, 0, 0, 0));
	if (nfds < 0)
	    p4_error("listener select", nfds);
	if (nfds == 0)
	    p4_dprintfl(70, "select timeout\n");

	fd = 0;
	for (i = 0; i < nfds && !done; i++)
	{
	    while (fd < p4_global->max_connections)
	    {
		if (FD_ISSET(fd, &read_fds))
		{
		    if (fd == l->listening_fd || fd == l->slave_fd)
			break;
		}
		fd++;
	    }

	    p4_dprintfl(70, "got fd=%d listening_fd=%d slave_fd=%d\n",
			fd, l->listening_fd, l->slave_fd);

	    if (fd == l->listening_fd)
		done = process_connect_request(fd);
	    else
		done = process_slave_message(fd);
	    fd++;
	}
    }

    p4_dprintfl(70, "exit listener\n");
    exit(0);
}

P4BOOL process_connect_request(fd)
int fd;
{
    struct slave_listener_msg msg;
    int type;
    int connection_fd, slave_fd;
    int from, lport, to_pid, to;
    P4BOOL rc = FALSE;

    p4_dprintfl(70, "processing connect check/request on %d\n", fd);

    connection_fd = net_accept(fd);

    p4_dprintfl(70, "accepted on connection_fd=%d reading size=%d\n", connection_fd,sizeof(msg));

    if (net_recv(connection_fd, &msg, sizeof(msg)) == PRECV_EOF)
    {
	return (FALSE);
    }

    type = p4_n_to_i(msg.type);
    switch (type)
    {
      case IGNORE_THIS:
	p4_dprintfl(70, "got IGNORE_THIS\n");
	break;

      case CONNECTION_REQUEST:
	from = p4_n_to_i(msg.from);
	to_pid = p4_n_to_i(msg.to_pid);
	to = p4_n_to_i(msg.to);
	lport = p4_n_to_i(msg.lport);
	p4_dprintfl(70, "connection_request2: poking slave: from=%d lport=%d to_pid=%d to=%d\n",
		    from, lport, to_pid, to);

	slave_fd = listener_info->slave_fd;

	if (kill(to_pid, LISTENER_ATTN_SIGNAL) == -1)
	{
	    p4_dprintf("Listener: Unable to interrupt client pid=%d.\n", to_pid);
	    break;
	}

	net_send(slave_fd, &msg, sizeof(msg), FALSE);
	/* wait for msg from slave indicating it got connected */
	/*
	 * do not accept any more connections for slave until it has fully
	 * completed this one, i.e. do not want to interrupt it until it has
	 * handled this interrupt
	 */
	p4_dprintfl(70, "waiting for slave to handle interrupt\n");
	net_recv(slave_fd, &msg, sizeof(msg));
	/* Check that we get a valid message; for now (see p4_sock_conn/
	   handle_connection_interrupt) this is just IGNORE_THIS */
	if (p4_i_to_n(msg.type) != IGNORE_THIS) {
	    p4_dprintf("received incorrect handshake message type=%d\n", 
		       p4_i_to_n(msg.type) );
	    p4_error("slave_listener_msg: broken handshake", 
		     p4_i_to_n(msg.type));
	    }
	p4_dprintfl(70, "back from slave handling interrupt\n");
	break;

      default:
	p4_dprintf("invalid type %d in process_connect_request\n", type);
	break;
    }
    close(connection_fd);
    return (rc);
}

P4BOOL process_slave_message(fd)
int fd;
{
    struct slave_listener_msg msg;
    int type;
    int from;
    P4BOOL rc = FALSE;
    int status;

    status = net_recv(fd, &msg, sizeof(msg));
    if (status == PRECV_EOF)
    {
	p4_error("slave_listener_msg: got eof on fd=", fd);
    }

    type = p4_n_to_i(msg.type);
    from = p4_n_to_i(msg.from);

    switch (type)
    {
      case DIE:
	p4_dprintfl(70, "received die msg from %d\n", from);
	rc = TRUE;
	break;

      default:
	p4_dprintf("received unknown message type=%d from=%d\n", type, from);
	p4_error("slave_listener_msg: unknown message type", type);
	break;
    }

    return (rc);
}
