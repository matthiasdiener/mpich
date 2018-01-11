/* 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/rx_atm.h,v 1.11 1996/10/07 04:40:14 tuecke Exp $" 
 */
#define NEXUS_RX_PROTO_ATM
#define NEXUS_PROTO_TYPE_RX NEXUS_PROTO_TYPE_ATM
#define NEXUS_PROTO_NAME_RX "atm"
#define RX_PROTOCOL_INFO _nx_pr_atm_info
#define RX_INTERFACE_NAME_KEY "atm_interface"

#include <fore_atm/fore_atm_user.h>

/* When atm_accept() or atm_connect() fails, retry this many times. */
#define ATM_ACCEPT_RETRIES		1000
#define ATM_CONNECT_RETRIES		1000

extern int			atm_errno; /* FORE's error variable */

/*
 * Name of my atm_device
 */
static char *			arg_atm_device; /* i.e. /dev/fa0 */
static char *			atm_recv_buffer;
static char *			atm_recv_buffer_pointer;
static int 			atm_recv_buffer_count;
static int			atm_recv_errno;

extern struct dl_error_msg {
    int		errnum;
    char       *errstring;
} dl_error_msgtab[];

/*
 * Biggest chunk that ATM AAL5 can send or receive. 
 * XXX implementation dependent.  Should be changed to be dynamically set.
 *
 */
#if 0
#define FORE_MAX_TRANSMISSION_UNIT	4092
#define ATM_MAX_TRANSMISSION_UNIT	4092
#endif
#define FORE_MAX_TRANSMISSION_UNIT	1024
#define ATM_MAX_TRANSMISSION_UNIT	1024


static int		atm_server(int tcp_fd);
static int		atm_client(char *hostname, int connected_socket);
static int		atm_rx_recv(int fd, char *buf, int size);

static rx_adaptor_funcs_t fore_api_adaptor_funcs =
{
    RX_FORE_ATM,
    atm_client,
    atm_server, 
    atm_send,
    atm_rx_recv,
    atm_close
};


static void fore_atm_defeat_warnings(void)
{
    char *_fore_atm_user_h_sccsid_lint = _fore_atm_user_h_sccsid;
    char *_cdefs_h_rcsid_lint = _cdefs_h_rcsid;
    char *_fore_msg_h_rcsid_lint = _fore_msg_h_rcsid;
    char *_fore_xdr_x_rcsid_lint = _fore_xdr_x_rcsid;

    _fore_atm_user_h_sccsid_lint = _fore_atm_user_h_sccsid_lint;
    _cdefs_h_rcsid_lint = _cdefs_h_rcsid_lint;
    _fore_msg_h_rcsid_lint = _fore_msg_h_rcsid_lint;
    _fore_xdr_x_rcsid_lint = _fore_xdr_x_rcsid;
}

/*
 * rx_type_to_funcs
 */
static rx_adaptor_funcs_t *rx_type_to_funcs( int type )
{
    /* set the adaptor_funcs pointer to use the appropriate adaptor type */
    if (type == RX_FORE_ATM)
        return &fore_api_adaptor_funcs;

    nexus_fatal("rx_type_to_funcs: unknown type %d\n", type);
    return NULL;
} /* rx_type_to_funcs() */


static char *rx_atm_error_string( int errnum )
{
    int i;

    if ( errnum >= 0 )
    {
        return ( dl_error_msgtab[ errnum ].errstring );
    }
    else
    {
        return ( _nx_md_system_error_string(/* errno */) );
    }
} /* rx_atm_error_string */


static int rx_default_adaptor_type( void )
{

    return RX_FORE_ATM;

}  /* rx_default_adaptor_type() */


static void rx_adaptor_init(void)
{
    NexusMalloc(rx_adaptor_init(), atm_recv_buffer, char *,
		sizeof(char) * FORE_MAX_TRANSMISSION_UNIT);
    atm_recv_buffer_pointer = NULL;
    atm_recv_buffer_count = 0;
    atm_recv_errno = 0;

} /* rx_adaptor_init() */


/*
 * atm_server(): 
 * 
 * listen for an atm connection, and act as the server side.
 * 
 * precondition: tcp_fd is connected to the client executing do_connect()
 *
 * postconditions: returns a connected atm_fd using the assigned sap from
 * atm_bind.  This sap is also written to the peer using the tcp_fd.
 *
 */
static int atm_server( int tcp_fd )
{
    int		fd, mtu, qlen, conn_id;
    u_int	switchid, portid;
    Atm_info	info;
    Atm_endpoint calling;
    Atm_qos	qos;
    Atm_sap	ssap;
    Aal_type	aal;
    Atm_dataflow dataflow = duplex;
    extern int	errno;
    int		rc, retries, sigmask;
    long	l_sap;
    nexus_bool_t connected = NEXUS_FALSE;
    int save_error;

    if ( (fd = atm_open( arg_atm_device, O_RDWR, &info )) < 0 ) 
    {
	save_error = atm_errno;
        rx_fatal("server: atm_open: %s\n", _nx_md_system_error_string(save_error));
    }
    mtu = info.mtu; /* Max packet size */
    NexusAssert2((mtu >= ATM_MAX_TRANSMISSION_UNIT),("atm_server(): mtu (%d) < ATM_MAX_TRANSMISSION_UNIT (%d)\n",mtu,ATM_MAX_TRANSMISSION_UNIT));

    /*
     * Bind to a sap and set pending
     * connect request queue length.
     */
    ssap = 0;
    qlen = 1;
    if (atm_bind(fd, ssap, &ssap, qlen) < 0) 
    {
	save_error = atm_errno;
        rx_fatal("atm_server(): atm_bind fails: %s", _nx_md_system_error_string(save_error));
    }

#if BUILD_DEBUG
    if (NexusDebug(3))
        nexus_printf("atm_server(): SAP assigned %d\n",(unsigned int) ssap);
#endif BUILD_DEBUG

    /*
     * Send our server sap to the client side.
     * After this write, do the atm_listen() without delay,
     * especially for any type of printf() that might
     * take a while.
     */
    l_sap = NX_HTONL( (long) ssap );
    if ( (rc = write( tcp_fd, &l_sap, sizeof l_sap )) != sizeof l_sap )
    {
	save_error = errno;
        nexus_fatal("atm_server(): write of l_sap to peer returned %d, error msg: = %s\n",
		    rc,((rc < 0) ? _nx_md_system_error_string(save_error): "< none >"));
    }
    
    /*
     * Wait for a connection request.
     */
    retries = 10;
    while ( !connected )
    {
        if ( (rc = atm_listen( fd, &conn_id, &calling, &qos, &aal ) ) < 0 )
        {
	    save_error = errno;
	    /* must check both atm_errno and plain old errno here! */
	    if (atm_errno == -1 && save_error == EINTR && retries > 0)
	    {
	        /* try again, don't try forever...*/
	        retries--;
	    }
	    else
	    {
	        nexus_fatal("atm_server(): atm_listen failed: %s\n",
			    rx_atm_error_string( atm_errno ) );
		/* Use atm_errno here.  Maybe fix this later to use 
		 * _nx_md_system_error_string */
		/*_nx_md_system_error_string(save_error)*/
            }
	} 
	else
        {
	    connected = NEXUS_TRUE;
	}
    }

    /*
     * Extract the switch id and port id from the NSAP.
     */
    GET_SWITCH(switchid, calling.nsap);
    GET_PORT(portid, calling.nsap);

    nexus_debug_printf(3,("atm_server(): calling switch=%u, port=%d, sap=%d \n", switchid, portid, calling.asap));

#if 0
    printf("qos target peak=%d, mean=%d, burst=%d\n",
        qos.peak_bandwidth.target,
        qos.mean_bandwidth.target,
        qos.mean_burst.target);

    printf("qos minimum peak=%d, mean=%d, burst=%d\n",
        qos.peak_bandwidth.minimum,
        qos.mean_bandwidth.minimum,
        qos.mean_burst.minimum);

    printf("connect conn_id=%d\n", conn_id);
#endif

    /*
     * Request some quality of service.
     */
#define RATE_SERVER 0
    qos.peak_bandwidth.target = RATE_SERVER; /* kbit/sec */
    qos.peak_bandwidth.minimum = 0; /* kbit/sec */
    qos.mean_bandwidth.target = 0; /* kbit/sec */
    qos.mean_bandwidth.minimum = 0; /* kbit/sec */
    qos.mean_burst.target = 0;  /* 205 kbit packet length? */
    qos.mean_burst.minimum = 0;  /* What do these last two fields do? XXX */


    /*
     * Accept the connection request.
     */
    retries = ATM_ACCEPT_RETRIES;
    sigmask = _nx_md_block_sigchld();

    while ( (rc = atm_accept( fd, fd, conn_id, &qos, dataflow ) ) < 0 )
    {
	save_error = errno;
        if ( atm_errno == -1 && save_error == EINTR && retries > 0)
        {
	    retries--;
        }
        else
        {
	    /* atm_error("atm_accept");*/
	    rx_fatal( "atm_server(): atm_accept failed: %s\n",
		       rx_atm_error_string( atm_errno ) );
	    /* Don't use _nx_md_system_error_string() here. */
	    /* _nx_md_system_error_string(save_error)*/
        }
    }
    _nx_md_deliver_sigchld( sigmask );

    return( fd );

} /* atm_server() */

/*
extern int child_death_signal_delivered;
*/

/*
 * atm_client()
 */
static int atm_client( char *hostname, int s )
{
    int fd, mtu, qlen;
    Atm_info info;
    Atm_endpoint dst;
    Atm_qos qos;
    Atm_qos_sel qos_selected;
    Atm_sap sap, ssap;
    Aal_type aal = aal_type_5;
    Atm_dataflow dataflow = duplex;
    int rc, retries, sigmask;
    long l_sap;
    int save_error;

    /* Read the sap of the connecting process */
    if ( ( rc = read( s, &l_sap, sizeof l_sap ) ) != sizeof l_sap )
    {
        save_error = errno;
        nexus_fatal("atm_client(): read of sap returns  %d, errno msg: %s\n", 
		 rc, save_error);
    }
    sap = (Atm_sap) NX_NTOHL( l_sap );
#if BUILD_DEBUG
    if (NexusDebug(3)) 
    {
        nexus_printf("atm_client(): hostname = %s, sap = %d\n",hostname, sap);
    }
#endif /* BUILD_DEBUG */
    if ( (fd = atm_open( arg_atm_device, O_RDWR, &info ) ) < 0 ) 
    {
	save_error = errno;
        rx_fatal("atm_client(): atm_open failed: \n",
		  rx_atm_error_string( atm_errno ) );
		  /*_nx_md_system_error_string(save_error)*/
    }
    mtu = info.mtu;
    NexusAssert2((mtu >= ATM_MAX_TRANSMISSION_UNIT),("atm_client(): mtu (%d) < ATM_MAX_TRANSMISSION_UNIT (%d)\n",mtu,ATM_MAX_TRANSMISSION_UNIT));

    /*
     * Let ATM driver assign a source SAP by specifying
     * sap of zero. Set incoming connect request length to 
     * zero since we're the client. Servers set queue length
     * to non zero.
     */
    ssap = 0;
    qlen = 0;
    if ( atm_bind( fd, ssap, &ssap, qlen ) < 0 ) 
    {
	save_error = errno;
        rx_fatal("atm_client(): atm_bind failed: %s\n",
		  rx_atm_error_string( atm_errno ) );
		  /*_nx_md_system_error_string(save_error)*/
    }
    nexus_debug_printf(3,("atm_client(): SAP assigned=%d\n", ssap));

    /*
     * Initialize the server's data link address.
     */

    if ( atm_gethostbyname( hostname, &dst.nsap ) < 0 ) 
    {
        rx_fatal("atm_gethostbyname failed [%s]: %s\n", hostname,
		  rx_atm_error_string( atm_errno ) );
	/*save_error = errno;
        rx_fatal("atm gethostbyname failed [%s]: %s\n", hostname,
		  _nx_md_system_error_string(save_error)*/
    }

    dst.asap = sap;

    /*
     * Request some quality of service.
     */

#define RATE_CLIENT 0
    qos.peak_bandwidth.target = RATE_CLIENT; /* kbit/sec */
    qos.peak_bandwidth.minimum = 0; /* kbit/sec */
    qos.mean_bandwidth.target = 0; /* kbit/sec */
    qos.mean_bandwidth.minimum = 0; /* kbit/sec */
    qos.mean_burst.target = 0;  /* 64 kbit packet length */
    qos.mean_burst.minimum = 0; /* 64 kbit packet length */

    
    retries = ATM_CONNECT_RETRIES;
    /*
     * block sigchild, but unblock it later.
     */
    sigmask = _nx_md_block_sigchld();

/*    child_death_signal_delivered = 0;*/
    while ( ( rc = atm_connect( fd, &dst, &qos, &qos_selected, aal, dataflow ) ) < 0 )
    {
	save_error = errno;
        if ( atm_errno == -1 && save_error == EINTR && retries > 0)
        {
#ifdef DONT_INCLUDE
	    if (child_death_signal_delivered == 1)
	    {
	        nexus_printf("child_death_signal_delivered\n");
		child_death_signal_delivered--;
	    }
#endif

	    /* try again, don't try forever... */
	    retries--;
	} 
        else
        {
	    /*nexus_printf("atm_client(): errno = %d\n",errno);*/
	    rx_fatal("atm_client(): atm_connect failed: %s\n",
		      rx_atm_error_string( atm_errno ) );
		      /*_nx_md_system_error_string(save_error)*/
        }
    }
    _nx_md_deliver_sigchld( sigmask );

#ifdef BUILD_DEBUG
    if (NexusDebug(6)) 
    {
        nexus_printf("atm_client(): selected qos peak=%d, mean=%d, burst=%d\n",
		     qos_selected.peak_bandwidth,
		     qos_selected.mean_bandwidth,
		     qos_selected.mean_burst);
    }
#endif /* BUILD_DEBUG */

    return( fd );

} /* atm_client() */


/*
 * atm_rx_recv()
 *
 * Dole out the contents of the last datagram delivered
 * by atm_recv() or recv() until it is gone.  Then try another
 * *_recv().  If nothing is ready, return -1 and leave
 * errno as is.
 *
 */

int atm_rx_recv(int fd, char *buf, int size)
{
    int rc;

    tracepoint("atm_rx_recv: enter");
    /* ignore these */
    if ( size == 0 ) return 0;

    /* see if we have anything */
    if ( atm_recv_buffer_count == 0 )
    {
        rc = atm_recv( fd, atm_recv_buffer, ATM_MAX_TRANSMISSION_UNIT );
	atm_recv_buffer_pointer = atm_recv_buffer;
	if ( rc > 0 ) 
        {
	    atm_recv_buffer_count = rc;
	} 
	else
	{
	    atm_recv_errno = atm_errno; /* save errno */
	    if ( atm_errno == -1) atm_recv_errno = errno; /* is this right? */
	    tracepoint("atm_rx_recv: exit with error");
	    return rc; 
	}
    }
    /* At this point, the buffer always has at least 1 byte */

    /* give the user what they want */
    if ( size < atm_recv_buffer_count )
    {
        memcpy( buf, atm_recv_buffer_pointer, size );
	atm_recv_buffer_pointer += size;
	atm_recv_buffer_count -= size;
	rc = size;
    }
    else if ( size >= atm_recv_buffer_count )
    {
        memcpy( buf, atm_recv_buffer_pointer, atm_recv_buffer_count );
	atm_recv_buffer_pointer = NULL;
	rc = atm_recv_buffer_count;
	atm_recv_buffer_count = 0;
    }
    tracepoint("atm_rx_recv: exit");
    return ( rc );

} /* atm_rx_recv() */
