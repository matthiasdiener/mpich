/*
 * module.c
 *
 * Module handling routines.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/module.c,v 1.5 1996/10/07 04:39:59 tuecke Exp $";

#include "internal.h"

/*
 * _nx_module_process_arguments_init()
 *
 * Initialize any data structures that will hold command line arguments.
 */
void _nx_module_process_arguments_init(void)
{
} /* _nx_module_process_arguments_init() */


/*
 * _nx_module_process_arguments()
 *
 * See if the 'current_arg' argument (of the package arguments)
 * is meant for us.  If so, then extract it, and return a
 * new 'current_arg' value.  If not, then just return 'current_arg'
 * unchanged.
 */
int _nx_module_process_arguments(int current_arg, int arg_count)
{
    return (current_arg);
} /* _nx_module_process_arguments() */


/*
 * _nx_module_usage_message()
 */
void _nx_module_usage_message(void)
{
} /* _nx_module_usage_message() */


/*
 * _nx_module_new_process_params()
 */
int _nx_module_new_process_params(char *buf, int size)
{
    return (size);
} /* _nx_module_new_process_params() */


/*
 * _nx_module_init()
 *
 * Initialize the handler table for the passed 'context'.
 */
void _nx_module_init(nexus_context_t *context)
{
} /* _nx_module_init() */


/*
 * _nx_module_load()
 *
 * Load and initialize a module with the given 'family_name'
 * and 'module_name'.
 *
 * Return a pointer to its function table, or NULL if the module
 * could not be successfully loaded.
 *
 * Return: 0 on success, or non-0 on failure
 */
int _nx_module_load(char *family_name,
		    char *module_name,
		    void **entry_return)
{
    nexus_bool_t found_preloaded = NEXUS_FALSE;
    int rc;
    
    /* Check the preloaded module list */

    if (found_preloaded)
    {
	/* If preloaded, then call its init function */
	rc = 1;
    }
    else
    {
	/*
	 * If not preloaded, then try to dynamically load it
	 * from a file.
	 */
	char *file_name[MAX_PATH_LENGTH];
	char *entry_function_name[64];

	/* Load the file */
	nexus_stdio_lock();
	sprintf(file_name, "%s_%s.o", family_name, module_name);
	sprintf(entry_function_name, "_nx_%s_%s_init",
		family_name, module_name);
	nexus_stdio_unlock();
	rc = nexus_load_file(file_name, entry_function_name, entry_return);

	if (rc == 0)
	{
	    /* Add the module to the table */
	}
    }

    return(rc);
} /* _nx_module_load() */


/*
 * nexus_load_file()
 *
 * Dynamically load a file into this program, and call the entry function.
 * The entry function should return a (void *) pointer.  Return this
 * value in *entry_return.
 *
 * Return: 0 on success, or non-0 on failure.
 *
 */
int nexus_load_file(char *file_name,
		    char *entry_function_name,
		    void **entry_return)
{
    int rc;

    rc = 1;
    *entry_return = NULL;

    return(rc);
} /* nexus_load_file() */

