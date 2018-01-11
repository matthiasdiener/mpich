
/* MPD-MAN Client Library */

#include "mpd.h"
#include "mpdlib.h"

#define MAN_INPUT       1
#define PEER_LISTEN     2

struct fdentry fdtable[MAXFDENTRIES];

char myid[IDSIZE];              /* myid is hostname_listener-portnum */
char mylongid[IDSIZE];

int debug = 0;

int myjob, myrank, myjobsize;
int man_msgs_fd, client_listener_fd;

int *peer_socket_table;

int MPD_Init()
{
    int i;
    char *p;

    setbuf(stdout,NULL);  /* turn off buffering for clients */

    if ( ( p = getenv( "MPD_JID" ) ) )
	myjob = atoi( p );
    else
	myjob = -1;
    if ( ( p = getenv( "MPD_JSIZE" ) ) )
	myjobsize = atoi( p );
    else
	myjobsize = -1;
    if ( ( p = getenv( "MPD_JRANK" ) ) )
	myrank = atoi( p );
    else
	myrank = -1;
    sprintf( myid, "cli_%d", myrank );
    if ( ( p = getenv( "MAN_MSGS_FD" ) ) )
	man_msgs_fd = atoi( p );
    else
	man_msgs_fd = -1;
    if ( ( p = getenv( "CLIENT_LISTENER_FD" ) ) )
	client_listener_fd = atoi( p );
    else
	client_listener_fd = -1;
    mpdprintf( debug, "MPD_Init: retrieved from env rank=%d manfd=%d clifd=%d\n",
	       myrank,man_msgs_fd,client_listener_fd );

    peer_socket_table = malloc(myjobsize * sizeof(int));
    for (i=0; i < myjobsize; i++)
    {
	peer_socket_table[i] = -1;
    }

    Signal( SIGUSR1, sigusr1_handler ); /* when poked by manager */

    return(0);
}

int MPD_Finalize()
{
    if (debug==1)
        fprintf(stderr,"MPI Finalize job=%d rank=%d\n", myjob, myrank);
    /* may need to clean up */
    return(0);
}

int MPD_Job()
{
    return(myjob);
}

int MPD_Size()
{
    return(myjobsize);
}

int MPD_Rank()
{
    return(myrank);
}

int MPD_Connect_to_peer(jobid,rank)
int jobid,rank;
{
    char buf[256];

    if (peer_socket_table[rank] >= 0)
	return(peer_socket_table[rank]);
    sprintf( buf, "cmd=request_peer_connection job=%d rank=%d\n", jobid, rank );
    write( man_msgs_fd, buf, strlen(buf)+1 );
    peer_socket_table[rank] = accept_connection( client_listener_fd );
    return(peer_socket_table[rank]);
}

int MPD_Get_ready_peer_info(peer_rank,peer_socket)
int *peer_rank, *peer_socket;
{
    int i, rc, num_fds;
    struct timeval tv;
    fd_set readfds, writefds;

    rc = -1;  /* in case interrupted in select */
    while (rc < 0)
    {
	FD_ZERO( &readfds );
	FD_ZERO( &writefds );

	for (i=0; i < myjobsize; i++)
	    if (peer_socket_table[i] != -1) 
		FD_SET(peer_socket_table[i],&readfds);
	
	num_fds = FD_SETSIZE;
	tv.tv_sec = 0;  /* setup for zero time (null would be indefinite) */
	tv.tv_usec = 0;

	rc = select( num_fds, &readfds, &writefds, NULL, &tv );

	if (rc == 0)
	    break;
	if (rc < 0)
	{
	    if (errno == EINTR)
	    {
		mpdprintf( debug, "select interrupted; continuing\n" );
		continue;
	    }
	    else
		break;
	}

	for (i=0; i < myjobsize; i++)
	{
	    if (peer_socket_table[i] != -1) 
	    {
		if (FD_ISSET(peer_socket_table[i],&readfds))
		{
		    rc = i + 1;  /* make rc > 0 */
		    *peer_rank = i;
		    *peer_socket = peer_socket_table[i];
		    break;
		}
	    }
	}
    }
    return(rc);
}

int MPD_Get_peer_host_and_port(job,rank,peerhost,peerport)
int job, rank, *peerport;
char *peerhost;
{
    int i;
    char buf[256];
	
    sprintf( buf, "cmd=findclient job=%d rank=%d\n", job, rank );
    write( man_msgs_fd, buf, strlen(buf)+1 );
    i = read_line( man_msgs_fd, buf, 256 );  
    mpdprintf(1,"MPDLIB rc=%d reply=>:%s:\n",i,buf);
    parse_keyvals( buf );
    getval( "cmd", buf );
    if ( strcmp( "foundclient", buf ) != 0 ) {
	mpdprintf( 1, "expecting foundclient, got :%s:\n", buf );
	return(-1);
    }
    getval( "host", peerhost );
    getval( "port", buf );
    *peerport = atoi( buf );
    if ( *peerport < 0 ) {
	mpdprintf( 1, "MPD_Get_peer_host_and_port: failed to find client :%d %d:\n", job,rank );
	return(-1);
    }
    mpdprintf( 1, "LOCATED job=%d rank=%d at peerhost=%s peerport=%d\n",
	       job, rank, peerhost, *peerport);
    return(0);	
}

void sigusr1_handler( signo )
int signo;
{
    int rc, peer_port, peer_rank;
    char buf[MAXLINE], peer_hostname[MAXLINE];

    mpdprintf( 1, "cli got SIGUSR1\n" );
    rc = read_line(man_msgs_fd,buf,MAXLINE);
    mpdprintf( 1, "sigusr1_handler got buf=:%s:\n",buf );
    parse_keyvals( buf );
    getval("requesting_host",peer_hostname);
    getval("requesting_port",buf);
    peer_port = atoi(buf);
    getval("rank",buf);
    peer_rank = atoi(buf);
    peer_socket_table[peer_rank] = network_connect(peer_hostname,peer_port);
}

