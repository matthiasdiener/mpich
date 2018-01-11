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

/* IRIX uses non-mmap code */
#if defined(MPI_IRIX) && defined(HAVE_MMAP)
#undef HAVE_MMAP
#endif

#if defined(MPI_hpux) || defined(HAVE_MMAP)
#    include <sys/types.h>
#    include <sys/mman.h>
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (8*1024*1024)
#endif 

#if defined(MPI_hpux)
/* HPUX uses a special lock/unlock set */
typedef int p2p_lock_t[4];
#define p2p_lock_init(l) { *((int*)(l)) = 1; }
#define p2p_lock(l)      acquire_lock(l)
#define p2p_unlock(l)    release_lock(l)

#elif defined(HAVE_MMAP)

#ifndef MPID_CACHE_LINE_SIZE
#define MPID_CACHE_LINE_SIZE 128
#define MPID_CACHE_LINE_LOG_SIZE 7
#endif

/* Place each lock on its own cache line.  We probably really want two
   lock types - one on its own line, and one within another structure */
typedef struct { msemaphore lock; 
		 char pad[MPID_CACHE_LINE_SIZE - sizeof(msemaphore)]; }
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

#elif defined(HAVE_SHMAT)
	 typedef struct { int semid;  int semnum; }   MD_lock_t;
#        include <sys/ipc.h>
#        include <sys/shm.h>
#        include <sys/sem.h>

         static struct sembuf sem_lock[1] = {
             0, -1, 0
         };
         static struct sembuf sem_unlock[1] = {
             0, 1, 0
         };

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
void p2p_init ANSI_ARGS((int,int));
void p2p_shfree ANSI_ARGS((char *));
void p2p_cleanup ANSI_ARGS((void));
#endif

