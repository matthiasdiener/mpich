/*
 * st_soldl.c
 *
 * Context loading using Solaris dlopen
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_soldl.c,v 1.13 1996/10/07 04:40:20 tuecke Exp $";

#include "internal.h"
#include <dlfcn.h>

static nexus_bool_t	st_soldl_start_context(char *executable_path,
				       int context_size,
				       nexus_global_pointer_t *reply_gp,
				       int checkin_number,
				       int *return_code,
				       nexus_context_t **new_local_context);

static int st_soldl_destroy_context(nexus_context_t *context);

static nexus_startup_funcs_t st_soldl_funcs =
{
    NULL /* st_soldl_preinit */,
    NULL /* st_soldl_get_master_node_name */,
    NULL /* st_soldl_init */,
    NULL /* st_soldl_shutdown */,
    NULL /* st_soldl_abort */,
    NULL /* st_soldl_start_node */,
    st_soldl_start_context,
    NULL /* st_soldl_register_handlers */,
};

typedef struct _soldl_handle_t
{
    void *     dlhandle;
    char *     filename;
} soldl_handle_t;

/*
 * _nx_st_soldl_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_soldl_info(void)
{
    return ((void *) (&st_soldl_funcs));
}

/*
 * st_soldl_start_context()
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
static nexus_bool_t st_soldl_start_context(char *executable_path,
				     int context_size,
				     nexus_global_pointer_t *reply_gp,
				     int checkin_number,
				     int *return_code,
				     nexus_context_t **new_local_context)
{
    soldl_handle_t *handle;
    char *dload_filename;
    int filenamelen=20;
    int rc;
    nexus_context_t *context;
    
    /*
     * assign a unique extension to the executable_path and create
     * a symbolic link to it in /tmp
     * record the name in the context structure
     */
    filenamelen += (strlen(executable_path) +
		    strlen(_nx_master_id_string) +
		    strlen(_nx_my_node.name));

    NexusMalloc(st_soldl_start_context(),
		dload_filename,
		char *,
		filenamelen);

    /*
     * allocate the context
     */
    context = _nx_context_alloc(context_size, NEXUS_FALSE);
	
    nexus_stdio_lock();
    sprintf(dload_filename,"/tmp/nx_%s_%s_%d_%x", 
	    _nx_master_id_string,
	    _nx_my_node.name, _nx_my_node.number,
	    context);
    nexus_stdio_unlock();

    rc = symlink( executable_path, dload_filename );
    if (rc) {
	nexus_printf("st_soldl_start_context(): error creating symlink, rc=%d\n", rc);
	*return_code = rc;
	return NEXUS_FALSE;
    }

    NexusMalloc(st_soldl_start_context(),
		handle,
		soldl_handle_t *,
		sizeof(soldl_handle_t));

    handle->filename = dload_filename;

    /*
     * load process into address space  (dlopen) calls constructors
     */
    nexus_debug_printf(1, ("st_soldl_start_context(): calling dlopen()\n"));

    handle->dlhandle = dlopen( dload_filename, RTLD_LAZY );
    if (handle->dlhandle == NULL) {
	nexus_printf("st_soldl_start_context(): Error attempting to dlopen( %s, RTLD_LAZY );\n", dload_filename);
	nexus_printf("st_soldl_start_context(): Error string:%s\n", dlerror());
	return NEXUS_FALSE;
    }

    context->handle = (void *)handle;

    /*
     * NexusBoot() will be called later but record a pointer to
     * that function in the context structure so that the right one
     * will be called
     */
    context->pfnNexusBoot = (int (*)(void))dlsym(handle->dlhandle, "NexusBoot");
    if (context->pfnNexusBoot == NULL ) {
	nexus_printf("st_soldl_start_context(): Fatal Error: Cannot find NexusBoot() in new context\n");
	nexus_printf("st_soldl_start_context(): dlerror string: %s\n", dlerror());
	return NEXUS_FALSE;
    }
    context->destructofunc = st_soldl_destroy_context;

    /*
     * return success or failure
     */
    *return_code = 0;
    *new_local_context = context;
    return(NEXUS_TRUE);
    
} /* st_soldl_start_context() */

static int st_soldl_destroy_context(nexus_context_t *context)
{
    int rc;

    /* do the dlclose thing */
    nexus_debug_printf(1, ("st_soldl_destroy_context(): calling dlclose\n"));
    rc = dlclose( ((soldl_handle_t *)context->handle)->dlhandle );
    if (rc) {
	nexus_printf("st_soldl_destroy_context(): error calling dlclose(). return code = %d\n", rc);
	return rc;
    }

    /* remove that silly symlink in /tmp */

    rc = unlink( ((soldl_handle_t*)context->handle)->filename );
    if (rc) {
	nexus_printf("st_soldl_destroy_context(): error removing symbolic link. rc=%d\n", rc);
	return rc;
    }

    NexusFree( ((soldl_handle_t *)context->handle)->filename );
    NexusFree( context->handle );

    return 0;
}
