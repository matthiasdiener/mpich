#include <stdio.h>

#include "nexus.h"

/* uncomment the following line if you want verbose output */
/* #define VERBOSE */

#include "nexus_startup_code.h"





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





static void time_nexus_poll (int iterations_to_run)
/*
** DESCRIPTION: This function calls nexus_poll() iterations_to_run times
** and determines the amount of wallclock time the call loop takes to run.
** The average amount of wallclock time per call is then calculated and
** displayed.
*/
{
    double elapse;
    double start;
    double stop;
    int    i;


#ifdef VERBOSE
    nexus_printf ("time_nexus_poll(): starting\n");
#endif
    start = nexus_wallclock ();
    for (i = 0; i < iterations_to_run; i++)
    {
	nexus_poll ();
    }
    stop = nexus_wallclock ();
    elapse = stop - start;

    nexus_printf ("time_nexus_poll(): iters=%d, elapse=%f, elapse/iters=%f\n",
		  iterations_to_run,
		  elapse, 
		  (elapse/iterations_to_run) );
#ifdef VERBOSE
    nexus_printf ("time_nexus_poll(): complete\n");
#endif
} /* time_nexus_poll() */





void main (int argc, char **argv)
/*
** PROGRAMMER: Unknown
** UPDATED:    Greg Koenig, Argonne National Laboratory, 06-FEB-1996
**
** DESCRIPTION: This program determines the average amount of wallclock
** time required for a call to nexus_poll() by making iterations_to_run
** calls and then dividing the total wallclock time used by iterations_to_run.
** The value of iterations_to_run is taken from the command-like if provided
** or is given the default value 10000 if not provided.
*/
{
    char         **my_argv;
    int          i;
    int          iterations_to_run;
    int          my_argc = 0;
    int          n_nodes;
    nexus_node_t *nodes;


    nexus_init (&my_argc,
		&my_argv,
		"NEXUS_ARGS",
		"nx",
		NULL,
		NULL,
		NULL,
		NULL,
		&nodes,
		&n_nodes);

    nexus_start ();

    if (argc == 2)
    {
	iterations_to_run = atoi (argv[1]);
    }
    else
    {
	iterations_to_run = 10000;
    }

    time_nexus_poll (iterations_to_run);

    for (i = 0; i < n_nodes; i++)
    {
	nexus_free (nodes[i].name);
	nexus_destroy_global_pointer (&nodes[i].gp);
    }
    nexus_free (nodes);

    nexus_destroy_current_context (NEXUS_FALSE);

    printf ("main(): ERROR: we should never get here\n");
} /* main() */
