/*
 *  $Id: chdebug.c,v 1.1.1.1 1997/02/18 16:47:12 thiruvat Exp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */


#include "mpid.h"
#include "dev.h"
#include <string.h>
#include <stdio.h>

/* 
   Unfortunately, stderr is not a guarenteed to be a compile-time
   constant in ANSI C, so we can't initialize MPID_DEBUG_FILE with
   stderr.  Instead, we set it to null, and check for null.  Note
   that stdout is used in chinit.c 
 */
FILE *MPID_DEBUG_FILE = 0;
FILE *MPID_TRACE_FILE = 0;
int MPID_DebugFlag = 0;

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
    
    if (!MPID_TRACE_FILE) MPID_TRACE_FILE = stdout;
}


void MPID_SetSpaceDebugFlag( flag )
int flag;
{
/*      DebugSpace = flag; */
#ifdef CHAMELEON_COMM   /* #CHAMELEON_START# */
/* This file may be used to generate non-Chameleon versions */
    if (flag) {
	/* Check the validity of the malloc arena on every use of 
	   trmalloc/free */
	trDebugLevel( 1 );
    }
#endif                  /* #CHAMELEON_END# */
}

void MPID_SetDebugFlag( f )
int f;
{
    MPID_DebugFlag = f;
}

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
}
