#ifndef MPDLIB_INCLUDE
#define MPDLIB_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

/***********************************************************
 *	mpdlib.h
 *	Functions callable from MPD Client
 ***********************************************************/

/************************* function prototypes ***************************/

int MPD_Init( void (*)(char *) );
int MPD_Job( void );
int MPD_Rank( void );
int MPD_Size( void );
int MPD_Peer_listen_fd( void );
int MPD_Poke_peer( int, int, char * );
int MPD_Get_peer_host_and_port( int, int, char *, int * ); 
void MPD_Abort( int );
int MPD_Finalize( void );
int MPD_Man_msgs_fd( void );
int MPD_Test_connections( int *, int * );
int MPD_Request_connect_from_peer( int, int );

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

#endif
