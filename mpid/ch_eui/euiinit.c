int __NUMNODES, __MYPROCID ,__ALLGRP,__EUILEN,__EUIFROM,__EUITYPE ;static double __EUI_DBL;extern double SY_GetElapsedTime();extern void SY_GetHostName();



#define PI_NO_MSG_SEMANTICS
/*
 *  $Id: chinit.c,v 1.24 1994/10/24 22:03:23 gropp Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char SCCSid[] = "%W% %G%";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it
 */

#include "mpid.h"

/* #CMMD DECLARATION# */

MPID_INFO *MPID_procinfo = 0;
MPID_H_TYPE MPID_byte_order;
int MPID_IS_HETERO = 0;
void (*MPID_ErrorHandler)() = MPID_DefaultErrorHandler;

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

void MPID_EUI_Myrank( rank )
int *rank;
{
*rank = __MYPROCID;
}

void MPID_EUI_Mysize( size )
int *size;
{
*size = __NUMNODES;
}

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    This version currently returns null, as all data is static.
 */
void *MPID_EUI_Init( argc, argv )
int  *argc;
char ***argv;
{
int  i;
char *work;

mp_environ(&__NUMNODES, &__MYPROCID );;
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Finished init\n", __MYPROCID );)
#endif                  /* #DEBUG_END# */

/* Turn off the resource monitors */
#if !defined(euih)
/* If we are euih, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
;
#endif

#ifdef MPID_HAS_HETERO           /* #HETERO_START# */
/* Eventually, this will also need to check for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Checking for heterogeneous systems...\n", __MYPROCID );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)malloc(__NUMNODES * sizeof(MPID_INFO) );
if (!(MPID_procinfo))exit(1);;
for (i=0; i<__NUMNODES; i++) {
    MPID_procinfo[i].byte_order = MPID_H_NONE;
    }
/* Set my byte ordering and convert if necessary.  Eventually, this
   should use xdr */
i = SYGetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Byte order is %d\n", __MYPROCID, i );)
#endif                  /* #DEBUG_END# */
if (i == 1)      MPID_byte_order = MPID_H_LSB;
else if (i == 2) MPID_byte_order = MPID_H_MSB;
else             MPID_byte_order = MPID_H_XDR;
MPID_procinfo[__MYPROCID].byte_order = MPID_byte_order;
/* Everyone uses the same format (MSB) */
if (i == 1) 
    SYByteSwapInt( (int*)&MPID_procinfo[__MYPROCID].byte_order, 1 );

/* Get everyone else's */
work = (char *)malloc(__NUMNODES * sizeof(MPID_INFO) );
if (!(work ))exit(1);;
/* ASSUMES MPID_INFO is ints */
PIgimax( MPID_procinfo, __NUMNODES, work, PSAllProcs );
free(work );

/* See if they are all the same */
MPID_IS_HETERO = 0;
for (i=1; i<__NUMNODES; i++) {
    if (MPID_procinfo[0].byte_order != MPID_procinfo[i].byte_order) {
    	MPID_IS_HETERO = 1;
    	break;
        }
    }
#endif                  /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_EUI_Init_recv_code();
MPID_EUI_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] leaving chinit\n", __MYPROCID );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_EUI_Abort( code )
int code;
{
fprintf( stderr, "Aborting program!\n" );
fflush( stderr );
fflush( stdout );
exit((char *)0);
}

void MPID_EUI_End()
{
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
fflush(stdout);fflush(stderr);mp_sync(&__ALLGRP);;
}

void MPID_EUI_Node_name( name, len )
char *name;
int  len;
{
SY_GetHostName(name,len );
}

void MPID_EUI_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

double MPID_EUI_Wtime()
{
return 
#ifdef MPI_euih
(tb0time(&__EUI_DBL),__EUI_DBL*0.001)
#else
SY_GetElapsedTime()
#endif
;
}

/* This returns a value that is correct but not the best value that
   could be returned */
double MPID_EUI_Wtick()
{
double t1, t2;
int    cnt;

cnt = 1000;
t1  = MPID_EUI_Wtime();
while (cnt-- && (t2 = MPID_EUI_Wtime()) <= t1) ;
if (!cnt) return 1.0e6;
return t2 - t1;
}

void MPID_EUI_Error_handler( r )
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
    fprintf( stderr, "%s\n", str );
MPID_EUI_Abort( code );
}

int MPID_EUI_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}

/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_EUI_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
int i; char *aa = (char *)address;

if (msg)
    printf( "[%d]%s\n", __MYPROCID, msg );
if (len < 78 && address) {
    for (i=0; i<len; i++) {
	printf( "%x", aa[i] );
	}
    printf( "\n" );
    }
fflush( stdout );
}
#endif                  /* #DEBUG_END# */
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
#include <sys/types.h>      
#include <sys/time.h>       
#include <time.h>
static struct timeval ELTime = { 0, 0 };
static int            NoBase = 1;
double SY_GetElapsedTime()
{
struct timeval tp;

gettimeofday( &tp, (struct timezone *)0 );
if (NoBase) {
    ELTime.tv_sec  = tp.tv_sec;
    ELTime.tv_usec = tp.tv_usec;
    NoBase = 0;
    return 0.0;
    }
return (double)(tp.tv_sec - ELTime.tv_sec) + 
       (1.0e-6)*(tp.tv_usec - ELTime.tv_usec);
}
