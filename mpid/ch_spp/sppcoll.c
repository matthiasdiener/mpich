/*
 *  $Id: chcoll.c,v 1.4 1995/01/03 19:40:12 gropp Exp gropp $
 *
 *  (C) 1993 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 */

#ifndef lint
static char vcid[] = "$Id$";
#endif

/* 
    This file contains the routines that provide the basic information 
    on the device, and initialize it.  

    ** UNTESTED **
 */

#include "p2p.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/cnx_ail.h>
#include "mpid.h"
#include "mpiimpl.h"
#include "mpisys.h"
#include "sppfastcoll.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int MPIR_Pack2();
extern int MPIR_Unpack2();
extern int MPIR_Pack_size();

static int MPID_SPP_iceil(double x);
MPID_Fastbar *MPID_SPP_Alloc_barrier();

static void MPID_SPP_waiteq();
static void MPID_SPP_waitneq();
extern void MPID_SPP_post0();
extern void MPID_SPP_post1();

#define MPID_SPP_waiteq(flag) { while (!*flag); }
#define MPID_SPP_waitneq(flag) { while (*flag !=0); }
#define BCOPY(s1,s2,n) pre_bcopy((unsigned int)( s1 ),(unsigned int)( s2 ),n)
extern int MPID_SPP_CNX_SAME_NODE;

/* 
   These routines provide hooks for the ADI - collective routines.

*/


double MPID_SPP_Barrier_sum_double();

/* This uses the Convex SPP interface by Stephen Fleischman */
int MPID_SPP_Comm_init( comm, newcomm )
MPI_Comm comm, newcomm;
{
/* won't do anything here do it all in first barrier */
newcomm->ADIBarrier = 0;
newcomm->ADIReduce  = 0;
newcomm->ADIScan    = 0;
newcomm->ADIBcast   = 0;
newcomm->ADICollect = 0;

return MPI_SUCCESS;
}    
int MPID_SPP_Comm_free( comm )
MPI_Comm comm;
{
if (comm->ADIBarrier)
    MPID_SPP_Free_barrier( comm);
if (comm->ADICollect)
    MPID_SPP_Free_collect( comm);

return MPI_SUCCESS;
}

int MPID_SPP_Barrier( comm ) 
MPI_Comm comm;
{
    MPI_Comm hcomm;
    MPID_Fastbar *bar;
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;
    if(hcomm->ADIBarrier)
        MPID_SPP_Wait_barrier(hcomm);
    else
        MPID_SPP_First_barrier(hcomm);
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    return MPI_SUCCESS;
}

void MPID_SPP_Reduce_sum_int( sendbuf, recvbuf, comm )
int *sendbuf, *recvbuf;
MPI_Comm comm;
{
}

void MPID_SPP_Reduce_sum_double( sendbuf, recvbuf, comm )
double *sendbuf, *recvbuf;
MPI_Comm comm;
{
/**recvbuf = MPID_SPP_Barrier_sum_double( comm->ADIReduce, sendbuf );*/
}

int MPID_SPP_Wait_barrier(comm)
MPI_Comm comm;
{
   int nproc,me;
   int ncycles,twok,k,modme,other;
   MPID_Fastbar *bar;
   volatile int *flag;

   bar = (MPID_Fastbar *)(comm->ADIBarrier);
   if(bar->np <2) return;

   ncycles = bar->nc;
   nproc = bar->np;
   me = bar->mypid;

   twok=1;
   for (k=0;k<ncycles;k++){
      if((modme = me%(twok*2))==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_post0(flag);
      }
      else if(modme==0 && (other=me+twok)<nproc) {
         flag = bar->barf[other].flag;
         MPID_SPP_waitneq(flag);
      }
      twok*=2;
   }
   for (k=0;k<ncycles;k++){
      twok/=2;
      if((modme = me%(twok*2))==0 && (other=me+twok) <nproc) {
         flag = bar->barf[other].flag;
         MPID_SPP_post1(flag);
      }
      else if(modme==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_waiteq(flag);
      }
   }
   return 0;
}
MPID_Fastbar *MPID_SPP_Alloc_barrier(comm)
MPI_Comm comm;
{
    /* alloc barrier data structure and attach it to communicator */
    MPID_Fastbar *bar;

    if( comm->ADIBarrier!=NULL) return (MPID_Fastbar *)comm->ADIBarrier;

    if(!(bar   = (MPID_Fastbar *) MALLOC(sizeof(MPID_Fastbar)))){
        (*MPID_ErrorHandler)( 1, 
        "No more memory for storing barrier in MPID_SPP_Alloc_barrier");
        return (MPID_Fastbar *)NULL;
    }

    /* this is really Convex SPP dependent get the hypernode*/
    bar->myhypernode = (int )MPID_SPP_getNodeId();

    /* get my barrier flag space from global memory */
    if(!(bar->barrier = (void *) p2p_shmalloc(CACHELINESIZE,bar->myhypernode))){
        (*MPID_ErrorHandler)( 1, 
        "No more global memory for storing barrier flags in MPID_SPP_Alloc_barrier");
         return (MPID_Fastbar *)NULL;
    }
    comm->ADIBarrier=(void *)bar;
    bar->mypid = comm->local_group->local_rank;
    bar->np = comm->local_group->np;
    /* barrier flag array */
    if(!(bar->barf  = (MPID_Barf *) MALLOC(bar->np*sizeof(MPID_Barf)))){
        (*MPID_ErrorHandler)( 1, 
        "No more memory for storing barrier flag array in MPID_SPP_Alloc_barrier");
         return (MPID_Fastbar *)NULL;
    }
    bar->barf[bar->mypid].flag = (int *)bar->barrier;
    *(bar->barf[bar->mypid].flag) = 1;
    bar->nc =MPID_SPP_iceil(log( (double)bar->np)/log(2.0));
    return bar;
}

void MPID_SPP_Free_barrier(comm)
MPI_Comm comm;
{
    MPID_Fastbar *bar;
    /* synchronize first*/
    /* just free my own global space */
    bar = (MPID_Fastbar *)(comm->ADIBarrier);
    p2p_shfree( bar->barrier);
    FREE(bar->barf);
    FREE(bar);
}

void MPID_SPP_Free_collect(comm)
MPI_Comm comm;
{
    MPID_Fastcoll *coll;

    if((coll = (MPID_Fastcoll *)(comm->ADICollect))) {
       FREE(coll->blockcounts);
       FREE(coll->displs);
       FREE(coll);
    }
}


void for_cxdb()
{
   return;
}
extern FILE *fp;
int MPID_SPP_Reduce_scatter ( sendbuf, recvbuf, recvcnts, datatype, op, comm )
void             *sendbuf;
void             *recvbuf;
int              *recvcnts;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
    MPI_Comm hcomm;
    int   rank, size, i, count=0;
    int   nbytes,nbytes0,nbytes1,nbytes2;
    void *s1,*s2;
    MPI_Aint extent;
    int   mpi_errno = MPI_SUCCESS;
    int   flag;
    MPI_User_function *uop;
    MPID_Fastbar *bar;
    int istart,jproc,isshared=0,inplace;

    /* Determine the "count" of items to reduce and set the displacements*/
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;

    MPI_Comm_size(hcomm,&size);
    MPI_Type_extent (datatype, &extent);
    MPI_Comm_rank   (hcomm, &rank);
  

    /* do first barrier to exchange addresses if necessary */
    if (!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);
    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);

    uop  = op->op;
  
    /* Warning!!!! Doesn't handle non-contiguous datatypes correctly.
                   But then again, neither does the default MPICH
                   implementation. */ 
  
    inplace=((void *)recvbuf==NULL);

    if (size < 2) {
        if(!inplace) {
            count = extent*recvcnts[0];
            BCOPY(sendbuf,recvbuf,count);
        }
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    count = 0;
    for (i=0;i<rank;i++) count += recvcnts[i];
    istart = count*extent;
  
    nbytes0 = count*extent;
  
    for (i=rank;i<size;i++) count +=recvcnts[i];
    nbytes=count*extent;
    if(nbytes<=0) return MPI_SUCCESS; 

    nbytes1 = nbytes - (nbytes0 + recvcnts[rank]*extent);
  
    
    if ((isshared = (p2p_shnode(sendbuf)>=0))) {
       *(bar->barf[rank].addr)=sendbuf;
    }
    else {
       *(bar->barf[rank].addr)=(void *)p2p_shmalloc(nbytes,bar->myhypernode);
    }
    *(bar->barf[rank].ival)=bar->myhypernode;
    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */

  
    /* set count and nbytes to the amount in my chunk */
    count = recvcnts[rank];
    nbytes2=count*extent;
    if(!isshared) {
        if (op->commute) {
           /* copy the piece before my chunk into the buffer */
           if(nbytes0>0) {
               s1 = (void *)sendbuf;
               s2 = (void *)*(bar->barf[rank].addr);
               BCOPY(s1,s2,nbytes0);
           }
           /* copy the piece after my chunk into the buffer */
           if(nbytes1>0) {
               s1 = (void *)((char *)(sendbuf)+istart+nbytes2);
               s2 = (void *)((char *)(*(bar->barf[rank].addr))+istart+nbytes2);
               BCOPY(s1,s2,nbytes1);
           }
        } 
        else {
           BCOPY((void *)sendbuf,(void *)(*(bar->barf[rank].addr)),
                              nbytes);
        } 
    }
    /* If not doing in-place copy my chunk to recvbuf.
       We detect the use of the in-place extension by whether or not
       recvbuf is NULL. This is non-standard. */
       
    if (op->commute && !inplace) {
        s2 = (void *)recvbuf;
        s1 = (void *)((char *)sendbuf + istart);
        if(nbytes2>0) BCOPY(s1,s2,nbytes2);
    }
    /* flush the piece before my chunk into the buffer */
    if(nbytes0>0) {
        s1 = (void *)*(bar->barf[rank].addr);
        dcache_flush_region(s1, nbytes0);
    }
    /* flush the piece after my chunk into the buffer */
    if(nbytes1>0) {
        s1 = (void *)((char *)(*(bar->barf[rank].addr))+istart+nbytes2);
        dcache_flush_region(s1, nbytes1);
    }
  
    nbytes=nbytes2;
  
    MPID_SPP_Wait_barrier(hcomm);
    /* If recvbuf is NULL, we are using the in-place extension. */
    s2 = (!inplace)? recvbuf : (void *)((char *)sendbuf + istart);
    if (op->commute) {
       for (jproc=rank+1;jproc<size;jproc++) {
           MPID_SPP_CNX_SAME_NODE=
                   ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
           s1 = (void *)((char *)(*(bar->barf[jproc].addr)) + istart);
           (*uop)(s1, s2, &count, &datatype);
           /* flush read only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
           dcache_flush_region(s1, nbytes);
       }
       for (jproc=0;jproc<rank;jproc++){
           MPID_SPP_CNX_SAME_NODE=
                ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
           s1 = (void *)((char *)(*(bar->barf[jproc].addr)) + istart);
           (*uop)(s1, s2, &count, &datatype);
           /* flush read only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
           dcache_flush_region(s1, nbytes);
       }
       MPID_SPP_Wait_barrier(hcomm);
    }
    else {
       for (jproc=1;jproc<size;jproc++)
           if(jproc!=0) (*uop)(
                  (void *)((char *)(*(bar->barf[jproc].addr)) + istart),
                  (void *)((char *)(*(bar->barf[0].addr))     + istart),
                  &count, &datatype);
       MPID_SPP_Wait_barrier(hcomm);
       if(nbytes>0) BCOPY((void *)((char *)(*(bar->barf[0].addr))+istart),
                          s2, nbytes);
    }
  
    MPID_SPP_CNX_SAME_NODE= 1; /* reset */
    if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
  
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    return (mpi_errno);
}

int MPID_SPP_Reduce_scatterv (sendbuf, sendcnts, displs, datatype,
    recvbuf, recvcnt, op, comm)
void             *sendbuf;
int              *sendcnts;
int              *displs;
MPI_Datatype      datatype;
void             *recvbuf;
int               recvcnt;
MPI_Op            op;
MPI_Comm          comm;
{
    MPI_Comm hcomm;
    int   rank, size, i, count=0;
    int   nbytes,nbytes0,nbytes1,nbytes2;
    void *s1,*s2;
    MPI_Aint extent;
    int   mpi_errno = MPI_SUCCESS;
    int   flag;
    MPI_User_function *uop;
    MPID_Fastbar *bar;
    int istart,jproc,isshared=0;
    int inplace;

    /* Determine the "count" of items to reduce and set the displacements*/
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;
    MPI_Comm_size(hcomm,&size);
    MPI_Type_extent (datatype, &extent);
    MPI_Comm_rank   (hcomm, &rank);
  
    inplace=((void *)recvbuf==NULL);

    /* do first barrier to exchange addresses if necessary */
    if (!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);
    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);

    uop  = op->op;
  
    /* Warning!!!! Doesn't handle non-contiguous datatypes correctly.
                   But then again, neither does the default MPICH
                   implementation. */ 
    if (size < 2) {
        count = extent*recvcnt;
        if(!inplace) BCOPY(sendbuf,recvbuf,count);
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    count = 0;
    for (i=0;i<size;i++) count += sendcnts[i];
    istart = displs[rank]*extent;
    nbytes = count*extent;
    if(nbytes<=0) return MPI_SUCCESS; 

    if ((isshared = (p2p_shnode(sendbuf)>=0))) {
       *(bar->barf[rank].addr)=sendbuf;
    }
    else {
       *(bar->barf[rank].addr)=(void *)p2p_shmalloc(nbytes,bar->myhypernode);
    }
    *(bar->barf[rank].ival)=bar->myhypernode;
    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */

    /* set count and nbytes to the amount in my chunk */
    if(!isshared) {
        s2 = *(bar->barf[rank].addr);
        if (op->commute) {
            /* copy the pieces before and after my chunk into the buffer */
            for (i=0;i<size;i++) {
                nbytes0 = sendcnts[i]*extent;
                if(i!=rank) {
                    s1 = (void *)((char *)sendbuf +displs[i]*extent);
                    BCOPY(s1,s2,nbytes0);
                    dcache_flush_region(s2, nbytes0);
                    s2 = (void *)((char *)s2 + nbytes0);
                }
                else {
                    s2 = (void *)((char *)s2 + nbytes0);
                }
            }
        } 
        else {
            for (i=0;i<size;i++) {
                nbytes0 = sendcnts[i]*extent;
                s1 = (void *)((char *)sendbuf +displs[i]*extent);
                BCOPY(s1,s2,nbytes0);
                dcache_flush_region(s2, nbytes0);
                s2 = (void *)((char *)s2 + nbytes0);
            }
        } 
    }
    /* If not doing in-place copy my chunk to recvbuf.
       We detect the use of the in-place extension by whether or not
       recvbuf is NULL. This is non-standard. */
       
    if (op->commute && !inplace){
        s2 = (void *)recvbuf;
        s1 = (void *)((char *)sendbuf + istart);
        if(sendcnts[rank]>0) BCOPY(s1,s2,sendcnts[rank]*extent);
    }
  
    MPID_SPP_Wait_barrier(hcomm);
    /* If recvbuf is NULL, we are using the in-place extension. */
    s2 = (!inplace)? recvbuf : (void *)((char *)sendbuf + istart);
    if (op->commute) {
       for (jproc=rank+1;jproc<size;jproc++) {
           MPID_SPP_CNX_SAME_NODE=
                   ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
           s1 = (void *)((char *)(*(bar->barf[jproc].addr)) + istart);
           (*uop)(s1, s2, &count, &datatype);
           /* flush read only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
           dcache_flush_region(s1, nbytes);
       }
       for (jproc=0;jproc<rank;jproc++){
           MPID_SPP_CNX_SAME_NODE=
                ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
           s1 = (void *)((char *)(*(bar->barf[jproc].addr)) + istart);
           (*uop)(s1, s2, &count, &datatype);
           /* flush read only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
           dcache_flush_region(s1, nbytes);
       }
       MPID_SPP_Wait_barrier(hcomm);
    }
    else {
       for (jproc=1;jproc<size;jproc++)
           if(jproc!=0) (*uop)(
                  (void *)((char *)(*(bar->barf[jproc].addr)) + istart),
                  (void *)((char *)(*(bar->barf[0].addr))     + istart),
                  &count, &datatype);
       MPID_SPP_Wait_barrier(hcomm);
       if(nbytes>0) BCOPY((void *)((char *)(*(bar->barf[0].addr))+istart),
                          s2, nbytes);
    }
  
    MPID_SPP_CNX_SAME_NODE= 1; /* reset */
    if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
  
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    return (mpi_errno);
}

int MPID_SPP_Allgatherv ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcounts, displs,   recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int              *recvcounts;
int              *displs;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
    MPI_Comm  hcomm;
    int size, rank, root;
    int mpi_errno = MPI_SUCCESS;
    int flag;
    MPID_Fastbar *bar;
    MPI_Aint extent;
    void *s1,*s2,*s3;
    void *my_recvblock;
    int count,jproc,isshared=0;
    int mem_size;
    int (*packcontig)() = 0;
    int (*unpackcontig)() = 0;
    void *packctx = 0;
    void *unpackctx = 0;
    int outlen, totlen;
    int srclen, destlen;
    int err;
    int inplace;

    
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;
    /* Get the size of the communicator */
    MPI_Comm_size ( hcomm, &size );
    MPI_Type_extent (recvtype, &extent);
    MPI_Comm_rank   (hcomm, &rank);

    /* do first barrier to exchange addresses if necessary */
    if (!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);

    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);
    inplace = (sendbuf==NULL);

    /* this is my piece of the recv buffer */
    my_recvblock = (void *)((char *)recvbuf + displs[rank]*extent);

    if(inplace) { /* in-place extension */
        if (size < 2) {
            MPID_THREAD_UNLOCK(comm->ADIctx,comm);
            return MPI_SUCCESS;
        }

        if((isshared = (recvtype->is_contig && 
               p2p_shnode(my_recvblock)>=0))) {   
            /* we can use recvbuf as the shared memory buffer */
            *(bar->barf[rank].addr)=my_recvblock;
        } /* either recvbuf is not in shared mem. or using non-contig type */
        else if(recvtype->is_contig) {
            mem_size = recvcounts[rank]*extent;
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            /* copy my task's input into shared memory buffer */
            BCOPY(my_recvblock,*(bar->barf[rank].addr),mem_size);
        }
        else { /* non-contiguous type */
            mem_size = recvcounts[rank]*recvtype->size;
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            err = MPIR_Pack2( my_recvblock, recvcounts[rank], recvtype, 
                    packcontig, packctx, *(bar->barf[rank].addr),
                    &outlen, &totlen );
        }
    }
    else {  /* normal usage */
        /* First allocate the shared memory buffer if necessary and
           set the address in my barrier flag. Then copy or pack into buffer
           if necessary. */
        if((isshared = (sendtype->is_contig &&
               p2p_shnode(sendbuf)>=0))) {
            /* we can use sendbuf as the shared memory buffer */
            *(bar->barf[rank].addr)=sendbuf;
        }
        else if(sendtype->is_contig) {
            if(size>1) {
                mem_size = sendcount*extent;
                *(bar->barf[rank].addr)=
                 (void *)p2p_shmalloc(mem_size,bar->myhypernode);
                /* copy my task's input into shared memory buffer */
                BCOPY(sendbuf,*(bar->barf[rank].addr),mem_size);
            }
            else {
                isshared = 1; /* don't want sendbuf passed to p2p_shfree */
                *(bar->barf[rank].addr)=sendbuf;
            }
        }
        else {
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(sendcount*sendtype->size,bar->myhypernode);
            err = MPIR_Pack2( sendbuf,sendcount, sendtype, 
                    packcontig, packctx, *(bar->barf[rank].addr),
                    &outlen, &totlen );
        }
        /* Now copy sendbuf into my recvbuf block. */
        if(recvtype->is_contig) { /* recvbuf is contiguous so bcopy */
            BCOPY(*(bar->barf[rank].addr),my_recvblock,sendcount*extent);
        }
        else { /* recvbuf is not contiguous so unpack into it */
            err = MPIR_Unpack2(*(bar->barf[rank].addr),
                               recvcounts[rank],recvtype,
                               unpackcontig,unpackctx,my_recvblock,
                               &srclen,&destlen);
        }
    }
    *(bar->barf[rank].ival)=bar->myhypernode;
    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */
    if (size < 2) {
        if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    /* flush the cache */ 
    if(recvtype->is_contig)
        dcache_flush_region(*(bar->barf[rank].addr),recvcounts[rank]*extent);

    MPID_SPP_Wait_barrier(hcomm);
  
    for (jproc=rank+1;jproc<size;jproc++) {
        MPID_SPP_CNX_SAME_NODE=
                 ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
        s1 = (void *)((char *)recvbuf + displs[jproc]*extent);
        s2 = *(bar->barf[jproc].addr);
        if(recvtype->is_contig) {
            count =recvcounts[jproc]*extent;
            BCOPY(s2,s1,count);
            /* flush read-only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
            dcache_flush_region(s2, count);
        }
        else
            err = MPIR_Unpack2(s2,recvcounts[jproc],recvtype,
                               unpackcontig,unpackctx,s1,
                               &srclen,&destlen);
    }
    for (jproc=0;jproc<rank;jproc++) {
        MPID_SPP_CNX_SAME_NODE=
                 ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
        s1 = (void *)((char *)recvbuf + displs[jproc]*extent);
        s2 = *(bar->barf[jproc].addr);
        if(recvtype->is_contig) {
            count =recvcounts[jproc]*extent;
            BCOPY(s2,s1,count);
            /* flush read-only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
            dcache_flush_region(s2, count);
        }
        else
            err = MPIR_Unpack2(s2,recvcounts[jproc],recvtype,
                               unpackcontig,unpackctx,s1,
                               &srclen,&destlen);
    }
  
    MPID_SPP_Wait_barrier(hcomm);
    MPID_SPP_CNX_SAME_NODE=1;

    if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
  
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);

    return (mpi_errno);
}

int MPID_SPP_Allgather ( sendbuf, sendcount,  sendtype, 
                     recvbuf, recvcount, recvtype, comm )
void             *sendbuf;
int               sendcount;
MPI_Datatype      sendtype;
void             *recvbuf;
int              recvcount;
MPI_Datatype      recvtype;
MPI_Comm          comm;
{
    MPI_Comm  hcomm;
    int size, rank, root;
    int mpi_errno = MPI_SUCCESS;
    int flag;
    MPID_Fastbar *bar;
    MPI_Aint extent;
    void *s1,*s2,*s3;
    void *my_recvblock;
    int count,jproc,isshared=0;
    int mem_size;
    int (*packcontig)() = 0;
    int (*unpackcontig)() = 0;
    void *packctx = 0;
    void *unpackctx = 0;
    int outlen, totlen;
    int srclen, destlen;
    int err;
    int inplace;

    
    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;
    /* Get the size of the communicator */
    MPI_Comm_size ( hcomm, &size );
    MPI_Type_extent (recvtype, &extent);
    MPI_Comm_rank   (hcomm, &rank);


    /* do first barrier to exchange addresses if necessary */
    if (!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);

    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);
    inplace = (sendbuf==NULL);

    /* this is my piece of the recv buffer */
    my_recvblock = (void *)((char *)recvbuf + recvcount*rank*extent);

    if(inplace) { /* in-place extension */
        if (size < 2) {
            MPID_THREAD_UNLOCK(comm->ADIctx,comm);
            return MPI_SUCCESS;
        }

        if((isshared = (recvtype->is_contig && 
               p2p_shnode(my_recvblock)>=0))) {   
            /* we can use recvbuf as the shared memory buffer */
            *(bar->barf[rank].addr)=my_recvblock;
        } /* either recvbuf is not in shared mem. or using non-contig type */
        else if(recvtype->is_contig) {
            mem_size = recvcount*extent;
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            /* copy my task's input into shared memory buffer */
            BCOPY(my_recvblock,*(bar->barf[rank].addr),mem_size);
        }
        else { /* non-contiguous type */
            mem_size = recvcount*recvtype->size;
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            err = MPIR_Pack2( my_recvblock, recvcount, recvtype, 
                    packcontig, packctx, *(bar->barf[rank].addr),
                    &outlen, &totlen );
        }
    }
    else {  /* normal usage */
        /* First allocate the shared memory buffer if necessary and
           set the address in my barrier flag. Then copy or pack into buffer
           if necessary. */
        if((isshared = (sendtype->is_contig &&
               p2p_shnode(sendbuf)>=0))) {
            /* we can use sendbuf as the shared memory buffer */
            *(bar->barf[rank].addr)=sendbuf;
        }
        else if(sendtype->is_contig) {
            if(size>1) {
                mem_size = sendcount*extent;
                *(bar->barf[rank].addr)=
                 (void *)p2p_shmalloc(mem_size,bar->myhypernode);
                /* copy my task's input into shared memory buffer */
                BCOPY(sendbuf,*(bar->barf[rank].addr),mem_size);
            }
            else {
                isshared = 1; /* don't want sendbuf passed to p2p_shfree */
                *(bar->barf[rank].addr)=sendbuf;
            }
        }
        else {
            *(bar->barf[rank].addr)=
             (void *)p2p_shmalloc(sendcount*sendtype->size,bar->myhypernode);
            err = MPIR_Pack2( sendbuf,sendcount, sendtype, 
                    packcontig, packctx, *(bar->barf[rank].addr),
                    &outlen, &totlen );
        }
        /* Now copy sendbuf into my recvbuf block. */
        if(recvtype->is_contig) { /* recvbuf is contiguous so bcopy */
            BCOPY(*(bar->barf[rank].addr),my_recvblock,sendcount*extent);
        }
        else { /* recvbuf is not contiguous so unpack into it */
            err = MPIR_Unpack2(*(bar->barf[rank].addr),
                               recvcount,recvtype,
                               unpackcontig,unpackctx,my_recvblock,
                               &srclen,&destlen);
        }
    }
    *(bar->barf[rank].ival)=bar->myhypernode;
    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */
    if (size < 2) {
        if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }
  
    /* flush the cache */ 
    if(recvtype->is_contig)
       dcache_flush_region(*(bar->barf[rank].addr),recvcount*extent);

    MPID_SPP_Wait_barrier(hcomm);
  
    count =recvcount*extent;
    for (jproc=rank+1;jproc<size;jproc++) {
        MPID_SPP_CNX_SAME_NODE=
                 ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
        s1 = (void *)((char *)recvbuf + jproc*recvcount*extent);
        s2 = *(bar->barf[jproc].addr);
        if(recvtype->is_contig) {
            BCOPY(s2,s1,count);
            /* flush read-only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
            dcache_flush_region(s2, count);
        }
        else
            err = MPIR_Unpack2(s2,recvcount,recvtype,
                               unpackcontig,unpackctx,s1,
                               &srclen,&destlen);
    }
    for (jproc=0;jproc<rank;jproc++) {
        MPID_SPP_CNX_SAME_NODE=
                 ( *(bar->barf[jproc].ival) != bar->myhypernode )?0:1;
        s1 = (void *)((char *)recvbuf + jproc*recvcount*extent);
        s2 = *(bar->barf[jproc].addr);
        if(recvtype->is_contig) {
            BCOPY(s2,s1,count);
            /* flush read-only data from my cache and off my hypernode 
              no one else should have this piece except for writer */
            dcache_flush_region(s2, count);
        }
        else
            err = MPIR_Unpack2(s2,recvcount,recvtype,
                               unpackcontig,unpackctx,s1,
                               &srclen,&destlen);
    }
  
    MPID_SPP_Wait_barrier(hcomm);
    MPID_SPP_CNX_SAME_NODE=1;

    if(!isshared) p2p_shfree(*(bar->barf[rank].addr));
  
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);

    return (mpi_errno);
}


int MPID_SPP_Reduce (sendbuf,recvbuf,count,datatype,op,root,comm)
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
int		  root;
MPI_Comm          comm;
{
    MPI_Comm hcomm;
    MPI_User_function *uop;
    int   rank, size, i;
    MPI_Aint extent;
    int   mpi_errno;
    int   flag;
    MPID_Fastbar *bar;
    int nremnd,nchunk;
    int jproc,to,istart;
    void *s2;
    int inplace;

    MPID_THREAD_LOCK(comm->ADIctx,comm);
    mpi_errno=MPI_SUCCESS;
    hcomm = comm->comm_coll;
    uop  = op->op;
    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);

    MPI_Comm_size ( hcomm, &size );
    inplace=((void *)recvbuf==NULL);

    if (size < 2) {
        if(!inplace) BCOPY(sendbuf,recvbuf,extent*count);
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    MPI_Comm_rank   (hcomm, &rank);
    MPI_Type_extent (datatype, &extent);

    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */
    /* small array code */
    if(count<size || count*extent<size*64) {
        /* do first barrier to exchange addresses if necessary */
        if(!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);
        MPID_SPP_Small_reduce(sendbuf,recvbuf,count,datatype,
                                   extent,op,root,hcomm);
        MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }


    nchunk = count/size;
    nremnd = count - nchunk*size;
    if (nremnd>0) {
        if(rank < nremnd) {
           nchunk++;
           istart = nchunk*rank*extent;
        }
        else {
           istart = (nchunk*rank+nremnd)*extent;
        }
    }
    else {
       istart = nchunk*rank*extent;
    }
        

    /* shared memory buffer */
    *(bar->barf[rank].addr)=(void *)p2p_shmalloc(count*extent,bar->myhypernode);
    *(bar->barf[rank].ival)=bar->myhypernode;

    /* copy sendbuf to shared memory buffer */
    BCOPY(sendbuf,*(bar->barf[rank].addr),count*extent);

    MPID_SPP_Wait_barrier(hcomm);
    to = (op->commute)?root:0;

    for (jproc=0;jproc<size;jproc++)
        if(jproc!=to) (*uop)(
            (void *)((char *)(*(bar->barf[jproc].addr)) + istart),
            (void *)((char *)(*(bar->barf[to].addr))    + istart),
            &nchunk, &datatype);

    MPID_SPP_Wait_barrier(hcomm);
    if(rank==root) {
        s2 = (!inplace)?recvbuf:sendbuf; /* check for in-place extension */
        BCOPY(*(bar->barf[to].addr),s2,count*extent);
    }
    p2p_shfree(*(bar->barf[rank].addr));

    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    return MPI_SUCCESS;
}


/* small array reduce
     does fan-in/fan-out algorithm
*/
int MPID_SPP_Small_reduce(sendbuf,recvbuf,count,datatype,extent,op,root,comm)
void             *sendbuf;
void             *recvbuf;
int               count;
int               extent;
MPI_Datatype      datatype;
MPI_Op            op;
int		  root;
MPI_Comm          comm;
{
   int nproc,me;
   int ncycles,twok,k,modme,other;
   MPID_Fastbar *bar;
   volatile int *flag;
   int nbytes;
   MPI_User_function *uop;
   int isshared = 0;
   void *s2;
   int inplace;

   bar = (MPID_Fastbar *)(comm->ADIBarrier);
   uop  = op->op;

   inplace=((void *)recvbuf==NULL);
   ncycles = bar->nc;
   nproc = bar->np;
   me = bar->mypid;

   nbytes = count*extent;
   if(nbytes<=0) return MPI_SUCCESS;
   *(bar->barf[me].addr)=(void *)p2p_shmalloc(nbytes,bar->myhypernode);
   BCOPY(sendbuf,(void *)(*(bar->barf[me].addr)),nbytes);
   *(bar->barf[me].ival)=bar->myhypernode;
   MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */

   twok=1;
   for (k=0;k<ncycles;k++){
      if((modme = me%(twok*2))==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_post0(flag);
      }
      else if(modme==0 && (other=me+twok)<nproc) {
         flag = bar->barf[other].flag;
         MPID_SPP_waitneq(flag);
         /* we are writing into my memory (pulling) */
         MPID_SPP_CNX_SAME_NODE=
                   ( *(bar->barf[other].ival) != bar->myhypernode )?0:1;
         (*uop)(*(bar->barf[other].addr),*(bar->barf[me].addr),
                &count, &datatype);
         /* flush read only data from my cache and off my hypernode 
             no one else should have this piece except for writer */
         dcache_flush_region(*(bar->barf[other].addr),nbytes);
      }
      twok*=2;
   }
   /* just finish the barrier without any data transfer */
   for (k=0;k<ncycles;k++){
      twok/=2;
      if((modme = me%(twok*2))==0 && (other=me+twok) <nproc) {
         flag = bar->barf[other].flag;
         MPID_SPP_post1(flag);
      }
      else if(modme==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_waiteq(flag);
      }
   }
   MPID_SPP_CNX_SAME_NODE=1;
   if(me==root){
      s2 = (!inplace)?recvbuf:sendbuf; /* in-place extension used? */
      BCOPY( (void *)(*(bar->barf[me].addr)),s2,nbytes);
   }
   p2p_shfree(*(bar->barf[me].addr));
   return 0;
}

int MPID_SPP_Allreduce(sendbuf,recvbuf,count,datatype,op,comm)
void             *sendbuf;
void             *recvbuf;
int               count;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
    MPI_Comm hcomm;
    int   rank, size, i;
    MPI_Aint extent;
    int   mpi_errno;
    int   flag;
    MPID_Fastbar *bar;
    MPID_Fastcoll *coll;
    int *displs,*blockcounts;
    int n,nremnd,nchunk;
    void *s1,*s2;
    int inplace;

    MPID_THREAD_LOCK(comm->ADIctx,comm);
    mpi_errno=MPI_SUCCESS;
    hcomm = comm->comm_coll;

    MPI_Comm_size ( hcomm, &size );
    MPI_Comm_rank   (hcomm, &rank);
    MPI_Type_extent (datatype, &extent);

    inplace=((void *)recvbuf==NULL);

    if (size < 2) {
        if(!inplace) BCOPY(sendbuf,recvbuf,extent*count);
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */

    /* small array code */
    if(count<size) {
        /* do first barrier to exchange addresses if necessary */
        if(!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);
        MPID_SPP_Small_allreduce(sendbuf,recvbuf,count,datatype,
                                   extent,op,hcomm);
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }
 
    if(!hcomm->ADICollect) {
        if(!(hcomm->ADICollect = (void *)MALLOC(sizeof(MPID_Fastcoll)))){
            (*MPID_ErrorHandler)( 1, 
             "No more memory for storing blockcounts in MPID_Allreduce");
            return MPI_ERR_EXHAUSTED;
        }
        coll = (MPID_Fastcoll *)hcomm->ADICollect;
        coll->bsize = size;
        if(!(coll->blockcounts = (int *)MALLOC(size*sizeof(int)))){
            (*MPID_ErrorHandler)( 1, 
             "No more memory for storing blockcounts in MPID_Allreduce");
            return MPI_ERR_EXHAUSTED;
        }
        if(!(coll->displs = (int *)MALLOC(size*sizeof(int)))){
            (*MPID_ErrorHandler)( 1, 
             "No more memory for storing displs in MPID_Allreduce");
            return MPI_ERR_EXHAUSTED;
        }
    }
    else if((coll=(MPID_Fastcoll *)hcomm->ADICollect)->bsize<size) {
        coll->bsize = size;
        FREE(coll->blockcounts);
        if(!(coll->blockcounts = (int *)MALLOC(size*sizeof(int)))){
            (*MPID_ErrorHandler)( 1, 
              "No more memory for storing blockcounts in MPID_Allreduce");
            return MPI_ERR_EXHAUSTED;
        }
        if(!(coll->displs = (int *)MALLOC(size*sizeof(int)))){
            (*MPID_ErrorHandler)( 1, 
             "No more memory for storing displs in MPID_Allreduce");
            return MPI_ERR_EXHAUSTED;
        }
    }

    displs = coll->displs;
    blockcounts = coll->blockcounts;
    nchunk = count/size;
    nremnd = count - nchunk*size;
    n = 0;
    if (nremnd>0) {
        for(i=0;i<nremnd;i++){
            displs[i]=n;
            n += nchunk+1;
            blockcounts[i]=nchunk+1;
        }
        for(i=nremnd;i<size;i++){
            displs[i]=n;
            n += nchunk;
            blockcounts[i]=nchunk;
        }
    }
    else {
        for(i=0;i<size;i++){
            displs[i]=n;
            n += nchunk;
            blockcounts[i]=nchunk;
        }
    }
    if(inplace) { /* In-place extension */
        s1 = (void *)NULL;
        s2 = sendbuf;
    }
    else {
        s1 = (void *)((char *)recvbuf + displs[rank]*extent);
        s2 = recvbuf;
    }
    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    /* These guys will do their own locking. */
    MPID_SPP_Reduce_scatter(sendbuf,s1,
                              coll->blockcounts,datatype,op,comm);
    MPID_SPP_Allgatherv(s1,
                     coll->blockcounts[rank],datatype, 
                     s2,coll->blockcounts,coll->displs,datatype,comm);
    return MPI_SUCCESS;
}

/* small array allreduce
     does fan-in/fan-out algorithm
*/
int MPID_SPP_Small_allreduce(sendbuf,recvbuf,count,datatype,extent,op,comm)
void             *sendbuf;
void             *recvbuf;
int               count;
int               extent;
MPI_Datatype      datatype;
MPI_Op            op;
MPI_Comm          comm;
{
   int nproc,me;
   int ncycles,twok,k,modme,other;
   MPID_Fastbar *bar;
   volatile int *flag;
   int nbytes;
   MPI_User_function *uop;
   int isshared = 0;
   void *s2;
   int inplace;

   bar = (MPID_Fastbar *)(comm->ADIBarrier);
   uop  = op->op;

   ncycles = bar->nc;
   nproc = bar->np;
   me = bar->mypid;
   inplace=((void *)recvbuf==NULL);


   nbytes = count*extent;
   if(nbytes<=0) return MPI_SUCCESS;
   /* Check for whether we are supplying the shared memory buffer.
      If recvbuf is NULL we are using the in-place extension. */
   /* note this routine doesn't do the non-contiguous data type case
      correctly. Neither doesn't the current default MPICH. */
   isshared = (!inplace)?(p2p_shnode(recvbuf)>=0):(p2p_shnode(sendbuf)>=0);
   if (isshared) {
      if(!inplace) {
         *(bar->barf[me].addr)=recvbuf;
         BCOPY(sendbuf,recvbuf,nbytes);
      }
      else { /* in-place extension */
         *(bar->barf[me].addr)=sendbuf;
      }
   }
   else {
      *(bar->barf[me].addr)=(void *)p2p_shmalloc(nbytes,bar->myhypernode);
      BCOPY(sendbuf,(void *)(*(bar->barf[me].addr)),nbytes);
   }
    *(bar->barf[me].ival)=bar->myhypernode;
    MPID_SPP_CNX_SAME_NODE=1; /* we set for safety */

   twok=1;
   for (k=0;k<ncycles;k++){
      if((modme = me%(twok*2))==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_post0(flag);
      }
      else if(modme==0 && (other=me+twok)<nproc) {
         flag = bar->barf[other].flag;
         MPID_SPP_waitneq(flag);
         /* we are writing into my memory (pulling) */
         MPID_SPP_CNX_SAME_NODE=
                   ( *(bar->barf[other].ival) != bar->myhypernode )?0:1;
         (*uop)(*(bar->barf[other].addr),*(bar->barf[me].addr),
                &count, &datatype);
         /* flush read only data from my cache and off my hypernode 
             no one else should have this piece except for writer */
         dcache_flush_region(*(bar->barf[other].addr),nbytes);
      }
      twok*=2;
   }
   for (k=0;k<ncycles;k++){
      twok/=2;
      if((modme = me%(twok*2))==0 && (other=me+twok) <nproc) {
         flag = bar->barf[other].flag;
         /* we are writing into other's memory (pushing) */
         /* probably need a write_prefetch bcopy here. later */ 
         MPID_SPP_CNX_SAME_NODE=1;
         BCOPY(*(bar->barf[me].addr),*(bar->barf[other].addr),nbytes);
         dcache_flush_region(*(bar->barf[other].addr),nbytes);
         MPID_SPP_post1(flag);
      }
      else if(modme==twok) {
         flag = bar->barf[me].flag;
         MPID_SPP_waiteq(flag);
      }
   }
   MPID_SPP_CNX_SAME_NODE=1;
   if(!isshared) {
       s2 = (!inplace)?recvbuf:sendbuf; /* in-place extension used? */
       BCOPY( (void *)(*(bar->barf[me].addr)),s2,nbytes);
       p2p_shfree(*(bar->barf[me].addr));
   }
   return 0;
}

int MPID_SPP_Bcast ( buffer, count, datatype, root, comm )
void             *buffer;
int               count;
MPI_Datatype      datatype;
int               root;
MPI_Comm          comm;
{
    MPI_Comm hcomm;
    int   rank, size;
    MPI_Aint extent;
    MPID_Fastbar *bar;
    int isshared=0;
    int mem_size;
    int (*packcontig)() = 0;
    int (*unpackcontig)() = 0;
    void *packctx = 0;
    void *unpackctx = 0;
    int outlen, totlen;
    int srclen, destlen;
    int err;

    MPID_THREAD_LOCK(comm->ADIctx,comm);
    hcomm = comm->comm_coll;

    MPI_Comm_size(hcomm,&size);
    if (size < 2) {
        MPID_THREAD_UNLOCK(comm->ADIctx,comm);
        return MPI_SUCCESS;
    }

    MPI_Type_extent (datatype, &extent);
    MPI_Comm_rank   (hcomm, &rank);


    /* do first barrier to exchange addresses if necessary */
    if (!hcomm->ADIBarrier) MPID_SPP_First_barrier(hcomm);
    bar = (MPID_Fastbar *)(hcomm->ADIBarrier);
    if(rank==root) {
        if((isshared = (datatype->is_contig && p2p_shnode(buffer)>=0))) {
            *(bar->barf[rank].addr)=buffer;
            mem_size = count*extent;
        }
        else if(datatype->is_contig) {
            mem_size = count*extent;
            *(bar->barf[root].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            /* copy my task's input into shared memory buffer */
            BCOPY(buffer,*(bar->barf[root].addr),mem_size);
            dcache_flush_region(*(bar->barf[root].addr), mem_size);
        }
        else { /* non-contiguous type */
            mem_size = count*datatype->size;
            *(bar->barf[root].addr)=
             (void *)p2p_shmalloc(mem_size,bar->myhypernode);
            err = MPIR_Pack2( buffer, count, datatype,
                    packcontig, packctx, *(bar->barf[root].addr),
                    &outlen, &totlen );
        }
        MPID_SPP_Wait_barrier(hcomm); /* let the games begin */
        MPID_SPP_Wait_barrier(hcomm); /* after the receivers have gotten it */
        if(!isshared) p2p_shfree(*(bar->barf[root].addr));
    }
    else {
        MPID_SPP_Wait_barrier(hcomm);
        if (datatype->is_contig) {
            mem_size = count*extent;
            BCOPY(*(bar->barf[root].addr),buffer,mem_size);
            dcache_flush_region(*(bar->barf[root].addr), mem_size);
        }
        else {
            mem_size = count*datatype->size;
            err = MPIR_Unpack2(*(bar->barf[root].addr),
                               count,datatype,
                               unpackcontig,unpackctx,buffer,
                               &srclen,&destlen);
            dcache_flush_region(*(bar->barf[root].addr), mem_size);
        }
        MPID_SPP_Wait_barrier(hcomm);
        
    }

    MPID_THREAD_UNLOCK(comm->ADIctx,comm);
    return MPI_SUCCESS;
}
static int
MPID_SPP_iceil(double x)
{
   /* CEILING FUNCTION */
   int ix;

   ix = (int)x;
   if(x>=0.0)
      if(x>(double)ix) ix++;
   else
      if(x<(double)ix) ix--;
   return ix;
}

