int __NUMNODES, __MYPROCID ,__N3LEN,__N3FROM,__N3TYPE,__N3FLAG ;
#define PI_NO_NSEND
#define PI_NO_NRECV


/*
 *  $Id: chinit.c,v 1.27 1995/01/07 20:03:34 gropp Exp $
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

/* For tracing channel operations by ADI underlayer */
FILE *MPID_TRACE_FILE = 0;

/* For debugging statements */
FILE *MPID_DEBUG_FILE = stdout;

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

void MPID_Set_tracefile( name )
char *name;
{
char filename[1024];

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

void MPID_N3_Myrank( rank )
int *rank;
{
*rank = MPID_MyWorldRank;
}

void MPID_N3_Mysize( size )
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
void *MPID_N3_Init( argc, argv )
int  *argc;
char ***argv;
{
int  i;
char *work;

{__NUMNODES = ncubesize();__MYPROCID = npid();};
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
   with 32 bit ints can interoperate with 64 bit ints, etc. */
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,
              "[%d] Checking for heterogeneous systems...\n", 
	      MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */
MPID_procinfo = (MPID_INFO *)malloc(MPID_WorldSize * sizeof(MPID_INFO) );
if (!(MPID_procinfo))exit(1);;
for (i=0; i<MPID_WorldSize; i++) {
    MPID_procinfo[i].byte_order = MPID_H_NONE;
    }
/* Set my byte ordering and convert if necessary.  Eventually, this
   should use xdr */
i = SYGetByteOrder( );
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] Byte order is %d\n",MPID_MyWorldRank, i );)
#endif                  /* #DEBUG_END# */
if (i == 1)      MPID_byte_order = MPID_H_LSB;
else if (i == 2) MPID_byte_order = MPID_H_MSB;
else             MPID_byte_order = MPID_H_XDR;
MPID_procinfo[MPID_MyWorldRank].byte_order = MPID_byte_order;
/* Everyone uses the same format (MSB) */
if (i == 1) 
    SYByteSwapInt( (int*)&MPID_procinfo[MPID_MyWorldRank].byte_order, 1 );

/* Get everyone else's */
work = (char *)malloc(MPID_WorldSize * sizeof(MPID_INFO) );
if (!(work ))exit(1);;
/* ASSUMES MPID_INFO is ints */
PIgimax( MPID_procinfo, MPID_WorldSize, work, PSAllProcs );
free(work );

/* See if they are all the same */
MPID_IS_HETERO = 0;
for (i=1; i<MPID_WorldSize; i++) {
    if (MPID_procinfo[0].byte_order != MPID_procinfo[i].byte_order) {
    	MPID_IS_HETERO = 1;
    	break;
        }
    }
#endif                  /* #HETERO_END# */

/* Initialize any data structures in the send and receive handlers */
MPID_N3_Init_recv_code();
MPID_N3_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(fprintf(MPID_DEBUG_FILE,"[%d] leaving chinit\n", MPID_MyWorldRank );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_N3_Abort( code )
int code;
{
fprintf( stderr, "Aborting program!\n" );
fflush( stderr );
fflush( stdout );
exit((char *)0);
}

void MPID_N3_End()
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
;
}

void MPID_N3_Node_name( name, len )
char *name;
int  len;
{
sprintf(name,"%d",__MYPROCID);
}

void MPID_N3_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

double MPID_N3_Wtime()
{
return (double)n_time(0);
}

/* This returns a value that is correct but not the best value that
   could be returned */
double MPID_N3_Wtick()
{
double t1, t2;
int    cnt;

cnt = 1000;
t1  = MPID_N3_Wtime();
while (cnt-- && (t2 = MPID_N3_Wtime()) <= t1) ;
if (!cnt) return 1.0e6;
return t2 - t1;
}

void MPID_N3_Error_handler( r )
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
MPID_N3_Abort( code );
}

int MPID_N3_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}

/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_N3_Print_pkt_data( msg, address, len )
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
