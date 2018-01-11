#include "mpdman.h"
#include "mpd.h"

struct fdentry fdtable[MAXFDENTRIES];

int done;
int lhs_idx = -1;
int rhs_idx = -1;
int parent_mpd_idx = -1;
int client_listener_idx = -1;
int client_idx = -1;
int myrank, jobsize;
int stdout_pipe_fds[2], stdin_pipe_fds[2], stderr_pipe_fds[2];
int my_listener_port;
int client_listener_port; 
int client_fd, client_pid, client_stat;
int client_listener_fd;
int client_state = CLNONE;
int con_cntl_fd, con_stdin_fd, con_stdout_fd, con_stderr_fd;
int con_cntl_idx = -1, con_stdin_idx = -1, con_stdout_idx = -1, con_stderr_idx = -1;
int client_cntl_idx = -1, client_stdin_idx = -1, client_stdout_idx = -1;
int parent_in_tree = -1, lchild_in_tree = -1, rchild_in_tree = -1;
int client_stderr_idx = -1;
int jobsync_is_here = 0;
int jobdeadsync_is_here = 0;
int parent_in_tree_port = -1;
int allexit_received = 0;
int man_client_msgs_fds[2];
int pre_build_print_tree;
char parent_in_tree_hostname[MAXHOSTNMLEN];
char myhostname[MAXHOSTNMLEN];

extern int debug;
extern char myid[IDSIZE];

int main( int argc, char *argv[], char *envp[] )
{
    int i, j, rc, num_fds, pid;
    struct timeval tv;
    fd_set readfds, writefds;
    char buf[MAXLINE];
    char client_pgm_name[MAXLINE];
    int  listener_fd, listener_idx, conport;
    int  mpd_man_fd;
    char conhost[MAXHOSTNMLEN];
    char left_peer_host[MAXHOSTNMLEN], right_peer_host[MAXHOSTNMLEN];
    int  left_peer_listener_port, right_peer_listener_port;
    char *client_env[50];
    char  env_man_client_listener[MAXLINE];
    char  env_client_listener_fd[MAXLINE];
    char  env_path_for_exec[MAXLINE];
    char  pwd_for_exec[MAXLINE];
    struct passwd *pwent;
    struct sockaddr_in sin;
    int sinlen = sizeof( sin );
    char cmd[MAXLINE];
    int optval = 1;

    sprintf( myid, "man_%s", getenv( "MPD_JRANK" ) );
    mpdprintf( debug, "manager starting\n" );

    myrank = atoi( getenv( "MPD_JRANK" ) );
    jobsize = atoi( getenv( "MPD_JSIZE" ) );

    if ((pwent = getpwuid(getuid())) == NULL) {
	printf("mpd: getpwuid failed");
	exit(99);
    }

    strcpy( client_pgm_name, getenv( "MAN_CLIENT_PGM" ) );
    listener_fd = atoi( getenv( "MAN_LISTENER_FD" ) ); 
    mpd_man_fd = atoi( getenv( "MAN_MPD_FD" ) );
    strcpy( left_peer_host, getenv( "MAN_PREVHOST" ) );
    left_peer_listener_port  = atoi( getenv( "MAN_PREVPORT" ) );
    strcpy( right_peer_host, getenv( "MAN_HOST0" ) );
    right_peer_listener_port = atoi( getenv( "MAN_PORT0" ) );
    strcpy( conhost, getenv( "MAN_CONHOST" ) );
    conport = atoi( getenv( "MAN_CONPORT" ) );
    pre_build_print_tree = atoi( getenv( "MAN_PRE_BUILD_PRINT_TREE" ) );
    debug = atoi( getenv( "MAN_DEBUG" ) );
    /* debug = 1; */

    if ( debug ) 
	for ( i=0; envp[i]; i++ ) 
	    mpdprintf( 1, " man envp[%d] = %s \n", i, envp[i] );

    init_fdtable();
    init_proctable();

    gethostname( myhostname, MAXHOSTNMLEN );

    /* Set up listener port.  The fd has been acquired by the mpd before the manager is
       created, and comes in as the first command-line argument. This will be an 
       all-purpose listener;  it will listen for any type of connection */

    listener_idx		  = allocate_fdentry();
    fdtable[listener_idx].fd	  = listener_fd;
    fdtable[listener_idx].read	  = 1;
    fdtable[listener_idx].write	  = 0;
    rc = getsockname( fdtable[listener_idx].fd, (struct sockaddr *) &sin, &sinlen ); 
    my_listener_port = ntohs(sin.sin_port);
    fdtable[listener_idx].portnum = my_listener_port;
    fdtable[listener_idx].handler = MAN_LISTEN;
    strcpy( fdtable[listener_idx].name, "listener" );
    mpdprintf( debug, "manager has listener on port %d\n", my_listener_port );
    mpdprintf( debug, "manager has prebuildflag=%d\n", pre_build_print_tree );

    /* Set up port to parent mpd.  This should be replaced by a pipe */

    parent_mpd_idx                  = allocate_fdentry();
    fdtable[parent_mpd_idx].fd	    = mpd_man_fd;
    fdtable[parent_mpd_idx].read    = 1;
    fdtable[parent_mpd_idx].write   = 0;
    fdtable[parent_mpd_idx].portnum = -1;
    fdtable[parent_mpd_idx].handler = PARENT_MPD_MSGS;
    strcpy(fdtable[parent_mpd_idx].name, "parent_mpd_msgs" );

    /* Set up port to lhs; host and port are from env, dummy for rank 0 */
    if ( strcmp( left_peer_host, DUMMYHOSTNAME ) != 0 ) {
	lhs_idx                  = allocate_fdentry();
	fdtable[lhs_idx].fd      = network_connect( left_peer_host,
						    left_peer_listener_port );
	fdtable[lhs_idx].read    = 1;
	fdtable[lhs_idx].write   = 0;
	fdtable[lhs_idx].portnum = left_peer_listener_port;
	fdtable[lhs_idx].handler = LHS_MSGS;
	strcpy(fdtable[lhs_idx].name, "lhs_msgs" );
        mpdprintf( debug, "manager has conn to lhs\n" );
    }

    /* Set up port to rhs; host and port are from env, dummy for all ranks
       except numprocs-1 */
    if ( strcmp( right_peer_host, DUMMYHOSTNAME ) != 0 ) {
	rhs_idx                  = allocate_fdentry();
	mpdprintf( debug, "INIT: set up rhs to %s %d on idx=%d\n",
		   right_peer_host, right_peer_listener_port, rhs_idx );
	fdtable[rhs_idx].fd      = network_connect( right_peer_host,
						    right_peer_listener_port );
	fdtable[rhs_idx].read    = 1;
	fdtable[rhs_idx].write   = 0;
	fdtable[rhs_idx].portnum = right_peer_listener_port;
	fdtable[rhs_idx].handler = RHS_MSGS;
	strcpy(fdtable[rhs_idx].name, "rhs_msgs" );
        mpdprintf( debug, "manager has conn to rhs\n" );
    }
	
    /* At this point the ring consists of one-way connections;  All processes except 0
       have a lhs;  process numprocs-1 has both a lhs and a rhs; process 0 has neither.
       Next we have to convert each link to a both-ways connection, by sending messages
       on the links we have */

    if ( lhs_idx != -1 ) {	/* I have a connection to my left peer (all except 0) */
	sprintf( buf, "cmd=new_man_rhs\n" );
        mpdprintf( debug, "sending to lhs, buf=:%s:\n",buf );
	write_line( lhs_idx, buf );
    }

    if ( rhs_idx != -1 ) {	/* I have a connection to my right peer (only numprocs-1)*/
	sprintf( buf, "cmd=new_man_lhs\n" );
        mpdprintf( debug, "sending to rhs, buf=:%s:\n",buf );
	write_line( rhs_idx, buf );
    }


    /* Manager with rank 0 sets up connections to console for I/O and control streams */
    /* Other managers join in a tree up to 0, which forwards on to the console */
    if ( myrank == 0 ) {
	jobsync_is_here = 1;

	con_stdout_idx		      = allocate_fdentry();
	fdtable[con_stdout_idx].fd    = network_connect( conhost, conport );
	setsockopt( fdtable[con_stdout_idx].fd, IPPROTO_TCP, TCP_NODELAY,
		    (char *) &optval, sizeof(optval) );
	fdtable[con_stdout_idx].read  = 0;
	fdtable[con_stdout_idx].write = 1;
	strcpy(fdtable[con_stdout_idx].name, "con_stdout" );
	sprintf( buf, "cmd=new_stdout_stream\n" );
	write_line( con_stdout_idx, buf );
        mpdprintf( debug, "manager has conn to con_stdout\n" );

	con_stderr_idx		      = allocate_fdentry();
	fdtable[con_stderr_idx].fd    = network_connect( conhost, conport );
	setsockopt( fdtable[con_stderr_idx].fd, IPPROTO_TCP, TCP_NODELAY,
		    (char *) &optval, sizeof(optval) );
	fdtable[con_stderr_idx].read  = 0;
	fdtable[con_stderr_idx].write = 1;
	strcpy(fdtable[con_stderr_idx].name, "con_stderr" );
	sprintf( buf, "cmd=new_stderr_stream\n" );
	write_line( con_stderr_idx, buf );
        mpdprintf( debug, "manager has conn to con_stderr\n" );

	con_stdin_idx		      = allocate_fdentry();
	fdtable[con_stdin_idx].fd     = network_connect( conhost, conport );
	fdtable[con_stdin_idx].read   = 1;
	fdtable[con_stdin_idx].write  = 0;
	fdtable[con_stdin_idx].handler = CON_STDIN;
	strcpy(fdtable[con_stdin_idx].name, "con_stdin" );
	sprintf( buf, "cmd=new_stdin_stream\n" );
	write_line( con_stdin_idx, buf );
        mpdprintf( debug, "manager has conn to con_stdin\n" );

	con_cntl_idx		      = allocate_fdentry();
	fdtable[con_cntl_idx].fd      = network_connect( conhost, conport );
	fdtable[con_cntl_idx].read    = 1;
	fdtable[con_cntl_idx].write   = 1;
	strcpy(fdtable[con_cntl_idx].name, "con_cntl" );
	fdtable[con_cntl_idx].handler = CON_CNTL;
	sprintf( buf, "cmd=new_ctl_stream\n" );
	write_line( con_cntl_idx, buf );
        mpdprintf( debug, "CON_CNTL fd=%d\n",fdtable[con_cntl_idx].fd );
    }	

    man_compute_nodes_in_print_tree(myrank,jobsize,&parent_in_tree,&lchild_in_tree,&rchild_in_tree);
    mpdprintf( debug, "parent=%d lchild=%d rchild=%d\n", 
	       parent_in_tree, lchild_in_tree, rchild_in_tree );

    /* Here start the client, redirecting I/O */
    pipe(stdin_pipe_fds);   /* to deliver stdin  to   client */
    pipe(stdout_pipe_fds);  /* to capture stdout from client */
    pipe(stderr_pipe_fds);  /* to capture stderr from client */

    socketpair( AF_UNIX, SOCK_STREAM, 0, man_client_msgs_fds );
    client_idx = allocate_fdentry();
    strcpy( fdtable[client_idx].name, "client" );
    fdtable[client_idx].fd      = man_client_msgs_fds[0];
    fdtable[client_idx].read    = 1;
    fdtable[client_idx].write   = 0;
    fdtable[client_idx].handler = MAN_CLIENT;

    pid = fork();
    error_check( pid, "manager couldn't fork client" );
    if (pid == 0) {             /* child (client) process */
	sprintf( myid, "cli_%d", myrank );
	mpdprintf( debug, "client is alive and about to redirect io\n" );

	dclose(0); 		/* close stdin */
	dup(stdin_pipe_fds[0]);
	dclose(stdin_pipe_fds[0]);
	dclose(stdin_pipe_fds[1]);

	dclose(1);		/* close stdout */
	dup(stdout_pipe_fds[1]);
	dclose(stdout_pipe_fds[0]);
	dclose(stdout_pipe_fds[1]);

	dclose(2);		/* close stderr */
	dup(stderr_pipe_fds[1]);
	/* dclose(stderr_pipe_fds[0]); */
	/* dclose(stderr_pipe_fds[1]); */

	/* build environment, by copying what came in from mpd, adding either fd or
	   listener port for client-manager messages, and not copying manager-specific env
	   */
	
	for ( i = 0, j = 0; envp[i]; i++ ) {
	    if ( strncmp( envp[i], "MAN_", 4 ) != 0 )
		client_env[j++] = envp[i];
	}

	dclose( man_client_msgs_fds[0] );
        sprintf( env_man_client_listener, "MAN_MSGS_FD=%d\n",man_client_msgs_fds[1] );
	client_env[j++] = env_man_client_listener;
	client_env[j]   = NULL;

	listener_fd = setup_network_socket( &my_listener_port );  
	strcpy( fdtable[listener_idx].name, "listener" );
	mpdprintf( debug, "setup listener socket on port=%d\n", my_listener_port);
        sprintf( env_client_listener_fd, "CLIENT_LISTENER_FD=%d\n", listener_fd );
	client_env[j++] = env_client_listener_fd;
	client_env[j]   = NULL;

        sprintf( env_path_for_exec, "PATH=%s", getenv( "PATH" ) );
	mpdprintf( 0, "pathforexec=:%s:\n", env_path_for_exec );
	client_env[j++] = env_path_for_exec;
	client_env[j]   = NULL;

	for ( i=0; client_env[i]; i++ ) {
	    putenv( client_env[i] );
	}

	sprintf( buf, "cmd=alive port=%d\n", my_listener_port );
	send_msg( man_client_msgs_fds[1], buf, strlen( buf ) );
	rc = read_line( man_client_msgs_fds[1], buf, 256 );
	mpdprintf( 0, "GO from parent=:%s:, rc=%d\n", buf, rc );
	parse_keyvals( buf );
	getval( "cmd", cmd );
	if ( strcmp( cmd, "cligo" ) != 0 ) {
	    mpdprintf( 1, "OOPS: recvd %s when expecting cligo\n",cmd);
	    exit(-1);
	}
	mpdprintf( debug, "execvp-ing program %s\n", client_pgm_name );
	argv[0] = client_pgm_name;
	
	strcpy( pwd_for_exec, getenv( "PWD" ) );
	mpdprintf( debug, "pwdforexec=:%s:\n", pwd_for_exec );
	rc = chdir( pwd_for_exec );
	
	rc = execvp( client_pgm_name, argv );
	error_check( rc, "execvp" );
    }
    else {			/* parent (manager) process */
	client_pid = pid;
	dclose(stdin_pipe_fds[0]);
	dclose(stdout_pipe_fds[1]);
	dclose(stderr_pipe_fds[1]);
	dclose( man_client_msgs_fds[1] );

	/* set up fdtable entries for client stdout and stderr */
	client_stdout_idx = allocate_fdentry();
	fdtable[client_stdout_idx].fd = stdout_pipe_fds[0];
	fdtable[client_stdout_idx].read = 1;
	fdtable[client_stdout_idx].write = 0;
	fdtable[client_stdout_idx].handler = CLIENT_STDOUT;

	client_stderr_idx = allocate_fdentry();
	fdtable[client_stderr_idx].fd = stderr_pipe_fds[0];
	fdtable[client_stderr_idx].read = 1;
	fdtable[client_stderr_idx].write = 0;
	fdtable[client_stderr_idx].handler = CLIENT_STDERR;
    }

    mpdprintf( debug, "entering main loop\n" );
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
            error_check( rc, "main loop: select" );
        }

        for ( i = 0; i < MAXFDENTRIES; i++ )
            if ( fdtable[i].active ) 
                if ( FD_ISSET( fdtable[i].fd, &readfds ) )
                    handle_input_fd( i );
    }
    man_cleanup();
    mpdprintf( debug, "manager exiting\n");
    return(0);
}

void handle_input_fd( idx )
int idx;
{
    /* RMB: following is debug */
    if (fdtable[idx].handler == TREE_STDOUT)
	mpdprintf( debug, "HANDLER=%d for idx=%d\n", fdtable[idx].handler,idx);

    if ( fdtable[idx].handler == NOTSET )
        mpdprintf( debug, "man:  handler not set for port %d\n", idx );
    else if ( fdtable[idx].handler == MAN_LISTEN )
        handle_listen_input( idx );
    else if ( fdtable[idx].handler == MAN_CLIENT )
        handle_client_msgs_input( idx );
    else if ( fdtable[idx].handler == LHS_MSGS )
        handle_lhs_msgs_input( idx );
    else if ( fdtable[idx].handler == RHS_MSGS )
        handle_rhs_msgs_input( idx );
    else if ( fdtable[idx].handler == CON_CNTL )
        handle_con_cntl_input( idx );
    else if ( fdtable[idx].handler == CON_STDIN )
        handle_con_stdin_input( idx );
    else if ( fdtable[idx].handler == CLIENT_STDOUT ||
	      fdtable[idx].handler == TREE_STDOUT      )
        handle_client_stdout_input( idx );
    else if ( fdtable[idx].handler == CLIENT_STDERR ||
	      fdtable[idx].handler == TREE_STDERR      )
        handle_client_stderr_input( idx );
    else
        mpdprintf( 1, "invalid handler for fdtable entry %d; handler is %d \n",
		   idx, fdtable[idx].handler );

    /* RMB: following is debug */
    if (fdtable[idx].handler == TREE_STDOUT)
	mpdprintf( debug, "LEAVE HANDLER=%d for idx=%d\n", fdtable[idx].handler,idx);
}


void handle_listen_input( int idx )
{
    int tmp_idx, length;
    char message[MAXLINE], tmpbuf[MAXLINE], cmd[MAXLINE], buf[MAXLINE];

    mpdprintf( debug, "man: handling listen input; idx=%d fd=%d\n",idx,fdtable[idx].fd ); 
    tmp_idx = allocate_fdentry();
    fdtable[tmp_idx].fd      = accept_connection( fdtable[idx].fd );
    fdtable[tmp_idx].read    = 1;
    if ( (length = read_line(fdtable[tmp_idx].fd, message, MAXLINE ) ) != 0 )
        mpdprintf( debug, "handle_listen_input: message from tmp_idx to handle = :%s:\n",
		   message );
    else {
        mpdprintf( 1, "handle_listen_input: failed to retrieve msg on conn to listener\n");
	return;
    }
    strcpy( tmpbuf, message );             
    parse_keyvals( tmpbuf );
    getval( "cmd", cmd );
    if ( strcmp( cmd, "new_man_lhs" ) == 0 ) {
        lhs_idx = tmp_idx;
	fdtable[lhs_idx].handler = LHS_MSGS;
	strcpy( fdtable[tmp_idx].name, "lhs_msgs" );
    }
    else if ( strcmp( cmd, "new_man_rhs" ) == 0 ) {
        rhs_idx = tmp_idx;
	mpdprintf( debug, "for new_man_rhs: set up rhs on idx=%d\n", rhs_idx );
	fdtable[rhs_idx].handler = RHS_MSGS;
	strcpy( fdtable[tmp_idx].name, "rhs_msgs" );
	if ( jobsync_is_here && client_state == CLALIVE ) { 
	    /* now have someone to send jobsync to */
	    sprintf( buf, "cmd=jobsync from=%s dest=anyone\n", myid );
	    mpdprintf( debug, "handle_listen_input: sending jobsync\n" );
	    write_line( rhs_idx, buf );
	    jobsync_is_here = 0;
	}
    }
    else if ( strcmp( cmd, "new_stdout_stream" ) == 0 ) {
	mpdprintf( debug, "setting tree_stdout for idx=%d\n",tmp_idx );
	fdtable[tmp_idx].handler = TREE_STDOUT;
	strcpy( fdtable[tmp_idx].name, "tree_stdout" );
    }
    else if ( strcmp( cmd, "new_stderr_stream" ) == 0 ) {
	fdtable[tmp_idx].handler = TREE_STDERR;
	strcpy( fdtable[tmp_idx].name, "tree_stderr" );
    }
    else {
        mpdprintf( 1, "unrecognized msg to listener = :%s:\n",cmd );
    }
}

void handle_con_stdin_input( int idx )
{
    int length;
    char message[MAXLINE];

    mpdprintf( debug, "handling con stdin input in manager\n" );
    if ( (length = read_line(fdtable[idx].fd, message, MAXLINE ) ) > 0 ) {
	/* only manager 0 should receive console input */
        mpdprintf( debug, "from stdin :%s:\n", message );
	send_msg( stdin_pipe_fds[1], message, length );
    }
    else {
	if ( length < 0 ) {
	    mpdprintf( 1, "failed to retrieve msg on con_stdin\n");
	    return;
	}
	else {
	    mpdprintf( debug, "eof on con_stdin\n" );
	    dclose( fdtable[idx].fd ); 
	    deallocate_fdentry( idx );
	}
    }
}

void handle_con_cntl_input( int idx )
{
    int length;
    char message[MAXLINE], buf[MAXLINE];
    char cmdval[MAXLINE], signo[MAXLINE];

    mpdprintf( debug, "handling con cntl input in manager\n" );
    if ( (length = read_line(fdtable[idx].fd, message, MAXLINE ) ) > 0 ) {
        mpdprintf( debug, "from cntl, length=%d, msg=:%s:\n", length, message );

	strcpy( buf, message );             
	parse_keyvals( message );
	getval( "cmd", cmdval );
        if ( strlen(cmdval) == 0 ) {
            mpdprintf( 1, "no command specified in msg from console :%s:\n", buf );
	}
	else if ( strcmp( cmdval, "manringtest" ) == 0 ) {
	    write_line( rhs_idx, buf );
	}
	else if ( strcmp( cmdval, "signal" ) == 0 ) {
	    getval( "signo", signo );
	    mpdprintf( debug, "manager got signal command, signal=%s\n", signo );
	    sig_all( signo );
	    if ( strcmp( signo, "SIGINT" ) == 0 ) {
		sprintf( buf, "cmd=allexit\n" );
		write_line( rhs_idx, buf );
	    }
	}
	else if ( strcmp( cmdval, "allexit" ) == 0 ) {
	    if ( rhs_idx != -1 ) {
		sprintf( buf, "cmd=allexit\n" );
		write_line( rhs_idx, buf );
	    }
	    if ( captured_io_sockets_closed() ) {
		mpdprintf( debug, "handle_con_cntl_input: setting done=1\n" );
		done = 1;
	    }
	    else {
		allexit_received = 1;
	    }
	}
	else 
	    mpdprintf( 1, "unrecognized command :%s: from console on cntl\n", cmdval );
    }
    else {
	if ( length < 0 ) {
	    mpdprintf( 1, "failed to retrieve msg on cntl\n" );
	    return;
	}
	else {
	    mpdprintf( debug, "eof on cntl\n" );
	    dclose( fdtable[idx].fd ); 
	    deallocate_fdentry( idx );
	}
    }
}

void handle_client_stdout_input( int idx )
{
    int n, optval = 1;
    char buf[STREAMBUFSIZE];
    
    /* in general, this goes to a stdout port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling client stdout input in manager\n" );
    if ( con_stdout_idx == -1 ) {
	con_stdout_idx       = allocate_fdentry();
	fdtable[con_stdout_idx].fd = 
	    network_connect( parent_in_tree_hostname,parent_in_tree_port );
	setsockopt( fdtable[con_stdout_idx].fd, IPPROTO_TCP, TCP_NODELAY,
		    (char *) &optval, sizeof(optval) );
	fdtable[con_stdout_idx].read    = 0;
	fdtable[con_stdout_idx].write   = 1;
	fdtable[con_stdout_idx].portnum = parent_in_tree_port;
	strcpy(fdtable[con_stdout_idx].name, "con_stdout" );
	sprintf( buf, "cmd=new_stdout_stream\n" );
	write_line( con_stdout_idx, buf );
    }
    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	if ( debug )
	    buf[n] = '\0';  /* for debug printing */
	mpdprintf( 0,
		   "handle_client_stdout_input: FORWARDING stdout"
		   " n=%d fromidx=%d handler=%s toidx=%d tofd=%d buf=|%s|\n",
		   n, idx, phandler(fdtable[idx].handler), con_stdout_idx, 
		   fdtable[con_stdout_idx].fd, buf);
	send_msg( fdtable[con_stdout_idx].fd, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( debug,
		   "handle_client_stdout_input: manager got EOF on stdout from client\n" );
	dclose( fdtable[idx].fd );  /* close client's stdout */
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_client_stdout_input: setting done=1\n" );
	    done = 1;
	}
    }
    else 
	fprintf( stderr,
		 "%s: manager failed to retrieve msg from client's stdout n=%d\n",
		 myid, n );
}

void handle_client_stderr_input( int idx )
{
    int n;
    char buf[STREAMBUFSIZE];
    
    /* in general, this goes to a stderr port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling client stderr input in manager\n" );
    if ( con_stderr_idx == -1 ) {
	con_stderr_idx       = allocate_fdentry();
	fdtable[con_stderr_idx].fd = 
	    network_connect( parent_in_tree_hostname,parent_in_tree_port );
	fdtable[con_stderr_idx].read    = 0;
	fdtable[con_stderr_idx].write   = 1;
	fdtable[con_stderr_idx].portnum = parent_in_tree_port;
	strcpy(fdtable[con_stderr_idx].name, "con_stderr" );
	sprintf( buf, "cmd=new_stderr_stream\n" );
	write_line( con_stderr_idx, buf );
    }
    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	if ( debug )
	    buf[n] = '\0';  /* for debug printing */
	mpdprintf( 0,
		   "handle_client_stdout_input: FORWARDING stdout"
		   " n=%d fromidx=%d handler=%s toidx=%d tofd=%d buf=|%s|\n",
		   n, idx, phandler(fdtable[idx].handler), con_stdout_idx, 
		   fdtable[con_stdout_idx].fd, buf);
	send_msg( fdtable[con_stderr_idx].fd, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( debug,
		   "handle_client_stdout_input: manager got EOF on stdout from client\n" );
	dclose( fdtable[idx].fd ); 
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_client_stderr_input: setting done=1\n" );
	    done = 1;
	}
    }
    else 
	fprintf( stderr,
		 "%s: manager failed to retrieve msg from client's stderr n=%d\n",
		 myid, n );
}

void handle_client_msgs_input( int idx )
{
    char buf[MAXLINE];

    mpdprintf( debug, "manager handling client_msgs input\n" );
    if ( read_line( fdtable[idx].fd, buf, MAXLINE ) != 0 ) {
        mpdprintf( debug, "manager received :%s: from client\n", buf );
	parse_keyvals( buf );
	getval( "cmd", buf );
	if ( strcmp( buf, "alive" ) == 0 )
	    man_cli_alive( fdtable[idx].fd );
	else if ( strcmp( buf, "findclient" ) == 0 )
	    man_cli_findclient( fdtable[idx].fd );
	else if ( strcmp( buf, "request_peer_connection" ) == 0 )
	    man_cli_request_peer_connection( fdtable[idx].fd );
    }
    else {                        /* client gone away */
        mpdprintf( debug, "manager lost contact with client\n" );
        dclose( fdtable[idx].fd ); 
        deallocate_fdentry( idx );
	client_state = CLDEAD;	  /* mark client as dead */
	waitpid( client_pid, &client_stat, 0 );
	if ( jobdeadsync_is_here == 1 ) {
	    sprintf( buf, "cmd=jobdeadsync dest=anyone\n" );
	    mpdprintf( debug,
		       "handle_client_msgs_input: sending jobdeadsync to rhs\n" );
	    write_line( rhs_idx, buf );
	    jobdeadsync_is_here = 0;
	}
    }
}

void handle_lhs_msgs_input( int idx )
{
    int length, destrank, rank;
    char message[MAXLINE], fwdbuf[MAXLINE], buf[MAXLINE];
    char cmdval[MAXLINE], dest[MAXLINE];
    int optval = 1;

    mpdprintf( debug, "handling lhs_msgs input in manager\n" );
    if ( (length = read_line(fdtable[idx].fd, message, MAXLINE ) ) > 0 ) {
        mpdprintf( debug, "msg from lhs :%s:\n", message );

	strcpy( fwdbuf, message );             
	parse_keyvals( message );
	getval( "cmd", cmdval );
        if ( strlen(cmdval) == 0 ) {
            mpdprintf( 1, "no command specified in msg from lhs :%s:\n", fwdbuf );
	}
	else if ( strcmp( cmdval, "manringtest" ) == 0 ) {
	    if ( myrank == 0 ) {
		sprintf( buf, "man ringtest completed\n" );
		write_line( con_cntl_idx, buf );
	    }
	    else
		write_line( rhs_idx, fwdbuf );
	}
	else if ( strcmp( cmdval, "jobsync" ) == 0 )
	{
	    if ( myrank == 0 ) {
		/* other nodes send tree info when receive jobgo */
	        if ( lchild_in_tree >= 0 )
	        {
		    mpdprintf( debug, "sending id_of_parent to %d\n", lchild_in_tree );
		    sprintf( buf, 
			     "cmd=id_of_parent_in_tree destrank=%d srcrank=%d "
			     "srchost=%s srcport=%d\n",
			     lchild_in_tree,  myrank, myhostname, my_listener_port );
		    write_line( rhs_idx, buf );
	        }
	        if ( rchild_in_tree >= 0 )
	        {
		    mpdprintf( debug, "sending id_of_parent to %d\n", rchild_in_tree );
		    sprintf( buf, 
			     "cmd=id_of_parent_in_tree destrank=%d srcrank=%d "
			     "srchost=%s srcport=%d\n",
			     rchild_in_tree,  myrank, myhostname, my_listener_port );
		    write_line( rhs_idx, buf );
	        }

		mpdprintf( debug, "handle_lhs_msgs_input: got jobsync, sending jobgo\n" );
		sprintf( buf, "cmd=jobgo from=%s dest=anyone\n", myid );
		write_line( rhs_idx, buf );
		jobdeadsync_is_here = 1; /* prepare the termination logic */
	    }
	    else if ( client_state == CLALIVE && rhs_idx != -1 ) {
		sprintf( buf, "cmd=jobsync from=%s dest=anyone\n", myid );
		mpdprintf( debug, "handle_lhs_msgs_input: sending jobsync\n" );
		write_line( rhs_idx, buf );
		jobsync_is_here = 0;
	    }
	    else
		jobsync_is_here = 1;
	}
	else if ( strcmp( cmdval, "jobdeadsync" ) == 0 )
	{
	    if ( client_state == CLDEAD ) {
		if ( myrank == 0 ) { /* it will have been all the way around */
		    mpdprintf( debug,
			       "handle_lhs_msgs_input: sending jobdead to console\n" );
		    sprintf( buf, "cmd=jobdead\n" );
		    write_line( con_cntl_idx, buf );
		}
		else {
		    sprintf( buf, "cmd=jobdeadsync dest=anyone\n" );
		    mpdprintf( debug,
			       "handle_lhs_msgs_input: sending jobdeadsync to rhs\n" );
		    write_line( rhs_idx, buf );
		}
	    }
	    else
		jobdeadsync_is_here = 1;
	}
	else if ( strcmp( cmdval, "jobgo" ) == 0 )
	{
	    mpdprintf( debug, "checking parent_in_tree_port=%d\n", parent_in_tree_port );
	    /* rank 0 sends tree info when jobsync gets back to it, before sending jobgo */
	    if ( myrank > 0 )
	    {
		if ( lchild_in_tree >= 0 )
		{
		    sprintf( buf, 
			     "cmd=id_of_parent_in_tree destrank=%d srcrank=%d "
			     "srchost=%s srcport=%d\n",
			     lchild_in_tree,  myrank, myhostname, my_listener_port );
		    write_line( rhs_idx, buf );
		}
		if ( rchild_in_tree >= 0 )
		{
		    sprintf( buf, 
			     "cmd=id_of_parent_in_tree destrank=%d srcrank=%d "
			     "srchost=%s srcport=%d\n",
			     rchild_in_tree,  myrank, myhostname, my_listener_port );
		    write_line( rhs_idx, buf );
		}
	    }

	    /* needs to also check prebuild tree flag */
	    if (parent_in_tree_port != -1)
	    {
		con_stdout_idx       = allocate_fdentry();
		fdtable[con_stdout_idx].fd = 
		    network_connect( parent_in_tree_hostname,parent_in_tree_port );
		setsockopt( fdtable[con_stdout_idx].fd, IPPROTO_TCP, TCP_NODELAY,
			    (char *) &optval, sizeof(optval) );
		fdtable[con_stdout_idx].read    = 0;
		fdtable[con_stdout_idx].write   = 1;
		fdtable[con_stdout_idx].portnum = parent_in_tree_port;
		strcpy(fdtable[con_stdout_idx].name, "con_stdout" );
		sprintf( buf, "cmd=new_stdout_stream\n" );
		write_line( con_stdout_idx, buf );
		con_stderr_idx       = allocate_fdentry();
		fdtable[con_stderr_idx].fd = 
		    network_connect( parent_in_tree_hostname,parent_in_tree_port );
		setsockopt( fdtable[con_stderr_idx].fd, IPPROTO_TCP, TCP_NODELAY,
			    (char *) &optval, sizeof(optval) );
		fdtable[con_stderr_idx].read    = 0;
		fdtable[con_stderr_idx].write   = 1;
		fdtable[con_stderr_idx].portnum = parent_in_tree_port;
		strcpy(fdtable[con_stderr_idx].name, "con_stderr" );
		sprintf( buf, "cmd=new_stderr_stream\n" );
		write_line( con_stderr_idx, buf );
	    }

	    if ( myrank != 0 )
		write_line( rhs_idx, fwdbuf );
	    sprintf( buf, "cmd=cligo\n" );
	    mpdprintf( debug, "handle_lhs_msgs_input: sending cligo\n" );
	    write_line( client_idx, buf );
	}
	else if ( strcmp( cmdval, "id_of_parent_in_tree" ) == 0 )
	{
	    destrank = atoi( getval( "destrank", buf ) );
	    if ( destrank == myrank ) {
	        getval( "srchost", parent_in_tree_hostname );
	        parent_in_tree_port = atoi( getval( "srcport", buf ) );
	        mpdprintf( debug, "got id_of_parent, host=%s port=%d\n",
		           parent_in_tree_hostname, parent_in_tree_port );
	    }
	    else
		write_line( rhs_idx, fwdbuf );
	}
	else if ( strcmp( cmdval, "sigall" ) == 0 ) {
	    int signum;
	    char signo[16], source[IDSIZE];
	    
	    getval( "src", source );
	    getval( "signo", signo );
	    if ( strcmp( source, myid ) != 0 ) {
		/* here forward the message to the next manager */
		sprintf( buf, "src=%s cmd=sigall signo=%s\n", source, signo );
		write_line( rhs_idx, buf );
	    }
	    signum = map_signo( signo );
	    mpdprintf( debug, "manager %s signalling %s (%d) to client process %d\n",
		       myid, signo, signum, client_pid );
	    kill( client_pid, signum );
	}
	else if ( strcmp( cmdval, "allexit" ) == 0 ) {
	    if ( rhs_idx != -1 ) {
		sprintf( buf, "cmd=allexit\n" );
		write_line( rhs_idx, buf );
	    }
	    if ( captured_io_sockets_closed() ) {
		mpdprintf( debug, "handle_lhs_msgs_input: setting done=1\n" );
		done = 1;
	    }
	    else {
		allexit_received = 1;
	    }
	}
	else if ( strcmp( cmdval, "findclient" ) == 0 )
	{
	    strcpy( dest, getval( "dest", buf ) );
	    rank = atoi( getval( "rank", buf ) );
	    if ( rank == myrank ) {
		mpdprintf( debug, "handle_lhs_msgs_input: sending foundclient\n" );
		sprintf( buf, "cmd=foundclient dest=%s host=%s port=%d pid=%d\n",
			 dest,myhostname,client_listener_port,client_pid );
		write_line( rhs_idx, buf );
	    }
	    else {
		if ( strcmp( dest, myid ) == 0 ) {
		    mpdprintf( 1, "handle_lhs_msgs_input: failed to find client: buf=:%s:\n",
			       fwdbuf );
		    sprintf( buf, "cmd=foundclient host=dummy port=-1 pid=-1\n" );
		    write_line( client_idx, buf );
		}
		else {
		    write_line( rhs_idx, fwdbuf );
		}
	    }
	}
	else if ( strcmp( cmdval, "foundclient" ) == 0 )
	{
	    strcpy( dest,getval( "dest", buf ) );
	    if ( strcmp( dest, myid ) == 0 ) {
		mpdprintf( debug, "handle_lhs_msgs_input: got foundclient\n" );
		write_line( client_idx, fwdbuf );
	    }
	    else
		write_line( rhs_idx, fwdbuf );
	}
	else if ( strcmp( cmdval, "request_peer_connection" ) == 0 )
	{
	    strcpy( dest, getval( "dest", buf ) );
	    rank = atoi( getval( "rank", buf ) );
	    if ( rank == myrank ) {
		mpdprintf( 1, "handle_lhs_msgs_input: poking client and sending request_peer_conn\n" );
		kill( client_pid, SIGUSR1 );
		write_line( client_idx, fwdbuf );  /* send on to client */
	    }
	    else {
		if ( strcmp( dest, myid ) == 0 ) {
		    mpdprintf( 1, "handle_lhs_msgs_input: failed to find client "
				  "for connection: buf=:%s:\n",
			       fwdbuf );
		    /***** might poke client here and tell him to come out of
		     his accept and deal with failed conn; on the other hand,
		     client may do a select for the accept and time out after
		     a while
		    *****/
		}
		else {
		    write_line( rhs_idx, fwdbuf );
		}
	    }
	}
	else
	    mpdprintf( 1, "unrecognized command :%s: on lhs_msgs\n", cmdval );
    }
    else {
	if ( length < 0 ) {
	    mpdprintf( 1, "failed to retrieve msg on cntl\n");
	    return;
	}
	else {
	    mpdprintf( debug, "eof on cntl\n");
	    dclose( fdtable[idx].fd ); 
	    deallocate_fdentry( idx );
	}
    }
}

void handle_rhs_msgs_input( int idx )
{
    mpdprintf( debug, "handle_rhs_msgs_input, should be EOF on %d\n", idx );
    deallocate_fdentry( rhs_idx );
    rhs_idx = -1;
}


void man_cli_alive( fd )
int fd;
{
    int i, n;
    char buf[MAXLINE];

    mpdprintf( debug, "handling cli_alive in manager\n" );
    getval("port",buf);
    client_listener_port = atoi(buf);
 
    client_fd = fd;
    client_state = CLALIVE;

    mpdprintf( debug, "man_cli_alive: alive, jobsync_is_here=%d rhs_idx=%d\n",
	       jobsync_is_here, rhs_idx );

    if ( jobsync_is_here && rhs_idx != -1 ) {
	sprintf( buf, "cmd=jobsync from=%s dest=anyone\n", myid );
	mpdprintf( debug, "cli_alive: sending jobsync\n" );
	write_line( rhs_idx, buf );
	jobsync_is_here = 0;
    }
}

void man_cli_findclient(client_fd)
int client_fd;
{    
    char buf[MAXLINE];
    int  rank, job;

    mpdprintf( debug, "handling cli_findclient in manager\n" );
    getval("job",buf);
    job = atoi(buf);
    getval("rank",buf);
    rank = atoi(buf);
    sprintf(buf,"src=%s dest=%s bcast=true cmd=findclient job=%d rank=%d\n",
	    myid, myid, job, rank);
    write_line( rhs_idx, buf );
    return;
}

void man_cli_request_peer_connection(client_fd)
int client_fd;
{    
    char buf[MAXLINE];
    int  rank, job;

    mpdprintf( 1, "handling man_cli_request_peer_connection in manager\n" );
    getval("job",buf);
    job = atoi(buf);
    getval("rank",buf);
    rank = atoi(buf);
    sprintf(buf,"src=%s dest=%s bcast=true cmd=request_peer_connection "
		"job=%d rank=%d requesting_host=%s requesting_port=%d\n",
	    myid, myid, job, rank, myhostname, client_listener_port );
    write_line( rhs_idx, buf );
    return;
}

void man_cleanup()
{
    int i;

    if ( debug )
	dump_fdtable( "in man_cleanup" );
    for ( i = 0; i < MAXFDENTRIES; i++ ) {
        if ( fdtable[i].active )  {
	    mpdprintf( debug, "port[%d] name-> %s\n", i,fdtable[i].name );
	    /* not doing anything right now */
        }
    }
}

void man_compute_nodes_in_print_tree( self, nprocs, parent, lchild, rchild )
int self, nprocs, *parent, *lchild, *rchild;
{
    if (self == 0)
	*parent = -1;
    else
	*parent = (self - 1) / 2;

    if ((*lchild = (2 * self) + 1) > nprocs - 1)
	*lchild = -1;

    if ((*rchild = (2 * self) + 2) > nprocs - 1)
	*rchild = -1;
}

void sig_all( char *signo )
{
    char buf[MAXLINE];

    mpdprintf( debug, "manager sending out signal %s on rhs_idx=%d, fd=%d\n",
	       signo, rhs_idx, fdtable[rhs_idx].fd );
    sprintf( buf, "src=%s cmd=sigall signo=%s\n", myid, signo );
    if ( rhs_idx != -1 )
	write_line( rhs_idx, buf );
    else
	mpdprintf( debug, "manager could not send signal %s to rhs\n", signo );
    /* manager 0 will be the last one to signal its client, after it has called this rtn */
}

/* returns false if I/O is still pending on input sockets managed by the manager,
   so returns true if it is safe for the manager to exit */
int captured_io_sockets_closed()
{
    int i;

    for ( i=0; i < MAXFDENTRIES; i++ ) {
	if ( fdtable[i].active  &&  
	     ( fdtable[i].handler == TREE_STDOUT   ||
	       fdtable[i].handler == TREE_STDERR   ||
               fdtable[i].handler == CLIENT_STDOUT ||
	       fdtable[i].handler == CLIENT_STDERR ) )
	    return( 0 );
    }
    return( 1 );
}

