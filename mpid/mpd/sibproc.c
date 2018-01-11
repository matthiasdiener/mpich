#include "mpd.h"
#if defined(ROOT_ENABLED)
#if !defined(__USE_BSD)
#define __USE_BSD
#endif
#include <grp.h>
#endif

extern struct fdentry fdtable[MAXFDENTRIES];
extern struct procentry proctable[MAXPROCS];
extern struct jobentry jobtable[MAXJOBS];
extern int    amfirst;
extern int    console_idx;
extern int    rhs_idx;
extern int    lhs_idx;
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
extern int    keyval_tab_idx;
extern char   mpd_passwd[PASSWDLEN];

void sib_reconnect_rhs(idx) 
int idx;
{
    int  newport;
    char buf[MAXLINE], new_rhs[MAXHOSTNMLEN], parse_buf[MAXLINE], cmd[MAXLINE];

    getval("rhshost",new_rhs);
    getval("rhsport",buf);
    newport = atoi( buf );
    mpdprintf( debug, "got cmd=reconnect_rhs host=%s port=%d\n",new_rhs,newport);
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
    sprintf( buf, "src=%s dest=%s_%d cmd=new_lhs_req host=%s port=%d\n",
             myid, rhshost, rhsport, mynickname, my_listener_port );
    write_line( rhs_idx, buf );
    recv_msg( fdtable[rhs_idx].fd, buf );
    strcpy( parse_buf, buf );
    parse_keyvals( parse_buf );
    getval( "cmd", cmd );
    if ( strcmp( cmd, "challenge" ) != 0 ) {
	mpdprintf( 1, "reconnect_rhs: expecting challenge, got %s\n", buf );
	exit( -1 );
    }
    newconn_challenge( rhs_idx );
    /* Now that we have an rhs, we can initialize the jobid pool, which might require
       sending messages.
    */
    init_jobids();		/* protected from executing twice */
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
    int  jobid;
    char buf[MAXLINE];

    getval( "jobid", buf );
    jobid = atoi( buf );
    kill_job( jobid, SIGKILL );
}

void sib_signaljob()
{ 
    int  pidx, jobid;
    char c_signum[32], buf[MAXLINE];

    getval( "jobid", buf );
    jobid = atoi( buf );
    getval( "signum", c_signum);
    for ( pidx=0; pidx < MAXPROCS; pidx++ )
    {
	if ( proctable[pidx].active  &&  proctable[pidx].jobid == jobid )  {
            sprintf( buf, "cmd=signaljob signo=%s\n", c_signum );
	    write( proctable[pidx].clientfd, buf, strlen(buf) );
	}
    }
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
    char src[MAXLINE], program[MAXLINE], username[MAXLINE];
    int  pid, i, saved_i, j, rc, argc, envc, locc, line_labels, shmemgrpsize;
    char *argv[25];
    char *env[50];
    int  jobrank, jobid, jobsize, jidx, cid; 
    char conhost[MAXHOSTNMLEN];
    int  conport;
    int  man_mpd_socket[2];
    int  man_idx;
    char env_mpd_fd[80], env_job[80], env_rank[80], env_size[80];
    char env_man_pgm[256], env_man_listener_fd[80], env_man_prevhost[MAXHOSTNMLEN];
    char env_man_prevport[80], env_man_host0[MAXHOSTNMLEN], env_man_port0[80];
    char env_man_conport[80], env_man_conhost[MAXHOSTNMLEN], env_man_debug[80];
    char env_man_prebuildprinttree[80], env_line_labels[80];
    char env_shmemkey[80], env_shmemgrpsize[80], env_shmemgrprank[80];
    int  man_listener_fd, last_man_listener_fd, man_listener_port, last_man_listener_port;
    int  first_man_listener_port = -1, first_man_listener_fd;
    char host0[MAXHOSTNMLEN], prevhost[MAXHOSTNMLEN];
    char groups[6*MAXGIDS];
    gid_t gidlist[MAXGIDS];
    int  port0, prevport;
    char host0_next_mpd[MAXHOSTNMLEN];
    int  port0_next_mpd;
    int  hopcount, iotree, do_mpexec_here, groupid, numgids;
    struct passwd *pwent;

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
    getval( "iotree", buf );
    iotree = atoi( buf );
    getval( "line_labels", buf );
    line_labels = atoi( buf );
    getval( "shmemgrpsize", buf );
    shmemgrpsize = atoi( buf );
    getval( "username", username );
    getval( "groupid", buf );
    groupid = atoi( buf );
    getval( "groups", groups );

    if ( jobrank >= jobsize ) {
	mpdprintf( debug, "mpexec returning, jobrank=%d, jobsize=%d\n", jobrank, jobsize );
	return;			/* all processes already forked upstream */
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

    /* This is to stop an infinite loop when the user has specified in -MPDLOC- only
       invalid machines */
    if ( ( hopcount > 1 ) &&
	 ( jobrank == 0 ) &&
	 ( strcmp( src, myid ) == 0 ) &&
	 ( !do_mpexec_here ) ) {
	mpdprintf( 1, "could not start any processes\n" );
	return;
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

    if ((pwent = getpwnam( username )) == NULL)
    {
	mpdprintf( 1, "mpd: getpwnam failed" );
	exit( -1 );
    }

    /* First acquire a socket to be used by the last manager to be forked *here*, to
       send to the next mpd. 
       This will be the general-purpose listener port for the manager.  It is acquired now
       so that the manager will have it ready before the next manager to the right, on
       the next mpd, if there is one, attempts to connect on it.
       */

    last_man_listener_port = 0;
    last_man_listener_fd = setup_network_socket( &last_man_listener_port );
    if ( shmemgrpsize > 1 ) {
	first_man_listener_port = 0;
	first_man_listener_fd = setup_network_socket( &first_man_listener_port );
    }
    else {
	first_man_listener_fd = last_man_listener_fd;
	first_man_listener_port = last_man_listener_port;
    }

    mpdprintf( debug, "last_man_fd=%d, last_man_listener_port=%d, "
	       "first_man_fd=%d, first_man_listener_port=%d\n",
	       last_man_listener_fd, last_man_listener_port,
	       first_man_listener_fd, first_man_listener_port); 

    /* For rank 0, the incoming mpexec command formulated by conproc does not have
       (host0, port0) (since it doesn't know), or (prevhost,prevport) (since they
       don't exist). */

    if ( jobrank == 0 ) {	/* I am the mpd that is starting the first manager */
	strcpy( host0_next_mpd, myhostname );
	if ( shmemgrpsize == 1 )
	    port0_next_mpd = last_man_listener_port;
	else
	    port0_next_mpd = first_man_listener_port;
    }
    else {			
        strcpy( host0_next_mpd, getval( "host0", buf ) );
	port0_next_mpd = atoi( getval( "port0", buf ) );
    }

    mpdprintf( debug, "before sending:  port0_next_mpd=%d, prevport=%d\n",
	       port0_next_mpd, last_man_listener_port );

    sprintf(fwdbuf,
	    "cmd=mpexec conhost=%s conport=%d host0=%s port0=%d prevhost=%s prevport=%d "
	    "iotree=%d rank=%d src=%s dest=anyone job=%d jobsize=%d prog=%s hopcount=%d "
	    "line_labels=%d shmemgrpsize=%d username=%s groupid=%d groups=%s ",
	    conhost, conport, host0_next_mpd, port0_next_mpd, myhostname,
	    last_man_listener_port, iotree, jobrank + shmemgrpsize, src, jobid, jobsize,
	    program, hopcount + 1, line_labels, shmemgrpsize, username, groupid, groups );
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
        argv[i] = (char *) malloc(MAXLINE);
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
	env[i] = (char *) malloc(MAXLINE);
	destuff_arg(tempenv,env[i]);
    }
    /* set   env[i] = NULL;    below */
    saved_i = i;

    strcat( fwdbuf, "\n" );
    mpdprintf( debug, "sib_mpexec: sending to rhs: :%s:\n", fwdbuf );
    write_line( rhs_idx, fwdbuf );

    /* We have now forwarded the appropriate mpexec command to the next mpd, so we
       now proceed to create shmemgrpsize number of managers at this mpd. */ 

    for ( j = 0; j < shmemgrpsize && jobrank < jobsize; j++ ) {

	i = saved_i;
	if ( ( jidx = find_jobid_in_jobtable(jobid) ) < 0 ) {
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
	mpdprintf( debug, "socketpair for manager %d is %d and %d\n",
		   jobrank, man_mpd_socket[0], man_mpd_socket[1] );

	/* plant environment variables for client process */
	cid = allocate_procent();
	sprintf(env_job,  "MPD_JID=%d", jobid );
	env[i++] = env_job; 
	sprintf(env_rank, "MPD_JRANK=%d", jobrank );
	env[i++] = env_rank; 
	sprintf(env_size, "MPD_JSIZE=%d", jobsize );
	env[i++] = env_size; 
	sprintf(env_shmemkey, "MPD_SHMEMKEY=%d",
		generate_shmemkey( my_listener_port, jobrank / shmemgrpsize, jobid ) );
	env[i++] = env_shmemkey; 
	sprintf(env_shmemgrpsize, "MPD_SHMEMGRPSIZE=%d", shmemgrpsize );
	env[i++] = env_shmemgrpsize; 
	sprintf(env_shmemgrprank, "MPD_SHMEMGRPRANK=%d", j );
	env[i++] = env_shmemgrprank; 
	/* plant environment variables for manager process */
	sprintf( env_man_pgm, "MAN_CLIENT_PGM=%s", program );
	env[i++] = env_man_pgm;
	sprintf( env_mpd_fd, "MAN_MPD_FD=%d", man_mpd_socket[1] );
	env[i++] = env_mpd_fd; 

	if ( jobrank == 0 ) {
	    strcpy( prevhost, DUMMYHOSTNAME );
	    prevport = DUMMYPORTNUM;
	}
	else {
	    if ( j == 0 ) {	/* I am setting up first manager on this mpd, but not
				 the very first manager */
		strcpy( prevhost, getval( "prevhost", buf ) );
		prevport = atoi( getval( "prevport", buf ) );
	    }
	    else {
		strcpy( prevhost, myhostname );
		prevport = man_listener_port; /* from previous iteration of loop, below */
	    }
	}
	sprintf( env_man_prevhost, "MAN_PREVHOST=%s", prevhost );
	env[i++] = env_man_prevhost;
	sprintf( env_man_prevport, "MAN_PREVPORT=%d", prevport );
	env[i++] = env_man_prevport;

	if ( jobrank != jobsize - 1 ) {	         /* Not the globally last manager */
	    strcpy( host0, DUMMYHOSTNAME );
	    port0 = DUMMYPORTNUM;
	}
	else {
	    if ( jobrank >= shmemgrpsize ) {     /* So there is host0,port0 in
						    incoming message */
		strcpy( host0, getval( "host0", buf ) );
		port0 = atoi( getval( "port0", buf ) );
	    }
	    else {		/* We are on first mpd, so no
				   host0,port0 in incoming message */
		strcpy( host0, myhostname );
		if ( j != 0 )
		    port0 = first_man_listener_port;
		else		/* globally first manager (first mpd, j = 0) */
		    port0 = last_man_listener_port;
	    }
	}
	sprintf( env_man_host0, "MAN_HOST0=%s", host0 );
	env[i++] = env_man_host0;
	sprintf( env_man_port0, "MAN_PORT0=%d", port0 );
	env[i++] = env_man_port0;

	if ( j == shmemgrpsize - 1 || jobrank == jobsize - 1 ) { /* last man on this mpd */
	    man_listener_fd = last_man_listener_fd;
	    man_listener_port = last_man_listener_port;
	}
	else
	    if ( j == 0 ) {
		man_listener_fd = first_man_listener_fd; /* acquired at top */
		man_listener_port = first_man_listener_port;
	    }
	    else {
		man_listener_port = 0;
		man_listener_fd = setup_network_socket( &man_listener_port );
	    }
	sprintf( env_man_listener_fd, "MAN_LISTENER_FD=%d", man_listener_fd );
	env[i++] = env_man_listener_fd;

	sprintf( env_man_conhost, "MAN_CONHOST=%s", conhost );
	env[i++] = env_man_conhost;
	sprintf( env_man_conport, "MAN_CONPORT=%d", conport );
	env[i++] = env_man_conport;
	sprintf( env_man_debug, "MAN_DEBUG=%d", debug );
	env[i++] = env_man_debug;
	sprintf( env_man_prebuildprinttree, "MAN_PREBUILD_PRINT_TREE=%d", iotree );
	env[i++] = env_man_prebuildprinttree;
	sprintf( env_line_labels, "MAN_LINE_LABELS=%d", line_labels );
	env[i++] = env_line_labels;
	env[i] = NULL;

	proctable[cid].jobid    = jobid;
	proctable[cid].jobrank  = jobrank;
	proctable[cid].state    = CLSTART; /* not running yet */
	proctable[cid].clientfd = man_mpd_socket[0];
	strcpy( proctable[cid].name, MANAGER_PATHNAME );

	Signal( SIGCHLD, sigchld_handler );
	mpdprintf( debug, "starting program %s\n", MANAGER_PATHNAME );
	pid = fork();
	proctable[cid].pid = pid;
	if ( pid < 0 ) {
	    mpdprintf( 1, "could not fork manager\n" );
	    deallocate_procent( cid );
	}
	else if ( pid == 0 ) {                  /* child manager */
	    sprintf( myid, "man_%d_before_exec", jobrank );
	    mpdprintf( debug, "manager before exec closing fd %d\n", man_mpd_socket[0] );
	    dclose( man_mpd_socket[0] );
	    setpgid(0,0);
	    setuid( pwent->pw_uid );
	    setgid( groupid );
	    mpdprintf( 0, "groups before execve = :%s:\n", groups );
	    parse_groups( groups, gidlist, &numgids );
	    for ( i = 0; i < numgids; i++ ) 
		mpdprintf( 0, "sibproc:  member of group %d\n", gidlist[i] );
#if defined(ROOT_ENABLED)
	    /* set group membership here */
	    rc = setgroups( numgids, gidlist );
	    error_check( rc, "setting groups" );
#endif
	    rc = execve( MANAGER_PATHNAME, argv, env );
	    error_check( rc, "execve");
	}
	/* parent mpd */
	dclose( man_listener_fd );	/* close listener fd setup on behalf of manager */
	dclose( man_mpd_socket[1] );

	jobrank++;
    }                                   /* end of loop through creation of managers */

    i = 1;  /* argv[0] wasn't malloc'd */
    while (argv[i])
	free(argv[i++]);
    for (i=0; i < envc; i++)
	free(env[i]);
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

    mpdprintf( debug, "sib_jobsync: entering with jobid=%d, jobsize=%d, sofar=%d\n",
	       jobid, jobsize, sofar );

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
	    mpdprintf( 1, "sib_jobgo: sending go to client for job=%d, rank=%d\n",
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
    }
    else {
	mpdprintf(debug,"sending my trace info to %s\n",srcid);
        sprintf( buf,"src=%s dest=%s cmd=trace_info lhs=%s_%d rhs=%s_%d rhs2=%s_%d\n",
                 myid, srcid, lhshost, lhsport, rhshost, rhsport,
                 rhs2host, rhs2port );
        write_line( rhs_idx, buf );
    }
}

void sib_trace_trailer()
{
    char buf[MAXLINE];
    char srcid[IDSIZE];

    getval( "src", srcid );
    if ( strcmp( srcid, myid ) == 0 ) {
        sprintf( buf, "trace done\n" );
        write_line( console_idx, buf );
    }
    else {
        sprintf( buf, "cmd=trace_trailer src=%s\n", srcid );
        write_line( rhs_idx, buf );
    }
}

void sib_trace_info()
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

void sib_listjobs()
{
    int i;
    char buf[MAXLINE];
    char srcid[IDSIZE];

    getval( "src", srcid );
    if ( strcmp( srcid, myid ) == 0 ) {
	for (i=0; i < MAXJOBS; i++) {
	    if ( jobtable[i].active ) {
                sprintf( buf, "%s:  jobid=%d\n", myid, jobtable[i].jobid );
                write_line( console_idx, buf );
	    }
	}
    }
    else {
	for (i=0; i < MAXJOBS; i++) {
	    if ( jobtable[i].active ) {
                sprintf( buf,"src=%s dest=%s cmd=listjobs_info jobid=%d\n",
                         myid, srcid, jobtable[i].jobid );
                write_line( rhs_idx, buf );
	    }
	}
    }
}

void sib_listjobs_trailer()
{
    char buf[MAXLINE];
    char srcid[IDSIZE];

    getval( "src", srcid );
    if ( strcmp( srcid, myid ) == 0 ) {
        sprintf( buf, "listjobs done\n" );
        write_line( console_idx, buf );
    }
    else {
        sprintf( buf, "cmd=listjobs_trailer src=%s\n", srcid );
        write_line( rhs_idx, buf );
    }
}

void sib_listjobs_info()
{
    char buf[MAXLINE];
    char srcid[IDSIZE], jobid[IDSIZE], destid[IDSIZE];

    getval( "src", srcid );
    getval( "jobid", jobid );
    getval( "dest", destid );
    if ( strcmp( srcid, myid ) == 0 ) {
        sprintf( buf, "%s:  jobid=%s\n", srcid, jobid );
        write_line( console_idx, buf );
    }
    else {
	sprintf( buf,"src=%s dest=%s cmd=listjobs_info jobid=%s\n",
		 srcid, destid, jobid );
        write_line( rhs_idx, buf );
    }
}

void sib_dump()
{
    char buf[MAXLINE];
    char srcid[IDSIZE], what[80];

    getval( "src", srcid );
    getval( "what", what );

    if ( strcmp( what, "jobtable") == 0 ||
	 strcmp( what, "all"     ) == 0 )
	dump_jobtable( 1 );
    if ( strcmp( what, "proctable") == 0 ||
	 strcmp( what, "all"      ) == 0 )
	dump_proctable( "procentries" );
    if ( strcmp( what, "fdtable") == 0 ||
	 strcmp( what, "all"      ) == 0 )
	dump_fdtable( "fdentries" );

    if ( strcmp( srcid, myid ) != 0 ) {
        sprintf( buf, "src=%s dest=anyone cmd=dump what=%s\n", srcid, what );
        write_line( rhs_idx, buf );
    }
}

void sib_mandump( void )
{
    int  i, jobid, manrank;
    char buf[MAXLINE];
    char srcid[IDSIZE], what[80], manager[8];

    getval( "src", srcid );
    getval( "jobid", buf );
    jobid = atoi( buf );
    getval( "manrank", manager );
    manrank = atoi( manager );
    getval( "what", what );

    mpdprintf(1, "got mandump command for jobid=%d manrank=%d what=%s\n",
	      jobid, manrank, what );

    /* next step:  look up to see if I have a client with that rank */
    for ( i = 0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active &&
	   ( proctable[i].jobrank == manrank) &&
	   ( proctable[i].jobid == jobid ) ) {
	    mpdprintf( 1, "sib_mandump: job=%d, rank=%d what=%s\n",
		       jobid, manrank, what );
	    sprintf( buf, "cmd=mandump what=%s\n", what );
	    send_msg( proctable[i].clientfd, buf, strlen( buf ) );
	}
    }

    if ( strcmp( srcid, myid ) != 0 ) {
        sprintf( buf,
		 "src=%s dest=anyone cmd=mandump jobid=%d manrank=%d what=%s\n",
		 srcid, jobid, manrank, what );
        write_line( rhs_idx, buf );
    }
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

extern void mpd_cleanup( void );

void sigint_handler( int signo )
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

/* returns a key that is shared by processes in same cluster in same job, but no others */
int generate_shmemkey( int portid, int clusterid, int jobid )
{
    int newportid, newclusterid, newjobid, shmemkey;

    newportid	 = portid % ( 1 << 16 );
    newclusterid = clusterid % ( 1 << 8 );
    newjobid	 = jobid % ( 1 << 8 );

    shmemkey = ( newportid << 16 ) + ( newclusterid << 8 ) + ( newjobid );

    mpdprintf( debug, "shmemkey = 0x%x = %d\n", shmemkey, shmemkey );

    return shmemkey;

}

int parse_groups( char *groups, int gids[], int *numgids )
{
    int i;
    char *c, groupstring[5*MAXGIDS];

    mpdprintf( debug, "group string in parse_groups = :%s:\n", groups );
    strcpy( groupstring, groups );
    c = strtok( groupstring, ", " );
    i = 0;
    while ( c ) {
	mpdprintf( 0, "group = %s\n", c );
	gids[i] = atoi( c );
	c = strtok( NULL, ", " );
	i++;
    }
    *numgids = i;
    return 0;
}    




