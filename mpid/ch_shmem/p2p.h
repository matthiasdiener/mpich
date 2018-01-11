#ifndef P2P_INCLUDED
#define P2P_INCLUDED

#if !defined(P2P_EXTERN)
#define P2P_EXTERN extern
#endif

#define MPID_CACHE_ALIGN
#if defined(MPID_CACHE_ALIGN)
#define PAD(n) char pad[n];
#else
#define PAD(n)
#endif

#if defined(MPI_hpux)
#    include <sys/mman.h>
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (8*1024*1024)
 
typedef int p2p_lock_t[4];
#define p2p_lock_init(l) { *((int*)(l)) = 1; }
#define p2p_lock(l)      acquire_lock(l)
#define p2p_unlock(l)    release_lock(l)


#endif

#if defined(MPI_solaris)
#    include <sys/mman.h>
#    include <sys/systeminfo.h>
#    include <sys/processor.h>
#    include <sys/procset.h>
#    include <synch.h>
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (16*1024*1024)
     typedef mutex_t p2p_lock_t;
#    define p2p_lock_init(l) mutex_init(l,USYNC_PROCESS,(void *)NULL)
#    define p2p_lock(l)      mutex_lock(l)
#    define p2p_unlock(l)    mutex_unlock(l)
#endif


#if defined(MPI_IRIX)
#    include <ulocks.h>
#    include <malloc.h>

#ifdef MPID_CACHE_ALIGN
     typedef usema_t *MD_lock_t;

     typedef union { MD_lock_t lock; PAD(128) } p2p_lock_t;
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


#    define GLOBMEMSIZE  (8*1024*1024)
/*   MD_lock_init must be defined in p2p_MD.c */
/*   spinlock method */
/*   semaphore method */
/*****
#    define MD_lock(l)      uspsema(*(l))
#    define MD_unlock(l)    usvsema(*(l))
*****/

P2P_EXTERN usptr_t *p2p_sgi_usptr;
P2P_EXTERN char p2p_sgi_shared_arena_filename[64];

#endif


/* following is for POSIX std versions of Unix */
#if defined(MPI_IRIX)  ||  defined(MPI_hpux)
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

void *p2p_shmalloc ANSI_ARGS((int));
double p2p_wtime ANSI_ARGS((void));

#endif

