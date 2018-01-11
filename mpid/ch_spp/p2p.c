#define P2P_EXTERN
#include "p2p.h"
#include <stdio.h>

#define p2p_dprintf printf

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void *xx_shmalloc();
void xx_shfree();
void xx_init_shmalloc();

static int MPID_SPP_ppid = 0; 

void p2p_init(memsize, numNodes)
int memsize;
unsigned int numNodes;
{

  caddr_t p2p_start_shared_area;

  p2p_start_shared_area = (char *) mmap((caddr_t) 0, memsize,
                          PROT_READ|PROT_WRITE|PROT_EXEC,
                          MAP_SHARED|MAP_ANONYMOUS
                          |MAP_VARIABLE|CNX_MAP_BLOCK_SHARED, 
                          -1, (off_t) 0);

/*fprintf(stderr,"memsize = %#x, address = %#x\n",
      memsize,p2p_start_shared_area);*/

  if (p2p_start_shared_area == (caddr_t)-1)
  {
    perror("mmap failed");
    p2p_error("OOPS: mmap failed: cannot map shared memory, size=",
              memsize);
  }
  xx_init_shmalloc( p2p_start_shared_area, memsize, numNodes );
}

cnx_node_t
  p2p_migrateInitialProcess(cnx_node_t myNode, unsigned int numNodes,
			    unsigned int *numCPUs)
{
  int i, rc;

  for (i = 0; i < numNodes; i++)
    if (numCPUs[i])
    {
      myNode = i; 
      if ((rc = cnx_sc_fork(CNX_INHERIT_SC, myNode)) == -1)
        p2p_error("p2p_init: fork failed\n",(-1));
      else if (rc)
        exit(0);
      break;
    }
  return myNode;
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

RETSIGTYPE MPID_handle_child( sig, code, scp )
int               sig, code;
struct sigcontext *scp;
/* Some systems have an additional char *addr */
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

void p2p_create_procs(numprocs, procNode, procID)
int numprocs;
unsigned int procNode[];
int *procID;
{
  int i, rc;

  (void) signal( SIGCHLD, MPID_handle_child );
  *procID = 0;
  for (i = 1; i < numprocs; i++)
  {
    if ((rc = cnx_sc_fork(CNX_INHERIT_SC, procNode[i])) == -1)
      p2p_error("p2p_init: fork failed\n",(-1));
    else if (rc == 0)
    {
      if(setpgid(0,MPID_SPP_ppid)) {
          p2p_error("p2p_init: failure in setpgid\n",(-1));
          exit(-1);
      }
      *procID = i;
      return;
    }
    else {
	/* Save pid of child so that we can detect child exit */
	MPID_child_pid[i-1] = rc;
	MPID_numprocs       = i;
	}
  }
}


void *p2p_shmalloc( unsigned int size, cnx_node_t node )
{
  void *p;

  p = (void *) xx_shmalloc(size, node);
/*printf("p2p_shmalloc size = %#x node = %d address = %#x\n", size, node, p); */
  return (p);
}

void p2p_shfree( char *ptr )
{

  (void) xx_shfree(ptr);
}


#include <signal.h>
/* Cleanup is the NORMAL termination code; it may be called in abormal
   termination as well */
p2p_cleanup()
{
}

void
p2p_setpgrp()
{
   MPID_SPP_ppid = getpid();
   if(setpgid(MPID_SPP_ppid,MPID_SPP_ppid)) {
       perror("failure in p2p_setpgrp");
       exit(-1);
   }
}

/*****

void p2p_dprintf(fmt, va_alist)
char *fmt;
va_dcl
{
  va_list ap;
 
  printf("%s: ", p2p_my_dprintf_id);
  va_start(ap);
#if defined(HAVE_VPRINTF)
  vprintf(fmt, ap);
#else
  _doprnt(fmt, ap, stdout);
#endif
  va_end(ap);
  fflush(stdout);
}
 
void p2p_dprintfl(level, fmt, va_alist)
int level;
char *fmt;
va_dcl
{
  va_list ap;
 
  if (level > p2p_debug_level) return;
  printf("%d: %s: ", level, p2p_my_dprintf_id);
  va_start(ap);
#if defined(HAVE_VPRINTF)
  vprintf(fmt, ap);
#else
  _doprnt(fmt, ap, stdout);
#endif
  va_end(ap);
  fflush(stdout);
}

*****/

p2p_error(string,value)
char *string;
int value;
{
  printf("%s %d\n",string, value);
  printf("p2p_error is not fully cleaning up at present\n");
  p2p_cleanup();
  if (MPID_SPP_ppid) 
      kill( -MPID_SPP_ppid, SIGKILL );
  exit(99);
}
#include <sys/time.h>

void p2p_wtime_init()
{
}

#include <sys/cnx_ail.h>
double p2p_wtime()
{
  return toc_read() * 0.000001;
}

/*
   Yield to other processes (rather than spinning in place)
 */
void p2p_yield()
{
#if defined(MPI_IRIX)
sginap(0);

#else
/* Use a short select as a way to suggest to the OS to deschedule the 
   process */
struct timeval tp;
tp.tv_sec  = 0;
tp.tv_usec = 10;
select( 0, (void *)0, (void *)0, (void *)0, &tp );
#endif
}

/*
  Memory management routines from ANSI K&R C, modified to manage
  a single block of shared memory.
  Have stripped out all the usage monitoring to keep it simple.

  To initialize a piece of shared memory:
    xx_init_shmalloc(char *memory, unsigned nbytes)

  Then call xx_shmalloc() and xx_shfree() as usual.
*/

#define LOG_ALIGN 6
#define ALIGNMENT (1 << LOG_ALIGN)

/* ALIGNMENT is assumed below to be bigger than sizeof(p2p_lock_t) +
   sizeof(Header *), so do not reduce LOG_ALIGN below 4 */

union header
{
  struct
  {
    union header *ptr;  /* next block if on free list */
    unsigned size;    /* size of this block */
  } s;
  char align[ALIGNMENT];  /* Align to ALIGNMENT byte boundary */
};

typedef union header Header;

     /* pointer to pointer to start of free list of current interest */
static Header **freep;
#define MPID_MAX_NODES 16
     /* Pointers to locks */
static p2p_lock_t *p2p_shmem_lock[MPID_MAX_NODES];
     /* Pointer to block-specific free-list */
static Header **blockFreePtr[MPID_MAX_NODES];
static Header *region[MPID_MAX_NODES+1];

void xx_init_shmalloc(memory, nbytes, numNodes)
char *memory;
unsigned nbytes;
unsigned int numNodes;
/*
  memory points to a region of shared memory nbytes long.
  initialize the data structures needed to manage this memory
*/
{
  int i;
  unsigned int nunits;
  unsigned int shmemBlockSize;

    /* Quick check that things are OK */

  if (ALIGNMENT != sizeof(Header) ||
      ALIGNMENT < (sizeof(Header *) + sizeof(p2p_lock_t)))
  {
     p2p_dprintf("%d %d\n",sizeof(Header),sizeof(p2p_lock_t));
     p2p_error("xx_init_shmem: Alignment is wrong", ALIGNMENT);
  }

  if (!memory) p2p_error("xx_init_shmem: Passed null pointer", 0);

  shmemBlockSize = nbytes / numNodes;
  nunits = shmemBlockSize >> LOG_ALIGN;
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

  for (i = 0 ; i < numNodes; i++)
  {
    region[i] = (Header *)(memory + i * shmemBlockSize);
            /* Free space pointer in first block  */
    blockFreePtr[i] = (Header **) region[i];
            /* aligned for HP  */
    p2p_shmem_lock[i] = (p2p_lock_t *) ((char *)region[i] + 16);
            /* Data in rest */
    (region[i] + 1)->s.ptr = *blockFreePtr[i] = region[i] + 1;
            /* One header consumed already */
    (region[i] + 1)->s.size = nunits - 1; 
    p2p_lock_init(p2p_shmem_lock[i]);                /* Initialize the lock */
/*fprintf(stderr, "blockFreePtr[i] = %#x *blockFreePtr[i] = %#x\n", blockFreePtr[i], *blockFreePtr[i]); */
  }
  region[numNodes] = (Header *)(memory + nbytes);
}

void *xx_shmalloc(nbytes, node)
unsigned nbytes;
cnx_node_t node;
{
  Header *p, *prevp;
  char *address = (char *) NULL;
  unsigned nunits;

    /* Force entire routine to be single threaded */
  (void) p2p_lock(p2p_shmem_lock[node]);

  nunits = ((nbytes + sizeof(Header) - 1) >> LOG_ALIGN) + 1;

  prevp = *blockFreePtr[node];
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
  {
    if (p->s.size >= nunits)
    {      /* Big enuf */
      if (p->s.size == nunits)  /* exact fit */
        prevp->s.ptr = p->s.ptr;
      else
      {      /* allocate tail end */
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      *blockFreePtr[node] = prevp;
      address = (char *) (p + 1);
      break;
    }
    if (p == *blockFreePtr[node])
    {      /* wrapped around the free list ... no fit
         * found */
      address = (char *) NULL;
      break;
    }
  }

    /* End critical region */
  (void) p2p_unlock(p2p_shmem_lock[node]);

  if (address == NULL)
    p2p_dprintf("xx_shmalloc: returning NULL; requested %d bytes\n",
                nbytes);
  return address;
}

void xx_shfree(ap)
char *ap;
{
  Header *bp, *p;
  int i;
  unsigned int node;

    /* figure out what node block is local to */
  for (i = 0; i < MPID_MAX_NODES; i++)
  {
    if (ap > (char *)region[i] && ap < (char *)region[i+1])
    {
      node = i;
      break;
    }
  }
    /* Begin critical region */
  (void) p2p_lock(p2p_shmem_lock[node]);

  if (!ap) return;      /* Do nothing with NULL pointers */

  bp = (Header *) ap - 1;  /* Point to block header */

  for (p = *blockFreePtr[node]; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
                break;    /* Freed block at start of end of arena */

  if (bp + bp->s.size == p->s.ptr)
  {        /* join to upper neighbour */
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;

  if (p + p->s.size == bp)
  {        /* Join to lower neighbour */
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;

  *blockFreePtr[node] = p;

  /* End critical region */
  (void) p2p_unlock(p2p_shmem_lock[node]);
}

/* interface to hpux locks */
#include "mem.c"
