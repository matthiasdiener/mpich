/* 
   This is sample code originally written for the Encore Multimax.
   The original may be found in ~gropp/fmmp/barrier.c 

   The intent is to make this available on general systems that 
   have shared memory or global address spaces.

  (note that one the Cray T3D, since the shmem_put and shmem_get operations
  are NOT cache-coherent, these may not work well on that system)

  This version is designed for use with P4.

 */

#include "p4.h"

#define NULL  0

/* LINESIZE is number of ints per cache line; experiments showed that on an
   SGI Onyx, this had little if any effect. */
#define ILINESIZE 1
#define DLINESIZE (ILINESIZE *sizeof(int)/sizeof(double))
#define IOFFSET(a) ((a)*ILINESIZE)
#define DOFFSET(a) ((a)*DLINESIZE)
#define NPMAX 32

#ifndef __STDC__
#define volatile 
#endif

typedef struct {
  int np, mypid;
  volatile int    *phase, *myphase, *p1,*p2,*p3,*p4,*p5;
  volatile double *value, *myvalue, *v1,*v2,*v3,*v4,*v5;
} MPID_Fastbar;

/*
    MPID_CH_init_barrier returns a pointer to a MPID_Fastbar structure.
    flag says whether this process should do acquire the shared memory and
    broadcast it or not.  flag = 1 -> get the memory
*/

void *MPID_CH_init_barrier(npf,flag)
int *npf, flag;
{
    int i;
    MPID_Fastbar *bar;

    bar   = (MPID_Fastbar *) p4_malloc(sizeof (MPID_Fastbar));
    bar->np = *npf;
    if (flag)
    {
        /* 
	   We should probably change these sizes so that there is at most
	   one item per cache line.
         */
	bar->phase = (int *)    p4_shmalloc(NPMAX*ILINESIZE*sizeof(int));
	bar->value = (double *) p4_shmalloc(NPMAX*DLINESIZE*sizeof(double));
	for (i = 0;  i < bar->np;  i++)
	  bar->phase[i] = 0;
    }
    return (void *) bar;
}

MPID_CH_Setup_barrier(bar,mypidf)
MPID_Fastbar *bar;
int *mypidf;
{
    int mypid,np;
    volatile int *phase;
    volatile double *value;

    mypid = bar->mypid = *mypidf;
    np    = bar->np;
    phase = bar->phase;
    value = bar->value;

    bar->myphase = &phase[IOFFSET(mypid)];
    bar->p1 = (mypid%2 == 0  &&  mypid+1 < np) ?  
	&phase[IOFFSET(mypid+1)] : NULL;
    bar->p2 = (mypid%4 == 0  &&  mypid+2 < np) ?  
	&phase[IOFFSET(mypid+2)] : NULL;
    bar->p3 = (mypid%8 == 0  &&  mypid+4 < np) ?  
	&phase[IOFFSET(mypid+4)] : NULL;
    bar->p4 = (mypid%16== 0  &&  mypid+8 < np) ?  
	&phase[IOFFSET(mypid+8)] : NULL;
    bar->p5 = (mypid%32== 0  &&  mypid+16< np) ?  
	&phase[IOFFSET(mypid+16)]: NULL;
    bar->myvalue = &value[DOFFSET(mypid)];
    bar->v1 = (mypid%2 == 0  &&  mypid+1 < np) ?  
	&value[DOFFSET(mypid+1)] : NULL;
    bar->v2 = (mypid%4 == 0  &&  mypid+2 < np) ?  
	&value[DOFFSET(mypid+2)] : NULL;
    bar->v3 = (mypid%8 == 0  &&  mypid+4 < np) ?  
	&value[DOFFSET(mypid+4)] : NULL;
    bar->v4 = (mypid%16== 0  &&  mypid+8 < np) ?  
	&value[DOFFSET(mypid+8)] : NULL;
    bar->v5 = (mypid%32== 0  &&  mypid+16< np) ?  
	&value[DOFFSET(mypid+16)]: NULL;
}

MPID_CH_Wait_barrier(bar)
MPID_Fastbar *bar;
{
    register int oldphase;
    
    oldphase = *(bar->myphase);
    if (bar->p1)  {while (*(bar->p1) == oldphase) ;
    if (bar->p2)  {while (*(bar->p2) == oldphase) ;
    if (bar->p3)  {while (*(bar->p3) == oldphase) ;
    if (bar->p4)  {while (*(bar->p4) == oldphase) ;
    if (bar->p5)  {while (*(bar->p5) == oldphase) ; }}}}}
    ++(*(bar->myphase));
    while (*(bar->phase) == oldphase) ;
}

double MPID_CH_Barrier_sum_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double sum;

    oldphase = *(bar->myphase);         sum = *x;
    if (bar->p1)  {while (*(bar->p1) == oldphase) ; sum += *(bar->v1);
    if (bar->p2)  {while (*(bar->p2) == oldphase) ; sum += *(bar->v2);
    if (bar->p3)  {while (*(bar->p3) == oldphase) ; sum += *(bar->v3);
    if (bar->p4)  {while (*(bar->p4) == oldphase) ; sum += *(bar->v4);
    if (bar->p5)  {while (*(bar->p5) == oldphase) ; sum += *(bar->v5); }}}}}
    *(bar->myvalue) = sum;  ++(*(bar->myphase));
    while (*(bar->phase) == oldphase) ;
    return (*(bar->value));
}

double MPID_CH_Barrier_max_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double max;

    oldphase = *(bar->myphase);                max = *x;
    if (bar->p1)  {while (*(bar->p1) == oldphase) ; 
		   if (max < *(bar->v1)) max = *(bar->v1);
    if (bar->p2)  {while (*(bar->p2) == oldphase) ; 
		   if (max < *(bar->v2)) max = *(bar->v2);
    if (bar->p3)  {while (*(bar->p3) == oldphase) ; 
		   if (max < *(bar->v3)) max = *(bar->v3);
    if (bar->p4)  {while (*(bar->p4) == oldphase) ; 
		   if (max < *(bar->v4)) max = *(bar->v4);
    if (bar->p5)  {while (*(bar->p5) == oldphase) ; 
		   if (max < *(bar->v5)) max = *(bar->v5); }}}}}
    *(bar->myvalue) = max;  ++(*(bar->myphase));
    while (*(bar->phase) == oldphase) ;
    return (*(bar->value));
}

double MPID_CH_Barrier_min_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double min;

    oldphase = *(bar->myphase);                min = *x;
    if (bar->p1)  {while (*(bar->p1) == oldphase) ; 
		   if (min > *(bar->v1)) min = *(bar->v1);
    if (bar->p2)  {while (*(bar->p2) == oldphase) ; 
		   if (min > *(bar->v2)) min = *(bar->v2);
    if (bar->p3)  {while (*(bar->p3) == oldphase) ; 
		   if (min > *(bar->v3)) min = *(bar->v3);
    if (bar->p4)  {while (*(bar->p4) == oldphase) ; 
		   if (min > *(bar->v4)) min = *(bar->v4);
    if (bar->p5)  {while (*(bar->p5) == oldphase) ; 
		   if (min > *(bar->v5)) min = *(bar->v5); }}}}}
    *(bar->myvalue) = min;  ++(*(bar->myphase));
    while (*(bar->phase) == oldphase) ;
    return (*(bar->value));
}


/* Example usage */
#ifdef BUILD_MAIN
main(argc,argv)
int argc;
char **argv;
{
    int i, j, n, myid, start, end;
    double x, ymin, ymax, ysum;
    unsigned long int starttime, endtime, onetime, manytime, manyp4time;
    unsigned long int maxtime, mintime, sumtime;
    void *bar;

    p4_initenv(&argc,argv);

    if (argc != 2)
        p4_error("must indicate total # procs on cmd line",(-1));
    else
        n = atoi(argv[1]);

    /* Create the initial area BEFORE spawning the processes */
    bar = MPID_CH_init_barrier(&n);

    p4_create_procgroup();
    myid = p4_get_my_id();
    x = (double) myid;
    /* Initialize the barriers */
    MPID_CH_Setup_barrier(bar,&myid);
    if (n != p4_num_cluster_ids())
        p4_error("number of procs mismatch",(-1));

    if (p4_get_my_id() == 100)
     sleep(5);

    starttime = p4_ustimer();
    for (i = 0; i < 1000; i++)
      MPID_CH_Wait_barrier(bar);
    endtime = p4_ustimer();
    manytime = endtime - starttime;

    starttime = p4_ustimer();
    for (i = 0; i < 1000; i++)
      ysum = MPID_CH_Barrier_sum_double(bar,&x);
    endtime = p4_ustimer();
    sumtime = endtime - starttime;

    p4_wait_for_end();

    if (myid == 0)
      {
        printf("time for 1000 barriers = %d microseconds\n",manytime);
	printf("time for 1000 sums (%f) =  %d microseconds\n",ysum,sumtime);
/*
	printf("time for 1000 maxs (%f) =  %d microseconds\n",ymax,maxtime);
	printf("time for 1000 mins (%f) =  %d microseconds\n",ymin,mintime);
*/
      }

}
#endif
