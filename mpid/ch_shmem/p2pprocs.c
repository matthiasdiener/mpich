/*
 * This file contains the routines to implement "process" creation from
 * an existing process.  The new processes run the same executable as the
 * old process, and may or may not share address space with it.
 *
 * Currently, this system supports two approaches: full processes (with fork)
 * or a special kind of "thread with private memory".  Unfortunately, most
 * thread systems run a separate routine in the new thread, rather than simply
 * starting a new thread of control from the current point.  To work around 
 * this,  we exploit the fact that in MPI, we are allowed to call MPI_Init
 * only once, and the number of "processes" running before MPI_Init is 
 * undefined.  Thus, we can call a routine which calls the user's main 
 * program over again.  In other situations, we migh use setjmp/longjmp.
 * Note that the status of "shared" data may be different in these two
 * environments; in a fork, all data is available to the new processes (copy
 * on write).  In a thread that calls main, any initialization code (including 
 * "int a = 0;" may be re-executed.
 *
 * There is a separate kind of fork supported on SGI systems, called sproc.
 * This is like thread creation, in that it insists on calling a routine.
 *
 * For several reasons, these routines currently should be called at most once.
 * They are less general than a fork or thread create.  MPI_Spawn (or whatever)
 * will require different code.
 */

/*
 * USE_PROCESSES is defined if separate processes are used.  Processes are
 * objects in Unix that have a unique process id.  
 * 
 *  There are separate subclasses of processes.  The simplest is fork:
 *   USE_FORK
 *  Some systems have their own prefered method for creating new processes
 *   USE_CNXFORK (Convex/HP cspp systems)
 *
 * USE_PTHREADS_SX4 is defined is "threads with separate memory" are used
 * Since this relies on NEC SX-4-specific extensions to both pthreads and
 * the compiler, 
 *
 */
#ifndef PROCESS_CREATE_METH
#define USE_PROCESSES
#endif

#define FORK 1
#define PTHREADS_SX4 2

#ifdef USE_PROCESSES
#define PROCESS_CREATE_METH FORK
#endif
#ifdef USE_PTHREADS_SX4
#define PROCESS_CREATE_METH PTHREADS_SX4
#endif

/* 
 * Various data-structures require preallocating storage for process/thread 
 * information.  This size is setup here.
 */
#ifndef MPID_MAX_PROCS
#define MPID_MAX_PROCS 32
#endif

/*
 * Here begins the specific code for each startup method.
 */
#if PROCESS_CREATE_METH == FORK

/* Already defined FORK as 1 */
#define CNXFORK 2

#ifndef PROCESS_FORK 
#define PROCESS_FORK FORK
#endif

/* 
   The create_procs routine keeps track of the processes (stores the
   rc from PROCESS_FORK) and is prepared to kill the children if it
   receives a SIGCHLD.  One problem is making sure that the kill code
   isn't invoked during a normal shutdown.  This is handled by turning
   off the signals while in the rundown part of the code; this introduces
   a race condition in failures that I'm not prepared for yet.

   This is complicated by the decision of POSIX to chose unreliable signals
   as the default signal behavior (probably to ensure the adoption of 
   Windows NT).

   The interface is this:
   To set a reliable signal handler, 
   SIGNAL_HAND_SET(signame,sigf)

   To set a reliable signal handler and get the old handler back
   SIGNAL_HAND_SET_RET(signame,sigf,oldsigf)

   Before exiting a signal handler (needed with unreliable signals to 
   re-establish the handler!)
   SIGNAL_HAND_CLEANUP(signame,sigf)

   Finally, signal handler declarations are a mess.  Some systems even
   require users to declare them in ways that don't match the definition
   or need!  To declare a signal handler, use

   SIGNAL_HAND_DECL(sigf)
   It will use arguments sig [,code, scp] .  Depend ONLY on sig.
 */
#if defined(HAVE_SIGACTION)
/*
 * In the case where SA_RESETHAND is supported (i.e., reliable signals), 
 * we can use that and don't need to reset the handler.  Otherwise, we'll
 * have to.
 */
#if defined(SA_RESETHAND)
/* Here is the most reliable version.  Systems that don't provide
   SA_RESETHAND are basically broken at a deep level. 
 */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldsigf = oldact.sa_handler;\
oldact.sa_handler = sigf;\
oldact.sa_flags   = oldact.sa_flags & ~(SA_RESETHAND);\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_SET(signame,sigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldact.sa_handler = sigf;\
oldact.sa_flags   = oldact.sa_flags & ~(SA_RESETHAND);\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_CLEANUP(signame,sigf)
#else
/* If SA_RESETHAND is not defined, we hope that by masking off the
   signal we're catching that it won't deliver that signal to SIG_DFL
 */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldsigf = oldact.sa_handler;\
oldact.sa_handler = sigf;\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_SET(signame,sigf) {\
struct sigaction oldact;\
sigaction( signame, (struct sigaction *)0, &oldact );\
oldact.sa_handler = sigf;\
sigaddset( &oldact.sa_mask, signame );\
sigaction( signame, &oldact, (struct sigaction *)0 );}

#define SIGNAL_HAND_CLEANUP(signame,sigf) SIGNAL_HAND_SET(signame,sigf)
#endif /* SA_RESETHAND */

#elif defined(HAVE_SIGNAL)
/* Assumes reliable signals. The ';' in the definitions keep the emacs
   indentation code from doing stupid things */
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) \
oldsigf = signal(signame,sigf);
#define SIGNAL_HAND_SET(signame,sigf) \
(void) signal(signame,sigf);
#define SIGNAL_HAND_CLEANUP(signame,sigf)

#elif defined(HAVE_SIGSET)
#define SIGNAL_HAND_SET_RET(signame,sigf,oldsigf) \
oldsigf = sigset(signame,sigf);
#define SIGNAL_HAND_SET(signame,sigf) \
(void) sigset(signame,sigf);
#define SIGNAL_HAND_CLEANUP(signame,sigf) 

#else
/* no signal handlers! */
'Error - no signal handler available'
#endif

/* Signal handler declarations */
#if defined(HAVE_SIGHAND3)
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig, code, scp )\
int sig, code; struct sigcontext *scp;
#elif defined(HAVE_SIGHAND4)
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig, code, scp, addr )\
int sig, code; struct sigcontext *scp;char *addr;
#else
#define SIGNAL_HAND_DECL(sigf) \
RETSIGTYPE sigf( sig )\
int sig;
#endif
/* End of signal handler definitions */

/*
 * Establish a handler for termination signals from the child.
 * Process ids are stored in an array and this array is used to wait on
 * terminating processes.  If a SIGCHLD/SIGCLD is received, the handler
 * does a wait and removes the child from the list.  If the signal is 
 * unexpected (the "join"/finalize routine hasn't been called), then 
 * initiate a termination of the job.
 */
static int MPID_child_pid[MPID_MAX_PROCS];
static int MPID_numprocs = 0;

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
/* Set SIGCHLD handler */
static int MPID_child_status = 0;
#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif
/* Define standard signals if SysV version is loaded */
#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD SIGCLD
#endif

SIGNAL_HAND_DECL(MPID_handle_child)
{
  int prog_stat, pid;
  int i, j;

  /* Really need to block further signals until done ... */
  /* fprintf( stderr, "Got SIGCHLD...\n" ); */
  pid	   = waitpid( (pid_t)(-1), &prog_stat, WNOHANG );
  if (MPID_numprocs && pid && 
      (WIFEXITED(prog_stat) || WIFSIGNALED(prog_stat))) {
#ifdef MPID_DEBUG_ALL
    if (MPID_DebugFlag) printf("Got signal for child %d (exited)... \n", pid );
#endif
    /* The child has stopped. Remove it from the jobs array */
    for (i = 0; i<MPID_numprocs; i++) {
      if (MPID_child_pid[i] == pid) {
	MPID_child_pid[i] = 0;
	if (WIFEXITED(prog_stat)) {
	  MPID_child_status |= WEXITSTATUS(prog_stat);
	}
	else if (WIFSIGNALED(prog_stat)) {
	  /* If we're not exiting, cause an abort. */
	    p2p_error( "Child process died unexpectedly from signal", 
		       WTERMSIG(prog_stat) );
	}
	else
	  p2p_error( "Child process exited unexpectedly", i );
	break;
      }
    }
    /* Child may already have been deleted by Leaf exit; we should
       use conn->state to record it rather than zeroing it */
    /* 
    if (i == MPID_numprocs) {
	fprintf( stderr, "Received signal from unknown child!\n" );
	}
	*/
  }
/* Re-enable signals if necessary */
SIGNAL_HAND_CLEANUP(SIGCHLD,MPID_handle_child);
}

void p2p_clear_signal()
{
  SIGNAL_HAND_SET( SIGCHLD, SIG_IGN );
}

#if PROCESS_FORK == FORK
void p2p_create_procs(numprocs, argc, argv )
int numprocs;
int argc;
char **argv;
{
    int i, rc;

    SIGNAL_HAND_SET( SIGCHLD, MPID_handle_child );
    for (i = 0; i < numprocs; i++)
    {
        /* Clear in case something happens ... */
        MPID_child_pid[i] = 0;
	rc = fork();
	if (rc == -1)
	  {
	    p2p_error("p2p_init: fork failed\n",(-1));
	  }
	else if (rc == 0)
	  {
	    /* Should we close stdin (fd==0)? */
	    return;
	  }
	else {
	  /* Save pid of child so that we can detect child exit */
	  MPID_child_pid[i] = rc;
	  MPID_numprocs     = i+1;
	}
    }
}
#elif PROCESS_FORK == CNXFORK
void p2p_create_procs(numprocs, argc, argv)
int numprocs;
int argc;
char **argv;
{
    int i, rc;

    SIGNAL_HAND_SET( SIGCHLD, MPID_handle_child );
    for (i = 0; i < numprocs; i++)
    {
        /* Clear in case something happens ... */
        MPID_child_pid[i] = 0;
	/*
	 * Skip the master process.
	 */
	rc = (i == masterid) ?
	  getpid() : cnx_sc_fork(CNX_INHERIT_SC, procNode[i]);
	if (rc == -1)
	  {
	    p2p_error("p2p_init: fork failed\n",(-1));
	  }
	else if (rc == 0)
	  {
	    masterid = -1;
	    
	    if (cnx_exec == 0) {
	      if(setpgid(0,MPID_SHMEM_ppid)) {
		p2p_error("p2p_init: failure in setpgid\n",(-1));
	      }
	    }
	    return;
	  }
	else {
	  /* Save pid of child so that we can detect child exit */
	  MPID_child_pid[i] = rc;
	  MPID_numprocs     = i+1;
	}
    }
}
#else
'No fork defined!'
#endif

/* Common code for setting process groups */
/* This routine is a place holder */
void p2p_makesession()
{
/* On some systems (SGI IRIX 6), process exit sometimes kills all processes
   in the process GROUP.  This code attempts to fix that.  
   We DON'T do it if stdin (0) is connected to a terminal, because that
   disconnects the process from the terminal.
 */
#if defined(HAVE_SETSID) && defined(HAVE_ISATTY) && defined(USE_NEW_PGRP)
if (!isatty(0)) {
    pid_t rc;
    rc = setsid();
    if (rc < 0) {
	p4_dprintfl( 90, "Could not create new process group\n" );
	}
    else {
	p4_dprintfl( 80, "Created new process group %d\n", rc );
	}
    }
else {
	p4_dprintfl( 80, 
         "Did not created new process group because isatty returned true\n" );
    }
#endif
}

/* 
   This is a process group, used to help clean things up when a process dies 
   It turns out that this causes strange failures when running a program
   under another program, like a debugger or mpirun.  Until this is
   resolved, I'm ifdef'ing this out.
 */
#if defined(USE_SETPGID)
static int MPID_SHMEM_ppid = 0;
void
p2p_setpgrp()
{
#ifdef FOO
   MPID_SHMEM_ppid = getpid();
   if(setpgid(MPID_SHMEM_ppid,MPID_SHMEM_ppid)) {
       perror("failure in p2p_setpgrp");
       exit(-1);
   }
#endif

#if defined(MPI_cpss)
   if (cnx_exec == 0) {
	MPID_SHMEM_ppid = getpid();
	if(setpgid(MPID_SHMEM_ppid,MPID_SHMEM_ppid)) {
		perror("failure in p2p_setpgrp");
		exit(-1);
	}
   }
#endif
}
#endif /* USE_SETPGID */

/* We can use common code to handle stopping processes */
void p2p_kill_procs()
{
  if (MPID_myid == 0) {
    int i;
    /* We are no longer interested in signals from the children */
    SIGNAL_HAND_SET( SIGCHLD, SIG_IGN );
    /* numprocs - 1 because the parent is not in the list */
    for (i=0; i<MPID_numprocs-1; i++) {
      if (MPID_child_pid[i] > 0) 
	kill( MPID_child_pid[i], SIGINT );
    }
  }

#ifdef FOO
    if (MPID_SHMEM_ppid) 
	kill( -MPID_SHMEM_ppid, SIGKILL );
#endif

#if defined(MPI_cspp)
    if (MPID_SHMEM_ppid && (cnx_exec == 0))
	kill( -MPID_SHMEM_ppid, SIGKILL );
#endif
}

#endif /* PROCESS_CREATE_METH == FORK */

#if PROCESS_CREATE_METH == PTHREAD_SX4
#include <pthread.h>
/*
 * --- create processes ---
 * We create the processes by calling a routine that starts main over again.
 * This is ok for MPI, since we don't say what is running (or when!) before
 * MPI_Init
 */
typedef struct { int argc; char **argv; } MPID_startarg;

void *MPID_startup( aptr )
void *aptr;
{
  static int rval;
  MPID_startarg *a = (MPID_startarg*)aptr;
  rval = main( a->argc, a->argv );
  return (void *)&rval;
}

/* 
 * It is someone else's responsibility NOT to call this again.
 * This RE-CALLS the user's MAIN program.
 */
pthread_t thread[MPID_MAX_PROCS];
#pragma _pthread shared_begin
int MPID_IsReady = 0;              /* Used to make sure only master thread
				      calls thread create */
int MPID_Globid = 0;               /* Used to enumerate threads */
pthread_mutex_t MPID_mutex;        /* Provides a common lock */
void *MPID_shared_memory = 0;      /* Provides for a common shared memory 
				      area */
#pragma _pthread shared_end
void p2p_create_procs( numprocs, argc, argv )
int  numprocs;
int  argc;
char **argv;
{
  MPID_startarg args;
  int i, rc;

  if (MPID_IsReady) return;
  MPID_IsReady = 1;
  args.argc = argc;
  args.argv = argv;
  for (i=0; i<numprocs; i++) {
    /* pthread_attr_default */
    rc = pthread_create( &thread[i], (pthread_attr_t *) 0, 
                         MPID_startup, &args );
    if (rc != 0) {
      p2p_error( "p2p_init: thread-fork failed\n", (-1));
    }
  }
}
/* 
 * --- cleanup --
 * We can't wait for the threads to return from MPID_startup, because this
 * means returning from main (which may be loooong after MPI_Finalize).
 * Instead, we make the threads call pthread_exit and the master checks the
 * return codes for problems.
 */
void p2p_cleanup()
{
  int status, i;

  if (MPID_MyWorldRank != 0) {
    status = 0;
    pthread_exit( &status );
  }
  else {
    for (i=0; i<MPID_MyWorldSize-1; i++) {
      pthread_join( thread[i], (void **)&status );
      /* Status is the return code from pthread_exit */
    }
  }
  
}
void p2p_kill_procs()
{
  if (not master) 
    ;
  else {
    /* Cancel/stop the threads */
  }
}
#endif /* PROCESS_CREATE_METH == PTHREAD_SX4 */

