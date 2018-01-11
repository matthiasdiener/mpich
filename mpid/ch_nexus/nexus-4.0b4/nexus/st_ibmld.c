/*
 * st_ibmld.c
 *
 * Context loading using IBM load
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_ibmld.c,v 1.13 1996/10/07 04:40:17 tuecke Exp $";

#include "internal.h"
#include "ibm_cpp_load.h"
#include <sys/ldr.h>

static nexus_bool_t	st_ibmld_start_context(char *executable_path,
				       int context_size,
				       nexus_global_pointer_t *reply_gp,
				       int checkin_number,
				       int *return_code,
				       nexus_context_t **new_local_context);

static int st_ibmld_destroy_context(nexus_context_t *context);

static nexus_startup_funcs_t st_ibmld_funcs =
{
    NULL /* st_ibmld_preinit */,
    NULL /* st_ibmld_get_master_node_name */,
    NULL /* st_ibmld_init */,
    NULL /* st_ibmld_shutdown */,
    NULL /* st_ibmld_abort */,
    NULL /* st_ibmld_start_node */,
    st_ibmld_start_context,
    NULL /* st_ibmld_register_handlers */
};

typedef struct _ibmld_handle_t
{
    void *     dlhandle;
    char *     filename;
} ibmld_handle_t;

/*
 * _nx_st_ibmld_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_ibmld_info(void)
{
    return ((void *) (&st_ibmld_funcs));
}

/*
 * st_ibmld_start_context()
 *
 * Create a context in the same process as the calling thread, using
 * 'executable_name'.
 *
 * Return: If this function cannot be used, return NEXUS_FALSE.  In this
 *	case, the startup interface will try to start the local process
 *	using another startup module.
 *	Otherwise, return NEXUS_TRUE, and set 'return_code' to 0 on
 *	success or non-0 on failure.
 */
static nexus_bool_t st_ibmld_start_context(char *executable_path,
				     int context_size,
				     nexus_global_pointer_t *reply_gp,
				     int checkin_number,
				     int *return_code,
				     nexus_context_t **new_local_context)
{
    ibmld_handle_t *handle;
    char *dload_filename;
    int filenamelen=20;
    int rc;
/*
 * assign a unique extension to the executable_path and create
 * a symbolic link to it in /tmp
 * record the name in the context structure
 */
    nexus_debug_printf(1, ("st_ibmld_start_context(): creating context %s\n", executable_path));

    filenamelen += (strlen(executable_path) +
		    strlen(_nx_master_id_string) +
		    strlen(_nx_my_node.name));

    NexusMalloc(st_ibmld_start_context(),
		dload_filename,
		char *,
		filenamelen);
	
    nexus_stdio_lock();
    sprintf(dload_filename,"/tmp/nx_%s_%s_%d_%x", 
	    _nx_master_id_string,
	    _nx_my_node.name, _nx_my_node.number,
	    context);
    nexus_stdio_unlock();

    rc = symlink( executable_path, dload_filename );
    if (rc) {
	nexus_printf("st_ibmld_start_context(): error creating symlink, rc=%d\n", rc);
	*return_code = rc;
	return NEXUS_FALSE;
    }

    NexusMalloc(st_ibmld_start_context(),
		handle,
		ibmld_handle_t *,
		sizeof(ibmld_handle_t));

    handle->filename = dload_filename;

    /*
     * allocate the context
     */
    context = _nx_context_alloc(context_size, NEXUS_FALSE);
    
    /*
     * load process into address space
     */
    handle->dlhandle = loadAndInit( dload_filename, 1, NULL );
    if (handle->dlhandle == NULL) {
	nexus_printf("st_ibmld_start_context(): Error attempting to load( %s, 1, NULL );\n", dload_filename);
	nexus_printf("st_ibmld_start_context(): Error number:%d\n", errno);
	return NEXUS_FALSE;
    }

    nexus_debug_printf(1, ("st_ibmld_start_context(): load() returned\n"));

    context->handle = (void *)handle;

    /*
     * NexusBoot() will be called later but record a pointer to
     * that function in the context structure so that the right one
     * will be called
     */
    /*
    context->pfnNexusBoot = (int (*)())dlsym(handle->dlhandle, "NexusBoot");
    if (context->pfnNexusBoot == NULL ) {
	nexus_printf("st_ibmld_start_context(): Fatal Error: Cannot find NexusBoot() in new context\n");
	nexus_printf("st_ibmld_start_context(): dlerror string: %s\n", dlerror());
	return NEXUS_FALSE;
    }
    */
    context->pfnNexusBoot = (int (*)())(handle->dlhandle);
    context->destructofunc = st_ibmld_destroy_context;

    nexus_debug_printf(1, ("st_ibmld_start_context(): NexusBoot:%x\n",context->pfnNexusBoot));
    nexus_debug_printf(1, ("st_ibmld_start_context(): destruction func:%x\n", context->destructofunc));

    /*
     * return success or failure
     */
    nexus_debug_printf(1, ("st_ibmld_start_context(): returning successfully\n"));

    *return_code = 0;
    *new_local_context = context;
    return(NEXUS_TRUE);

} /* st_ibmld_start_context() */


static int st_ibmld_destroy_context(nexus_context_t *context)
{
    int rc;

    nexus_debug_printf(1, ("st_ibmld_destroy_context(): beginning destruction\n"));

    /* do the dlclose thing */
    rc = terminateAndUnload( ((ibmld_handle_t *)context->handle)->dlhandle );
    if (rc) {
	nexus_printf("st_ibmld_destroy_context(): error calling dlclose(). return code = %d\n", rc);
	return rc;
    }

    nexus_debug_printf(1, ("st_ibmld_destroy_context(): module unloaded\n"));

    /* remove that silly symlink in /tmp */

    rc = unlink( ((ibmld_handle_t*)context->handle)->filename );
    if (rc) {
	nexus_printf("st_ibmld_destroy_context(): error removing symbolic link. rc=%d\n", rc);
	return rc;
    }

    NexusFree( ((ibmld_handle_t *)context->handle)->filename );
    NexusFree( context->handle );

    nexus_debug_printf(1, ("st_ibmld_destroy_context(): returning\n"));

    return 0;
}
