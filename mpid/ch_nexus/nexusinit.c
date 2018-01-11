/*
 *  $Id: chinit.c,v 1.36 1995/08/11 00:23:53 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chinit.c,v 1.36 1995/08/11 00:23:53 gropp Exp gropp $";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#if defined(rs6000) && !defined(_ALL_SOURCE)
/* AIX's version of netinet/in.h REQUIRES but DOES NOT CHECK 
   for _ALL_SOURCE! 
   (This is needed below incase sys/types.h is loaded in these includes)
 */
#define _ALL_SOURCE
#endif

#include "mpid.h"

void (*MPID_ErrorHandler)() = MPID_DefaultErrorHandler;
/* For tracing channel operations by ADI underlayer */
FILE *MPID_TRACE_FILE = 0;

/* For debugging statements */
FILE *MPID_DEBUG_FILE = 0;

#ifdef MPID_PKT_VAR_SIZE
int MPID_PKT_DATA_SIZE = MPID_PKT_MAX_DATA_SIZE;
int MPID_SetPktSize( len )
int len;
{
if (len < 0) return MPID_PKT_MAX_DATA_SIZE;
if (len > MPID_PKT_MAX_DATA_SIZE) len = MPID_PKT_MAX_DATA_SIZE;
MPID_PKT_DATA_SIZE = len;
return len;
}
#else
int MPID_SetPktSize( len )
int len;
{
return MPID_PKT_MAX_DATA_SIZE;
}
#endif

/* #define DEBUG(a) {a} */
#define DO_DEBUG(a)

static int DebugSpace = 0;
int MPID_DebugFlag = 0;

MPID_SetSpaceDebugFlag( flag )
int flag;
{
DebugSpace = flag;
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
if (flag) {
    /* Check the validity of the malloc arena on every use of trmalloc/free */
    trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}
void MPID_SetDebugFlag( ctx, f )
void *ctx;
int f;
{
MPID_DebugFlag = f;
}

void MPID_SetDebugFile(char *name)
{
    char filename[1024];

    if (strcmp(name, "-") == 0)
    {
	MPID_DEBUG_FILE = stdout;
	return;
    }
    if (strchr(name, '%'))
    {
	sprintf(filename, name, MPID_MyWorldRank);
	MPID_DEBUG_FILE = fopen(filename, "w");
    }
    else
    {
	MPID_DEBUG_FILE = fopen(name, "w");
    }

    if (!MPID_DEBUG_FILE)
    {
	MPID_DEBUG_FILE = stdout;
    }
}

void MPID_Set_tracefile( name )
char *name;
{
char filename[1024];

if (strcmp( name, "-" ) == 0) {
    MPID_TRACE_FILE = stdout;
    return;
    }
if (strchr( name, '%' )) {
    sprintf( filename, name, MPID_MyWorldRank );
    MPID_TRACE_FILE = fopen( filename, "w" );
    }
else
    MPID_TRACE_FILE = fopen( name, "w" );

/* Is this the correct thing to do? */
if (!MPID_TRACE_FILE)
    MPID_TRACE_FILE = stdout;
}


#ifndef MPID_STAT_NONE
int MPID_n_short       = 0,         /* short messages */
    MPID_n_long        = 0,         /* long messages */
    MPID_n_unexpected  = 0,         /* unexpected messages */
    MPID_n_syncack     = 0;         /* Syncronization acknowledgments */
#endif

/***************************************************************************/
/* Some operations are completed in several stages.  To ensure that a      */
/* process does not exit from MPID_End while requests are pending, we keep */
/* track of how many are out-standing                                      */
/***************************************************************************/
int MPID_n_pending     = 0;         /* Number of uncompleted split requests */

/*****************************************************************************
  Here begin the interface routines themselves
 *****************************************************************************/
void MPID_NEXUS_Myrank( rank )
int *rank;
{
*rank = MPID_MyWorldRank;
}

void MPID_NEXUS_Mysize( size )
int *size;
{
*size = MPID_WorldSize;
}

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    This version currently returns null, as all data is static.
 */

#ifdef NEXUS_ARCH_MPL

#include <sys/signal.h>

extern void *_nx_st_mpl_info();
extern void *_nx_st_fork_info();
extern void *_nx_st_rsh_info();
extern void *_nx_st_ss_info();
extern void *_nx_pr_mpl_info();
extern void *_nx_pr_tcp_info();
extern void *_nx_database_file_info();

static nexus_module_list_t module_list[] =
{
    {"startups", "mpl", _nx_st_mpl_info},
    {"startups", "fork", _nx_st_fork_info},
    {"startups", "rsh", _nx_st_rsh_info},
    {"startups", "ss", _nx_st_ss_info},
    {"protocols", "mpl", _nx_pr_mpl_info},
    {"protocols", "tcp", _nx_pr_tcp_info},
    {"database", "file", _nx_database_file_info},
    { NULL, NULL, NULL},
};
#endif

void *MPID_NEXUS_Init( argc, argv )
int  *argc;
char ***argv;
{
    nexus_node_t *orig_nodes;
    int n_orig_nodes;

/* Set the file for Debugging output.  The actual output is controlled
   by MPIDDebugFlag */
if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;

#ifndef FORTRANM
#ifdef NEXUS_ARCH_MPL
    nexus_init(argc, argv,
	       "MPI_ARGS", "mpi",
	       NULL, NULL, NULL,
	       module_list,
	       &orig_nodes, &n_orig_nodes);
#else
    nexus_init(argc, argv,
    	       "MPI_ARGS", "mpi",
	       NULL, NULL, NULL, NULL,
	       &orig_nodes, &n_orig_nodes);
#endif

    nexus_start_nonblocking();

    if (orig_nodes)
    {
	/*
	 * This is the first node in the computation, and is the only
	 * one with nodes.  It must pass them to the others.
	 */
        MPID_NEXUS_setup_nodes(orig_nodes, n_orig_nodes);
        MPID_NEXUS_publicize_nodes(argc, argv);
    }

    MPID_NEXUS_wait_for_gps(argc, argv);

    if (orig_nodes)
    {
	MPID_NEXUS_init_hetero(NEXUS_TRUE, argc, argv);
    }
    else
    {
	MPID_NEXUS_init_hetero(NEXUS_FALSE, argc, argv);
    }

#endif /* FORTRANM */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DO_DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Finished init\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

#if defined(NEXUS_ARCH_MPL)
    signal(SIGALRM, SIG_IGN);
#endif

/* Turn off the resource monitors */
#if !defined(euih) && !defined(eui)
/* If we are euih or SP2 eui, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
;
#endif


/* Initialize any data structures in the send and receive handlers */
MPID_NEXUS_Init_recv_code();
MPID_NEXUS_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DO_DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] leaving chinit\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_NEXUS_Abort( code )
int code;
{
    nexus_fatal("Aborting program!\n");
}

void MPID_NEXUS_End()
{
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d] Entering MPID_End\n", MPID_MyWorldRank );
    }
#endif                  /* #DEBUG_END# */
/* Finish off any pending transactions */
MPID_NEXUS_Complete_pending();

if (MPID_GetMsgDebugFlag()) {
    MPID_PrintMsgDebug();
    }
#ifdef CHAMELEON_COMM       /* #CHAMELEON_START# */
if (DebugSpace)
    trdump( stdout );
#endif                      /* #CHAMELEON_END# */
/* We should really generate an error or warning message if there 
   are uncompleted operations... */
MPID_NEXUS_finalize();
}

void MPID_NEXUS_Node_name( name, len )
char *name;
int  len;
{
gethostname( name, len );
}

void MPID_NEXUS_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

#ifndef MPID_NEXUS_Wtime
#if defined(HAVE_GETTIMEOFDAY)
#include <sys/types.h>
#include <sys/time.h>
#endif
/* I don't know what the correct includes are for the other versions... */
double MPID_NEXUS_Wtime()
{
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(USE_BSDGETTIMEOFDAY)
    struct timeval tp;
    struct timezone tzp;

    BSDgettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(USE_WIERDGETTIMEOFDAY)
    /* This is for Solaris, where they decided to change the CALLING
       SEQUENCE OF gettimeofday! */
    struct timeval tp;

    gettimeofday(&tp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#else
    return SYGetElapsedTime();
#endif
}
#endif

/* This returns a value that is correct but not the best value that
   could be returned.
   It makes several separate stabs at computing the tickvalue.
*/
double MPID_NEXUS_Wtick()
{
static double tickval = -1.0;
double t1, t2;
int    cnt;
int    icnt;

if (tickval < 0.0) {
    tickval = 1.0e6;
    for (icnt=0; icnt<10; icnt++) {
	cnt = 1000;
	t1  = MPID_NEXUS_Wtime();
	while (cnt-- && (t2 = MPID_NEXUS_Wtime()) <= t1) ;
	if (cnt && t2 - t1 < tickval)
	    tickval = t2 - t1;
	}
    }
return tickval;
}

void MPID_NEXUS_Error_handler( r )
void (*r)();
{
if (r)
    MPID_ErrorHandler = r;
else
    MPID_ErrorHandler = MPID_DefaultErrorHandler;
}

void MPID_DefaultErrorHandler( code, str )
int  code;
char *str;
{
    nexus_fatal("%s\n", str);
}


/* We also need an "ErrorsReturn" and a sensible error return strategy */

/*
   Data about messages
 */
static int DebugMsgFlag = 0;
void MPID_SetMsgDebugFlag( f )
int f;
{
DebugMsgFlag = f;
}
int MPID_GetMsgDebugFlag()
{
return DebugMsgFlag;
}
void MPID_PrintMsgDebug()
{
#ifndef MPID_STAT_NONE
fprintf( stdout, "[%d] short = %d, long = %d, unexpected = %d, ack = %d\n",
	 MPID_MyWorldRank, MPID_n_short, MPID_n_long, MPID_n_unexpected, 
	 MPID_n_syncack );
#endif
}