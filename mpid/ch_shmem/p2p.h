#ifndef P2P_INCLUDED
#define P2P_INCLUDED

#if !defined(P2P_EXTERN)
#define P2P_EXTERN extern
#endif

/* 
   We choose special options for SGI based on MPI_IRIX; we want to use
   these same options for MPI_IRIX64
 */
#ifdef MPI_IRIX64
#define MPI_IRIX
#endif

/* Definitions for improving Cache locality */
#define MPID_CACHE_ALIGN
#if defined(MPID_CACHE_ALIGN)
#define PAD(n) char pad[n];
#else
#define PAD(n)
#endif
#ifndef MPID_CACHE_LINE_SIZE
#define MPID_CACHE_LINE_SIZE 128
#define MPID_CACHE_LINE_LOG_SIZE 7
#endif

/* 
 * We try to keep this code general, with identification of special cases on 
 * a system-by-system basis.
 */
/* IRIX uses non-mmap code; we'll test for IRIX specially */
#if defined(MPI_IRIX) && defined(HAVE_MMAP)
#undef HAVE_MMAP
#endif
#if defined(MPI_IRIX) && defined(HAVE_SEMOP)
#undef HAVE_SEMOP
#endif

/* HPUX uses special locks */
#if defined(MPI_hpux) && defined(HAVE_SEMOP)
#undef HAVE_SEMOP
#endif

/* Solaris also has its own locks */
#if defined(HAVE_MUTEX_INIT) && defined(HAVE_SEMOP)
#undef HAVE_SEMOP
#endif

#if defined(MPI_cspp)
#define MPID_FLUSH_CACHE(addr,size) dcache_flush_region(addr, size);
#endif
/* Choose shared memory scheme; prefer mmap to shmat */
#if defined(MPI_IRIX)
#    include <ulocks.h>
P2P_EXTERN usptr_t *p2p_sgi_usptr;
P2P_EXTERN char p2p_sgi_shared_arena_filename[64];
#    define GLOBMEMSIZE  (8*1024*1024)

#elif defined(HAVE_MMAP)
#    include <sys/types.h>
#    include <sys/mman.h>
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (8*1024*1024)

#elif defined(HAVE_SHMAT)
#        include <sys/shm.h>
#    define USE_XX_SHMALLOC

#else
    Choke - no shared memory !
#endif 

/*
 * We may want to use a distributed shmalloc instead of a single
 * area malloc.
 */
#ifdef USE_DISTRIB_SHMALLOC
#define SHMALLOC(size,region) p2p_shmalloc(size,region)
#else
#define SHMALLOC(size,region) p2p_shmalloc(size)
#endif

/* It is often advisable to flush cache of shared memory objects when
   they are no longer needed.  This macro does that; if it is undefined,
   then nothing happens 
 */
#ifndef MPID_FLUSH_CACHE
#define  MPID_FLUSH_CACHE(addr,size)
#endif

/* Choose lock scheme */
#if defined(MPI_IRIX)
#    include <ulocks.h>
#    include <malloc.h>

/* This is probably NOT a good idea, since the standard locks are
   retrieved as addresses by the usnewlock routine.
   However, this DOES let the ADDRESSES of the locks reside on 
   different cachelines */
#if 0 && defined(MPID_CACHE_ALIGN)
     typedef usema_t *MD_lock_t;

     typedef union { MD_lock_t lock; PAD(MPID_CACHE_LINE_SIZE) } p2p_lock_t;
#define p2p_lock_init(l)  MD_lock_init(l)
#define p2p_lock(l)       MD_lock((*l).lock)
#define p2p_unlock(l)     MD_unlock((*l).lock)
#    define MD_lock(l) ussetlock((l))
#    define MD_unlock(l) usunsetlock((l))
#else
     typedef usema_t *MD_lock_t;

     typedef MD_lock_t p2p_lock_t;
#define p2p_lock_init(l)  MD_lock_init(l)
#define p2p_lock(l)       MD_lock(l)
#define p2p_unlock(l)     MD_unlock(l)
#    define MD_lock(l) ussetlock(*(l))
#    define MD_unlock(l) usunsetlock(*(l))
#endif


/*   MD_lock_init must be defined in p2p_MD.c */
/*   spinlock method */
/*   semaphore method */
/*****
#    define MD_lock(l)      uspsema(*(l))
#    define MD_unlock(l)    usvsema(*(l))
*****/

#elif defined(MPI_hpux)
/* HPUX uses a special lock/unlock set (see mem.c file) */
typedef int p2p_lock_t[4];
#define p2p_lock_init(l) { *((int*)(l)) = 1; }
#define p2p_lock(l)      MPID_SHMEM__acquire_lock(l)
#define p2p_unlock(l)    MPID_SHMEM__release_lock(l)
typedef struct msemaphore MPID_msemaphore;

#elif defined(HAVE_MSEM_INIT)
/* If Sys V Semaphores also available, turn them off for now (not used
   for locks) */
#ifdef HAVE_SEMOP
#undef HAVE_SEMOP
#endif
/* Place each lock on its own cache line.  We probably really want two
   lock types - one on its own line, and one within another structure */
/* Problem - some systems use struct msemaphore, others use just
   msemaphore */
typedef msemaphore MPID_msemaphore;
/* 
 typedef struct msemaphore MPID_msemaphore;
 */
typedef struct { MPID_msemaphore lock; 
		 char pad[MPID_CACHE_LINE_SIZE - sizeof(MPID_msemaphore)]; }
        p2p_lock_t;
/* An alternative to this is to ALLOCATE these on separate cache lines,
   but this is simpler for now */
#define p2p_lock_init(l) msem_init(&(l)->lock, MSEM_UNLOCKED)
#define p2p_lock(l)      msem_lock(&(l)->lock, 0)
#define p2p_unlock(l )   msem_unlock(&(l)->lock, 0)

/* typedef msemaphore p2p_lock_t; */
/*#define p2p_lock_init(l) msem_init(l, MSEM_UNLOCKED)
* #define p2p_lock(l)      msem_lock(l, 0)
* #define p2p_unlock(l )   msem_unlock(l, 0) */

#elif defined(HAVE_MUTEX_INIT)
/* Only known system is Solaris */
#    include <sys/systeminfo.h>
#    include <sys/processor.h>
#    include <sys/procset.h>
#    include <synch.h>
     typedef mutex_t p2p_lock_t;
#    define p2p_lock_init(l) mutex_init(l,USYNC_PROCESS,(void *)NULL)
#    define p2p_lock(l)      mutex_lock(l)
#    define p2p_unlock(l)    mutex_unlock(l)
typedef mutex_t MPID_msemaphore;

#elif defined(HAVE_SEMOP)
#        include <sys/ipc.h>
#        include <sys/sem.h>

	 typedef struct { int semid;  int semnum; }   MD_lock_t;
         static struct sembuf sem_lock[1] = {
             0, -1, 0
         };
         static struct sembuf sem_unlock[1] = {
             0, 1, 0
         };
     typedef MD_lock_t p2p_lock_t;
#else
    Choke - no lock code!
#endif


/* following is for POSIX std versions of Unix */
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif


/* Bindings */
#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

#ifdef USE_DISTRIB_SHMALLOC
void *p2p_shmalloc ANSI_ARGS((int,int));
#else
void *p2p_shmalloc ANSI_ARGS((int));
#endif
double p2p_wtime ANSI_ARGS((void));
void p2p_init ANSI_ARGS((int,int));
void p2p_shfree ANSI_ARGS((char *));
void p2p_cleanup ANSI_ARGS((void));
void p2p_error  ANSI_ARGS((char *, int));
void p2p_setpgrp ANSI_ARGS((void));
#endif

