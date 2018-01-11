/*

MPID daemon - invoke original one with no args.

Serves three kinds of socket connections:

 a)  A UNIX domain socket to an mpich "console"
 b)  An INET domain socket available for random connections
 c)  A set of INET domain sockets established to other MPICH daemons.
     Currently this set consists of a "next" and "prev" connection, and
     mpd's will be connected in a ring, with next an output port and
     prev an input port.  For the first mpd the output is connected
     to the input
 d)  A possible set of connections to "client" processes on local machine.
*/

#include "mpd.h"

/* for command line argument processing */
extern char *optarg;
extern int  optind;
int         opt;

struct fdentry fdtable[MAXFDENTRIES];
extern int fdtable_high_water_mark;

extern void     sigint_handler( int );
extern Sigfunc  *Signal( int, Sigfunc * );

char mydir[MAXLINE];
char lhshost[MAXHOSTNMLEN] = {'\0'};
int  lhsport = -1;                
char rhshost[MAXHOSTNMLEN];        
int  rhsport = -1;                
char rhs2host[MAXHOSTNMLEN];        
int  rhs2port = -1;
char myhostname[MAXHOSTNMLEN];
char mynickname[MAXHOSTNMLEN];
int  my_listener_port = 0;	/* might be set on command line, else bind chooses */
char console_name[MAXLINE];
char logfile_name[MAXLINE];

int logfile_idx          = -1;
int listener_idx	 = -1;
int console_listener_idx = -1;   
int console_idx		 = -1;   
int client_listener_idx	 = -1;   
int manager_listener_idx = -1;   
int client_idx		 = -1;   
int lhs_idx		 = -1;
int rhs_idx		 = -1;
int my_listener_fd	 = -1;
int done		 = 0;
int debug		 = 0;
int amfirst		 = 1; /* overwritten below if host is on command line */
int allexiting		 = 0; /* flag to disable auto reconnect when all mpds are exiting*/
int backgrounded	 = 0; /* flag to indicate I should become a daemon */

char myid[IDSIZE];            /* myid is hostname_listener-portnum */
char mylongid[IDSIZE];
char mpd_passwd[PASSWDLEN];

/* jobid data */
int first_avail, last_avail, first_pool, last_pool;

extern struct keyval_pairs keyval_tab[64];
extern int keyval_tab_idx;

int main( int argc, char *argv[] )
{
    int  allow_console = 0;
    char in_buf[MAXLINE], out_buf[MAXLINE], cmd[MAXLINE];
    int  rc, num_fds, i;
    struct timeval tv;
    struct passwd *pwent;

    fd_set readfds, writefds;

    Signal( SIGINT,  sigint_handler );  /* Cleanup upon SIGINT  */
    Signal( SIGTERM, sigint_handler );  /* Cleanup upon SIGTERM */
    Signal( SIGSEGV, sigint_handler );  /* Cleanup upon SIGSEGV */
    Signal( SIGBUS,  sigint_handler );  /* Cleanup upon SIGBUS */
    Signal( SIGCHLD, sigchld_handler ); /* Cleanup upon SIGCHLD */
    Signal( SIGUSR1, sigusr1_handler ); /* Complain upon SIGUSR1 */

#ifdef ROOT_ENABLED
    fprintf( stderr, "mpd configured to run as root\n" );
#endif

    allow_console = 1;   /* allows a console by default*/
    while ( ( opt = getopt( argc, argv, "cp:nh:?d:w:l:b" ) ) != EOF ) {
        switch ( opt ) {
        case 'w':
            chdir( optarg );             break;
        case 'h':
            amfirst = 0;
            strcpy( lhshost, optarg );   break;
        case 'p':
            amfirst = 0;
            lhsport = atoi( optarg );    break;
        case 'c':
            allow_console = 1;           break;
        case 'n':
            allow_console = 0;           break;
        case 'd':
            debug = atoi( optarg );      break;
	case 'l':
	    my_listener_port = atoi( optarg ); break;
	case 'b':
            backgrounded = 1;            break;
        case '?':
            usage(argv[0]);              break;
        default:
            usage(argv[0]);
        }
    }

    /* get password from file */
    if ( get_local_pw( mpd_passwd, PASSWDLEN ) < 0 )
	exit( -1 );

    /* Record information about self */
    my_listener_fd = setup_network_socket( &my_listener_port );
    getcwd( mydir, MAXLINE );
    gethostname( myhostname, MAXHOSTNMLEN );
    sprintf( mylongid, "%s_%d", myhostname, my_listener_port );
    strcpy( mynickname, myhostname );
    strtok( mynickname, "." );
    sprintf( myid, "%s_%d", mynickname, my_listener_port );

    mpdprintf( 0, "MPD starting\n");  /* first place with a valid id */

    if ( ( !amfirst ) && ( lhsport == -1 || lhshost[0] == '\0' ) ) {
	mpdprintf( 1, "must specify both host and port or else neither\n" );
	exit( -1 );
    }
    
    init_fdtable();
    init_jobtable();
    init_proctable();

    /* set up listener fd */
    listener_idx                  = allocate_fdentry();
    fdtable[listener_idx].read    = 1;
    fdtable[listener_idx].write   = 0;
    fdtable[listener_idx].handler = LISTEN;
    fdtable[listener_idx].fd      = my_listener_fd;
    fdtable[listener_idx].portnum = my_listener_port;
    strcpy( fdtable[listener_idx].name, "listener" );

    if ((pwent = getpwuid(getuid())) == NULL)
    {
	mpdprintf( 1, "mpd: getpwuid failed" );
	exit( -1 );
    }

    /* set up console fd */
    if ( allow_console ) {
        console_listener_idx                  = allocate_fdentry();
        fdtable[console_listener_idx].read    = 1;
        fdtable[console_listener_idx].write   = 0;
        fdtable[console_listener_idx].handler = CONSOLE_LISTEN;
        /* sprintf( console_name, "%s_%d", CONSOLE_NAME, my_listener_port ); */
        sprintf( console_name, "%s_%s", CONSOLE_NAME, pwent->pw_name );
        strcpy( fdtable[console_listener_idx].name, console_name );
        fdtable[console_listener_idx].fd = setup_unix_socket( console_name );  
	if ( fdtable[console_listener_idx].fd < 0 )  {
	    mpdprintf( 1," mpd setup_unix_socket failed to setup console\n" );
	    exit( -1 );
	}
    }

    /* first mpd is own lhs */
    if ( amfirst ) {
        strcpy( lhshost, mynickname );
        lhsport = fdtable[listener_idx].portnum;
        init_jobids();		/* protected from executing twice */
    }

    /* set up left-hand side fd */
    lhs_idx                  = allocate_fdentry();
    fdtable[lhs_idx].read    = 1;
    fdtable[lhs_idx].write   = 0;
    fdtable[lhs_idx].handler = LHS;
    fdtable[lhs_idx].fd      = network_connect( lhshost, lhsport );
    fdtable[lhs_idx].portnum = lhsport;
    strcpy( fdtable[lhs_idx].name, lhshost );

    /* Send message to lhs, telling him to treat me as his new rhs */
    sprintf( out_buf, "dest=%s_%d cmd=new_rhs_req host=%s port=%d\n",
             lhshost, lhsport, mynickname, my_listener_port ); 
    mpdprintf( 0, "sending to lhs: %s", out_buf );        
    write_line( lhs_idx, out_buf );
    if ( ! amfirst ) {
	recv_msg( fdtable[lhs_idx].fd, in_buf );
	strcpy( out_buf, in_buf );
	parse_keyvals( out_buf );
	getval( "cmd", cmd );
	if ( strcmp( cmd, "challenge" ) != 0 ) {
	    mpdprintf( 1, "expecting challenge, got %s\n", in_buf );
	    exit( -1 );
	}
	newconn_challenge( lhs_idx );
    }

    /* set up right_hand side fd */
    if ( amfirst ) {
        strcpy( rhshost, mynickname );
        rhsport = my_listener_port;
        /* set up "next-next" */
        strcpy( rhs2host, mynickname );
        rhs2port = my_listener_port;
        /* accept connection from self, done in "set up lhs fd" above */
        rhs_idx                  = allocate_fdentry();
        fdtable[rhs_idx].read    = 1;
        fdtable[rhs_idx].write   = 0;
        fdtable[rhs_idx].handler = RHS;
        fdtable[rhs_idx].fd      = accept_connection( fdtable[listener_idx].fd );
        fdtable[rhs_idx].portnum = rhsport;
        strcpy( fdtable[rhs_idx].name, rhshost );
        recv_msg( fdtable[rhs_idx].fd, in_buf );
        /* check that it worked */
        if ( strncmp( in_buf, out_buf, strlen( out_buf ) ) ) {
             mpdprintf( 1, "initial test message to self failed!\n" );
             exit( -1 );
        }
    }
    else
    {
        /* If not first, then rhs will be set up later in response
	 * to a message from our lhs, telling us whom to connect to
	 * on the right.  Get ready for that.
	 */
    }

    /* put myself in the background if flag is set */
    if ( backgrounded )
    {
        if ( fork() != 0 )  /* parent exits; child in background */
	    exit( 0 );
	setsid();           /* become session leader; no controlling tty */
	Signal( SIGHUP, SIG_IGN ); /* make sure no sighup when leader ends */
	/* leader exits; svr4: make sure do not get another controlling tty */
        if ( fork() != 0 )  
	    exit( 0 );
	chdir("/");         /* free up filesys for umount */
	umask(0);
	/* openlog( argv[0], LOG_PID, facility ); */  /* to use syslog if we want */

	/* create a logfile entry just for cleanup */
        logfile_idx                  = allocate_fdentry();
        fdtable[logfile_idx].read    = 0;    /* do not select on this for rd or wt */
        fdtable[logfile_idx].write   = 0;    /*   used mostly for cleanup */
        fdtable[logfile_idx].handler = LOGFILE_OUTPUT;
        fdtable[logfile_idx].fd      = 1;   /* stdout */
        sprintf( logfile_name, "%s_%s", LOGFILE_NAME, pwent->pw_name );
        strcpy( fdtable[logfile_idx].name, logfile_name );
        freopen( logfile_name, "a", stdout );
        freopen( logfile_name, "a", stderr );
	close( 0 );
    }


    /* Main Loop */
    mpdprintf( debug, "entering main loop\n" );
    while ( !done ) {
        FD_ZERO( &readfds );
        FD_ZERO( &writefds );
        for ( i = 0; i <= fdtable_high_water_mark; i++ ) {
            if ( fdtable[i].active ) {
                mpdprintf( 0, "active fd:%s,fd=%d\n",
                         fdtable[i].name,fdtable[i].fd);
                if ( fdtable[i].read )
		{
                    FD_SET( fdtable[i].fd, &readfds );
		}
		/*****
                if ( fdtable[i].write )
                    FD_SET( fdtable[i].fd, &writefds );
		*****/
            }
        }

        num_fds = FD_SETSIZE;
        tv.tv_sec = 3600;
        tv.tv_usec = 0;

        rc = select( num_fds, &readfds, &writefds, NULL, &tv );

        if ( rc == 0 ) {
            mpdprintf( debug, "select timed out after %ld minutes\n", 
                        tv.tv_sec/60 );
            continue;
        } 
        if ( ( rc == -1 ) && ( errno == EINTR ) ) {
            mpdprintf( debug, "select interrupted; continuing\n" );
            continue;
        }
        if ( rc < 0 ) {
            done = 1;
            error_check( rc, "mpd main loop: select" );
        }

        for ( i = 0; i <= fdtable_high_water_mark; i++ ) {
            if ( fdtable[i].active ) {
                if ( FD_ISSET( fdtable[i].fd, &readfds ) )
                    handle_input_fd( i );
		/*****
                if ( FD_ISSET( fdtable[i].fd, &writefds ) )
                   handle_output( i );
		*****/
            }
        }
    }
    if ( debug )
	dump_fdtable( "at exit from mpd" );
    mpd_cleanup( );
    return( 0 );
}

void handle_input_fd( idx )
int idx;
{
    mpdprintf( 0, "handle_input_fd: lhs=%s %d rhs=%s %d rhs2=%s %d\n",
            lhshost, lhsport, rhshost, rhsport, rhs2host, rhs2port );
    if ( fdtable[idx].handler == NOTSET )
        mpdprintf( debug, "handler not set for port %d\n", idx );
    else if ( fdtable[idx].handler == CONSOLE_LISTEN )
        handle_console_listener_input( idx );
    else if ( fdtable[idx].handler == MANAGER )
        handle_manager_input( idx );
    else if ( fdtable[idx].handler == CONSOLE )
        handle_console_input( idx );
    else if ( fdtable[idx].handler == LISTEN )
        handle_listener_input( idx );
    else if ( fdtable[idx].handler == NEWCONN )
        handle_newconn_input( idx );
    else if ( fdtable[idx].handler == LHS )
        handle_lhs_input( idx );
    else if ( fdtable[idx].handler == RHS )
        handle_rhs_input( idx );
    else
        mpdprintf( debug, "invalid handler for fdtable entry %d\n", idx );
}


void init_jobids()
{
    char buf[MAXLINE];
    static int first_call = 1;

    if ( first_call ) {
	if ( amfirst ) {
	    first_avail = 1;
	    last_avail  = CHUNKSIZE;
	    first_pool  = CHUNKSIZE + 1;
	    last_pool   = 2000 * BIGCHUNKSIZE;
	}
	else {
	    first_avail = 0;
	    last_avail  = -1;
	    first_pool  = 0;
	    last_pool   = -1;
	    sprintf( buf, "src=%s dest=anyone cmd=needjobids\n", myid  ); 
	    mpdprintf( 0, "init_jobids: sending needjobids\n" );
	    write_line( rhs_idx, buf );
	}
	first_call = 0;
    }
}

int allocate_jobid()
{
    char buf[MAXLINE];
    int  new_jobid;

    if ( first_avail <= last_avail )   /* ids are available */
	new_jobid = first_avail++;
    else if ( first_pool + CHUNKSIZE - 1 <= last_pool ) { /* avail empty, but not pool */
	first_avail = first_pool;
	last_avail  = first_avail + CHUNKSIZE - 1;
	first_pool  = first_pool + CHUNKSIZE;
	mpdprintf( 0, "after allocating to avail from pool, fp=%d, lp=%d, fa=%d, la=%d\n",
		   first_pool, last_pool, first_avail, last_avail );
	if ( first_pool > last_pool ) {	        /* pool now empty, request more jobids */
	    sprintf( buf, "src=%s dest=anyone cmd=needjobids\n", myid );
	    mpdprintf( 0, "allocate_jobid: sending needjobids\n" );
	    write_line( rhs_idx, buf );
	}
	new_jobid = first_avail++;
    }
    else {
	mpdprintf( 1, "PANIC: could not allocate jobid\n" );
	new_jobid = -1;
    }
    return new_jobid;
}

void add_jobids( first, last )
int first, last;
{
    mpdprintf( 0, "received new jobids: first=%d, last=%d\n", first, last );
    first_pool = first;
    last_pool  = last;
}

int steal_jobids( first, last )
int *first, *last;
{
    if ( last_pool >= first_pool + 2 * BIGCHUNKSIZE ) {
	*first = first_pool;
	*last  = first_pool + BIGCHUNKSIZE - 1;
	first_pool = first_pool + BIGCHUNKSIZE;
	mpdprintf( 0, "after stealing jobids: first_pool=%d, last_pool=%d\n",
		   first_pool, last_pool );
	return 0;
    }
    else
	return -1;
}

int get_local_pw( char *passwd, int len )
{
    char *homedir, passwd_pathname[MAXLINE];
    struct stat statbuf;
    int n, fd;

    if ( ( homedir = getenv( "HOME" ) ) == NULL ) {
	mpdprintf( 1, "get_local_pw: unable to obtain pathname for home directory\n" );
	return( -1 );
    }
#ifdef ROOT_ENABLED
    strcpy( passwd_pathname, "/etc/mpdpasswd" );
#else
    sprintf( passwd_pathname, "%s/.mpdpasswd", homedir );
#endif
    if ( lstat( passwd_pathname, &statbuf ) != 0 ) {
	mpdprintf( 1, "get_local_pw: unable to stat %s\n", passwd_pathname );
	return( -1 );
    }
    if ( statbuf.st_mode & 00077 ) {  /* if anyone other than owner  can access the file */
	mpdprintf( 1, "get_local_pw: other users can access %s\n", passwd_pathname );
	return( -1 );
    }
    if ( ( fd = open( passwd_pathname, O_RDONLY ) ) == -1 ) {
	mpdprintf( 1, "get_local_pw: cannot open %s\n", passwd_pathname );
	return( -1 );
    }
    if ( (n = read( fd, passwd, len ) ) == -1 ) {
	mpdprintf( 1, "get_local_pw: failed to read passwd from %s\n", passwd_pathname );
	return( -1 );
    }

    if ( n < PASSWDLEN )
	if ( passwd[n-1] == '\n' )
	    passwd[n-1] = '\0';
	else
	    passwd[n] = '\0';
    else
	passwd[n-1] = '\0';

    return( 0 );
}

void encode_num( int rn, char *buf )
{
    char tempbuf[PASSWDLEN+32];

    sprintf( tempbuf, "%s%d", mpd_passwd, rn );
    strcpy( buf, crypt( tempbuf, "el" ) );
}
