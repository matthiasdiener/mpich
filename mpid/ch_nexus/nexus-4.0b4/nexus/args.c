/*
 * args.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/args.c,v 1.52 1996/12/11 05:41:23 tuecke Exp $";

#include "internal.h"

/*
 * package_id_*
 *
 * Various incarnations of the package id
 */
static char *package_id_base;
static char *package_id;
static char *package_id_end;
static char *package_id_prefix;
static int   package_id_prefix_len;
static char *package_id_nf_prefix;
static int   package_id_nf_prefix_len;

/*
 * MAX_ARGC
 *
 * Maximum number of arguments that can handled.
 */
#define MAX_ARGC 1024

/*
 * Retained arguments.
 *
 * Any package arguments that remain at the end of nexus_init()
 * will be copied into retained_argc/retained_argv by _nx_args_cleanup().
 * It is assumed that all such arguments are double arguments,
 * (i.e. "-nx_foo bar"), unless they are explicitly listed
 * in the singleton_arguments[] array.
 *
 * Retained arguments are needed in order to get arguments to
 * dynamically loaded packages.
 *
 * These retained arguments are accessible by using
 * nexus_find_argument() and nexus_remove_arguments() with NULL argc/argv.
 *
 * When a new process is to be created, _nx_new_process_params() will
 * automatically put each retained argument onto the front of
 * the argument list, unless that argument begins with "nf_" 
 * (i.e. "-nx_nf_foo").  ("nf" stand for "not forwarded")
 *
 * retained_arg_count[i] contains the number of argument positions
 * used by the nexus argument contained in retained_argv[i]
 */
static int	retained_argc;
static char **	retained_argv;
static int	retained_arg_count[MAX_ARGC];

static char *	singleton_arguments[] =
{
    "-nomap",
    NULL
};

/*
 * Storage for arguments
 */
static int	local_argc;
static char *	local_argv[MAX_ARGC];

/*
 * Parameters for this process.
 */
char *_nx_my_process_params = (char *) NULL;

/*
 * Variable to stow away some of the initialization arguments
 */
static void (*usage_message_func_save)(void) = NULL;
static int  (*new_process_params_func_save)(char *, int) = NULL;

/*
 * Forward function declarations
 */
static void usage_message(void);


/*
 * nexus_find_argument()
 */
int nexus_find_argument(int *argc,
			char ***argv,
			char *arg,
			int count)
{
    return ( ports0_find_argument(argc, argv, arg, count));
} /* nexus_find_argument() */


/*
 * nexus_remove_arguments()
 */
void nexus_remove_arguments(int *argc,
			    char ***argv,
			    int arg_num,
			    int count)
{
    ports0_remove_arguments(argc, argv, arg_num, count);
} /* nexus_remove_arguments() */


/*
 * nexus_get_retained_arguments()
 */
void nexus_get_retained_arguments(int **argc, char ****argv)
{
    *argc = &retained_argc;
    *argv = &retained_argv;
} /* nexus_get_retained_arguments() */


/*
 * _nx_get_args()
 *
 * Arguments can come from three sources:
 *	1) Environment variable
 *	2) A parameter string that is passed to a new process
 *	3) Command line (argc/argv)
 *
 * They should be handled in this order, with later arguments
 * overriding earlier arguments if there are duplicates.
 *
 * This routine packages these various arguments up into a
 * coherent whole, and puts them into argc/argv.
 */
void _nx_get_args(int *argc,
		  char ***argv,
		  char *args_env_variable,
		  char *package_designator,
		  void (*usage_message_func)(void),
		  int (*new_process_params_func)(char *, int),
		  nexus_bool_t ignore_command_line_args)
{
    char *env_params;
    int i;
    int tmp_argc;
    char *tmp_argv[MAX_ARGC];

    /*
     * Save away some of the arguments for future use
     */
    usage_message_func_save = usage_message_func;
    new_process_params_func_save = new_process_params_func;
     
    /*
     * Get the parameters that are in the environment variable,
     * and fill them into package_argv.
     */
    local_argc = 1;
    if (args_env_variable)
    {
	env_params = getenv(args_env_variable);
    }
    else
    {
	env_params = (char *) NULL;
    }
    if (env_params)
    {
	/* Need to wrap these with -nx and -nx_end */
	local_argc = _nx_split_args(env_params,
				    local_argc,
				    MAX_ARGC,
				    local_argv,
				    (char *) NULL);
	if (local_argc < 0)
	{
	    if (local_argc == -1)
	    {
		printf("_nx_get_args(): Error: Mismatched quotes in the environment variable %s\n", args_env_variable);
	    }
	    else if (local_argc == -2)
	    {
		printf("_nx_get_args(): Error: Too many arguments in the environment variable %s\n", args_env_variable);
	    }
	    _nx_md_exit(1);
	}
    }

    /*
     * Now add any _nx_my_process_params to local_argv
     */
    if (_nx_my_process_params)
    {
	local_argc = _nx_split_args(_nx_my_process_params,
				    local_argc,
				    MAX_ARGC,
				    local_argv,
				    (char *) NULL);
				       
	if (local_argc < 0)
	{
	    if (local_argc == -1)
	    {
		printf("_nx_get_args(): Error: Mismatched quotes in my new process parameters\n");
	    }
	    else if (local_argc == -2)
	    {
		printf("_nx_get_args(): Error: Too many arguments in my new process parameters + environment variable %s\n", args_env_variable);
	    }
	    _nx_md_exit(1);
	}
    }

    if (!ignore_command_line_args)
    {
	/*
	 * Copy over any command line arguments.
	 * Use the user supplied argc/argv, if argc > 0.
	 * Otherwise, try to fetch the command line arguments
	 * by some other machine dependent method.
	 *
	 * Fill in local_argv[0] correctly with the program name from argv[0].
	 */
	if (*argc > 0)
	{
	    local_argv[0] = _nx_copy_string((*argv)[0]);
	    for (i = 1; i < *argc; i++)
	    {
		if (local_argc >= MAX_ARGC)
		{
		    printf("_nx_get_args(): Error: Too many arguments in argc/argv + new process parameters + environment variable %s\n", args_env_variable);
		    _nx_md_exit(1);
		}
		local_argv[local_argc++] = _nx_copy_string((*argv)[i]);
	    }
	}
	else
	{
	    tmp_argc = _nx_md_get_command_line_args(0, MAX_ARGC, tmp_argv);
	    if (tmp_argc <= 0)
	    {
		tmp_argv[0] = _nx_copy_string("");
	    }
	    local_argv[0] = tmp_argv[0];
	    for (i = 1; i < tmp_argc; i++)
	    {
		if (local_argc >= MAX_ARGC)
		{
		    printf("_nx_get_args(): Error: Too many arguments in my fetched argc/argv + new process parameters + environment variable %s\n", args_env_variable);
		    _nx_md_exit(1);
		}
		local_argv[local_argc++] = tmp_argv[i];
	    }
	}
    }
    else
    {
	/*
	 * If we are ignoring the command line arguments,
	 * then we just need to fill in local_argv[0].
	 */
	if (*argc > 0)
	{
	    local_argv[0] = _nx_copy_string((*argv)[0]);
	}
	else
	{
	    tmp_argc = _nx_md_get_command_line_args(0, MAX_ARGC, tmp_argv);
	    if (tmp_argc <= 0)
	    {
		tmp_argv[0] = _nx_copy_string("");
	    }
	    local_argv[0] = tmp_argv[0];
	}
    }
    
    /*
    for(i = 0; i < local_argc; i++)
    {
	printf("(%d) local_argv[%d]=\"%s\"\n",
	       _nx_md_getpid(), i, local_argv[i]);
    }
    */

    /*
     * Return the new argc/argv
     */
    *argc = local_argc;
    *argv = local_argv;

} /* _nx_get_args() */


/*
 * _nx_args_init()
 *
 * Extract any global arguments from argc/argv
 */
void _nx_args_init(int *argc, char ***argv)
{
    int len;
    int arg_num;
    
    /*
     * Stow away the package_id
     */
    package_id_base = _nx_copy_string(ports0_get_package_id());
    len = strlen(package_id_base);

    NexusMalloc(_nx_args_init(), package_id, char *, len+2);
    NexusMalloc(_nx_args_init(), package_id_end, char *, len+6);
    NexusMalloc(_nx_args_init(), package_id_prefix, char *, len+3);
    NexusMalloc(_nx_args_init(), package_id_nf_prefix, char *, len+6);
    sprintf(package_id, "-%s", package_id_base);
    sprintf(package_id_end, "%s_end", package_id);
    sprintf(package_id_prefix, "%s_", package_id);
    package_id_prefix_len = strlen(package_id_prefix);
    sprintf(package_id_nf_prefix, "%s_nf_", package_id);
    package_id_nf_prefix_len = strlen(package_id_nf_prefix);

    /*
     * Initialize variable to hold arguments
     */
#ifdef BUILD_DEBUG    
    _nx_debug_level = -1;
    _nx_num_debug_levels = 0;
#endif

    _nx_stdout = stdout;
    /*
    _nx_stdout = stderr;
    */

    _nx_pause_on_fatal = NEXUS_FALSE;
    _nx_pause_on_startup = NEXUS_FALSE;
    _nx_dont_start_processes = NEXUS_FALSE;
    _nx_dont_expand_executables = NEXUS_FALSE;
    _nx_skip_poll_count = 0;

    /*
     * Get the arguments
     */
    if (   ((arg_num = nexus_find_argument(argc, argv, "help", 1)) >= 0)
	|| ((arg_num = nexus_find_argument(argc, argv, "h", 1)) >= 0) )
    {
	usage_message();
	_nx_md_exit(0);
    }
    
    if ((arg_num = nexus_find_argument(argc, argv, "Dnexus", 2)) >= 0)
    {
#ifdef BUILD_DEBUG
	_nx_debug_level = atoi((*argv)[arg_num + 1]);

	_nx_debug_levels[0].catagory  = 0xffffffff;
	_nx_debug_levels[0].module    = 0xffffffff;
	_nx_debug_levels[0].operation = 0xffffffff;
	_nx_debug_levels[0].level     = _nx_debug_level;

	_nx_num_debug_levels = 1;

	nexus_remove_arguments(argc, argv, arg_num, 2);
#else /* BUILD_DEBUG */
	printf("The -Dnexus option is not supported in the non-debugging Nexus library.\n");
	_nx_md_exit(1);
#endif /* BUILD_DEBUG */
    }

    if ((arg_num = nexus_find_argument(argc, argv, "dbg_state", 2)) >= 0)
    {
#ifdef BUILD_DEBUG
	int level=0;
	int idx=0;
	char *arg_string, *end_pointer;
	unsigned long *dbg_states;

	dbg_states = (unsigned long *)_nx_debug_levels;
	arg_string=(*argv)[arg_num + 1];
/*	printf("argstring: %s\n",arg_string); */
	do 
	{
	    dbg_states[4*level+idx] = strtoul(arg_string, &end_pointer, 16);
	    if( end_pointer == arg_string )
	    {
		printf("Error forming debug level structure\n");
		_nx_md_exit(1);
	    }
	    switch(*end_pointer)
	    {
	    case ':':
		idx++;
		arg_string = end_pointer + 1;
/*		printf("found :\n"); */
		break;
	    case '#':
		idx=0;
		level++;
		arg_string = end_pointer + 1;
/*		printf("found #\n"); */
		break;
	    case '\0':
		break;
	    default:
		printf("Bad character %c/%d in debug state string\n",*end_pointer,*end_pointer);
		_nx_md_exit(1);
	    }
	} while (*end_pointer != '\0');
	_nx_num_debug_levels = level+1;
	nexus_remove_arguments(argc, argv, arg_num, 2);
#else /* BUILD_DEBUG */
	printf("The -dbg_state option is not supported in the non-debugging Nexus library.\n");
	_nx_md_exit(1);
#endif /* BUILD_DEBUG */
    }    
    
    if ((arg_num = nexus_find_argument(argc, argv, "pause_on_fatal", 1)) >= 0)
    {
	_nx_pause_on_fatal = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    
    if ((arg_num = nexus_find_argument(argc, argv, "pause_on_startup", 1)) >=0)
    {
	_nx_pause_on_startup = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    
    if ((arg_num = nexus_find_argument(argc, argv, "nostart", 1)) >= 0)
    {
	_nx_dont_start_processes = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    
    if ((arg_num = nexus_find_argument(argc, argv, "nonameexpand", 1)) >= 0)
    {
	_nx_dont_expand_executables = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    
    if ((arg_num = nexus_find_argument(argc, argv, "skip_poll", 2)) >= 0)
    {
	_nx_skip_poll_count = atoi((*argv)[arg_num + 1]);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }

} /* _nx_args_init() */


/*
 * get_arg_count()
 */
static int get_arg_count(char *arg, int remaining_args)
{
    int i;
    int count;
    
    for (i = 0;
	 (   (singleton_arguments[i] != (char *) NULL)
	  && (strcmp(singleton_arguments[i], arg) != 0) );
	 i++)
	;
    if (   (singleton_arguments[i] == (char *) NULL)
	&& (remaining_args >= 2) )
    {
	count = 2;
    }
    else
    {
	count = 1;
    }
    return (count);
} /* get_arg_count() */


/*
 * _nx_args_cleanup()
 *
 * Remove any remaining nexus arguments from argc/argv and
 * put them in retained_argc/retained_argv.
 */
void _nx_args_cleanup(int *argc, char ***argv)
{
    int current_arg_index;
    char *current_arg;
    int count;
    int arg_count;
    char *arg;
    int i;

    NexusMalloc(_nx_args_cleanup(),
		retained_argv,
		char **,
		sizeof(char *) * (*argc));
    retained_argc = 1;
    retained_argv[0] = _nx_copy_string((*argv)[0]);

    current_arg_index = 0;

    while (current_arg_index < *argc)
    {	
	current_arg = (*argv)[current_arg_index];
	
	if ((strncmp(current_arg,
		     package_id_prefix,
		     package_id_prefix_len) == 0) )
	{
	    /*
	     * Found an argument.
	     *
	     * First decide whether it is a single or double argument.
	     * Then move that argument to the retained argc/argv list.
	     */

	    /* Determine size of the argument */
	    count = get_arg_count(current_arg+package_id_prefix_len,
				  *argc - current_arg_index);
	    retained_arg_count[retained_argc] = count;
	    
	    /* Move the arguments */
	    for (i = 0; i < count; i++)
	    {
		retained_argv[retained_argc++] = (*argv)[current_arg_index+i];
	    }
	    nexus_remove_arguments(argc, argv, current_arg_index, count);
	}
	else if (strcmp(current_arg, package_id) == 0)
        {
	    /*
	     * Got package_id start.
	     * Move all subsequent arguments until package_id_end or
	     * the end of arguments to the retained argc/argv.
	     * Strip the package_id and package_id_end, and turn
	     * all of the other arguments into freestanding arguments
	     * in prepending package_id_prefix.
	     */
	    for (count = 1;
		 (   (current_arg_index + count < *argc)
		  && (strcmp((*argv)[current_arg_index+count],
			     package_id_end) != 0) );
		 count++)
	    {
		current_arg = (*argv)[current_arg_index+count];
		if (current_arg[0] == '-')
		{
		    /*
		     * Turn this into a freestanding argument.
		     * This makes life easier for _nx_new_process_params
		     */
		    arg_count = get_arg_count(current_arg + 1,
					      *argc-current_arg_index-count);
		    retained_arg_count[retained_argc] = arg_count;
		    NexusMalloc(_nx_args_cleanup(),
				arg,
				char *,
				strlen(current_arg)+package_id_prefix_len);
		    strcpy(arg, package_id_prefix);
		    strcpy(arg + package_id_prefix_len, current_arg+1);
		    retained_argv[retained_argc++] = arg;
		}
		else
		{
		    retained_argv[retained_argc++]
			= (*argv)[current_arg_index+count];
		}
	    }
	    if (current_arg_index + count < *argc)
	    {
		/* Skip the package_id_end argument */
		count++;
	    }
	    nexus_remove_arguments(argc, argv, current_arg_index, count);
	}
	else
	{
	    current_arg_index++;
	}
    }
    
} /* _nx_args_cleanup() */


/*
 * usage_message()
 *
 * Print a usage message.
 *
 * This will loop through the various modules, allowing them to
 * print a usage message also.
 */
static void usage_message(void)
{
    /*
     * Print the basic nexus arguments
     */
    printf("Usage: %s [<user_arguments>] [%s <runtime_arguments>]\n",
	   local_argv[0],
	   package_id );
    printf("    -help or -h               : Print this help message\n");
    printf("    -Dnexus <integer>         : Set the nexus debug level\n");
    printf("    -pause_on_fatal           : When a fatal error occurs, print its pid and\n");
    printf("                                pause the process.  This is useful for\n");
    printf("                                attaching a debugger when a process dies.\n");
    printf("    -nostart                  : Print message instead of starting processes\n");
    printf("    -nonameexpand             : When determining executable names for new\n");
    printf("                                processes, don't expand relative paths.\n");

    
    /*
     * Print usage message for the various modules.
     */
    ports0_usage_message();
    _nx_md_usage_message();
#ifndef BUILD_LITE
    _nx_thread_usage_message();
#endif /* BUILD_LITE */
    _nx_proto_usage_message();
#ifdef HAVE_SHM
    _nx_shm_usage_message();
#endif
    _nx_process_usage_message();
    _nx_context_usage_message();
#ifdef DEPRICATED_HANDLER
    _nx_handler_usage_message();
#endif
    _nx_fault_tolerance_usage_message();
    _nx_startup_usage_message();
    _nx_rdb_usage_message();
    _nx_commlink_usage_message();
    _nx_attach_usage_message();
    
    if (usage_message_func_save)
    {
	(usage_message_func_save)();
    }
    
} /* usage_message() */


/*
 * _nx_new_process_params()
 *
 * Construct the complete new_process_params strings to pass to
 * a new process.  This string should contain all runtime configuration
 * arguments.
 *
 * 'buf' points to an already allocated character array, into which
 * we will write arguments in string form.  'size' is the number of
 * characters left in 'buf'.
 *
 * 'node_number' is passed only to the protocol modules.
 *
 * This routine writes its arguments in string form into 'buf', up
 * to a maximum of 'size' characters.  It should NULL terminate
 * 'buf' when it is done.
 *
 * Return: The number of characters written to 'buf'.
 */
int _nx_new_process_params(char *buf, int size)
{
    int i;
    int n_left;
    int n_added;
    char *b;
    char tmp_buf1[512];
    char tmp_buf2[512];

    n_left = size - 1; /* leave room for null termination */
    b = buf;
    *b = '\0';

    /*
     * Put the basic nexus arguments into the buffer
     */
    tmp_buf1[0] = '\0';

#ifdef BUILD_DEBUG
    if (_nx_debug_level >= 0)
    {
	sprintf(tmp_buf2, "-Dnexus %d ", _nx_debug_level);
	strcat(tmp_buf1, tmp_buf2);
    }
#endif

#ifdef BUILD_DEBUG
    if (_nx_num_debug_levels > 0 && _nx_debug_level==-1)
    {
	int i=0;
	sprintf(tmp_buf2, "-dbg_state ");
	strcat(tmp_buf1, tmp_buf2);
	while( i < _nx_num_debug_levels )
	{
	    if( i!=0 )
		sprintf(tmp_buf2, "#%lx:%lx:%lx:%lx",
			_nx_debug_levels[i].catagory,
			_nx_debug_levels[i].module,
			_nx_debug_levels[i].operation,
			_nx_debug_levels[i].level);
	    else
		sprintf(tmp_buf2, "%lx:%lx:%lx:%lx",
			_nx_debug_levels[i].catagory,
			_nx_debug_levels[i].module,
			_nx_debug_levels[i].operation,
			_nx_debug_levels[i].level);
	    i++;
	    strcat(tmp_buf1, tmp_buf2);
	}
	strcat(tmp_buf1," ");
    }
#endif /* BUILD_DEBUG */
    
    if (_nx_pause_on_fatal == NEXUS_TRUE)
    {
	strcat(tmp_buf1, "-pause_on_fatal ");
    }

    if (_nx_pause_on_startup == NEXUS_TRUE)
    {
	strcat(tmp_buf1, "-pause_on_startup ");
    }

    if (_nx_dont_expand_executables == NEXUS_TRUE)
    {
	strcat(tmp_buf1, "-nonameexpand ");
    }
    
    if (_nx_skip_poll_count > 0)
    {
	sprintf(tmp_buf2, "-skip_poll %d ", _nx_skip_poll_count);
	strcat(tmp_buf1, tmp_buf2);
    }
    
    n_added = strlen(tmp_buf1);
    if (n_added > n_left)
    {
	nexus_fatal("_nx_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }

    strcpy(b, tmp_buf1);
    b += n_added;
    n_left -= n_added;

    /*
     * Add in the retained arguments that do not start with "nf"
     */
    for (i = 1; i < retained_argc; i++)
    {
	if (strncmp(retained_argv[i],
		    package_id_nf_prefix,
		    package_id_nf_prefix_len) == 0)
	{
	    i += retained_arg_count[i];
	}
	else
	{
	    sprintf(tmp_buf1, "\"%s\" ", retained_argv[i]);
	    n_added = strlen(tmp_buf1);
	    if (n_added > n_left)
	    {
		nexus_fatal("_nx_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
	    }
	    strcat(b, tmp_buf1);
	    b += n_added;
	    n_left -= n_added;
	}
    }
    
    /*
     * Put arguments for the various modules.
     */
#ifndef BUILD_LITE
    n_added = ports0_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
#endif /* BUILD_LITE */
    
    n_added = _nx_md_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
    
#ifndef BUILD_LITE
    n_added = _nx_thread_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
#endif /* BUILD_LITE */

    n_added = _nx_proto_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_process_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_context_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

#ifdef DEPRICATED_HANDLER
    n_added = _nx_handler_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
#endif

    n_added = _nx_startup_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_rdb_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_commlink_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_attach_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

    n_added = _nx_fault_tolerance_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;

#ifdef BUILD_PROFILE    
    n_added = _nx_pablo_new_process_params(b, n_left);
    b += n_added;
    n_left -= n_added;
#endif    

    if (new_process_params_func_save)
    {
	n_added = (new_process_params_func_save)(b, n_left);
	b += n_added;
	n_left -= n_added;
    }
    
    return (size - n_left);
} /* _nx_new_process_params() */


/*
 * _nx_split_args()
 *
 * Split the string 'arg_string' into its component arguments.
 * Place those component arguments into 'argv', starting
 * at the 'starting_argc' position.  Do not go past the 'max_argc'
 * position.
 *
 * 'arg_string' will not be modified.
 *
 * This routine separates arguments by looking for spaces that
 * are not in either single or double quotes.  Quotes will
 * be removed unless they are escaped (preceeded by a \),
 * in which case the \ will be removed.
 *
 * Return: A new argc, which is the element in argv immediately
 *		after the last element that we put into it.
 *	   -1 if there are mismatched quotes in the string
 *	   -2 if there are too many arguments
 */
int _nx_split_args(char *arg_string,
		   int starting_argc,
		   int max_argc,
		   char **argv,
		   char *string_buffer)
{
    char *old, *new;
    char *arg;
    int argc;
    nexus_bool_t in_single_quote = NEXUS_FALSE;
    nexus_bool_t in_double_quote = NEXUS_FALSE;

    if (string_buffer == (char *) NULL)
    {
	NexusMalloc(_nx_split_args(),
		    new,
		    char *,
		    (strlen(arg_string) + (size_t)(1)) );
    }
    else
    {
	new = string_buffer;
    }
    
    argc = starting_argc;
    old = arg_string;
    
    while (*old != '\0')
    {
	/* Skip past leading spaces */
	while (*old == ' ')
	    old++;
	
	arg = new;
	while (*old != '\0')
	{
	    if (*old == ' ')
	    {
		if (!in_single_quote && !in_double_quote)
		{
		    /* space outside a quote */
		    break;
		}
		else
		{
		    /* space inside a quote, so end of argument */
		    *new++ = *old++;
		}
	    }
	    else if (*old == '\\' && (*(old+1) == '\'' || *(old+1) == '\"'))
	    {
		/* an escaped quote */
		*new++ = *(old+1);
		old += 2;
	    }
	    else if (*old == '\'')
	    {
		/* an un-escaped single quote */
		in_single_quote = !in_single_quote;
		old++;
	    }
	    else if (*old == '\"')
	    {
		/* an un-escaped double quote */
		in_double_quote = !in_double_quote;
		old++;
	    }
	    else
	    {
		/* a plain old, boring character */
		*new++ = *old++;
	    }
	}
	
	if (in_single_quote || in_double_quote)
	{
	    return (-1);
	}
	
	*new++ = '\0';
	if (*old != '\0')
	{
	    old++;
	}
	
	argv[argc++] = arg;
	
	if ((argc >= max_argc) && (*old != '\0'))
	{
	    return (-2);
	}
    }

    return (argc);
} /* _nx_split_args() */


/*
 * _nx_get_package_id_start()
 */
char *_nx_get_package_id_start(void)
{
    return (package_id);
} /* _nx_get_package_id_start() */


/*
 * _nx_get_package_id_end()
 */
char *_nx_get_package_id_end(void)
{
    return (package_id_end);
} /* _nx_get_package_id_end() */


/*
 * _nx_get_argv0()
 */
char *_nx_get_argv0(void)
{
    return (local_argv[0]);
} /* _nx_get_argv0() */
