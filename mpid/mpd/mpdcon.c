#include "mpd.h"
#include <sys/param.h>

#define STDIN_STREAM    0
#define STDOUT_STREAM   1
#define STDERR_STREAM   2
#define CONTROL_STREAM  4
#define LISTEN_STREAM   5
#define TEMP_STREAM     6
#define USER_STDIN      7


struct fdentry fdtable[MAXFDENTRIES];  /* for external defn */

int cfd, debug = 0;
int listener_idx = -1;
int ctl_idx = -1;
int stdin_idx = -1;
int stdout_idx = -1;
int stderr_idx = -1;

int done;

int pre_build_print_tree = 1; /* hard-coded for now; pass in on cmd-line later */

char myid[IDSIZE];

int mpdhelp( int, char** );
int mpdcleanup( int, char** );
int mpdtrace( int, char** );
int mpdringtest( int, char** );
int mpdmpexec( int, char** );
int mpdexit( int, char** );
int mpdallexit( int, char** );
int mpdbomb( int, char** );
int mpirun( int, char** );
void handle_listen_input( int );
void handle_control_input( int );
void handle_stdout_input( int );
void handle_stderr_input( int );
void handle_temp_input( int );
void handle_user_stdin( int );
void con_sig_handler( int );

int main( argc, argv )
int argc;
char *argv[];
{
    int rc;
    char *s, pgmname[128], console_name[128];
    struct passwd *pwent;

    if ((s = rindex(argv[0],'/')) == NULL)
	strcpy(pgmname,argv[0]);
    else
	strcpy(pgmname,s+1);
    if ( strcmp( pgmname,"mpdhelp" ) == 0 )
	rc = mpdhelp( argc, argv );
    else if ( strcmp( pgmname,"mpdcleanup" ) == 0 )
	rc = mpdcleanup( argc, argv );
    else
    {
	strcpy( myid, "mpdcon");
	if ((pwent = getpwuid(getuid())) == NULL)
	{
	    printf("getpwuid failed");
	    exit(-1);
	}
	sprintf( console_name, "%s_%s", CONSOLE_NAME, pwent->pw_name );
	mpdprintf( debug, "connecting to console name :%s:\n", console_name );
	cfd = local_connect( console_name );
	if ( cfd == -1 ) {  
	    if ( strcmp( pgmname,"mpirun" ) == 0 )
		cfd = start_mpds( console_name );
	}
	error_check( cfd, "local_connect: " );
	mpdprintf( debug, "local_connect; socket=%d\n", cfd );

	if ( strcmp( pgmname,"mpdringtest" ) == 0 )
	    rc = mpdringtest( argc, argv );
	else if ( strcmp( pgmname,"mpdtrace" ) == 0 )
	    rc = mpdtrace( argc, argv );
	else if ( strcmp( pgmname,"mpddump" ) == 0 )
	    rc = mpddump( argc, argv );
	else if ( strcmp( pgmname,"mpdmpexec" ) == 0 )
	    rc = mpdmpexec( argc, argv );
	else if ( strcmp( pgmname,"mpdexit" ) == 0 )
	    rc = mpdexit( argc, argv );
	else if ( strcmp( pgmname,"mpdallexit" ) == 0 )
	    rc = mpdallexit( argc, argv );
	else if ( strcmp( pgmname,"mpdbomb" ) == 0 )
	    rc = mpdbomb( argc, argv );
	else if ( strcmp( pgmname,"mpirun" ) == 0 )
	    rc = mpirun( argc, argv );
	else {
	    printf( "unrecognized pgm name from console \n" );
	    exit( -1 );
	}
    }
    return(0);
}

int mpdcleanup( argc, argv )
int argc;
char *argv[];
{
    struct passwd *pwent;
    char file_name[MAXLINE];
    char cmd[MAXLINE];
    char cmd2[MAXLINE];

    if ((pwent = getpwuid(getuid())) == NULL)
    {
	printf("mpdcleanup: getpwuid failed");
	exit(99);
    }
    sprintf( file_name, "%s_%s", CONSOLE_NAME, pwent->pw_name );
    sprintf( cmd2, "mpdallexit");
    /* system( cmd2 ); */  /* prone to failure if there is no mpd */
    sprintf( cmd, "/bin/rm -f %s", file_name );
    system( cmd );

    return(0);
}

int mpdringtest(argc,argv)
int argc;
char *argv[];
{
    int  count;
    char buf[MAXLINE];
	
    if (argc < 2)
    {
	printf("usage: mpdringtest count\n" );
	return(0);
    }
    count = atoi( argv[1] );
    if ( count > 0 ) {
        /* send message around ring to self */
        sprintf( buf, "ringtest %d\n", count );
        send_msg( cfd, buf, strlen( buf ) );
    }
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdringtest: msg from mpd: %s", buf );
    read_line( cfd, buf, MAXLINE );  /* get ringtest completed msg */
    printf( "mpdringtest: msg from mpd: %s", buf );
    return(0);
}

int mpddump(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];
    char what_to_dump[80];
	
    if (argc < 2)
	strcpy( what_to_dump, "all" );
    else
	strcpy( what_to_dump, argv[1] );

    sprintf( buf, "dump %s\n", what_to_dump );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug,"mpddump: msg from mpd: %s", buf );
    return(0);
}

int mpdtrace(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];
	
    sprintf( buf, "trace\n" );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdtrace: msg from mpd: %s", buf );
    while ( strcmp( buf, "trace done\n" ) != 0 ) {
        read_line( cfd, buf, MAXLINE );
	if ( strcmp( buf, "trace done\n" ) != 0 )
	    printf( "mpdtrace: %s", buf );
    }
    return(0);
}

int mpdbomb(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];

    if (argc < 2)
    {
	printf( "usage: mpdbomb mpd_id \n" );
	return(0);
    }
    sprintf( buf, "bomb %s\n", argv[1] );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdbomb: msg from mpd: %s", buf );
    return(0);
}

int mpdexit(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];

    if (argc < 2)
    {
	printf( "usage: mpdexit mpd_id \n" );
	return(0);
    }
    sprintf( buf, "exit %s\n", argv[1] );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf(debug,"mpdexit: msg from mpd: %s",buf);
    return(0);
}

int mpdallexit(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];

    if (argc != 1)
    {
	printf( "usage: mpdallexit \n" );
	return(0);
    }
    sprintf( buf, "allexit\n" );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf(debug,"mpdallexit: msg from mpd: %s",buf);
    return(0);
}

int mpdhelp(argc,argv)
int argc;
char *argv[];
{
    printf("\n" );
    printf("mpdhelp\n" );
    printf("  prints this information\n");
    printf("mpdcleanup \n" );
    printf("  deletes unix socket files /tmp/mpd.* if necessary \n");
    printf("mpdtrace\n" );
    printf("  causes each mpd in the ring to respond with \n");
    printf("  a message identifying itself and its neighbors\n");
    printf("mpdringtest count\n" );
    printf("  sends a message around the ring \"count\" times\n");
    printf("mpdexit mpd_id \n" );
    printf("  causes the specified mpd_id to exit gracefully;\n");
    printf("  mpd_id is specified as host_portnum;\n");
    printf("mpdbomb mpd_id \n" );
    printf("  for testing: causes the specified mpd_id to \"fail\";\n");
    printf("  mpd_id is specified as host_portnum\n");
    printf("\n" );
    return(0);
}

/******  This is the console that talks to managers  ****/

int mpdmpexec( argc, argv )
int argc;
char *argv[];
{
    int i, argcnt, envcnt, envflag, loccnt, locflag, rc, num_fds;
    char buf[MAXLINE], argbuf[MAXLINE], stuffed_arg[MAXLINE], wdirname[MAXPATHLEN];
    char path[MAXPATHLEN];
    char myhostname[MAXHOSTNMLEN];
    fd_set readfds, writefds;
    struct timeval tv;
    int path_was_supplied_by_user, user_stdin_idx;

    if (argc < 3)
    {
	printf( "usage: mpdmpexec numprocs executable [args] [-MPDENV- env] [-MPDLOC- loc(s)]\n" );
	return(0);
    }

    init_fdtable();

    /* Set up listener port.  This will be used by the manager with rank 0 to connect
       a control stream and streams for stdin, stdout, and stderr. */

    listener_idx		  = allocate_fdentry();
    fdtable[listener_idx].fd	  = setup_network_socket( &fdtable[listener_idx].portnum);
    fdtable[listener_idx].read	  = 1;
    fdtable[listener_idx].write	  = 0;
    fdtable[listener_idx].handler = LISTEN_STREAM;
    strcpy( fdtable[listener_idx].name, "listener" );

    gethostname( myhostname, MAXHOSTNMLEN );

    getcwd(wdirname,MAXPATHLEN);
    mpdprintf( debug, "current console working directory = %s\n", wdirname );
    strcpy( path, getenv( "PATH" ) ); /* may want to propagate to manager */
    mpdprintf( debug, "current path = %s\n", path );
    sprintf( buf, "mpexec %s %d %d %s %s",
	     myhostname, fdtable[listener_idx].portnum, pre_build_print_tree,
	     argv[1], argv[2] );
    argcnt = 0;
    envcnt = 0;
    loccnt = 0;
    envflag = 0;
    locflag = 0;
    path_was_supplied_by_user = 0;

    if (argc > 3)
    {
	strcat( buf, " " );  /* extra blank before args */
	for (i=3; i < argc; i++)
	{
	    if (strcmp(argv[i],"-MPDENV-") == 0) {
	        envflag = 1;
	        locflag = 0;
	    }
	    else if (strcmp(argv[i],"-MPDLOC-") == 0) {
	        locflag = 1;
	        envflag = 0;
	    }
	    else {
		stuff_arg(argv[i],stuffed_arg);
		if (locflag) {
		    loccnt++;
		    sprintf( argbuf, " loc%d=%s", loccnt, stuffed_arg );
		}
		else if (envflag) {
		    envcnt++;
		    sprintf( argbuf, " env%d=%s", envcnt, stuffed_arg );
		    if ( strncmp( argv[i], "PATH=", 5 ) == 0 )
			path_was_supplied_by_user = 1;
		}
		else {
		    argcnt++;
		    sprintf( argbuf, " arg%d=%s", argcnt, stuffed_arg );
		}
		strcat( buf, argbuf );
	    }
	}
    }
    sprintf( argbuf, " argc=%d", argcnt );  
    strcat( buf, argbuf );
    if ( ! path_was_supplied_by_user ) {
	sprintf( argbuf, "PATH=%s", path );
	stuff_arg(argbuf,stuffed_arg);
	envcnt++;
	sprintf( argbuf, " env%d=%s", envcnt, stuffed_arg );
	strcat( buf, argbuf );
    }
    sprintf( argbuf, "PWD=%s", wdirname );
    stuff_arg(argbuf,stuffed_arg);
    envcnt++;
    sprintf( argbuf, " env%d=%s", envcnt, stuffed_arg );
    strcat( buf, argbuf );

    sprintf( argbuf, " envc=%d", envcnt );  
    strcat( buf, argbuf );
    sprintf( argbuf, " locc=%d", loccnt );  
    strcat( buf, argbuf );

    strcat( buf, "\n" );
    mpdprintf( 0, "CONSOLE: MPEXEC CMD=:%s:\n", buf );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE ); /* get jobid from mpd */
    mpdprintf( 0, "mpdmpexec: msg from mpd: %s", buf );
    /* fprintf( stderr, "%s", buf ); */ 	/* print job id */
    /* dclose( cfd ); */    /* don't close socket to mpd in case we want to have error
                               messages come back during job startup */

    /* put stdin in fdtable */
    user_stdin_idx		    = allocate_fdentry();
    fdtable[user_stdin_idx].fd	    = 0;
    fdtable[user_stdin_idx].read    = 1;
    fdtable[user_stdin_idx].write   = 0;
    fdtable[user_stdin_idx].handler = USER_STDIN;
    strcpy( fdtable[user_stdin_idx].name, "user_stdin" );

    /* Main loop */
    done = 0;
    while ( !done ) {
        FD_ZERO( &readfds );
        FD_ZERO( &writefds );
        for ( i = 0; i < MAXFDENTRIES; i++ )
            if ( fdtable[i].active && fdtable[i].read ) 
		FD_SET( fdtable[i].fd, &readfds );

        num_fds = FD_SETSIZE;
        tv.tv_sec = 3600;
        tv.tv_usec = 0;

        rc = select( num_fds, &readfds, &writefds, NULL, &tv );

        if ( rc == 0 ) {
            mpdprintf( debug, "select timed out after %ld minutes\n", tv.tv_sec/60 );
            continue;
        } 
        if ( ( rc == -1 ) && ( errno == EINTR ) ) {
            mpdprintf( debug, "select interrupted; continuing\n" );
            continue;
        }
        if ( rc < 0 ) {
            done = 1;
            error_check( rc, "console main loop: select" );
        }

        for ( i = 0; i < MAXFDENTRIES; i++ ) {
            if ( fdtable[i].active ) {
                if ( FD_ISSET( fdtable[i].fd, &readfds ) )
                    handle_input_fd( i );
            }
        }
    }
    
    return(0);
}

/******  This is the mpd version of mpirun; it uses mpdmpexec  ****/

int mpirun( argc, argv )
int argc;
char *argv[];
{
    int i;

    if (argc < 4  ||  strcmp( argv[1],"-np" ) != 0 )
    {
	printf( "usage: mpirun -np numprocs executable [args]\n" );
	return(0);
    }
    for( i=1; i < argc-1; i++ )
	argv[i] = argv[i+1];
    argc -= 1;
    mpdmpexec( argc, argv );
}


void handle_input_fd( idx )
int idx;
{
    if ( fdtable[idx].handler == NOTSET )
        mpdprintf( debug, "man:  handler not set for port %d\n", idx );
    else if ( fdtable[idx].handler == LISTEN_STREAM )
        handle_listen_input( idx );
    else if ( fdtable[idx].handler == TEMP_STREAM )
        handle_temp_input( idx );
    else if ( fdtable[idx].handler == CONTROL_STREAM )
        handle_control_input( idx );
    else if ( fdtable[idx].handler == STDOUT_STREAM)
        handle_stdout_input( idx );
    else if ( fdtable[idx].handler == STDERR_STREAM )
        handle_stderr_input( idx );
    else if ( fdtable[idx].handler == STDIN_STREAM )
	handle_stdin_input( idx );
    else if ( fdtable[idx].handler == USER_STDIN )
        handle_user_stdin( idx );
    else
        mpdprintf( debug, "invalid handler for fdtable entry %d\n", idx );
}

void handle_listen_input( int idx )
{
    int tmp_idx;

    mpdprintf( debug, "console: handling listen input, accept here\n" ); 
    tmp_idx = allocate_fdentry();
    fdtable[tmp_idx].fd      = accept_connection( fdtable[idx].fd );
    fdtable[tmp_idx].handler = TEMP_STREAM;
    fdtable[tmp_idx].read    = 1;
}

void handle_temp_input( int idx )
{
    int  length;
    char message[MAXLINE], tmpbuf[MAXLINE], cmd[MAXLINE];

    if ( ( length = read_line(fdtable[idx].fd, message, MAXLINE ) ) != 0 )
        mpdprintf( debug, "message from manager to handle = :%s:\n", message );
    else {
        mpdprintf( 1, "console failed to retrieve msg on conn to listener\n");
	return;
    }
    strcpy( tmpbuf, message );             
    parse_keyvals( tmpbuf );
    getval( "cmd", cmd );
    if ( strcmp( cmd, "new_ctl_stream" ) == 0 ) {
        ctl_idx = idx;
	fdtable[ctl_idx].handler = CONTROL_STREAM;
	fdtable[ctl_idx].read = 1;
	strcpy( fdtable[ctl_idx].name, "ctl_stream" );
	/* control connection now open, so set up to pass interrupts to manager */
	Signal( SIGTSTP,  con_sig_handler );  /* Pass suspension to manager */
	Signal( SIGCONT,  con_sig_handler );  /* Pass cont to manager  */
	Signal( SIGINT,   con_sig_handler );  /* Pass kill to manager  */
    }
    else if ( strcmp( cmd, "new_stdin_stream" ) == 0 ) {
        stdin_idx = idx;
	fdtable[stdin_idx].handler = STDIN_STREAM;
	fdtable[stdin_idx].read = 1; /* in order to detect EOF when manager dies */
	strcpy( fdtable[stdin_idx].name, "stdin_stream" );
    }
    else if ( strcmp( cmd, "new_stdout_stream" ) == 0 ) {
        stdout_idx = idx;
	fdtable[stdout_idx].handler = STDOUT_STREAM;
	fdtable[stdout_idx].read = 1;
	strcpy( fdtable[stdout_idx].name, "stdout_stream" );
    }
    else if ( strcmp( cmd, "new_stderr_stream" ) == 0 ) {
        stderr_idx = idx;
	fdtable[stderr_idx].handler = STDERR_STREAM;
	fdtable[stderr_idx].read = 1;
	strcpy( fdtable[stderr_idx].name, "stderr_stream" );
    }
    else {
        mpdprintf( 1, "unrecognized msg to console's listener = :%s:\n",cmd );
    }
}

void handle_control_input( int idx )
{
    int length;
    char buf[MAXLINE];
    char cmd[80];

    if ( ( length = read_line(fdtable[idx].fd, buf, MAXLINE ) ) > 0 ) {
        mpdprintf( debug, "console received on control from manager: :%s:\n", buf );
	parse_keyvals( buf );
	getval( "cmd", cmd );
	if ( strcmp( cmd, "jobdead" ) == 0 ) {
	    mpdprintf( debug, "handle_control_input writing\n");
	    sprintf( buf, "cmd=allexit\n" );
	    write_line( ctl_idx, buf );
	    mpdprintf( debug, "parallel job exited\n" );
	    /* exit( 0 ); */ /* hang around until manager 0 ends */
	}
	else
	    mpdprintf( 1, "unrecognized message from job manager\n" );
    }
    else if ( length == 0 ) {
	mpdprintf( debug, "eof on cntl input\n" );
	exit( 0 );
    }
    else
        mpdprintf( 1, "console failed to retrieve msg from control stream\n" );
}

void handle_stdout_input( int idx )
{
    int n;
    char buf[STREAMBUFSIZE];

    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	mpdprintf( debug, "handle_stdout_input writing\n");
	write( 1, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( debug, "console received eof on stdout from manager\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
    }
    else
	fprintf( stderr, "console failed to retrieve msg from stdout stream\n" );
}

void handle_stderr_input( int idx )
{
    int n;
    char buf[STREAMBUFSIZE];

    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	mpdprintf( debug, "handle_stderr_input writing\n");
	write( 2, buf, n );
    }
    else  if ( n == 0 ) {
	mpdprintf( debug, "console received eof on stderr from manager\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
    }
    else
	fprintf( stderr, "console failed to retrieve msg from stderr stream\n" );
}

void handle_stdin_input( int idx )
{
    int n;
    char buf[STREAMBUFSIZE];

    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	mpdprintf( 1,
		   "console received unexpected input from manager on stdin_out: :%s:\n",
		   buf );
    }
    else {
	/* manager 0 has closed stdin, so we should not pass stdin through to him */
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	stdin_idx = -1;
    }
}

void handle_user_stdin( int idx )
{
    int n;
    char buf[STREAMBUFSIZE], buf2[STREAMBUFSIZE];
    
    if ( ( n = read_line( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 )
	if ( strncmp( buf, "_", 1 ) == 0 ) { /* escape character to access cntl */
	    sprintf( buf2, "cmd=%s", buf + 1 );
	    mpdprintf( debug, "handle_user_stdin writing\n");
	    write_line( ctl_idx, buf2 );
	}
	else {
	    if ( stdin_idx != -1 ) {
		mpdprintf( debug, "handle_user_stdin doing send_msg\n");
		send_msg( fdtable[stdin_idx].fd, buf, n );
	    }
	}
    else if ( n == 0 ) {
	mpdprintf( debug, "console got EOF on its stdin\n" );
	dclose( fdtable[idx].fd ); /* console's own stdin */
	deallocate_fdentry( idx );
	/* close input connections to manager */
	dclose( fdtable[stdin_idx].fd ); 
	deallocate_fdentry( stdin_idx );
	stdin_idx = -1;
    }
    else 
	fprintf( stderr, "console failed to retrieve msg from console's stdin\n" );
}

void con_sig_handler( int signo )
{
    char buf[MAXLINE], signame[24];
    int pid;

    unmap_signum( signo, signame );
    mpdprintf( debug, "Console got signal %d (%s)\n", signo, signame );

    if ( signo == SIGTSTP ) {
	mpdprintf( debug, "parallel job suspended\n" );
	sprintf( buf, "cmd=signal signo=%s\n", "SIGTSTP" );
	write_line( ctl_idx, buf );
	/* suspend self*/
	Signal( SIGTSTP, SIG_DFL );  /* Set interrupt handler to default */
	pid = getpid();
	kill( pid, SIGTSTP );
    }
    else if ( signo == SIGCONT ) {
	mpdprintf( debug, "parallel job resumed\n" );
	sprintf( buf, "cmd=signal signo=%s\n", "SIGCONT" );
	write_line( ctl_idx, buf );
	Signal( SIGTSTP, con_sig_handler );   /* Restore this signal handler */
    }
    else if ( signo == SIGINT ) {
	mpdprintf( debug, "parallel job killed\n" );
	sprintf( buf, "cmd=signal signo=%s\n", "SIGINT" );
	write_line( ctl_idx, buf );
	dclose( fdtable[ctl_idx].fd );
	exit( -1 );
    }
    else {
	mpdprintf( 1, "unknown signal %d (%s)\n", signo, signame );
    }
}

int start_mpds( name )
char *name;

{
    char cmd[50]; 

    sprintf(cmd, "startdaemons 5" );
    system( cmd );

    cfd = local_connect( name );
    return cfd; 
    
}
