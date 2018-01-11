/*
 * p0_malloc.c
 *
 * Malloc debugging stuff, and reentrant versions of malloc routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_malloc.c,v 1.7 1996/02/28 20:44:05 patton Exp $";

#include "p0_internal.h"



#ifdef PORTS0_MALLOC_NOT_REENTRANT

/*
 * ports0_reentrant_malloc()
 *
 * Reentrant version of malloc().
 */
void *ports0_reentrant_malloc(size_t bytes)
{
    void *rc;

    if (!_p0_ports0_init_called)
    {
	ports0_preinit();
	rc = malloc(bytes);
    }
    else
    {
        ports0_reentrant_lock();
        rc = malloc(bytes);
        ports0_reentrant_unlock();
    }
    return(rc);
} /* ports0_reentrant_malloc() */


/*
 * ports0_reentrant_realloc()
 *
 * Reentrant version of realloc().
 */
void *ports0_realloc(void *ptr,
		     size_t bytes)
{
    void *rc;

    if (!_p0_ports0_init_called)
    {
    	ports0_preinit();
	rc = realloc(ptr, bytes);
    }
    else
    {
        ports0_reentrant_lock();
        rc = realloc(ptr, bytes);
        ports0_reentrant_unlock();
    }
    return(rc);
} /* ports0_reentrant_realloc() */


/*
 * ports0_reentrant_calloc()
 *
 * Reentrant version of calloc().
 */
void *ports0_calloc(size_t nobj, 
		    size_t bytes)
{
    void *rc;

    if (!_p0_ports0_init_called)
    {
        ports0_preinit();
	rc = calloc(nobj, bytes);
    }
    else
    {
        ports0_reentrant_lock();
        rc = calloc(nobj, bytes);
        ports0_reentrant_unlock();
    }
    return(rc);
} /* ports0_reentrant_calloc() */


/*
 * ports0_reentrant_free()
 *
 * Reentrant version of free().
 */
void ports0_free(void *ptr)
{

    if (!_p0_ports0_init_called)
    {
	ports0_preinit();
	free(ptr);
    }
    else
    {
        ports0_reentrant_lock();
        free(ptr);
        ports0_reentrant_unlock();
    }
} /* ports0_reentrant_free() */

#endif /* PORTS0_MALLOC_NOT_REENTRANT */


#ifdef PORTS0_MALLOC_DEBUG

/*
 * Memory allocation debugging and diagnostics code.
 */


#ifndef PORTS0_MALLOC_PAD
#define PORTS0_MALLOC_PAD _p0_malloc_pad_size
#endif

typedef struct _malloc_rec_t
{
    char *addr;
    char *file;
    char *free_file;
    int size;
    short line;
    short free_line;
    short freed;
} *malloc_rec_t;

#ifndef PORTS0_N_MALLOC_RECS
#define PORTS0_N_MALLOC_RECS 20000
#endif

static struct _malloc_rec_t malloc_recs[PORTS0_N_MALLOC_RECS];
static int next_malloc_rec = 0;

static char last_successful_file[1024];
static int last_successful_line;

static ports0_mutex_t debug_mutex;
static int initialized = 0;

static ports0_bool_t	arg_got_malloc_debug = PORTS0_FALSE;
static ports0_bool_t	arg_got_malloc_pad = PORTS0_FALSE;

#define START_MAGIC 0xf00dface
#define END_MAGIC 0xeeaaddff


#ifdef HAVE_MALLOCERR
static void malloc_error_func(int err)
{
    static char *error_codes[] =
    {
	"vm_allocate failed",
	"vm_deallocate failed",
	"vm_copy failed",
	"attempt to free or realloc freed space",
	"corrupt heap",
	"attempt to free or realloc non-allocated space",
    };

    ports0_fatal("Malloc error: %s (%d)\n",
		 error_codes[err], err);
}
#endif /* HAVE_MALLOCERR */


/*
 * _p0_malloc_debug_usage_message()
 */
void _p0_malloc_debug_usage_message(void)
{
    /* Need to put something here */
} /* _p0_malloc_debug_usage_message() */


/*
 * _p0_malloc_debug_new_process_params()
 */
int _p0_malloc_debug_new_process_params(char *buf, int size)
{
    char tmp_buf1[256];
    char tmp_buf2[256];
    int len;

    tmp_buf1[0] = '\0';
    
    if (arg_got_malloc_debug)
    {
	sprintf(tmp_buf2, "-Dmalloc %d ", _p0_malloc_debug_level);
	strcat(tmp_buf1, tmp_buf2);
    }
    if (arg_got_malloc_pad)
    {
	sprintf(tmp_buf2, "-malloc_pad %d ", _p0_malloc_pad_size);
	strcat(tmp_buf1, tmp_buf2);
    }
    if (_p0_trace_malloc)
    {
	strcat(tmp_buf1, "-trace_malloc ");
    }
    if (_p0_no_frees)
    {
	strcat(tmp_buf1, "-no_frees ");
    }

    len = strlen(tmp_buf1);
    if (len > size)
    {
	ports0_fatal("_p0_malloc_debug_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    strcpy(buf, tmp_buf1);
    return (len);

} /* _p0_malloc_debug_new_process_params() */


/*
 * _p0_malloc_debug_init()
 */
void _p0_malloc_debug_init(int *argc, char ***argv)
{
    int arg_num;
    
    _p0_malloc_debug_level = 0;
    _p0_malloc_pad_size = 256;
    _p0_trace_malloc = PORTS0_FALSE;
    _p0_no_frees = PORTS0_FALSE;

    if ((arg_num = ports0_find_argument(argc, argv, "Dmalloc", 2)) >= 0)
    {
	arg_got_malloc_debug = PORTS0_TRUE;
	_p0_malloc_debug_level = atoi((*argv)[arg_num+1]);
	ports0_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = ports0_find_argument(argc, argv, "malloc_pad", 2)) >= 0)
    {
	arg_got_malloc_pad = PORTS0_TRUE;
	_p0_malloc_pad_size = atoi((*argv)[arg_num + 1]);
	ports0_remove_arguments(argc, argv, arg_num, 2);
    }
    if ((arg_num = ports0_find_argument(argc, argv, "trace_malloc", 1)) >= 0)
    {
	_p0_trace_malloc = PORTS0_TRUE;
	ports0_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = ports0_find_argument(argc, argv, "no_frees", )) >= 0)
    {
	_p0_no_frees = PORTS0_TRUE;
	ports0_remove_arguments(argc, argv, arg_num, 1);
    }
    
#ifdef HAVE_MALLOCERR
    /* Setup the malloc debugging in NeXT's own malloc package */
    malloc_debug(31);
    malloc_error(malloc_error_func);
#endif

    ports0_mutex_init(&debug_mutex, (ports0_mutexattr_t *) NULL);
    initialized = 1;
} /* _p0_malloc_debug_init() */


/*
 * ports0_debug_malloc()
 *
 * Malloc wrapper that can print out the size and location
 * of allocations when the -Dmalloc argument has been given.
 *
 * The intent is to define a macro of the form
 *
 *	#ifdef BUILD_DEBUG
 *	#define malloc(size) ports0_debug_malloc(size, __FILE__, __LINE__)
 *	#endif
 *
 * in order to trace memory allocation in detail.
 * 
 */
void *ports0_debug_malloc(int size, char *file, int line)
{
    void *rc, *addr;
    malloc_rec_t rec;
    int *p;

    if (initialized && Ports0MallocDebug(2))
    {
	if (Ports0MallocDebug(3))
	{
	    ports0_debug_malloc_check(file, line);
	}
	
	while ((size & 0x07) != 0)
	    size++;
	
	ports0_mutex_lock(&debug_mutex);

	if (next_malloc_rec >= PORTS0_N_MALLOC_RECS)
	{
	    ports0_warning("Too many malloc recs\n");
	    _p0_malloc_debug_level = 1;
	    rc = malloc(size);
	}
	else
	{
	    rec = &malloc_recs[next_malloc_rec];
	    
	    addr = malloc(size + 2 * PORTS0_MALLOC_PAD);
	    rc = (char *) addr + PORTS0_MALLOC_PAD;
	    if (Ports0MallocDebug(5))
	    {
		ports0_printf("malloc(%d) at %s:%d returns %x idx=%d\n",
			    size, file, line, rc, next_malloc_rec);
	    }
	    
	    rec->addr = addr;
	    rec->file = file;
	    rec->line = line;
	    rec->size = size;
	    rec->free_file = (char *) NULL;
	    rec->free_line = -1;
	    rec->freed = 0;

	    if (PORTS0_MALLOC_PAD >= 4)
	    {
		*((int *) addr) = next_malloc_rec;
	    
		for (p = (int *) addr + 1; p < (int *) rc; p++)
		{
		    *p = START_MAGIC;
		}
		for (p = (int *) ((char *) addr + size + PORTS0_MALLOC_PAD);
		     p < (int *) ((char *) addr + size + 2 * PORTS0_MALLOC_PAD); p++)
		{
		    *p = END_MAGIC;
		}
	    }

	    next_malloc_rec++;
	}
	ports0_mutex_unlock(&debug_mutex);
    }
    else
    {
	rc = malloc(size);

	if (_p0_trace_malloc)
	{
	    ports0_printf("malloc(%d) at %s:%d returns %x\n",
			size, file, line, rc);
	}
    }

    return rc;
} /* ports0_debug_malloc() */


/*
 * ports0_debug_malloc_check()
 *
 * Walk the list of allocated blocks looking for munged memory.
 */
void ports0_debug_malloc_check(char *file, int line)
{
    int i;
    malloc_rec_t rec;
    int *p;

    if (PORTS0_MALLOC_PAD < 4)
	return;

    if (!initialized)
	return;
    
    ports0_mutex_lock(&debug_mutex);
    for (i = 0; i < next_malloc_rec; i++)
    {
	rec = &malloc_recs[i];

	if (rec->freed)
	    continue;

	if (*((int *) rec->addr) != i)
	{
	    ports0_fatal("Malloc check (start) failed for idx %d at %s:%d for allocation at %s:%d of size %d. Last successful check was %s:%d\n",
			i,
			file, line,
			rec->file, rec->line,
			rec->size,
			last_successful_file,
			last_successful_line);
	}

	for (p = (int *) rec->addr + 1; p < (int *) ((char *) rec->addr + PORTS0_MALLOC_PAD); p++)
	{
	    if (*p != START_MAGIC)
	    {
		ports0_fatal("Malloc check (start) failed for idx %d at %s:%d for allocation at %s:%d of size %d Last successful check was %s:%d\n",
			    i,
			    file, line,
			    rec->file, rec->line,
			    rec->size,
			    last_successful_file,
			    last_successful_line);
	    }
	}
	
	for (p = (int *) ((char *) rec->addr + rec->size + PORTS0_MALLOC_PAD);
	     p < (int *) ((char *) rec->addr + rec->size + 2 * PORTS0_MALLOC_PAD); p++)
	{
	    if (*p != END_MAGIC)
	    {
		ports0_fatal("Malloc check (end) failed for idx %d at %s:%d for allocation at %s:%d of size %d Last successful check was %s:%d\n",
			    i,
			    file, line,
			    rec->file, rec->line,
			    rec->size,
			    last_successful_file,
			    last_successful_line);
	    }
	}
    }
    strcpy(last_successful_file, file);
    last_successful_line = line;
    ports0_mutex_unlock(&debug_mutex);
} /* ports0_debug_malloc_check() */

void ports0_debug_mem_check(int size, void *address)
{
    int i;
    malloc_rec_t rec;
    char *pad1_start, *pad1_end, *pad2_start, *pad2_end, *a_start, *a_end;

    if (!initialized)
	return;
    
    ports0_mutex_lock(&debug_mutex);
    for (i = 0; i < next_malloc_rec; i++)
    {
	rec = &malloc_recs[i];

	if (rec->freed)
	    continue;

	pad1_start = rec->addr;
	pad1_end = pad1_start + PORTS0_MALLOC_PAD;

	pad2_start = rec->addr + rec->size + PORTS0_MALLOC_PAD;
	pad2_end = pad2_start + PORTS0_MALLOC_PAD;

	a_start = address;
	a_end = a_start + size - 1;

	if ((a_start >= pad1_start && a_start < pad1_end) ||
	    (a_end >= pad1_start && a_end < pad1_end) ||
	    (a_start >= pad2_start && a_start < pad2_end) ||
	    (a_end >= pad2_start && a_end < pad2_end) ||
	    (a_start < pad1_start && a_end > pad1_end) ||
	    (a_start < pad2_start && a_end > pad2_end))
	{
	    ports0_fatal("Malloc memory check for address %x length %s failed for idx %d for allocation at %s:%d of size %d.\n",
			address,
			size,
			i,
			rec->file, rec->line,
			rec->size);
	}
    }
    ports0_mutex_unlock(&debug_mutex);
} /* ports0_debug_mem_check() */


/*
 * ports0_debug_show_freed_blocks()
 *
 * Walk the list of allocated blocks looking blocks that
 * were never freed.
 */
void ports0_debug_show_freed_blocks()
{
    int i;
    malloc_rec_t rec;

    for (i = 0; i < next_malloc_rec; i++)
    {
	rec = &malloc_recs[i];

	if (!rec->freed)
	{
	    ports0_printf("Unfreed block %d size=%5d at %s:%d\n",
			i, rec->size, rec->file, rec->line);
	}
    }
} /* ports0_debug_show_freed_blocks() */


/*
 * ports0_debug_show_malloc_stats()
 *
 * Print a summary of memory allocation statistics.
 */
void ports0_debug_show_malloc_stats(void)
{
    int i;
    malloc_rec_t rec;
    int bytes, bytes_freed;
    int n_blocks_freed;

    bytes = bytes_freed = n_blocks_freed = 0;

    for (i = 0; i < next_malloc_rec; i++)
    {
	rec = &malloc_recs[i];

	bytes += rec->size;

	if (rec->freed)
	{
	    bytes_freed += rec->size;
	    n_blocks_freed++;
	}
    }

    ports0_printf("Malloc statistics:\n");
    ports0_printf("\tbytes allocated:  %d\n", bytes);
    ports0_printf("\tbytes freed:      %d\n", bytes_freed);
    ports0_printf("\tbytes unfreed:    %d\n", bytes - bytes_freed);
    ports0_printf("\tblocks allocated: %d\n", next_malloc_rec);
    ports0_printf("\tblocks freed:     %d\n", n_blocks_freed);
    ports0_printf("\tblocks unfreed:   %d\n", next_malloc_rec - n_blocks_freed);
} /* ports0_debug_show_malloc_stats() */


/*
 * ports0_debug_free()
 */
void ports0_debug_free(void *ptr, char *file, int line)
{
    void *addr;
    int idx;

    if (initialized && Ports0MallocDebug(2))
    {
	malloc_rec_t rec;

	ports0_mutex_lock(&debug_mutex);

	if (PORTS0_MALLOC_PAD >= 4)
	{
	    addr = (char *) ptr - PORTS0_MALLOC_PAD;
	
	    idx = *((int *) addr);

	    rec = &(malloc_recs[idx]);

	    if (rec->freed)
	    {
		ports0_fatal("ports0_debug_free(): block %x idx %d allocated at %s:%d was freed twice at %s:%d and %s:%d\n",
			    ptr, idx,
			    rec->file, rec->line,
			    rec->free_file, rec->free_line,
			    file, line);
	    }
	
	    rec->freed = 1;
	    rec->free_file = file;
	    rec->free_line = line;
	    free(addr);
	}
	else
	{
	    idx = -1;
	    free(ptr);
	}
	if (Ports0MallocDebug(5))
	{
	    ports0_printf("free(%x) at %s:%d index=%d\n",
			ptr, file, line, idx);
	}
	ports0_mutex_unlock(&debug_mutex);
    }
    else
    {
	if (_p0_trace_malloc)
	{
	    ports0_printf("free(%x) at %s:%d\n",
			 ptr, file, line);
	}

	if (!_p0_no_frees)
	{
	    free(ptr);
	}
    }
} /* ports0_debug_free() */


/*
 * ports0_malloc_debug_level()
 */
ports0_bool_t ports0_malloc_debug_level(void)
{
    return (_p0_malloc_debug_level);
} /* ports0_malloc_debug_level() */

#endif /* BUILD_DEBUG */
