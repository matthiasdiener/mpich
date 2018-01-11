/* 
   This is sample code originally written for the Encore Multimax.
   The original may be found in ~gropp/fmmp/barrier.c 

   The intent is to make this available on general systems that 
   have shared memory or global address spaces.

  (note that one the Cray T3D, since the shmem_put and shmem_get operations
  are NOT cache-coherent, these may not work well on that system)
 */
#include <parallel.h>
/* change to the new (libpp) names */

#define NULL  0
#define NPMAX 20
static int np, mypid;
static volatile int    *phase, *myphase, *p1,*p2,*p3,*p4,*p5;
static volatile double *value, *myvalue, *v1,*v2,*v3,*v4,*v5;
static volatile long   *lvalue, *mylvalue, *l1, *l2, *l3, *l4, *l5;

initbar_ (npf)
int *npf;
{
    int i;

    np	   = *npf;
    phase  = (int *)    share (0, NPMAX*sizeof(int));
    value  = (double *) share (0, NPMAX*sizeof(double));
    lvalue = (long *)  share (0, NPMAX*sizeof(long));
    for (i = 0;  i < np;  i++)
	phase[i] = 0;
}

/*
   This works by setting {p/v/l} to the address of a leaf if 
   the appropriate bit is EVEN and to NULL otherwise.
   Thus, the root (id == 0) has log(p) valid addresses and the odd nodes
   have none.
 */
pidbar_ (mypidf)
int *mypidf;
{
    mypid = *mypidf - 1;
    myphase = &phase[mypid];
    p1 = (mypid%2 == 0  &&  mypid+1 < np) ?  &phase[mypid+1] : NULL;
    p2 = (mypid%4 == 0  &&  mypid+2 < np) ?  &phase[mypid+2] : NULL;
    p3 = (mypid%8 == 0  &&  mypid+4 < np) ?  &phase[mypid+4] : NULL;
    p4 = (mypid%16== 0  &&  mypid+8 < np) ?  &phase[mypid+8] : NULL;
    p5 = (mypid%32== 0  &&  mypid+16< np) ?  &phase[mypid+16]: NULL;
    myvalue = &value[mypid];
    v1 = (mypid%2 == 0  &&  mypid+1 < np) ?  &value[mypid+1] : NULL;
    v2 = (mypid%4 == 0  &&  mypid+2 < np) ?  &value[mypid+2] : NULL;
    v3 = (mypid%8 == 0  &&  mypid+4 < np) ?  &value[mypid+4] : NULL;
    v4 = (mypid%16== 0  &&  mypid+8 < np) ?  &value[mypid+8] : NULL;
    v5 = (mypid%32== 0  &&  mypid+16< np) ?  &value[mypid+16]: NULL;
    mylvalue = &lvalue[mypid];
    l1 = (mypid%2 == 0  &&  mypid+1 < np) ?  &lvalue[mypid+1] : NULL;
    l2 = (mypid%4 == 0  &&  mypid+2 < np) ?  &lvalue[mypid+2] : NULL;
    l3 = (mypid%8 == 0  &&  mypid+4 < np) ?  &lvalue[mypid+4] : NULL;
    l4 = (mypid%16== 0  &&  mypid+8 < np) ?  &lvalue[mypid+8] : NULL;
    l5 = (mypid%32== 0  &&  mypid+16< np) ?  &lvalue[mypid+16]: NULL;
}

waitbar_()
{
    register int oldphase;

    oldphase = *myphase;
    if (p1)  {while (*p1 == oldphase) ;
    if (p2)  {while (*p2 == oldphase) ;
    if (p3)  {while (*p3 == oldphase) ;
    if (p4)  {while (*p4 == oldphase) ;
    if (p5)  {while (*p5 == oldphase) ; }}}}}
    ++*myphase;
    while (*phase == oldphase) ;
}

double sumbar_(x)
double *x;
{
    register int oldphase;
    register double sum;

    oldphase = *myphase;                sum = *x;
    if (p1)  {while (*p1 == oldphase) ; sum += *v1;
    if (p2)  {while (*p2 == oldphase) ; sum += *v2;
    if (p3)  {while (*p3 == oldphase) ; sum += *v3;
    if (p4)  {while (*p4 == oldphase) ; sum += *v4;
    if (p5)  {while (*p5 == oldphase) ; sum += *v5; }}}}}
    *myvalue = sum;  ++*myphase;
    while (*phase == oldphase) ;
    return (*value);
}

double maxbar_(x)
double *x;
{
    register int oldphase;
    register double max;

    oldphase = *myphase;                max = *x;
    if (p1)  {while (*p1 == oldphase) ; if (max < *v1) max = *v1;
    if (p2)  {while (*p2 == oldphase) ; if (max < *v2) max = *v2;
    if (p3)  {while (*p3 == oldphase) ; if (max < *v3) max = *v3;
    if (p4)  {while (*p4 == oldphase) ; if (max < *v4) max = *v4;
    if (p5)  {while (*p5 == oldphase) ; if (max < *v5) max = *v5; }}}}}
    *myvalue = max;  ++*myphase;
    while (*phase == oldphase) ;
    return (*value);
}

double minbar_(x)
double *x;
{
    register int oldphase;
    register double min;

    oldphase = *myphase;                min = *x;
    if (p1)  {while (*p1 == oldphase) ; if (min > *v1) min = *v1;
    if (p2)  {while (*p2 == oldphase) ; if (min > *v2) min = *v2;
    if (p3)  {while (*p3 == oldphase) ; if (min > *v3) min = *v3;
    if (p4)  {while (*p4 == oldphase) ; if (min > *v4) min = *v4;
    if (p5)  {while (*p5 == oldphase) ; if (min > *v5) min = *v5; }}}}}
    *myvalue = min;  ++*myphase;
    while (*phase == oldphase) ;
    return (*value);
}

/* Broadcast a long from the root = 0 */
long bcastbar_(x)
long *x;
{
    register int oldphase;
    register long data;

    oldphase = *myphase;                *mylvalue = *x;
    /* At the first possition, wait for the parent to place the data into
       my local location, then read it and propogate it down */
    if (p1)  {while (*p1 == oldphase) ; data = *mylvalue; *v1 = data;
    if (p2)  {while (*p2 == oldphase) ; *v2 = data;
    if (p3)  {while (*p3 == oldphase) ; *v3 = data;
    if (p4)  {while (*p4 == oldphase) ; *v4 = data;
    if (p5)  {while (*p5 == oldphase) ; *v5 = data; }}}}}
    ++*myphase;
    while (*phase == oldphase) ;
    return (*mylvalue);
}


