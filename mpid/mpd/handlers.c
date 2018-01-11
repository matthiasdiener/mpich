/*
 *        handlers.c
 *        Handle incoming messages
 */
#include "mpd.h"

extern struct fdentry fdtable[MAXFDENTRIES];
extern int    console_idx;
extern int    client_idx;
extern char   lhshost[MAXHOSTNMLEN];
extern int    lhsport;
extern char   rhshost[MAXHOSTNMLEN];
extern int    rhsport;
extern char   rhs2host[MAXHOSTNMLEN];
extern int    rhs2port;
extern int    debug;
extern int    allexiting;
extern char   myid[IDSIZE];
extern int    my_listener_port;
extern char   mynickname[MAXHOSTNMLEN];
extern int    rhs_idx;
extern int    lhs_idx;
extern struct keyval_pairs keyval_tab[64];
extern int    keyval_tab_idx;
extern char   mpd_passwd[PASSWDLEN];

void handle_lhs_input( int idx )
{
    int  fwd, length, exec;
    char message[MAXLINE];
    char srcid[MAXLINE];
    char destid[MAXLINE];
    char bcastval[MAXLINE];
    char cmdval[MAXLINE];
    char fwdbuf[MAXLINE];

    mpdprintf( 0, "handling lhs input\n" );
    if ( (length = read_line(fdtable[idx].fd, message, MAXLINE ) ) != 0 ) {
        mpdprintf( debug, "message from lhs to handle =:%s: (read %d)\n", 
		   message, length );
	/* parse whole message */ 
	strcpy( fwdbuf, message );             
	parse_keyvals( message );
	/* dump_keyvals(); */
	getval( "src", srcid );
	getval( "dest", destid );
	getval( "bcast", bcastval );
	getval( "cmd", cmdval );
        if ( strlen(cmdval) == 0 ) 
	{
            mpdprintf( debug, "no command specified in msg\n" );
            return;
        }

	fwd = 0;
	if ( strcmp( bcastval, "true" ) == 0  &&  strcmp( srcid, myid ) != 0 )
	{
	    fwd = 1;
	}
	else if ( strcmp( destid, "anyone" ) != 0  && 
		  strcmp( destid, myid )     != 0  &&
	          strcmp( srcid, myid )      != 0 )
	{
	    fwd = 1;
	}
	if ( fwd )  {
	    mpdprintf( debug, "forwarding :%s: to :%s_%d:\n", fwdbuf, rhshost,rhsport );
	    write_line( rhs_idx, fwdbuf );
	}

	exec = 0;
	if ( strcmp( bcastval, "true" ) == 0 )
	    exec = 1;
	else if ( strcmp( destid, myid ) == 0 || strcmp( destid, "anyone" ) == 0 )
	     exec = 1;
	if (!exec)
	    return;

	if ( strcmp( cmdval, "ping" ) == 0 )
	    sib_ping();
	else if ( strcmp( cmdval, "ping_ack" ) == 0 )
	    sib_ping_ack();
	else if ( strcmp( cmdval, "ringtest" ) == 0 )
	    sib_ringtest();
	else if ( strcmp( cmdval, "trace" ) == 0 )
	    sib_trace();
	else if ( strcmp( cmdval, "trace_info" ) == 0 )
	    sib_trace_info();
	else if ( strcmp( cmdval, "trace_trailer" ) == 0 )
	    sib_trace_trailer();
	else if ( strcmp( cmdval, "dump" ) == 0 )
	    sib_dump();
	else if ( strcmp( cmdval, "mandump" ) == 0 )
	    sib_mandump();
	else if ( strcmp( cmdval, "rhs2info" ) == 0 )
	    sib_rhs2info( idx );
	else if ( strcmp( cmdval, "reconnect_rhs" ) == 0 )
	    sib_reconnect_rhs(idx);
	else if ( strcmp( cmdval, "listjobs" ) == 0 )
	    sib_listjobs();
	else if ( strcmp( cmdval, "listjobs_info" ) == 0 )
	    sib_listjobs_info();
	else if ( strcmp( cmdval, "listjobs_trailer" ) == 0 )
	    sib_listjobs_trailer();
	else if ( strcmp( cmdval, "signaljob" ) == 0 )
	    sib_signaljob();
	else if ( strcmp( cmdval, "killjob" ) == 0 )
	    sib_killjob();
	else if ( strcmp( cmdval, "exit" ) == 0 )
	    sib_exit();
	else if ( strcmp( cmdval, "allexit" ) == 0 )
	    sib_allexit();
	else if ( strcmp( cmdval, "mpexec" ) == 0 )
	    sib_mpexec();
	else if ( strcmp( cmdval, "jobsync" ) == 0 )
	    sib_jobsync();
	else if ( strcmp( cmdval, "jobgo" ) == 0 )
	    sib_jobgo();
	else if ( strcmp( cmdval, "bomb" ) == 0 )
	    sib_bomb();
	else if ( strcmp( cmdval, "debug" ) == 0 )
	    sib_debug();
	else if ( strcmp( cmdval, "needjobids" ) == 0 )
	    sib_needjobids();
	else if ( strcmp( cmdval, "newjobids" ) == 0 )
	    sib_newjobids();
	else
	    mpdprintf( 1, "invalid msg string from lhs = :%s:\n", fwdbuf );
        return;        
    }
    else { /* sibling gone away */
	mpdprintf( debug, "lost contact with sibling idx=%d fd=%d\n",
		   idx,fdtable[idx].fd); 
        dclose( fdtable[idx].fd ); 
        deallocate_fdentry( idx );
	if ( idx == lhs_idx)
	    lhs_idx = -1;
    }
}

/*
 *        Handler for console input
 */
void handle_console_input( idx )
int idx;
{
    char buf[MAXLINE];
    char parsebuf[MAXLINE];
    char errbuf[MAXLINE];
    char cmd[MAXLINE];

    mpdprintf( 0, "handling console input\n" );
    if ( read_line( fdtable[idx].fd, buf, MAXLINE ) != 0 ) {
        mpdprintf( debug, "mpd received :%s: from console\n", 
		   buf );
        /* get first word and branch accordingly, but pass whole buf */
	strcpy( parsebuf, buf );
	parse_keyvals( parsebuf );
	getval( "cmd", cmd );
	if ( cmd[0] ) {
	    if ( strcmp( cmd, "mpexec" ) == 0 )
		con_mpexec( );
	    else if ( strcmp( cmd, "ringtest" ) == 0 )
		con_ringtest( );
	    else if ( strcmp( cmd, "debug" ) == 0 )
		con_debug( );
	    else if ( strcmp( cmd, "trace" ) == 0 )
		con_trace( );
	    else if ( strcmp( cmd, "dump" ) == 0 )
		con_dump( );
	    else if ( strcmp( cmd, "mandump" ) == 0 )
		con_mandump( );
	    else if ( strcmp( cmd, "ping" ) == 0 )
		con_ping( );
	    else if ( strcmp( cmd, "bomb" ) == 0 )
		con_bomb( );
	    else if ( strcmp( cmd, "exit" ) == 0 )
		con_exit( );
	    else if ( strcmp( cmd, "allexit" ) == 0 )
		con_allexit( );
	    else if ( strcmp( cmd, "listjobs" ) == 0 )
		con_listjobs( );
	    else if ( strcmp( cmd, "signaljob" ) == 0 )
		con_signaljob( );
	    else if ( strcmp( cmd, "killjob" ) == 0 )
		con_killjob( );
	    else if ( strcmp( cmd, "addmpd" ) == 0 )
		con_addmpd( buf );  /* RMB: eliminate need for buf */
	    else {
		if ( strlen( buf ) > 1 ) {    /* newline already there */
		    sprintf( errbuf, "%s: %s", "invalid console buf\n", buf );
		    write_line( console_idx, errbuf );
		}
	    }
	}
        strcpy( buf, "ack_from_mpd\n" );
        write_line( console_idx, buf );
    }
    else {                        /* console gone away */
        mpdprintf( 0,
		   "eof on console fd; closing console fd %d idx=%d console_idx=%d\n",
		   fdtable[console_idx].fd, idx, console_idx ); 
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx ); 
	console_idx = -1;
    }
}

void handle_listener_input( int idx )
{
    int new_idx;
    
    mpdprintf( debug, "handling listener input, accept here\n" ); 
    new_idx = allocate_fdentry();
    strcpy( fdtable[new_idx].name, "temp" );
    fdtable[new_idx].fd      = accept_connection( fdtable[idx].fd );
    fdtable[new_idx].read    = 1;
    fdtable[new_idx].handler = NEWCONN;
    mpdprintf( debug, "accepted new tmp connection on %d\n", fdtable[new_idx].fd ); 
}

void handle_console_listener_input( int idx )
{
    int new_idx;
    
    mpdprintf( debug, "handling console listener input\n" );
    if ( console_idx == -1 ) {
	new_idx = allocate_fdentry();
	strcpy( fdtable[new_idx].name, "console" );
	fdtable[new_idx].fd      = accept_unix_connection( fdtable[idx].fd );
	fdtable[new_idx].read    = 1;
	fdtable[new_idx].write   = 0;
	fdtable[new_idx].handler = CONSOLE;
	mpdprintf( 0, "accepted new console connection on %d\n", fdtable[new_idx].fd );
	console_idx = new_idx;
    }
    else 
	mpdprintf( 0, "delaying new console connection\n" );
}

void handle_rhs_input( int idx )
{
    int n;
    char buf[MAXLINE], parse_buf[MAXLINE], cmd[MAXLINE];
 
    if ( allexiting )
	mpdprintf( debug, "ignoring eof on rhs since all mpd's are exiting\n" );
    else {
	buf[0] = '\0';
	n = read_line( fdtable[idx].fd, buf, MAXLINE );
	if ( n == 0 ) { /* EOF, next sib died */
	    mpdprintf( debug, "next sibling died; reconnecting to rhs2\n" );
	    dclose( fdtable[idx].fd );
	    fdtable[idx].fd      = network_connect( rhs2host, rhs2port );
	    fdtable[idx].read    = 1;
	    fdtable[idx].write   = 0;
	    fdtable[idx].handler = RHS;
	    strcpy( rhshost, rhs2host );
	    rhsport = rhs2port;
	    sprintf( buf, "src=%s dest=%s_%d cmd=new_lhs_req host=%s port=%d\n",
		     myid, rhshost, rhsport, mynickname, my_listener_port );
	    write_line( idx, buf );
	    recv_msg( fdtable[idx].fd, buf );
	    strcpy( parse_buf, buf );
	    parse_keyvals( parse_buf );
	    getval( "cmd", cmd );
	    if ( strcmp( cmd, "challenge" ) != 0 ) {
		mpdprintf( 1, "handle_rhs_input: expecting challenge, got %s\n", buf );
		exit( -1 );
	    }
	    newconn_challenge( idx );
	    /* special case logic */
	    if ( strcmp( lhshost, rhshost ) != 0  ||  lhsport != rhsport )
	    {
		sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
			 myid, lhshost, lhsport, rhs2host, rhs2port );
		write_line( idx, buf );
	    }
	    rhs_idx = idx;
	}
	else {
	    mpdprintf( 1, "unexpected non-EOF message on rhs; n=%d msg=:%s:\n",n,buf );
	}
    }
}

void handle_manager_input( int idx )
{
    int jobid;
    char buf[MAXLINE], cmdval[MAXLINE];

    mpdprintf( debug, "handling manager input\n" );
    if ( read_line( fdtable[idx].fd, buf, MAXLINE ) != 0 ) {
        mpdprintf( debug, "mpd handling msg from manager :%s\n", buf );
	parse_keyvals( buf );
	getval( "cmd", cmdval );
	/* handle msg from manager */
	if (strcmp(cmdval,"killjob") == 0)
	{
	    jobid = atoi( getval( "jobid", buf ) );
	    mpdprintf( debug, "handle_manager_input:  sending killjob jobid=%d\n", jobid );
            sprintf( buf, "src=%s bcast=true cmd=killjob jobid=%d\n", myid, jobid );
            write_line( rhs_idx, buf );
	}
	else if (strcmp(cmdval,"mandump_output") == 0)
	{
	    mpdprintf( 1, "mpd:  mandump_output not yet implemented\n" );
	}
	else
	    mpdprintf( 1, "mpd received unknown msg from manager :%s\n", buf );
    }
    else {                        /* manager gone away */
        mpdprintf( debug, "lost contact with manager %s\n", fdtable[idx].name );
        dclose( fdtable[idx].fd );
        deallocate_fdentry( idx );
    }
}

void handle_newconn_input( int idx )
{
    int n;
    char buf[MAXLINE], parse_buf[MAXLINE], cmdval[MAXLINE];
    
    buf[0] = '\0';
    n = read_line( fdtable[idx].fd, buf, MAXLINE );
    if ( n == 0 ) {
	mpdprintf( debug, "newconn died\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx ); 
	return;
    }
    mpdprintf( debug, "handling newconn msg=:%s:\n",buf ); 
    strcpy( parse_buf, buf );             
    parse_keyvals( parse_buf );
    getval( "cmd", cmdval );
    if ( strcmp( cmdval, "new_rhs_req" ) == 0 )
	newconn_new_rhs_req( idx );
    else if (strcmp( cmdval, "new_rhs" ) == 0 )
	newconn_new_rhs( idx );
    else if ( strcmp( cmdval, "new_lhs_req" ) == 0 )
	newconn_new_lhs_req( idx );
    else if ( strcmp( cmdval, "new_lhs" ) == 0 )
	newconn_new_lhs( idx );
    else if ( strcmp( cmdval, "challenge" ) == 0 )
	newconn_challenge( idx );
    else
	mpdprintf( 1, "invalid msg from newconn: msg=:%s:\n",buf );
}

void newconn_challenge( int idx )
{
    char buf[MAXLINE], encoded_num[16], type[MAXLINE];
    int  challenge_num;

    getval( "rand", buf );
    getval( "type", type );
    challenge_num = atoi( buf );
    encode_num( challenge_num, encoded_num );
    sprintf( buf, "cmd=%s dest=anyone encoded_num=%s host=%s port=%d\n",
	     type, encoded_num, mynickname, my_listener_port );
    write_line( idx, buf );
    mpdprintf( debug, "newconn_challenge: sent response=:%s:\n",buf );
}

/* A new mpd enters the ring by connecting to the listener port of an existing mpd and
   sending it a new_rhs_req message.
*/
void newconn_new_rhs_req( int idx )
{
    /* validate new mpd attempting to enter the ring */
    int newport;
    mpd_sockopt_len_t salen;
    struct timeval tv;
    struct hostent *hp;
    struct sockaddr_in sa;
    char buf[MAXLINE], challenge_buf[MAXLINE], newhost[MAXHOSTNMLEN], *fromhost;

    getval( "port", buf ); 
    newport = atoi( buf );
    getval( "host", newhost ); 

    /* validate remote host */
    salen = sizeof( sa );
    /* AIX wants namelen to be size_t */
    if ( getpeername( fdtable[idx].fd, (struct sockaddr *) &sa, &salen ) != 0 ) {
	mpdprintf( 1, "getpeername failed: %s\n", strerror( errno ) );
    }
    fromhost = inet_ntoa( sa.sin_addr );
    hp = gethostbyaddr( (char *) &sa.sin_addr,sizeof( sa.sin_addr ),(int) sa.sin_family );
    if (hp == NULL)
	mpdprintf( 1, "Cannot get host info for %s", fromhost );
    else {
	fromhost = hp->h_name;
	mpdprintf( debug, "accepted connection from %s\n", fromhost );
    }
    /* Someday, check this host name or its address against list of approved hosts */

    mpdprintf( debug, "got cmd=new_rhs_req host=%s port=%d\n", newhost, newport ); 
    gettimeofday( &tv, ( struct timezone * ) 0 );
    srandom( tv.tv_usec * 167.5 );
    fdtable[idx].rn = random( );
    sprintf( challenge_buf, "cmd=challenge dest=anyone rand=%d type=new_rhs\n", fdtable[idx].rn );
    write_line( idx, challenge_buf );
}

void newconn_new_lhs_req( int idx )
{
    /* validate new mpd attempting to enter the ring */
    int newport;
    mpd_sockopt_len_t salen;
    struct timeval tv;
    struct hostent *hp;
    struct sockaddr_in sa;
    char buf[MAXLINE], challenge_buf[MAXLINE], newhost[MAXHOSTNMLEN], *fromhost;

    getval( "port", buf ); 
    newport = atoi( buf );
    getval( "host", newhost ); 

    /* validate remote host */
    salen = sizeof( sa );
    if ( getpeername( fdtable[idx].fd, (struct sockaddr *) &sa, &salen ) != 0 ) {
	mpdprintf( 1, "getpeername failed: %s\n", strerror( errno ) );
    }
    fromhost = inet_ntoa( sa.sin_addr );
    hp = gethostbyaddr( (char *) &sa.sin_addr,sizeof( sa.sin_addr ),(int) sa.sin_family );
    if (hp == NULL)
	mpdprintf( 1, "Cannot get host info for %s", fromhost );
    else {
	fromhost = hp->h_name;
	mpdprintf( debug, "accepted connection from %s\n", fromhost );
    }
    /* Someday, check this host name or its address against list of approved hosts */

    mpdprintf( debug, "got cmd=new_lhs_req host=%s port=%d\n", newhost, newport ); 
    gettimeofday( &tv, ( struct timezone * ) 0 );
    srandom( tv.tv_usec * 167.5 );
    fdtable[idx].rn = random( );
    sprintf( challenge_buf, "cmd=challenge dest=anyone rand=%d type=new_lhs\n", fdtable[idx].rn );
    write_line( idx, challenge_buf );
}

void newconn_new_rhs( int idx ) 
{
    int  newport;
    char buf[MAXLINE], new_rhs[MAXHOSTNMLEN], encoded_num[16];

    getval( "host", new_rhs ); 
    getval( "port", buf ); 
    newport = atoi( buf );
    getval( "encoded_num", encoded_num );
    mpdprintf( debug, "newconn_new_rhs: host=%s port=%d, encoded_num=%s\n",
	       new_rhs, newport, encoded_num ); 
    /* validate response */
    encode_num( fdtable[idx].rn, buf );
    if ( strcmp( buf, encoded_num ) != 0 ) {
	/* response did not meet challenge */
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	return;
    }

    /* make this port our next sibling port */
    if ( rhs_idx != -1 ) {
	dclose( fdtable[rhs_idx].fd );
	deallocate_fdentry( rhs_idx );  /* dealloc old one */
    }

    rhs_idx = idx;                /* new one already alloced */
    fdtable[rhs_idx].portnum = newport;
    fdtable[rhs_idx].handler = RHS;
    strcpy(fdtable[rhs_idx].name,"next");
    fdtable[rhs_idx].read = 1;  /* in case of EOF, if he goes away */
    if ( strcmp( lhshost, mynickname ) == 0  &&  lhsport == my_listener_port ) {
        sprintf( buf, "src=%s dest=%s_%d cmd=reconnect_rhs rhshost=%s rhsport=%d rhs2host=%s rhs2port=%d\n",
                 myid, new_rhs, newport, rhshost, rhsport, new_rhs, newport );
    }
    else {
        sprintf( buf, "src=%s dest=%s_%d cmd=reconnect_rhs rhshost=%s rhsport=%d rhs2host=%s rhs2port=%d\n",
                 myid, new_rhs, newport, rhshost, rhsport, rhs2host, rhs2port );
    }
    write_line( rhs_idx, buf );
    strcpy( rhs2host, rhshost );   /* old rhs becomes rhs2 */
    rhs2port = rhsport;
    strcpy( rhshost, new_rhs );    /* install the new mpd */
    rhsport = newport;
    /***** next block is special case logic *****/
    if ( strcmp( lhshost, mynickname ) != 0  ||
         lhsport != my_listener_port )
    {
        sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
                 myid, lhshost, lhsport, rhshost, rhsport );
        write_line( rhs_idx, buf );
    }
    /* Now that we have an rhs, we can initialize the jobid pool,
       which might require sending messages.
    */
    init_jobids();		/* protected from executing twice */
}

void newconn_new_lhs( idx )
int idx;
{
    int  newport;
    char buf[MAXLINE], new_lhs[MAXHOSTNMLEN], encoded_num[16];

    getval( "host", new_lhs ); 
    newport = atoi( getval( "port", buf ) ); 
    getval( "encoded_num", encoded_num );
    mpdprintf( debug, "got cmd=new_lhs host=%s port=%d, encoded_num=%s\n",
	       new_lhs, newport, encoded_num ); 
    /* validate response */
    encode_num( fdtable[idx].rn, buf );
    if ( strcmp( buf, encoded_num ) != 0 ) {
	/* response did not meet challenge */
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	return;
    }

    if ( lhs_idx != -1 ) {
	dclose( fdtable[lhs_idx].fd );
	deallocate_fdentry( lhs_idx );  /* dealloc old one */
    }
    lhs_idx = idx;                /* new one already alloced */
    fdtable[lhs_idx].portnum = newport;
    fdtable[lhs_idx].handler = LHS;
    strcpy(fdtable[lhs_idx].name,"prev");
    fdtable[lhs_idx].read = 1;
    strcpy( lhshost,new_lhs );
    lhsport = newport;
    sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
	     myid, lhshost, lhsport, rhshost, rhsport );
    write_line( rhs_idx, buf );
}

