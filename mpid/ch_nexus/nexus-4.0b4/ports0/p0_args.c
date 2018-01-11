/*
 * p0_args.c
 *
 * Argument handling code
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_args.c,v 1.7 1996/01/29 22:47:22 patton Exp $";

#include "p0_internal.h"

#define MAX_PACKAGE_ID_LENGTH 32
static ports0_bool_t package_id_initialized = 0;
static char package_id_base[MAX_PACKAGE_ID_LENGTH+1];
static char package_id[MAX_PACKAGE_ID_LENGTH+2];
static char package_id_end[MAX_PACKAGE_ID_LENGTH+6];
static char package_id_prefix[MAX_PACKAGE_ID_LENGTH+3];
static int  package_id_prefix_len;

/*
 * ports0_set_package_id()
 */
int ports0_set_package_id(char *package_id_base_arg)
{
    if (strlen(package_id_base) > MAX_PACKAGE_ID_LENGTH)
	return(1); /* the package_id is too long */

    strcpy(package_id_base, package_id_base_arg);
    sprintf(package_id, "-%s", package_id_base);
    sprintf(package_id_end, "%s_end", package_id);
    sprintf(package_id_prefix, "%s_", package_id);
    package_id_prefix_len = strlen(package_id_prefix);

    package_id_initialized = 1;
    
    return(0); /* success */
} /* ports0_set_package_id() */


/*
 * ports0_get_package_id()
 */
char *ports0_get_package_id(void)
{
    return (package_id_base);
} /* ports0_get_package_id() */


/*
 * ports0_find_argument()
 *
 * Find the last argument named 'arg', which occupies 'count'
 * argument positions, in argc/argv.
 *
 * If there are multiple occurrances of 'arg' in argc/argv, then
 * delete all but the last one.
 */
int ports0_find_argument(int *argc, char **argv[], char *arg, int count)
{
    ports0_bool_t got_package_id_start;
    int current_arg_index;
    char *current_arg;
    int found_arg_index = -1;
    
    /* Check to make sure things have been initialized */
    if (!package_id_initialized)
	return(-1);
    
    got_package_id_start = PORTS0_FALSE;
    current_arg_index = 0;

    while (current_arg_index < *argc)
    {	
	current_arg = (*argv)[current_arg_index];
	
	if (   ( /* check for arg with package_id_prefix */
		   (strncmp(current_arg,
			    package_id_prefix,
			    package_id_prefix_len) == 0)
		&& (strcmp(current_arg+package_id_prefix_len,
			   arg) == 0)
		&& (current_arg_index + count <= *argc) )
	    || ( /* check for arg between package_id & package_id_end */
		   (got_package_id_start)
		&& (current_arg[0] == '-')
		&& (strcmp(current_arg+1, arg) == 0)
		&& (current_arg_index + count <= *argc) ) )
	{
	    /* Found the argument */
	    if (found_arg_index >= 0)
	    {
		/* We found one previously, so previous one */
		ports0_remove_arguments(argc, argv, found_arg_index, count);
		found_arg_index = current_arg_index - count;
	    }
	    else
	    {
		/* This is the first occurance of this argument */
		found_arg_index = current_arg_index;
		current_arg_index += count;
	    }
	}
	else if (strcmp(current_arg, package_id) == 0)
        {
	    /*
	     * Got package_id start.
	     * All subsequent arguments until package_id_end
	     * are ports0 arguments.
	     */
            got_package_id_start = PORTS0_TRUE;
	    current_arg_index++;
	}
        else if (strcmp(current_arg, package_id_end) == 0)
	{
	    /* Got package_id end. */
	    got_package_id_start = PORTS0_FALSE;
	    current_arg_index++;
	}
	else
	{
	    current_arg_index++;
	}
    }

    return(found_arg_index);
    
} /* ports0_find_argument() */


/*
 * ports0_remove_arguments()
 *
 * Remove arguments arg_num..arg_numb+count-1 from argc/argv.
 *
 * Do NOT free the memory of the argument.  This allows code that
 * grabs arguments to not have to malloc room for them.
 */
void ports0_remove_arguments(int *argc,
			     char **argv[],
			     int arg_num,
			     int count)
{
    int i;

    /* Check to make sure things have been initialized */
    if (!package_id_initialized)
	return;

    /* Check for overflow, and do nothing if that happens */
    if ( (arg_num + count > *argc) || (arg_num < 0) )
	return;

    /* Remove the arguments by packing down argv and decrementing argc */
    for (i = arg_num + count; i < *argc; i++)
    {
	(*argv)[i - count] = (*argv)[i];
    }
    *argc -= count;

    /*
     * Check to see if we are left with adjacent
     * package_id and package_id_end arguments due to the removal
     * of these arguments.
     * If so, then remove those arguments.
     */
    if (   (arg_num > 0)
	&& (arg_num < *argc)
	&& (strcmp((*argv)[arg_num-1], package_id) == 0)
	&& (strcmp((*argv)[arg_num], package_id_end) == 0) )
    {
	ports0_remove_arguments(argc, argv, arg_num-1, 2);
    }

    /*
     * Check to see if we are left with just a package_id argument
     * at the end of argv.
     * If so, then get rid of it.
     */
    if (strcmp((*argv)[*argc-1], package_id) == 0)
    {
	(*argc)--;
    }

} /* ports0_remove_arguments() */


/*
 * _p0_args_init()
 *
 * Grab any arguments that set global ports0 parameters.
 */
void _p0_args_init(int *argc, char **argv[])
{
} /* _p0_args_init() */


/*
 * ports0_usage_message()
 *
 * Print a usage message for each ports0 argument.
 *
 * This will loop through the various modules, allowing them to
 * print a usage message also.
 */
void ports0_usage_message(void)
{
    /* Print usage message for global arguments here */
    
#ifndef BUILD_LITE
    _p0_thread_usage_message();
#endif /* BUILD_LITE */
    
#ifdef PORTS0_MALLOC_DEBUG
    _p0_malloc_debug_usage_message();
#endif
} /* ports0_usage_message() */


#ifndef BUILD_LITE
/*
 * ports0_new_process_params()
 *
 * Add any ports0 arguments to 'buf' that should be passed to new
 * ports0 programs.  'buf' contains 'size' bytes.
 *
 * Return: The number of characters written to 'buf'.
 */
int ports0_new_process_params(char *buf, int size)
{
    int n_left;
    int n_added;
    char *b;

    n_left = size;
    b = buf;

    /* Put the global ports0 arguments into the buffer here */
    
    /*
     * Put arguments for the various modules.
     */
    n_added = _p0_thread_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
    
#ifdef PORTS0_MALLOC_DEBUG
    n_added = _p0_malloc_debug_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
#endif

    return (size - n_left);
} /* ports0_new_process_params() */
#endif /* BUILD_LITE */
