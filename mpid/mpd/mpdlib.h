#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

/***********************************************************
 *	mpdlib.h
 *	Function callable from MPD Client
 ***********************************************************/

/************************* function prototypes ***************************/
#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif

#if defined(__STDC__) || defined(__cplusplus)
#define USE_STDARG
#define ANSI_ARGS(x) x
#else
#define ANSI_ARGS(x) ()
#endif

int MPD_Init ANSI_ARGS(());
int MPD_Comm_spawn ANSI_ARGS(());
int MPD_Send ANSI_ARGS(());
int MPD_Recv  ANSI_ARGS(());
int MPD_Comm_size ANSI_ARGS(());
int MPD_Get_processor_name ANSI_ARGS(());
int MPD_Job ANSI_ARGS(());
int MPD_Rank ANSI_ARGS(());
int MPD_Size ANSI_ARGS(());
int MPD_Finalize ANSI_ARGS(());

#define MPD_MAX_PROCESSOR_NAME 128
#define MAXSOCKNAMELEN 128
/*
 * data type
 */
#define MPD_INT 1
#define MPD_DOUBLE 2
/*
 *   Function 
 */
#define MPD_SUM 2

struct portentry {
    int active;			/* whether this entry is filled */
    int fd;			/* fd assigned by system when opened */
    int read;			/* whether this fd should be selected for reading */
    int write;			/* whether this fd should be selected for writing */
    int portnum;		/* optional unix port number, for telling others */
    FILE *file;			/* file from fdopen, if present, for using fgets */
    int handler;		/* function to call to handle input after connection */
    char name[MAXSOCKNAMELEN];	/* name of port (optional) */
    int rank;
    int job;
};
