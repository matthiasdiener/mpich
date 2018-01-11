#define P2P_EXTERN
#include "p2p.h"
#include <stdio.h>

#define p2p_dprintf printf

#if !defined(MPI_solaris) && !defined(MPI_IRIX) && !defined(MPI_hpux)
    ??? shmem device not defined for this architecture  
#endif

#if defined(MPI_solaris)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void *xx_shmalloc();
void xx_shfree();
void xx_init_shmalloc();

#endif

#if defined(MPI_hpux)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void *xx_shmalloc();
void xx_shfree();
void xx_init_shmalloc();

#endif

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
	p2p_error("p2p_init: usconfig failed: cannot make shared-only\n");
    p2p_sgi_usptr = usinit(p2p_sgi_shared_arena_filename);
    if (p2p_sgi_usptr == NULL)
	p2p_error("p2p_init: usinit failed: can't map shared arena\n",memsize);

#endif

/* USE_XX_SHMALLOC is defined in p2p.h for MPI_solaris and MPI_hpux */
#if defined(USE_XX_SHMALLOC)

    caddr_t p2p_start_shared_area;
    static int p2p_shared_map_fd;
#if defined(MPI_solaris)    
    p2p_shared_map_fd = open("/dev/zero", O_RDWR);
    p2p_start_shared_area = (char *) mmap((caddr_t) 0, memsize,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_SHARED, 
			p2p_shared_map_fd, (off_t) 0);
#endif

#if defined(MPI_hpux)
    p2p_start_shared_area = (char *) mmap((caddr_t) 0, memsize,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_SHARED|MAP_ANONYMOUS|MAP_VARIABLE, 
			-1, (off_t) 0);

    /* fprintf(stderr,"memsize = %d, address = 0x%x\n",
	    memsize,p2p_start_shared_area); */

#endif

    if (p2p_start_shared_area == (caddr_t)-1)
    {
        perror("mmap failed");
	p2p_error("OOPS: mmap failed: cannot map shared memory, size=",
		  memsize);
    }
    xx_init_shmalloc(p2p_start_shared_area,memsize);
#endif

}

void p2p_create_procs(numprocs)
int numprocs;
{
    int i, rc;

    for (i = 0; i < numprocs; i++)
    {
	if ((rc = fork()) == -1)
	{
	    p2p_error("p2p_init: fork failed\n",(-1));
	}
	else if (rc == 0)
	    return;
    }
}


void *p2p_shmalloc(size)
int size;
{
    void *p;

#if defined(MPI_IRIX)
    p = usmalloc(size,p2p_sgi_usptr);
#endif

#if defined(USE_XX_SHMALLOC)
    p = (void *) xx_shmalloc(size);
#endif

    return (p);
}

void p2p_shfree(ptr)
char *ptr;
{

#if defined(MPI_IRIX)
    usfree(ptr,p2p_sgi_usptr);
#endif

#if  defined(USE_XX_SHMALLOC)
  (void) xx_shfree(ptr);
#endif

}


#if defined(MPI_IRIX)

/* p2p_lock and p2p_unlock are defined in p2p.h for IRIX */

/* p2p_lock_init, p2p_lock, and  p2p_unlock 
   are defined in p2p.h for solaris 
*/

#ifdef MPID_CACHE_ALIGN
void p2p_lock_init(L) 
p2p_lock_t *L;
{ 
    (*L).lock = usnewlock(p2p_sgi_usptr);
}
#else
/* this is the spinlock method */
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
#endif /* MPI_IRIX */


p2p_cleanup()
{
 
#if defined(MPI_IRIX)
    unlink(p2p_sgi_shared_arena_filename);
#endif

#if defined(MPI_solaris)
#endif

#if defined(MPI_hpux)
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

p2p_error(string,value)
char * string;
int value;
{
    printf("%s %d\n",string, value);
    printf("p2p_error is not fully cleaning up at present\n");
    p2p_cleanup();
    exit(99);
}
#include <sys/time.h>

void p2p_wtime_init()
{
}

double p2p_wtime()
{
    double timeval;
    struct timeval tp;

#if defined(MPI_IRIX) || defined(MPI_hpux)
    struct timezone tzp;
#endif

#if defined(MPI_IRIX)
    BSDgettimeofday(&tp,&tzp);
#endif

#if defined(MPI_hpux)
    gettimeofday(&tp,&tzp);
#endif

#if defined(MPI_solaris)
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
sginap(0);
#endif

#if defined(MPI_solaris)
#endif

#if defined(MPI_hpux)
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

#define LOG_ALIGN 6
#define ALIGNMENT (1 << LOG_ALIGN)

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
#if defined(MPI_hpux)
    int msemsize = sizeof(struct msemaphore);
#endif

    /* Force entire routine to be single threaded */
    (void) p2p_lock(p2p_shmem_lock);

#if defined(MPI_hpux)
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
/*
#if defined(MPI_hpux)
    while (((int)address % msemsize) != 0)
	address++;
#endif
*/
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
/* interface to hpux locks by Dan Golan */
#include "mem.c"
#endif
