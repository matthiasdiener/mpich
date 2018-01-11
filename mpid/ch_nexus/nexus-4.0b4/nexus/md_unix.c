/*
 * md_unix.c
 */

static char *rcsid = "/net/bcomp/src/master/nexus/md_unix.c,v 1.51 1995/09/15 16:36:57 tuecke Exp";

#include "internal.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include <sys/signal.h>
#include <sys/time.h>

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef TARGET_ARCH_PARAGON
#define NEXUS_IGNORE_CHILDREN
void _nx_clean_quit (int);
#endif

#ifdef JGG
/* I have left this in since I have not tested HAVE_POSIX_SIGNALS on
 * all these platforms yet.
 *
 *  I do know that it compiles correctly on:
 *
 *    SGI
 *    FSU_PTHREADS
 *    AIX 3.2.5
 */
#if defined(TARGET_ARCH_SGI) || defined(HAVE_PTHREAD_DRAFT_6) \
    || defined(TARGET_ARCH_HPUX) || defined(TARGET_ARCH_AIX4) \
    || defined(TARGET_ARCH_CRAYC90)
#define USE_POSIX_SIGNALS
#endif
#else /* JGG */
#ifdef HAVE_POSIX_SIGNALS
#define USE_POSIX_SIGNALS
#endif
#endif /* JGG */

/* HAVE_WAITPID is preferable, since it is POSIX compliant */
#if !defined(HAVE_WAITPID) && !defined(HAVE_WAIT3)
#error Need either HAVE_WAITPID or HAVE_WAIT3
#endif

/* NEXUS_HAS_SIGHOLD is preferable, since it is SVID2 compliant */
#if !defined(HAVE_SIGHOLD) && !defined(HAVE_SIGBLOCK) \
    && !defined(USE_POSIX_SIGNALS)
#error Need either HAVE_SIGHOLD or HAVE_SIGBLOCK
#endif

#ifdef BUILD_LITE
#define NEXUS_IGNORE_CHILDREN
#endif

/*
 * Command line arguments...
 */
#ifndef BUILD_LITE
static nexus_bool_t	trapping_sigtrap;
static nexus_bool_t	trapping_sigfpe;
static nexus_bool_t	no_trapping;
#endif /* BUILD_LITE */

#ifndef NEXUS_IGNORE_CHILDREN    
static void child_death(int s);
static int  n_live_children = 0;
#ifdef USE_POSIX_SIGNALS
static sigset_t set, emptyset;
#endif /* USE_POSIX_SIGNALS */
#endif /* NEXUS_IGNORE_CHILDREN */


#ifndef BUILD_LITE
static void abnormal_death(int s);
static void interrupt(int s);
static void terminate(int s);
#endif /* BUILD_LITE */


/*
 * _nx_md_usage_message()
 */
void _nx_md_usage_message(void)
{
#ifndef BUILD_LITE
    printf("    -no_catching              : Do not catch any signals\n");
    printf("    -catch_sigtrap            : Catch (and terminate on) the SIGTRAP signal\n");
    printf("    -catch_fpe                : Catch (and terminate on) floating point\n");
    printf("                                exceptions\n");
#endif /* BUILD_LITE */
    
} /* _nx_md_usage_message() */


/*
 * _nx_md_new_process_params()
 */
int _nx_md_new_process_params(char *buf, int size)
{
    char tmp_buf1[1024];
    int n_added;
    
    tmp_buf1[0] = '\0';
    
    nexus_stdio_lock();

#ifndef BUILD_LITE
    if (trapping_sigtrap)
	strcat(tmp_buf1, "-catch_sigtrap ");
    if (trapping_sigfpe)
	strcat(tmp_buf1, "-catch_fpe ");
    if (no_trapping)
	strcat(tmp_buf1, "-no_catching ");
#endif /* BUILD_LITE */
    
    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
	nexus_fatal("_nx_md_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    
    strcpy(buf, tmp_buf1);

    nexus_stdio_unlock();
    
    return (n_added);
    
} /* _nx_md_new_process_params() */


/*
 * _nx_md_init()
 *
 * Initialize any machine dependent stuff
 */
void _nx_md_init(int *argc, char ***argv)
{
#ifndef BUILD_LITE
    int arg_num;
#endif

    /*
     * Parse any arguments
     */
#ifndef BUILD_LITE
    no_trapping = NEXUS_FALSE;
    trapping_sigtrap = NEXUS_FALSE;
    trapping_sigfpe = NEXUS_FALSE;
    if ((arg_num = nexus_find_argument(argc, argv, "catch_sigtrap", 1)) >= 0)
    {
#ifndef USE_POSIX_SIGNALS
	signal(SIGTRAP, abnormal_death);
#else
	struct sigaction act;

	act.sa_handler = abnormal_death;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGTRAP, &act, NULL);
#endif /* USE_POSIX_SIGNALS */
	trapping_sigtrap = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "catch_fpe", 1)) >= 0)
    {
#ifndef USE_POSIX_SIGNALS
	signal(SIGFPE, abnormal_death);
#else
	struct sigaction act;

	act.sa_handler = abnormal_death;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGFPE, &act, NULL);
#endif /* USE_POSIX_SIGNALS */
	trapping_sigfpe = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "no_catching", 1)) >= 0)
    {
	no_trapping = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
#endif /* BUILD_LITE */

    
#ifdef TARGET_ARCH_PARAGON
    signal (SIGUSR1, _nx_clean_quit);
#endif 

#ifndef NEXUS_IGNORE_CHILDREN
#if   defined(TARGET_ARCH_SOLARIS)
    signal(SIGCHLD, SIG_IGN);
#elif defined(USE_POSIX_SIGNALS)
    {
	struct sigaction act;

	sigemptyset(&set);
	sigemptyset(&emptyset);

	act.sa_handler = child_death;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGCHLD);
	act.sa_flags = 0;

	sigaction(SIGCHLD, &act, NULL);
	sigaddset(&set, SIGCHLD);
    }
#else
    signal(SIGCHLD, child_death);
#endif    
#endif /* NEXUS_IGNORE_CHILDREN */
    
#ifndef USE_POSIX_SIGNALS
    
    signal(SIGPIPE, SIG_IGN);
#ifndef BUILD_LITE
    signal(SIGINT, interrupt);
    signal(SIGTERM, terminate);
    if (!no_trapping)
    {
	signal(SIGBUS, abnormal_death);
#ifndef NEXUS_DONT_CATCH_SIGSEGV	
	signal(SIGSEGV, abnormal_death);
#endif
	signal(SIGABRT, abnormal_death);
	signal(SIGILL, abnormal_death);
    }
#endif /* BUILD_LITE */
    
#else /* USE_POSIX_SIGNALS */
    {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &act, NULL);
#ifndef BUILD_LITE
	act.sa_handler = interrupt;
	sigaction(SIGINT, &act, NULL);
	act.sa_handler = terminate;
	sigaction(SIGTERM, &act, NULL);
	if (!no_trapping)
	{
		act.sa_handler = abnormal_death;
		sigaction(SIGBUS, &act, NULL);
#ifndef NEXUS_DONT_CATCH_SIGSEGV
		sigaction(SIGSEGV, &act, NULL);
#endif
		sigaction(SIGABRT, &act, NULL);
		sigaction(SIGILL, &act, NULL);
	}
#endif /* BUILD_LITE */
    }
#endif /* USE_POSIX_SIGNALS */
    
} /* _nx_md_init() */


/*
 * _nx_md_shutdown()
 *
 * Shutdown any machine dependent stuff.
 */
void _nx_md_shutdown(void)
{
} /* _nx_md_shutdown() */


/*
 * _nx_md_abort()
 */
void _nx_md_abort(int return_code)
{
    exit(return_code);
} /* _nx_md_abort() */


/*
 * _nx_md_exit()
 */
void _nx_md_exit(int return_code)
{
    _nx_md_wait_for_children();
    exit(return_code);
} /* _nx_md_exit() */


/*
 * _nx_md_get_command_line_args()
 *
 * Retrieve the command line arguments and fill them into 'argv',
 * starting at 'starting_argc', up to a maximum of 'max_argc'.
 * Return a new argc.  The char* pointers of argv[] will be set
 * to strings in malloced memory.
 *
 * There is a chicken and egg problem.  With CC++, the global
 * constructors run before main() is called with argc/argv.
 * Thus, Nexus needs to be setup by the first global constructor,
 * and that Nexus setup may need command line arguments to configure
 * itself properly.  However, since main() as not yet been called
 * we do not yet have argc/argv.
 *
 * Fortunately, most versions of Unix store the command line
 * arguments in the negative environ[] locations, so they can
 * actually be retrieved before main() is even called.  This is
 * hack used here.
 *
 * If argc/argv are not retrievable prior to main() being called,
 * then the user must resort to using the package environment
 * variable to set configuration options.
 *
 * Note: If need be, _nx_split_args() can be called to split
 * a string into its component arguments.
 *
 * Return: A new argc, which is the element in argv immediately
 *		after the last element that we put into it.
 */
int _nx_md_get_command_line_args(int starting_argc,
				 int max_argc,
				 char **argv)
{

/* NOTE PARAGON DOES NOT SUPPORT THIS */


#if defined(TARGET_ARCH_CRAYC90)
    
    /*
     * On the Cray,  the arguments are stored in reverse order
     * in environ[] from -2 to -base_arg, where environ[-(base_arg-1)]
     * is the program name.
     *
     * So structure of environ (for most machines) seems to be:
     *
     * -(argc+4)      -(argc+1)              -3       -2       -1       0
     *     argc |.|.| argv[argc-1] | ... | argv[1] | argv[0] | NULL | envars
     *             :
     *         -base_arg
     *
     * base_arg can be found by scanning back through environ[]
     * until (environ[-(base_arg+2)]==base_arg-2) (that is, until
     * we find argc in environ[]).
     */
    
    int base_arg;
    int argc, current_argc;
    int i, arg;
    extern char **environ;

    /*
     * Find the first argument in environ[] and initialize
     * base_arg with that value.
     */
    for (base_arg = 2; 
	 (int) environ[-(base_arg+2)] != base_arg-2 && 
	 environ[-base_arg] != (char *) 0; 
	 base_arg++
	 )
	;
    argc = base_arg - 2;

    if (starting_argc + argc > max_argc)
    {
	printf("_nx_md_get_command_line_args(): Error: Too many command line arguments\n");
	_nx_md_exit(1);
    }

    current_argc = starting_argc;
    for (i = 0; i < argc; i++)
    {
	arg = base_arg - i - 1;
	argv[current_argc++] = _nx_copy_string(environ[-arg]);
    }

    return (current_argc);
    
#elif 0 /* defined(TARGET_ARCH_PARAGON) */
	/* Well hack doesn't work on the Paragon so need to use
	Environment variable */
	int new_argc;
	int current_argc = starting_argc;
	char* nexus_args_str = getenv(NEXUS_ARGS);
	char** env;
	extern char** environ;

	env = environ;
#if defined (BUILD_DEBUG) && defined (TAL)
	if (NexusDebug (2)) {
	if (mynode() > 0) {
	printf ("In _nx_md_get_command_line_args() Environ: .\n");
	while (*env != NULL) {
		nexus_printf ("%s\n", *env++);
	}
	}
	nexus_printf ("In _nx_md_get_command_line_args() -- PGON.\n");
	nexus_printf ("nexus_args_str: %s\n", nexus_args_str);
	}
#endif
	if (nexus_args_str) {
		if ((new_argc =
		 _nx_split_args (nexus_args_str, starting_argc, 
		 max_argc, argv, NULL)) < 0) {
			nexus_printf (
			 "Error while reading args from Environment %s\n", NEXUS_ARGS);
			_nx_md_exit(1);
		}
		current_argc = new_argc;
	}
	else {
		nexus_printf (
		 "Nexus on Paragon requires placing runtime flags in Environment %s\n",
		 NEXUS_ARGS);
		_nx_md_exit(1);
	}

#if defined (BUILD_DEBUG) && defined (TAL)
	if (NexusDebug (2)) {
	nexus_printf ("In _nx_md_get_command_line_args().\n");
	nexus_printf ("%d current_argc\n", current_argc);
	for (new_argc = 0; new_argc < current_argc; new_argc++) 
		nexus_printf ("--argv[%d]: %s\n", new_argc, argv[new_argc]);
	}
#endif
	return (current_argc);

#else  /* TARGET_ARCH_CRAYC90 */
    
    /*
     * The structure of environ (for most machines) seems to be:
     *
     * -(argc+2)  -(argc+1)  -argc                -2       -1       0
     *     argc | argv[0] | argv[1] | ... | argv[argc-1] | NULL | envars
     * -base_arg
     * 
     * The RS/6000 puts a NULL in environ[-(argc+2)] instead of argc.
     *
     * This has been tested on at least the following machines:
     *   Sun (Sparc) SunOS 4.x, Sun (Sparc) Solarix 2.x,
     *   IBM RS/6000 AIX, SGI Indigo, Sequent Symmetry Dynix,
     *   Intel iPSC/860 SRM (386 running SysV 3.2),
     *   NEXTSTEP (486 and Motorola), PC running SysVr4
     */

#ifdef TARGET_ARCH_PARAGON    
#define ArgStart 2
    int null_pos = ArgStart;
#endif    
    int base_arg;
    int argc, current_argc;
    int i, arg;
    extern char **environ;

    /*
     * Find the first argument in environ[] and initialize
     * base_arg with that value.
     */
#ifndef TARGET_ARCH_PARAGON
    
    for (base_arg = 2; 
	 (long) environ[-(base_arg)] != base_arg-2 && 
	 environ[-base_arg] != (char *) 0; 
	 base_arg++
	 )
	;
    argc = base_arg - 2;
    if (starting_argc + argc > max_argc)
    {
	printf("_nx_md_get_command_line_args(): Error: Too many command line arguments\n");
	_nx_md_exit(1);
    }

    current_argc = starting_argc;
    for (i = 0; i < argc; i++)
    {
	arg = base_arg - i - 1;
	argv[current_argc++] = _nx_copy_string(environ[-arg]);
    }
    
#else /* TARGET_ARCH_PARAGON */
    
    for (base_arg = null_pos;
	 (int) environ [-base_arg] != base_arg-ArgStart;
	 base_arg++)
    {
	if (environ [-base_arg] == NULL)
	    null_pos = base_arg;
    }
	
    argc = base_arg - null_pos -1;
    nexus_debug_printf(2,("argc %d, base_arg %d, null_pos %d.\n", argc, base_arg, null_pos));

    if (starting_argc + argc > max_argc)
    {
	printf("_nx_md_get_command_line_args(): Error: Too many command line arguments\n");
	_nx_md_exit(1);
    }

    current_argc = starting_argc;
    for (i = 0, base_arg--; i < argc; i++, base_arg--)
    {
	/*
	arg = base_arg - i - 1;
	*/
	/*
	printf("(%ld,%ld) current_argc %d env[-%d] %s\n", mynode(), myptype(),
	       current_argc, base_arg, environ[-base_arg]);
	*/
	argv[current_argc++] = _nx_copy_string(environ[-base_arg]);
    }
    
#endif /* TARGET_ARCH_PARAGON */


    return (current_argc);
    
#endif /* TARGET_ARCH_CRAYC90 */
    
} /* _nx_md_get_command_line_args() */


/*
 * _nx_md_system_error_string()
 *
 * Return the string for the current errno.
 */
char *_nx_md_system_error_string(int the_error)
{
    extern char *sys_errlist[];
    return (sys_errlist[the_error]);
} /* _nx_md_system_error_string() */


/*
 * _nx_md_get_unique_session_string()
 *
 * Return a malloced string containing a unique string.
 * This string should be unique for all time, not just within
 * this process but across all process on all machines.
 * This string is composed of my hostname, process id, and the
 * current time (seconds since 1970).
 */
char *_nx_md_get_unique_session_string()
{
    char hostname[MAXHOSTNAMELEN];
    char tmp_buf[MAXHOSTNAMELEN + 32];
    _nx_md_gethostname(hostname, MAXHOSTNAMELEN);
    nexus_stdio_lock();
    sprintf(tmp_buf, "%s_%lx_%lx",
	    hostname,
	    (unsigned long) _nx_md_getpid(),
	    (unsigned long) time(0));
    nexus_stdio_unlock();
    return(_nx_copy_string(tmp_buf));
} /* _nx_md_get_unique_session_string() */

    
/*
 * _nx_md_gethostname()
 *
 * Return the hostname that this process is running on.
 * Do any error checking.
 *
 * Lookup the "domain" for this hostname in the resource database.
 * If it is present, then add it to this hostname if the hostname
 * doesn't already end in that domain.
 */
void _nx_md_gethostname(char *name, int len)
{
    static char hostname[MAXHOSTNAMELEN];
    static int hostname_length = -1;
    
    if (hostname_length == -1)
    {
	char *domain;
	int domain_length;
	
	if (gethostname(hostname, MAXHOSTNAMELEN) < 0)
	{
	    nexus_fatal("_nx_md_gethostname(): gethostname() failed\n");
	}
	hostname_length = strlen(hostname);

	domain = nexus_rdb_lookup(hostname, "domain");
	if (domain)
	{
	    domain_length = strlen(domain);
	    if (   (domain_length >= hostname_length)
		|| (strcmp(&(hostname[hostname_length-domain_length]),
			   domain) != 0) )
	    {
		if ((hostname_length + domain_length) >= MAXHOSTNAMELEN)
		{
		    nexus_fatal("_nx_md_gethostname(): hostname+domain is too long\n");
		}
		strcat(hostname, domain);
		hostname_length += domain_length;
	    }
	    nexus_rdb_free(domain);
	}
	
    }

    if (hostname_length < len)
    {
	strcpy(name, hostname);
    }
    else
    {
	nexus_fatal("_nx_md_gethostname(): hostname too long\n");
    }
} /* _nx_md_gethostname() */


/*
 * _nx_md_getpid()
 */
int _nx_md_getpid(void)
{
    static int pid = -1;
    if (pid == -1)
    {
	pid = (int) getpid();
    }
    return(pid);
} /* _nx_md_getpid() */


#ifndef NEXUS_IGNORE_CHILDREN    
#ifndef TARGET_ARCH_SOLARIS
/*
 * child_death()
 *
 * Signal handler for SIGCHLD
 *
 * Do not call nexus_printf() from this signal handler, as
 * it could cause deadlock on the stdio lock.
 */

static RETSIGTYPE child_death(int s)
{
    int pid;
#if defined(HAVE_SYS_WAIT_H) || defined(HAVE_WAITPID)
    int status;
#else
    union wait status;
#endif

#ifdef HAVE_WAITPID
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
#else
    while ((pid = wait3(&status, WNOHANG, NULL)) > 0)
#endif
    {
	/*
        int signal, retcode;
#ifdef NEXUS_HAS_WAIT_STATUS_MACROS
	signal = WTERMSIG(status);
	retcode = WEXITSTATUS(status);
#else
	signal = status.w_termsig;
	retcode = status.w_retcode;
#endif
        */
	n_live_children--;
    }
} /* child_death() */
#endif /* TARGET_ARCH_SOLARIS */
#endif /* NEXUS_IGNORE_CHILDREN */


/*
 * _nx_md_fork()
 *
 * Fork a child process with just one thread.
 * Register this child so that we can wait for it later.
 */
int _nx_md_fork(void)
{
    int child;

#ifndef NEXUS_IGNORE_CHILDREN
#ifndef TARGET_ARCH_SOLARIS
#ifndef USE_POSIX_SIGNALS
#ifdef HAVE_SIGHOLD
    sighold(SIGCHLD);
#else
    int mask = sigblock(sigmask(SIGCHLD));
#endif
#else /* USE_POSIX_SIGNALS */
    sigprocmask(SIG_BLOCK, &set, NULL);
#endif  /* USE_POSIX_SIGNALS */
#endif  /* TARGET_ARCH_SOLARIS */
#endif  /* NEXUS_IGNORE_CHILDREN */

#ifndef BUILD_LITE
    _nx_thread_prefork();
#endif /* BUILD_LITE */
    
#ifdef HAVE_FORK1
    child = fork1();
#else
    child = fork();
#endif
    
#ifndef BUILD_LITE
    if (child > 0)
    {
	_nx_thread_postfork();
    }
#endif /* BUILD_LITE */
    
#ifndef NEXUS_IGNORE_CHILDREN

#ifdef NEXUS_ARCH_AEROSPACE_CSRD
#ifndef BUILD_LITE
    /* hack -- turn off vtalrm */
    if ( child == 0 )
    {
	signal(SIGVTALRM, SIG_IGN);
    }
#endif /* BUILD_LITE */
#endif /* NEXUS_ARCH_AEROSPACE_CSRD */

#ifndef TARGET_ARCH_SOLARIS
    if(child == 0)
    {
	n_live_children = 0;
    }
    else
    {
	n_live_children++;
    }
#ifndef USE_POSIX_SIGNALS
#ifdef HAVE_SIGHOLD
    sigrelse(SIGCHLD);
#else
    sigsetmask(mask);
#endif /* HAVE_SIGHOLD */
#else /* USE_POSIX_SIGNALS */
    sigprocmask(SIG_UNBLOCK, &set, NULL);
#endif /* USE_POSIX_SIGNALS */
#endif /* TARGET_ARCH_SOLARIS */
#endif /* NEXUS_IGNORE_CHILDREN */

    return(child);
} /* _nx_md_fork() */

/*
 * _nx_md_block_sigchld()
 *
 * Block sigchld.  Use _nx_md_deliver_sigchld() to
 * restore the signal state.
 */
int _nx_md_block_sigchld(void)
{
    int mask = 0;
#ifndef NEXUS_IGNORE_CHILDREN
#ifndef TARGET_ARCH_SOLARIS
    if (n_live_children > 0)
    {
#ifndef USE_POSIX_SIGNALS
#ifdef HAVE_SIGHOLD
        sighold(SIGCHLD);
#else
        mask = sigblock(sigmask(SIGCHLD));
#endif
#else /* USE_POSIX_SIGNALS */
	mask = sigprocmask(SIG_BLOCK, &set, NULL);
#endif /* USE_POSIX_SIGNALS */
    }
#endif  /* TARGET_ARCH_SOLARIS */
#endif  /* NEXUS_IGNORE_CHILDREN */

    return mask;
} /* _nx_md_block_sigchld() */

    
void _nx_md_deliver_sigchld(int mask)
{
#ifndef NEXUS_IGNORE_CHILDREN
#ifndef TARGET_ARCH_SOLARIS
    if (n_live_children > 0)
    {
#ifndef USE_POSIX_SIGNALS
#ifdef HAVE_SIGHOLD
        sigrelse(SIGCHLD);
#else
        sigsetmask(mask);
#endif /* HAVE_SIGHOLD */
#else /* USE_POSIX_SIGNALS */
	sigprocmask(SIG_UNBLOCK, &set, NULL);
#endif /* USE_POSIX_SIGNALS */
    }
#endif /* TARGET_ARCH_SOLARIS */
#endif /* NEXUS_IGNORE_CHILDREN */

} /* _nx_md_deliver_sigchld() */

/*
 * _nx_md_wait_for_children()
 *
 * Wait for all of my child processes to terminate
 */
RETSIGTYPE _nx_md_wait_for_children(void)
{
#ifndef NEXUS_IGNORE_CHILDREN
    int pid;
#if defined(HAVE_SYS_WAIT_H) || defined(USE_POSIX_SIGNALS)
    int status;
#else
    union wait status;
#endif


#if !defined(TARGET_ARCH_SOLARIS) && !defined(USE_POSIX_SIGNALS)
    /*
    signal(SIGCHLD, SIG_IGN);
    */
    _nx_md_block_sigchld();
    while (n_live_children > 0)
    {
	if ((pid = wait(&status)) > 0)
	{
	    n_live_children--;
	}
    }
#elif defined(USE_POSIX_SIGNALS)
    /*
    sigprocmask(SIG_BLOCK, &set, NULL);
    */
    _nx_md_block_sigchld();
    while(n_live_children > 0)
    {
	if (waitpid(-1, &status, WNOHANG) > 0)
	{
	    n_live_children--;
	}
    }
#else
    {
	struct sigaction act;
	int flagresult;

	sigaction( SIGCHLD, NULL, &act );

	flagresult = act.sa_flags & SA_NOCLDWAIT ;
	
	if( flagresult != 0 ) {
	    if ( ((pid = wait(&status)) == -1 ) ) {
		n_live_children=0;
	    }
	} else {
	    while (n_live_children > 0) {
		if ((pid = wait(&status)) > 0) {
		    n_live_children--;
		}
	    }
	}
    }
#endif
#endif
} /* _nx_md_wait_for_children() */


#ifndef BUILD_LITE

/*
 * abnormal_death()
 *
 * Signal handler for various signals that should kill the process
 */
static RETSIGTYPE abnormal_death(int s)
{
#ifdef TARGET_ARCH_AIX
    _nx_traceback();
#endif
    nexus_fatal("Got Signal %d\n", s);
} /* abnormal_death() */


/*
 * terminate()
 *
 * Signal handler for SIGTERM, sent by nexus_kill()
 */
static RETSIGTYPE terminate(int s)
{
    nexus_silent_fatal();
} /* terminate() */


/*
 * interrupt()
 *
 * Signal handler for interrupt.
 */
static RETSIGTYPE interrupt(int s)
{
    nexus_printf("Exiting due to interrupt (signal %d)\n", s);
    _nx_nodelock_cleanup();
    _nx_md_exit(0);
}

#endif /* BUILD_LITE */

#ifdef TARGET_ARCH_PARAGON
/* 
 * _nx_clean_quit ()
 *
 * Signal handler for User defined interrupt.
 */
RETSIGTYPE _nx_clean_quit (int s)
{
	/* May want to have a test to see which interrrupt was used in
	case this function is shared. */
    nexus_debug_printf(3, ("Exiting due to interrupt (signal %d)\n", s));
    _nx_nodelock_cleanup();
    _nx_md_exit(0);
}
#endif /* PARAGON */


/*
 * nexus_kill()
 *
 * Kill the named process.
 */
void nexus_kill(int pid)
{
    kill(pid, SIGTERM);
} /* nexus_kill() */


/*
 * nexus_usleep()
 *
 * Sleep for usleep microseconds.
 */
void nexus_usleep(long usec)
{
    struct timeval timeout;
    
    timeout.tv_sec = usec/1000000;
    timeout.tv_usec = usec%1000000;
    
    select(0, NULL, NULL, NULL, &timeout);
} /* nexus_usleep() */
     

/*
 * nexus_wallclock()
 *
 * Return the current wallclock time in seconds.
 */
double nexus_wallclock(void)
{
    struct timeval tv;

    gettimeofday(&tv, 0);
    return (((double) tv.tv_sec) + ((double) tv.tv_usec) / 1000000.0);
} /* nexus_wallclock() */


/*
 * nexus_pause()
 */
void nexus_pause(void)
{
    nexus_printf("Process %d pausing...\n", _nx_md_getpid());
    pause();
} /* nexus_pause() */
