#include "mpd.h"

extern struct portentry porttable[MAXFDENTRIES];
extern int    myrank;	
extern int    console_idx;
extern int    debug;

int setup_unix_socket( pathname )	
char *pathname;
{
    int backlog = 15;
    int rc;
    int skt_fd;
    struct sockaddr_un sa;

    bzero( (void *) &sa, sizeof( sa ) );
    sa.sun_family = AF_UNIX;
    strncpy( sa.sun_path, pathname, sizeof( sa.sun_path ) - 1 );

    skt_fd = socket( AF_UNIX, SOCK_STREAM, 0 );
    if ( skt_fd < 0 )
        return( skt_fd );

    rc = bind( skt_fd, ( struct sockaddr * )&sa, sizeof( sa ) );
    if ( skt_fd < 0 )
        return( skt_fd );

    rc = listen( skt_fd, backlog );
    if ( rc < 0 )
        return( rc );
    
    mpdprintf( debug, "listening on local socket %d\n", skt_fd );
    return( skt_fd );
}

int network_connect( hostname, port )
char *hostname;
int  port;
{
    int s;
    struct sockaddr_in sa;
    struct hostent *hp;
    int optval = 1;
    int rc, numtriesleft, connected;
#define NUMTOTRY 100

    hp = gethostbyname( hostname );
    if (hp == NULL)
    {
	char errmsg[80];
	sprintf( errmsg, "network_connect: gethostbyname=:%s:", hostname );
	perror( errmsg );
	fatal_error( -1, errmsg );	
    }

    mpdprintf( debug, "attempting network connection to %s, port %d\n",
	     hostname, port );

    bzero((void *)&sa, sizeof(sa));
    bcopy((void *)hp->h_addr, (void *)&sa.sin_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port	  = htons(port);

    connected = 0;
    numtriesleft  = NUMTOTRY; 

    while ( !connected && numtriesleft > 0 ) {
	s = socket( AF_INET, SOCK_STREAM, 0 );
	error_check( s, "network_connect, socket" );

	rc = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof( optval ) );
	error_check( rc, "network_connect, setsockopt" );

	rc = connect( s, (struct sockaddr *) &sa, sizeof(sa) );
	if ( rc == 0 )
	    connected = 1;
	else {
	    numtriesleft--;
	    dclose( s );
	}
    }
    if ( !connected ) 
	mpdprintf( 1, "failed to connect after %d tries\n", NUMTOTRY );

    error_check( rc, "network_connect, connect");

    if ( numtriesleft < NUMTOTRY )
	mpdprintf( 1, "network_connect, connected on fd %d after %d %s\n", s,
		   NUMTOTRY + 1 - numtriesleft,
		   NUMTOTRY + 1 - numtriesleft > 1 ? "tries" : "try" );
    return s;
}

int accept_connection( skt )
int skt;
{
    struct sockaddr_in from;
    int new_skt, gotit, rc;
    mpd_sockopt_len_t fromlen;
    int optval = 1;

    mpdprintf( 0, "accepting connection on %d\n", skt );
    fromlen = sizeof( from );
    gotit = 0;
    while ( !gotit ) {
	new_skt = accept( skt, ( struct sockaddr * ) &from, &fromlen );
	if ( new_skt == -1 )
	{
	    if ( errno == EINTR )
		continue;
	    else
		error_check( new_skt, "accept_connection accept" );
	}
	else
	    gotit = 1;
    }

    rc = setsockopt( new_skt, IPPROTO_TCP, TCP_NODELAY, (char *) &optval,
		     sizeof( optval ) );
    error_check( rc, "accept_connection, setsockopt" );

    mpdprintf( debug, "accept_connection; new socket = %d\n", new_skt );
    return( new_skt );
}


int accept_unix_connection( skt )
int skt;
{
    struct sockaddr_in from;
    int new_skt, gotit;
    mpd_sockopt_len_t fromlen;

    mpdprintf( 0, "accepting unix connection on %d\n", skt );
    fromlen = sizeof( from );
    gotit = 0;
    while ( !gotit ) {
	new_skt = accept( skt, ( struct sockaddr * ) &from, &fromlen );
	if ( new_skt == -1 )
	{
	    if ( errno == EINTR )
		continue;
	    else
		error_check( new_skt, "accept_connection accept" );
	}
	else
	    gotit = 1;
    }

    mpdprintf( debug, "accept_unix_connection; new socket = %d\n", new_skt );
    return( new_skt );
}

int recv_msg( fd, buf )
int fd;
char *buf;
{
    int n;

    n = read( fd, buf, 999 );
    error_check( n, "recv_msg read" );
    if ( n == 0 )
	return( RECV_EOF );
    return( RECV_OK );
}


