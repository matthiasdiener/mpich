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
void con_mpexec(command)
char *command;
{
    char *c;
    char mpexecbuf[MAXLINE];
    char program[80];
    char buf[MAXLINE];
    char console_hostname[MAXHOSTNMLEN];
    int  console_portnum;
    int  numprocs, jid;
    int  pre_build_print_tree;

    c = strtok( command, " \n" );     /* should be mpexec */

    if ( !( c = strtok( NULL, " \n" ) ) ) { /* should be hostname where console is */
	sprintf( buf, "must specify hostname for mpexec\n" );
	write_line( console_idx, buf );
	return;
    }
    strcpy( console_hostname, c );

    if ( !( c = strtok( NULL, " \n" ) ) ) { /* should be port where console is listening*/
	sprintf( buf, "must specify console listen port for mpexec\n" );
	write_line( console_idx, buf );
	return;
    }
    console_portnum = atoi( c );

    if ( !( c = strtok( NULL, " \n" ) ) ) { /* should be pre_build_print_tree flag*/
	sprintf( buf, "must specify pre_build_print_tree flag for mpexec\n" );
	write_line( console_idx, buf );
	return;
    }
    pre_build_print_tree = atoi( c );

    if ( !( c = strtok( NULL, " \n" ) ) ) { /* should be numprocs */
	sprintf( buf, "must specify numprocs for mpexec\n" );
	write_line( console_idx, buf );
	return;
    }
    numprocs = atoi( c );	
    if ( !( c = strtok( NULL, " \n" ) ) ) {	 /* should be program */
	sprintf( buf,"must specify program for mpexec\n" );
	write_line( console_idx, buf );
	return;
    }
    strcpy( program, c );
    jid = allocate_jobid();
    mpdprintf( debug, "con_mpexec: new job id allocated = %d\n", jid );
    sprintf( buf, "job id is %d\n", jid );
    write_line( console_idx, buf );

    /* hopcount is for checking that an mpexec message has gone around the ring without
       any processes getting started, which indicates a bad machine name in MPDLOC */
    sprintf(mpexecbuf,
	    "cmd=mpexec conhost=%s conport=%d rank=0 src=%s "
	    "prebuildprinttree=%d "
	    "dest=anyone job=%d jobsize=%d prog=%s hopcount=0 ",
	    console_hostname, console_portnum, myid, 
	    pre_build_print_tree,
	    jid, numprocs, program);

    while (*c != '\0') /* skip pgm */ 
	c++;
    c++;		/* skip over \0 planted by strtok */
    if (*c)
	strcat( mpexecbuf, c );  /* add args, env, and newline */
    else
	strcat( mpexecbuf, "\n" );
    mpdprintf( debug, "con_mpexec sending :%s:\n", mpexecbuf );
    write_line( rhs_idx, mpexecbuf );
}

void con_pkill(command)
char *command;
{
    char *c;
    char pkillbuf[MAXLINE];
    char buf[MAXLINE];
    int  jid;

    c = strtok( command, " \n" );	 /* should be pkill */
    if ( !( c = strtok( NULL, " \n" ) ) ) {	 /* should be jid */
	sprintf( buf, "must specify job id for pkill\n" );
	write_line( console_idx, buf );
	return;
    }
    else {
	jid = atoi( c );
	/* broadcast killjob message */
	sprintf( pkillbuf, "src=%s bcast=true cmd=killjob job=%d\n", 
		 myid, jid );
	write_line( rhs_idx, pkillbuf );
	kill_job( jid, SIGINT );
    }
}

void con_exit(command)
char *command;
{
    char *c;
    char buf[MAXLINE];

    c = strtok( command, " " );	  /* should be "exit" */
    if ( !c ) {
	mpdprintf( debug, "did not get expected exit command\n" );
	return;
    }
    c = strtok( NULL , " \n" );	  /* should be id of mpd to exit */
    if ( !c ) {
	mpdprintf( debug, "did not get expected id to exit\n" );
	return;
    }
    sprintf( buf, "src=%s dest=%s cmd=exit\n", myid, c );
    write_line( rhs_idx, buf );
}

void con_allexit(command)
char *command;
{
    char *c;
    char buf[MAXLINE];

    c = strtok( command, " " );	  /* should be "allexit" */
    if ( !c ) {
	mpdprintf( debug, "did not get expected allexit command\n" );
	return;
    }
    allexiting = 1;
    sprintf( buf, "src=%s bcast=true cmd=allexit\n", myid );
    write_line( rhs_idx, buf );
}

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

void con_debug(command)
char *command;
{
    char *c, *to_id;
    char buf[MAXLINE];
    int  flag;
	
    c = strtok(command," \n" ); /* should be debug */
    to_id = strtok( NULL , " \n" );
    if ( !to_id ) {
	sprintf(buf,"must specify to-id for debug\n" );
	write_line( console_idx, buf );
	return;
    }
    c = strtok(NULL," \n" ); /* should be count */
    if (!c)
    {
	sprintf(buf,"must specify flag for debug cmd\n" );
	write_line( console_idx, buf );
	return;
    }
    else {
	flag = atoi( c );
	sprintf( buf, "src=%s dest=%s cmd=debug flag=%d\n",
		 myid, to_id, flag );
	write_line( rhs_idx, buf );
    }
}

void con_ringtest(command)
char *command;
{
    char *c;
    char ringtestbuf[MAXLINE];
    int  count;
    char buf[MAXLINE];
    double timestamp;

    c = strtok(command," \n" ); /* should be ringtest */
    c = strtok(NULL," \n" ); /* should be count */
    if (!c)
    {
	sprintf(buf,"must specify count for ringtest\n" );
	write_line( console_idx, buf );
	return;
    }
    else {
	count = atoi( c );
	if ( count > 0 ) {
	    /* send message around ring to self */
	    timestamp = mpd_timestamp();
	    sprintf( ringtestbuf, "src=%s dest=%s cmd=ringtest count=%d starttime=%f\n",
		     myid, myid, count, timestamp );
	    write_line( rhs_idx, ringtestbuf );
	}
    }
}

void con_trace( command )
char *command;
{
    char tracebuf[MAXLINE];
	
    /* send message to next mpd in ring; it will be forwarded all the way around */
    sprintf( tracebuf, "src=%s bcast=true cmd=trace\n", myid );
    write_line( rhs_idx, tracebuf );
}
void con_dump( command )
char *command;
{
    char dumpbuf[MAXLINE], what[80];
    char *c;

    c = strtok(command," \n" ); /* should be dump */
    c = strtok(NULL," \n" ); /* should be what to dump */
    strcpy( what, c );
    mpdprintf( debug, "conproc sending dump message to rhs, src=%s, what=%s\n",
	       myid, what );
    sprintf( dumpbuf, "src=%s dest=anyone cmd=dump what_to_dump=%s\n", myid, what );
    write_line( rhs_idx, dumpbuf );
}

void con_ping(command)
char *command;
{
    char *c;
    char buf[MAXLINE];
    char *pingee_id;

    c = strtok( command, " " );		  /* should be "ping" */
    if ( !c ) {
	mpdprintf( debug, "did not get expected ping command\n" );
	return;
    }
    pingee_id = strtok( NULL , " \n" );
    if ( !pingee_id ) {
	mpdprintf( debug, "did not get expected id to ping\n" );
	return;
    }
    sprintf( buf, "src=%s dest=%s cmd=ping\n", myid, pingee_id );
    write_line( rhs_idx, buf );
}

/* cmd to cause an mpd to "fail" for testing */
void con_bomb(command)
char *command;
{
    char *c;
    char buf[MAXLINE];

    c = strtok( command, " " );	  /* should be "bomb" */
    if ( !c ) {
	mpdprintf( debug, "did not get expected bomb command\n" );
	return;
    }
    c = strtok( NULL , " \n" );	  /* should be id of mpd to bomb */
    if ( !c ) {
	mpdprintf( debug, "did not get expected id to bomb\n" );
	return;
    }
    sprintf( buf, "src=%s dest=%s cmd=bomb\n", myid, c );
    write_line( rhs_idx, buf );
}
