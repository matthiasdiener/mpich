#include "mpi.h"

/*
   Handler prints warning messsage and starts the specified debugger.

   This code draws on one of the PETSc error handlers written by 
   Barry Smith
 */
#include <signal.h>
#include <stdio.h>
#if HAVE_STDLIB_H || STDC_HEADERS
#include <stdlib.h>
#else
#ifdef __STDC__
extern void	free(/*void * */);
extern void	*malloc(/*size_t*/);
#else
extern char *malloc();
extern int free();
#endif
#endif


#if defined(SIGSTOP)
/* 
   In order to allow more elaborate debuggers, such as starting the
   debugger in an xterm or with a -display option, we provide a
   default argument stream in baseargs.  In this case, the arguments are
   baseargs, followed by the process id of the job to stop.
   If baseargs not set, then the given debugger is used.
 */
static char *program=0;
static char *debugger=0;
static char **baseargs=0;
static int  nbaseargs=0;

MPE_Start_debugger( )
{
int child, i;

  child = fork(); 
  if (child) { /* I am the parent will run the debugger */
    char  **args, pid[10];
    args = (char **)malloc( (4 + nbaseargs) * sizeof(char *) );
    kill(child,SIGSTOP);
    sprintf(pid,"%d",child); 
    if (nbaseargs > 0) {
	for (i=0; i<nbaseargs; i++) {
	    args[i] = baseargs[i];
	    }
	args[i++] = pid;
	args[i++] = 0;
        debugger  = args[0];
	}
    else {
	args[0] = debugger; args[1] = program; args[2] = pid; args[3] = 0;
    }
    fprintf(stderr,"Attaching %s to %s %s\n", debugger, program, pid);
    if (execvp(debugger, args)  < 0) {
      perror("Unable to start debugger");
      exit(0);
    }
  }
  else { /* I am the child, continue with user code */
    sleep(10);
  }
}

/* This is the actual error handler */
void MPE_Errors_to_dbx( comm, code, string, file, line )
MPI_Comm *comm;
int      *code, *line;
char     *string, *file;
{
  char buf[MPI_MAX_ERROR_STRING];
  int  myid, result_len; 
  int child;

  if (MPI_COMM_WORLD) MPI_Comm_rank( MPI_COMM_WORLD, &myid );
  else myid = -1;
  MPI_Error_string( *code, buf, &result_len );
  fprintf( stderr, "%d -  File: %s   Line: %d\n", myid, 
		   file, *line );
  fprintf( stderr, "%d - %s : %s\n", myid, 
          string ? string : "<NO ERROR MESSAGE>", buf );

  MPE_Start_debugger();
}

/*@
   MPE_Errors_call_debugger - On an error, print a message and (attempt) to
   start the specified debugger on the program

   Input Parameters:
.  pgm - Name of the program.
.  dbg - Name of the debugger.  If null, use a default (usually dbx)
.  args - arguments to use in generating the debugger.
   This allows things like "xterm -e dbx pgm pid", or 
   "xdbx -geometry +%d+%d pgm pid".  The list should be null terminated.
   (The %d %d format is not yet supported).
@*/
void MPE_Errors_call_debugger( pgm, dbg, args )
char *pgm, *dbg, **args;
{
MPI_Errhandler err;
int            i;

if (args) {
    while (args[nbaseargs]) nbaseargs++;
    baseargs = (char **)malloc( (nbaseargs+1) * sizeof(char *) );
    for (i=0; i<=nbaseargs; i++) 
	baseargs[i] = args[i];
    }
else if (!dbg) {
    dbg = "/usr/ucb/dbx";
    }

if (!pgm) {
    fprintf( stderr, 
    "Must specify the program name when setting errors-call-debugger\n" );
    return;
    }
program = (char *)malloc( strlen(pgm) + 1 );
strcpy( program, pgm );
if (dbg) {
    debugger = (char *)malloc( strlen(dbg) + 1 );
    strcpy( debugger, dbg );
    }

MPI_Errhandler_create( MPE_Errors_to_dbx, &err );
MPI_Errhandler_set( MPI_COMM_WORLD, err );
}

char *MPER_Copy_string( str )
char *str;
{
char *new;
new = (char *)malloc( strlen(str) + 1 );
strcpy( new, str );
return new;
}

void MPE_Errors_call_xdbx( pgm, display )
char *pgm, *display;
{
char **args;

/* By default, we use the name of the root node */
if (!display) {
    extern char *getenv();
    display = getenv( "DISPLAY" );
    if (!display || display[0] == ':') {
	/* Replace display with hostname:0 */
	display = (char *)malloc( 100 );
	gethostname( display, 100 );
	strcat( display, ":0" );
	}
    }

args    = (char **)malloc( 5 * sizeof(char *) );
args[0] = MPER_Copy_string( "/usr/X11/bin/xdbx" );
args[1] = MPER_Copy_string( "-display" );
args[2] = MPER_Copy_string( display );
args[3] = MPER_Copy_string( pgm );
args[4] = 0;

MPE_Errors_call_debugger( pgm, (char *)0, args );
}

/* This routine is collective; all processes in MPI_COMM_WORLD must call */
void MPE_Errors_call_dbx_in_xterm( pgm, display )
char *pgm, *display;
{
char **args;
int  myid, str_len;

/* By default, we use the name of the root node */
if (!display) {
    MPI_Comm_rank( MPI_COMM_WORLD, &myid );
    if (myid == 0) {
	extern char *getenv();
	display = getenv( "DISPLAY" );
	if (!display || display[0] == ':') {
	    /* Replace display with hostname:0 */
	    display = (char *)malloc( 100 );
	    gethostname( display, 100 );
	    strcat( display, ":0" );
	    }
	str_len = strlen( display ) + 1;
	}
    MPI_Bcast( &str_len, 1, MPI_INT, 0, MPI_COMM_WORLD );
    if (myid != 0)
	display = (char *) malloc( str_len );
    MPI_Bcast( display, str_len, MPI_CHAR, 0, MPI_COMM_WORLD );
    }

args    = (char **)malloc( 7 * sizeof(char *) );
args[0] = MPER_Copy_string( "xterm" );
args[1] = MPER_Copy_string( "-display" );
args[2] = MPER_Copy_string( display );
args[3] = MPER_Copy_string( "-e" );
args[4] = MPER_Copy_string( "/usr/ucb/dbx" );
args[5] = MPER_Copy_string( pgm );
args[6] = 0;

MPE_Errors_call_debugger( pgm, (char *)0, args );
}

/* This routine handles signals that usually abort a user's code */
static char *SIGNAME[] = { "Unknown", "HUP", "INT", "QUIT", "ILL",
                           "TRAP",    "ABRT", "EMT", "FPE", "KILL", 
                           "BUS", "SEGV", "SYS", "PIPE", "ALRM",
                           "TERM", "URG", "STOP", "TSTP", "CONT", 
                           "CHLD" }; /* There are others, put we won't 
					report them */
/*
  MPE_DefaultHandler - Default signal handler.

  Input Parameters:
. sig   - signal value
. code,scp,addr - see the signal man page
*/
void MPE_DefaultHandler( sig, code, scp, addr )
int               sig, code;
struct sigcontext *scp;
char              *addr;
{
static char buf[128];

signal( sig, SIG_DFL );
if (sig >= 0 && sig <= 20) 
    sprintf( buf, "Caught signal %s", SIGNAME[sig] );
else
    strcpy( buf, "Caught signal " );
fprintf( stderr, "%s\n", buf );

MPE_Start_debugger( );
}

/*@
    MPE_Signals_call_debugger - Process-killing signals invoke the MPE
    error handler

    Notes:
    This invokes the MPE error handler which prints some information
    about the signal and then attempts to start the debugger.  Some
    systems will not support calling the debugger.
@*/
MPE_Signals_call_debugger()
{
signal( SIGQUIT, (void (*)())MPE_DefaultHandler );
signal( SIGILL,  (void (*)())MPE_DefaultHandler );
signal( SIGFPE,  (void (*)())MPE_DefaultHandler );
signal( SIGBUS,  (void (*)())MPE_DefaultHandler );
signal( SIGSEGV, (void (*)())MPE_DefaultHandler );
signal( SIGSYS,  (void (*)())MPE_DefaultHandler );
}

#else
/* Some systems don't support SIGSTOP; for them, we just issue a warning 
   message ... */
void MPE_Errors_call_dbx_in_xterm( pgm, display )
char *pgm, *display;
{
fprintf( stderr, "This system does not support SIGSTOP, needed to implement\n\
calling of the debugger from a this program.\n" );
}
MPE_Signals_call_debugger()
{
}

#endif
