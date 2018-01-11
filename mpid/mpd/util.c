/* 
   util.c
   This file contains routines needed by both the mpd daemons and
   their clients, such as consoles and application programs.
   Main routines linking to this should set the string mpdid to identify 
   sources of error messages 
*/
#include "mpd.h"
#include <string.h>
#include <stdarg.h>

struct procentry proctable[MAXPROCS];
struct jobentry jobtable[MAXJOBS];

extern struct fdentry fdtable[MAXFDENTRIES];
int fdtable_high_water_mark = -1;

extern int  debug;
extern char myid[IDSIZE];

struct keyval_pairs keyval_tab[64];
int keyval_tab_idx;

/*
 *	port table routines
 */
void init_fdtable()
{
    int i;
    for ( i = 0; i < MAXFDENTRIES; i++ ) {
	fdtable[i].active = 0;
    }
}

int allocate_fdentry()
{
    int i;

    for ( i = 0; i < MAXFDENTRIES; i++ )
	if ( fdtable[i].active == 0 )
	    break;
    if ( i >= MAXFDENTRIES )
    {
	mpdprintf( 1, "*** WARNING: mpd's fdtable size exceeded\n" );
        return( -1 );
    }
    if ( i > fdtable_high_water_mark )
        fdtable_high_water_mark = i;
    fdtable[i].active	= 1;
    fdtable[i].fd	= -1;
    fdtable[i].read	= 0;
    fdtable[i].write	= 0;
    fdtable[i].portnum	= -1;
    fdtable[i].file	= NULL;
    fdtable[i].handler	= NOTSET;
    strcpy( fdtable[i].name, "" );
    mpdprintf( 0, "allocated fdtable entry %d\n", i );
    return( i );
}

void deallocate_fdentry( idx )
int idx;
{
    fdtable[idx].active = 0;
}

void dump_fdtable( identifier )
char *identifier;
{
    int i;
    
    mpdprintf( 1, "fdtable( %s )\n", identifier );
    for ( i = 0; i < MAXFDENTRIES; i++ ) {
	if ( fdtable[i].active == 1 )
	    mpdprintf( 1,
		"fd[%d]: handler=%s, fd=%d, rd=%d, wr=%d, port=%d, file=%d, name=%s\n",
		i, phandler(fdtable[i].handler), fdtable[i].fd,
		fdtable[i].read, fdtable[i].write,
		fdtable[i].portnum, fdtable[i].file, fdtable[i].name );
    }
}

void init_jobtable()
{
    int i;
    for ( i = 0; i < MAXJOBS; i++ )
	jobtable[i].active = 0;
}

int allocate_jobent()
{
    int i;
    for ( i = 0; i < MAXJOBS; i++ )
	if ( jobtable[i].active == 0 )
	    break;
    if (i >= MAXJOBS) {
	mpdprintf( 1, "could not allocate job table entry; MAXJOBS = %d\n", MAXJOBS );
	return(-1);
    }
    jobtable[i].active = 1;
    jobtable[i].jobid = -1;
    jobtable[i].jobsize = -1;
    jobtable[i].alive_here_sofar = 0;
    jobtable[i].alive_in_job_sofar = 0;
    jobtable[i].added_to_job_sofar = 0;
    jobtable[i].jobsync_is_here = 0;
    return i;
}

int find_jobid_in_jobtable( jobid )
int jobid;
{
    int i;

    for ( i=0; i < MAXJOBS; i++ )
	if ( jobtable[i].active  &&  jobtable[i].jobid == jobid )
	    return(i);
    return(-1);
}

void deallocate_jobent( idx )
int idx;
{
    jobtable[idx].active = 0;
}

void remove_from_jobtable( jobid )
int jobid;
{
    int i;
 
    for ( i = 0; i < MAXJOBS; i++ ) {
        if ( jobtable[i].active && ( jobtable[i].jobid == jobid ) ) {
            deallocate_jobent( i );
            break;
        }
    }
}

void dump_jobtable( flag )
int flag;
{
    int i;

    for ( i = 0; i < MAXJOBS; i++ ) {
	if ( jobtable[i].active )
	    mpdprintf( flag,
	       "job[%d]: jobid=%d jobsize=%d jobsync_is_here=%d\n"
               "    alive_here_sofar=%d alive_in_job_sofar=%d added_to_job_sofar=%d\n",
               i, jobtable[i].jobid, jobtable[i].jobsize, jobtable[i].jobsync_is_here,
               jobtable[i].alive_here_sofar, jobtable[i].alive_in_job_sofar,
               jobtable[i].added_to_job_sofar );	
    }
}

void init_proctable()
{
    int i;
    for ( i = 0; i < MAXPROCS; i++ )
	proctable[i].active = 0;
}

int allocate_procent()
{
    int i, found;

    found = 0;
    for ( i = 0; i < MAXPROCS; i++ )
	if ( proctable[i].active == 0 ) {
	    found = 1;
	    break;
	}
    if (found) {
	proctable[i].active   =  1;
	proctable[i].pid      = -1;
	proctable[i].jobid    = -1;
	proctable[i].jobrank  = -1;
	proctable[i].clientfd = -1;
	proctable[i].lport    = -1;
	proctable[i].state    = CLNOTSET;
	strcpy( proctable[i].name, "none" );
	return i;
    }
    else {
	mpdprintf( 1, "unable to allocate proctable entry, MAXPROCS = %d\n", MAXPROCS );
	return -1;
    }
}

void deallocate_procent( idx )
int idx;
{
    proctable[idx].active = 0;
}

int find_proclisten( job, rank )
int job;
int rank;
{
    int i;
    for ( i = 0; i < MAXPROCS;i++ ) {
	if ( ( proctable[i].active) &&
	     ( job==proctable[i].jobid) &&
	     ( rank==proctable[i].jobrank)) {
	    if ( proctable[i].state == CLALIVE ||
	         proctable[i].state == CLRUNNING )
		return proctable[i].lport;
	    else if ( proctable[i].state == CLSTART )
		return -1;	/* peer client should ask again */
	    else
		mpdprintf( 1,
			   "find_proclisten: invalid state for job=%d rank=%d state=%d\n",
			   job, rank, proctable[i].state );
	}
    }   
    return -2;
}

int find_proclisten_pid( job, rank )
int job;
int rank;
{
    int i;
    for ( i = 0; i < MAXPROCS;i++ ) {
	if ( ( proctable[i].active) &&
	     ( job==proctable[i].jobid) &&
	     ( rank==proctable[i].jobrank)) {
	    return proctable[i].pid;
	}   
    }
    return -2;
}

void remove_from_proctable( pid )
int pid;
{
    int i;
 
    for ( i = 0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active && ( proctable[i].pid == pid ) ) {
            deallocate_procent( i );
            break;
        }
    }
}

void kill_rank( int job, int rank, int signum )
{
    int  i;

    for ( i = 0; i < MAXPROCS;i++ )
	if ( ( proctable[i].active ) &&
	     (job == proctable[i].jobid ) &&
	     (rank == proctable[i].jobrank ) )
	    kill(proctable[i].pid,signum);
}

void kill_job( int jobid, int signum )
{
    int  i;

    for ( i=0; i < MAXPROCS; i++ )
	if ( proctable[i].active  &&  jobid == proctable[i].jobid ) {
	    mpdprintf( debug, "kill_job: killing jobid=%d pid=%d\n",
		       jobid, proctable[i].pid );
	    kill( proctable[i].pid, signum );
	}
}

void kill_allproc( int signum )
{
    int i, stat;
 
    for ( i = 0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active && proctable[i].pid > 0 ) {
	    mpdprintf( 1, "killing process %d at entry %d\n", proctable[i].pid, i);
	    kill( proctable[i].pid, signum );
	    waitpid( proctable[i].pid, &stat, 0 );
	}
    }
}

void dump_proctable( char *identifier )
{
    int i;

    mpdprintf( 1, "proctable( %s )\n", identifier );
    for ( i = 0; i < MAXPROCS; i++ ) {
	if ( proctable[i].active == 1 )
	    mpdprintf( 1,
	       "proc[%d]: pid=%d, jid=%d, jrank=%d, jfd=%d, lport=%d, name=%s, state=%s\n",
		i, proctable[i].pid, proctable[i].jobid, proctable[i].jobrank,
  	        proctable[i].clientfd, proctable[i].lport, proctable[i].name,
		pstate( proctable[i].state ) );
    }
}

char *pstate( int state )
{
    if ( state == CLNOTSET )
	return( "NOTSET" );
    else if ( state == CLSTART )
	return( "START" );
    else if ( state == CLALIVE )
	return( "ALIVE" );
    else if ( state == CLRUNNING )
	return( "RUNNING" );
    else if ( state == CLDEAD )
	return( "DEAD" );	      
    else
	return( "UNKNOWN" );
}

char *phandler( handler )
int handler;
{
    if ( handler == NOTSET )
	return "NOTSET";
    else if ( handler == CONSOLE_LISTEN )
	return "CONSOLE_LISTEN";
    else if ( handler == CONSOLE )
	return "CONSOLE";
    else if ( handler == PARENT )
	return "PARENT";
    else if ( handler == LHS )
	return "LHS";
    else if ( handler == RHS )
	return "RHS";
    else if ( handler == CLIENT_LISTEN )
	return "CLIENT_LISTEN";
    else if ( handler == CLIENT )
	return "CLIENT";
    else if ( handler == MPD )
	return "MPD";
    else if ( handler == LISTEN )
	return "LISTEN";
    else if ( handler == STDIN )
	return "STDIN";
    else if ( handler == CONTROL )
	return "CONTROL";
    else if ( handler == DATA )
	return "DATA";
    else if ( handler == MANAGER_LISTEN )
	return "MANAGER_LISTEN";
    else if ( handler == MANAGER )
	return "MANAGER";
    else if ( handler == MAN_LISTEN )
	return "MAN_LISTEN";
    else if ( handler == LHS_MSGS )
	return "LHS_MSGS";
    else if ( handler == RHS_MSGS )
	return "RHS_MSGS";
    else if ( handler == PARENT_MPD_MSGS )
	return "PARENT_MPD_MSGS";
    else if ( handler == CON_STDIN )
	return "CON_STDIN";
    else if ( handler == CON_CNTL )
	return "CON_CNTL";
    else if ( handler == MAN_CLIENT )
	return "MAN_CLIENT";
    else if ( handler == CLIENT_STDOUT )
	return "CLIENT_STDOUT";
    else if ( handler == CLIENT_STDERR )
	return "CLIENT_STDERR";
    else if ( handler == TREE_STDOUT )
	return "TREE_STDOUT";
    else if ( handler == TREE_STDERR )
	return "TREE_STDERR";
    else if ( handler == LOGFILE_OUTPUT )
	return "LOGFILE_OUTPUT";
    else
	return "UNKNOWN";
}
/*
 *	Networking routines
 */
int local_connect( name )	
char *name;
{
    int s, rc;
    struct sockaddr_un sa;

    bzero( (void *)&sa, sizeof( sa ) );

    sa.sun_family = AF_UNIX;
    strncpy( sa.sun_path, name, sizeof( sa.sun_path ) - 1 );

    s = socket( AF_UNIX, SOCK_STREAM, 0 );
    error_check( s, "local_connect: socket" );

    rc = connect( s, ( struct sockaddr * ) &sa, sizeof( sa ) );

    if ( rc != -1 ) {
	mpdprintf( debug, "local_connect; socket = %d\n", s );
	return ( s );
    }
    else
	return( rc );
}

void send_msg( fd, buf, size )	
int fd;
char *buf;
int size;
{
    int n;

    /* maybe should check whether size < MAXLINE? */
    n = write( fd, buf, size );
    if ( n < 0 )
	mpdprintf(1, "error on write; buf=:%s:\n", buf );
    error_check( n, "send_msg write" );
}

void write_line( idx, buf )	
int idx;
char *buf;
{
    int size, n;

    size = strlen( buf );
    if ( size > MAXLINE ) {
	buf[MAXLINE] = '\0';
	mpdprintf( 1, "write_line: message string too big: :%s:\n", buf );
    }
    else if ( buf[strlen( buf ) - 1] != '\n' )  /* error:  no newline at end */
	    mpdprintf( 1, "write_line: message string doesn't end in newline: :%s:\n",
		       buf );
    else {
	if ( idx != -1 ) {
	    n = write( fdtable[idx].fd, buf, size );
	    if ( n < size)
		mpdprintf( 1, "write_line did not write whole message\n" );
	    if ( n < 0 )
		mpdprintf( 1, "write_line error; fd=%d buf=:%s:\n", fdtable[idx].fd, buf );
	    error_check( n, "write_line write" );
	}
	else
	    mpdprintf( debug, "write_line attempted write to idx -1\n" );
    }
}

int setup_network_socket( port ) /* returns fd */
int *port;
{
    int backlog = 15;
    int rc;
    mpd_sockopt_len_t sinlen;
    int skt_fd;
    struct sockaddr_in sin;

    sin.sin_family	= AF_INET;
    sin.sin_addr.s_addr	= INADDR_ANY;
    sin.sin_port	= htons( *port );
    sinlen              = sizeof( sin );

    skt_fd = socket( AF_INET, SOCK_STREAM, 0 );
    error_check( skt_fd, "setup_network_socket: socket" );

    rc = bind( skt_fd, ( struct sockaddr * ) &sin, sizeof( sin ) );
    error_check( rc, "setup_network_socket: bind" );

    rc = getsockname( skt_fd, (struct sockaddr *) &sin, &sinlen ); 
    error_check( rc, "setup_network_socket: getsockname" );

    mpdprintf( 0, "network socket port is %d, len = %d\n",
	    ntohs(sin.sin_port), sinlen);
    *port = ntohs(sin.sin_port);

    rc = listen( skt_fd, backlog );
    error_check( rc, "setup_network_socket: listen" );
    mpdprintf( debug, "listening on network socket %d\n", skt_fd );

    return skt_fd;
}

/* versions of basic stream routines from Stevens */

int writen( fd, buf, n )
int fd, n;
char *buf;
{
    int nleft, nwritten;
    char *ptr;

    ptr	  = buf;
    nleft = n;

    while ( nleft > 0 ) {
	if ( ( nwritten = write( fd, ptr, nleft ) ) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;	/* try again */
	    else
		return( -1 );	/* error */
	}
	nleft -= nwritten;
	ptr   += nwritten;
    }	    
    return( n );
}

/* This function reads until it finds a newline character.  It returns the number of
   characters read, including the newline character.  The newline character is stored
   in buf, as in fgets.  It does not supply a string-terminating null character.
*/
int read_line( fd, buf, maxlen )
int fd, maxlen;
char *buf;
{
    int n, rc;
    char c, *ptr;

    ptr = buf;
    for ( n = 1; n < maxlen; n++ ) {
      again:
	if ( ( rc = read( fd, &c, 1 ) ) == 1 ) {
	    *ptr++ = c;
	    if ( c == '\n' )	/* note \n is stored, like in fgets */
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return( 0 );	/* EOF, no data read */
	    else
		break;		/* EOF, some data read */
	}
	else {
	    if ( errno == EINTR )
		goto again;
	    return ( -1 );	/* error, errno set by read */
	}
    }
    *ptr = 0;			/* null terminate, like fgets */
    return( n );
}

/*
 * 
 *  from Stevens book 
 *
 */
Sigfunc *Signal( signo, func )
int signo;
Sigfunc *func;
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    if ( signo == SIGALRM ) {
#ifdef  SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;   /* SunOS 4.x */
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;     /* SVR4, 4.4BSD */
#endif
    }
    if ( sigaction( signo,&act, &oact ) < 0 )
        return ( SIG_ERR );
    return( oact.sa_handler );
}


int parse_keyvals( char *st )
{
    char *p, *keystart, *valstart;

    if ( !st )
	return( -1 );

    keyval_tab_idx = 0;          
    p = st;
    while ( 1 ) {
	while ( *p == ' ' )
	    p++;
	/* got non-blank */
	if ( *p == '=' ) {
	    mpdprintf( 1, "parse_keyvals:  unexpected = at character %d in %s\n",
		       p - st, st );
	    return( -1 );
	}
	if ( *p == '\n' || *p == '\0' )
	    return( 0 );	/* normal exit */
	/* got normal character */
	keystart = p;		/* remember where key started */
	while ( *p != ' ' && *p != '=' && *p != '\n' && *p != '\0' )
	    p++;
	if ( *p == ' ' || *p == '\n' || *p == '\0' ) {
	    mpdprintf( 1,
		       "parse_keyvals: unexpected key delimiter at character %d in %s\n",
		       p - st, st );
	    return( -1 );
	}
        strncpy( keyval_tab[keyval_tab_idx].key, keystart, p - keystart );
	keyval_tab[keyval_tab_idx].key[p - keystart] = '\0'; /* store key */

	valstart = ++p;			/* start of value */
	while ( *p != ' ' && *p != '\n' && *p != '\0' )
	    p++;
        strncpy( keyval_tab[keyval_tab_idx].value, valstart, p - valstart );
	keyval_tab[keyval_tab_idx].value[p - valstart] = '\0'; /* store value */
	keyval_tab_idx++;
	if ( *p == ' ' )
	    continue;
	if ( *p == '\n' || *p == '\0' )
	    return( 0 );	/* value has been set to empty */
    }
}
 
#if 0
void parse_keyvals( st )
char *st;
{
    char *p, stcp[MAXLINE];

    keyval_tab_idx = 0;
    strcpy( stcp, st );
    if (! (p = strtok(stcp,"= ")) )
        return;
    strcpy(keyval_tab[keyval_tab_idx].key,p);
    strcpy(keyval_tab[keyval_tab_idx++].value,strtok(NULL," \n"));
    while ( (p = strtok(NULL,"= ")) )
    {
	strcpy(keyval_tab[keyval_tab_idx].key,p);
	strcpy(keyval_tab[keyval_tab_idx++].value,strtok(NULL," \n"));
    }
}
#endif

void dump_keyvals()
{
    int i;
    for (i=0; i < keyval_tab_idx; i++) 
	mpdprintf(1, "  %s=%s\n",keyval_tab[i].key, keyval_tab[i].value);
}

char *getval(keystr,valstr)
char *keystr;
char *valstr;
{
    int i;

    for (i=0; i < keyval_tab_idx; i++) {
       if ( strcmp( keystr, keyval_tab[i].key ) == 0 ) { 
	    strcpy( valstr, keyval_tab[i].value );
	    return valstr;
       } 
    }
    valstr[0] = '\0';
    return NULL;
}

void chgval( keystr, valstr )
char *keystr, *valstr;
{
    int i;

    for ( i = 0; i < keyval_tab_idx; i++ ) {
       if ( strcmp( keystr, keyval_tab[i].key ) == 0 )
	    strcpy( keyval_tab[i].value, valstr );
    }
}

void reconstruct_message_from_keyvals( buf )
char *buf;
{
    int i;
    char tempbuf[MAXLINE];

    buf[0] = '\0';
    for (i=0; i < keyval_tab_idx; i++) {
	sprintf( tempbuf, "%s=%s ", keyval_tab[i].key, keyval_tab[i].value );
	strcat( buf, tempbuf );
    }
    buf[strlen(buf)-1] = '\0';  /* chop off trailing blank */
    strcat( buf, "\n" );
}

void error_check( int val, char *str )	
{
    extern void fatal_error( int, char * );

    if ( val < 0 ) {
	char errmsg[80];
	sprintf( errmsg, "[%s] %s: %d", myid, str, val );
	perror( errmsg );
	mpd_cleanup();
	fatal_error( val, str );		
    }
}
/*
 *	Default Fatal exit handling routine 
 */
void def_fatalerror( int val, char *st )
{
    mpdprintf( debug, "error code=%d msg=%s\n",val,st);
    exit(val);
}

void (* _fatal_err)( int, char* ) = def_fatalerror;
/*
 *	invoke fatal error routine 
 */
void fatal_error( int val, char *str )
{
    (* _fatal_err)(val,str);
}
void set_fatalerr_handler( void (*func)(int,char *) ); /* had to prototype this first */
void set_fatalerr_handler( void (*func)(int,char *) )
{
    _fatal_err = func;
}

void usage( st )
char *st;
{
    fprintf( stderr, "Usage: %s  <options>\n", st );
    fprintf( stderr, "Options are:\n" );
    fprintf( stderr, "-h <host to connect to>\n" );
    fprintf( stderr, "-p <port to connect to>\n" );
    fprintf( stderr, "-c (allow console, the default)\n" );
    fprintf( stderr, "-n (don't allow console)\n" );
    fprintf( stderr, "-d <debug (0 or 1)>\n" );
    fprintf( stderr, "-w <working directory>\n" );
    fprintf( stderr, "-l <listener port>\n" );
    fprintf( stderr, "-b (background; daemonize)\n" );
    exit( 1 );
}

void mpd_cleanup()
{
    int i;

    if ( debug )
	dump_fdtable( "in mpd_cleanup" );
    for ( i = 0; i < MAXFDENTRIES; i++ ) {
        if ( fdtable[i].active )  {
	    mpdprintf( debug, "i=%d name=%s handler=%s\n",
	               i, fdtable[i].name, phandler(fdtable[i].handler) );
	    if ( ( fdtable[i].handler == CONSOLE_LISTEN ) )  {
                mpdprintf( debug, "unlinking  %s\n", fdtable[i].name );
                unlink( fdtable[i].name );
            }
	    else if ( ( fdtable[i].handler == LOGFILE_OUTPUT ) )  {
                mpdprintf( debug, "unlinking  %s\n", fdtable[i].name );
                unlink( fdtable[i].name );
            }
        }
    }
    /* Kill off all child processes by looping thru proctable */
    kill_allproc( SIGINT );	/* SIGKILL seems too violent */
}

void mpdprintf( int print_flag, char *fmt, ... )
{
    va_list ap;

    if (print_flag) {
	fprintf( stderr, "[%s]: ", myid );
	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	fflush( stderr );
    }
}

#define     END ' '
#define ESC_END '"'
#define     ESC '\\'
#define ESC_ESC '\''

void stuff_arg(arg,stuffed)
char arg[], stuffed[];
{
    int i,j;

    for (i=0, j=0; i < strlen(arg); i++)
    {
	switch (arg[i]) {
	    case END:
		stuffed[j++] = ESC;
		stuffed[j++] = ESC_END;
		break;
	    case ESC:
		stuffed[j++] = ESC;
		stuffed[j++] = ESC_ESC;
		break;
	    default:
		stuffed[j++] = arg[i];
	}
    }
    stuffed[j] = '\0';
}

void destuff_arg(stuffed,arg)
char stuffed[], arg[];
{
    int i,j;

    i = 0;
    j = 0;
    while (stuffed[i]) {        /* END pulled off in parse */
	switch (stuffed[i]) {
	    case ESC:
		i++;
		switch (stuffed[i]) {
		    case ESC_END:
			arg[j++] = END;
			i++;
			break;
		    case ESC_ESC:
			arg[j++] = ESC;
			i++;
			break;
		}
		break;
	    default:
		arg[j++] = stuffed[i++];
	}
    }
    arg[j] = '\0';
}

double mpd_timestamp()
{
    struct timeval tv;

    gettimeofday( &tv, ( struct timezone * ) 0 );
    return ( tv.tv_sec + (tv.tv_usec / 1000000.0) );
}

int dclose( int fd )		/* version of close for debugging */
{
    int rc;
    
    mpdprintf( debug, "closing fd %d\n", fd );
    if ( ( rc = close( fd ) ) < 0 )
	mpdprintf( 1, "failed to close fd %d\n", fd );
    return rc;
}


int map_signo( char *signo )
{
    if ( strcmp( signo, "SIGTSTP" ) == 0 )
	 return SIGTSTP;
    else if ( strcmp( signo, "SIGCONT" ) == 0 )
	 return SIGCONT;
    else if ( strcmp( signo, "SIGINT" ) == 0 )
	 return SIGINT;
    else 
	return -1;
}

void unmap_signum( int signum, char *signo )
{
    if ( signum == SIGTSTP )
	strcpy( signo, "SIGTSTP" );
    else if ( signum == SIGCONT )
	strcpy( signo, "SIGCONT" );
    else if ( signum == SIGINT )
	strcpy( signo, "SIGINT" );
    else
	strcpy( signo, "UNKNOWN_SIGNUM" );
    return;
}
	    
