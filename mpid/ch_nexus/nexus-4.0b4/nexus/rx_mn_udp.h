/* 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/rx_mn_udp.h,v 1.10 1996/10/07 04:40:15 tuecke Exp $"
 */
#define NEXUS_RX_PROTO_MN_UDP
#define NEXUS_PROTO_TYPE_RX NEXUS_PROTO_TYPE_MN_UDP
#define NEXUS_PROTO_NAME_RX "mn_udp"
#define RX_PROTOCOL_INFO _nx_pr_mn_udp_info
#define RX_INTERFACE_NAME_KEY "mn_udp_interface"

#include <mn_cglue.h>

static char *			mn_udp_recv_buffer;
static char *			mn_udp_recv_buffer_pointer;
static int			mn_udp_recv_buffer_count;
static int			mn_udp_recv_errno;

/*
 * Biggest chunk that interface can send or receive. 
 * XXX implementation dependent.  Should be changed to be dynamically set.
 *
 * Fix this for UDP later
 */

#define MN_UDP_MAX_TRANSMISSION_UNIT	8192
#define ATM_MAX_TRANSMISSION_UNIT	8192

static int		mn_udp_server(int tcp_fd); 
static int		mn_udp_client(char *hostname, int connected_socket);
static int		mn_udp_send(int fd, const char *buf, int len);
static int		mn_udp_recv(int fd, char *buf, int size);

static rx_adaptor_funcs_t mn_udp_adaptor_funcs =
{
    RX_MN_UDP,
    mn_udp_client,
    mn_udp_server,
    mn_udp_send,
    mn_udp_recv,
    mn_close
};


/*
 * rx_type_to_funcs
 */
static rx_adaptor_funcs_t *rx_type_to_funcs( int type )
{
    /* set the adaptor_funcs pointer to use the appropriate adaptor type */

    if (type == RX_MN_UDP)
        return &mn_udp_adaptor_funcs;
    nexus_fatal("rx_type_to_funcs: unknown type %d\n", type);
    return NULL;

} /* rx_type_to_funcs() */


static int rx_default_adaptor_type(void)
{
    return RX_MN_UDP;

} /* rx_default_adaptor_type() */


static void rx_adaptor_init(void)
{
    NexusMalloc(rx_adaptor_init(), mn_udp_recv_buffer, char *,
		sizeof(char) * MN_UDP_MAX_TRANSMISSION_UNIT);
    mn_udp_recv_buffer_pointer = NULL;
    mn_udp_recv_buffer_count = 0;
    mn_udp_recv_errno = 0;

} /* rx_adaptor_init() */


static int mn_udp_server( int tcp_fd )
{
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    int len;
    int udp_fd;
    short udp_port;
    short s_port;
    int rc;
    int save_error;

    /* create and bind() the local socket. */ 
    udp_fd = mn_socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0)
    {
	rx_fatal("mn_udp_server(): socket() call failed: %s\n",
		  _nx_md_system_error_string());
    }

    /* figure out my local IP addr from the tcp_fd */
    /* not sure if getsockname(tcp_fd) will fill in local or remote... */
    len = sizeof(my_addr);
    if (getsockname(tcp_fd, (struct sockaddr *) &my_addr, &len) < 0)
    {
	rx_fatal("mn_udp_server(): getsockname() call1 failed: %s\n",
		  _nx_md_system_error_string());
    }

    /* Choose a new port */
    my_addr.sin_port = htons(0);

    if ( mn_bind(udp_fd, (struct sockaddr *) &my_addr, sizeof( my_addr )) < 0)
    {
	rx_fatal("mn_udp_server(): mn_bind() call failed: %s\n",
		  _nx_md_system_error_string());
    }
    
    /* figure out the local port number */
    len = sizeof( my_addr );
    if (mn_getsockname( udp_fd, (struct sockaddr *)&my_addr, &len) < 0)
    {
        rx_fatal("mn_udp_server(): getsockname() call2 failed: %s\n",
		  _nx_md_system_error_string());
    }
    udp_port = ntohs( my_addr.sin_port );

    /* send the local port number to the remote host */
    if ((rc = write(tcp_fd, &udp_port, sizeof(udp_port))) != sizeof(udp_port))
    {
	save_error = errno;
        rx_fatal("mn_udp_server(): write of udp_port to peer returned %d. error msg: %s\n",
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
	rx_fatal("mn_udp_server(): getpeername() failed: %s\n",
		  _nx_md_system_error_string());
    }

    client_addr.sin_port = s_port;
    if ( mn_connect(udp_fd, 
		    (struct sockaddr *)&client_addr, 
		    sizeof(client_addr)) < 0 )
    {
	save_error = mn_errno;
        rx_fatal("mn_udp_server(): connect failed, errno =  %d. error msg: %s\n",
		  save_error, _nx_md_system_error_string(save_error));
    }

    return (udp_fd);

} /* mn_udp_server() */


/*
 * mn_udp_client()
 */
static int mn_udp_client( char *hostname, int connected_socket )
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
    if ((rc = read(connected_socket, &s_port, sizeof s_port)) != sizeof s_port)
    {
	save_error = errno;
	rx_fatal("mn_udp_client(): read of udp_port returns %d, errno =%d\n",
		    rc, save_error);
    }

    /* create the socket on this end.  Then bind() it to a local
     * port and connect() it to the remote port */
    udp_fd = mn_socket( AF_INET, SOCK_DGRAM, 0 );
    if ( udp_fd < 0 )
    {
	rx_fatal("mn_udp_client(): socket() call failed: %s\n",
		  _nx_md_system_error_string());
    }

    /*udp_set_socket_size( udp_fd );*/

    /* figure out my local IP addr from the tcp_fd */
    /* not sure if getsockname() will fill in local or remote... */
    len = sizeof(my_addr);
    if ( getsockname( connected_socket, (struct sockaddr *) &my_addr, &len ) < 0 )
    {
	rx_fatal("mn_udp_client(): getsockname() call1 failed: %s\n",
		  _nx_md_system_error_string());
    }
    
/*    ZeroOutMemory( &server_addr, sizeof server_addr );*/
/*    server_addr.sin_addr.s_addr = INADDR_ANY;*/
/*    serer_addr.sin_family = AF_INET;*/

    /* Choose a new port */
    my_addr.sin_port = htons(0);

    if ( mn_bind( udp_fd, (struct sockaddr *) &my_addr, sizeof( my_addr ) ) < 0 )
    {
	rx_fatal("mn_udp_client(): mn_bind() call failed: %s\n",
		  _nx_md_system_error_string());
    }
    
    /* figure out the local port number */
    len = sizeof( my_addr );
    if ( mn_getsockname( udp_fd, (struct sockaddr *)&my_addr, &len) < 0 )
    {
        rx_fatal("mn_udp_client(): getsockname() call2 failed: %s\n",
		  _nx_md_system_error_string());
    }
    udp_port = ntohs( my_addr.sin_port );

    /* send the local port number to the remote host */
    if ((rc = write( connected_socket, &udp_port, sizeof(udp_port)) ) 
	!= sizeof(udp_port) )
    {
	save_error = errno;
        rx_fatal("mn_udp_client(): write of udp_port to peer returned %d. error msg: %s\n",
		  rc, ((rc < 0) ? _nx_md_system_error_string(save_error): "< none >"));
    }

    /* s_port has remote port */
    /* figure out address of peer */
    len = sizeof( server_addr );
    if ( getpeername( connected_socket, (struct sockaddr *) &server_addr, &len )
	< 0 )
    {
	rx_fatal("mn_udp_client(): getpeername() failed: %s\n",
		  _nx_md_system_error_string());
    }
    
    /* Use this port */
    server_addr.sin_port = s_port;

    if ( mn_connect(udp_fd, 
		    (struct sockaddr *)&server_addr, 
		    sizeof server_addr ) < 0 )
    {
	save_error = mn_errno;
        rx_fatal("mn_udp_client(): connect failed, errno =  %d. error msg: %s\n",
		  save_error, _nx_md_system_error_string(save_error));
    }

    return( udp_fd );

} /* mn_udp_client() */


/*
 * mn_udp_send()
 */
int mn_udp_send( int fd, const char *buf, int len )
{
    return ( mn_write( fd, (char *) buf, len ) );

} /* mn_udp_send() */


/*
 * mn_udp_recv()
 *
 * Dole out the contents of the last datagram delivered
 * by recv() until it is gone.  Then try another
 * recv().  If nothing is ready, return -1 and leave
 * errno as is.
 *
 */
int mn_udp_recv( int fd, char *buf, int size )
{
    int rc;

    tracepoint("mn_udp_recv: enter");
    /* ignore these */
    if (size == 0) return 0;

    /* see if we have anything */
    if (mn_udp_recv_buffer_count == 0)
    {
        rc = mn_read( fd, mn_udp_recv_buffer, ATM_MAX_TRANSMISSION_UNIT );
	mn_udp_recv_buffer_pointer = mn_udp_recv_buffer;
	if (rc > 0) 
        {
	    mn_udp_recv_buffer_count = rc;
	} 
	else
        {
	    mn_udp_recv_errno = mn_errno; /* save errno */
	    tracepoint("mn_udp_recv: exit with error");
	    return rc; 
        }
    }
    /* At this point, the buffer always has at least 1 byte */

    /* give the user what they want */
    if ( size < mn_udp_recv_buffer_count )
    {
        memcpy( buf, mn_udp_recv_buffer_pointer, size );
	mn_udp_recv_buffer_pointer += size;
	mn_udp_recv_buffer_count -= size;
	rc = size;
    }
    else if ( size >= mn_udp_recv_buffer_count )
    {
        memcpy( buf, mn_udp_recv_buffer_pointer, mn_udp_recv_buffer_count);
	mn_udp_recv_buffer_pointer = NULL;
	rc = mn_udp_recv_buffer_count;
	mn_udp_recv_buffer_count = 0;
    }
    tracepoint("mn_udp_recv: exit");
    return rc;

} /* mn_udp_recv() */

