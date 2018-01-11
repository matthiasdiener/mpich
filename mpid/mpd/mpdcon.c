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
extern int fdtable_high_water_mark;

int cfd, debug = 0;
int listener_idx = -1;
int ctl_idx = -1;
int stdin_idx = -1;
int stdout_idx = -1;
int stderr_idx = -1;

int jobid, done;
int control_input_closed = 0;
int stdout_input_closed = 0;
int stderr_input_closed = 0;

int iotree     = 1;		   /* default is to prebuild print tree */
int gdb	       = 0;		   /* whether we are running mpigdb or not */
int mpirunning = 0;		   /* whether we are running mpirun or not */
int numprompts = 0;		   /* how many prompts have been received from gdb */
int jobsize;			   /* size of job in mpirun, mpigdb, mpdmpexec */
int mergeprompts;		   /* how many prompts to merge in mpigdb mode */

char myid[IDSIZE];

struct passwd *pwent;

int mpdhelp( int, char** );
int mpdcleanup( int, char** );
int mpdtrace( int, char** );
int mpdlistjobs( int, char** );
int mpdkilljob( int, char** );
int mpddump( int, char** );
int mpdmandump( int, char** );
int mpdringtest( int, char** );
int mpdmpexec( int, char** );
int mpdexit( int, char** );
int mpdallexit( int, char** );
int mpdbomb( int, char** );
int mpirun( int, char** );
int mpigdb( int, char** );
void handle_listen_input( int );
void handle_control_input( int );
void handle_stdout_input( int );
void handle_stderr_input( int );
void handle_temp_input( int );
void handle_user_stdin( int );
void con_sig_handler( int );
int  start_mpds( char * );
void process_buf( char *, char *, int *, int * );


int main( int argc, char *argv[] )
{
    int rc;
#   if defined(ROOT_ENABLED)
    int old_uid, old_gid;
#   endif
    char *s, pgmname[128], console_name[128];

    strcpy( myid, "mpdcon");

    if ((pwent = getpwuid(getuid())) == NULL) {
        printf( "getpwuid failed\n" );
        exit( -1 );
    }

#   if defined(ROOT_ENABLED)
    old_uid = getuid();
    old_gid = getgid();
    if ( geteuid() != 0 ) {
        printf( "this pgm must run as setuid root\n" );
        exit( -1 );
    }
    setuid(0);  /* temporarily until I connect to unix socket; only possible if euid=0 */
    setgid(0);
#   endif
    
    if ((s = rindex(argv[0],'/')) == NULL)
	strcpy(pgmname,argv[0]);
    else
	strcpy(pgmname,s+1);
    if ( strcmp( pgmname,"mpdhelp" ) == 0 )
	rc = mpdhelp( argc, argv );
    else if ( strcmp( pgmname,"mpdcleanup" ) == 0 )
	rc = mpdcleanup( argc, argv );
    else if ( strcmp( pgmname,"mpirun" ) == 0  &&  
              (argc < 4 || strcmp( argv[1], "-np" ) != 0 ) )
    {
        usage_mpirun();
	exit( -1 );
    }
    else
    {
#       if defined(ROOT_ENABLED)
	sprintf( console_name, "%s_%s", CONSOLE_NAME, "root" );
#       else
	sprintf( console_name, "%s_%s", CONSOLE_NAME, pwent->pw_name );
#       endif
	mpdprintf( debug, "connecting to console name :%s:\n", console_name );
	cfd = local_connect( console_name );
#       if defined(AUTO_START)
	if ( cfd == -1 ) {  
	    if ( strcmp( pgmname,"mpirun" ) == 0 )
		cfd = start_mpds( console_name );
	}
#       endif
	error_check( cfd, "local_connect failed to connect to an mpd: " );
	mpdprintf( debug, "local_connect; socket=%d\n", cfd );

#       if defined(ROOT_ENABLED)
	setuid(old_uid);  /* chg back now that I have the local socket */
	setgid(old_gid);
#       endif

	if ( strcmp( pgmname,"mpdringtest" ) == 0 )
	    rc = mpdringtest( argc, argv );
	else if ( strcmp( pgmname,"mpdtrace" ) == 0 )
	    rc = mpdtrace( argc, argv );
	else if ( strcmp( pgmname,"mpdlistjobs" ) == 0 )
	    rc = mpdlistjobs( argc, argv );
	else if ( strcmp( pgmname,"mpdkilljob" ) == 0 )
	    rc = mpdkilljob( argc, argv );
	else if ( strcmp( pgmname,"mpddump" ) == 0 )
	    rc = mpddump( argc, argv );
	else if ( strcmp( pgmname,"mpdmandump" ) == 0 )
	    rc = mpdmandump( argc, argv );
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
	else if ( strcmp( pgmname,"mpigdb" ) == 0 )
	    rc = mpigdb( argc, argv );
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
    char file_name[MAXLINE];
    char cmd[MAXLINE];
    char cmd2[MAXLINE];

    sprintf( cmd2, "mpdallexit");
    /* system( cmd2 ); */  /* prone to failure if there is no mpd */

    sprintf( file_name, "%s_%s", CONSOLE_NAME, pwent->pw_name );
    sprintf( cmd, "/bin/rm -f %s", file_name );
    system( cmd );

    sprintf( file_name, "%s_%s", LOGFILE_NAME, pwent->pw_name );
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
        sprintf( buf, "cmd=ringtest laps=%d\n", count );
        send_msg( cfd, buf, strlen( buf ) );
    }
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdringtest: msg from mpd: %s", buf );
    read_line( cfd, buf, MAXLINE );  /* get ringtest completed msg */
    printf( "mpdringtest: msg from mpd: %s", buf );
    return(0);
}

int mpdkilljob( int argc, char *argv[] )
{
    char buf[MAXLINE];
	
    if (argc < 2)
    {
	printf( "usage: mpdkilljob jobid \n" );
	return(0);
    }

    sprintf( buf, "cmd=killjob jobid=%s\n", argv[1] );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdkilljob: msg from mpd: %s", buf );
    return(0);
}

int mpddump(argc,argv)
int argc;
char *argv[];
{
    char buf[MAXLINE];
    char what[80];
	
    if (argc < 2)
	strcpy( what, "all" );
    else
	strcpy( what, argv[1] );

    sprintf( buf, "cmd=dump what=%s\n", what );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug,"mpddump: msg from mpd: %s", buf );
    return(0);
}

int mpdmandump( argc, argv )
int argc;
char *argv[];
{
    char buf[MAXLINE];
    char what[80];
	
    if ( argc == 4 )
	strcpy( what, argv[3] );
    else if ( argc == 3 )
	strcpy( what, "all" );
    else {
	fprintf( stderr, "Usage: mpdmandump <jobid> <man rank> [<what to dump>]\n" );
	return( -1 );
    }
    fprintf( stderr, "console: dumping from job %d, manager %d\n",
	     atoi( argv[1] ), atoi( argv[2] ) );

    sprintf( buf, "cmd=mandump jobid=%s rank=%s what=%s\n", argv[1], argv[2], what );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( 1, "mpdmandump: msg from mpd: %s", buf );
    /* The following code  is only necessary if we route the mandump output back to
       the console.  We are not doing this yet.
    while ( strcmp( buf, "mandump done\n" ) != 0 ) {
	read_line( cfd, buf, MAXLINE );
	if ( strcmp( buf, "mandump done\n" ) != 0 )
	    printf( "mpdmandump: %s", buf );
    }
    */
    return(0);
}

int mpdtrace( int argc, char *argv[] )
{
    char buf[MAXLINE];
	
    sprintf( buf, "cmd=trace\n" );
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

int mpdlistjobs( int argc, char *argv[] )
{
    char buf[MAXLINE];
	
    sprintf( buf, "cmd=listjobs\n" );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE );  /* get ack from mpd */
    mpdprintf( debug, "mpdlistjobs: msg from mpd: %s", buf );
    while ( strcmp( buf, "listjobs done\n" ) != 0 ) {
        read_line( cfd, buf, MAXLINE );
	if ( strcmp( buf, "listjobs done\n" ) != 0 )
	    printf( "mpdlistjobs: %s", buf );
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
    sprintf( buf, "cmd=bomb mpd_id=%s\n", argv[1] );
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
    sprintf( buf, "cmd=exit mpd_id=%s\n", argv[1] );
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
    sprintf( buf, "cmd=allexit\n" );
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
    printf("mpddump [what]\n" );
    printf("  causes all the mpds to dump data.\n");
    printf("  \"what\" can be \"fdtable\", \"jobtable\", or \"proctable\".\n");
    printf("  It defaults to \"all\".\n");
    printf("mpdmandump jobid manrank [what]\n" );
    printf("  causes the manager given by <jobid> and <manrank> to dump data\n");
    printf("  \"what\" is currently being defined.\n");
    printf("  It defaults to \"all\".\n");
    printf("mpdringtest count\n" );
    printf("  sends a message around the ring \"count\" times\n");
    printf("mpdexit mpd_id \n" );
    printf("  causes the specified mpd_id to exit gracefully;\n");
    printf("  mpd_id is specified as host_portnum;\n");
    printf("mpdallexit \n" );
    printf("  causes all mpds to exit gracefully;\n");
    printf("mpdbomb mpd_id \n" );
    printf("  for testing: causes the specified mpd_id to \"fail\";\n");
    printf("  mpd_id is specified as host_portnum\n");
    printf("mpdlistjobs \n" );
    printf("  lists active jobs managed by mpds in ring\n");
    printf("mpdkilljob job_id \n" );
    printf("  aborts the specified job\n");
    printf("\n" );
    return(0);
}

/******  This is the console that talks to managers  ****/

int mpdmpexec( argc, argv )
int argc;
char *argv[];
{
    int i, argcnt, envcnt, envflag, loccnt, locflag, rc, num_fds, optcount;
    int shmemgrpsize;
    char buf[MAXLINE], argbuf[MAXLINE], stuffed_arg[MAXLINE], wdirname[MAXPATHLEN];
    char path[MAXPATHLEN], executable[MAXPATHLEN], jobidbuf[8];
    char myhostname[MAXHOSTNMLEN];
    fd_set readfds, writefds;
    struct timeval tv;
    int path_was_supplied_by_user, user_stdin_idx, line_labels, close_stdin;
    size_t numgids;
    gid_t gidlist[MAXGIDS];
    char groups[5*MAXGIDS], groupbuf[6];

    if (argc < 3) {
	printf( "usage: mpdmpexec -n numprocs [-l] "
	        "[-g <shmemgrpsize>] [-s] executable"
		" [args] [-MPDENV- env] [-MPDLOC- loc(s)]\n" );
	return(0);
    }

    init_fdtable();

    /* Set up listener port.  This will be used by the manager with rank 0 to connect
       a control stream and streams for stdin, stdout, and stderr. */

    listener_idx		  = allocate_fdentry();
    fdtable[listener_idx].portnum = 0;
    fdtable[listener_idx].fd	  = setup_network_socket( &fdtable[listener_idx].portnum);
    fdtable[listener_idx].read	  = 1;
    fdtable[listener_idx].write	  = 0;
    fdtable[listener_idx].handler = LISTEN_STREAM;
    strcpy( fdtable[listener_idx].name, "listener" );

    optcount = 1;		/* counts argv[0] */
    gethostname( myhostname, MAXHOSTNMLEN );
    getcwd(wdirname,MAXPATHLEN);
    mpdprintf( debug, "current console working directory = %s\n", wdirname );
    strcpy( path, getenv( "PATH" ) ); /* may want to propagate to manager */
    mpdprintf( debug, "current path = %s\n", path );
    line_labels  = 0;
    shmemgrpsize = 1;
    close_stdin = 0;
    while ( optcount < argc  &&  argv[optcount][0] == '-' ) {
	if ( argv[optcount][1] == 'n' ) {
	    for (i=0; i < strlen( argv[optcount+1] ); i++) {
	        if ( ! isdigit( argv[optcount+1][i] ) ) {
		    printf( "invalid jobsize specified\n" );
		    return( -1 );
		}
	    }
	    jobsize = atoi( argv[optcount + 1] );
	    optcount += 2;
	}
	else if ( argv[optcount][1] == 'i' ) {
	    iotree = 0;
	    optcount++;
	}
	else if ( argv[optcount][1] == 'l' ) {
	    line_labels = 1;
	    optcount++;
	}
	else if ( argv[optcount][1] == 's' ) {
	    close_stdin = 1;
	    optcount++;
	}
	else if ( argv[optcount][1] == 'g' ) {
	    for (i=0; i < strlen( argv[optcount+1] ); i++) {
	        if ( ! isdigit( argv[optcount+1][i] ) ) {
		    printf( "invalid shmemsize specified\n" );
		    return( -1 );
		}
	    }
	    shmemgrpsize = atoi( argv[optcount + 1] );
	    optcount += 2;
	}
	else if ( strcmp( argv[optcount], "-mvhome" ) == 0 )
	    optcount++;		/* ignore this argument */
	else if ( strcmp( argv[optcount], "-mvback" ) == 0 )
	    optcount += 2;	/* ignore this argument and the next */
	else {
	    if ( mpirunning )
		usage_mpirun( );
	    else
		printf( "usage: mpdmpexec -n numprocs [-l] "
			"[-g <shmemgrpsize>] [-s] executable"
			" [args] [-MPDENV- env] [-MPDLOC- loc(s)]\n" );
	    return(-1);
	}
    }
    if ( gdb )
        strcpy( executable, "gdb" );
    else {
        if ( optcount >= argc ) {
	    printf( "no executable specified\n" );
	    return( -1 );
	}
        strcpy( executable, argv[optcount++] );
    }

    if ( gdb ) {
	line_labels = 1;
	mergeprompts = jobsize;	   /* initially talking to all gdb's */
    }

/* get user's groups, especially for ROOT_ENABLED stuff */
    numgids = getgroups( MAXGIDS, gidlist );
    error_check( numgids, "mpdmpexec: could not get groups" );
    for ( i = 0; i < numgids; i++ ) 
	mpdprintf( 0, "member of group %d\n", gidlist[i] );
/* create string of group ids */
    groups[0] = '\0';		/* set group string to empty */
    for ( i = 0; i < numgids ; i++ ) {
	sprintf( groupbuf, "%d,", gidlist[i] );
	strncat( groups, groupbuf, sizeof( groupbuf ) );
    }
    groups[strlen(groups) - 1] = '\0'; /* chop off trailing comma */
    mpdprintf( 0, "group string = :%s:\n", groups );

    sprintf( buf,
	     "cmd=mpexec hostname=%s portnum=%d iotree=%d numprocs=%d "
	     "executable=%s line_labels=%d shmemgrpsize=%d "
             "username=%s groupid=%d groups=%s ",
	     myhostname, fdtable[listener_idx].portnum, iotree, jobsize,
	     executable, line_labels, shmemgrpsize,
             pwent->pw_name, getgid( ), groups );
    argcnt  = 0;
    envcnt  = 0;
    loccnt  = 0;
    envflag = 0;
    locflag = 0;
    path_was_supplied_by_user = 0;

    if ( gdb )
    {
        argcnt++;
	sprintf( argbuf, " arg%d=-q", argcnt );
	strcat( buf,argbuf );
	argcnt++;
	sprintf( argbuf, " arg%d=%s", argcnt, argv[optcount++] );
	strcat( buf,argbuf );
    }

    if (argc > optcount)
    {
	strcat( buf, " " );  /* extra blank before args */
	for ( i = optcount; i < argc; i++ ) {
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
    mpdprintf( 0, "mpdmpexec: sending to mpd :%s:\n", buf );
    send_msg( cfd, buf, strlen( buf ) );
    read_line( cfd, buf, MAXLINE ); /* get jobid from mpd */
    mpdprintf( debug, "mpdmpexec: msg from mpd: %s", buf );
    parse_keyvals( buf );
    getval( "jobid", jobidbuf );
    jobid = atoi( jobidbuf );
    /* fprintf( stderr, "%s", buf );*/  	/* print job id */

    /* don't close socket to mpd until later when we get ctl stream from mpdman*/
    /* dclose( cfd ); */

    if ( close_stdin ) {
        dclose( 0 );
    }
    else {
        /* put stdin in fdtable */
        user_stdin_idx		    = allocate_fdentry();
        fdtable[user_stdin_idx].fd	    = 0;
        fdtable[user_stdin_idx].read    = 1;
        fdtable[user_stdin_idx].write   = 0;
        fdtable[user_stdin_idx].handler = USER_STDIN;
        strcpy( fdtable[user_stdin_idx].name, "user_stdin" );
    }

    /* Main loop */
    done = 0;
    while ( !done ) {
        FD_ZERO( &readfds );
        FD_ZERO( &writefds );
        for ( i=0; i <= fdtable_high_water_mark; i++ )
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

        for ( i=0; i <= fdtable_high_water_mark; i++ ) {
            if ( fdtable[i].active ) {
                if ( FD_ISSET( fdtable[i].fd, &readfds ) )
                    handle_input_fd( i );
            }
        }
	mpdprintf( debug, "control_input_closed=%d stdout_input_closed=%d "
	                "stderr_input_closed=%d\n",control_input_closed,
			stdout_input_closed,stderr_input_closed );
	if ( control_input_closed && stdout_input_closed && stderr_input_closed )
	    done = 1;
    }
    
    return(0);
}

/******  This is the mpd version of mpirun; it uses mpdmpexec  ****/

int mpirun( int argc, char *argv[] )
{
    if (argc < 4 || strcmp( argv[1], "-np" ) != 0 )
    {
	usage_mpirun();
	exit(1);
    }
    argv[1][2] = '\0';		/* replace -np by -n */
    mpirunning = 1;		/* so command-line parsing will do corrent err_msg */
    mpdmpexec( argc, argv );
    return( 0 );
}

/******  This is the debugging version of mpirun  ****/

int mpigdb( int argc, char *argv[] )
{
    gdb = 1;	        /* set flag to indicate we are debugging under gdb */
    mpirun( argc, argv );
    return( 0 );
}


void handle_input_fd( int idx )
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
        mpdprintf( debug, "message from manager to handle = :%s: (read %d)\n",
		   message, length );
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
	dclose( cfd );  /* now that we have a ctl stream from mpdman */
	if ( gdb )
	    write_line( ctl_idx, "cmd=set stdin=all\n" );

	/* can ONLY do con_bnr_put's after we have a valid ctl_idx ( ! -1) */
	/*****
	sprintf( tmpbuf, "cmd=con_bnr_put attr=RALPH val=BUTLER gid=0\n" );
	write_line( ctl_idx, tmpbuf );
	*****/
    }
    else if ( strcmp( cmd, "new_stdin_stream" ) == 0 ) {
        stdin_idx = idx;
	fdtable[stdin_idx].handler = STDIN_STREAM;
	fdtable[stdin_idx].read = 0;
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
	    mpdprintf( debug, "handle_control_input sending allexit\n");
	    sprintf( buf, "cmd=allexit\n" );
	    write_line( ctl_idx, buf );
	    mpdprintf( debug, "parallel job exited\n" );
	    /* exit( 0 ); */ /* hang around until manager 0 ends */
	}
	else if ( strcmp( cmd, "jobaborted" ) == 0 ) {
	    printf( "job %d aborted with code %d by process %d\n",
	            atoi( getval( "job", buf ) ),
	            atoi( getval( "code", buf ) ),
	            atoi( getval( "rank", buf ) ) );
	    /* exit( 0 ); */ /* hang around until manager 0 ends */
	}
	else if ( strcmp( cmd, "man_ringtest_completed" ) == 0 )
	    printf( "manringtest completed\n" );
	else
	    mpdprintf( 1, "unrecognized message from job manager\n" );
    }
    else if ( length == 0 ) {
	mpdprintf( debug, "eof on cntl input\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	control_input_closed = 1;
    }
    else
        mpdprintf( 1,
		   "console failed to retrieve msg from control stream, errno = %d\n",
		   errno );
}

void handle_stdout_input( int idx )
{
    int n, promptsfound, len_stripped;
    char buf[STREAMBUFSIZE+1], newbuf[STREAMBUFSIZE];
    static int first_prompts = 1;

    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	buf[n] = '\0';		   /* null terminate for string processing */
	if ( gdb ) {
	    /* fprintf( stderr, "read |%s|\n", buf ); */
	    process_buf( buf, newbuf, &promptsfound, &len_stripped );
	    numprompts += promptsfound;
	    mpdprintf( debug, "handle_stdout_input writing %d\n", n - ( len_stripped ) );
	    write( 1, newbuf, n - ( len_stripped ) );
	    if ( numprompts >= mergeprompts ) {
		printf( "(mpigdb) " );
		fflush( stdout );
		numprompts = 0;
		if ( first_prompts ) {
		    first_prompts = 0;
		    write_line( stdin_idx, "handle SIGUSR1 nostop noprint\n" );
		}
	    }
	}
	else 
	    write( 1, buf, n );
    }
    else if ( n == 0 ) {
	mpdprintf( debug, "console received eof on stdout from manager\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	stdout_input_closed = 1;
    }
    else
	fprintf( stderr, "console failed to retrieve msg from stdout stream\n" );
}

void handle_stderr_input( int idx )
{
    int n, promptsfound, len_stripped;
    char buf[STREAMBUFSIZE+1], newbuf[STREAMBUFSIZE];

    if ( ( n = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	buf[n] = '\0';		   /* null terminate for string processing */
	if ( gdb ) {
	    process_buf( buf, newbuf, &promptsfound, &len_stripped );
	    numprompts += promptsfound;
	    mpdprintf( debug, "handle_stderr_input writing %d\n", n - ( len_stripped ) );
	    write( 2, ".", 1);	/* temporarily mark stderr - RL */
	    write( 2, newbuf, n - ( len_stripped ) );
	    if ( numprompts >= mergeprompts ) {
		fprintf( stderr, ".(mpigdb) " );
		fflush( stderr );
		numprompts = 0;
	    }
	}
	else { 
	    write( 2, ".", 1);	/* temporarily mark stderr - RL */
	    write( 2, buf, n );
	}
    }
    else  if ( n == 0 ) {
	mpdprintf( debug, "console received eof on stderr from manager\n" );
	dclose( fdtable[idx].fd );
	deallocate_fdentry( idx );
	stderr_input_closed = 1;
    }
    else
	fprintf( stderr, "console failed to retrieve msg from stderr stream\n" );
}

void handle_stdin_input( int idx )
{
    int length;
    char buf[STREAMBUFSIZE];

    if ( ( length = read( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
	mpdprintf( 1,
		   "console received unexpected input from manager on stdin_out: :%s: (read %d)\n",
		   buf, length );
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
    int n, target;
    char buf[STREAMBUFSIZE], buf2[STREAMBUFSIZE];
    
    if ( ( n = read_line( fdtable[idx].fd, buf, STREAMBUFSIZE ) ) > 0 ) {
				   /* note n includes newline but not NULL */
	mpdprintf( debug, "handle_user_stdin: got %s", buf ); 
	if ( buf[0] == '_' ) { /* escape character to access cntl */
	    sprintf( buf2, "cmd=%s", buf + 1 );
	    write_line( ctl_idx, buf2 );
	}
	else {
	    if ( stdin_idx != -1 ) {
		if ( gdb ) {	   /* check for 'z' command */
		    if ( strncmp( buf, "z", 1 ) == 0 ) {
			if ( n == 2) {	        /* z only, set stdin target to all */
			    mergeprompts = jobsize;
			    numprompts   = 0;   /* reset number seen */
			    write_line( ctl_idx, "cmd=set stdin=all\n" );
			}
			else {	                /* z <target>, set target */
			    if ( sscanf( buf+1, "%d", &target ) > 0 ) {
				if ( ( target > jobsize - 1 ) || ( target < 0 ) )
				    fprintf( stderr, "target out of range\n" );
				else {
				    mergeprompts = 1;
				    sprintf( buf2, "cmd=set stdin=%d\n", target );
				    write_line( ctl_idx, buf2 );
				}
			    }
			    else
				fprintf( stderr, "Usage: z <target process> OR z\n" );
			}
			printf( "(mpigdb) " ); fflush(stdout);
		    }
		    else {
			mpdprintf( debug, "handle_user_stdin doing send_msg\n");
			send_msg( fdtable[stdin_idx].fd, buf, n );
		    }
		}
		else {
		    mpdprintf( debug, "handle_user_stdin doing send_msg\n");
		    send_msg( fdtable[stdin_idx].fd, buf, n );
		}
	    }
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
	fprintf( stderr, "job %d suspended\n", jobid );
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
	mpdprintf( debug, "parallel job being killed\n" );
	sprintf( buf, "cmd=signal signo=%s\n", "SIGINT" );
	write_line( ctl_idx, buf );
	dclose( fdtable[ctl_idx].fd );
	exit( -1 );
    }
    else {
	mpdprintf( 1, "unknown signal %d (%s)\n", signo, signame );
    }
}

int start_mpds( char *name )
{
    char cmd[50]; 

    sprintf(cmd, "startdaemons 5" );
    system( cmd );

    cfd = local_connect( name );
    return cfd; 
    
}

void process_buf( char *inbuf, char* outbuf, int *promptsfound, int *len_stripped )
/* This routine removes (gdb) prompts from the buffer and counts the number it finds.
   The idea is to enable the caller to issue a (mpigdb) prompt once each of the 
   instances of gdb has been heard from.
*/
{
    char out[STREAMBUFSIZE+1];
    char *p, *q;
    int  num, len, len_to_strip, total_len_stripped = 0;

    /* copy the input and null terminate it just in case it isn't already */
    strcpy( out, inbuf );
    
    len = strlen( out );
    num	= 0;
    p	= out; 
    while ( ( p = strstr( p, ": (gdb) " ) ) != NULL ) {
	num++;
	q = p;
	while ( q != out && *(q - 1) != '\n' && *(q - 1) != ' ' )
	    q--;	/* back up over line label to previous nl or beginning of buffer */
	len_to_strip = p + 8 - q;
	total_len_stripped += len_to_strip;
	len = len - len_to_strip;
	memmove( q, q + len_to_strip, len + 1 ); /* len + 1 to include the \0 */
	p = q;			/* reset for next search */
    }
    *len_stripped = total_len_stripped;
    *promptsfound = num;
    memmove( outbuf, out, len ); 
    /* fprintf( stderr, "old=:\n%s:\nnew=:\n%s:\nlen=%d, len_stripped=%d\n",
       inbuf, outbuf, len, *len_stripped ); */
}

void usage_mpirun()
{
    fprintf( stderr, "Usage: mpirun <args> executable <args_to_executable>\n" );
    fprintf( stderr, "Arguments are:\n" );
    fprintf( stderr, "  -np num_processes_to_run  (required)\n" );
    fprintf( stderr, "  [-s]  (close stdin; can run in bkgd w/o tty input problems)\n" );
    fprintf( stderr, "  [-g shmem_group_size]\n" );
    fprintf( stderr, "  [-l]  (line labels; unique id for each process' output\n" );
}
