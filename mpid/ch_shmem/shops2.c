/* 
   This is sample code originally written for the Encore Multimax.
   The original may be found in ~gropp/fmmp/barrier.c 

   The intent is to make this available on general systems that 
   have shared memory or global address spaces.

  (note that on the Cray T3D, since the shmem_put and shmem_get operations
  are NOT cache-coherent, these may not work well on that system.

  This version is designed for use with P2.

  Systems with special support for shared memory can almost certainly do
  better than these by making use of system-specific information about 
  how memory is accessed (with particular reference to avoiding spin loops
  that block or impeed other processes from accessing memory.
 */

#include "p2p.h"

#ifndef NULL
#define NULL  0
#endif
#ifndef MALLOC 
#define MALLOC(a) malloc((unsigned)(a))
#endif
#ifndef FREE
#define FREE(a)   free((char *)a)
#endif

/* 
   LINESIZE is number of ints per cache line; experiments showed that on an
   SGI Onyx, this had little if any effect. 
   (but linesizes are LONG on SGI; should try 32 ints) 
 */
/* #define ILINESIZE 1 */
#define ILINESIZE 32
#if ILINESIZE > 1
#define DLINESIZE (ILINESIZE *sizeof(int)/sizeof(double))
#else
#define DLINESIZE 1
#endif
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
  volatile long   *lvalue, *mylvalue, *l1, *l2, *l3, *l4, *l5;
} MPID_Fastbar;

/* Spin on a variable.  backs off a little on failure.  Wait for *var == val */
#ifdef MPID_SIMPLE_SPIN
#define MPID_SPINWAIT(var,val) {\
    while (*(var) == (val)) ;\
    }
#else
#define MPID_SPINWAIT(var,val) {int cnt, backoff;\
    if (*(var) == (val)) {\
        backoff = 1;\
	while (*(var) == (val)) {\
	    cnt = backoff; while (cnt--); backoff = 2 * backoff;\
	    if (*(var) == (val)) p2p_yield();\
	    }\
	}\
    }
#endif
/*
    MPID_SHMEM_init_barrier returns a pointer to a MPID_Fastbar structure.
    flag says whether this process should do acquire the shared memory and
    broadcast it or not.  flag = 1 -> get the memory

    Note that as written, this must be called BEFORE the processes are
    created.  Otherwise, EACH process gets its own shared memory.
*/

void *MPID_SHMEM_Init_barrier(npf,flag)
int npf, flag;
{
    int i;
    MPID_Fastbar *bar;

    /* Check for too many processes */
    if (npf > 32) return 0;

    bar   = (MPID_Fastbar *) MALLOC(sizeof (MPID_Fastbar));
    bar->np = npf;
    if (flag)
    {
        /* 
	   We should probably change these sizes so that there is at most
	   one item per cache line.
         */
	bar->phase = (int *)    p2p_shmalloc(npf*ILINESIZE*sizeof(int));
	bar->value = (double *) p2p_shmalloc(npf*DLINESIZE*sizeof(double));
	bar->lvalue = (long *)  p2p_shmalloc(npf*DLINESIZE*sizeof(long));
	for (i = 0;  i < bar->np;  i++)
	  bar->phase[i] = 0;
	if (!bar->phase || !bar->value || !bar->lvalue) 
	    return 0;
    }
    return (void *) bar;
}

void MPID_SHMEM_Free_barrier(vbar)
void *vbar;
{
    MPID_Fastbar *bar = (MPID_Fastbar *)vbar;

    p2p_shfree( (char *)(bar->phase) );
    p2p_shfree( (char *)(bar->value) );
    p2p_shfree( (char *)(bar->lvalue) );
    FREE( bar );
}

MPID_SHMEM_Setup_barrier(bar,mypidf)
volatile MPID_Fastbar *bar;
int mypidf;
{
    int mypid,np;
    volatile int *phase;
    volatile double *value;
    volatile long *lvalue;

    mypid  = bar->mypid = mypidf;
    np	   = bar->np;
    phase  = bar->phase;
    value  = bar->value;
    lvalue = bar->lvalue;

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

    bar->mylvalue = &lvalue[DOFFSET(mypid)];
    bar->l1 = (mypid%2 == 0  &&  mypid+1 < np) ?  
	&lvalue[DOFFSET(mypid+1)] : NULL;
    bar->l2 = (mypid%4 == 0  &&  mypid+2 < np) ?  
	&lvalue[DOFFSET(mypid+2)] : NULL;
    bar->l3 = (mypid%8 == 0  &&  mypid+4 < np) ?  
	&lvalue[DOFFSET(mypid+4)] : NULL;
    bar->l4 = (mypid%16== 0  &&  mypid+8 < np) ?  
	&lvalue[DOFFSET(mypid+8)] : NULL;
    bar->l5 = (mypid%32== 0  &&  mypid+16< np) ?  
	&lvalue[DOFFSET(mypid+16)]: NULL;
}

MPID_SHMEM_Wait_barrier(bar)
volatile MPID_Fastbar *bar;
{
    register int oldphase;
    
    oldphase = *(bar->myphase);
    if (bar->p1)  {MPID_SPINWAIT(bar->p1,oldphase) ;
    if (bar->p2)  {MPID_SPINWAIT(bar->p2,oldphase) ;
    if (bar->p3)  {MPID_SPINWAIT(bar->p3,oldphase) ;
    if (bar->p4)  {MPID_SPINWAIT(bar->p4,oldphase) ;
    if (bar->p5)  {MPID_SPINWAIT(bar->p5,oldphase) ; }}}}}
    ++(*(bar->myphase));
    MPID_SPINWAIT(bar->phase,oldphase) ;
}

double MPID_SHMEM_Barrier_sum_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double sum;

    oldphase = *(bar->myphase);         sum = *x;
    if (bar->p1)  {MPID_SPINWAIT(bar->p1,oldphase) ; sum += *(bar->v1);
    if (bar->p2)  {MPID_SPINWAIT(bar->p2,oldphase) ; sum += *(bar->v2);
    if (bar->p3)  {MPID_SPINWAIT(bar->p3,oldphase) ; sum += *(bar->v3);
    if (bar->p4)  {MPID_SPINWAIT(bar->p4,oldphase) ; sum += *(bar->v4);
    if (bar->p5)  {MPID_SPINWAIT(bar->p5,oldphase) ; sum += *(bar->v5); }}}}}
    *(bar->myvalue) = sum;  ++(*(bar->myphase));
    MPID_SPINWAIT(bar->phase,oldphase) ;
    return (*(bar->value));
}

double MPID_SHMEM_Barrier_max_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double max;

    oldphase = *(bar->myphase);                max = *x;
    if (bar->p1)  {MPID_SPINWAIT(bar->p1,oldphase) ; 
		   if (max < *(bar->v1)) max = *(bar->v1);
    if (bar->p2)  {MPID_SPINWAIT(bar->p2,oldphase) ; 
		   if (max < *(bar->v2)) max = *(bar->v2);
    if (bar->p3)  {MPID_SPINWAIT(bar->p3,oldphase) ; 
		   if (max < *(bar->v3)) max = *(bar->v3);
    if (bar->p4)  {MPID_SPINWAIT(bar->p4,oldphase) ; 
		   if (max < *(bar->v4)) max = *(bar->v4);
    if (bar->p5)  {MPID_SPINWAIT(bar->p5,oldphase) ; 
		   if (max < *(bar->v5)) max = *(bar->v5); }}}}}
    *(bar->myvalue) = max;  ++(*(bar->myphase));
    MPID_SPINWAIT(bar->phase,oldphase) ;
    return (*(bar->value));
}

double MPID_SHMEM_Barrier_min_double(bar,x)
MPID_Fastbar *bar;
double *x;
{
    register int oldphase;
    register double min;

    oldphase = *(bar->myphase);                min = *x;
    if (bar->p1)  {MPID_SPINWAIT(bar->p1,oldphase) ; 
		   if (min > *(bar->v1)) min = *(bar->v1);
    if (bar->p2)  {MPID_SPINWAIT(bar->p2,oldphase) ; 
		   if (min > *(bar->v2)) min = *(bar->v2);
    if (bar->p3)  {MPID_SPINWAIT(bar->p3,oldphase) ; 
		   if (min > *(bar->v3)) min = *(bar->v3);
    if (bar->p4)  {MPID_SPINWAIT(bar->p4,oldphase) ; 
		   if (min > *(bar->v4)) min = *(bar->v4);
    if (bar->p5)  {MPID_SPINWAIT(bar->p5,oldphase) ; 
		   if (min > *(bar->v5)) min = *(bar->v5); }}}}}
    *(bar->myvalue) = min;  ++(*(bar->myphase));
    MPID_SPINWAIT(bar->phase,oldphase) ;
    return (*(bar->value));
}

/* Broadcast a long from the root = 0 */
/* This code is WRONG */
long MPID_SHMEM_Bcast_long(bar, x)
MPID_Fastbar *bar;
long         *x;
{
    register int oldphase;
    register long data;

    oldphase = *(bar->myphase);             *(bar->mylvalue) = *x;
    /* At the first position, wait for the parent to place the data into
       my local location, then read it and propogate it down */
    if (bar->p1)  {MPID_SPINWAIT(bar->p1,oldphase) ; 
		   data = *(bar->mylvalue); *(bar->l1) = data;
    if (bar->p2)  {MPID_SPINWAIT(bar->p2,oldphase) ; *(bar->l2) = data;
    if (bar->p3)  {MPID_SPINWAIT(bar->p3,oldphase) ; *(bar->l3) = data;
    if (bar->p4)  {MPID_SPINWAIT(bar->p4,oldphase) ; *(bar->l4) = data;
    if (bar->p5)  {MPID_SPINWAIT(bar->p5,oldphase) ; *(bar->l5) = data; }}}}}
    ++*(bar->myphase);
    MPID_SPINWAIT(bar->phase,oldphase) ;
    return (*(bar->mylvalue));
}



/* Example usage */
#ifdef BUILD_MAIN

typedef long MPID_PKT_T;
#include "channel.h"
VOLATILE MPID_SHMEM_globmem *MPID_shmem;

main(argc,argv)
int argc;
char **argv;
{
    int i, j, n, myid, start, end;
    double x, ymin, ymax, ysum;
    double starttime, endtime, onetime, manytime, manyp4time, bcasttime;
    double maxtime, mintime, sumtime, locktime;
    void *bar;
    long l;

    if (argc != 2)
        p2p_error("must indicate total # procs on cmd line",(-1));
    else
        n = atoi(argv[1]);

    p2p_init( n, MPID_MAX_SHMEM );
    MPID_shmem = p2p_shmalloc( sizeof( MPID_SHMEM_globmem ) );
    MPID_shmem->globid = 0;
    p2p_lock_init( &MPID_shmem->globlock );

    bar = MPID_SHMEM_Init_barrier(n,1);
    if (!bar) {
	fprintf( stderr, "Could not allocate barrier structure\n" );
	return 1;
	}
    p2p_create_procs( n - 1 );
    
    p2p_lock( &MPID_shmem->globlock );
    myid = MPID_shmem->globid++;
    p2p_unlock( &MPID_shmem->globlock );

    x = (double) myid;

    /* Initialize the barriers */
    /* bar = MPID_SHMEM_Init_barrier(n,1); */
    MPID_SHMEM_Setup_barrier(bar,myid);

    MPID_SHMEM_Wait_barrier(bar);
    if (myid == 0)
	printf( "Starting lock test\n" );
    starttime = p2p_wtime();
    for (i=0; i<1000; i++) {
	p2p_lock( &MPID_shmem->globlock );
	p2p_unlock( &MPID_shmem->globlock );
	}
    endtime = p2p_wtime();
    locktime = endtime - starttime;
    if (myid == 0)
	printf( "Completed lock test\n" );

    MPID_SHMEM_Wait_barrier(bar);
    starttime = p2p_wtime();
    for (i = 0; i < 1000; i++) {
/* 	printf( "[%d] starting barrier\n",myid ); fflush(stdout); */
      MPID_SHMEM_Wait_barrier(bar);
      }
    endtime = p2p_wtime();
    manytime = endtime - starttime;
    if (myid == 0) 
	printf( "Completed barrier test\n" );

    if (myid == 0) 
	printf( "Starting reduce test\n" );
    MPID_SHMEM_Wait_barrier(bar);
    starttime = p2p_wtime();
    for (i = 0; i < 1000; i++)
      ysum = MPID_SHMEM_Barrier_sum_double(bar,&x);
    endtime = p2p_wtime();
    sumtime = endtime - starttime;

    /* Test broadcast */
    if (myid == 0) 
	l = 0x1234efab;
    else 
	l = 0xefab1234;
    l = MPID_SHMEM_Bcast_long( bar, &l );
    if (l != 0x1234efab) {
	fprintf( stderr, "[%d] got %d for bcast; expected %d\n", 
		 myid, l, 0x1234efab );
	}
    MPID_SHMEM_Wait_barrier(bar);
    starttime = p2p_wtime();
    for (i=0; i<1000; i++)
	MPID_SHMEM_Free_barrier( bar );
    endtime = p2p_wtime();
    bcasttime = endtime - starttime;
    p2p_cleanup();

    if (myid == 0)
      {
        printf("time for 1000 locks = %f seconds\n",locktime);
	if (locktime > 0) 
	    printf( " (rate = %f ms / lock)\n", locktime );
        printf("time for 1000 barriers = %f seconds\n",manytime);
	if (manytime > 0) 
	    printf( " (rate = %f ms / barrier)\n", manytime );
	printf("time for 1000 sums (%f) =  %f seconds\n",ysum,sumtime);
	if (manytime > 0) 
	    printf( " (rate = %f ms / reduction)\n", sumtime );
        printf("time for 1000 bcasts = %f seconds\n", bcasttime );
	if (bcasttime > 0)
	    printf( " (rate = %f ms / bcast)\n", bcasttime );
/*
	printf("time for 1000 maxs (%f) =  %f seconds\n",ymax,maxtime);
	printf("time for 1000 mins (%f) =  %f seconds\n",ymin,mintime);
*/
      }
return 0;
}
#endif





