
/* MPD Client Library */

#include "mpd.h"
#include "mpdlib.h"

#define MAN_INPUT       1
#define PEER_LISTEN     2

/* definitions used by utils */
char myid[IDSIZE];              /* descriptive character string, like man_2, cli_3 */
int  debug = 0;
struct fdentry fdtable[MAXFDENTRIES]; 

/* mpdlib global variables */
int  mpdlib_myjob, mpdlib_myrank, mpdlib_myjobsize;
int  mpdlib_man_msgs_fd, mpdlib_peer_listen_fd;
void (*MPD_user_peer_msg_handler)(char *) = NULL;  /* default */

int MPD_Init( void (*peer_msg_handler)(char *) )
{
    char *p;
    char buf[MAXLINE];
    static int firstcall = 1;
    
    if ( firstcall )
	firstcall = 0;
    else
	return( 0 );

    setbuf(stdout,NULL);  /* turn off buffering for clients */

    MPD_user_peer_msg_handler = peer_msg_handler;
    if ( ( p = getenv( "MPD_JID" ) ) )
        mpdlib_myjob = atoi( p );
    else
        mpdlib_myjob = -1;
    if ( ( p = getenv( "MPD_JSIZE" ) ) )
        mpdlib_myjobsize = atoi( p );
    else
        mpdlib_myjobsize = -1;
    if ( ( p = getenv( "MPD_JRANK" ) ) )
        mpdlib_myrank = atoi( p );
    else
        mpdlib_myrank = -1;
    sprintf( myid, "cli_%d", mpdlib_myrank );
    if ( ( p = getenv( "MAN_MSGS_FD" ) ) )
        mpdlib_man_msgs_fd = atoi( p );
    else
        mpdlib_man_msgs_fd = -1;
    if ( ( p = getenv( "CLIENT_LISTENER_FD" ) ) )
        mpdlib_peer_listen_fd = atoi( p );
    else
        mpdlib_peer_listen_fd = -1;
    mpdprintf( debug, "MPD_Init: retrieved from env rank=%d manfd=%d clifd=%d\n",
               mpdlib_myrank,mpdlib_man_msgs_fd,mpdlib_peer_listen_fd );

    Signal( SIGUSR1, sigusr1_handler ); /* when poked by manager */
    sprintf( buf, "cmd=accepting_signals pid=%d\n",getpid() );
    write( mpdlib_man_msgs_fd, buf, strlen(buf) );
    mpdprintf( debug, "MPD_Init: sent accepting_signals to man\n");

    return(0);
}

int MPD_Finalize( void )
{
    if (debug==1)
        fprintf(stderr,"MPI Finalize job=%d rank=%d\n", mpdlib_myjob, mpdlib_myrank);
    close( mpdlib_man_msgs_fd );
    /* may need to clean up */
    return(0);
}

int MPD_Job( void )
{
    return(mpdlib_myjob);
}

int MPD_Size( void )
{
    return(mpdlib_myjobsize);
}

int MPD_Rank( void )
{
    return(mpdlib_myrank);
}

int MPD_Peer_listen_fd( void )
{
    return(mpdlib_peer_listen_fd);
}

int MPD_Man_msgs_fd( void )
{
    return(mpdlib_man_msgs_fd);
}

int MPD_Poke_peer( int jobid, int rank, char *msg )
{
    char buf[MAXLINE];

    sprintf( buf, "cmd=interrupt_peer_with_msg job=%d torank=%d fromrank=%d msg=%s\n",
             jobid, rank, mpdlib_myrank, msg );
    write( mpdlib_man_msgs_fd, buf, strlen(buf) );
    return(0);
}

void MPD_Abort( int code )
{
    int rank, jobid;
    char buf[MAXLINE];
    
    rank   = MPD_Rank();
    jobid  = MPD_Job();
    mpdprintf( 0, "MPD_Abort: process %d aborting with code %d\n", rank, code );

    sprintf( buf, "cmd=abort_job job=%d rank=%d abort_code=%d\n", jobid, rank, code );
    write( mpdlib_man_msgs_fd, buf, strlen(buf) );

    sleep( 20 );
    mpdprintf( 1, "MPD_Abort:  exiting after 20 seconds\n" );  fflush( stderr );
    exit( -1 );
}

int MPD_Get_peer_host_and_port( int job, int rank, char *peerhost, int *peerport )
{
    int i;
    char buf[MAXLINE];
        
    sprintf( buf, "cmd=findclient job=%d rank=%d\n", job, rank );
    write( mpdlib_man_msgs_fd, buf, strlen(buf) );
    i = read_line( mpdlib_man_msgs_fd, buf, MAXLINE );  
    mpdprintf( debug ,"MPDLIB rc=%d reply=>:%s:\n", i, buf );
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
    int rc, numfds, done;
    char buf[MAXLINE];
    struct timeval tv;
    fd_set readfds;

    done = 0;
    while (!done)
    {
        FD_ZERO( &readfds );
        FD_SET( mpdlib_man_msgs_fd, &readfds );
        numfds = mpdlib_man_msgs_fd + 1;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        rc = select( numfds, &readfds, NULL, NULL, &tv );
        if ( ( rc == -1 ) && ( errno == EINTR ) )
            continue;
        if ( rc < 0 )
            error_check( rc, "signal handler: select" );
        if ( FD_ISSET( mpdlib_man_msgs_fd, &readfds ) )
        {
            rc = read_line(mpdlib_man_msgs_fd,buf,MAXLINE);
            mpdprintf( debug, "sigusr1_handler got buf=:%s:\n",buf );
            (*MPD_user_peer_msg_handler)(buf);
        }
        else
            done = 1;
    }
}
