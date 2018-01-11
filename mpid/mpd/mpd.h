/* mpd.h */
#ifndef _MPD
#define _MPD

#include "mpdconf.h"

/* mpduser.h includes any external values that uses can access */
#include "mpduser.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/socket.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <sys/un.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <pwd.h>

#ifndef CONSOLE_NAME    
#define CONSOLE_NAME "/tmp/mpd.console"
#endif
#ifndef LOGFILE_NAME
#define LOGFILE_NAME "/tmp/mpd.logfile"
#endif

#define RECV_OK    0
#define RECV_EOF  -1
#define DEFAULT_P4_PORT           6001
#define DEFAULT_NEXT_SIBLING_PORT 6002
#define DEFAULT_PREV_SIBLING_PORT 6003

/*
char *sys_errlist[];
*/

/* Macros to convert from integer to net byte order and vice versa */
#define i_to_n(n)  (int) htonl( (u_long) n)
#define n_to_i(n)  (int) ntohl( (u_long) n)

#define MAXFDENTRIES   256
#define MAXJOBS         32
#define MAXPROCS       128
#define MAXFILES        32
#define MAXHOSTNMLEN    64
#define MAXSOCKNAMELEN 128
#define MAXFILENAMELEN 256
#define MAXGIDS         32
#define STREAMBUFSIZE 4096
#define IDSIZE        (MAXHOSTNMLEN+8)
#define PASSWDLEN       80

#define CHUNKSIZE        3
#define BIGCHUNKSIZE     6      /* must be multiple of chunksize */

/* MAXLINE is in mpduser.h */

struct fdentry {
    int active;			/* whether this entry is filled */
    int fd;			/* fd assigned by system when opened */
    int job;			/* job of owner*/
    int rank;			/* rank of owner*/
    int read;			/* whether this fd should be selected for reading */
    int write;			/* whether this fd should be selected for writing */
    int portnum;		/* optional unix port number, for debugging */
    int rn;                     /* place to stash challenge number */
    FILE *file;			/* file from fdopen, if present, for using fgets */
    int handler;		/* function to call to handle input after connection */
    char name[MAXSOCKNAMELEN];	/* name of fd, for debugging */
};

struct jobentry {
    int active;
    int jobid;
    int jobsize;
    int jobsync_is_here;
    int alive_here_sofar;
    int alive_in_job_sofar;
    int added_to_job_sofar;
};

/* 
 *	State of MPD client process 
 */
#define CLSTART		1
#define CLALIVE		2    
#define CLNOTSET	3    
#define CLRUNNING	4    
#define CLDEAD          5

struct procentry {
    int active;			/* whether this entry is filled */
    int state;			/* state of client 		*/
    int pid;			/* process id of forked process */
    int jobid;			/* job that this process is part of, -1 if independent */
    int jobrank;		/* rank of this process in its job */
    int clientfd;		/* fd on which this process is connected to mpd */
    int lport;			/* port number on which this process is listening */
    char name[MAXSOCKNAMELEN];	/* name of port (optional) */
};

struct fileentry {
    int active;			/* whether this entry is filled */
    int fd;			/* file descriptor */
    int conn_id;		/* connection id */
    char name[MAXFILENAMELEN];	/* pathname */
};

struct keyval_pairs
{
    char key[32];
    char value[MAXLINE];	
};

/* handlers */
#define NOTSET          0
#define CONSOLE_LISTEN  1
#define CONSOLE         2
#define PARENT	        3
#define LHS             4
#define RHS             5
#define CLIENT_LISTEN   6
#define CLIENT          7
#define MPD             9
#define LISTEN	        8
#define STDIN          10
#define CONTROL        11
#define DATA           12
#define MANAGER_LISTEN 13
#define MANAGER        14
#define LOGFILE_OUTPUT 15
#define NEWCONN        16
/* manager handlers */
#define  MAN_LISTEN         100
#define  LHS_MSGS           200
#define  RHS_MSGS           300
#define  PARENT_MPD_MSGS    400
#define  CON_STDIN          500
#define  CON_CNTL           600
#define  MAN_CLIENT         700
#define  CLIENT_STDOUT      800
#define  CLIENT_STDERR      900
#define  TREE_STDOUT        1000
#define  TREE_STDERR        1100

/************************* function prototypes ***************************/
#ifdef USE_SOCKLEN_T
typedef socklen_t mpd_sockopt_len_t;
#elif defined(USE_SIZE_T_FOR_SOCKLEN_T)
typedef size_t mpd_sockopt_len_t;
#else
typedef int mpd_sockopt_len_t;
#endif

void mpd_cleanup( void );
void handle_input_fd( int );
int  setup_unix_socket( char * );
int  setup_network_socket( int * );
int  network_connect( char *, int );
int  local_connect( char * );
void handle_console_input( int );
void handle_manager_input( int );
int  accept_connection( int );
int  accept_unix_connection( int );
int  recv_msg( int, char * );
void write_line( int, char * );
void error_check( int, char * );
void handle_p4_input( int );
void handle_listener_input( int );
void handle_console_listener_input( int );
void handle_client_listener_input( int );
void handle_client_input( int );
void handle_sibling_input( int );
void process_sibling_input( char *, int );
void process_client_input( char * );
void handle_next_output( int );
int  allocate_port( void );
void chgval( char *, char * );
void deallocate_port( int );
int  allocate_fileent( void );
void deallocate_fileent( int );
void dump_jobtable( int );
void init_jobtable( void );
int  allocate_jobent( void );
int  find_jobid_in_jobtable( int );
void remove_from_jobtable( int );
void deallocate_jobent( int );
void init_proctable( void );
int  allocate_procent( void );
void remove_from_proctable( int );
void deallocate_procent( int );
char *phandler( int );
char *pstate( int );
int  count_hosts( void );
void dump_porttable( char * );
void dump_proctable( char * );
int  allocate_jobid( void );
int  find_host( int );
void handle_console_fd( int );
void handle_mpd_input( int );
void handle_stdin_input( int );
void unlink_clients( void );
void process_next_input( char *, int );
void usage( char * );
void usage_mpirun( void );
void mpdprintf( int, char *, ... );
void init_fdtable( void );
void dump_fdtable( char * );
int  allocate_fdentry( void );
void deallocate_fdentry( int );
void handle_lhs_input( int );
void handle_rhs_input( int );
void handle_newconn_input( int );
void newconn_challenge( int );
void newconn_new_lhs_req( int );
void newconn_new_rhs_req( int );
void newconn_new_lhs( int );
void newconn_new_rhs( int );
void sib_ping( void );
void sib_ping_ack( void );
void sib_ringtest( void );
void sib_trace( void );
void sib_trace_info( void );
void sib_trace_trailer( void );
void sib_listjobs( void );
void sib_listjobs_info( void );
void sib_listjobs_trailer( void );
void sib_signaljob( void );
void sib_dump( void );
void sib_mandump( void );
void sib_trace_ack( void );
void sib_rhs2info( int );
void sib_reconnect_rhs( int );
void sib_killjob( void );
void sib_exit( void );
void sib_jobsync( void );
void sib_jobgo( void );
void sib_bomb( void );
void sib_findclient( void );
void sib_foundclient( void );
void sib_debug( void );
void sib_allexit( void );
void sib_mpexec( void );
void sib_needjobids( void );
void sib_newjobids( void );
void fatal_error( int, char * );
void dump_keyvals( void );
void reconstruct_message_from_keyvals( char * );
void kill_job( int, int );
void kill_rank( int, int, int );
void kill_allproc( int );
int  find_proclisten( int, int );
int  find_proclisten_pid( int, int );
void con_pkill( char * );
void con_ringtest( void );
void con_trace( void );
void con_dump( void );
void con_ping( void );
void con_exit( void );
void con_bomb( void );
void con_addmpd( char * );
void con_listjobs( void );
void con_killjob( void );
void con_signaljob( void );
void con_mandump( void );
void con_debug( void );
void con_mpexec( void );
void con_allexit( void );
void cli_alive( int );
void cli_findclient( int );
void stuff_arg( char *, char * );
void destuff_arg( char *, char * );
void init_jobids( void );
int  steal_jobids( int *, int * );
void add_jobids( int, int );
int  dclose( int );
int  map_signo( char * );
void unmap_signum( int, char * );
int  captured_io_sockets_closed( void );
void init_fileent( void );
void def_fatalerror( int, char * );
int  get_local_pw( char *, int );
int  generate_shmemkey( int, int, int );
void encode_num( int, char * );
int  parse_groups( char *, int[], int * );

/* for rio */
void handle_rio_listen_input( int );
void handle_control_input( int );
void handle_data_inout( int );
void process_control_command( char * );
double mpd_timestamp( void );

/* couldn't find on Solaris */
/* int gethostname(char *, int );   */

/* from Stevens book */
typedef void Sigfunc( int );
Sigfunc *Signal( int, Sigfunc * );
void sigchld_handler( int );
void sigusr1_handler( int );
void sigint_handler( int );
int writen( int, char *, int );

/* manager stuff */

#define DUMMYHOSTNAME "_dummyhost_"
#define DUMMYPORTNUM  -2

#endif /* _MPD */

