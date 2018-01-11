#define P2P_EXTERN
#include "p2p.h"
#include <stdio.h>

#define p2p_dprintf printf

#if defined(HAVE_MMAP)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void *xx_shmalloc();
void xx_shfree();
void xx_init_shmalloc();

#elif defined(HAVE_SHMAT)
void *MD_init_shmem();

#endif

/* This is a process group, used to help clean things up when a process dies 
   It turns out that this causes strange failures when running a program
   under another program, like a debugger or mpirun.  Until this is
   resolved, I'm ifdef'ing this out.
 */
static int MPID_SHMEM_ppid = 0;
void
p2p_setpgrp()
{
#ifdef FOO
   MPID_SHMEM_ppid = getpid();
   if(setpgid(MPID_SHMEM_ppid,MPID_SHMEM_ppid)) {
       perror("failure in p2p_setpgrp");
       exit(-1);
   }
#endif
}

void p2p_init(maxprocs,memsize)
int maxprocs;
int memsize;
{

#if defined(MPI_IRIX)
       
    strcpy(p2p_sgi_shared_arena_filename,"/tmp/p2p_shared_arena_");
    sprintf(&(p2p_sgi_shared_arena_filename[strlen(p2p_sgi_shared_arena_filename)]),"%d",getpid());

    if (usconfig(CONF_INITUSERS,maxprocs) == -1)
	p2p_error("p2p_init: usconfig failed for users: \n",maxprocs);
    if (usconfig(CONF_INITSIZE,memsize) == -1)
	p2p_error("p2p_init: usconfig failed: cannot map shared arena\n",
		  memsize);
    if (usconfig(CONF_ARENATYPE,US_SHAREDONLY) == -1)
	p2p_error("p2p_init: usconfig failed: cannot make shared-only\n",0);
    p2p_sgi_usptr = usinit(p2p_sgi_shared_arena_filename);
    if (p2p_sgi_usptr == NULL)
	p2p_error("p2p_init: usinit failed: can't map shared arena\n",memsize);

#endif

#if defined(USE_XX_SHMALLOC)

    caddr_t p2p_start_shared_area;
    static int p2p_shared_map_fd;
#if defined(MPI_solaris) || defined(MPI_sun4)
    p2p_shared_map_fd = open("/dev/zero", O_RDWR);
    p2p_start_shared_area = (char *) mmap((caddr_t) 0, memsize,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_SHARED, 
			p2p_shared_map_fd, (off_t) 0);

#elif defined(HAVE_MMAP)
    p2p_start_shared_area = (char *) mmap((caddr_t) 0, memsize,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_SHARED|MAP_ANONYMOUS|MAP_VARIABLE, 
			-1, (off_t) 0);

    /* fprintf(stderr,"memsize = %d, address = 0x%x\n",
	    memsize,p2p_start_shared_area); */

#elif defined(HAVE_SHMAT)
    p2p_start_shared_area = MD_init_shmem(&memsize);
#endif

    if (p2p_start_shared_area == (caddr_t)-1)
    {
        perror("mmap failed");
	p2p_error("OOPS: mmap failed: cannot map shared memory, size=",
		  memsize);
    }
    xx_init_shmalloc(p2p_start_shared_area,memsize);
#endif /* USE_XX_SHMALLOC */
}

/* 
   The create_procs routine keeps track of the processes (stores the
   rc from fork) and is prepared to kill the children if it
   receives a SIGCHLD.  One problem is making sure that the kill code
   isn't invoked during a normal shutdown.  This is handled by turning
   off the signals while in the rundown part of the code; this introduces
   a race condition in failures that I'm not prepared for yet.
 */
#ifndef MPID_MAX_PROCS
#define MPID_MAX_PROCS 32
#endif
static int MPID_child_pid[MPID_MAX_PROCS];
static int MPID_numprocs = 0;

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
/* Set SIGCHLD handler */
static int MPID_child_status = 0;
#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif
/* Define standard signals if SysV version is loaded */
#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD SIGCLD
#endif

/* POSIX/Solaris handlers seem to have a single argument ... */
#ifdef MPI_solaris
void MPID_handle_child( sig )
int               sig;
#else
RETSIGTYPE MPID_handle_child( sig, code, scp )
int               sig, code;
struct sigcontext *scp;
#endif
/* Some systems have an additional char *addr 
   */
{
int prog_stat, pid;
int i, j;

/* Really need to block further signals until done ... */
/* fprintf( stderr, "Got SIGCHLD...\n" ); */
pid	   = waitpid( (pid_t)(-1), &prog_stat, WNOHANG );
if (MPID_numprocs && pid && (WIFEXITED(prog_stat) || WIFSIGNALED(prog_stat))) {
#ifdef MPID_DEBUG_ALL
    if (MPID_DebugFlag) printf("Got signal for child %d (exited)... \n", pid );
#endif
    /* The child has stopped. Remove it from the jobs array */
    for (i = 0; i<MPID_numprocs; i++) {
	if (MPID_child_pid[i] == pid) {
	    MPID_child_pid[i] = 0;
	    if (WIFEXITED(prog_stat)) {
		MPID_child_status |= WEXITSTATUS(prog_stat);
		}
	    /* If we're not exiting, cause an abort. */
	    if (WIFSIGNALED(prog_stat)) {
		p2p_error( "Child process died unexpectedly from signal", 
			   WTERMSIG(prog_stat) );
	        }
	    else
		p2p_error( "Child process exited unexpectedly", i );
	    break;
	    }
	}
    /* Child may already have been deleted by Leaf exit; we should
       use conn->state to record it rather than zeroing it */
    /* 
    if (i == MPID_numprocs) {
	fprintf( stderr, "Received signal from unknown child!\n" );
	}
	 */
    }
/* Re-enable signals */
}

void p2p_clear_signal()
{
(void)signal( SIGCHLD, SIG_IGN );
}

void p2p_create_procs(numprocs)
int numprocs;
{
    int i, rc;

    (void) signal( SIGCHLD, MPID_handle_child );
    for (i = 0; i < numprocs; i++)
    {
	if ((rc = fork()) == -1)
	{
	    p2p_error("p2p_init: fork failed\n",(-1));
	}
	else if (rc == 0) {
#ifdef FOO
	    if(setpgid(0,MPID_SHMEM_ppid)) {
		p2p_error("p2p_init: failure in setpgid\n",(-1));
		exit(-1);
		}	    
#endif
	    return;
	    }
	else {
	    /* Save pid of child so that we can detect child exit */
	    MPID_child_pid[i] = rc;
	    MPID_numprocs     = i+1;
	    }
	    
    }
}



void *p2p_shmalloc(size)
int size;
{
    void *p = 0;

#if defined(MPI_IRIX)
    p = usmalloc(size,p2p_sgi_usptr);

#elif defined(USE_XX_SHMALLOC)
    p = (void *) xx_shmalloc(size);
#endif

    return (p);
}

void p2p_shfree(ptr)
char *ptr;
{

#if defined(MPI_IRIX)
    usfree(ptr,p2p_sgi_usptr);

#elif defined(USE_XX_SHMALLOC)
  (void) xx_shfree(ptr);
#endif

}


#if defined(MPI_IRIX)
/* this is the spinlock method */
#if 0 && defined(MPID_CACHE_ALIGN)
void p2p_lock_init(L) 
p2p_lock_t *L;
{ 
    (*L).lock = usnewlock(p2p_sgi_usptr);
}
#else
void p2p_lock_init(L) 
p2p_lock_t *L;
{ 
    (*L) = usnewlock(p2p_sgi_usptr);
}

/* this is the semaphore method */
/**********
void p2p_lock_init(L) 
p2p_lock_t *L;
{ 
    (*L) = usnewsema(p2p_sgi_usptr,1); 
}
**********/
#endif /* MPID_CACHE_ALIGN */

#elif defined(HAVE_SEMOP)
/* This is the SYSV_IPC from p4.  This is needed under SunOS 4.x, since 
   SunOS has no mutex/msem routines */
/* The shmat code is present because, under AIX 4.x, it
   was suggested that shmat would be faster than mmap */

/* Currently unedited!!!*/
int MD_init_sysv_semset(setnum)
int setnum;
{
    int i, semid;
#   if defined(MPI_solaris)
    union semun{
      int val;
      struct semid_ds *buf;
      ushort *array;
      } arg;
#   elif defined(MPI_ibm3090) || defined(MPI_rs6000) ||    \
       defined(MPI_dec5000) ||    \
       defined(MPI_hpux) || defined(MPI_ksr)  
    int arg;
#   else
    union semun arg;
#   endif

#   if defined(MPI_solaris)
    arg.val = 1;
#   elif defined(MPI_ibm3090) || defined(MPI_rs6000) ||    \
       defined(MPI_dec5000) ||    \
       defined(MPI_hpux) || defined(MPI_ksr) 
    arg = 1;
#   else
    arg.val = 1;
#   endif

    if ((semid = semget(getpid()+setnum,10,IPC_CREAT|0600)) < 0)
    {
	p2p_error("semget failed for setnum=%d\n",setnum);
    }
    for (i=0; i < 10; i++)
    {
	if (semctl(semid,i,SETVAL,arg) == -1)
	{
	    p2p_error("semctl setval failed\n",-1);
	}
    }
    return(semid);
}

void MD_lock_init(L)
MD_lock_t *L;
{
int setnum;

/* Is p4_global in shared memory or not?   */
    MD_lock(&(p4_global->slave_lock));
    setnum = p4_global->sysv_next_lock / 10;
    if (setnum > P4_MAX_SYSV_SEMIDS)
    {
	p2p_error("exceeding max num of p4 semids\n",P4_MAX_SYSV_SEMIDS);
    }
    if (p4_global->sysv_next_lock % 10 == 0)
    {
	p4_global->sysv_semid[setnum] = init_sysv_semset(setnum);
	p4_global->sysv_num_semids++;
    }
    L->semid  = p4_global->sysv_semid[setnum];
    L->semnum = p4_global->sysv_next_lock - (setnum * 10);
    p4_global->sysv_next_lock++;
    MD_unlock(&(p4_global->slave_lock));
}


void MD_lock(L)
MD_lock_t *L;
{
    sem_lock[0].sem_num = L->semnum;
    if (semop(L->semid,&sem_lock[0],1) < 0)
    {
        p2p_error("OOPS: semop lock failed\n",*L);
    }
}

void MD_unlock(L)
MD_lock_t *L;
{
    sem_unlock[0].sem_num = L->semnum;
    if (semop(L->semid,&sem_unlock[0],1) < 0)
    {
        p2p_error("OOPS: semop unlock failed\n",L);
    }
}
#endif

#if defined(MPI_IRIX)
#elif defined(HAVE_MMAP)
#elif defined(HAVE_SHMAT)
void *MD_init_shmem(memsize)
int *memsize;
{
    int i,nsegs;
    unsigned size, segsize = P2_SYSV_SHM_SEGSIZE;
    char *mem, *tmem, *pmem;

    if (*memsize  &&  (*memsize % P2_SYSV_SHM_SEGSIZE) == 0)
	nsegs = *memsize / segsize;
    else
	nsegs = *memsize / segsize + 1;
    size = nsegs * segsize;
    *memsize = size;
    if ((sysv_shmid[0] = shmget(getpid(),segsize,IPC_CREAT|0600)) == -1)
    {
	p2p_error("OOPS: shmget failed\n",sysv_shmid[0]);
    }
    if ((mem = (char *)shmat(sysv_shmid[0],NULL,0)) == (char *)-1)
    {
	p2p_error("OOPS: shmat failed\n",mem);
    }
    sysv_num_shmids++;
    nsegs--;
    pmem = mem;
    for (i=1; i <= nsegs; i++)
    {
	if ((sysv_shmid[i] = shmget(i+getpid(),segsize,IPC_CREAT|0600)) == -1)
	{
	    p2p_error("OOPS: shmget failed\n",sysv_shmid[i]);
	}
        if ((tmem = (char *)shmat(sysv_shmid[i],pmem+segsize,0)) == (char *)-1)
        {
            if ((tmem = (char *)shmat(sysv_shmid[i],pmem-segsize,0)) == (char *)-1)
            {
                p2p_error("OOPS: shmat failed\n",tmem);
            }
	    else
	    {
		mem = tmem;
	    }
        }
	sysv_num_shmids++;
	pmem = tmem;
    }
    return mem;
}
#endif /* MPI_IRIX etc */


/* Cleanup is the NORMAL termination code; it may be called in abormal
   termination as well */
void p2p_cleanup()
{
 
#if defined(MPI_IRIX)
    unlink(p2p_sgi_shared_arena_filename);
#endif

}

/*****

void p2p_dprintf(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;
 
    printf("%s: ", p2p_my_dprintf_id);
    va_start(ap);
#   if defined(HAVE_VPRINTF)
    vprintf(fmt, ap);
#   else
    _doprnt(fmt, ap, stdout);
#   endif
    va_end(ap);
    fflush(stdout);
}
 
void p2p_dprintfl(level, fmt, va_alist)
int level;
char *fmt;
va_dcl
{
    va_list ap;
 
    if (level > p2p_debug_level)
        return;
    printf("%d: %s: ", level, p2p_my_dprintf_id);
    va_start(ap);
#   if defined(HAVE_VPRINTF)
    vprintf(fmt, ap);
#   else
    _doprnt(fmt, ap, stdout);
#   endif
    va_end(ap);
    fflush(stdout);
}

*****/

void p2p_error(string,value)
char * string;
int value;
{
    printf("%s %d\n",string, value);
    /* printf("p2p_error is not fully cleaning up at present\n"); */
    p2p_cleanup();
    /* We need to do an abort to make sure that the children get killed */
#ifdef FOO
    if (MPID_SHMEM_ppid) 
	kill( -MPID_SHMEM_ppid, SIGKILL );
#endif
    abort();
    /* exit(99); */
}
#include <sys/time.h>

void p2p_wtime_init()
{
}

double p2p_wtime()
{
    double timeval;
    struct timeval tp;

#if defined(MPI_IRIX) || defined(MPI_hpux) || defined(HAVE_GETTIMEOFDAY) || \
    defined(USE_BSDGETTIMEOFDAY) || defined(USE_WIERDGETTIMEOFDAY)
    struct timezone tzp;
#endif

    tp.tv_sec  = 0;
    tp.tv_usec = 0;

#if defined(MPI_IRIX) || defined(USE_BSDGETTIMEOFDAY)
    BSDgettimeofday(&tp,&tzp);
#endif

#if defined(MPI_hpux) || defined(HAVE_GETTIMEOFDAY)
    gettimeofday(&tp,&tzp);
#endif

#if defined(MPI_solaris) || defined(USE_WIERDGETTIMEOFDAY)
    gettimeofday(&tp);
#endif

    timeval = (double) tp.tv_sec;
    timeval = timeval + (double) ((double) .000001 * (double) tp.tv_usec);

    return(timeval);
}

/*
   Yield to other processes (rather than spinning in place)
 */
void p2p_yield()
{

#if defined(MPI_IRIX)
/* Multiprocessor IRIX machines may want to comment this out for lower
   latency */
sginap(0);

#elif defined(USE_SELECT_YIELD)
/* Use a short select as a way to suggest to the OS to deschedule the 
   process */
struct timeval tp;
tp.tv_sec  = 0;
tp.tv_usec = 10;
select( 0, (void *)0, (void *)0, (void *)0, &tp );
#endif
}


#if defined(USE_XX_SHMALLOC)
/* This is not machine dependent code but is only used on some machines */

/*
  Memory management routines from ANSI K&R C, modified to manage
  a single block of shared memory.
  Have stripped out all the usage monitoring to keep it simple.

  To initialize a piece of shared memory:
    xx_init_shmalloc(char *memory, unsigned nbytes)

  Then call xx_shmalloc() and xx_shfree() as usual.
*/

#ifdef MPID_CACHE_LINE_SIZE
#define ALIGNMENT (2*MPID_CACHE_LINE_SIZE)
#define LOG_ALIGN (MPID_CACHE_LINE_LOG_SIZE+1)
#else
#define LOG_ALIGN 6
#define ALIGNMENT (1 << LOG_ALIGN)
#endif
/* ALIGNMENT is assumed below to be bigger than sizeof(p2p_lock_t) +
   sizeof(Header *), so do not reduce LOG_ALIGN below 4 */

union header
{
    struct
    {
	union header *ptr;	/* next block if on free list */
	unsigned size;		/* size of this block */
    } s;
    char align[ALIGNMENT];	/* Align to ALIGNMENT byte boundary */
};

typedef union header Header;

static Header **freep;		/* pointer to pointer to start of free list */
static p2p_lock_t *p2p_shmem_lock;	/* Pointer to lock */

void xx_init_shmalloc(memory, nbytes)
char *memory;
unsigned nbytes;
/*
  memory points to a region of shared memory nbytes long.
  initialize the data structures needed to manage this memory
*/
{
    int nunits = nbytes >> LOG_ALIGN;
    Header *region = (Header *) memory;

    /* Quick check that things are OK */

    if (ALIGNMENT != sizeof(Header) ||
	ALIGNMENT < (sizeof(Header *) + sizeof(p2p_lock_t)))
    {
        p2p_dprintf("%d %d\n",sizeof(Header),sizeof(p2p_lock_t));
	p2p_error("xx_init_shmem: Alignment is wrong", ALIGNMENT);
    }

    if (!region)
	p2p_error("xx_init_shmem: Passed null pointer", 0);

    if (nunits < 2)
	p2p_error("xx_init_shmem: Initial region is ridiculously small",
		 (int) nbytes);

    /*
     * Shared memory region is structured as follows
     * 
     * 1) (Header *) freep ... free list pointer 2) (p2p_lock_t) p2p_shmem_lock ...
     * space to hold lock 3) padding up to alignment boundary 4) First header
     * of free list
     */

    freep = (Header **) region;	/* Free space pointer in first block  */
#if defined(MPI_hpux)    
    p2p_shmem_lock = (p2p_lock_t *) ((char *)freep + 16);/* aligned for HP  */
#else 
    p2p_shmem_lock = (p2p_lock_t *) (freep + 1);/* Lock still in first block */
#endif
    (region + 1)->s.ptr = *freep = region + 1;	/* Data in rest */
    (region + 1)->s.size = nunits - 1;	/* One header consumed already */

    p2p_lock_init(p2p_shmem_lock);                /* Initialize the lock */

}

void *xx_shmalloc(nbytes)
unsigned nbytes;
{
    Header *p, *prevp;
    char *address = (char *) NULL;
    unsigned nunits;
    int msemsize = sizeof(MPID_msemaphore);

    /* Force entire routine to be single threaded */
    (void) p2p_lock(p2p_shmem_lock);

#if defined(MPI_hpux) || defined(HAVE_MSEM_INIT)
    /* Why? */
    nbytes += msemsize;
#endif

    nunits = ((nbytes + sizeof(Header) - 1) >> LOG_ALIGN) + 1;

    prevp = *freep;
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
    {
	if (p->s.size >= nunits)
	{			/* Big enuf */
	    if (p->s.size == nunits)	/* exact fit */
		prevp->s.ptr = p->s.ptr;
	    else
	    {			/* allocate tail end */
		p->s.size -= nunits;
		p += p->s.size;
		p->s.size = nunits;
	    }
	    *freep = prevp;
	    address = (char *) (p + 1);
	    break;
	}
	if (p == *freep)
	{			/* wrapped around the free list ... no fit
				 * found */
	    address = (char *) NULL;
	    break;
	}
    }

    /* End critical region */
    (void) p2p_unlock(p2p_shmem_lock);

    if (address == NULL)
	p2p_dprintf("xx_shmalloc: returning NULL; requested %d bytes\n",nbytes);
    return address;
}

void xx_shfree(ap)
char *ap;
{
    Header *bp, *p;

    /* Begin critical region */
    (void) p2p_lock(p2p_shmem_lock);

    if (!ap)
	return;			/* Do nothing with NULL pointers */

    bp = (Header *) ap - 1;	/* Point to block header */

    for (p = *freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
	    break;		/* Freed block at start of end of arena */

    if (bp + bp->s.size == p->s.ptr)
    {				/* join to upper neighbour */
	bp->s.size += p->s.ptr->s.size;
	bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
	bp->s.ptr = p->s.ptr;

    if (p + p->s.size == bp)
    {				/* Join to lower neighbour */
	p->s.size += bp->s.size;
	p->s.ptr = bp->s.ptr;
    }
    else
	p->s.ptr = bp;

    *freep = p;

    /* End critical region */
    (void) p2p_unlock(p2p_shmem_lock);
}
#endif

#if defined(MPI_hpux)
/* interface to hpux locks by Dan Golan of Convex Computer 
   (Putting this here is easier than editing the Makefile) */
#include "mem.c"
#endif
