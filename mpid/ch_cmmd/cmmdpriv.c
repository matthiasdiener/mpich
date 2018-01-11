/* 
 * This file contains code private to the p4 implementation of the ADI device
 * Primarily, this contains the code to setup the initial environment
 * and terminate the program
 */
#include "mpid.h"
#include "packets.h"

#include <sys/file.h>
int __NUMNODES, __MYPROCID;

void MPID_CMMD_Init( argc, argv )
int *argc;
char ***argv;
{
    int numprocs, i;
    char **Argv = *($2);

    __NUMNODES = CMMD_partition_size();
    __MYPROCID = CMMD_self_address();

    numprocs = __NUMNODES;
    for (i=1; i<*($1); i++) {
	if (strcmp( Argv[i], "-np" ) == 0) {
	    /* Need to remove both args and check for missing value for -np */
	    if (i + 1 == *($1)) {
		fprintf( stderr, 
			 "Missing argument to -np for number of processes\n" );
		exit( 1 );
	    }
	    numprocs = atoi( Argv[i+1] );
	    Argv[i] = 0;
	    Argv[i+1] = 0;
	    MPID_ArgSqueeze( $1, *($2) );
	    break;
	}
    }
    if (numprocs <= 0 || numprocs > __NUMNODES) {
	fprintf( stderr, "Invalid number of processes (%d) invalid\n", 
		 numprocs );
	exit( 1 );
    }
    CMMD_reset_partition_size( numprocs ); __NUMNODES = numprocs;
    MPID_MyWorldSize = __NUMNODES;
    MPID_MyWorldRank = __MYPROCID;

    CMMD_fset_io_mode( stdout, CMMD_independent );
    CMMD_fset_io_mode( stdin, CMMD_independent );
    fcntl( fileno(stdout), F_SETFL, O_APPEND );
    CMMD_fset_io_mode( stderr, CMMD_independent );
    fcntl( fileno(stderr), F_SETFL, O_APPEND );
}

void MPID_CMMD_End()
{
    fflush( stdout );
    fflush( stderr );
}
