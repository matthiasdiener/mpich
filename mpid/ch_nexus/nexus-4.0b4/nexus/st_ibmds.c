/*
 * st_ibmds.c
 *
 * Context loading using dseg trick on IBM
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_ibmds.c,v 1.15 1996/10/07 04:40:16 tuecke Exp $";

#include "internal.h"
#include <a.out.h>
#undef FREAD
#undef FWRITE
#include <ldfcn.h>
#include <sys/ldr.h>

#include "st_ibmds.h"

static ContextInfo *load_context(char *);

typedef ContextInfo * (*fp)(char *) ; 

static char *buffer[1024];
static char buffer2[1024];

extern char _text[];
extern char _etext[];
extern char _data[];
extern char _edata[];
extern char _end[];

static nexus_bool_t   st_ibmds_preinit(nexus_startup_node_t **,
				 nexus_startup_node_t **);

static nexus_bool_t	st_ibmds_start_context(char *executable_path,
				       int context_size,
				       nexus_global_pointer_t *reply_gp,
				       int checkin_number,
				       int *return_code,
				       nexus_context_t **new_local_context);

static int st_ibmds_destroy_context(nexus_context_t *context);

static nexus_startup_funcs_t st_ibmds_funcs =
{
    NULL /* st_ibmds_preinit */,
    NULL /* st_ibmds_get_master_node_name */,
    NULL /* st_ibmds_init */,
    NULL /* st_ibmds_shutdown */,
    NULL /* st_ibmds_abort */,
    NULL /* st_ibmds_start_node */,
    st_ibmds_start_context,
    NULL /* st_ibmds_register_handlers */
};


/*
 * _nx_st_ibmds_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_ibmds_info(void)
{
    return ((void *) (&st_ibmds_funcs));
}

/*
 * st_ibmds_start_context()
 *
 * Create a context in the same process as the calling thread, using
 * 'executable_name'.
 *
 * Set 'new_local_process' since this new context is not in a
 * separate process.
 *
 * Return: If this function cannot be used, return NEXUS_FALSE.  In this
 *	case, the startup interface will try to start the local process
 *	using another startup module.
 *	Otherwise, return NEXUS_TRUE, and set 'return_code' to 0 on
 *	success or non-0 on failure.
 */
static nexus_bool_t st_ibmds_start_context(char *executable_path,
				     int context_size,
				     nexus_global_pointer_t *reply_gp,
				     int checkin_number,
				     int *return_code,
				     nexus_context_t **new_local_context)
{
    ibmds_handle_t *handle;
    char *load_filename;
    int filenamelen;
    int rc;
    nexus_context_t *context;

    /*
     * assign a unique extension to the executable_path and create
     * a symbolic link to it in /tmp
     * record the name in the context structure
     */
    filenamelen = strlen(executable_path) + 21;

    NexusMalloc(st_ibmds_start_context(),
		load_filename,
		char *,
		filenamelen);
	
    NexusMalloc(st_ibmds_start_context(),
		handle,
		ibmds_handle_t *,
		sizeof(ibmds_handle_t));

    strcpy( load_filename, executable_path );

    handle->filename = load_filename;

    /*
     * allocate the context
     */
    context = _nx_context_alloc(context_size, NEXUS_FALSE);
    
    /*
     * load process into address space  
     */
    nexus_debug_printf(1, ("st_ibmds_start_context(): initiating load_context(%s)\n",executable_path));

    handle->contextPtr = load_context( executable_path );
    if (handle->contextPtr == NULL)
    {
	nexus_printf("st_ibmds_start_context(): Error attempting to load context %s", load_filename);

	return NEXUS_FALSE;
    }

    /*
     * NexusBoot() will be called later but record a pointer to
     * that function in the context structure so that the right one
     * will be called
     */
    context->pfnNexusBoot = handle->contextPtr->NexusBoot;
    context->destructofunc = st_ibmds_destroy_context;
    context->handle = (void *)handle;

    /*
     * return success or failure
     */
    *return_code = 0;
    *new_local_context = context;
    return(NEXUS_TRUE);

} /* st_ibmds_start_context() */


static int st_ibmds_destroy_context(nexus_context_t *context)
{
    nexus_context_t *my_context;
    int remaining_contexts;
    ContextInfo *target_context;

    nexus_debug_printf(2, ("st_ibmds_destroy_context(): entering\n"));

    target_context =
	(ContextInfo *)(((ibmds_handle_t *)(context->handle))->contextPtr);

    _nx_context(&my_context);

    _nx_set_context(context);
    remaining_contexts = (target_context->killfunc)(target_context);
    _nx_set_context(my_context);
    if( remaining_contexts == 0 )
    {
#ifdef NEXUS_CPP_HACK
	nexus_debug_printf(1, ("st_ibmds_destroy_context(): Calling terminateAndUnload()\n"));
	terminateAndUnload(target_context->entry_point);
#else
	nexus_debug_printf(1, ("st_ibmds_destroy_context(): Calling unload()\n"));
	unload(target_context->entry_point);
	nexus_debug_printf(1, ("st_ibmds_destroy_context(): unload() complete\n"));
#endif
    }

    NexusFree( ((ibmds_handle_t *)(context->handle))->filename );
    NexusFree( context->handle );

    return(0);
} /* st_ibmds_destroy_context() */


/*
 * load_context()
 */
static ContextInfo *load_context(char *filename) 
{

    fp entry_point;
    fp aux_entry_point;
    long aux_entry_point_offset;
    long lentry_point;
    long *memptr;
    ContextInfo * contextPtr;
    
    nexus_debug_printf(1, ("load_context(): Initiating OS dynamic load:%s\n",filename));
#ifdef NEXUS_CPP_HACK
    nexus_debug_printf(1, ("load_context(): Calling loadAndInit()\n"));
    entry_point = (fp) loadAndInit(filename,1,NULL);
#else
    nexus_debug_printf(1, ("load_context(): Calling load()\n"));
    entry_point = (fp) load(filename,1,NULL);
#endif
    nexus_debug_printf(1, ("load_context(): Returned from OS dynamic load... entry point:%x\n", entry_point));
    
    /* Try to dynamically load the specified file */
    buffer[0] = "execerror";
    buffer[1] = "first try on load";
    if (entry_point == (fp) NULL)
    {
	loadquery(L_GETMESSAGES,&buffer[2],sizeof buffer - 8);
	nexus_printf("load_context: attempted load of %s\n",filename);
	perror("load_context"); 
	execvp("/usr/sbin/execerror",buffer);
    }

    /*
     * determine the location of the the auxiliary entry point
     */
    
    memptr = (long *)entry_point;
    lentry_point = (long)entry_point;

    if (memptr[1] == memptr[3]) {
	/* 2 word function descriptor ... aux_ent_off at offset 4 */
	aux_entry_point_offset = (long)(memptr[4]);
    } else if (memptr[1] == memptr[4]) {
	/* 3 word function descriptor ... aux_ent_off at offset 2 */
	aux_entry_point_offset = (long)(memptr[2]);
    } else {
	/* whoops where the hell am i? */
	nexus_printf("Error finding the auxiliary\n");
	return(0);
    }

    aux_entry_point = (fp)(lentry_point + aux_entry_point_offset);

    contextPtr = (aux_entry_point)(filename);
    contextPtr->entry_point = entry_point; /* store real entry point */

    return(contextPtr);
} /* load_context() */
