void SY_ByteSwapInt();

int __NUMNODES, __MYPROCID ,__P4LEN,__P4TYPE,__P4FROM,__P4GLOBALTYPE ;extern void SY_GetHostName();double p4_usclock();
#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_HAS_HETERO


/*
 *  $Id: chinit.c,v 1.38 1995/12/21 22:24:16 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chinit.c,v 1.38 1995/12/21 22:24:16 gropp Exp gropp $";
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

/* #CMMD DECLARATION# */

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
#define DEBUG(a)

static int DebugSpace = 0;
int MPID_DebugFlag = 0;

void MPID_SetSpaceDebugFlag( flag )
int flag;
{
DebugSpace = flag;
}
void MPID_SetDebugFlag( ctx, f )
void *ctx;
int f;
{
MPID_DebugFlag = f;
}
void MPID_SetDebugFile( name )
char *name;
{
char filename[1024];

if (strcmp( name, "-" ) == 0) {
    MPID_DEBUG_FILE = stdout;
    return;
    }
if (strchr( name, '%' )) {
    sprintf( filename, name, MPID_MyWorldRank );
    MPID_DEBUG_FILE = fopen( filename, "w" );
    }
else
    MPID_DEBUG_FILE = fopen( name, "w" );

if (!MPID_DEBUG_FILE) MPID_DEBUG_FILE = stdout;
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
void MPID_P4_Myrank( rank )
int *rank;
{
*rank = MPID_MyWorldRank;
}

void MPID_P4_Mysize( size )
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
void *MPID_P4_Init( argc, argv )
int  *argc;
char ***argv;
{
/* Set the file for Debugging output.  The actual output is controlled
   by MPIDDebugFlag */
if (MPID_DEBUG_FILE == 0) MPID_DEBUG_FILE = stdout;

{int _narg,_nlen,_i,*_arglen;
char *_p,*_argstr;p4_initenv(argc,*(argv ));
__MYPROCID = p4_get_my_id();if (!__MYPROCID)p4_create_procgroup();
__MYPROCID = p4_get_my_id();
__NUMNODES = p4_num_total_slaves()+1;
__P4GLOBALTYPE = 1010101010;
if (__MYPROCID==0){p4_broadcastx(__P4GLOBALTYPE,argc,sizeof(int),P4INT);}
else {{char *__p4lbuf=0;__P4LEN=sizeof(int);__P4FROM= -1;__P4TYPE=__P4GLOBALTYPE;p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(argc,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};};
_narg   = *(argc);
_arglen = (int *)malloc( _narg * sizeof(int) );
if (_narg>0 && !_arglen) { 
    p4_error( "Could not allocate memory for commandline arglen",_narg);}
if (__MYPROCID==0) {
    for (_i=0; _i<_narg; _i++) 
	_arglen[_i] = strlen((*(argv ))[_i]) + 1;
    }
if (__MYPROCID==0){p4_broadcastx(__P4GLOBALTYPE,_arglen,sizeof(int)*_narg,P4INT);}
else {{char *__p4lbuf=0;__P4LEN=sizeof(int)*_narg;__P4FROM= -1;__P4TYPE=__P4GLOBALTYPE;p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(_arglen,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};};
_nlen = 0;
for (_i=0; _i<_narg; _i++) 
    _nlen += _arglen[_i];
_argstr = (char *)malloc( _nlen );
if (_nlen>0 && !_argstr) { 
    p4_error( "Could not allocate memory for commandline args",_nlen);}
if (__MYPROCID==0) {
    _p = _argstr;
    for (_i=0; _i<_narg; _i++) {
	strcpy( _p, (*argv)[_i] );
	_p  += _arglen[_i];
	}
    }
if (__MYPROCID==0){p4_broadcastx(__P4GLOBALTYPE,_argstr,_nlen,P4NOX);}
else {{char *__p4lbuf=0;__P4LEN=_nlen;__P4FROM= -1;__P4TYPE=__P4GLOBALTYPE;p4_recv(&__P4TYPE,&__P4FROM,&__p4lbuf,&__P4LEN);memcpy(_argstr,__p4lbuf,__P4LEN);p4_msg_free(__p4lbuf);};};
if (__MYPROCID!=0) {
    *(argv ) = (char **) malloc(_nlen * sizeof(char *) );
    if (_nlen > 0 && !*(argv )) { 
    p4_error( "Could not allocate memory for commandline argv",_nlen);}
    _p = _argstr;
    for (_i=0; _i<_narg; _i++) {
	(*(argv ))[_i] = _p;
	_p += _arglen[_i];
	}
    }
else
    free(_argstr);
free(_arglen);
}
;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Finished init\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

/* Turn off the resource monitors */
#if !defined(euih) && !defined(eui)
/* If we are euih or SP2 eui, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
;
#endif

#ifdef MPID_HAS_HETERO /* #HETERO_START# */
MPID_P4_init_hetero( argc, argv );
#endif                 /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_P4_Init_recv_code();
MPID_P4_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] leaving chinit\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

/* Barry Smith suggests that this indicate who is aborting the program.
   There should probably be a separate argument for whether it is a 
   user requested or internal abort.
 */
void MPID_P4_Abort( code )
int code;
{
fprintf( stderr, "[%d] Aborting program!\n", MPID_MyWorldRank );
fflush( stderr );
fflush( stdout );
p4_error((char *)0,code );
}

void MPID_P4_End()
{
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
if (MPID_DebugFlag) {
    fprintf( MPID_DEBUG_FILE,
   "[%d] Entering MPID_End\n", MPID_MyWorldRank );
    }
#endif                  /* #DEBUG_END# */
/* Finish off any pending transactions */
MPID_P4_Complete_pending();

if (MPID_GetMsgDebugFlag()) {
    MPID_PrintMsgDebug();
    }
#ifdef MPID_HAS_HETERO /* #HETERO_START# */
MPID_P4_Hetero_free();
#endif                 /* #HETERO_END# */
/* We should really generate an error or warning message if there 
   are uncompleted operations... */
p4_wait_for_end();
}

void MPID_P4_Node_name( name, len )
char *name;
int  len;
{
SY_GetHostName(name,len );
}

void MPID_P4_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

#ifndef MPID_P4_Wtime
#if defined(HAVE_GETTIMEOFDAY) || defined(HAVE_WIERDGETTIMEOFDAY)
#include <sys/types.h>
#include <sys/time.h>
#endif
/* I don't know what the correct includes are for the other versions... */
double MPID_P4_Wtime()
{
#if defined(USE_WIERDGETTIMEOFDAY)
    /* This is for Solaris, where they decided to change the CALLING
       SEQUENCE OF gettimeofday! */
    struct timeval tp;

    gettimeofday(&tp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(HAVE_GETTIMEOFDAY)
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#elif defined(USE_BSDGETTIMEOFDAY)
    struct timeval tp;
    struct timezone tzp;

    BSDgettimeofday(&tp,&tzp);
    return((double) tp.tv_sec + .000001 * (double) tp.tv_usec);
#else
    return (double)p4_usclock();
#endif
}
#endif

/* This returns a value that is correct but not the best value that
   could be returned.
   It makes several separate stabs at computing the tickvalue.
*/
double MPID_P4_Wtick()
{
static double tickval = -1.0;
double t1, t2;
int    cnt;
int    icnt;

if (tickval < 0.0) {
    tickval = 1.0e6;
    for (icnt=0; icnt<10; icnt++) {
	cnt = 1000;
	t1  = MPID_P4_Wtime();
	while (cnt-- && (t2 = MPID_P4_Wtime()) <= t1) ;
	if (cnt && t2 - t1 < tickval)
	    tickval = t2 - t1;
	}
    }
return tickval;
}

void MPID_P4_Error_handler( r )
void (*r)();
{
if (r)
    MPID_ErrorHandler = r;
else
    MPID_ErrorHandler = MPID_DefaultErrorHandler;
}

/* This is the "panic" handler.  Correctable errors should be passed on
   to the user (see MPID_CHK_MSGLEN) */
void MPID_DefaultErrorHandler( code, str )
int  code;
char *str;
{
if (str) 
    fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, str );
MPID_P4_Abort( code );
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
#if defined(solaris) || defined(HAVE_UNAME)
#include <sys/utsname.h>
#endif
#if defined(solaris) || defined(HAVE_SYSINFO)
#if defined(HAVE_SYSTEMINFO_H)
#include <sys/systeminfo.h>
#else
#ifdef HAVE_SYSINFO
#undef HAVE_SYSINFO
#endif
#endif
#endif

void SY_GetHostName( name, nlen )
int  nlen;
char *name;
{
#if defined(solaris) || defined(HAVE_UNAME)
  struct utsname utname;
  uname(&utname);
  strncpy(name,utname.nodename,nlen);
#elif defined(HAVE_GETHOSTNAME)
  gethostname(name, nlen);
#elif defined(HAVE_SYSINFO)
  sysinfo(SI_HOSTNAME, name, nlen);
#else 
  sprintf( name, "%d", __MYPROCID );
#endif
/* See if this name includes the domain */
  if (!strchr(name,'.')) {
    int  l;
    l = strlen(name);
    name[l++] = '.';
    name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
#if defined(solaris) || defined(HAVE_SYSINFO)
    sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#elif defined(HAVE_GETDOMAINNAME)
    getdomainname( name+l, nlen - l );
#endif
  }
}
int SY_GetByteOrder( )
{
int l;
char *b = (char *)&l;

l = 1;
if (b[0] == 1) return 1;
if (b[sizeof(int)-1] == 1) return 2;
return 0;
}
void SY_ByteSwapInt(buff,n)
int *buff,n;
{
  int  i,j,tmp;
  char *ptr1,*ptr2 = (char *) &tmp;
  for ( j=0; j<n; j++ ) {
    ptr1 = (char *) (&buff[j]);
    for (i=0; i<sizeof(int); i++) {
      ptr2[i] = ptr1[sizeof(int)-1-i];
    }
    buff[j] = tmp;
  }
}
