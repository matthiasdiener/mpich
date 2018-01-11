/*
 *
 *  Process client request
 *
 */
#include "mpd.h"

extern struct fdentry fdtable[MAXFDENTRIES];
extern struct procentry proctable[MAXPROCS];
extern struct jobentry jobtable[MAXJOBS];
extern int  rhs_idx;
extern char myid[IDSIZE];
extern int    debug;


void cli_alive( client_fd )
int client_fd;
{
    int i, n, cid, jidx, listenport, num_here_in_job;
    char buf[MAXLINE];

    getval("port",buf);
    listenport = atoi(buf);
    getval("cid",buf);
    cid = atoi(buf);
 
    proctable[cid].lport = listenport;
    proctable[cid].clientfd = client_fd;
    proctable[cid].state = CLALIVE;

    if ( ( jidx = find_jobid_in_jobtable(proctable[cid].jobid) ) < 0 )
    {
	mpdprintf( 1,"cli_alive: did not find job %d in jobtable\n",proctable[cid].jobid );
	exit(-1);
    }

    jobtable[jidx].alive_here_sofar++;
    mpdprintf( debug, "client cid=%d alive, jobsync_is_here=%d alive_here_sofar=%d\n",
	       cid,jobtable[jidx].jobsync_is_here,jobtable[jidx].alive_here_sofar);
    for ( i=0,num_here_in_job=0; i < MAXPROCS; i++ ) {
        if ( proctable[i].active && proctable[i].jobid == jobtable[jidx].jobid ) {
	    num_here_in_job++;
	}
    }
    if ( jobtable[jidx].jobsync_is_here  &&  
	 num_here_in_job == jobtable[jidx].alive_here_sofar )
    {
	n = jobtable[jidx].alive_here_sofar - jobtable[jidx].added_to_job_sofar;
	/* RMB BUG ??  jobtable[jidx].alive_here_sofar += n; */
	jobtable[jidx].added_to_job_sofar += n;
	jobtable[jidx].alive_in_job_sofar += n;
	sprintf( buf, "src=%s dest=anyone cmd=jobsync job=%d jobsize=%d sofar=%d\n",
		 myid, jobtable[jidx].jobid, jobtable[jidx].jobsize, 
		 jobtable[jidx].alive_in_job_sofar );
	mpdprintf( debug, "cli_alive: sending jobsync: job=%d sofar=%d\n",
		 jobtable[jidx].jobid, jobtable[jidx].alive_in_job_sofar );
	write_line( rhs_idx, buf );
	jobtable[jidx].jobsync_is_here = 0;
    }
}

void cli_findclient(client_fd)
int client_fd;
{    
    char buf[MAXLINE];
    int cid, rank, job;

    getval("cid",buf);
    cid = atoi(buf);
    getval("job",buf);
    job = atoi(buf);
    getval("rank",buf);
    rank = atoi(buf);
    sprintf(buf,"src=%s dest=%s bcast=true cid=%d cmd=findclient job=%d rank=%d\n",
	    myid,myid,cid,job,rank);
    write_line( rhs_idx, buf );
    return;
}
