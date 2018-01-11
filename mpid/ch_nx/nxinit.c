int __NUMNODES, __MYPROCID  ;extern double dclock();







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

void MPID_NX_Myrank( rank )
int *rank;
{
*rank = __MYPROCID;
}

void MPID_NX_Mysize( size )
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
void *MPID_NX_Init( argc, argv )
int  *argc;
char ***argv;
{
int  i;
char *work;

{__NUMNODES = numnodes();__MYPROCID = mynode();};
#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] Finished init\n", __MYPROCID );)
#endif                  /* #DEBUG_END# */

/* Turn off the resource monitors */
#if !defined(euih)
/* If we are euih, we can't use SIGALRM; this call sets SIGALRM to 
   SIG_IGN */
;
#endif


/* Initialize any data structures in the send and receive handlers */
MPID_NX_Init_recv_code();
MPID_NX_Init_send_code();

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */
DEBUG(printf("[%d] leaving chinit\n", __MYPROCID );)
#endif                  /* #DEBUG_END# */

return (void *)0;
}

void MPID_NX_Abort( code )
int code;
{
fprintf( stderr, "Aborting program!\n" );
fflush( stderr );
fflush( stdout );

killproc(-1,0);

killproc(-1,-1);

exit((char *)0);

;
}

void MPID_NX_End()
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

void MPID_NX_Node_name( name, len )
char *name;
int  len;
{
sprintf(name,"%d",__MYPROCID);
}

void MPID_NX_Version_name( name )
char *name;
{
sprintf( name, "ADI version %4.2f - transport %s", MPIDPATCHLEVEL, 
	 MPIDTRANSPORT );
}

double MPID_NX_Wtime()
{
return dclock();
}

/* This returns a value that is correct but not the best value that
   could be returned */
double MPID_NX_Wtick()
{
double t1, t2;
int    cnt;

cnt = 1000;
t1  = MPID_NX_Wtime();
while (cnt-- && (t2 = MPID_NX_Wtime()) <= t1) ;
if (!cnt) return 1.0e6;
return t2 - t1;
}

void MPID_NX_Error_handler( r )
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
MPID_NX_Abort( code );
}

int MPID_NX_Dest_byte_order( dest )
int dest;
{
if (MPID_IS_HETERO)
    return MPID_procinfo[dest].byte_order;
else 
    return MPID_H_NONE;
}

/* We also need an "ErrorsReturn" and a sensible error return strategy */

#ifdef MPID_DEBUG_ALL   /* #DEBUG_START# */

MPID_NX_Print_pkt_data( msg, address, len )
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
