/* 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/rx_udp.h,v 1.11 1996/10/07 04:40:15 tuecke Exp $" 
 */
#define NEXUS_RX_PROTO_UDP
#define NEXUS_PROTO_TYPE_RX NEXUS_PROTO_TYPE_UDP
#define NEXUS_PROTO_NAME_RX "udp"
#define RX_PROTOCOL_INFO _nx_pr_udp_info
#define RX_INTERFACE_NAME_KEY "udp_interface"

static char *			udp_recv_buffer;
static char *			udp_recv_buffer_pointer;
static int 			udp_recv_buffer_count;
static int			udp_recv_errno;


/*
 * Fix this for UDP later
 */
#define UDP_MAX_TRANSMISSION_UNIT	1024
#define ATM_MAX_TRANSMISSION_UNIT	1024

static void		udp_set_socket_size(int s);
static int		udp_server(int tcp_fd); 
static int		udp_client(char *hostname, int connected_socket);
static int		udp_send(int fd, const char *buf, int len);
static int		udp_recv(int fd, char *buf, int size);


static rx_adaptor_funcs_t udp_adaptor_funcs =
{
    RX_UDP,
    udp_client,
    udp_server,
    udp_send,
    udp_recv,
    close
};


/*
 * rx_type_to_funcs
 */
static rx_adaptor_funcs_t *rx_type_to_funcs( int type )
{
    /* set the adaptor_funcs pointer to use the appropriate adaptor type */

    if (type == RX_UDP)
        return &udp_adaptor_funcs;

    nexus_fatal("rx_type_to_funcs: unknown type %d\n", type);
    return NULL;

} /* rx_type_to_funcs() */


static int rx_default_adaptor_type(void)
{
    return RX_UDP;

} /* rx_default_adaptor_type() */


static void rx_adaptor_init(void)
{
    NexusMalloc(rx_adaptor_init(), udp_recv_buffer, char *,
		sizeof(char) * UDP_MAX_TRANSMISSION_UNIT);
    udp_recv_buffer_pointer = NULL;
    udp_recv_buffer_count = 0;
    udp_recv_errno = 0;

} /* rx_adaptor_init() */


/*
 * udp_set_socket_size()
 */
#define MAX_SOCKET_SEND_BUFFER		9000
#define MAX_SOCKET_RECEIVE_BUFFER	18032

static void udp_set_socket_size( int s )
{
    int size;
    int sock_size;
    int sock_opt_len = sizeof(int);
    int save_error;

    size = MAX_SOCKET_SEND_BUFFER;
    setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *) size, sizeof(int) );

    if ( getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &sock_size, 
		   &sock_opt_len) < 0 ) 
    {
	save_error = errno;
        rx_fatal("udp_set_socket_size: getsockopt failed. errno = %d\n",
		  save_error);
        sock_size = -1;
    }
    if ( sock_size != size )
    {
        nexus_printf("udp_set_socket_size: set to %d instead of %d\n",
		     sock_size, size);
    }

    size = MAX_SOCKET_RECEIVE_BUFFER;

    /* bug fix, 7/5/95 cdematt, there was no & in front of size... */
    
    setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(int) );
    if ( getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &sock_size, 
		   &sock_opt_len) < 0 ) 
    {
	save_error = errno;
        rx_fatal("udp_set_socket_size: getsockopt failed. errno = %d\n",
		  save_error);
        sock_size = -1;
    }
    if ( sock_size != size )
    {
        nexus_printf("udp_set_socket_size: set to %d instead of %d\n",
		     sock_size, size);
    }
    
} /* udp_set_socket_size */


/*
 * udp_server():
 *
 * Create a UDP socket, bind a port and act as the server side.
 * Send the port number to the peer using the TCP socket.  Then read
 * back the peer host's port number and do a connect() on it so
 * that datagrams sent using write() will be delivered to the remote
 * host.
 *
 * precondition: tcp_fd is connected to the client executing do_connect()
 *
 * postconditions: returns a udp_fd using the assigned port from
 * bind, connected to the remote host. 
 */

static int udp_server(int tcp_fd)
{
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    int len;
    int udp_fd;
    short udp_port;
    short s_port;
/*    int remote_udp_port;*/
/*    struct hostent *hp;*/
    int rc;
    int save_error;

    /* create and bind() the local socket. */ 
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    save_error = errno;
    if (udp_fd < 0)
    {
	rx_fatal("udp_server(): socket() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }

    udp_set_socket_size( udp_fd );

    /* figure out my local IP addr from the tcp_fd */
    /* not sure if getsockname(tcp_fd) will fill in local or remote... */
    len = sizeof(my_addr);
    if (getsockname(tcp_fd, (struct sockaddr *) &my_addr, &len) < 0)
    {
        save_error = errno;
	rx_fatal("udp_server(): getsockname() call1 failed: %s\n",
		  _nx_md_system_error_string(errno));
    }

    /* Choose a new port */
    my_addr.sin_port = htons(0);

    if ( bind(udp_fd, (struct sockaddr *) &my_addr, sizeof( my_addr )) < 0)
    {
        save_error = errno;
	rx_fatal("udp_server(): bind() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    
    /* figure out the local port number */
    len = sizeof( my_addr );
    if (getsockname( udp_fd, (struct sockaddr *)&my_addr, &len) < 0)
    {
        save_error = errno;
        rx_fatal("udp_server(): getsockname() call2 failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    udp_port = ntohs( my_addr.sin_port );

    /* send the local port number to the remote host */
    if ((rc = write(tcp_fd, &udp_port, sizeof(udp_port))) != sizeof(udp_port))
    {
	save_error = errno;
        rx_fatal("udp_server(): write of udp_port to peer returned %d. error msg: %s\n",
		  rc, ((rc < 0) ? _nx_md_system_error_string(save_error): "< none >"));
    }

    /* read the port that the remote host is using. 
     * leave it in remote_udp_port */
    if ((rc = read(tcp_fd, &s_port, sizeof(s_port))) != sizeof(s_port))
    {
	save_error = errno;
	nexus_fatal("udp_server(): read of udp_port returns %d, errno =%d\n",
		    rc, save_error);
    }
    /* connect() the local socket to the remote host and port */
    
    /* figure out address of peer */
    len = sizeof(client_addr);
    if (getpeername(tcp_fd, (struct sockaddr *) &client_addr, &len) < 0)
    {
        save_error = errno;
	rx_fatal("udp_server(): getpeername() failed: %s\n",
		  _nx_md_system_error_string(errno));
    }

    client_addr.sin_port = s_port;
    if ( connect(udp_fd, 
		 (struct sockaddr *)&client_addr, 
		 sizeof(client_addr)) < 0 )
    {
	save_error = errno;
        rx_fatal("udp_server(): connect failed, errno =  %d. error msg: %s\n",
		  save_error, _nx_md_system_error_string(save_error));
    }

    return (udp_fd);

} /* udp_server() */


/*
 * udp_client()
 */
static int udp_client( char *hostname, int connected_socket )
{
    struct sockaddr_in server_addr;
    struct sockaddr_in my_addr;
    int len;
    int udp_fd;
    short udp_port;
    short s_port;
    int rc;
    int save_error;

    /* read the port that the remote host is using.  leave it in udp_port */
    if ( (rc = read(connected_socket, &s_port, sizeof s_port)) != sizeof s_port )
    {
	save_error = errno;
	rx_fatal("udp_client(): read of udp_port returns %d, errno =%d\n",
		    rc, save_error);
    }

    /* create the socket on this end.  Then bind() it to a local
     * port and connect() it to the remote port */
    udp_fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( udp_fd < 0 )
    {
        save_error = errno;
	rx_fatal("udp_client(): socket() call failed: %s\n",
		  _nx_md_system_error_string(errno));
    }

    udp_set_socket_size( udp_fd );

    /* figure out my local IP addr from the tcp_fd */
    /* not sure if getsockname() will fill in local or remote... */
    len = sizeof( my_addr );
    if ( getsockname( connected_socket, (struct sockaddr *) &my_addr, &len ) < 0 )
    {
        save_error = errno;
	rx_fatal("udp_client(): getsockname() call1 failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    
/*    ZeroOutMemory( &server_addr, sizeof server_addr );*/
/*    server_addr.sin_addr.s_addr = INADDR_ANY;*/
/*    serer_addr.sin_family = AF_INET;*/

    /* Choose a new port */
    my_addr.sin_port = htons(0);

    if ( bind(udp_fd, (struct sockaddr *) &my_addr, sizeof( my_addr )) < 0)
    {
        save_error = errno;
	rx_fatal("udp_client(): bind() call failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    
    /* figure out the local port number */
    len = sizeof( my_addr );
    if (getsockname( udp_fd, (struct sockaddr *)&my_addr, &len) < 0)
    {
        save_error = errno;
        rx_fatal("udp_client(): getsockname() call2 failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    udp_port = ntohs( my_addr.sin_port );

    /* send the local port number to the remote host */
    if ((rc = write(connected_socket, &udp_port, sizeof(udp_port))) 
	!= sizeof(udp_port))
    {
	save_error = errno;
        rx_fatal("udp_client(): write of udp_port to peer returned %d. error msg: %s\n",
		  rc, ((rc < 0) ? _nx_md_system_error_string(save_error): "< none >"));
    }

    /* s_port has remote port */
    /* figure out address of peer */
    len = sizeof(server_addr);
    if ( getpeername(connected_socket, (struct sockaddr *) &server_addr, &len )
	< 0)
    {
        save_error = errno;
	rx_fatal("udp_client(): getpeername() failed: %s\n",
		  _nx_md_system_error_string(save_error));
    }
    
    /* Use this port */
    server_addr.sin_port = s_port;

    if (connect(udp_fd, 
		(struct sockaddr *)&server_addr, 
		sizeof server_addr) < 0)
    {
	save_error = errno;
        rx_fatal("udp_client(): connect failed, errno =  %d. error msg: %s\n",
		  save_error, _nx_md_system_error_string(save_error));
    }

    return( udp_fd );

} /* udp_client() */


/*
 * udp_send()
 */
int udp_send(int fd, const char *buf, int len)
{
    return (send( fd, buf, len, 0));

} /* udp_send() */


/*
 * udp_recv()
 *
 * Dole out the contents of the last datagram delivered
 * by recv() until it is gone.  Then try another
 * recv().  If nothing is ready, return -1 and leave
 * errno as is.
 *
 */
int udp_recv(int fd, char *buf, int size)
{
    int rc;

    tracepoint("udp_recv: enter");
    /* ignore these */
    if (size == 0) return 0;

    /* see if we have anything */
    if ( udp_recv_buffer_count == 0 )
    {
        rc = recv( fd, udp_recv_buffer, ATM_MAX_TRANSMISSION_UNIT, 0 );
	udp_recv_buffer_pointer = udp_recv_buffer;
	if ( rc > 0 ) 
        {
	    udp_recv_buffer_count = rc;
	} 
	else
	{
	    udp_recv_errno = errno; /* save errno */
	    tracepoint("udp_recv: exit with error");
	    return ( rc ); 
	}
    }
    /* At this point, the buffer always has at least 1 byte */

    /* give the user what they want */
    if ( size < udp_recv_buffer_count )
    {
        memcpy( buf, udp_recv_buffer_pointer, size );
	udp_recv_buffer_pointer += size;
	udp_recv_buffer_count -= size;
	rc = size;
    }
    else if ( size >= udp_recv_buffer_count )
    {
        memcpy( buf, udp_recv_buffer_pointer, udp_recv_buffer_count );
	udp_recv_buffer_pointer = NULL;
	rc = udp_recv_buffer_count;
	udp_recv_buffer_count = 0;
    }
    tracepoint("udp_recv: exit");
    return ( rc );

} /* udp_recv() */



