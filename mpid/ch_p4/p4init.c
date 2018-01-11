void SY_ByteSwapInt();

int __NUMNODES, __MYPROCID ,__P4LEN,__P4TYPE,__P4FROM,__P4GLOBALTYPE ;extern void SY_GetHostName();double p4_usclock();
#define PI_NO_NSEND
#define PI_NO_NRECV
#define MPID_HAS_HETERO


/*
 *  $Id: chinit.c,v 1.34 1995/06/30 17:35:45 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id: chinit.c,v 1.34 1995/06/30 17:35:45 gropp Exp gropp $";
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

MPID_INFO *MPID_procinfo = 0;
MPID_H_TYPE MPID_byte_order;
static char *(ByteOrderName[]) = { "None", "LSB", "MSB", "XDR" };
int MPID_IS_HETERO = 0;
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

MPID_SetSpaceDebugFlag( flag )
int flag;
{
DebugSpace = flag;
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
if (flag) {
    /* Check the validity of the malloc arena on every use of trmalloc/free */
    ;
    }
#endif                  /* #CHAMELEON_END# */
}
void MPID_SetDebugFlag( ctx, f )
void *ctx;
int f;
{
MPID_DebugFlag = f;
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
#ifdef MPID_HAS_HETERO     /* #HETERO_START# */
int  i, use_xdr;
char *work;
#endif                     /* #HETERO_END# */

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

#ifdef MPID_HAS_HETERO           /* #HETERO_START# */
/* Eventually, this will also need to check for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. 
   We can check just the basic signed types: short, int, long, float, double 
   Eventually, we should probably also check the long long and long double
   types.
   We do this by collecting an array of word sizes, with each processor
   contributing the 5 lengths.  This is combined with the "byte order"
   field.

   We still need to identify IEEE and non-IEEE systems.  Perhaps we'll
   just use a field from configure (only CRAY vector systems are non IEEE
   of the systems we interact with).
 */
/*
   We look for the argument -mpixdr to force use of XDR for debugging and
   timing comparision.
 */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,
              "[%d] Checking for heterogeneous systems...\n", 
	      MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)malloc(MPID_WorldSize * sizeof(MPID_INFO) );
if (!(MPID_procinfo))exit(1);;
for (i=0; i<MPID_WorldSize; i++) {
    MPID_procinfo[i].byte_order	 = MPID_H_NONE;
    MPID_procinfo[i].short_size	 = 0;
    MPID_procinfo[i].int_size	 = 0;
    MPID_procinfo[i].long_size	 = 0;
    MPID_procinfo[i].float_size	 = 0;
    MPID_procinfo[i].double_size = 0;
    MPID_procinfo[i].float_type  = 0;
    }
/* Set my byte ordering and convert if necessary.  */

/* Set the floating point type.  IEEE is 0, Cray is 2, others as we add them
   (MPID_FLOAT_TYPE?) Not yet set: VAX floating point,
   IBM 360/370 floating point, and other. */
#ifdef MPID_FLOAT_CRAY
MPID_procinfo[MPID_MyWorldRank].float_type = 2;
#endif
use_xdr = 0;
/* printf ("Checking args for -mpixdr\n" ); */
for (i=1; i<*argc; i++) {
    /* printf( "Arg[%d] is %s\n", i, (*argv)[i] ? (*argv)[i] : "<NULL>" ); */
    if ((*argv)[i] && strcmp( (*argv)[i], "-mpixdr" ) == 0) {
	/* printf( "Found -mpixdr\n" ); */
	use_xdr = 1;
	break;
	}
    }
if (use_xdr) 
    MPID_byte_order = MPID_H_XDR;
else {
    i = SY_GetByteOrder();
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Byte order is %d\n",MPID_MyWorldRank, i );)
#endif                  /* #DEBUG_END# */
    if (i == 1)      MPID_byte_order = MPID_H_LSB;
    else if (i == 2) MPID_byte_order = MPID_H_MSB;
    else             MPID_byte_order = MPID_H_XDR;
    }
MPID_procinfo[MPID_MyWorldRank].byte_order  = MPID_byte_order;
MPID_procinfo[MPID_MyWorldRank].short_size  = sizeof(short);
MPID_procinfo[MPID_MyWorldRank].int_size    = sizeof(int);
MPID_procinfo[MPID_MyWorldRank].long_size   = sizeof(long);
MPID_procinfo[MPID_MyWorldRank].float_size  = sizeof(float);
MPID_procinfo[MPID_MyWorldRank].double_size = sizeof(double);

/* Everyone uses the same format (MSB) */
/* This should use network byte order OR the native collective operation 
   with heterogeneous support */
/* if (i == 1) 
    SY_ByteSwapInt((int*)&MPID_procinfo[MPID_MyWorldRank].byte_order,1 );
 */
/* Get everyone else's */
work = (char *)malloc(MPID_WorldSize * sizeof(MPID_INFO) );
if (!(work ))exit(1);;
/* ASSUMES MPID_INFO is ints */
p4_global_op(__P4GLOBALTYPE,MPID_procinfo,7 * MPID_WorldSize,sizeof(int),p4_int_max_op,P4INT);
free(work );

/* See if they are all the same and different from XDR*/
MPID_IS_HETERO = MPID_procinfo[0].byte_order == MPID_H_XDR;
for (i=1; i<MPID_WorldSize; i++) {
    if (MPID_procinfo[0].byte_order  != MPID_procinfo[i].byte_order ||
	MPID_procinfo[i].byte_order  == MPID_H_XDR ||
	MPID_procinfo[0].short_size  != MPID_procinfo[i].short_size ||
	MPID_procinfo[0].int_size    != MPID_procinfo[i].int_size ||
	MPID_procinfo[0].long_size   != MPID_procinfo[i].long_size ||
	MPID_procinfo[0].float_size  != MPID_procinfo[i].float_size ||
	MPID_procinfo[0].double_size != MPID_procinfo[i].double_size ||
	MPID_procinfo[0].float_type  != MPID_procinfo[i].float_type) {
    	MPID_IS_HETERO = 1;
	break;
        }
    }
/* 
   When deciding to use XDR, we need to check for size as well (if 
   [myid].xxx_size != [j].xxx_size, set [j].byte_order = XDR).  Note that 
   this is reflexive; if j decides that i needs XDR, i will also decide that
   j needs XDR;
 */
if (MPID_IS_HETERO) {
    for (i=0; i<MPID_WorldSize; i++) {
	if (i == MPID_MyWorldRank) continue;
	if (MPID_procinfo[MPID_MyWorldRank].short_size  != 
	    MPID_procinfo[i].short_size ||
	    MPID_procinfo[MPID_MyWorldRank].int_size    != 
	    MPID_procinfo[i].int_size ||
	    MPID_procinfo[MPID_MyWorldRank].long_size   != 
	    MPID_procinfo[i].long_size ||
	    MPID_procinfo[MPID_MyWorldRank].float_size  != 
	    MPID_procinfo[i].float_size ||
	    MPID_procinfo[MPID_MyWorldRank].double_size != 
	    MPID_procinfo[i].double_size ||
	    MPID_procinfo[MPID_MyWorldRank].float_type !=
	    MPID_procinfo[i].float_type) {
	    MPID_procinfo[i].byte_order = MPID_H_XDR;
	    }
	}
    }
#ifdef FOO
if (MPID_IS_HETERO && MPID_MyWorldRank == 0) {
    printf( "Warning: heterogenity only partially supported\n" );
    printf( "Ordering short int long float double sizes float-type\n" );
    for (i=0; i<MPID_WorldSize; i++) {
	printf( "<%d> %s %d %d %d %d %d %d\n", i,
	        ByteOrderName[MPID_procinfo[i].byte_order],
	        MPID_procinfo[i].short_size, 
	        MPID_procinfo[i].int_size, 
	        MPID_procinfo[i].long_size, 
	        MPID_procinfo[i].float_size, 
	        MPID_procinfo[i].double_size,
	        MPID_procinfo[i].float_type );
	}
    }
#endif
#endif                  /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_P4_Init_recv_code();
MPID_P4_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] leaving chinit\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

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
if (MPID_procinfo) 
    free(MPID_procinfo );
#ifdef CHAMELEON_COMM       /* #CHAMELEON_START# */
if (DebugSpace)
    ;
#endif                      /* #CHAMELEON_END# */
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
#if defined(HAVE_GETTIMEOFDAY)
#include <sys/types.h>
#include <sys/time.h>
#endif
/* I don't know what the correct includes are for the other versions... */
double MPID_P4_Wtime()
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

void MPID_DefaultErrorHandler( code, str )
int  code;
char *str;
{
if (str) 
    fprintf( stderr, "[%d] %s\n", MPID_MyWorldRank, str );
MPID_P4_Abort( code );
}

int MPID_P4_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}


#ifdef MPID_HAS_HETERO       /* #HETERO_START# */
/* This routine is ensure that the elements in the packet HEADER can
   be read by the receiver without further processing (unless XDR is
   used, in which case we use network byte order)
   This routine is defined ONLY for heterogeneous systems

   Note that different packets have different lengths and layouts; 
   this makes the conversion more troublesome.  I'm still thinking
   about how to do this.
 */
#include <sys/types.h>
#include <netinet/in.h>
MPID_P4_Pkt_pack( pkt, size, dest )
MPID_PKT_T *pkt;
int        size, dest;
{
int i;
unsigned int *d;
if (MPID_IS_HETERO &&
    MPID_procinfo[dest].byte_order != MPID_byte_order) {
    
    if (MPID_procinfo[dest].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/sizeof(int); i++) {
	    *d = htonl(*d);
	    d++;
	    }
	}
    else {
	/* Need to swap to receiver's order.  We ALWAYS reorder at the
	   sender's end (this is because a message can be received with 
	   MPI_Recv instead of MPI_Recv/MPI_Unpack, and hence requires us 
	   to use a format that matches the receiver's ordering without 
	   requiring a user-unpack.  */
	SY_ByteSwapInt((int*)pkt,size / sizeof(int) );
	}
    }
}
MPID_P4_Pkt_unpack( pkt, size, from )
MPID_PKT_T *pkt;
int        size, from;
{
int i;
unsigned int *d;
if (MPID_IS_HETERO &&
    MPID_procinfo[from].byte_order != MPID_byte_order) {
    
    if (MPID_procinfo[from].byte_order == MPID_H_XDR ||
	MPID_byte_order == MPID_H_XDR) {
	d = (unsigned int *)pkt;
	for (i=0; i<size/sizeof(int); i++) {
	    *d = ntohl(*d);
	    d++;
	    }
	}
    }
}
#endif                       /* #HETERO_END# */


/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_P4_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
int i; char *aa = (char *)address;

if (msg)
    printf( "[%d]%s\n", MPID_MyWorldRank, msg );
if (len < 78 && address) {
    for (i=0; i<len; i++) {
	printf( "%x", aa[i] );
	}
    printf( "\n" );
    }
fflush( stdout );
}
#endif                  /* #DEBUG_END# */

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
#include <sys/systeminfo.h>
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
