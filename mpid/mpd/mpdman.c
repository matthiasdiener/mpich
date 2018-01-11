#include "mpdman.h"
#include "mpd.h"
#include "bnr.h"

struct fdentry fdtable[MAXFDENTRIES];
extern int fdtable_high_water_mark;

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
int client_state = CLNOTSET;
int con_cntl_fd, con_stdin_fd, con_stdout_fd, con_stderr_fd;
int con_cntl_idx = -1, con_stdin_idx = -1, con_stdout_idx = -1, con_stderr_idx = -1;
int client_cntl_idx = -1, client_stdin_idx = -1, client_stdout_idx = -1;
int parent_in_tree = -1, lchild_in_tree = -1, rchild_in_tree = -1;
int client_stderr_idx = -1;
int jobsync_is_here = 0;
int jobdeadcntr = -1;
int parent_in_tree_port = -1;
int man_client_msgs_fds[2];
int allexit_received = 0;
int prebuild_print_tree = 0, line_labels = 0;
int stdintarget = 0;
char parent_in_tree_hostname[MAXHOSTNMLEN];
char myhostname[MAXHOSTNMLEN];
double timestamp_begin_execution, timestamp_jobgo_rcvd;

int debug;
char myid[IDSIZE];

int client_signal_status = NOT_ACCEPTING_SIGNALS;

BNR_gid grp[1024];
char attrs[1024][BNR_MAXATTRLEN];
char vals[1024][BNR_MAXVALLEN];
int av_idx = 0;

int client_fenced_in = 0;
int bnr_fence_in_msg_here = 0, bnr_fence_cnt = 0;
char bnr_fence_in_src[IDSIZE];
char bnr_fence_out_src[IDSIZE];

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
#ifdef NEED_PWENT
    struct passwd *pwent;
#endif
    struct sockaddr_in sin;
    mpd_sockopt_len_t sinlen = sizeof( sin );
    char cmd[MAXLINE];
    int optval = 1;
    char *shmemkey, *shmemgrpsize, *shmemgrprank;

    sprintf( myid, "man_%s", getenv( "MPD_JRANK" ) );
    mpdprintf( 000, "manager starting\n" );

    myrank = atoi( getenv( "MPD_JRANK" ) );
    jobsize = atoi( getenv( "MPD_JSIZE" ) );

    if ( myrank == 0 )
	timestamp_begin_execution = mpd_timestamp();

#ifdef NEED_PWENT
    if ((pwent = getpwuid(getuid())) == NULL) {
	printf("mpd: getpwuid failed");
	exit(99);
    }
#endif

    strcpy( client_pgm_name, getenv( "MAN_CLIENT_PGM" ) );
    listener_fd = atoi( getenv( "MAN_LISTENER_FD" ) ); 
    mpd_man_fd = atoi( getenv( "MAN_MPD_FD" ) );
    strcpy( left_peer_host, getenv( "MAN_PREVHOST" ) );
    left_peer_listener_port  = atoi( getenv( "MAN_PREVPORT" ) );
    strcpy( right_peer_host, getenv( "MAN_HOST0" ) );
    right_peer_listener_port = atoi( getenv( "MAN_PORT0" ) );
    strcpy( conhost, getenv( "MAN_CONHOST" ) );
    conport = atoi( getenv( "MAN_CONPORT" ) );
    prebuild_print_tree = atoi( getenv( "MAN_PREBUILD_PRINT_TREE" ) );
    line_labels = atoi( getenv( "MAN_LINE_LABELS" ) );
    debug = atoi( getenv( "MAN_DEBUG" ) );
    /* debug = 1; */
    
    /* plant shmemkey, shmemgrpsize, and shmemgrprank, if they exit, in BNR database */
    if ( ( shmemkey = getenv( "MPD_SHMEMKEY" ) ) != NULL ) {
	strcpy( attrs[av_idx], "SHMEMKEY" );
	strcpy( vals[av_idx],  shmemkey );
	av_idx++;
    }
    if ( ( shmemgrpsize = getenv( "MPD_SHMEMGRPSIZE" ) ) != NULL ) {
	strcpy( attrs[av_idx], "SHMEMGRPSIZE" );
	strcpy( vals[av_idx],  shmemgrpsize );
	av_idx++;
    }
    if ( ( shmemgrprank = getenv( "MPD_SHMEMGRPRANK" ) ) != NULL ) {
	strcpy( attrs[av_idx], "SHMEMGRPRANK" );
	strcpy( vals[av_idx],  shmemgrprank );
	av_idx++;
    }

    mpdprintf( debug, "right_peer_port=%d, left_peer_port=%d\n",
	       right_peer_listener_port, left_peer_listener_port );

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

    /* Set up port to parent mpd.  This should be replaced by a pipe.  HAS IT??? - RL */

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
        mpdprintf( debug, "manager has conn to lhs on host=%s port=%d\n",
		   left_peer_host, left_peer_listener_port);
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
        mpdprintf( debug, "manager has conn to rhs on host=%s port=%d\n",
		   right_peer_host, right_peer_listener_port);
    }
	
    /* At this point the ring consists of one-way connections;  All processes except 0
       have a lhs;  process numprocs-1 has both a lhs and a rhs; process 0 has neither.
       Next we have to convert each link to a both-ways connection, by sending messages
       on the links we have */

    if ( lhs_idx != -1 ) {	/* I have a connection to my left peer (all except 0) */
	sprintf( buf, "cmd=new_man_rhs\n" );
        mpdprintf( debug, "sending to lhs, buf=:%s:\n", buf );
	write_line( lhs_idx, buf );
    }

    if ( rhs_idx != -1 ) {	/* I have a connection to my right peer (only numprocs-1)*/
	sprintf( buf, "cmd=new_man_lhs\n" );
        mpdprintf( debug, "sending to rhs, buf=:%s:\n", buf );
	write_line( rhs_idx, buf );
    }


    /* Manager with rank 0 sets up connections to console for I/O and control streams */
    /* Other managers join in a tree up to 0, which forwards on to the console */
    if ( myrank == 0 ) {
	jobsync_is_here = 1;

	con_cntl_idx		      = allocate_fdentry();
	fdtable[con_cntl_idx].fd      = network_connect( conhost, conport );
	fdtable[con_cntl_idx].read    = 1;
	fdtable[con_cntl_idx].write   = 1;
	strcpy(fdtable[con_cntl_idx].name, "con_cntl" );
	fdtable[con_cntl_idx].handler = CON_CNTL;
	sprintf( buf, "cmd=new_ctl_stream\n" );
	write_line( con_cntl_idx, buf );
        mpdprintf( debug, "CON_CNTL fd=%d\n",fdtable[con_cntl_idx].fd );

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
    }	

    man_compute_nodes_in_print_tree( myrank, jobsize, &parent_in_tree,
				    &lchild_in_tree, &rchild_in_tree );
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
	dclose(stderr_pipe_fds[0]);
	dclose(stderr_pipe_fds[1]);

	/* build environment, by copying what came in from mpd, adding either fd or
	   listener port for client-manager messages, and not copying manager-specific env
	   */
	
	for ( i = 0, j = 0; envp[i]; i++ ) {
	    if ( strncmp( envp[i], "MAN_", 4 ) != 0 )
		client_env[j++] = envp[i];
	}

	dclose( man_client_msgs_fds[0] );
        sprintf( env_man_client_listener, "MAN_MSGS_FD=%d", man_client_msgs_fds[1] );
	client_env[j++] = env_man_client_listener;
	client_env[j]   = NULL;

	my_listener_port = 0;
	listener_fd = setup_network_socket( &my_listener_port );  
	strcpy( fdtable[listener_idx].name, "listener" );
	mpdprintf( debug, "setup listener socket for client on port=%d\n", my_listener_port);
        sprintf( env_client_listener_fd, "CLIENT_LISTENER_FD=%d", listener_fd );
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
	
	client_signal_status = NOT_ACCEPTING_SIGNALS;
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

	for ( i = 0; i <= fdtable_high_water_mark; i++ )
	    if ( fdtable[i].active && fdtable[i].read ) 
		FD_SET( fdtable[i].fd, &readfds );
	    
        num_fds = FD_SETSIZE;
        tv.tv_sec = 3600;
        tv.tv_usec = 0;

        rc = select( num_fds, &readfds, &writefds, NULL, &tv );
	/* mpdprintf( 1, "manager back from select, rc = %d\n", rc ); */

        if ( rc == 0 ) {
            mpdprintf( debug, "select timed out after %ld minutes\n", tv.tv_sec/60 );
            continue;
        } 
        if ( ( rc == -1 ) && ( errno == EINTR ) ) {
            mpdprintf( debug, "select interrupted; continuing\n" );
            continue;
        }
        if ( rc < 0 ) {
            error_check( rc, "mpdman main loop: select" );
        }

	for ( i = 0; i <= fdtable_high_water_mark; i++ )
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
    else if ( fdtable[idx].handler == PARENT_MPD_MSGS )
        handle_parent_mpd_input( idx );
    else if ( fdtable[idx].handler == CON_CNTL )
        handle_con_cntl_input( idx );
    else if ( fdtable[idx].handler == CON_STDIN )
        handle_con_stdin_input( idx );
    else if ( fdtable[idx].handler == CLIENT_STDOUT )
        handle_client_stdout_input( idx );
    else if ( fdtable[idx].handler == TREE_STDOUT )
	handle_tree_stdout_input( idx );
    else if ( fdtable[idx].handler == CLIENT_STDERR )
        handle_client_stderr_input( idx );
    else if ( fdtable[idx].handler == TREE_STDERR )
	handle_tree_stderr_input( idx ); 
    else
        mpdprintf( 1, "invalid handler for fdtable entry %d; handler is %d \n",
		   idx, fdtable[idx].handler );
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
        mpdprintf( debug, "handle_listen_input: message from tmp_idx to handle = :%s: (read %d)\n",
		   message, length );
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
    char message[MAXLINE], fwdbuf[MAXLINE], stuffed[MAXLINE];

    /* We look at the global variable stdintarget to decide where the input should go;
       -1 means to all processes, numbers refer to ranks, 0 is the default
    */

    mpdprintf( debug, "handling con stdin input in manager\n" );
    if ( (length = read_line(fdtable[idx].fd, message, MAXLINE ) ) > 0 ) {
	/* only manager 0 should receive console input directly from the console */
        mpdprintf( debug, "from con stdin :%s:\n", message );

	/* forward if it is for a specific other process */
	if ( stdintarget != myrank ) {
	    stuff_arg( message, stuffed );
	    mpdprintf( debug, "handle_con_stdin_input: sending :%s:\n", stuffed );
	    sprintf( fwdbuf, "cmd=stdin torank=%d message=%s",  /* nl already on message */
		     stdintarget, stuffed );
	    write_line( rhs_idx, fwdbuf );
	}
	/* send it to our client's stdin if it is for us or for all */
	if ( stdintarget == myrank || stdintarget == -1 ) {
	    mpdprintf( debug, "sending length %d to client :%s:\n", length, message );
	    send_msg( stdin_pipe_fds[1], message, length );
	}
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

void handle_parent_mpd_input( int idx )
{
    int length, signum;
    char signo[80], what[80];
    char message[MAXLINE], buf[MAXLINE], cmdval[MAXLINE];

    mpdprintf( debug, "handling parent mpd input in manager\n" );
    if ( (length = read_line(fdtable[idx].fd, buf, MAXLINE ) ) > 0 ) {
        mpdprintf( debug, "from parent mpd, length=%d, msg=:%s:\n", length, buf );

	strcpy( message, buf );   /* copy into message for forwarding */
	parse_keyvals( buf );
	getval( "cmd", cmdval );
        if ( strlen(cmdval) == 0 ) {
            mpdprintf( 1, "no command specified in msg from parent mpd :%s:\n", buf );
	}
	else if ( strcmp( cmdval, "mandump" ) == 0 ) {
	    mpdprintf( 000, "handle_parent_mpd_input: cmd=mandump\n" );
	    /* formulate the dump output and send it up as cmd=mandump_output */
	    getval( "what", what );
	    if ( strcmp( what, "all" ) != 0 ) 
		mpdprintf( 1, "mandump:  don't know how to dump %s\n", what );
	    else {
		mpdprintf( 1, "mandump: jobsync_is_here=%d jobdeadcntr=%d "
			   "client_state=%s\n",
			   jobsync_is_here, jobdeadcntr, pstate( client_state ) );
		dump_fdtable( "from inside handle_parent_mpd_input" );
		/* write_line( parent_mpd_idx, mandumpbuf ); */
	    }
	}
	else if ( strcmp( cmdval, "signaljob" ) == 0 )  {
	    getval( "signo", signo );
	    signum = map_signo( signo );
	    mpdprintf( debug, "signalling %s (%d) to client process %d\n",
		       signo, signum, client_pid );
	    kill( client_pid, signum );
	}
	else 
	    mpdprintf( 1, "unrecognized command :%s: from console on cntl\n", cmdval );
    }
    else {
	if ( length < 0 ) {
	    mpdprintf( 1, "failed to retrieve msg on parent mpd input\n" );
	    return;
	}
	else {
	    mpdprintf( debug, "eof on parent mpd input\n" );
	    dclose( fdtable[idx].fd ); 
	    deallocate_fdentry( idx );
	}
    }
}

void handle_con_cntl_input( int idx )
{
    int length, stdinval;
    char message[MAXLINE], buf[MAXLINE];
    char cmdval[MAXLINE], signo[MAXLINE];
    char stdinstr[8];

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
	else if ( strcmp( cmdval, "set" ) == 0 ) {
	    if ( getval( "stdin", stdinstr ) ) {
		if ( strcmp( stdinstr, "all" ) == 0 )
		    stdinval = -1;
		else
		    stdinval = atoi( stdinstr );
		if ( stdinval > jobsize )
		    mpdprintf( 1, "stdin rank %d too big\n", stdinval );
		else 
		    stdintarget = stdinval; /* set global stdin target */
	    } 
	    else {
		mpdprintf( 1, "set requires stdin=<val> argument, val = 'all' or rank\n" );
	    }
	}
	else if ( strcmp( cmdval, "con_bnr_put" ) == 0 ) {
	    getval( "attr", attrs[av_idx] );
	    getval( "val", vals[av_idx] );
	    grp[av_idx] = atoi( getval( "gid", buf ) );
	    mpdprintf( debug, "manager got con_bnr_put command, attr=%s val=%s gid=%d\n",
	               attrs[av_idx], vals[av_idx], grp[av_idx] );
	    av_idx++;
	}
	else if ( strcmp( cmdval, "signal" ) == 0 ) {
	    getval( "signo", signo );
	    mpdprintf( debug, "manager got signal command, signal=%s\n", signo );
	    sig_all( signo );
	    if ( strcmp( signo, "SIGINT" ) == 0 ) {
		sprintf( buf, "cmd=killjob jobid=%s\n", getenv( "MPD_JID" ) );
		write_line( parent_mpd_idx, buf );
	    }
	}
	else if ( strcmp( cmdval, "allexit" ) == 0 ) {
	    mpdprintf( debug, "handle_con_cntl_input: cmd=allexit\n" );
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

void handle_tree_stdout_input( int idx )
{
    int n, optval = 1;
    char buf[STREAMBUFSIZE];
    
    /* in general, this goes to a stdout port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling tree stdout input in manager\n" );
    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	if ( debug )
	    buf[n] = '\0';  /* for debug printing */
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
	    if ( jobdeadcntr >= 0 ) {
	        sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	        write_line( rhs_idx, buf );
		jobdeadcntr = -1;
	    }
	}
	mpdprintf( 0,
		   "handle_tree_stdout_input: FORWARDING stdout"
		   " n=%d fromidx=%d handler=%s toidx=%d tofd=%d buf=|%s|\n",
		   n, idx, phandler(fdtable[idx].handler), con_stdout_idx, 
		   fdtable[con_stdout_idx].fd, buf);
	send_msg( fdtable[con_stdout_idx].fd, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( 000,
		   "handle_tree_stdout_input: manager got EOF on stdout from client\n" );
	dclose( fdtable[idx].fd );  /* close child's stdout input*/
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_tree_stdout_input: setting done=1\n" );
	    done = 1;
	}
    }
    else 
	mpdprintf( 1, "manager failed to retrieve msg from tree's stdout n=%d\n", n );
}

void handle_client_stdout_input( int idx )
{
    int n, readsize, writesize, offset, optval = 1;
    char buf[STREAMBUFSIZE], cntlbuf[80];
    char *writefrom, *readinto;
    static int neednum = 1;	/* flag to say whether the next buffer passed through here
				   needs a line label, if we are handling line labels */

    /* in general, this goes to a stdout port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling client stdout input in manager\n" );

    if ( line_labels ) {
	char *p;
	int  n1, len;
	readinto = buf + 6;
	readsize = STREAMBUFSIZE - 6;
	sprintf( buf, "%4d: ", myrank );
	if ( myrank < 10 ) offset = 3;
	else if ( myrank < 100 ) offset = 2;
	else if ( myrank < 1000 ) offset = 1;
	else offset = 0;

	if ( ( n = read( fdtable[idx].fd, readinto, readsize ) ) > 0 ) {
	    if ( con_stdout_idx == -1 ) {
		mpdprintf( debug, "setting up stdout upwards in tree\n" );
		con_stdout_idx       = allocate_fdentry();
		fdtable[con_stdout_idx].fd = 
		    network_connect( parent_in_tree_hostname, parent_in_tree_port );
		setsockopt( fdtable[con_stdout_idx].fd, IPPROTO_TCP, TCP_NODELAY,
			    (char *) &optval, sizeof(optval) );
		fdtable[con_stdout_idx].read    = 0;
		fdtable[con_stdout_idx].write   = 1;
		fdtable[con_stdout_idx].portnum = parent_in_tree_port;
		strcpy(fdtable[con_stdout_idx].name, "con_stdout" );
		sprintf( cntlbuf, "cmd=new_stdout_stream\n" );
		write_line( con_stdout_idx, cntlbuf );
	        if ( jobdeadcntr >= 0 ) {
	            sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	            write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
	        }
	    }
	    n1 = n;
	    p = memchr( readinto, '\n', n1 );
	    while ( p ) {	/* found a newline */
		len = p - readinto + 1; /* include newline */
		if ( neednum ) {
		    writefrom = buf + offset;
		    writesize = len + ( 6 - offset );
		}
		else {
		    writefrom = buf + 6;
		    writesize = len;
		}
		send_msg( fdtable[con_stdout_idx].fd, writefrom, writesize );
		neednum = 1;
		n1 = n1 - len;
		/* shift chars after newline up to front of buffer */
		memmove( readinto, readinto + len, n1 );
		p = memchr( readinto, '\n', n1 );
	    }
	    if ( n1 ) {		/* trailing chars with no newline */
		writesize = n1;
		if ( neednum ) {
		    writefrom = buf + offset;
		    writesize = writesize + ( 6 - offset );
		}
		else {
		    writefrom = buf + 6;
		}
		send_msg( fdtable[con_stdout_idx].fd, writefrom, writesize );
		neednum = 0;

		/* special code to handle prompts */
		if ( strncmp( buf + 6, ">>> ", 4 ) == 0 ||    /* for testing w/itest */
		     strncmp( buf + 6, "(gdb)", 5 ) == 0 )
		    neednum = 1;
	    }
	    else
		neednum = 1;
	}
    }
    else {
	if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	    if ( con_stdout_idx == -1 ) {
		mpdprintf( debug, "setting up stdout upwards in tree\n" );
		con_stdout_idx       = allocate_fdentry();
		fdtable[con_stdout_idx].fd = 
		    network_connect( parent_in_tree_hostname, parent_in_tree_port );
		setsockopt( fdtable[con_stdout_idx].fd, IPPROTO_TCP, TCP_NODELAY,
			    (char *) &optval, sizeof(optval) );
		fdtable[con_stdout_idx].read    = 0;
		fdtable[con_stdout_idx].write   = 1;
		fdtable[con_stdout_idx].portnum = parent_in_tree_port;
		strcpy(fdtable[con_stdout_idx].name, "con_stdout" );
		sprintf( cntlbuf, "cmd=new_stdout_stream\n" );
		write_line( con_stdout_idx, cntlbuf );
	        if ( jobdeadcntr >= 0 ) {
	            sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	            write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
	        }
	    }
	    send_msg( fdtable[con_stdout_idx].fd, buf, n );
	}
    }

    if ( n == 0 ) {
	mpdprintf( debug,
		   "handle_client_stdout_input: manager got EOF on stdout from client\n" );
	dclose( fdtable[idx].fd );  /* close client's stdout */
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_client_stdout_input: setting done=1\n" );
	    done = 1;
	}
    }

    if ( n < 0 )
	mpdprintf( 1, "manager failed to retrieve msg from client's stdout n=%d\n", n );
}

void handle_tree_stderr_input( int idx )
{
    int n;
    char buf[STREAMBUFSIZE];
    
    /* in general, this goes to a stderr port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling tree stderr input in manager\n" );
    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	if ( debug )
	    buf[n] = '\0';  /* for debug printing */
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
	    if ( jobdeadcntr >= 0 ) {
	        sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	        write_line( rhs_idx, buf );
		jobdeadcntr = -1;
	    }
	}
	mpdprintf( 0,
		   "handle_tree_stdout_input: FORWARDING stdout"
		   " n=%d fromidx=%d handler=%s toidx=%d tofd=%d buf=|%s|\n",
		   n, idx, phandler(fdtable[idx].handler), con_stdout_idx, 
		   fdtable[con_stdout_idx].fd, buf);
	send_msg( fdtable[con_stderr_idx].fd, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( 000,
		   "handle_tree_stderr_input: manager got EOF on stderr from child\n" );
	dclose( fdtable[idx].fd ); 
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_tree_stderr_input: setting done=1\n" );
	    done = 1;
	}
    }
    else 
	mpdprintf( 1,"manager failed to retrieve msg from tree's stderr n=%d\n", n );
}

void handle_client_stderr_input( int idx )
{
    int n, readsize, writesize, offset;
    char buf[STREAMBUFSIZE];
    char *writefrom, *readinto;
    static int neednum = 1;	/* flag to say whether the next buffer passed through here
				   needs a line label, if we are handling line labels */

    /* in general, this goes to a stderr port up in tree, console for rank 0*/
    if ( myrank != 0  && parent_in_tree_port == -1 )  /* no parent info yet */
	return;
    mpdprintf( debug, "handling client stderr input in manager\n" );

    if ( line_labels ) {
	char *p;
	int  n1, len;
	readinto = buf + 6;
	readsize = STREAMBUFSIZE - 6;
	sprintf( buf, "%4d: ", myrank );
	if ( myrank < 10 ) offset = 3;
	else if ( myrank < 100 ) offset = 2;
	else if ( myrank < 1000 ) offset = 1;
	else offset = 0;

	if ( ( n = read( fdtable[idx].fd, readinto, readsize ) ) > 0 ) {
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
	        if ( jobdeadcntr >= 0 ) {
	            sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	            write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
	        }
	    }
	    n1 = n;
	    p = memchr( readinto, '\n', n1 );
	    while ( p ) {	/* found a newline */
		len = p - readinto + 1; /* include newline */
		if ( neednum ) {
		    writefrom = buf + offset;
		    writesize = len + ( 6 - offset );
		}
		else {
		    writefrom = buf + 6;
		    writesize = len;
		}
		send_msg( fdtable[con_stderr_idx].fd, writefrom, writesize );
		neednum = 1;
		n1 = n1 - len;
		/* shift chars after newline up to front of buffer */
		memmove( readinto, readinto + len, n1 );
		p = memchr( readinto, '\n', n1 );
	    }
	    if ( n1 ) {		/* trailing chars with no newline */
		writesize = n1;
		if ( neednum ) {
		    writefrom = buf + offset;
		    writesize = writesize + ( 6 - offset );
		}
		else {
		    writefrom = buf + 6;
		}
		send_msg( fdtable[con_stderr_idx].fd, writefrom, writesize );
		neednum = 0;

		/* special code to handle prompts */
		if ( strncmp( buf + 6, ">>> ", 4 ) == 0 ||    /* for testing w/itest */
		     strncmp( buf + 6, "(gdb)", 5 ) == 0 )
		    neednum = 1;
	    }
	    else
		neednum = 1;
	}
    }
    else {
	if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
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
	        if ( jobdeadcntr >= 0 ) {
	            sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	            write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
	        }
	    }
	    send_msg( fdtable[con_stderr_idx].fd, buf, n );
	}
    }

    if ( n == 0 ) {
	mpdprintf( 000,
		   "handle_client_stderr_input: manager got EOF on stderr from client\n" );
	dclose( fdtable[idx].fd );  /* close client's stderr */
	deallocate_fdentry( idx );
	if ( allexit_received  &&  captured_io_sockets_closed() ) {
	    mpdprintf( debug, "handle_client_stderr_input: setting done=1\n" );
	    done = 1;
	}
    }

    if ( n < 0 )
	mpdprintf( 1, "manager failed to retrieve msg from client's stderr n=%d\n", n );

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
	else if ( strcmp( buf, "accepting_signals" ) == 0 )
	    man_cli_accepting_signals( fdtable[idx].fd );
	else if ( strcmp( buf, "abort_job" ) == 0 )
	    man_cli_abort_job( fdtable[idx].fd );
	else if ( strcmp( buf, "findclient" ) == 0 )
	    man_cli_findclient( fdtable[idx].fd );
	else if ( strcmp( buf, "interrupt_peer_with_msg" ) == 0 )
	    man_cli_interrupt_peer_with_msg( fdtable[idx].fd );
	else if ( strcmp( buf, "client_bnr_get" ) == 0 )
	    man_cli_bnr_get( fdtable[idx].fd );
	else if ( strcmp( buf, "client_bnr_put" ) == 0 )
	    man_cli_bnr_put( fdtable[idx].fd );
	else if ( strcmp( buf, "client_bnr_fence_in" ) == 0 )
	    man_cli_bnr_fence_in( fdtable[idx].fd );
    }
    else {                        /* client gone away */
        mpdprintf( debug, "manager lost contact with client\n" );
        dclose( fdtable[idx].fd ); 
        deallocate_fdentry( idx );
	client_state = CLDEAD;	  /* mark client as dead */
	waitpid( client_pid, &client_stat, 0 );
	if ( myrank == 0 ) {
	    /* force it to 0 while I handle my child's final I/O */
	    sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
	    mpdprintf( debug,
		       "handle_client_msgs_input: sending jobdeadcntr=0 to rhs\n" );
	    write_line( rhs_idx, buf );
	}
	else {
	    if ( jobdeadcntr >= 0 ) {
	        /* force it back to 0 while I handle my child's final I/O */
		sprintf( buf, "cmd=jobdeadcntr cntr=0 dest=anyone\n" );
		mpdprintf( debug,
			   "handle_client_msgs_input: forwarding jobdeadcntr to rhs\n" );
		write_line( rhs_idx, buf );
		jobdeadcntr = -1;
	    }
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
		sprintf( buf, "cmd=man_ringtest_completed\n" );
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
		jobdeadcntr = 1; /* prepare the termination logic */
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
	else if ( strcmp( cmdval, "jobdeadcntr" ) == 0 )
	{
	    jobdeadcntr = atoi( getval( "cntr", buf ) );
	    mpdprintf( debug, "received jobdeadcntr=%d\n", jobdeadcntr );
	    if ( jobdeadcntr == jobsize ) {
		if ( myrank == 0 ) {
		    mpdprintf( debug,
			       "handle_lhs_msgs_input: sending jobdead to console\n" );
		    sprintf( buf, "cmd=jobdead\n" );
		    write_line( con_cntl_idx, buf );
		}
		else {
		    sprintf( buf, "cmd=jobdeadcntr cntr=%d dest=anyone\n", jobdeadcntr );
		    mpdprintf( debug,
			       "handle_lhs_msgs_input: fwding jobdeadcntr=%d to rhs\n",
			       jobdeadcntr );
		    write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
		}
	    }
	    else {
	        if ( client_state == CLDEAD ) {
		    sprintf( buf, "cmd=jobdeadcntr cntr=%d dest=anyone\n", jobdeadcntr+1 );
		    mpdprintf( debug,
			       "handle_lhs_msgs_input: incr&fwd jobdeadcntr=%d to rhs\n",
			       jobdeadcntr );
		    write_line( rhs_idx, buf );
		    jobdeadcntr = -1;
	        }
	        else {
		    mpdprintf( debug, "holding jobdeadcntr=%d\n", jobdeadcntr );
	        }
	    }
	}
	else if ( strcmp( cmdval, "jobgo" ) == 0 )
	{
            if ( myrank == 0 ) {
	        timestamp_jobgo_rcvd = mpd_timestamp();
		mpdprintf( debug, "time to start = %f\n",
		           timestamp_jobgo_rcvd-timestamp_begin_execution);
	    }

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

	    if ( parent_in_tree_port != -1  &&  prebuild_print_tree )
	    {
		mpdprintf( 000, "prebuilding iotree\n" );
		con_stdout_idx = allocate_fdentry();
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
	    mpdprintf( 000, "handle_lhs_msgs: cmd=allexit\n" );
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
	else if ( strcmp( cmdval, "abort_job" ) == 0 ) {
	    if ( myrank != 0 )
		write_line( rhs_idx, fwdbuf );
	    else {
		sig_all( "SIGINT" );
		sprintf( buf, "cmd=jobaborted job=%d code=%d rank=%d\n",
			 atoi( getval( "job", buf ) ),
			 atoi( getval( "abort_code", buf ) ),
			 atoi( getval( "rank", buf ) ) );
		write_line( con_cntl_idx, buf );
		sprintf( buf, "cmd=allexit\n" );
		write_line( rhs_idx, buf );
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
	else if ( strcmp( cmdval, "interrupt_peer_with_msg" ) == 0 )
	{
	    strcpy( dest, getval( "dest", buf ) );
	    rank = atoi( getval( "torank", buf ) );
	    if ( rank == myrank ) {
		mpdprintf( 000,"handle_lhs_msgs_input: poking client: state=%d idx=%d fd=%d\n",
			   client_state, client_idx, fdtable[idx].fd );
		getval( "msg", buf );
		strcat( buf, "\n" );
		if (client_state != CLDEAD)
		    write_line( client_idx, buf );  /* send on to client */
		if (client_signal_status == ACCEPTING_SIGNALS)
		    kill( client_pid, SIGUSR1 );
		else
		    client_signal_status = SIGNALS_TO_BE_SENT;
	    }
	    else {
		if ( strcmp( dest, myid ) == 0 ) {
		    mpdprintf( 1, "handle_lhs_msgs_input: failed to find client "
				  "for connection: buf=:%s:\n", fwdbuf );
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
	else if ( strcmp( cmdval, "stdin" ) == 0 ) {
	    char torankstr[8], stuffed[MAXLINE], unstuffed[MAXLINE];
	    int torank;

	    if ( myrank == 0 )
		return;		/* manager 0 has already handled stdin */
	    torank = atoi( getval( "torank", torankstr ) );
	    if ( myrank != torank ) 
		write_line( rhs_idx, fwdbuf );
	    if ( myrank == torank || torank == -1 ) {
		getval( "message", stuffed );
		destuff_arg( stuffed, unstuffed );
		strcat( unstuffed, "\n" );
		send_msg( stdin_pipe_fds[1], unstuffed, strlen( unstuffed ) );
	    }
	}
	else if ( strcmp( cmdval, "bnr_get" ) == 0 )
	{
	    int i, found, gid;
	    char attr[BNR_MAXATTRLEN];

	    strcpy( dest, getval( "dest", buf ) );
	    strcpy( attr, getval( "attr", attr ) );
	    gid = atoi( getval( "gid", buf ) );
	    for (i=0,found=0; i < av_idx && !found; i++)
	    {
		if (grp[i] == gid  &&  strcmp(attrs[i],attr) == 0)
		{
		    found = 1;
		}
	    }
	    if ( found ) {
		mpdprintf(debug,"handle_lhs_msgs: found :%s: with val=:%s:\n",attr,vals[i-1]);
		sprintf(buf,"cmd=bnr_get_output dest=%s attr=%s val=%s\n",
			dest,attr,vals[i-1] );
		write_line( rhs_idx, buf );
	    }
	    else {
		if ( strcmp( dest, myid ) == 0 ) {
		    mpdprintf( 1, "handle_lhs_msgs_input: failed for bnr_get: buf=:%s:\n",
			       fwdbuf );
		    sprintf( buf, "cmd=client_bnr_get_failed\n" );
		    write_line( client_idx, buf );
		}
		else {
		    write_line( rhs_idx, fwdbuf );
		}
	    }
	}
	else if ( strcmp( cmdval, "bnr_get_output" ) == 0 )
	{
	    char val[BNR_MAXVALLEN];

	    strcpy( dest, getval( "dest", buf ) );
	    strcpy( val, getval( "val", buf ) );
	    if ( strcmp( dest, myid ) == 0 ) {
		mpdprintf( debug, "handle_lhs_msgs_input: bnr_get_output: buf=:%s:\n",
			   fwdbuf );
		sprintf( buf, "cmd=client_bnr_get_output val=%s\n", val );
		write_line( client_idx, buf );
	    }
	    else {
		write_line( rhs_idx, fwdbuf );
	    }
	}
	else if ( strcmp( cmdval, "bnr_fence_in" ) == 0 )
	{
	    mpdprintf( debug, "handle_lhs_msgs: rcvd bnr_fence_in\n" );
	    strcpy( bnr_fence_in_src, getval( "dest", buf ) );
	    bnr_fence_cnt = atoi( getval( "cnt", buf ) );
	    if ( strcmp( myid,bnr_fence_in_src ) == 0 ) {
		sprintf( buf,"cmd=client_bnr_fence_out\n" );
		write_line( client_idx, buf );
		sprintf( buf,"cmd=bnr_fence_out dest=%s\n", myid );
		write_line( rhs_idx, buf );
	    }
	    else {
	        if ( client_fenced_in ) {
	            sprintf( buf, "cmd=bnr_fence_in dest=%s cnt=%d\n",
			     bnr_fence_in_src,bnr_fence_cnt );
	            write_line( rhs_idx, buf );
	        }
		else {
		    bnr_fence_in_msg_here = 1;
		}
	    }
	}
	else if ( strcmp( cmdval, "bnr_fence_out" ) == 0 )
	{
	    mpdprintf( debug, "handle_lhs_msgs: rcvd bnr_fence_out\n" );
	    strcpy( bnr_fence_out_src, getval( "dest", buf ) );
	    if ( strcmp( myid, bnr_fence_out_src ) != 0 ) {
		sprintf( buf,"cmd=client_bnr_fence_out\n" );
		write_line( client_idx, buf );
	        sprintf( buf,"cmd=bnr_fence_out dest=%s\n", bnr_fence_out_src );
	        write_line( rhs_idx, buf );
		client_fenced_in = 0; /* reset fence in case of future fences */
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
	mpdprintf( debug, "man_cli_alive: sending jobsync\n" );
	write_line( rhs_idx, buf );
	jobsync_is_here = 0;
    }
}

void man_cli_abort_job( fd )
int fd;
{
    char buf[MAXLINE], buf2[MAXLINE];

    mpdprintf( debug, "handling cli_abort_job in manager\n" );
    sprintf( buf, "cmd=abort_job job=%d rank=%d abort_code=%d\n",
	     atoi( getval( "job", buf2 ) ),
	     atoi( getval( "rank", buf2 ) ),
	     atoi( getval( "abort_code", buf2 ) ) );
    write_line( rhs_idx, buf );
}

void man_cli_accepting_signals( fd )
int fd;
{
    char buf[16];

    mpdprintf( debug, "handling accepting_signals in manager\n" );
    client_pid = atoi( getval( "pid", buf ) );
    if (client_signal_status == SIGNALS_TO_BE_SENT)
	kill( client_pid, SIGUSR1 );
    client_signal_status = ACCEPTING_SIGNALS;
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

void man_cli_interrupt_peer_with_msg(client_fd)
int client_fd;
{    
    char buf[MAXLINE], msg[MAXLINE];
    int  torank, fromrank, job;

    mpdprintf( debug, "handling man_cli_interrupt_peer_with_msg in manager\n" );
    getval("job",buf);
    job = atoi(buf);
    getval("torank",buf);
    torank = atoi(buf);
    getval("fromrank",buf);
    fromrank = atoi(buf);
    getval("msg",msg);
    sprintf(buf,"src=%s dest=%s bcast=true cmd=interrupt_peer_with_msg "
		"job=%d torank=%d fromrank=%d msg=%s\n",
	    myid, myid, job, torank, fromrank, msg );
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
int captured_io_sockets_closed( void )
{
    int i;

    for ( i = 0; i <= fdtable_high_water_mark; i++ ) {
	if ( fdtable[i].active  &&  
	     ( fdtable[i].handler == TREE_STDOUT   ||
	       fdtable[i].handler == TREE_STDERR   ||
               fdtable[i].handler == CLIENT_STDOUT ||
	       fdtable[i].handler == CLIENT_STDERR ) )
	    return( 0 );
    }
    return( 1 );
}

void man_cli_bnr_put( int fd )
{
    char buf[MAXLINE];

    getval( "attr", buf );
    strcpy( attrs[av_idx],buf );
    mpdprintf(00,"man_cli_bnr_put: putting attr=:%s:\n",attrs[av_idx]);
    getval( "val", buf );
    strcpy( vals[av_idx],buf );
    getval( "gid", buf );
    grp[av_idx] = atoi( buf );
    av_idx++;
}

void man_cli_bnr_get( int fd )
{
    int i, found, gid;
    char attr[BNR_MAXATTRLEN], buf[MAXLINE];

    getval( "attr", attr );
    getval( "gid", buf );
    gid = atoi(buf);
    mpdprintf(00,"man_cli_bnr_get: searching for :%s:\n",attr);
    for (i=0,found=0; i < av_idx && !found; i++)
    {
        if (grp[i] == gid  &&  strcmp(attrs[i],attr) == 0)
	{
	    mpdprintf(00,"man_cli_bnr_get: found :%s:\n",attr);
	    found = 1;
	    sprintf(buf,"cmd=client_bnr_get_output val=%s\n",vals[i]);
            send_msg( fd, buf, strlen( buf ) );
	}
    }
    if (!found)
    {
	mpdprintf( debug, "man_cli_bnr_get: did not find :%s:\n", attr );
	sprintf(buf,"cmd=bnr_get src=%s dest=%s bcast=true attr=%s gid=%d\n",
		myid, myid, attr, gid);
	write_line( rhs_idx, buf );
    }
}

void man_cli_bnr_fence_in( int fd )
{
    int gid, grank, gsize;
    char buf[MAXLINE];

    client_fenced_in = 1;
    gid   = atoi( getval( "gid", buf ) );
    grank = atoi( getval( "grank", buf ) );
    gsize = atoi( getval( "gsize", buf ) );
    mpdprintf( debug ,"man_cli_bnr_fence_in: gid=%d grank=%d gsize=%d flag=%d\n",
              gid, grank, gsize, bnr_fence_in_msg_here );
    if ( grank == 0 )
    {
	sprintf( buf,"cmd=bnr_fence_in dest=%s cnt=%d\n", myid, gsize-1 );
	write_line( rhs_idx, buf );
    }
    else
    {
        if (bnr_fence_in_msg_here)
	{
	    bnr_fence_cnt -= 1;
	    sprintf( buf, "cmd=bnr_fence_in dest=%s cnt=%d\n",
		     bnr_fence_in_src, bnr_fence_cnt );
	    bnr_fence_in_msg_here = 0;
	    write_line( rhs_idx, buf );
	    mpdprintf(000,"man_cli_bnr_fence_in: sending fence_out\n");
	}
    }
}
