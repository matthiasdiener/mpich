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
    int i, rank = 99999; 
    char filename[80];

    i = MPD_Init();
    sprintf( filename, "hellofile.%d", rank );
    outfile = fopen( filename, "w" );
    i = MPD_Rank();
    fprintf( stdout,  "FROM %d \n", i );
    fprintf( outfile, "FROM %d\n", i );

    return(0);
}
