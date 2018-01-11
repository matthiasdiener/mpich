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
#include "system/system.h"
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
    trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}

void MPID_CH_Myrank( rank )
int *rank;
{
*rank = PImytid;
}

void MPID_CH_Mysize( size )
int *size;
{
*size = PInumtids;
}

/* 
    In addition, Chameleon processes many command-line arguments 

    This should return a structure that contains any relavent context
    (for use in the multiprotocol version)

    This version currently returns null, as all data is static.
 */
void *MPID_CH_Init( argc, argv )
int  *argc;
char ***argv;
{
int  i;
char *work;

PIiInit( argc, argv );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Finished init\n", PImytid );)
#endif                  /* #DEBUG_END# */

/* Turn off the resource monitors */
#if !defined(euih)
/* If we are euih, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
SYSetResourceClockOff();
#endif

#ifdef MPID_HAS_HETERO           /* #HETERO_START# */
/* Eventually, this will also need to check for word sizes, so that systems
   with 32 bit ints can interoperate with 64 bit ints, etc. */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Checking for heterogeneous systems...\n", PImytid );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)MALLOC( PInumtids * sizeof(MPID_INFO) );
CHKPTRN(MPID_procinfo);
for (i=0; i<PInumtids; i++) {
    MPID_procinfo[i].byte_order = MPID_H_NONE;
    }
/* Set my byte ordering and convert if necessary.  Eventually, this
   should use xdr */
i = SYGetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Byte order is %d\n", PImytid, i );)
#endif                  /* #DEBUG_END# */
if (i == 1)      MPID_byte_order = MPID_H_LSB;
else if (i == 2) MPID_byte_order = MPID_H_MSB;
else             MPID_byte_order = MPID_H_XDR;
MPID_procinfo[PImytid].byte_order = MPID_byte_order;
/* Everyone uses the same format (MSB) */
if (i == 1) 
    SYByteSwapInt( (int*)&MPID_procinfo[PImytid].byte_order, 1 );

/* Get everyone else's */
work = (char *)MALLOC( PInumtids * sizeof(MPID_INFO) );
CHKPTRN( work );
/* ASSUMES MPID_INFO is ints */
PIgimax( MPID_procinfo, PInumtids, work, PSAllProcs );
FREE( work );

/* See if they are all the same */
MPID_IS_HETERO = 0;
for (i=1; i<PInumtids; i++) {
    if (MPID_procinfo[0].byte_order != MPID_procinfo[i].byte_order) {
    	MPID_IS_HETERO = 1;
    	break;
        }
    }
#endif                  /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_CH_Init_recv_code();
MPID_CH_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] leaving chinit\n", PImytid );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_CH_Abort( code )
int code;
{
fprintf( stderr, "Aborting program!\n" );
fflush( stderr );
fflush( stdout );
SYexitall( (char *)0, code );
}

void MPID_CH_End()
{
if (MPID_GetMsgDebugFlag()) {
    MPID_PrintMsgDebug();
    }
if (MPID_procinfo) 
    FREE( MPID_procinfo );
#ifdef CHAMELEON_COMM       /* #CHAMELEON_START# */
if (DebugSpace)
    trdump( stdout );
#endif                      /* #CHAMELEON_END# */
/* We should really generate an error or warning message if there 
   are uncompleted operations... */
PIiFinish();
}

void MPID_CH_Node_name( name, len )
char *name;
int  len;
{
PINodeName( name, len );
}

void MPID_CH_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

double MPID_CH_Wtime()
{
return SYGetElapsedTime();
}

/* This returns a value that is correct but not the best value that
   could be returned */
double MPID_CH_Wtick()
{
double t1, t2;
int    cnt;

cnt = 1000;
t1  = MPID_CH_Wtime();
while (cnt-- && (t2 = MPID_CH_Wtime()) <= t1) ;
if (!cnt) return 1.0e6;
return t2 - t1;
}

void MPID_CH_Error_handler( r )
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
MPID_CH_Abort( code );
}

int MPID_CH_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}

/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_CH_Print_pkt_data( msg, address, len )
char *msg;
char *address;
int  len;
{
int i; char *aa = (char *)address;

if (msg)
    printf( "[%d]%s\n", PImytid, msg );
if (len < 78 && address) {
    for (i=0; i<len; i++) {
	printf( "%x", aa[i] );
	}
    printf( "\n" );
    }
fflush( stdout );
}
#endif                  /* #DEBUG_END# */
