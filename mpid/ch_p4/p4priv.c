/* 
 * This file contains code private to the p4 implementation of the ADI device
 * Primarily, this contains the code to setup the initial environment
 * and terminate the program
 */
#include "mpid.h"
#include "mpiddev.h"
#include "mpimem.h"

int __P4FROM, __P4LEN, __P4TYPE, __P4GLOBALTYPE;

static char **P4Args = 0;

void MPID_P4_Init( argc, argv )
int *argc;
char ***argv;
{
    int narg,nlen,i,*arglen;
    char *p,*argstr;

    p4_initenv(argc,*argv);
    MPID_MyWorldRank = p4_get_my_id();
    if (!MPID_MyWorldRank) {
	p4_set_hard_errors( 0 );
	if (p4_create_procgroup()) {
	    /* Error creating procgroup.  Generate error message and
	       return */
	    MPID_Abort( (MPI_Comm)0, 1, (char *)0, 
	    "! Could not create p4 procgroup.  Possible missing file\n\
or program started without mpirun.\n" );
	    return;
	}
	p4_set_hard_errors( 1 );
    }
    MPID_MyWorldRank = p4_get_my_id();
    MPID_MyWorldSize = p4_num_total_slaves()+1;
    __P4GLOBALTYPE = 1010101010;
    if (MPID_MyWorldRank == 0) 
	p4_broadcastx( __P4GLOBALTYPE,argc,sizeof(int),P4INT);
    else {
	PIbrecv(__P4GLOBALTYPE,argc,sizeof(int),P4INT);
    }
    narg   = *(argc);
    arglen = (int *)MALLOC( narg * sizeof(int) );
    if (narg>0 && !arglen) { 
	p4_error( "Could not allocate memory for commandline arglen",narg);}
    if (PImytid==0) {
	for (i=0; i<narg; i++) 
	    arglen[i] = strlen((*(argv))[i]) + 1;
    }
    if (MPID_MyWorldRank == 0) 
	p4_broadcastx( __P4GLOBALTYPE,arglen,sizeof(int)*narg,P4INT);
    else {
	PIbrecv(__P4GLOBALTYPE,arglen,sizeof(int)*narg,P4INT);
    }
    nlen = 0;
    for (i=0; i<narg; i++) 
	nlen += arglen[i];
    argstr = (char *)MALLOC( nlen );
    if (nlen>0 && !argstr) { 
	p4_error( "Could not allocate memory for commandline args",nlen);}
    if (PImytid==0) {
	p = argstr;
	for (i=0; i<narg; i++) {
	    strcpy( p, (*argv)[i] );
	    p  += arglen[i];
	}
    }
    if (MPID_MyWorldRank == 0) 
	p4_broadcastx( __P4GLOBALTYPE,argstr,nlen,P4NOX);
    else {
	PIbrecv(__P4GLOBALTYPE,argstr,nlen,P4NOX);
        }
    if (PImytid!=0) {
	*(argv) = (char **) MALLOC( (nlen + 1) * sizeof(char *) );
	if (nlen > 0 && !*(argv)) { 
	    p4_error( "Could not allocate memory for commandline argv",nlen);}
	p = argstr;
	for (i=0; i<narg; i++) {
	    (*(argv))[i] = p;
	    p += arglen[i];
	}
	/* Some systems expect a null terminated argument string */
	(*(argv))[narg] = 0;
	P4Args = *argv;
    }
    else
	FREE(argstr);
    FREE(arglen);
}

void MPID_P4_End()
{
    if (P4Args) {
	FREE( *P4Args );
	FREE( P4Args );
    }
    p4_wait_for_end();
}
