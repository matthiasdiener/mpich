#include <stdio.h>
#include "cmnargs.h"
#include "mpid.h"
#ifdef MPIR_MEMDEBUG
#include "tr2.h"
#endif
#ifdef MPID_FLOW_CONTROL
void MPID_FlowDebug ANSI_ARGS(( int ));
#endif

/*
 * This file contains common argument handling routines for MPI ADIs
 * 
 */
/*
   MPID_ArgSqueeze - Remove all null arguments from an arg vector; 
   update the number of arguments.
 */
void MPID_ArgSqueeze( Argc, argv )
int  *Argc;
char **argv;
{
    int argc, i, j;
    
    /* Compress out the eliminated args */
    argc = *Argc;
    j    = 0;
    i    = 0;
    while (j < argc) {
	while (argv[j] == 0 && j < argc) j++;
	if (j < argc) argv[i++] = argv[j++];
    }
    /* Back off the last value if it is null */
    if (!argv[i-1]) i--;
    *Argc = i;
}

void MPID_ProcessArgs( argc, argv )
int *argc;
char ***argv;
{
int i;

char **str;

if (argv && *argv) {
    for (i=1; i<*argc; i++) {
	str = (*argv)+i;
	if (str && *str) {
	    if (strcmp(*str,"-mpipktsize" ) == 0) {
		int len;
		*str = 0;
		i++;
		if (i <*argc) {
		    len = atoi( (*argv)[i] );
		    MPID_SetPktSize( len );
		    (*argv)[i] = 0;
		    }
		else {
		    printf( "Missing argument for -mpipktsize\n" );
		    }
		}
#ifdef HAVE_NICE
	    else if (strcmp(*str,"-mpinice" ) == 0) {
		int niceincr;
		*str = 0;
		i++;
		if (i <*argc) {
		    niceincr = atoi( (*argv)[i] );
		    nice(niceincr);
		    (*argv)[i] = 0;
		    }
		else {
		    printf( "Missing argument for -mpinice\n" );
		    }
		}
#endif
#ifdef MPID_HAS_DEBUG
	    else if (strcmp(*str,"-mpichdebug") == 0) {
		MPID_SetDebugFlag( 1 );
		*str = 0;
		}
	    else if (strcmp(*str,"-mpidbfile" ) == 0) {
		MPID_SetDebugFlag( 1 );
		*str = 0;
		i++;
		if (i <*argc) {
		    MPID_SetDebugFile( (*argv)[i] );
		    (*argv)[i] = 0;
		    }
		else {
		    printf( "Missing filename for -mpdbfile\n" );
		    }
		}
	    else if (strcmp(*str,"-chmemdebug" ) == 0) {
		MPID_SetSpaceDebugFlag( 1 );
		*str = 0;
		}
	    else if (strcmp(*str,"-mpichmsg" ) == 0) {
		MPID_SetMsgDebugFlag( 1 );
		*str = 0;
		}
	    else if (strcmp(*str,"-mpitrace" ) == 0) {
		*str = 0;
		i++;
		if (i <*argc) {
		    MPID_Set_tracefile( (*argv)[i] );
		    (*argv)[i] = 0;
		    }
		else {
		    printf( "Missing filename for -mpitrace\n" );
		    }
		}
#endif
#ifdef MPIR_MEMDEBUG
	    else if (strcmp(*str,"-mpimem" ) == 0) {
		MPID_trDebugLevel( 1 );
	    }
#endif
#ifdef MPID_FLOW_CONTROL
	    else if (strcmp( *str, "-mpidbflow" ) == 0) {
		MPID_FlowDebug( 1 );
	    }
#endif
	    }
	}
    /* Remove the null arguments */
    MPID_ArgSqueeze( argc, *argv ); 
    }
}
