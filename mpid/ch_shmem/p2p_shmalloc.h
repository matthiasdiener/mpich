/* p2p shared-memory includes.  This file includes code for picking a strategy,
 * whether generic (e.g., MMAP or SHMAT) or machine specific.  First we choose
 * a method, and then, in a second step, generate the appropriate declarations.
 */

/* Choose method for providing shared memory.  The decision will be made by
 * combining preferences (defined here) with available capabilities (should be,
 * and for the most part are, defined by configure as HAVE_, al).  Some of the
 * HAVE_'s may be defined in p2p_special.h, at least temporarily.
 */

#if !defined(SHMEM_PICKED)

/* SGI has shared arenas, which are preferred there */
#if defined(MPI_IRIX)
#    define PREFER_ARENAS
#endif

#if defined(MPI_cspp)
#    define PREFER_MMAP		/* unnecessary because of general mmap pref? */
#endif

/* Now we convert our preferences and our capabilites into choices of what
 * to actually use.  Some cumbersomeness is generated by checking that we
 * do not pick two different schemes.
 */

#ifdef SHMEM_PICKED
#    undef SHMEM_PICKED		/* start clean, no final decisions so far */
#endif

#if defined(HAVE_ARENAS) && defined(PREFER_ARENAS)
#    if defined(SHMEM_PICKED)
         'Oops - trying to use two different shmem schemes'
#    else
#        define USE_ARENAS
#        define SHMEM_PICKED
#    endif
#endif

/* If there is nothing special then we will use mmap if we have it. */
#if !defined(SHMEM_PICKED) && defined(HAVE_MMAP)
#    define USE_MMAP
#    define SHMEM_PICKED
#endif

/* Reluctant default: System V shared memory segments.  Look out for ipcs. */
#if !defined(SHMEM_PICKED) && defined(HAVE_SHMAT)
#    define USE_SHMAT
#    define SHMEM_PICKED
#endif

/* Check that we picked at least one model for shared memory.  We have already
 * checked that we did not pick more than one.
 */
#if !defined(SHMEM_PICKED)
      Choke - no shared memory implementation picked
#endif

#endif /* !defined(SHMEM_PICKED) */

/* At this point we should have generated exactly one USE_xxx.
 * Now we generate the appropriate includes and declarations.
 */

/* SGI shared arenas */
#if defined(USE_ARENAS)
#    include <malloc.h>
     P2P_EXTERN usptr_t *p2p_sgi_usptr;
     P2P_EXTERN char p2p_sgi_shared_arena_filename[64];
#endif

/* mmap */
#if defined(USE_MMAP)
#    if defined (MPI_cspp)
#        include <sys/cnx_mman.h>
#    endif
#    include <sys/mman.h>
#    define USE_XX_SHMALLOC
#endif

/* System V shared memory - look out for dangling ipcs when used */
#if defined(USE_SHMAT)
#    include <sys/ipc.h>
#    include <sys/shm.h>
#    define USE_XX_SHMALLOC
#endif

/* A few special cases */

#if defined(MPI_cspp)
#    define GLOBMEMSIZE  (16*1024*1024)
#endif

/*
 * We may want to use a distributed shmalloc instead of a single area malloc.
 */
#if defined(USE_DISTRIB_SHMALLOC)
#    define SHMALLOC(size, region) p2p_shmalloc(size, region)
#    define INITSHMALLOC(region,size,nnodes) xx_init_shmalloc(size,region,nnodes)
#else
#    define SHMALLOC(size,region) p2p_shmalloc(size)
#    define INITSHMALLOC(region,size,nnodes) xx_init_shmalloc(size,region)
#endif

/* default shared-memory size is 8 Megs */
#if !defined(GLOBMEMSIZE)
#    define GLOBMEMSIZE  (8*1024*1024)
#endif

