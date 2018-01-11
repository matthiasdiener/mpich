#include "p4.h"
#include "p4_sys.h"

int create_remote_processes(pg)
struct p4_procgroup *pg;
{
    struct p4_procgroup_entry *pe;
    struct net_initial_handshake hs;
    int i, serv_port, serv_fd, rm_fd, rm_fds[P4_MAXPROCS], rm_num;

    net_setup_anon_listener(5, &serv_port, &serv_fd);
    if (execer_starting_remotes)
    {
	put_execer_port(serv_port);
	for (i=1, pe = pg->entries+1; i < pg->num_entries; i++, pe++)
	{
	    rm_fd = net_accept(serv_fd);
	    hs.pid = (int) htonl(getpid());
	    net_send(rm_fd, &hs, sizeof(hs), FALSE);
	    net_recv(rm_fd, &hs, sizeof(hs));
	    rm_num = (int) ntohl(hs.rm_num);
	    rm_fds[rm_num] = rm_fd;
	}
	for (i=1, pe = pg->entries+1; i < pg->num_entries; i++, pe++)
	{
	    pe = pg->entries+i;
	    net_slave_info(pe, rm_outfile_head, rm_fds[i], i);
	}
    }
    else
    {
	for (i=1, pe = pg->entries+1; i < pg->num_entries; i++, pe++)
	{
	    rm_fd = net_create_slave(serv_port,serv_fd,
				     pe->host_name,
				     pe->slave_full_pathname,
				     pe->username);
	    net_slave_info(pe, rm_outfile_head, rm_fd, i);
	}
    }

    return (0);
}

P4VOID net_slave_info(pe, outfile, rm_fd, rm_num)
struct p4_procgroup_entry *pe;
char *outfile;
int rm_fd, rm_num;
{
    struct bm_rm_msg msg;
    P4BOOL done;
    int type, status, port, remote_switch_port;
    int slave_idx, slave_pid, pidx, rm_ind;

    msg.type = p4_i_to_n(INITIAL_INFO);
    msg.numinproctab = p4_i_to_n(p4_global->num_in_proctable);
    msg.numslaves = p4_i_to_n(pe->numslaves_in_group);
    strcpy(msg.outfile, outfile);
    msg.debug_level = p4_i_to_n(remote_debug_level);
    msg.memsize = p4_i_to_n(globmemsize);
    msg.logging_flag = p4_i_to_n(logging_flag);
    strcpy(msg.application_id, p4_global->application_id);
    strcpy(msg.version, P4_PATCHLEVEL);
    strcpy(msg.pgm, pe->slave_full_pathname);

    net_send(rm_fd, &msg, sizeof(msg), FALSE);

    port = -1;
    pidx = -1;
    for (done = FALSE; !done;)
    {
	status = net_recv(rm_fd, &msg, sizeof(msg));
	if (status == PRECV_EOF)
	{
	    p4_dprintf("OOPS! got EOF in net_slave_info\n");
	    return;
	}


	type = p4_n_to_i(msg.type);
	switch (type)
	{
	  case REMOTE_LISTENER_INFO:
	    port = p4_n_to_i(msg.port);
	    break;

	  case REMOTE_MASTER_INFO:
	  case REMOTE_SLAVE_INFO:
	    if (type == REMOTE_MASTER_INFO)
	       rm_ind = TRUE;
	    else
	       rm_ind = FALSE;
	    slave_idx = p4_n_to_i(msg.slave_idx);
	    slave_pid = p4_n_to_i(msg.slave_pid);
	    remote_switch_port = p4_n_to_i(msg.switch_port);
	    if (port == -1)
		p4_dprintf("OOPS! got slave_info w/o getting port first\n");
	    /* big master installing remote processes */
	    pidx = install_in_proctable(rm_num,port,slave_pid,
					pe->host_name,slave_idx,
					msg.machine_type,remote_switch_port);
            p4_dprintfl(90, "net_slave_info: adding connection to %d (%d) \n",
		        pidx,rm_num);

            if (p4_local->conntab[pidx].type == CONN_REMOTE_SWITCH)
            {
	        p4_local->conntab[pidx].switch_port = remote_switch_port;
	        p4_local->conntab[pidx].port = rm_fd;
            }
            else if (p4_local->conntab[pidx].type == CONN_REMOTE_NON_EST)
            {
		if (type == REMOTE_MASTER_INFO)
		{
	            p4_local->conntab[pidx].type = CONN_REMOTE_EST;
	            p4_local->conntab[pidx].port = rm_fd;
		    p4_local->conntab[pidx].same_data_rep =
			same_data_representation(p4_local->my_id,pidx);
		}
            }
            else
            {
	        p4_error("net_slave_info: invalid conn type in conntab\n",
		         p4_local->conntab[pidx].type);
            }
	    break;

	  case REMOTE_SLAVE_INFO_END:
	    done = TRUE;
	    break;
	}
    }
}

/* This routine is called if the net_accept fails to complete quickly */
#include <sys/time.h>
#ifndef TIMEOUT_VALUE 
#define TIMEOUT_VALUE 300
#endif
static char *curhostname = 0;
static char errbuf[256];
static int  child_pid = 0;
P4VOID p4_accept_timeout(sigval)
int sigval;
{
if (child_pid) {
    kill( child_pid, SIGQUIT );
    }
if (curhostname) {
    sprintf( errbuf, "Timeout in making connection to remote process on %s", 
	    curhostname );
    p4_error( errbuf, 0 );
    }
else {
    p4_error( "Timeout in making connection to remote process", 0 );
    }
exit(1);
}

/*
 *	Run the slave pgm on host; returns the file descriptor of the
 *	connection to the slave.
 */
int net_create_slave(serv_port, serv_fd, host, pgm, username)
int serv_port, serv_fd;
char *host;
char *pgm;
char *username;
{
    char hostname[100];
    struct net_initial_handshake hs;
    char myhost[100];
    struct net_message_t msg;
    char remote_shell[64];
    char serv_port_c[64];
    int connection_fd, success, rc;
    int slave_fd, master_pid;

#   if defined(LINUX)
    char *am_slave_c = "\\-p4amslave";
#   else
    char *am_slave_c = "-p4amslave";
#   endif

#   if defined(SYMMETRY) || defined(SUN) || \
    defined(DEC5000)  || defined(SGI) || \
    defined(RS6000)   || defined(HP)  || \
    defined(NEXT)     || defined(CRAY) || \
    defined(CONVEX)   || defined(KSR)  || \
    defined(FX2800)   || defined(FX2800_SWITCH)  || \
    defined(SP1)
    char *getpw_ss();
#   endif

#   if defined(SP1)
    strcpy(myhost,p4_global->proctable[0].host_name);
    p4_dprintfl(80,"net_create_slave: myhost=%s\n",myhost);
#   else
    myhost[0] = '\0';
    get_qualified_hostname(myhost);
#   endif

    if (hand_start_remotes)
    {
	printf("waiting for process on host %s:\n%s %s %d %s\n",
	       host, pgm, myhost, serv_port, am_slave_c);
        rc = 0;
    }
    else
    {
	/* try to connect to (secure) server */

#       if defined(SYMMETRY) || defined(SUN) || \
           defined(DEC5000)  || defined(SGI) || \
           defined(RS6000)   || defined(HP)  || \
           defined(NEXT)     || defined(CRAY) || \
           defined(CONVEX)   || defined(KSR)  || \
           defined(FX2800)   || defined(FX2800_SWITCH)  || \
           defined(SP1)

	/*****  secure server stuff  *******/
	p4_dprintfl(20, "trying to create remote slave on %s via server\n",host);
	rc = start_slave(host, username, pgm, serv_port, am_slave_c, getpw_ss);

	if (rc < -1)
	{
	    extern char *start_prog_error;

	    p4_dprintfl(20,"Warning from secure server: %s\n", start_prog_error);
	}
	else if (rc == 0)
	    p4_dprintfl(10, "created remote slave on %s via server\n",host);
	/*****************************************/

#       else

	rc = -1;

#       endif
    }

    if (rc <= -1)
    {
	/* try to connect to (old) server */
	connection_fd = net_conn_to_listener(host, UNRESERVED_PORT, 1);

	if (connection_fd >= 0)
	{
	    p4_dprintfl(20, "creating remote slave on %s via old server\n",host);
	    msg.type = p4_i_to_n(NET_EXEC);
	    strcpy(msg.pgm, pgm);
	    strcpy(msg.host, myhost);
	    strcpy(msg.am_slave, am_slave_c);
	    msg.port = p4_i_to_n(serv_port);
	    net_send(connection_fd, &msg, sizeof(msg), FALSE);
	    net_recv(connection_fd, &msg, sizeof(msg));

	    success = p4_n_to_i(msg.success);
	    if (!success)
	    {
		p4_dprintf("create failed: %s\n", msg.message);
		return (-1);
	    }
	    close(connection_fd);
	    p4_dprintfl(10, "created remote slave on %s via old server\n",host);
	}
	else
	{
#if defined(DELTA)
	    p4_dprintf("delta cannot create remote processes\n");
#else
	    p4_dprintfl(20, "creating remote slave on %s via remote shell\n",host);
#ifdef P4BSD
	    strcpy(remote_shell, "rsh");
#endif

/* RL - added || defined(RS6000) to get around afs problems.  In earlier
   versions of AIX we could not use rsh since rsh was the restricted
   shell, which has been renamed Rsh  */
#ifdef P4SYSV
#    if defined(TITAN) || defined(SGI) || defined(SUN_SOLARIS) || defined(RS6000)
	    strcpy(remote_shell, "rsh");
#    else
#        if defined(SYMMETRY_PTX)
	    strcpy(remote_shell, "resh");
#        else
	    strcpy(remote_shell, "remsh");
#        endif
#    endif
#endif

	    sprintf(serv_port_c, "%d", serv_port);
	    /* We should remember ALL of the children's pid's so we can 
	       forcibly stop them if necessary */
	    child_pid = rc = fork_p4();
	    if (rc == 0)
	    {
		rc = execlp(remote_shell, remote_shell,
			    host, "-l", username, "-n", pgm,
			    myhost, serv_port_c, am_slave_c, NULL);
		/* host,"-n","cluster","5",pgm,myhost,serv_port_c,0); for butterfly */
		if (rc < 0)
		    p4_error("net_create_slave: execlp", rc);
	    }
	    p4_dprintfl(10, "created remote slave on %s via remote shell\n",host);
	    p4_dprintfl(90, "remote slave is running program %s as user %s\n",
			pgm, username );
#endif
	}
    }
    /* WDG - There is a chance that we'll hang here forever.  Thus, we set a 
       timeout that causes the whole job to fail if we don't get a timely
       response from the created process.  See MPIBUGS #989; I got a traceback 
       showing this exact problem. 
     */
    curhostname = host;
    SIGNAL_P4(SIGALRM,p4_accept_timeout);
    {
#ifndef CRAY
    struct itimerval timelimit;
    struct timeval tval;
    struct timeval tzero;
    tval.tv_sec		  = TIMEOUT_VALUE;
    tval.tv_usec	  = 0;
    tzero.tv_sec	  = 0;
    tzero.tv_usec	  = 0;
    timelimit.it_interval = tzero;       /* Only one alarm */
    timelimit.it_value	  = tval;
    setitimer( ITIMER_REAL, &timelimit, 0 );
#else
    alarm( TIMEOUT_VALUE );
#endif
    slave_fd		  = net_accept(serv_fd);
    /* Go back to default handling of alarms */
    curhostname		  = 0;
    child_pid		  = 0;
#ifndef CRAY
    timelimit.it_value	  = tzero;   /* Turn off timer */
    setitimer( ITIMER_REAL, &timelimit, 0 );
#else
    alarm( 0 );
#endif
    SIGNAL_P4(SIGALRM,SIG_DFL);
    }

    hs.pid = (int) htonl(getpid());
    hs.rm_num = 0;   /* To make Insight etc happy */
    net_send(slave_fd, &hs, sizeof(hs), FALSE);
    net_recv(slave_fd, &hs, sizeof(hs));

    return(slave_fd);
}


