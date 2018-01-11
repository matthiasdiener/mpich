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
extern int keyval_tab_idx;

void handle_lhs_input( idx )
int idx;
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
        mpdprintf( debug, "message from lhs to handle =:%s:\n", message );
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
	if (fwd && strcmp(cmdval,"trace") != 0)  {
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

	if ( strcmp( cmdval, "new_rhs" ) == 0 )
	    sib_new_rhs(idx);
	else if ( strcmp( cmdval, "ping" ) == 0 )
	    sib_ping();
	else if ( strcmp( cmdval, "ping_ack" ) == 0 )
	    sib_ping_ack();
	else if ( strcmp( cmdval, "ringtest" ) == 0 )
	    sib_ringtest();
	else if ( strcmp( cmdval, "trace" ) == 0 )
	    sib_trace();
	else if ( strcmp( cmdval, "dump" ) == 0 )
	    sib_dump();
	else if ( strcmp( cmdval, "trace_ack" ) == 0 )
	    sib_trace_ack();
	else if ( strcmp( cmdval, "new_lhs" ) == 0 )
	    sib_new_lhs(idx);
	else if ( strcmp( cmdval, "rhs2info" ) == 0 )
	    sib_rhs2info(idx);
	else if ( strcmp( cmdval, "reconnect_rhs" ) == 0 )
	    sib_reconnect_rhs(idx);
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
	else if ( strcmp( cmdval, "findclient" ) == 0 )
	    sib_findclient();
	else if ( strcmp( cmdval, "foundclient" ) == 0 )
	    sib_foundclient();
	else if ( strcmp( cmdval, "debug" ) == 0 )
	    sib_debug();
	else if ( strcmp( cmdval, "needjobids" ) == 0 )
	    sib_needjobids();
	else if ( strcmp( cmdval, "newjobids" ) == 0 )
	    sib_newjobids();
	else
	    mpdprintf( 1, "invalid msg string from lhs = :%s:\n", fwdbuf );
	if (fwd && strcmp(cmdval,"trace") == 0)  {
	    mpdprintf( debug, "forwarding after trace:%s: to :%s_%d:\n",
                       fwdbuf, rhshost,rhsport );
	    write_line( rhs_idx, fwdbuf );
	}
        return;        
    }
    else { /* sibling gone away */
	mpdprintf( debug, "lost contact with sibling port[%d] fd=\n",
		   idx,fdtable[idx].fd); 
        dclose( fdtable[idx].fd ); 
        deallocate_fdentry( idx );
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
    char *cmd;

    mpdprintf( 0, "handling console input\n" );
    if ( read_line( fdtable[idx].fd, buf, MAXLINE ) != 0 ) {
        mpdprintf( debug, "mpd received :%s: from console\n", buf );
        /* get first word and branch accordingly, but pass whole buf */
	strcpy( parsebuf, buf );
	cmd = strtok( parsebuf, "\n " );
	if ( cmd ) {
	    if ( strcmp( cmd, "mpexec" ) == 0 )
		con_mpexec( buf );
	    else if ( strcmp( cmd, "pkill" ) == 0 )
		con_pkill( buf );
	    else if ( strcmp( cmd, "ringtest" ) == 0 )
		con_ringtest( buf );
	    else if ( strcmp( cmd, "debug" ) == 0 )
		con_debug( buf );
	    else if ( strcmp( cmd, "trace" ) == 0 )
		con_trace( buf );
	    else if ( strcmp( cmd, "dump" ) == 0 )
		con_dump( buf );
	    else if ( strcmp( cmd, "ping" ) == 0 )
		con_ping( buf );
	    else if ( strcmp( cmd, "bomb" ) == 0 )
		con_bomb( buf );
	    else if ( strcmp( cmd, "exit" ) == 0 )
		con_exit( buf );
	    else if ( strcmp( cmd, "allexit" ) == 0 )
		con_allexit( buf );
	    else if ( strcmp( cmd, "addmpd" ) == 0 )
		con_addmpd( buf );
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
        mpdprintf( debug, "lost contact with console; closing console fd %d\n",
		   fdtable[console_idx].fd ); 
	dclose( fdtable[console_idx].fd );
	deallocate_fdentry( console_idx ); 
	console_idx = -1;
    }
}

void handle_listener_input( idx )
int idx;
{
    int new_idx;
    
    mpdprintf( debug, "handling listener input,accept here\n" ); 
    new_idx = allocate_fdentry();
    strcpy( fdtable[new_idx].name, "temp" );
    fdtable[new_idx].fd      = accept_connection( fdtable[idx].fd );
    fdtable[new_idx].read    = 1;
    fdtable[new_idx].handler = LHS;
    mpdprintf( debug, "accepted new tmp connection on %d\n", fdtable[new_idx].fd ); 
}

void handle_console_listener_input( idx )
int idx;
{
    int new_idx;
    
    mpdprintf( debug, "handling console listener input\n" );
    new_idx = allocate_fdentry();
    strcpy( fdtable[new_idx].name, "console" );
    fdtable[new_idx].fd      = accept_unix_connection( fdtable[idx].fd );
    fdtable[new_idx].read    = 1;
    fdtable[new_idx].write   = 0;
    fdtable[new_idx].handler = CONSOLE;
    mpdprintf( 0, "accepted new console connection on %d\n", fdtable[new_idx].fd );
    console_idx = new_idx;
}

void handle_rhs_input( idx )
int idx;
{
    char buf[MAXLINE];
 
    if ( allexiting )
	mpdprintf( debug, "ignoring eof on rhs since all mpd's are exiting\n" );
    else {
	if ( read_line( fdtable[idx].fd, buf, MAXLINE ) == 0 ) { /* EOF, next sib died */
	    mpdprintf( debug, "next sibling died; reconnecting to rhs2\n" );
	    dclose( fdtable[idx].fd );
	    fdtable[idx].fd      = network_connect( rhs2host, rhs2port );
	    fdtable[idx].read    = 1;
	    fdtable[idx].write   = 0;
	    fdtable[idx].handler = RHS;
	    /* RMB: NEW special case logic */
	    if ( strcmp( lhshost, rhshost ) != 0  ||  lhsport != rhsport )
	    {
		sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
			 myid, lhshost, lhsport, rhs2host, rhs2port );
		write_line( idx, buf );
	    }
	    strcpy( rhshost, rhs2host );
	    rhsport = rhs2port;
	    sprintf( buf, "src=%s dest=%s_%d cmd=new_lhs host=%s port=%d\n",
		     myid, rhs2host, rhs2port, mynickname, my_listener_port );
	    write_line( idx, buf );
	}
	else {
	    mpdprintf( debug, "unexpected non-EOF message on rhs\n" );
	}
    }
}

void handle_manager_input( idx )
int idx;
{
    char buf[MAXLINE];

    mpdprintf( debug, "handling manager input\n" );
    if ( read_line( fdtable[idx].fd, buf, MAXLINE ) != 0 ) {
        mpdprintf( 1, "mpd received unexpected msg from manager :%s\n", buf );
    }
    else {                        /* manager gone away */
        mpdprintf( debug, "lost contact with manager %s\n", fdtable[idx].name );
        dclose( fdtable[idx].fd );
        deallocate_fdentry( idx );
    }
}
