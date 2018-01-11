#include "p4.h"
#include "p4_sys.h"

static char hold_patchlevel[16];
static char hold_machine_type[16];

char *p4_version()
{
    strcpy(hold_patchlevel,P4_PATCHLEVEL);
    return(hold_patchlevel);
}

char *p4_machine_type()
{
    strcpy(hold_machine_type,P4_MACHINE_TYPE);
    return(hold_machine_type);
}


int p4_initenv(argc, argv)
int *argc;
char **argv;
{
    int i, rc;
    P4BOOL am_slave = FALSE;

    /*****
    char hname[100];
    int hlen = 100;

    gethostname(hname,hlen);
    printf("entering p4_initenv on %s\n",hname);
    *****/

    sprintf(whoami_p4, "xm_%d", getpid());

    logging_flag = FALSE;
    globmemsize = GLOBMEMSIZE;
    sserver_port = 753;
    process_args(argc,argv);

    for (i=0; i < *argc; i++)
    {
	if ((strcmp(argv[i], "-p4amslave") == 0) ||
	    (strcmp(argv[i], "-amp4slave") == 0) ||
            (strcmp(argv[i], "-p4amp4slave") == 0))
	{
	    strcpy(argv[i]," ");    
	    /* strip this arg as Prolog engine flags begin with - SRM */
	    am_slave = TRUE;
	}
    }

    p4_dprintfl(90,"mport=%d nodenum=%d\n",execer_mastport,execer_mynodenum);
    if (execer_mastport  &&  execer_mynodenum)
        am_slave = TRUE;

#   if defined(SP1_EUI)
    mpc_environ(&eui_numtasks,&eui_mynode);
#   endif

#   if defined(SP1_EUIH)
    mp_environ(&euih_numtasks,&euih_mynode);
#   endif

#   if defined(MEIKO_CS2)
    mpsc_init();
#   endif

#   if defined(IPSC860) || defined(CM5)     || defined(NCUBE)  \
                        || defined(SP1_EUI) || defined(SP1_EUIH)

    if (MYNODE() != 0)
        am_slave = TRUE;
#    endif

#   if defined(CM5)
    CMMD_fset_io_mode( stdin,  CMMD_independent );
    CMMD_fset_io_mode( stdout, CMMD_independent );
    CMMD_fset_io_mode( stderr, CMMD_independent );
#   endif

    p4_local  = NULL;
    p4_global = NULL;

    if (am_slave)
    {
#       if defined(IPSC860)  ||  defined(CM5)     ||  defined(NCUBE) \
	                     ||  defined(SP1_EUI) || defined(SP1_EUIH)
        if (MYNODE() == 0)
	{
#           if defined(IPSC860_SOCKETS)  ||  defined(CM5_SOCKETS)  ||  defined(NCUBE_SOCKETS)
	    rc = rm_start(argc, argv);
#           endif
	}
	else
	{
	    rc = ns_start(argc, argv);
	}
#       else
	rc = rm_start(argc, argv);
#       endif
	ALOG_SETUP(p4_local->my_id,ALOG_TRUNCATE);
    }
    else
    {
	rc = bm_start(argc, argv);
        ALOG_MASTER(0,ALOG_TRUNCATE);
        ALOG_DEFINE(BEGIN_USER,"beg_user","");
        ALOG_DEFINE(END_USER,"end_user","");
        ALOG_DEFINE(BEGIN_SEND,"beg_send","");
        ALOG_DEFINE(END_SEND,"end_send","");
        ALOG_DEFINE(BEGIN_RECV,"beg_recv","");
        ALOG_DEFINE(END_RECV,"end_recv","");
        ALOG_DEFINE(BEGIN_WAIT,"beg_wait","");
        ALOG_DEFINE(END_WAIT,"end_wait","");
    }
    ALOG_LOG(p4_local->my_id,BEGIN_USER,0,"");
    return (rc);
}

char *p4_shmalloc(n)
int n;
{
    char *rc;

    if ((rc = MD_shmalloc(n)) == NULL)
	p4_dprintf("p4_shmalloc returning NULL; request = %d bytes\n",n);
    return (rc);
}

P4VOID p4_shfree(p)
char *p;
{
    MD_shfree(p);
}

int p4_num_cluster_ids()
{
    return (p4_global->local_slave_count + 1);
}

int p4_num_total_ids()
{
    return (p4_global->num_in_proctable);
}

int p4_num_total_slaves()
{
    return (p4_global->num_in_proctable - 1);
}


P4VOID p4_global_barrier(type)
int type;
{
    int dummy[1];

    dummy[0] = 0;
    p4_global_op(type, (char *) dummy, 1, sizeof(int), p4_int_sum_op, P4INT);
}


P4VOID p4_get_cluster_masters(numids, ids)
int *numids, ids[];
{
    int node;

    ids[0] = 0;
    *numids = 1;
    for (node = 1; node < p4_global->num_in_proctable; node++)
    {
	if (p4_global->proctable[node].slave_idx != 0)
	    continue;
	ids[(*numids)++] = node;
    }
}


P4VOID p4_get_cluster_ids(start, end)
int *start;
int *end;
{

    *start = p4_global->low_cluster_id;
    *end = p4_global->hi_cluster_id;
}

/* This is used to figure out the local id of the calling process by
 * indexing into the proctable until you find a hostname and a unix id
 * that are the same as yours.
 */
int p4_get_my_id_from_proc()
{
    int i, my_unix_id;
    struct proc_info *pi;
    struct hostent *myhp, *pghp;

#   if (defined(IPSC860)  &&  !defined(IPSC860_SOCKETS))  ||  \
       (defined(CM5)      &&  !defined(CM5_SOCKETS))      ||  \
       (defined(NCUBE)    &&  !defined(NCUBE_SOCKETS))    ||  \
       (defined(SP1_EUI))                                 ||  \
       (defined(SP1_EUIH))
    return(MYNODE());
#   else
    my_unix_id = getpid();
    if (p4_local->my_id == LISTENER_ID)
	return (LISTENER_ID);
    myhp = gethostbyname_p4(p4_global->my_host_name);

    for (pi = p4_global->proctable, i = 0; i < p4_global->num_in_proctable; i++, pi++)
    {
	if (pi->unix_id == my_unix_id)
	{
	    if (strcmp(pi->host_name, p4_global->my_host_name) == 0)
	    {
		return (i);
	    }
	    /* all nodes on sp1 seem to have same inet address by this test */
#           if !defined(SP1)
	    else
	    {
		pghp = gethostbyname_p4(pi->host_name);
		if (bcmp(myhp->h_addr, pghp->h_addr, myhp->h_length) == 0)
		{
		    return (i);
		}
	    }
#           endif
	}
    }
    p4_dprintf("process not in process table; my_unix_id = %d my_host=%s\n",
	       getpid(), p4_global->my_host_name);
    p4_dprintf("Probable cause:  local slave on uniprocessor without shared memory\n");
    p4_dprintf("Probable fix:  ensure only one process on %s\n",p4_global->my_host_name);
    p4_dprintf("(on master process this means 'local 0' in the procgroup file)\n");
    p4_dprintf("You can also remake p4 with SYSV_IPC set in the OPTIONS file\n");
    p4_error("p4_get_my_id_from_proc",0);
#   endif
    return (-1);
}

int p4_get_my_id()
{
    return (p4_local->my_id);
}

int p4_get_my_cluster_id()
{
#   if (defined(IPSC860)  &&  !defined(IPSC860_SOCKETS))  ||  \
       (defined(CM5)      &&  !defined(CM5_SOCKETS))      ||  \
       (defined(NCUBE)    &&  !defined(NCUBE_SOCKETS))    ||  \
       (defined(SP1_EUI))                                 ||  \
       (defined(SP1_EUIH))              
    return(MYNODE());
#   else
    if (p4_local->my_id == LISTENER_ID)
	return (LISTENER_ID);
    else
	return (p4_global->proctable[p4_local->my_id].slave_idx);
#   endif
}

P4BOOL p4_am_i_cluster_master()
{
    if (p4_local->my_id == LISTENER_ID)
	return (0);
    else
	return (p4_global->proctable[p4_local->my_id].slave_idx == 0);
}

P4BOOL in_same_cluster(i, j)
int i, j;
{
    return (p4_global->proctable[i].group_id ==
	    p4_global->proctable[j].group_id);
}

P4VOID p4_cluster_shmem_sync(cluster_shmem)
P4VOID **cluster_shmem;
{
    int myid = p4_get_my_cluster_id();

    if (myid == 0)  /* cluster master */
	p4_global->cluster_shmem = *cluster_shmem;
    p4_barrier(&(p4_global->cluster_barrier),p4_num_cluster_ids());
    if (myid != 0)
	*cluster_shmem = p4_global->cluster_shmem;
}

#if defined(USE_XX_SHMALLOC)
/* This is not machine dependent code but is only used on some machines */

/*
  Memory management routines from ANSI K&R C, modified to manage
  a single block of shared memory.
  Have stripped out all the usage monitoring to keep it simple.

  To initialize a piece of shared memory:
    xx_init_shmalloc(char *memory, unsigned nbytes)

  Then call xx_shmalloc() and xx_shfree() as usual.
*/

#define LOG_ALIGN 6
#define ALIGNMENT (1 << LOG_ALIGN)

/* ALIGNMENT is assumed below to be bigger than sizeof(p4_lock_t) +
   sizeof(Header *), so do not reduce LOG_ALIGN below 4 */

union header
{
    struct
    {
	union header *ptr;	/* next block if on free list */
	unsigned size;		/* size of this block */
    } s;
    char align[ALIGNMENT];	/* Align to ALIGNMENT byte boundary */
};

typedef union header Header;

static Header **freep;		/* pointer to pointer to start of free list */
static p4_lock_t *shmem_lock;	/* Pointer to lock */

P4VOID xx_init_shmalloc(memory, nbytes)
char *memory;
unsigned nbytes;
/*
  memory points to a region of shared memory nbytes long.
  initialize the data structures needed to manage this memory
*/
{
    int nunits = nbytes >> LOG_ALIGN;
    Header *region = (Header *) memory;

    /* Quick check that things are OK */

    if (ALIGNMENT != sizeof(Header) ||
	ALIGNMENT < (sizeof(Header *) + sizeof(p4_lock_t)))
    {
        p4_dprintfl(00,"%d %d\n",sizeof(Header),sizeof(p4_lock_t));
	p4_error("xx_init_shmem: Alignment is wrong", ALIGNMENT);
    }

    if (!region)
	p4_error("xx_init_shmem: Passed null pointer", 0);

    if (nunits < 2)
	p4_error("xx_init_shmem: Initial region is ridiculously small",
		 (int) nbytes);

    /*
     * Shared memory region is structured as follows
     * 
     * 1) (Header *) freep ... free list pointer 2) (p4_lock_t) shmem_lock ...
     * space to hold lock 3) padding up to alignment boundary 4) First header
     * of free list
     */

    freep = (Header **) region;	/* Free space pointer in first block  */
    shmem_lock = (p4_lock_t *) (freep + 1);	/* Lock still in first block */
    (region + 1)->s.ptr = *freep = region + 1;	/* Data in rest */
    (region + 1)->s.size = nunits - 1;	/* One header consumed already */

#   ifdef SYSV_IPC
    shmem_lock->semid = sysv_semid0;
    shmem_lock->semnum = 0;
#   else
    p4_lock_init(shmem_lock);                /* Initialize the lock */
#   endif

}

char *xx_shmalloc(nbytes)
unsigned nbytes;
{
    Header *p, *prevp;
    char *address = (char *) NULL;
    unsigned nunits;

    /* Force entire routine to be single threaded */
    (P4VOID) p4_lock(shmem_lock);

    nunits = ((nbytes + sizeof(Header) - 1) >> LOG_ALIGN) + 1;

    prevp = *freep;
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
    {
	if (p->s.size >= nunits)
	{			/* Big enuf */
	    if (p->s.size == nunits)	/* exact fit */
		prevp->s.ptr = p->s.ptr;
	    else
	    {			/* allocate tail end */
		p->s.size -= nunits;
		p += p->s.size;
		p->s.size = nunits;
	    }
	    *freep = prevp;
	    address = (char *) (p + 1);
	    break;
	}
	if (p == *freep)
	{			/* wrapped around the free list ... no fit
				 * found */
	    address = (char *) NULL;
	    break;
	}
    }

    /* End critical region */
    (P4VOID) p4_unlock(shmem_lock);

    if (address == NULL)
	p4_dprintf("xx_shmalloc: returning NULL; requested %d bytes\n",nbytes);
    return address;
}

P4VOID xx_shfree(ap)
char *ap;
{
    Header *bp, *p;

    /* Begin critical region */
    (P4VOID) p4_lock(shmem_lock);

    if (!ap)
	return;			/* Do nothing with NULL pointers */

    bp = (Header *) ap - 1;	/* Point to block header */

    for (p = *freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
	    break;		/* Freed block at start of end of arena */

    if (bp + bp->s.size == p->s.ptr)
    {				/* join to upper neighbour */
	bp->s.size += p->s.ptr->s.size;
	bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
	bp->s.ptr = p->s.ptr;

    if (p + p->s.size == bp)
    {				/* Join to lower neighbour */
	p->s.size += bp->s.size;
	p->s.ptr = bp->s.ptr;
    }
    else
	p->s.ptr = bp;

    *freep = p;

    /* End critical region */
    (P4VOID) p4_unlock(shmem_lock);
}
#endif

P4VOID get_pipe(end_1, end_2)
int *end_1;
int *end_2;
{
    int p[2];

#   if defined(IPSC860)  ||  defined(CM5)  ||  defined(NCUBE)  ||  defined(SP1_EUI) || defined(SP1_EUIH)
    p4_dprintf("WARNING: get_pipe: socketpair assumed unavailable on this machine\n");
    return;
#   else
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
	p4_error("get_pipe: socketpair failed ", -1);
    *end_1 = p[0];
    *end_2 = p[1];
#   endif
}

P4VOID setup_conntab()
{
    int i, my_id;

    p4_dprintfl(60, "setup_conntab: myid=%d, switch_port=%d, app_id=%s\n",
		p4_local->my_id,
		p4_global->proctable[p4_local->my_id].switch_port,
		p4_global->application_id);
    p4_local->conntab = (struct connection *)
	p4_malloc(p4_global->num_in_proctable * sizeof(struct connection));
    my_id = p4_get_my_id();

    for (i = 0; i < p4_global->num_in_proctable; i++)
    {
	if (i == my_id)
	{
	    p4_local->conntab[i].type = CONN_ME;
#           if defined(IPSC860) || defined(CM5)  ||  defined(NCUBE)  ||  defined(SP1_EUI) || defined(SP1_EIUH)
	    p4_local->conntab[i].port = MYNODE();
#           endif
#           if defined(TCMP)
	    p4_local->conntab[i].port = i;
#           endif
	}
	else if (in_same_cluster(i, my_id))
	{
	    p4_local->conntab[i].type = CONN_LOCAL;
#           if defined(IPSC860) || defined(CM5)  ||  defined(NCUBE)  || defined(SP1_EUI) || defined(SP1_EUIH)
	    p4_local->conntab[i].port = MYNODE() + i - p4_local->my_id;
#           endif
#           if defined(TCMP)
	    p4_local->conntab[i].port = i - p4_global->low_cluster_id;
#           endif
	}
	else if ((p4_global->proctable[my_id].switch_port != -1) &&
		 (p4_global->proctable[i].switch_port != -1) &&
		 (p4_global->proctable[my_id].switch_port !=
		  p4_global->proctable[i].switch_port))
	{
	    p4_local->conntab[i].type = CONN_REMOTE_SWITCH;
	    p4_local->conntab[i].switch_port = p4_global->proctable[i].switch_port;
	}
	else
	{
	    p4_local->conntab[i].type = CONN_REMOTE_NON_EST;
	    p4_local->conntab[i].port = p4_global->proctable[i].port;
	}
    }
    p4_dprintfl(60, "conntab after setup_conntab:\n");
    dump_conntab(60);
}

#ifdef SYSV_IPC
P4VOID remove_sysv_ipc()
{
    int i;
    struct p4_global_data *g = p4_global;

    /* ignore -1 return codes below due to multiple processes cleaning
       up the same sysv stuff; commented out "if" used to make sure
       that only the cluster master cleaned up in each cluster
    */
    /* if (p4_local != NULL  &&  p4_get_my_cluster_id() != 0) return; */

    if (sysv_shmid[0] == -1)
	return;
    for (i=0; i < sysv_num_shmids; i++)
        shmctl(sysv_shmid[i],IPC_RMID,0);
    if (g == NULL)
        return;
    if (sysv_semid0 != -1)
	semctl(g->sysv_semid[0],0,IPC_RMID,0);  /* delete initial set */
    for (i=1; i < g->sysv_num_semids; i++)  /* delete other sets */
    {
	semctl(g->sysv_semid[i],0,IPC_RMID,0);
    }
}
#endif

int p4_wait_for_end()
{
    int status;
    int i, n_forked_slaves, pid;
    struct slave_listener_msg msg;
    char job_filename[64];
    struct p4_msg *mptr;

    ALOG_LOG(p4_local->my_id,END_USER,0,"");
    ALOG_OUTPUT;

#   if defined(IPSC860)
    for (i=0; i < NUMAVAILS; i++)
    {
	mptr = p4_global->avail_buffs[i].buff;
	while (mptr)
	{
	    if ((mptr->msg_id != -1) && (!msgdone((long) mptr->msg_id)))
		msgwait((long) mptr->msg_id);
	    mptr = mptr->link;
	}
    }
#   endif

#   if defined(MEIKO_CS2)
    mpsc_fini();
#   endif

    if (p4_get_my_cluster_id() != 0)
	return (0);

    /* Free avail buffers */
    free_avail_buffs();

    /* Wait for all forked processes except listener to die */
    p4_dprintfl(90, "enter wait_for_end nfpid=%d\n",p4_global->n_forked_pids);
    if (p4_local->listener_fd == (-1))
        n_forked_slaves = p4_global->n_forked_pids;
    else
        n_forked_slaves = p4_global->n_forked_pids - 1;
    for (i = 0; i < n_forked_slaves; i++)
    {
	pid = wait(&status);
	p4_dprintfl(90, "detected that proc %d died \n", pid);
    }

#   if defined(CAN_DO_SOCKET_MSGS)
    /* Tell the listener to die and wait for him to do so */
    if (p4_local->listener_fd != (-1))
    {
	p4_dprintfl(90, "tell listener to die listpid=%d fd=%d\n",
		    p4_global->listener_pid, p4_local->listener_fd);
	msg.type = p4_i_to_n(DIE);
	msg.from = p4_i_to_n(p4_get_my_id());
	net_send(p4_local->listener_fd, &msg, sizeof(msg), FALSE);
	pid = wait(&status);
	p4_dprintfl(90, "detected that proc %d died \n", pid);
    }
    /* free listener data structures */
    

#   endif

#   if defined(SYSV_IPC)
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

    if (p4_get_my_id())
        p4_dprintfl(20,"process exiting\n");
    p4_dprintfl(90, "exit wait_for_end \n");

    /* free assorted data structures */
    p4_free(listener_info);
    if (p4_local->procgroup) 
	p4_free(p4_local->procgroup);
    p4_free(p4_local->conntab);
    p4_shfree(p4_local->queued_messages->m.qs);
    p4_free(p4_local->queued_messages);
    p4_free(p4_local->xdr_buff);
    p4_free(p4_local);
    free_avail_quels();		/* (in p4_global)  */

    for (i = 0; i < P4_MAX_MSG_QUEUES; i++) 
	p4_shfree(p4_global->shmem_msg_queues[i].m.qs);
    p4_shfree(p4_global->cluster_barrier.m.qs);
    p4_shfree(p4_global);

    return (0);
}


/* static variables private to fork_p4 and zap_p4_processes */
static int n_pids = 0;
static int pid_list[P4_MAXPROCS];

int fork_p4()
/*
  Wrapper round fork for sole purpose of keeping track of pids so 
  that can signal error conditions.  See zap_p4_processes.
*/
{
    int pid;

#   if defined(IPSC860)  ||  defined(CM5)  ||  defined(NCUBE)  ||  defined(SP1_EUI) || defined(SP1_EIUH)
    p4_error("fork_p4: nodes cannot fork processes",0);    
#   else
    if (p4_global->n_forked_pids >= P4_MAXPROCS)
	p4_error("forking too many local processes; max = ", P4_MAXPROCS);
    p4_global->n_forked_pids++;

    fflush(stdout);
    pid = fork();

    if (pid > 0)
    {
	/* Parent process */
	pid_list[n_pids++] = pid;
#if defined(SUN_SOLARIS)
/*****	{ processorid_t proc = 0;
	  if(p_online(proc,P_STATUS) != P_ONLINE)
	    printf("Could not bind parent to processor 0\n");
	  else
	    {
	      processor_bind(P_PID,P_MYID,proc, &proc);
	      printf("Bound parent to processor 0 , previous binding was %d\n",
		     proc);
	    }
	}
*****/
#endif
    }
    else if (pid == 0)
    {
	/* Child process */
	pid_list[n_pids++] = getppid();
    }
    else
	p4_error("fork_p4: fork failed", pid);
#   endif

    return pid;
}

P4VOID zap_p4_processes()
{
    int n;
    
    if (p4_global == NULL)
        return;
    n = p4_global->n_forked_pids;
    while (n--)
    {
	kill(pid_list[n], SIGINT);
    }
}

P4VOID get_qualified_hostname(str)
char *str;
{
#   if (defined(IPSC860)  &&  !defined(IPSC860_SOCKETS))  ||  \
       (defined(CM5)      &&  !defined(CM5_SOCKETS))      ||  \
       (defined(NCUBE)    &&  !defined(NCUBE_SOCKETS))    ||  \
       (defined(SP1_EUI))                                 ||  \
       (defined(SP1_EUIH))
    strcpy(str,"cube_node");
#   else
#       if defined(SUN_SOLARIS)
        if (*str == '\0')
            if (sysinfo(SI_HOSTNAME, str, HOSTNAME_LEN) == -1)
	        p4_error("could not get qualified hostname", getpid());
#       else
        if (*str == '\0')
            if (p4_global)
                strcpy(str,p4_global->my_host_name);
            else
                gethostname(str, 100);
#       endif
    if (*local_domain != '\0'  &&  !index(str,'.'))
    {
	strcat(str,".");
	strcat(str,local_domain);
    }
#endif
}


int getswport(hostname)
char *hostname;
{
    char local_host[256];

#ifdef CAN_DO_SWITCH_MSGS
    if (strcmp(hostname, "local") == 0)
    {
	local_host[0] = '\0';
	get_qualified_hostname(local_host);
	return getswport(local_host);
    }
    if (strcmp(hostname, "hurley") == 0)
	return 1;
    if (strcmp(hostname, "hurley.tcg.anl.gov") == 0)
	return 1;
    if (strcmp(hostname, "hurley.mcs.anl.gov") == 0)
	return 1;
    if (strcmp(hostname, "campus.mcs.anl.gov") == 0)
	return 2;
    if (strcmp(hostname,"mpp1") == 0)
      return 3;
    if (strcmp(hostname,"mpp2") == 0)
      return 28;
    if (strcmp(hostname,"mpp3") == 0)
      return 6;
    if (strcmp(hostname,"mpp4") == 0)
      return 7;
    if (strcmp(hostname,"mpp7") == 0)
      return 14;
    if (strcmp(hostname,"mpp8") == 0)
      return 25;
    if (strcmp(hostname,"mpp9") == 0)
      return 20;
    if (strcmp(hostname,"mpp10") == 0)
      return 11;
#endif

    return -1;
}

P4BOOL same_data_representation(id1,id2)
{
    struct proc_info *p1 = &(p4_global->proctable[id1]);
    struct proc_info *p2 = &(p4_global->proctable[id2]);

    return (data_representation(p1->machine_type) == data_representation(p2->machine_type));
}


put_execer_port(port)
int port;
{
    int fd;
    char job_filename[64];
    char port_c[16];

    sprintf(port_c,"%d",port);
    strcpy(job_filename,"/tmp/p4_");
    strcat(job_filename,execer_jobname);
    if ((fd = open(job_filename, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
    {
	p4_error("put_execer_port: open failed ",fd);
    }
    if ((write(fd,port_c,strlen(port_c)+1)) != strlen(port_c)+1)
    {
	p4_error("put_execer_port: write failed ",(-1));
    }
    close(fd);
}

get_execer_port(master_hostname)
char *master_hostname;
{
    int port, num_read, sleep_time, status;
    FILE *fp;
    char cmd[128];

    sprintf(cmd,"rsh %s cat /tmp/p4_%s",master_hostname,execer_jobname);
    num_read = 0;
    sleep_time = 4;
    while (num_read != 1  &&  sleep_time < 128)
    {
        if ((fp = (FILE *) popen(cmd,"r")) == NULL)
        {
	    wait(&status);  /* for the rsh started by popen */
            sleep(sleep_time);
	    sleep_time *= 2;
        }
	else
	{
	    num_read = fscanf(fp,"%d",&port);
	    pclose(fp);
	}
    }

    if (num_read != 1)
    {
	p4_error("get_execer_port: never got good port",(-1));
    }

    return(port);
}

/* high-resolution clock, made out of p4_clock and p4_ustimer */

static int clock_start_ms;
static usc_time_t ustimer_start;
static usc_time_t usrollover;

P4VOID init_usclock()
{
    clock_start_ms = p4_clock();
    ustimer_start  = p4_ustimer();
    usrollover     = usc_rollover_val();
}

double p4_usclock()
{
    int elapsed_ms, q, r;
    double rc, roll, beginning, end;

    if (usrollover == 0)
	return( .001*p4_clock() );

    elapsed_ms = p4_clock() - clock_start_ms; /* milliseconds */
    q  =  elapsed_ms / (int)(usrollover/1000);/* num rollover-sized intervals*/
    beginning = (double)(usrollover - ustimer_start); /* initial segment */
    end = p4_ustimer();                               /* terminal segment */
    roll = (double)(usrollover * 0.000001);           /* rollover in seconds */
    if (q == 0)
        if (ustimer_start < end)
            rc = (double) ((end - (double)ustimer_start) * 0.000001);
        else
            rc = (double) (beginning + end);
    else
        rc = (double) (((beginning + end ) * 0.000001) + (q * roll));
    return(rc);
}
