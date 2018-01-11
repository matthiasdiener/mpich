#include <stdio.h>

#include "nexus.h"

/* uncomment the following line if you want verbose output */
/* #define VERBOSE */

#include "nexus_startup_code.h"

nexus_bool_t test_arg;



nexus_bool_t all_tests_okay;



int NexusBoot (void)
/*
** DESCRIPTION: This function is automatically invoked when a context is
** created and is used to do initialization activities such as registering
** handlers required by the context.
**
** This function must return a return code of zero for the context to
** be successfully initialized.
*/
{
    int rc;


#ifdef VERBOSE
    nexus_printf ("NexusBoot(): entering\n");
#endif
    nexus_register_handlers (system_handlers);
    rc = 0;    
#ifdef VERBOSE
    nexus_printf ("NexusBoot(): exiting, returning %d\n", rc);
#endif
    return (rc);
} /* NexusBoot() */





static int package_args_init (int *argc, char ***argv)
/*
** DESCRIPTION: This function is automatically invoked during Nexus
** initialization because it is listed as a callback function for
** nexus_init() called in main().
**
** This function examines the package arguments (those located after
** the "-nx" argument) for an argument "-test".  If this argument is
** found, the global boolean test_arg is set to NEXUS_TRUE and the
** test argument is removed from the arguments list.  Otherwise, the
** global boolean test_arg is set to NEXUS_FALSE.
**
** The global boolean test_arg is examined later to verify that the test
** argument was correctly parsed out of the command-line arguments.
*/
{
    int arg_num;
    

    if ((arg_num = nexus_find_argument (argc, argv, "test", 1)) >= 0)
    {
	test_arg = NEXUS_TRUE;
	nexus_remove_arguments (argc, argv, arg_num, 1);
    }
    else
    {
	test_arg = NEXUS_FALSE;
    }
    
    return (0);
} /* package_args_init */





void main (int argc, char **argv)
/*
** PROGRAMMER: Unknown
** UPDATED:    Greg Koenig, Argonne National Laboratory, 06-FEB-1996
**
** DESCRIPTION: This program exercises the Nexus functions used to examine
** and manipulate command-line arguments.  The program should be invoked
** with any number of normal command-line arguments and with a Nexus package
** argument (i.e. an argument after "-nx") of "-test".  The program echos
** the supplied command-line arguments and also displays a message indicating
** whether the package argument was successfully located.
**
** A sample invocation of this program is as follows:
**
**    test_args myarg1 myarg2 myarg3 -nx -test
*/
{
    char         **my_argv;
    int          i;
    int          my_argc = 0;
    int          n_nodes;
    nexus_node_t *nodes;


    nexus_init (&my_argc,
		&my_argv,
		"NEXUS_ARGS",
		"nx",
		package_args_init,
		NULL,
		NULL,
		NULL,
		&nodes,
		&n_nodes);

    nexus_start ();

#ifdef VERBOSE
    nexus_printf ("args_test(): argc=%d\n", my_argc);
    for (i = 0; i < my_argc; i++)
    {
	nexus_printf ("args_test(): argv[%d]=%s\n", i, my_argv[i]);
    }
    if (test_arg)
    {
	nexus_printf ("test_args(): got the test argument\n");
    }
    else
    {
	nexus_printf ("test_args(): did not get the test argument\n");
    }
#endif

#ifndef VERBOSE
    all_tests_okay = test_arg;
    all_tests_okay = all_tests_okay && (my_argc == 4);
    if (all_tests_okay)
    {
	all_tests_okay = all_tests_okay && !strcmp (my_argv[1], "test1");
	all_tests_okay = all_tests_okay && !strcmp (my_argv[2], "test2");
	all_tests_okay = all_tests_okay && !strcmp (my_argv[3], "test3");
    }
#endif

    for (i = 0; i < n_nodes; i++)
    {
	nexus_free (nodes[i].name);
	nexus_destroy_global_pointer (&nodes[i].gp);
    }
    nexus_free (nodes);

#ifndef VERBOSE
    if (all_tests_okay)
    {
	printf ("All tests in this module completed successfully.\n");
	nexus_exit (0, NEXUS_TRUE);
    }
    else
    {
	printf ("There were errors in some of the tests in this module.\n");
	nexus_exit (1, NEXUS_TRUE);
    }
#endif

#ifdef VERBOSE
    nexus_destroy_current_context (NEXUS_FALSE);
#endif

    printf ("main(): ERROR: we should never get here\n");
} /* main() */
