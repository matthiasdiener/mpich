#include "mpd.h"

extern struct fdentry fdtable[MAXFDENTRIES];
extern struct procentry proctable[MAXPROCS];
extern struct jobentry jobtable[MAXJOBS];
extern int    amfirst;
extern int    console_idx;
extern int    rhs_idx;
extern int    prev_sibling_idx;
extern int    listener_idx;
extern int    done;                     /* global done flag */
extern char   mynickname[MAXHOSTNMLEN];
extern char   myhostname[MAXHOSTNMLEN];
extern int    my_listener_port;
extern char   rhshost[MAXHOSTNMLEN];
extern int    rhsport;
extern char   rhs2host[MAXHOSTNMLEN];
extern int    rhs2port;
extern char   myid[IDSIZE];
extern char   lhshost[MAXHOSTNMLEN];
extern int    lhsport;
extern int    debug;
extern int    allexiting;
extern struct keyval_pairs keyval_tab[64];
extern int keyval_tab_idx;

void sib_new_rhs(idx) 
int idx;
{
    int  newport;
    char buf[MAXLINE];
    char new_rhs[MAXHOSTNMLEN];

    getval("host",new_rhs); 
    getval("port",buf); 
    newport = atoi( buf );
    mpdprintf( debug, "got new_rhs cmd host=%s port=%d\n",
             new_rhs, newport ); 
    /* make this port our next sibling port */
    if ( rhs_idx != -1 ) {
	dclose( fdtable[rhs_idx].fd );
	deallocate_fdentry(rhs_idx);  /* dealloc old one */
    }
    rhs_idx = idx;                /* new one already alloced */
    fdtable[rhs_idx].portnum = newport;
    fdtable[rhs_idx].handler = RHS;
    strcpy(fdtable[rhs_idx].name,"next");
    fdtable[rhs_idx].read = 1;  /* in case of EOF, if he goes away */
    sprintf( buf, "src=%s dest=%s_%d cmd=reconnect_rhs rhshost=%s rhsport=%d rhs2host=%s rhs2port=%d\n",
             myid, new_rhs, newport, rhshost, rhsport, rhs2host, rhs2port );
    write_line( rhs_idx, buf );
    strcpy( rhs2host, rhshost );   /* old rhs becomes rhs2 */
    rhs2port = rhsport;
    strcpy( rhshost, new_rhs );    /* install the new mpd */
    rhsport = newport;
    /***** RMB: next block is special case logic (only one mpd in ring) *****/
    if ( strcmp( lhshost, mynickname ) != 0  ||
         lhsport != my_listener_port )
    {
        sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
                 myid, lhshost, lhsport, rhshost, rhsport );
        write_line( rhs_idx, buf );
    }
    /*****/
    /* Now that we have an rhs, we can initialize the jobid pool, which might require
       sending messages.
    */
    init_jobids();		/* protected from executing twice */
}

void sib_reconnect_rhs(idx) 
int idx;
{
    int  newport;
    char buf[MAXLINE];
    char new_rhs[MAXHOSTNMLEN];

    mpdprintf( debug, "entering reconnect_rhs\n");
    getval("rhshost",new_rhs);
    getval("rhsport",buf);
    newport = atoi( buf );
    strcpy(rhshost,new_rhs);
    rhsport = newport;
    getval("rhs2host",buf);
    strcpy( rhs2host, buf );
    getval("rhs2port",buf);
    rhs2port = atoi(buf);

    if ( rhs_idx == -1 )
	rhs_idx = allocate_fdentry();
    else
        dclose( fdtable[rhs_idx].fd );
    fdtable[rhs_idx].fd = network_connect(new_rhs,newport);
    fdtable[rhs_idx].active = 1;  /* in case a new one */
    fdtable[rhs_idx].read = 1;
    fdtable[rhs_idx].write   = 0;
    fdtable[rhs_idx].handler = RHS;
    fdtable[rhs_idx].portnum = newport;
    strcpy( fdtable[rhs_idx].name, new_rhs );
    /* RMB: send my rhs a new_lhs msg */
    sprintf( buf, "src=%s dest=%s_%d cmd=new_lhs host=%s port=%d\n",
             myid, rhshost, rhsport,
             mynickname, my_listener_port );
    write_line( rhs_idx, buf );
    /* Now that we have an rhs, we can initialize the jobid pool, which might require
       sending messages.
    */
    init_jobids();		/* protected from executing twice */
}

void sib_new_lhs( idx )
int idx;
{
    char buf[MAXLINE];

    getval( "host", lhshost );
    lhsport = atoi( getval( "port", buf ) );
    mpdprintf( debug, "newlhs = %s,%d\n", lhshost, lhsport );
    mpdprintf( debug, "SENDING RHS2INFO= :%s: :%d: dest=%s_%d\n",
             rhshost, rhsport, lhshost, lhsport);
    sprintf( buf, "src=%s dest=%s_%d cmd=rhs2info rhs2host=%s rhs2port=%d\n",
             myid, lhshost, lhsport, rhshost, rhsport );
    write_line( rhs_idx, buf );
    /***** RMB ?????
    strcpy( rhs2host, mynickname );
    rhs2port = my_listener_port;
    *****/
}

void sib_rhs2info( idx )
int idx;
{
    char buf[10];

    getval("rhs2host", rhs2host );
    rhs2port = atoi( getval( "rhs2port", buf ) );
}

void sib_killjob()
{ 
    int  jid;
    char buf[MAXLINE];

    getval( "job", buf );
    jid = atoi( buf );
    kill_job( jid, SIGINT );
}

void sib_bomb()
{
    mpdprintf( debug, "%s bombing\n", myid );
    exit(1);  /* not graceful; mimic machine dying etc. */
}

void sib_exit()
{
    done = 1;
}

void sib_allexit()
{
    allexiting = 1;
    done       = 1;
}

void sib_debug()
{
    char buf[MAXLINE];

    getval( "flag", buf );
    debug = atoi( buf );
    mpdprintf( debug, "[%s] debugging set to %d\n", myid, debug );
}

void sib_mpexec()		/* designed to work with process managers */
{
    char buf[MAXLINE], fwdbuf[MAXLINE], 
	 temparg[MAXLINE], argid[MAXLINE],
	 tempenv[MAXLINE], envid[MAXLINE],
	 temploc[MAXLINE], locid[MAXLINE];
    char src[MAXLINE], program[MAXLINE];
    int  pid, i, rc, argc, envc, locc;
    char *argv[25];
    char *env[50];
    int  jobrank, jobid, jobsize, jidx, cid; 
    char conhost[MAXHOSTNMLEN];
    int  conport;
    int  man_mpd_socket[2];
    int  man_idx;
    char env_mpd_fd[80], env_job[80], env_rank[80], env_size[80], env_cid[80];
    char env_man_pgm[256], env_man_listener_fd[80], env_man_prevhost[MAXHOSTNMLEN];
    char env_man_prevport[80], env_man_host0[MAXHOSTNMLEN], env_man_port0[80];
    char env_man_conport[80], env_man_conhost[MAXHOSTNMLEN], env_man_debug[80];
    char env_man_prebuildprinttree[80];
    int  man_listener_fd, man_listener_port;
    char host0[MAXHOSTNMLEN], prevhost[MAXHOSTNMLEN];
    int  port0, prevport;
    char host0_next_man[MAXHOSTNMLEN];
    int  port0_next_man;
    struct passwd *pwent;
    int hopcount, pre_build_print_tree, do_mpexec_here;

    mpdprintf( debug, "sib_mpexec: entering\n");
    
    getval( "job", buf );
    jobid = atoi( buf );
    getval( "jobsize", buf );
    jobsize = atoi( buf );
    getval( "prog", program );
    getval( "rank", buf );
    jobrank = atoi( buf );
    getval( "conhost", conhost ); 
    getval( "conport", buf );
    conport = atoi( buf );
    getval( "src", src );
    getval( "hopcount", buf );
    hopcount = atoi( buf );
    getval( "prebuildprinttree", buf );
    pre_build_print_tree = atoi( buf );

    if ( jobrank >= jobsize ) {
	mpdprintf( debug, "mpexec returning, all processes previously forked\n" );
	return;			/* all processes already forked upstream */
    }

    /* This is to stop an infinite loop when the user has specified in -MPDLOC- only
       invalid machines */
    if ( ( hopcount > 1 ) && (jobrank == 0 ) && ( strcmp( src, myid ) == 0 ) ) {
	mpdprintf( 1, "could not start any processes\n" );
	return;
    } 

    do_mpexec_here = 0;

    if ( getval( "locc", buf ) )
	locc = atoi( buf );
    else
	locc = 0;

    if ( locc == 0 )
	do_mpexec_here = 1;
    else {
	for (i=1; i <= locc; i++) {
	    sprintf(locid,"loc%d",i);
	    getval(locid,buf);
	    destuff_arg(buf,temploc);
	    if ( ( strcmp( temploc, myhostname ) == 0 ||
		   strcmp( temploc, mynickname ) == 0 ) ) {
		do_mpexec_here = 1;
		break;
	    }
	}
    }

    if ( ! do_mpexec_here ) {
	sprintf( buf, "%d", hopcount + 1 );
	chgval( "hopcount", buf );
	reconstruct_message_from_keyvals( fwdbuf );
	mpdprintf( debug, "fwding mpexec cmd instead of execing it; fwdbuf=%s\n", fwdbuf );
	write_line( rhs_idx, fwdbuf );
	return;
    }

    mpdprintf( debug, "executing mpexec here\n" );

    /* This will be the general-purpose listener for the manager.  It is acquired now
       so that the manager will have it ready before the next manager to the right, if
       there is one, attempts to connect on it.
       */
    man_listener_fd = setup_network_socket( &man_listener_port );

    /* For rank 0, the incoming mpexec command formulated by conproc does not have
       (host0, port0) (since it doesn't know), or (prevhost,prevport) (since they
       don't exist). */

    if ( jobrank == 0 ) {
	if ( jobsize == 1 ) {
	    strcpy( prevhost, myhostname );
	    prevport = man_listener_port;
	}
	else {
	    strcpy( prevhost, DUMMYHOSTNAME );
	    prevport = DUMMYPORTNUM;
	}
	
	strcpy( host0, DUMMYHOSTNAME );
	port0 = DUMMYPORTNUM;
	
	strcpy( host0_next_man, myhostname );
	port0_next_man = man_listener_port;
    }
    else {
	getval( "prevhost", prevhost );
	getval( "prevport", buf );
	prevport = atoi( buf );

	if ( jobrank == (jobsize-1) ) {   /* if I am rightmost in ring */
	    getval( "host0", host0 );
	    getval( "port0", buf );
	    port0 = atoi( buf );
	}
	else {
	    strcpy( host0, DUMMYHOSTNAME );
	    port0 = DUMMYPORTNUM;
	}

        strcpy( host0_next_man, getval( "host0",buf ) );
	port0_next_man = atoi( getval( "port0",buf ) );
    }

    sprintf(fwdbuf,
	    "cmd=mpexec conhost=%s conport=%d host0=%s port0=%d prevhost=%s prevport=%d "
	    "prebuildprinttree=%d "
	    "rank=%d src=%s dest=anyone job=%d jobsize=%d prog=%s hopcount=%d",
	    conhost, conport, host0_next_man, port0_next_man, myhostname,
	    man_listener_port,
	    pre_build_print_tree,
	    jobrank + 1, src, jobid, jobsize, program, hopcount + 1 );
    /* no newline in above buffer because we are not finished adding things to it */

    /* set up locations for fwded message; locc already parsed above */

    if (locc > 0) {
	sprintf( buf, " locc=%d", locc );
	strcat( fwdbuf, buf);
	for ( i=1; i <= locc; i++ ) {
	    sprintf( locid, "loc%d", i );
	    getval( locid, temploc);
	    sprintf(buf," loc%d=%s", i, temploc );
	    strcat( fwdbuf, buf );
	}
    }

    argv[0] = MANAGER_PATHNAME;
    env[0]  = NULL;       /* in case there are no env strings */

    getval("argc",buf);
    argc = atoi(buf);
    if (argc > 0) {
	strcat(fwdbuf," argc=");
	strcat(fwdbuf,buf);
    }
    for (i=1; i <= argc; i++) {
	sprintf(argid,"arg%d",i);
	getval(argid,temparg);
	sprintf(buf," arg%d=%s",i,temparg);
	strcat(fwdbuf,buf);
        argv[i] = (char *)malloc(MAXLINE);
	destuff_arg(temparg,argv[i]);
    }
    argv[i] = NULL;

    getval( "envc", buf );
    envc = atoi( buf );
    if (envc > 0) {
	strcat(fwdbuf," envc=");
	strcat(fwdbuf,buf);
    }
    for (i=0; i < envc; i++) {
	sprintf(envid,"env%d",i+1);
	getval(envid,tempenv);
	sprintf(buf," env%d=%s",i+1,tempenv);
	strcat(fwdbuf,buf);
	env[i] = (char *)malloc(MAXLINE);
	destuff_arg(tempenv,env[i]);
    }
    /* set   env[i] = NULL;    below */

    strcat( fwdbuf, "\n" );
    mpdprintf( debug, "sib_mpexec: sending to rhs: :%s:\n", fwdbuf );
    write_line( rhs_idx, fwdbuf );

    if ((pwent = getpwuid(getuid())) == NULL)
    {
	printf("sib_mpexec: getpwuid failed");
	exit(99);
    }

    if ( ( jidx = find_jobid_in_jobtable(jobid) ) < 0 )
    {
	if ( ( jidx = allocate_jobent() ) < 0 ) {
	    mpdprintf( 1, "sib_mpexec: could not find empty slot in jobtable\n" );
	    exit(-1);
	}
    }
    jobtable[jidx].jobid = jobid;
    jobtable[jidx].jobsize = jobsize;
    mpdprintf( debug, "sib_mpexec: jobid=%d in jobtable at jidx=%d: \n",jobid,jidx );

    /* set up socket for mpd-manager communication */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, man_mpd_socket) < 0) 
	error_check( -1, "could not create socketpair to manager" );
    else {
	man_idx			 = allocate_fdentry();
	fdtable[man_idx].fd	 = man_mpd_socket[0];
	fdtable[man_idx].read	 = 1;
	fdtable[man_idx].write	 = 0;
	fdtable[man_idx].handler = MANAGER;
	sprintf( fdtable[man_idx].name, "manager_%d", jobrank );
    }
    mpdprintf( debug, "socketpair for manager is %d and %d\n",
	       man_mpd_socket[0], man_mpd_socket[1] );

    /* plant environment variables for client process */
    cid = allocate_procent();
    sprintf(env_job,  "MPD_JID=%d", jobid );
    env[i++] = env_job; 
    sprintf(env_rank, "MPD_JRANK=%d", jobrank );
    env[i++] = env_rank; 
    sprintf(env_size, "MPD_JSIZE=%d", jobsize );
    env[i++] = env_size; 
    sprintf(env_cid, "MPD_JCID=%d", cid );
    env[i++] = env_cid; 
    /* plant environment variables for manager process */
    sprintf( env_man_pgm, "MAN_CLIENT_PGM=%s", program );
    env[i++] = env_man_pgm;
    sprintf( env_mpd_fd, "MAN_MPD_FD=%d", man_mpd_socket[1] );
    env[i++] = env_mpd_fd; 
    sprintf( env_man_listener_fd, "MAN_LISTENER_FD=%d", man_listener_fd );
    env[i++] = env_man_listener_fd;
    sprintf( env_man_prevhost, "MAN_PREVHOST=%s", prevhost );
    env[i++] = env_man_prevhost;
    sprintf( env_man_prevport, "MAN_PREVPORT=%d", prevport );
    env[i++] = env_man_prevport;
    sprintf( env_man_host0, "MAN_HOST0=%s", host0 );
    env[i++] = env_man_host0;
    sprintf( env_man_port0, "MAN_PORT0=%d", port0 );
    env[i++] = env_man_port0;
    sprintf( env_man_conhost, "MAN_CONHOST=%s", conhost );
    env[i++] = env_man_conhost;
    sprintf( env_man_conport, "MAN_CONPORT=%d", conport );
    env[i++] = env_man_conport;
    sprintf( env_man_debug, "MAN_DEBUG=%d", debug );
    env[i++] = env_man_debug;
    sprintf( env_man_prebuildprinttree, "MAN_PRE_BUILD_PRINT_TREE=%d", pre_build_print_tree );
    env[i++] = env_man_prebuildprinttree;
    env[i] = NULL;

    proctable[cid].jobid = jobid;
    proctable[cid].jobrank = jobrank;
    proctable[cid].state = CLSTART;
    strcpy(proctable[cid].name,program);

    Signal( SIGCHLD, sigchld_handler );
    mpdprintf( debug, "starting program %s\n", MANAGER_PATHNAME );
    pid = fork();
    proctable[cid].pid = pid;
    if ( pid < 0 ) {
	mpdprintf( 1, "could not fork manager\n" );
        deallocate_procent( cid );
    }
    else if ( pid == 0 ) {                  /* child */
	sprintf( myid, "man_%d_before_exec", jobrank );
	mpdprintf( debug, "manager before exec closing fd %d\n", man_mpd_socket[0] );
	dclose( man_mpd_socket[0] );
	rc = execve( MANAGER_PATHNAME, argv, env );
        error_check( rc, "execve");
    }
    else {       		/* parent mpd */
        dclose( man_listener_fd );	/* close listener fd setup on behalf of manager */
	dclose( man_mpd_socket[1] );
	i = 1;  /* argv[0] wasn't malloc'd */
	while (argv[i])
	    free(argv[i++]);
	for (i=0; i < envc; i++)
	    free(env[i]);
    }
}

void sib_jobsync()
{
    char buf[MAXLINE], src[MAXLINE];
    int i, n, jobid, jobsize, sofar, jidx, num_here_in_job;

    getval( "job", buf );
    jobid = atoi( buf );
    getval( "jobsize", buf );
    jobsize = atoi( buf );
    getval( "sofar", buf );
    sofar = atoi( buf );
    getval( "src", src );

    mpdprintf( debug, "sib_jobsync: entering with jobid=%d, jobsize=%d, sofar=%d\n", jobid, jobsize, sofar );

    if ( sofar == jobsize ) {
	sprintf( buf, "src=%s bcast=true cmd=jobgo job=%d\n", myid, jobid ); 
	mpdprintf( debug, "sib_jobsync: sending jobgo! job=%d\n", jobid );
	write_line( rhs_idx, buf );
	return;
    }

    for ( jidx = 0; jidx < MAXJOBS; jidx++ )
    {
	if ( jobtable[jidx].active  &&  jobtable[jidx].jobid == jobid )
	    break;
    }
    if ( jidx >= MAXJOBS ) {
	mpdprintf( 1, "sib_jobsync: could not find jobid=%d in table\n",jobid );
	exit(-1);
    }

    mpdprintf( debug,"sib_jobsync: setting jobsync_is_here for jobid=%d at jidx=%d \n",
	       jobid, jidx );
    jobtable[jidx].jobsync_is_here = 1;
    for ( i=0,num_here_in_job=0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active && proctable[i].jobid == jobid ) {
	    num_here_in_job++;
	}
    }
    jobtable[jidx].alive_in_job_sofar = sofar;
    if (num_here_in_job == jobtable[jidx].alive_here_sofar)
    {
	n = jobtable[jidx].alive_here_sofar - jobtable[jidx].added_to_job_sofar;
	sofar += n;
	jobtable[jidx].added_to_job_sofar += n;
	sprintf( buf, "src=%s dest=anyone cmd=jobsync job=%d jobsize=%d sofar=%d\n",
		 myid, jobid, jobsize, sofar );
	mpdprintf( debug, "sib_jobsync: sending jobsync: job=%d sofar=%d\n", jobid,sofar );
	write_line( rhs_idx, buf );
	jobtable[jidx].jobsync_is_here = 0;
    }
}


void sib_jobgo()
{
    char buf[MAXLINE];
    int i, jobid;

    getval( "job", buf );
    jobid = atoi( buf );
    for ( i = 0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active && ( proctable[i].jobid == jobid ) ) {
	    proctable[i].state = CLRUNNING;
	    mpdprintf( debug, "jobgo: sending go to client for job=%d, rank=%d\n",
		       jobid, proctable[i].jobrank );
	    sprintf( buf, "cmd=go\n" );
	    send_msg( proctable[i].clientfd, buf, strlen( buf ) );
	}
    }
}

void sib_ringtest()
{
    int  count;
    char buf[MAXLINE];
    char srcid[IDSIZE];
    char destid[IDSIZE];
    char timestamp[80];
    double time1, time2;

    getval( "count", buf );                 
    count = atoi( buf );
    getval( "src", srcid );
    getval( "dest", destid );
    getval( "starttime", timestamp );

    mpdprintf( debug, "ringtest myid=%s count=%d starttime=%s\n",
	       myid, count, timestamp ); 
    if ( strcmp( destid, myid ) == 0 ) {
        count--;
        if ( count <= 0 ) {
	    time2 = mpd_timestamp();
	    time1 = atof( timestamp );
            sprintf( buf, "ringtest completed in %f seconds\n", time2 - time1 );
            write_line( console_idx, buf );
        }
    }
    if ( count > 0 ) {
        sprintf( buf,"src=%s dest=%s cmd=ringtest count=%d starttime=%s\n",
                srcid, destid, count, timestamp );
        write_line( rhs_idx, buf );
    }
}

void sib_trace()
{
    char buf[MAXLINE];
    char srcid[IDSIZE];

    getval( "src", srcid );

    if ( strcmp( srcid, myid ) == 0 ) {
        sprintf( buf, "%s:  lhs=%s_%d  rhs=%s_%d  rhs2=%s_%d\n",
                 myid, lhshost, lhsport, rhshost, rhsport, rhs2host, rhs2port);
        write_line( console_idx, buf );
        sprintf( buf, "trace done\n" );
        write_line( console_idx, buf );
    }
    else {
	mpdprintf(debug,"sending my trace info to %s\n",srcid);
        sprintf( buf,"src=%s dest=%s cmd=trace_ack lhs=%s_%d rhs=%s_%d rhs2=%s_%d\n",
                 myid, srcid, lhshost, lhsport, rhshost, rhsport,
                 rhs2host, rhs2port );
        write_line( rhs_idx, buf );
    }
}

void sib_dump()
{
    char buf[MAXLINE];
    char srcid[IDSIZE], what[80];

    getval( "src", srcid );
    getval( "what_to_dump", what );

    if ( strcmp( what, "jobtable") == 0 ||
	 strcmp( what, "all"     ) == 0 )
	dump_jobtable( 1 );
    if ( strcmp( what, "proctable") == 0 ||
	 strcmp( what, "all"      ) == 0 )
	dump_proctable( 1 );
    if ( strcmp( what, "fdtable") == 0 ||
	 strcmp( what, "all"      ) == 0 )
	dump_fdtable( "fdentries" );

    if ( strcmp( srcid, myid ) != 0 ) {
        sprintf( buf, "src=%s dest=anyone cmd=dump what_to_dump=%s\n", srcid, what );
        write_line( rhs_idx, buf );
    }
}

void sib_trace_ack()
{
    char buf[MAXLINE];
    char srcid[IDSIZE], lhsid[IDSIZE], rhsid[IDSIZE], rhs2id[IDSIZE];

    getval( "src", srcid );
    getval( "lhs", lhsid );
    getval( "rhs", rhsid );
    getval( "rhs2", rhs2id );
    sprintf( buf, "%s:  lhs=%s  rhs=%s  rhs2=%s\n",
             srcid, lhsid, rhsid, rhs2id );
    write_line( console_idx, buf );
}

void sib_ping_ack()
{
    char buf[MAXLINE];
    char fromid[IDSIZE];

    getval( "src", buf );
    strcpy( fromid, buf );
    sprintf( buf, "%s is alive\n", fromid );
    write_line( console_idx, buf );
}

void sib_ping()
{
    char buf[MAXLINE];
    char fromid[IDSIZE];

    getval( "src", fromid );
    sprintf( buf, "src=%s dest=%s cmd=ping_ack\n", myid, fromid );
    write_line( rhs_idx, buf );
}

/* Resolve <job,rank> to <host,port> if possible */
void sib_findclient()
{
    char buf[MAXLINE];
    char src[IDSIZE];
    int pid, cid, job, rank, port;

    getval( "job", buf );
    job = atoi( buf );
    getval( "rank", buf );
    rank = atoi( buf );
    getval( "cid", buf );
    cid = atoi( buf );
    getval( "src", src );
    port = find_proclisten( job, rank );
    pid  = find_proclisten_pid( job, rank );
    /* port is now the listener port,
           or -1 -> port is not *yet* set, but will be
           or -2 -> job is not on this mpd */
    if ( port >= -1 ) { 
        sprintf( buf,
		 "src=%s dest=%s cid=%d cmd=foundclient host=%s port=%d pid=%d job=%d rank=%d\n",
		 myid, src, cid, mynickname, port, pid, job, rank );
        write_line( rhs_idx, buf );
    }
    mpdprintf( debug, "sib_findclient: found port=%d pid=%d\n", port, pid );
}

/* 
 *  Forward mapping reply to client
 *  Search for <job,rank> to <host,port> mapping.
 */    
void sib_foundclient()
{
    char buf[MAXLINE];
    char host[MAXLINE];
    int pid, cid, rank, job, port;
        
    getval( "cid", buf );
    cid  = atoi( buf );
    getval( "port", buf );
    port = atoi( buf );
    getval( "pid", buf );
    pid = atoi( buf );
    getval( "rank", buf );
    rank = atoi( buf );
    getval( "job", buf );
    job  = atoi( buf );
    getval( "host", host );        
    sprintf( buf,"cmd=foundclient cid=%d job=%d rank=%d host=%s port=%d pid=%d\n",
            cid, job, rank, host, port, pid );
    send_msg( proctable[cid].clientfd, buf, strlen( buf ) );
}

void sib_needjobids()
{
    char buf[MAXLINE], srcbuf[MAXLINE];
    int first, last;

    getval( "src", srcbuf );
    if ( steal_jobids( &first, &last ) == 0 ) {
	sprintf( buf, "cmd=newjobids dest=%s first=%d last=%d\n", srcbuf, first, last );
	mpdprintf( 0, "sending newids, first=%d, last=%d to %s\n", first, last, srcbuf );
	write_line( rhs_idx, buf );
    }
    else {
	sprintf( buf, "src=%s dest=anyone cmd=needjobids\n", srcbuf );
	mpdprintf( 0, "forwarding needjobids message\n" );
	write_line( rhs_idx, buf );
    }
}

void sib_newjobids()
{
    char buf[MAXLINE];
    int first, last;

    getval( "first", buf );
    first = atoi( buf );
    getval( "last", buf );
    last  = atoi( buf );

    mpdprintf( 0, "accepting new jobids first=%d, last=%d\n", first, last );
    add_jobids( first, last );
}

void sigchld_handler( signo )
int signo;
{
    pid_t pid;
    int i, stat, jidx;

    /* pid = wait( &stat ); */
    while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 ) {
	for (i=0; i < MAXPROCS; i++) {
	    if (proctable[i].active && proctable[i].pid == pid) {
                jidx = find_jobid_in_jobtable(proctable[i].jobid);
		jobtable[jidx].alive_here_sofar--;
		if (jobtable[jidx].alive_here_sofar <= 0)
		    remove_from_jobtable( jobtable[jidx].jobid );
	    }
	}
        mpdprintf( debug, "child %d terminated\n", (int) pid );
        remove_from_proctable( (int) pid );
	dump_jobtable(0);
    }
    return;
}

void sigusr1_handler( signo )
int signo;
{
    mpdprintf( 1, "mpd got SIGUSR1\n" );
}

extern void mpd_cleanup();

void sigint_handler(signo)
int signo;
{
    char buf[MAXLINE];

    mpdprintf( debug, "\n MPD exit on SIGINT\n");
    if (amfirst) { /* for master , kill all */
       sprintf(buf,"src=%s dest=%s bcast=true cmd=bomb\n",
               myid,myid);
       write_line( rhs_idx, buf );
    }
    mpdprintf( 1, "calling mpd_cleanup from sigint_handler;sig=%d\n", signo );
    mpd_cleanup();
    exit(1);
}
