#ifndef P2P_INCLUDED
#define P2P_INCLUDED

#if !defined(P2P_EXTERN)
#define P2P_EXTERN extern
#endif

#    include <sys/cnx_mman.h>
#    include <sys/cnx_types.h>
int getSCTopology(cnx_node_t *, unsigned int *, unsigned int *, unsigned int *);
#    include <sys/mman.h>
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (16*1024*1024)
 
typedef int p2p_lock_t[4];
#define p2p_lock_init(l) { *((int*)(l)) = 1; }
#define p2p_lock(l)      MPID_SPP__acquire_lock(l)
#define p2p_unlock(l)    MPID_SPP__release_lock(l)

/* following is for POSIX std versions of Unix */
#include <unistd.h>

/* Bindings */
#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#if defined(__STDC__)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

void *p2p_shmalloc ANSI_ARGS((unsigned int, cnx_node_t));
double p2p_wtime ANSI_ARGS((void));
void p2p_setpgrp ANSI_ARGS((void));
#endif

