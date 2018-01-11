/*
 *	Process console command from user 
 */ 
#include "mpd.h"

extern struct fdentry fdtable[MAXFDENTRIES];
extern struct procentry proctable[MAXPROCS];
extern char   mydir[MAXLINE];
extern int    listener_idx;
extern int    console_idx;
extern int    console_listen_idx;
extern int    rhs_idx;
extern int    done;     		/* global done flag */
extern char   mynickname[MAXHOSTNMLEN];
extern char   nexthost[MAXHOSTNMLEN];   /* name of next host */ 
extern int    nextport;                 /* port that next host is listening on */
extern char   rhshost[MAXHOSTNMLEN];
extern int    rhsport;              
extern char   prevhost[MAXHOSTNMLEN];   /* name of prev host */
extern int    prevport;                 /* port that prev host is listening on */
extern char   myid[IDSIZE];
extern int    debug;
extern int    allexiting;

/*
 *	Execute command at multiple nodes, using manager process
 */
void con_mpexec( )
{
    char mpexecbuf[MAXLINE];
    char program[256];
    char buf[MAXLINE], eqbuf[80], locid[8], argid[8], envid[8];
    char console_hostname[MAXHOSTNMLEN];
    char username[MAXLINE];
    int  console_portnum, iotree;
    int  numprocs, jid, line_labels, shmemgrpsize;
    int  i, locc, envc, argc;
    int  groupid;
    char groups[5*MAXGIDS];

    getval( "hostname", console_hostname );
    getval( "portnum", buf );
    console_portnum = atoi( buf );
    getval( "iotree", buf );
    iotree = atoi( buf );
    getval( "line_labels", buf );
    line_labels = atoi( buf );
    getval( "numprocs", buf );
    numprocs = atoi( buf );
    getval( "shmemgrpsize", buf );
    shmemgrpsize = atoi( buf );
    getval( "executable", program );
    getval( "username", username );
    getval( "groupid", buf );
    groupid = atoi( buf );
    getval( "groups", groups );

    jid = allocate_jobid();
    mpdprintf( debug, "con_mpexec: new job id allocated = %d\n", jid );
    sprintf( buf, "jobid=%d\n", jid );
    write_line( console_idx, buf );

    /* hopcount is for checking that an mpexec message has gone around the ring without
       any processes getting started, which indicates a bad machine name in MPDLOC */
    sprintf(mpexecbuf,
	    "cmd=mpexec conhost=%s conport=%d rank=0 src=%s "
	    "iotree=%d dest=anyone job=%d jobsize=%d prog=%s hopcount=0 line_labels=%d "
	    "shmemgrpsize=%d username=%s groupid=%d, groups=%s ",
	    console_hostname, console_portnum, myid, iotree, jid, numprocs, program,
	    line_labels, shmemgrpsize, username, groupid, groups );

    /* now add other arguments, which are already in key=val form */
    if ( getval( "locc", buf ) )
	locc = atoi( buf );
    else
	locc = 0;
    sprintf( eqbuf, "locc=%s ", buf );
    strcat( mpexecbuf, eqbuf );
    for ( i = 1; i <= locc; i++ ) {
	sprintf( locid, "loc%d", i );
	getval( locid, buf );
	sprintf( eqbuf, "loc%d=%s ", i, buf );
	strcat( mpexecbuf, eqbuf );
    }

    if ( getval( "argc", buf ) )
	argc = atoi( buf );
    else
	argc = 0;
    sprintf( eqbuf, "argc=%s ", buf );
    strcat( mpexecbuf, eqbuf );
    for ( i=1; i <= argc; i++ ) {
	sprintf( argid, "arg%d", i );
	getval( argid, buf );
	sprintf( eqbuf, "arg%d=%s ", i, buf );
	strcat( mpexecbuf, eqbuf );
    }

    if ( getval( "envc", buf ) )
	envc = atoi( buf );
    else
	envc = 0;
    sprintf( eqbuf, "envc=%s ", buf );
    strcat( mpexecbuf, eqbuf );
    for ( i=1; i <= envc; i++ ) {
	sprintf( envid, "env%d", i );
	getval( envid, buf );
	sprintf( eqbuf, "env%d=%s ", i, buf );
	strcat( mpexecbuf, eqbuf );
    }

    mpexecbuf[strlen(mpexecbuf)-1] = '\n';
    mpdprintf( debug, "con_mpexec sending :%s:\n", mpexecbuf );
    write_line( rhs_idx, mpexecbuf );
}

void con_killjob( )
{
    char buf[MAXLINE];
    int  jobid;

    jobid = atoi( getval( "jobid", buf) );
    sprintf( buf, "src=%s bcast=true cmd=killjob jobid=%d\n", myid, jobid );
    write_line( rhs_idx, buf );
    mpdprintf( debug, "con_killjob: sending killjob jobid=%d\n", jobid );
}

void con_exit( )
{
    char buf[MAXLINE], mpd_id[IDSIZE];

    getval( "mpd_id", mpd_id );
    sprintf( buf, "src=%s dest=%s cmd=exit\n", myid, mpd_id );
    write_line( rhs_idx, buf );
}

void con_allexit( )
{
    char buf[MAXLINE];

    allexiting = 1;
    sprintf( buf, "src=%s bcast=true cmd=allexit\n", myid );
    write_line( rhs_idx, buf );
}

/* RMB: con_addmpd is woefully out of date */
void con_addmpd(command)
char *command;
{
    char *c;
    char rhostname[MAXHOSTNMLEN];
    char mpd_cmd[MAXLINE];
    char rsh_cmd[MAXLINE];

    strcpy( rsh_cmd, "rsh" );		  /* set remote shell command */
    c = strtok( command, "\n "); /* Throw command out */
    c = strtok( NULL, "\n ");
    if ( !c ) {
	mpdprintf( debug, "did not get expected hostname in addmpd command\n" );
	return;
    }
    strcpy( rhostname, c );
    sprintf( mpd_cmd, "%s/mpd", mydir );
    /* rsh another mpd onto specified host */
    {
	int rc;
	char l_port[6];
	sprintf( l_port, "%d", fdtable[listener_idx].portnum );
	rc = fork();	/* fork the rsh, which will run mpd remotely */
	if (rc == 0) {
	    rc = execlp( rsh_cmd, rsh_cmd, rhostname, "-n",
			 mpd_cmd,
			 "-h", mynickname,
			 "-p", l_port,
			 "-w", mydir,
			 NULL );
	    error_check( rc, "mpd: execlp failed" );
	}
	else {
	    mpdprintf( 0, "creating remote mpd on %s\n",rhostname);
	}
    }
}

void con_debug()
{
    char buf[MAXLINE], dest[MAXLINE], src[MAXLINE];
    int  flag;
	
    getval( "dest", dest );
    flag = atoi( getval( "flag", buf ) );
    if ( strcmp( dest, myid ) == 0 )  {
        debug = flag;
    }
    else  {
        getval( "src", src );
        sprintf( buf, "src=%s dest=%s cmd=debug flag=%d\n", myid, dest, flag );
        write_line( rhs_idx, buf );
    }
}

void con_ringtest( )
{
    char ringtestbuf[MAXLINE];
    int  count;
    char buf[MAXLINE];
    double timestamp;

    getval( "laps", buf );

    if ( buf[0] == '\0' ) {
	sprintf( buf, "must specify count for ringtest\n" );
	write_line( console_idx, buf );
	return;
    }
    count = atoi( buf );
    if ( count > 0 ) {
	/* send message around ring to self */
	timestamp = mpd_timestamp();
	sprintf( ringtestbuf, "src=%s dest=%s cmd=ringtest count=%d starttime=%f\n",
		 myid, myid, count, timestamp );
	write_line( rhs_idx, ringtestbuf );
    }
}

void con_trace( )
{
    char tracebuf[MAXLINE];
	
    /* send message to next mpd in ring; it will be forwarded all the way around */
    sprintf( tracebuf, "src=%s bcast=true cmd=trace\n", myid );
    write_line( rhs_idx, tracebuf );
    sprintf( tracebuf, "src=%s bcast=true cmd=trace_trailer\n", myid );
    write_line( rhs_idx, tracebuf );
}

void con_listjobs( )
{
    char listbuf[MAXLINE];
	
    /* send message to next mpd in ring; it will be forwarded all the way around */
    sprintf( listbuf, "src=%s bcast=true cmd=listjobs\n", myid );
    write_line( rhs_idx, listbuf );
    sprintf( listbuf, "src=%s bcast=true cmd=listjobs_trailer\n", myid );
    write_line( rhs_idx, listbuf );
}

void con_dump( )
{
    char dumpbuf[MAXLINE], what[80];

    getval( "what", what );
    mpdprintf( debug, "conproc sending dump message to rhs, src=%s, what=%s\n",
	       myid, what );
    sprintf( dumpbuf, "src=%s dest=anyone cmd=dump what=%s\n", myid, what );
    write_line( rhs_idx, dumpbuf );
}

void con_mandump( )
{
    char dumpbuf[MAXLINE], buf[MAXLINE], what[80];
    int  jobid, manrank;

    getval( "jobid", buf );
    jobid = atoi( buf );
    getval( "rank", buf );
    manrank = atoi( buf );
    getval( "what", what );
    mpdprintf( debug,
	       "conproc sending mandump message to rhs, src=%s, "
               "jobid=%d manrank=%d what=%s\n",
	       myid, jobid, manrank, what );
    sprintf( dumpbuf,
	     "src=%s dest=anyone cmd=mandump jobid=%d manrank=%d what=%s\n",
	     myid, jobid, manrank, what );
    write_line( rhs_idx, dumpbuf );
}

void con_ping( )
{
    char buf[MAXLINE];
    char *pingee_id;

    pingee_id = getval( "pingee", buf );
    if ( !pingee_id ) {
	mpdprintf( debug, "did not get expected id to ping\n" );
	return;
    }
    sprintf( buf, "src=%s dest=%s cmd=ping\n", myid, pingee_id );
    write_line( rhs_idx, buf );
}

/* cmd to cause an mpd to "fail" for testing */
void con_bomb( )
{
    char buf[MAXLINE], mpd_id[IDSIZE];

    getval( "mpd_id", mpd_id );
    sprintf( buf, "src=%s dest=%s cmd=bomb\n", myid, mpd_id );
    write_line( rhs_idx, buf );
}

void con_signaljob( )
{
    char c_signum[32];
    char buf[MAXLINE];
    int  jobid;

    jobid = atoi( getval( "jobid", buf) );
    strcpy( c_signum, getval( "signum", buf ) );
    sprintf( buf, "src=%s bcast=true cmd=signaljob jobid=%d signum=%s\n",
             myid, jobid, c_signum );
    write_line( rhs_idx, buf );
    mpdprintf( debug, "con_signaljob: signaling jobid=%d c_signum=%s\n",
               jobid, c_signum );
}

