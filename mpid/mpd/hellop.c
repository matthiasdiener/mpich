/* Test program for mpd startup */
#include "mpdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 256

int main( argc, argv, envp )
int argc;
char *argv[];
char *envp[];
{
    FILE *outfile;
    int  rc;
    int  jobsize, rank, jobid; 
    char filename[80];
    int  i,rjob,rrank;
    char buf[MAXLINE];
    int  peer_accept_socket, peer_socket, peer_rank;
    int  my_accept_port,fd0;
    char my_hostname[256];

    rc       = MPD_Init();
    jobid    = MPD_Job();
    rank     = MPD_Rank();
    jobsize  = MPD_Size();
    gethostname(buf,MAXLINE);
    fprintf(stderr, "[%d,%d] jobsize=%d on %s : hello\n",jobid,rank,jobsize,buf );

    sprintf( filename, "hellofile.%d", rank );
    outfile = fopen( filename, "w" );
    fprintf( stdout, "Hello %d was here\n", rank );
    fprintf( outfile, "Hello %d was here\n", rank );  fflush(outfile);

    fprintf( outfile, "  argc = %d \n", argc );  fflush(outfile);
    for ( i = 0; i < argc; i++ ) 
    {
        fprintf( outfile, "  argv[%d] = %s", i, argv[i] );
	fprintf( outfile, "\n" );
	fflush(outfile);
    }
    for ( i=0; envp[i]; i++ ) {
	fprintf( outfile, "  envp[%d]=%s\n", i, envp[i] );
	fflush(outfile);
    }

    if (rank == 0)
    {
	for (i=1; i < jobsize; i++)  /* for ranks 1-N */
	{
	    rc = 0;
	    while (rc == 0)
	    {
		rc = MPD_Get_ready_peer_info(&peer_rank,&peer_socket);
	    }
	    if (rc > 0)
	    {
		rc = read_line(peer_socket,buf,256); /* socket is a socket here */
		fprintf(stderr,"[%d,%d] received buf=:%s:\n",jobid,rank,buf);
	    }
	    else if (rc == 0)
	    {
		fprintf(stderr,"[%d,%d] no ready sockets\n",jobid,rank);
	    }
	    else
	    {
		fprintf(stderr,"[%d,%d] select failed\n",jobid,rank);
	    }
	}
    }
    else
    {
	peer_socket = MPD_Connect_to_peer(jobid,0);  /* to process 0 */
	sprintf(buf,"here is a msg from %d",rank);
	write(peer_socket,buf,strlen(buf)+1);  /* socket is a socket here */
    }

    printf("%d: CALLING FINALIZE \n",rank);  fflush(stdout);

    fprintf(stderr, "rank %d exiting\n", rank );  fflush(stderr);
    MPD_Finalize();
    return(0);
}
