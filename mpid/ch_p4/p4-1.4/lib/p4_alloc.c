#include "p4.h"
#include "p4_sys.h"

struct local_data *alloc_local_bm()
{
    struct local_data *l;

    l = (struct local_data *) p4_malloc(sizeof(struct local_data));
    if (l == NULL)
    {
	p4_dprintf("OOPS: alloc_local_bm: p4_malloc failed \n");
	return (l);
    }

    l->am_bm = TRUE;
    l->listener_fd = -1;
    l->my_id = -1;
    l->procgroup = NULL;
    l->queued_messages = (struct p4_msg_queue *)
	p4_malloc(sizeof(struct p4_msg_queue));
    initialize_msg_queue(l->queued_messages);
    l->soft_errors = 0;

#   ifdef CAN_DO_XDR
    if ((l->xdr_buff = (char *) p4_malloc(XDR_BUFF_LEN)) == NULL)
    {
	p4_error("OOPS: alloc_local_bm: unable to malloc xdr_buff\n",NULL);
    }
    xdrmem_create(&(l->xdr_enc), l->xdr_buff, XDR_BUFF_LEN, XDR_ENCODE);
    xdrmem_create(&(l->xdr_dec), l->xdr_buff, XDR_BUFF_LEN, XDR_DECODE);
#   endif

    return (l);
}

struct local_data *alloc_local_rm()
{
    struct local_data *l;

    l = (struct local_data *) p4_malloc(sizeof(struct local_data));
    if (l == NULL)
    {
	p4_dprintf("OOPS: alloc_local_rm: p4_malloc failed \n");
	return (l);
    }

    l->am_bm = FALSE;
    l->listener_fd = -1;
    l->my_id = -1;
    l->procgroup = NULL;
    l->queued_messages = (struct p4_msg_queue *)
	p4_malloc(sizeof(struct p4_msg_queue));
    initialize_msg_queue(l->queued_messages);
    l->soft_errors = 0;

#   ifdef CAN_DO_XDR
    if ((l->xdr_buff = (char *) p4_malloc(XDR_BUFF_LEN)) == NULL)
    {
	p4_error("OOPS: alloc_local_rm: unable to malloc xdr_buff\n",NULL);
    }
    xdrmem_create(&(l->xdr_enc), l->xdr_buff, XDR_BUFF_LEN, XDR_ENCODE);
    xdrmem_create(&(l->xdr_dec), l->xdr_buff, XDR_BUFF_LEN, XDR_DECODE);
#   endif

    return (l);
}				/* alloc_local_rm */

struct local_data *alloc_local_listener()
{
    struct local_data *l;

    l = (struct local_data *) p4_malloc(sizeof(struct local_data));

    l->am_bm = FALSE;
    l->listener_fd = -1;
    l->my_id = LISTENER_ID;
    l->procgroup = NULL;
    l->queued_messages = NULL;
    l->xdr_buff = NULL;
    l->soft_errors = 0;
    return (l);
}				/* alloc_local_listener */

struct local_data *alloc_local_slave()
{
    struct local_data *l;

    l = (struct local_data *) p4_malloc(sizeof(struct local_data));

    l->am_bm = FALSE;
    l->listener_fd = -1;
    l->my_id = -1;
    l->procgroup = NULL;
    l->queued_messages = (struct p4_msg_queue *)
	p4_malloc(sizeof(struct p4_msg_queue));
    initialize_msg_queue(l->queued_messages);
    l->soft_errors = 0;

#   ifdef CAN_DO_XDR
    if (!(p4_global->local_communication_only))
    {
        if ((l->xdr_buff = (char *) p4_malloc(XDR_BUFF_LEN)) == NULL)
        {
	    p4_error("OOPS: alloc_local_slave: unable to malloc xdr_buff\n",NULL);
        }
        xdrmem_create(&(l->xdr_enc), l->xdr_buff, XDR_BUFF_LEN, XDR_ENCODE);
        xdrmem_create(&(l->xdr_dec), l->xdr_buff, XDR_BUFF_LEN, XDR_DECODE);
    }
#   endif

    return (l);
}

/*
    This routine should be called before any sends and receives are done by
    the user.  If not, some buffers may be lost.
*/
P4VOID p4_set_avail_buff(bufidx,size)
int bufidx;
int size;
{
    p4_global->avail_buffs[bufidx].size = size;
    p4_global->avail_buffs[bufidx].buff = NULL;
}


P4VOID init_avail_buffs()
{
    static int sizes[NUMAVAILS] = 
            {64,256,1024,4096,16384,65536,262144,1048576};
    int i;

    for (i = 0; i < NUMAVAILS; i++)
    {
        p4_global->avail_buffs[i].size = sizes[i];
        p4_global->avail_buffs[i].buff = NULL;
    }
}

P4VOID p4_print_avail_buffs()
{
    int i, count;
    struct p4_msg *next;

    p4_dprintf("avail lists for message buffers:\n");
    p4_lock(&p4_global->avail_buffs_lock);
    for (i = 0; i < NUMAVAILS; i++)
    {
	count = 0;
	for (next = p4_global->avail_buffs[i].buff; next; next = next->link)
	     count++;
	p4_dprintf("%d buffers of size %d\n",
		   count, p4_global->avail_buffs[i].size);
    }
    p4_unlock(&p4_global->avail_buffs_lock);
}

struct p4_msg *alloc_p4_msg(msglen)
int msglen;
{
    struct p4_msg *rmsg = NULL, **trailer;
    int i, rounded, buff_len;
    P4BOOL found;

    p4_dprintfl(40, "allocating a buffer for message of size %d\n", msglen);

#   if defined(TCMP)

    buff_len = sizeof(struct p4_msg) + msglen - sizeof(char *);
    rmsg = (struct p4_msg *) tcmp_allocate(buff_len);
    p4_dprintfl(40, "allocated new buffer at 0x%x for msg of size %d\n",
		rmsg, msglen);
    rmsg->len = msglen;
    rmsg->orig_len = msglen;
    return(rmsg);

#   else

    i = 0;
    while ((i < NUMAVAILS) && (msglen > p4_global->avail_buffs[i].size))
	i++;

    if (i == NUMAVAILS)		/* didn't find a big enough avail buffer */
    {
        buff_len = sizeof(struct p4_msg) + msglen - sizeof(char *);
	rmsg = (struct p4_msg *) p4_shmalloc(buff_len);
	p4_dprintfl(40, "allocated new buffer at0x%x for message size %d\n",
		    rmsg, msglen);
    }
    else
    {
	rounded = p4_global->avail_buffs[i].size;
	buff_len = sizeof(struct p4_msg) + rounded - sizeof(char *);
	p4_lock(&p4_global->avail_buffs_lock);
	if (p4_global->avail_buffs[i].buff)
	{

#if defined(IPSC860)
	    rmsg = p4_global->avail_buffs[i].buff;
	    trailer = &(p4_global->avail_buffs[i].buff);
	    found = FALSE;
	    while (!found && (rmsg != NULL))
	    {
		if (rmsg->msg_id == -1)
		{
		    found = TRUE;
		}
		else if (msgdone((long) rmsg->msg_id))
		{
		    rmsg->msg_id = -1;	
		    (p4_global->cube_msgs_out)--;
		    found = TRUE;
		}
		else
		{
		    trailer = &((*trailer)->link);
		    rmsg = rmsg->link;
		}
	    }
	    if (!found && (p4_global->cube_msgs_out > P4_MAX_CUBE_MSGS_OUT))
	    {
		if ((rmsg = p4_global->avail_buffs[i].buff) != NULL)
		{
		    trailer =  &(p4_global->avail_buffs[i].buff);
		    msgwait((long) rmsg->msg_id);
		    rmsg->msg_id = -1;	
		    (p4_global->cube_msgs_out)--;
		    found = TRUE;
		}
	    }
	    if (!found)
	    {
		rmsg = (struct p4_msg *) p4_shmalloc(buff_len);
		p4_dprintfl(40, "allocated new buffer at 0x%x of size %d for message size %d\n", rmsg, rounded, msglen);
	    }
	    else
	    {
		*trailer = rmsg->link;
		p4_dprintfl(40, "reused a buffer of size %d for message size %d\n",
			    rounded, msglen);
	    }
#else
	    rmsg = p4_global->avail_buffs[i].buff;
	    p4_global->avail_buffs[i].buff = rmsg->link;
	    p4_dprintfl(40, "reused a buffer of size %d for message size %d\n",
			rounded, msglen);
#endif

	    p4_unlock(&p4_global->avail_buffs_lock);
	}
	else
	{
	    p4_unlock(&p4_global->avail_buffs_lock);
	    rmsg = (struct p4_msg *) p4_shmalloc(buff_len);
	    p4_dprintfl(40, "allocated new buffer at 0x%x of size %d for message size %d\n", rmsg, rounded, msglen);
	}
    }

    if ((rmsg == NULL) && !SOFTERR)
	p4_error("alloc_p4_msg failed", 0);

    rmsg->len = msglen;
    rmsg->orig_len = msglen;
    return(rmsg);

#   endif
}				/* alloc_p4_msg */

P4VOID free_p4_msg(tmsg)
struct p4_msg *tmsg;
{
    int i;
    struct p4_msg *p;

    p4_dprintfl(40, "freeing a buffer with bufflen=%d msglen=%d\n", 
		tmsg->orig_len,tmsg->len);

    /* Sanity check here as bad message pointer causes havoc */

    if ((tmsg->orig_len < 0) || (tmsg->orig_len > P4_MAX_MSGLEN))
	p4_error("free_p4_msg: bad hdr: msglen out of range", tmsg->len);

#   if defined(TCMP)
    if (tmsg)
	tcmp_deallocate(tmsg);
    return;

#   else

    i = 0;
    while ((i < NUMAVAILS) && (tmsg->orig_len > p4_global->avail_buffs[i].size))
	i++;

    if (i == NUMAVAILS)
    {
	/* buffer being freed is not a kept size */
	p4_shfree(tmsg);
	p4_dprintfl(40, "freeing a buffer at %d with bufflen=%d msglen=%d\n", 
		    tmsg,tmsg->orig_len,tmsg->len);
    }
    else
    {
	/* hook new buffer in at end of list */
	p4_lock(&p4_global->avail_buffs_lock);
	if ((p = p4_global->avail_buffs[i].buff) == NULL)
	    p4_global->avail_buffs[i].buff = tmsg;
	else
	{
	    while (p->link != NULL)
		p = p->link;
	    p->link = tmsg;
	}
	tmsg->link = NULL;
	p4_unlock(&p4_global->avail_buffs_lock);
	p4_dprintfl(40, "saved a buffer of size %d in avail list for size %d\n",
		    tmsg->orig_len, p4_global->avail_buffs[i].size);
    }

#   endif
}				/* free_p4_msg */

P4VOID alloc_global()
{
    int i;
    struct p4_global_data *g;

    p4_global = (struct p4_global_data *) p4_shmalloc(sizeof(struct p4_global_data));
    if (p4_global == NULL)
    {
	p4_error("alloc_global: alloc_global failed\n", sizeof(struct p4_global_data));
    }
    g = p4_global;

#ifdef SYSV_IPC
    g->slave_lock.semid = sysv_semid0;
    g->slave_lock.semnum = 1;
    g->sysv_semid[0] = sysv_semid0;
    g->sysv_num_semids = 1;
    g->sysv_next_lock = 2;  /* shmem_lock is 0 & slave_lock is 1 */
#else
    p4_lock_init(&g->slave_lock);
#endif

    g->listener_pid = -1;
    g->listener_port = -1;

    g->cube_msgs_out = 0;

    g->local_slave_count = 0;
    g->local_communication_only = TRUE;
    g->n_forked_pids = 0;

    for (i = 0; i < P4_MAX_MSG_QUEUES; i++)
    {
	initialize_msg_queue(&g->shmem_msg_queues[i]);
	g->dest_id[i] = -1;
    }

    p4_lock_init(&g->avail_buffs_lock);
    init_avail_buffs();
    p4_lock_init(&g->avail_quel_lock);
    g->avail_quel = NULL;

    g->num_in_proctable = 0;
    g->num_installed = 0;

#   if !defined(SUN_SOLARIS) && !defined(MEIKO_CS2)
    gethostname(g->my_host_name,HOSTNAME_LEN);
#   else
    sysinfo(SI_HOSTNAME, g->my_host_name, HOSTNAME_LEN);
#   endif

    /* get_qualified_hostname(g->my_host_name); */

    p4_barrier_init(&(g->cluster_barrier));

    sprintf(g->application_id, "p4_%-8d", getpid());

#   if defined(P4BSD)
    g->max_connections = getdtablesize();
#   endif

#   if defined(P4SYSV)
#       if defined(CRAY) || defined(RS6000) || \
	   defined(IBM3090) || defined(SYMMETRY_PTX) || defined(HP)
              g->max_connections = getdtablesize();
#       else
#       if defined(IPSC860) || defined(NCUBE)
              g->max_connections = 20;
#       else
              g->max_connections = (int) ulimit(4,0);
#       endif
#       endif
#   endif

}

struct listener_data *alloc_listener_info()
{
    struct listener_data *l;

    l = (struct listener_data *) p4_malloc(sizeof(struct listener_data));

    l->listening_fd = -1;
    l->slave_fd = -1;

    return (l);
}
