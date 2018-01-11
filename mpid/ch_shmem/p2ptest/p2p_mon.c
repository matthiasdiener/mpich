#include "p2p_mon.h"
#include <stdio.h>

#define p2p_dprintf printf

int p2p_moninit(m, i)
p2p_monitor_t *m;
int i;
{
    int j;
    struct p2p_mon_queue *q;
    int rc;

    p2p_lock_init(&(m->mon_lock));
    /*****
    fprintf(stderr,"original lock addr is %d\n",&(m->mon_lock));
    fprintf(stderr,"ret from p2p_lock_init on mon_lock: %d\n",rc);
    if (rc != &(m->mon_lock)  &&  rc <= 0)
	perror("bad ret from p2p_lock_init on mon_lock");
    *****/

    if (i)
    {
	m->qs = (struct p2p_mon_queue *) p2p_shmalloc(sizeof(struct p2p_mon_queue) * i);
	if (m->qs == NULL)
	{
	    p2p_dprintf("OOPS! p2p_moninit: p2p_shmalloc failed ***\n");
	    return (-1);
	}
	for (j = 0; j < i; j++)
	{
	    q = m->qs + j;
	    q->count = 0;
            fprintf(stderr,"about to init lock at 0x%x\n",&(q->delay_lock));
	    p2p_lock_init(&(q->delay_lock));
	    /*****
            fprintf(stderr,"ret from p2p_lock_init on delay lock: %d\n",rc);
	    if (rc != &(q->delay_lock)  &&  rc <= 0)
		perror("bad ret from p2p_lock_init on delay_lock");
	    *****/
            fprintf(stderr,"about to lock lock at 0x%x\n",&(q->delay_lock));
	    p2p_lock(&(q->delay_lock));
            fprintf(stderr,"locked lock at 0x%x\n",&(q->delay_lock));
	}
    }
    else
	m->qs = NULL;
    return (0);
}

void p2p_menter(m)
p2p_monitor_t *m;
{
    p2p_lock(&(m->mon_lock));
}

void p2p_mexit(m)
p2p_monitor_t *m;
{
    p2p_unlock(&(m->mon_lock));
}

void p2p_mdelay(m, i)
p2p_monitor_t *m;
int i;
{
    struct p2p_mon_queue *q;

    q = m->qs + i;
    q->count++;
    p2p_unlock(&(m->mon_lock));
    p2p_lock(&(q->delay_lock));
}

void p2p_mcontinue(m, i)
p2p_monitor_t *m;
int i;
{
    struct p2p_mon_queue *q;

    q = m->qs + i;
    if (q->count)
    {
	q->count--;
	p2p_unlock(&(q->delay_lock));
    }
    else
    {
	p2p_unlock(&(m->mon_lock));
    }
}

int num_in_mon_queue(m, i)
int i;
p2p_monitor_t *m;
{
    struct p2p_mon_queue *q;

    q = m->qs + i;
    return (q->count);
}


/* ------------------  getsub monitor -------------------- */

int p2p_getsub_init(gs)
p2p_getsub_monitor_t *gs;
{
    int rc;

    gs->sub = 0;
    rc = p2p_moninit(&(gs->m),1);
    return(rc);
}

void p2p_getsubs(gs, s, max, nprocs, stride)
p2p_getsub_monitor_t *gs;
int *s, max, nprocs, stride;
{
    p2p_menter(&(gs->m));
    if (gs->sub <= max)
    {
	*s = gs->sub;
	gs->sub += stride;
	p2p_mexit(&(gs->m));
    }
    else
    {
	*s = -1;
	if (num_in_mon_queue(&(gs->m), 0) < nprocs - 1)
	    p2p_mdelay(&(gs->m), 0);
	else
	    gs->sub = 0;
	p2p_mcontinue(&(gs->m), 0);
    }
}


/* ------------------  barrier monitor -------------------- */

int p2p_barrier_init(b)
p2p_barrier_monitor_t *b;
{
    int rc;

    fprintf(stderr,"p2p_barrier_init: calling p2p_moninit\n");
    rc = p2p_moninit(&(b->m),1);
    return(rc);
}

void p2p_barrier(b, nprocs)
p2p_barrier_monitor_t *b;
int nprocs;
{
    p2p_menter(&(b->m));
    if (num_in_mon_queue(&(b->m), 0) < nprocs - 1)
	p2p_mdelay(&(b->m), 0);
    p2p_mcontinue(&(b->m), 0);
}


/* ------------------  askfor monitor -------------------- */

int p2p_askfor_init(af)
p2p_askfor_monitor_t *af;
{
    int rc;

    fprintf(stderr,"entering p2p_askfor_init\n");
    fprintf(stderr,"askfor monitor at 0x%x\n",af);
    af->pgdone = 0;
    af->pbdone = 0;
    fprintf(stderr,"p2p_askfor_init: calling p2p_moninit\n");
    /* alog assumes only one askfor per program */
    rc = p2p_moninit(&(af->m),1);
    return(rc);
}

int p2p_askfor(af, nprocs, getprob_fxn, problem, reset_fxn)
p2p_askfor_monitor_t *af;
int nprocs;
int (*getprob_fxn) ();
void *problem;
void(*reset_fxn) ();
{
    int rc;

    p2p_menter(&(af->m));
    if (!(af->pgdone) && af->pbdone)
    {
	if (num_in_mon_queue(&(af->m), 0) < nprocs - 1)
	{
	    p2p_mdelay(&(af->m), 0);
	}
    }
    else
    {
	while (!(af->pgdone) && !(af->pbdone))
	{
	    if ((rc = (*getprob_fxn) (problem)) == 0)
	    {
		p2p_mcontinue(&(af->m), 0);
		return (rc);
	    }
	    else
	    {
		if (num_in_mon_queue(&(af->m), 0) == nprocs - 1)
		{
		    af->pbdone = 1;
		}
		else
		{
		    p2p_mdelay(&(af->m), 0);
		}
	    }
	}
    }
    if (af->pgdone)
    {
	rc = (-1);
	p2p_mcontinue(&(af->m), 0);
    }
    else
    {
	rc = af->pbdone;
	if (num_in_mon_queue(&(af->m), 0) == 0)
	{
	    (*reset_fxn) ();
	    af->pbdone = 0;
	}
	p2p_mcontinue(&(af->m), 0);
    }
    return (rc);
}

void p2p_update(af, putprob_fxn, problem)
p2p_askfor_monitor_t *af;
int (*putprob_fxn) ();
void *problem;
{
    p2p_menter(&(af->m));
    if (putprob_fxn(problem))
	p2p_mcontinue(&(af->m), 0);
    else
	p2p_mexit(&(af->m));
}

void p2p_probend(af, code)
p2p_askfor_monitor_t *af;
int code;
{
    p2p_menter(&(af->m));
    af->pbdone = code;
    p2p_mexit(&(af->m));
}

void p2p_progend(af)
p2p_askfor_monitor_t *af;
{
    p2p_menter(&(af->m));
    af->pgdone = 1;
    p2p_mcontinue(&(af->m), 0);
}

int p2p_create(fxn)
int (*fxn) ();
{
    int rc;

    p2p_dprintf("creating local slave via fork\n");
    if ((rc = (int) fork()) == 0)
    {
	/* slave process */
	(*fxn) ();
	exit(0);
    }
    /* else master process */
    p2p_dprintf("created local slave via fork\n");
    return (rc);
}
