#include "p4.h"
#include "p4_sys.h"

typedef long P4_Aint;

extern P4VOID exit();

static int interrupt_caught = 0; /* True if an interrupt was caught */

#if defined(ENCORE) || defined(SYMMETRY) || defined(TITAN) || \
    defined(SGI)    || defined(GP_1000)  || defined(TC_2000) || defined(SUN_SOLARIS)
#define P4_HANDLER_TYPE int
#else
#define P4_HANDLER_TYPE P4VOID
#endif

static P4_HANDLER_TYPE (*prev_sigint_handler) () = NULL;
static P4_HANDLER_TYPE (*prev_sigsegv_handler) () = NULL;
static P4_HANDLER_TYPE (*prev_sigbus_handler) () = NULL;
static P4_HANDLER_TYPE (*prev_sigfpe_handler) () = NULL;
static P4_HANDLER_TYPE (*prev_err_handler) () = NULL;
static int err_sig, err_code;
static struct sigcontext *err_scp;
static char *err_addr;


int p4_soft_errors(onoff)
int onoff;
{
    int old;

    if (!p4_local)
	p4_error("p4_soft_errors: p4_local must be allocated first", 0);

    old = p4_local->soft_errors;
    p4_local->soft_errors = onoff;
    return old;
}

P4VOID p4_error(string, value)
char *string;
int value;
{
    char job_filename[64];

    SIGNAL_P4(SIGINT,SIG_IGN);
    fflush(stdout);
    printf("%s:  p4_error: %s: %d\n",whoami_p4,string,value);
    if (value < 0)
        perror("    p4_error: latest msg from perror");
    fflush(stdout);

    /* Send interrupt to all known processes */
    zap_p4_processes();

    /* shutdown(sock,2), close(sock) all sockets */
#   ifdef CAN_DO_SOCKET_MSGS
    shutdown_p4_socks();
#   endif

#   ifdef SYSV_IPC
    remove_sysv_ipc();
#   endif

#   if defined(SGI)  &&  defined(VENDOR_IPC)
    unlink(p4_sgi_shared_arena_filename);
#   endif

    if (execer_starting_remotes  &&  execer_mynodenum == 0)
    {
	strcpy(job_filename,"/tmp/p4_");
	strcat(job_filename,execer_jobname);
	unlink(job_filename);
    }

    if (interrupt_caught && value != SIGINT)
    {
	switch (value)
	{
	  case SIGSEGV:
	    prev_err_handler = prev_sigsegv_handler;
	    break;
	  case SIGBUS:
	    prev_err_handler = prev_sigbus_handler;
	    break;
	  case SIGFPE:
	    prev_err_handler = prev_sigfpe_handler;
	    break;
	  default:
	    printf("p4_error: unidentified err handler\n");
	    prev_err_handler = NULL;
	    break;
	}
	if (prev_err_handler == (P4_HANDLER_TYPE (*) ()) NULL)
	{
	    /* return to default handling of the interrupt by the OS */
	    SIGNAL_P4(value,SIG_DFL); 
#           if defined(NEXT)  ||  defined(KSR)
            kill(getpid(),value);
#           endif
	    return;
	}
	else
	{
	    (*prev_err_handler) (err_sig, err_code, err_scp, err_addr);
	}
    }
    else
    {

#       if defined(SP1_EUI)
	mpc_stopall(value);
#       endif
	exit(1);
    }
}

/* static P4_HANDLER_TYPE sig_err_handler(sig, code, scp, addr) */

static P4VOID sig_err_handler(sig, code, scp, addr)
int sig, code;
struct sigcontext *scp;
char *addr;
{
    interrupt_caught = 1;
    err_sig = sig;
    err_code = code;
    err_scp = scp;
    err_addr = addr;
    if (sig == 11)
	p4_error("interrupt SIGSEGV", sig);
    else if (sig == 10)
	p4_error("interrupt SIGBUS", sig);
    else if (sig == 8)
	p4_error("interrupt SIGFPE", sig);
    else if (sig == 2)
	p4_error("interrupt SIGINT", sig);
    else
	p4_error("interrupt SIGx", sig);
    /* return( (P4_HANDLER_TYPE) NULL); */
}


/*
  Trap signals so that we can propagate error conditions and tidy up 
  shared system resources in a manner not possible just by killing procs
*/
P4VOID trap_sig_errs()
{
    P4_HANDLER_TYPE (*rc) ();

    rc = (P4_HANDLER_TYPE (*) ()) SIGNAL_P4(SIGINT, sig_err_handler);
    if (rc == (P4_HANDLER_TYPE (*) ()) -1)
	p4_error("trap_sig_errs: SIGNAL_P4 failed", SIGINT);
    if (((P4_Aint) rc > 1)  &&  ((P4_Aint) rc != (P4_Aint) sig_err_handler))
	prev_sigint_handler = rc;

/* we can not handle sigsegv on symmetry and balance because they use 
 * it for shmem stuff 
*/
#ifdef CAN_HANDLE_SIGSEGV
    rc = (P4_HANDLER_TYPE (*) ()) SIGNAL_P4(SIGSEGV, sig_err_handler);
    if ((P4_Aint) rc == -1)
	p4_error("trap_sig_errs: SIGNAL_P4 failed", SIGSEGV);
    if (((P4_Aint) rc > 1)  &&  ((P4_Aint) rc != (P4_Aint) sig_err_handler))
	prev_sigsegv_handler = rc;
#endif

    rc = (P4_HANDLER_TYPE (*) ()) SIGNAL_P4(SIGBUS, sig_err_handler);
    if ((P4_Aint) rc == -1)
	p4_error("trap_sig_errs: SIGNAL_P4 failed", SIGBUS);
    if (((P4_Aint) rc > 1)  &&  ((P4_Aint) rc != (P4_Aint) sig_err_handler))
	prev_sigbus_handler = rc;
    rc = (P4_HANDLER_TYPE (*) ()) SIGNAL_P4(SIGFPE, sig_err_handler);
    if ((P4_Aint) rc == -1)
	p4_error("trap_sig_errs: SIGNAL_P4 failed", SIGFPE);
    if (((P4_Aint) rc > 1)  &&  ((P4_Aint) rc != (P4_Aint) sig_err_handler))
	prev_sigfpe_handler = rc;
}
