
/* ------------------ Machine Dependent Definitions ---------------- */
/*
        It is important to maintain the order of many of the 
        definitions in this file.
*/


#if defined(SUN_SOLARIS)
#define HP
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "SUN"
#endif

#if defined(ALPHA)
#define DEC5000
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "DEC5000"
#endif

#if defined(MEIKO_CS2)
#define IPSC860
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "IPSC860"
#endif

#if defined(SP1)
#define RS6000
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "RS6000"
#endif

#if defined(SP1_EUI) || defined(SP1_EUIH)
#define RS6000
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "RS6000"
#endif

#if defined(SGI_MP) || defined(SGI_CH) || defined (SGI_CH64)
#define SGI
#define VENDOR_IPC
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "SGI"
#endif

#if defined(PARAGON)
#define IPSC860
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "IPSC860"
#endif

#if defined(LINUX)
#define P4_MACHINE_TYPE "LINUX"
#endif

#if defined(FREEBSD)
#define P4_MACHINE_TYPE "FREEBSD"
#endif

#if defined(CONVEX)
#define SUN
#undef P4_MACHINE_TYPE
#define P4_MACHINE_TYPE "SUN"
#endif

#if defined(TC_2000_TCMP)
#define TC_2000
#define TCMP
#endif

#if defined(FX8)  ||  defined(FX2800)  || defined(FX2800_SWITCH)
#define ALLIANT
#endif

#if defined(FX2800)  || defined(FX2800_SWITCH) || defined(FREEBSD)
#define VPRINTF
#endif

#if defined(DELTA) || defined(IPSC860_SOCKETS) || defined(PARAGON)
#define IPSC860
#endif

#if defined(CM5_SOCKETS)
#define CM5
#endif

#if defined(NCUBE_SOCKETS)
#define NCUBE
#endif

#if defined(NEXT)  || defined(KSR) ||  defined(IPSC860)  || defined(NCUBE)
#define GLOBAL
#endif


#if defined(SUN)        || defined(DEC5000)  || defined(LINUX) || \
    defined(NEXT)       || defined(KSR)      || defined(FREEBSD) || \
    defined(SYMMETRY)   || defined(BALANCE)  || \
    defined(ALLIANT)    || defined(MULTIMAX) ||  defined(CM5) || \
    defined(GP_1000)    || defined(TC_2000)  ||  defined(IBM3090)

#define P4BSD

#endif

#if defined(SUN)        || defined(DEC5000)  || defined(LINUX) || \
    defined(NEXT)       || defined(KSR)      || defined(FREEBSD) || \
    defined(SYMMETRY)   || defined(BALANCE)  || \
    defined(ALLIANT)    || defined(MULTIMAX) || \
    defined(GP_1000)    || defined(TC_2000)  ||  defined(IBM3090)

#define CAN_DO_SETSOCKOPT

#endif

#if defined(RS6000)          ||                          \
    defined(IPSC860_SOCKETS) ||                          \
    defined(NCUBE_SOCKETS)   ||                          \
    defined(DELTA)           || defined(TITAN)        || \
    defined(SGI)             || defined(CRAY)         || \
    defined(HP)              || defined(SYMMETRY_PTX)

#ifndef SGI
/* Recommended by SGI to NOT set */
#define CAN_DO_SETSOCKOPT
#endif

#ifdef NEEDS_NETINET
#include <netinet/in.h>
#endif

#endif


#if defined(RS6000)       || \
    defined(IPSC860)      ||                          \
    defined(NCUBE)        ||                          \
    defined(DELTA)        || defined(TITAN)        || \
    defined(SGI)          || defined(CRAY)         || \
    defined(HP)           || defined(SYMMETRY_PTX) || \
    defined(MEIKO_CS2)

#define P4SYSV

#endif


#ifdef P4SYSV
#   ifdef NCUBE
#   define SIGNAL_P4 signal
#   else
#   define SIGNAL_P4 sigset
#   endif
#else
#define SIGNAL_P4 signal
#endif


#ifndef P4BOOL
#define P4BOOL int
#endif

#if defined(BALANCE)  ||  defined(FX8)
#define P4VOID int
#else 
#define P4VOID void
#endif


/*----------------- IBM SP-1 with EUI library ------------- */

#if defined(SP1_EUI)

#define NO_TYPE_EUI     0 
#define ACK_REQUEST_EUI 1
#define ACK_REPLY_EUI   2
#define ANY_P4TYPE_EUI  (-1)

#define MYNODE()  eui_mynode

#define CAN_DO_CUBE_MSGS
#define MD_cube_send  MD_eui_send
#define MD_cube_recv  MD_eui_recv
#define MD_cube_msgs_available  MD_eui_msgs_available

#endif

/*----------------- IBM SP-1 with EUI-H library ------------- */

#if defined(SP1_EUIH)

#define NO_TYPE_EUIH     0 
#define ACK_REQUEST_EUIH 1
#define ACK_REPLY_EUIH   2
#define ANY_P4TYPE_EUIH  -1

#define MYNODE()  euih_mynode

#define CAN_DO_CUBE_MSGS
#define MD_cube_send            MD_euih_send
#define MD_cube_recv            MD_euih_recv
#define MD_cube_msgs_available  MD_euih_msgs_available

#endif


/*------------------ Encore Multimax ---------------------- */


#if defined(MULTIMAX)

#include <parallel.h>

#ifndef LINT
typedef LOCK *MD_lock_t;
#define MD_lock_init(l)  *(l) = spin_create(PAR_UNLOCKED);
#define MD_lock(l)       spin_lock(*(l));
#define MD_unlock(l)     spin_unlock(*(l));
#endif

#define GLOBMEMSIZE  (4*1024*1024)
#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define CAN_DO_SHMEM_MSGS
#define P4_MAX_MSG_QUEUES 64

#endif


/*------------------ Sequent Balance or Symmetry ---------------------- */


#if !defined(SYMMETRY) && !defined(SYMMETRY_PTX) && !defined(BALANCE)
#define CAN_HANDLE_SIGSEGV
#endif

#if defined(BALANCE) || defined(SYMMETRY) || defined(SYMMETRY_PTX)

#include <parallel/parallel.h>
#if defined(SYMMETRY_PTX)
#include <sys/timers.h>          /* for getclock */
#endif

typedef slock_t MD_lock_t;

#ifndef LINT
#define MD_lock_init(l)  s_init_lock(l);
#define MD_lock(l)       s_lock(l);
#define MD_unlock(l)     s_unlock(l);
#endif
extern char *shmalloc();
#if defined(SYMMETRY_PTX)
extern P4VOID *malloc();
#else
extern char *malloc();
#endif

#define GLOBMEMSIZE  (4*1024*1024)
#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define CAN_DO_SHMEM_MSGS
#define P4_MAX_MSG_QUEUES 64

#endif

/*---------------------------- Alliant -------------------------------- */
#if defined(ALLIANT)

typedef char MD_lock_t;

#ifndef LINT
#define MD_lock_init(l)  initialize_lock(l);
#define MD_lock(l)       lock(l);
#define MD_unlock(l)     unlock(l);
#endif
extern char *valloc();

#define GLOBMEMSIZE  (2*1024*1024)

#define USE_XX_SHMALLOC          /* If not defined uses dumb xx_malloc */

#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define CAN_DO_SHMEM_MSGS
#define P4_MAX_MSG_QUEUES 64

#endif

#if defined(FX2800_SWITCH)
#include "sw.h"
#define CAN_DO_SWITCH_MSGS
#endif


/*---------------------------- Others -------------------------- */

#if defined(CRAY) || defined(NEXT)
#define GLOBMEMSIZE  (4*1024*1024)
#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define P4_MAX_MSG_QUEUES 1
typedef int MD_lock_t;
#define MD_lock_init(l)
#define MD_lock(l)
#define MD_unlock(l)
#endif


#if defined(SUN_SOLARIS)
#include <sys/mman.h>
#include <sys/systeminfo.h>
#include <sys/processor.h>
#include <sys/procset.h>
#include <synch.h>
#endif 

#if    defined(SUN)     || defined(SGI)  \
    || defined(DEC5000) || defined(LINUX) \
    || defined(RS6000)  || defined(IBM3090) || defined(FREEBSD) \
    || defined(TITAN)  \
    || defined(HP)

#    define P4_SYSV_SHM_SEGSIZE (1*1024*1024)

#    if defined(SYSV_IPC)
#        define GLOBMEMSIZE  (4*1024*1024)
#        define CAN_DO_SOCKET_MSGS
#        define CAN_DO_XDR
#        define CAN_DO_SHMEM_MSGS
#        define USE_XX_SHMALLOC
#        define P4_MAX_MSG_QUEUES 4
#        define P4_MAX_SYSV_SHMIDS  8
#        define P4_MAX_SYSV_SEMIDS  8
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
#    endif

#    if !defined(SYSV_IPC)  &&  !defined(VENDOR_IPC)
#        define GLOBMEMSIZE  (4*1024*1024)
#        define CAN_DO_SOCKET_MSGS
#        define CAN_DO_XDR
#        define P4_MAX_MSG_QUEUES 1
	 typedef int MD_lock_t;
#        define MD_lock_init(l)
#        define MD_lock(l)
#        define MD_unlock(l)
#    endif
#endif


#if defined(SUN_SOLARIS)  &&  defined(VENDOR_IPC)

#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define P4_MAX_MSG_QUEUES 8
#    define CAN_DO_SHMEM_MSGS
#    define USE_XX_SHMALLOC
#    define GLOBMEMSIZE  (16*1024*1024)
     typedef mutex_t MD_lock_t;
#    define MD_lock_init(l) mutex_init(l,USYNC_PROCESS,(P4VOID *)NULL)
#    define MD_lock(l)      mutex_lock(l)
#    define MD_unlock(l)    mutex_unlock(l)
#endif


#if defined(SGI)  &&  defined(VENDOR_IPC)

#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define P4_MAX_MSG_QUEUES 20
#    include <ulocks.h>
#    include <malloc.h>
#    define CAN_DO_SHMEM_MSGS
#    define GLOBMEMSIZE  (8*1024*1024)
     typedef usema_t *MD_lock_t;
/*   MD_lock_init must be defined in p4_MD.c */
/*   spinlock method */
#    define MD_lock(l) ussetlock(*(l))
#    define MD_unlock(l) usunsetlock(*(l))
/*   semaphore method */
/*****
#    define MD_lock(l)      uspsema(*l)
#    define MD_unlock(l)    usvsema(*l)
*****/
#endif


/* following is for POSIX std versions of Unix */
#if defined(SGI)  ||  defined(RS6000)  ||  defined(HP)
#include <unistd.h>
#endif

/* Peter Krauss suggested this change (POSIX ?) */
#if defined(HP)
#define		getdtablesize()		sysconf(_SC_OPEN_MAX)
#endif



/*---------------------------- IPSC860 Cube --------------------------- */

#if defined(IPSC860)

#    if defined(DELTA)
#        define P4_MAX_CUBE_MSGS_OUT 5
#    else
#        define P4_MAX_CUBE_MSGS_OUT 5
#    endif

#define MD_cube_send  MD_i860_send
#define MD_cube_recv  MD_i860_recv
#define MD_cube_msgs_available  MD_i860_msgs_available

typedef int MD_lock_t;

#if defined(IPSC860)
#define MYNODE mynode
#endif

#ifndef LINT
#define MD_lock_init(l)
#define MD_lock(l)
#define MD_unlock(l)
#endif

#define GLOBMEMSIZE  (1*1024*1024)
#define CAN_DO_CUBE_MSGS
#define P4_MAX_MSG_QUEUES 1

#define ALL_NODES -1

#define NO_TYPE_IPSC     0 
#define ACK_REQUEST_IPSC 1
#define ACK_REPLY_IPSC   2
#if defined(MEIKO_CS2)
#define ANY_P4TYPE_IPSC    -1
#else
#define ANY_P4TYPE_IPSC    0x80000007
#endif

#define NODE_PID 0

#if defined(IPSC860_SOCKETS)
#define CAN_DO_SOCKET_MSGS
/*****
#include <CMC/sys/types.h>
#include <CMC/sys/socket.h>
#include <CMC/netinet/in.h>
#include <CMC/netdb.h>
*****/
#include <CMC/ntoh.h>
#endif

#endif    

/*---------------------------- CM-5 --------------------------- */

#if defined(CM5)

#include <cm/cmmd.h>
/* #include <cm/cmmd-io.h> */

typedef int MD_lock_t;

#if defined(CM5)
#define MYNODE CMMD_self_address
#endif

#ifndef LINT
#define MD_lock_init(l)
#define MD_lock(l)
#define MD_unlock(l)
#endif

#define GLOBMEMSIZE  (1*1024*1024)
#define CAN_DO_CUBE_MSGS
#define P4_MAX_MSG_QUEUES 1

#define NO_TYPE_CM5     0 
#define ACK_REQUEST_CM5 1
#define ACK_REPLY_CM5   2
#define ANY_P4TYPE_CM5    CMMD_ANY_TAG

#define MD_cube_send  MD_CM5_send
#define MD_cube_recv  MD_CM5_recv
#define MD_cube_msgs_available  MD_CM5_msgs_available

#endif    


/*---------------------------- NCUBE --------------------------- */

#if defined(NCUBE)

typedef int MD_lock_t;

#include <sysn.h> 

#define MYNODE npid

#ifndef LINT
#define MD_lock_init(l)
#define MD_lock(l)
#define MD_unlock(l)
#endif

#define GLOBMEMSIZE  (1*1024*1024)
#define CAN_DO_CUBE_MSGS
#define P4_MAX_MSG_QUEUES 1

#define NO_TYPE_NCUBE     0 
#define ACK_REQUEST_NCUBE 1
#define ACK_REPLY_NCUBE   2
#define ANY_P4TYPE_NCUBE  (-1)

#define NCUBE_ANY_NODE  (-1)
#define NCUBE_ANY_TAG   (-1)

#define MD_cube_send  MD_NCUBE_send
#define MD_cube_recv  MD_NCUBE_recv
#define MD_cube_msgs_available  MD_NCUBE_msgs_available

#endif    

/*----------------   KSR             -------------------------*/
#if defined(KSR)
#include <sys/mman.h>
#include <pthread.h>

#define USE_XX_SHMALLOC
#define GLOBMEMSIZE  (16*1024*1024)
#define P4_MAX_MSG_QUEUES 64
#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define CAN_DO_SHMEM_MSGS

#define MD_lock_t       msemaphore
#define MD_lock_init(l) msem_init(l, MSEM_UNLOCKED)
#define MD_lock(l)      msem_lock(l, 0)
#define MD_unlock(l )   msem_unlock(l, 0)

#endif



/*------------------ Butterfly TC-2000/GP-1000 -------------- */
#if defined(TC_2000)  ||  defined(GP_1000)
#include <mach.h>    
#include <sys/cluster.h>
#include <sys/kern_return.h>
#include <heap.h>

char *xx_malloc();
P4VOID MD_malloc_hint();

#ifdef MALLOC_STATS
static unsigned int allocated = 0;
#endif

#define MD_lock_t       int
#ifndef LINT
#define MD_lock_init(l) simple_unlock(l)
#define MD_lock(l)      simple_lock(l)
#define MD_unlock(l)    simple_unlock(l)
#endif

#define GLOBMEMSIZE  (8*1024*1024)
#define CAN_DO_SOCKET_MSGS
#define CAN_DO_XDR
#define CAN_DO_SHMEM_MSGS
#define P4_MAX_MSG_QUEUES 128

#endif

#ifdef TCMP
#define CAN_DO_TCMP_MSGS
/* #include </Net/sparky/sparky1/lusk/lepido/tcmp/tcmp.h> */
#include </usr/bbnm/tcmp/tcmp.h>
#endif

/* ----------------- Can be made machine dependent -------------------*/

typedef unsigned long p4_usc_time_t;

/* Bill says take this out, 12/22/94
extern P4VOID exit();
*/

#define P4_MAXPROCS 256

/* For sysinfo */
#if defined(SUN_SOLARIS) || defined(MEIKO_CS2)
#include <sys/systeminfo.h>
#endif

/* Note that defining MEMDEBUG fails on HPs unless the ANSI option is 
   selected in the compiler */
/* #define MEMDEBUG */

#if defined(MEMDEBUG)

#ifndef LINT
#define  P4_INCLUDED
#include "mpisys.h"
#define p4_malloc(size) MALLOC(size)
#define p4_free(p) FREE(p)
#define p4_clock MD_clock
#endif

#else

#ifndef LINT
#define p4_malloc malloc
#define p4_free free
#define p4_clock MD_clock
#endif

#endif
