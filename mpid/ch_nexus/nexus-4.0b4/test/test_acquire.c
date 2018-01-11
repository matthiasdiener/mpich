#include <stdio.h>

#include "nexus.h"

/* uncomment the following line if you want verbose output */
/* #define VERBOSE */

#include "nexus_startup_code.h"

#define TEST_DIR "resource_database:test_dir"
#define TEST_EXE "resource_database:test_exe"

#define ACQUIRE_FINISH_HANDLER_HASH 269

static void acquire_finish_handler (void *address, nexus_buffer_t *buffer);

static nexus_handler_t acquire_test_handlers[] =
{ \
  {"acquire_finish_handler",
   ACQUIRE_FINISH_HANDLER_HASH,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) acquire_finish_handler},
  {(char *) NULL,
   0,
   NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) NULL}
};



nexus_bool_t all_tests_okay = NEXUS_TRUE;



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
    nexus_register_handlers (acquire_test_handlers);
    rc = 0;
#ifdef VERBOSE
    nexus_printf ("NexusBoot(): exiting, returning %d\n", rc);
#endif
    return (rc);
} /* NexusBoot() */





static void acquire_finish_handler (void *address, nexus_buffer_t *buffer)
/*
** DESCRIPTION: This function is a handler that is used to shut down the
** contexts that are acquired during testing.
*/
{
    nexus_destroy_current_context (NEXUS_TRUE);
} /* acquire_finish_handler */





static void acquire_nodes_test (char *new_nodename, int new_node_number,
                                int node_count, nexus_node_t **nodes,
                                int *n_nodes)
/*
** DESCRIPTION: This function exercises nexus_acquire_nodes() by attempting
** to acquire node_count nodes on new_nodename.  Nodes begin with
** new_node_number.
**
** If this function is successful, nodes and n_nodes are updated to reflect
** the new nodes added.
*/
{
    int          i;
    int          loc;
    int          new_n_nodes;
    int          old_n_nodes;
    nexus_node_t *new_nodes;
    nexus_node_t *old_nodes;


#ifdef VERBOSE
    nexus_printf ("acquire_nodes_test(): starting\n");
#endif

    nexus_acquire_nodes (new_nodename, new_node_number, node_count,
			 TEST_DIR, TEST_EXE, &new_nodes, &new_n_nodes);

    if (new_n_nodes != node_count)
    {
	nexus_printf ("acquire_nodes_test(): ERROR: failed to acquire %d nodes (got %d)\n", node_count, new_n_nodes);
	all_tests_okay = NEXUS_FALSE;
    }
    else
    {
#ifdef VERBOSE
	nexus_printf ("acquire_nodes_test(): successfully acquired %d nodes\n", node_count);
#endif

	/* construct a new nodes with the acquired nodes */
	old_nodes = *nodes;
	old_n_nodes = *n_nodes;

	*n_nodes = old_n_nodes + new_n_nodes;
	*nodes = nexus_malloc (*n_nodes * sizeof(nexus_node_t));
	for (i = 0, loc = 0; i < old_n_nodes; i++, loc++)
	{
	    (*nodes)[loc] = old_nodes[i];
	}
	for (i = 0; i < new_n_nodes; i++, loc++)
	{
	    (*nodes)[loc] = new_nodes[i];
	}
	nexus_free (old_nodes);
    }

#ifdef VERBOSE
    nexus_printf ("acquire_nodes_test(): complete\n");
#endif
} /* acquire_nodes_test */





void main (int argc, char **argv)
/*
** PROGRAMMER: Unknown
** UPDATED:    Greg Koenig, Argonne National Laboratory, 06-FEB-1996
**
** DESCRIPTION: This program exercises the nexus_acquire_nodes() function.
** It accepts three command-line arguments for the name of the machine on
** which the nodes should be acquired, the beginning node number to acquire,
** and the number of nodes to acquire.  The list of nodes is printed before
** and after the call to nexus_acquire_nodes() is made so the user can verify
** that the nodes are indeed added correctly.
*/
{
    char                   **my_argv;
    char                   new_nodename[100];
    int                    gp_size;
    int                    i;
    int                    my_argc = 0;
    int                    n_nodes;
    int                    new_node_number;
    int                    node_count;
    int                    reply_gp_n_elements;
    nexus_buffer_t         buffer;
    nexus_global_pointer_t reply_gp;
    nexus_node_t           *nodes;


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

    /* print out the nodes array */
#ifdef VERBOSE
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf ("nodes[%d]: %s#%d, rc=%d\n",
		      i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }
#endif

    if (argc == 4)
    {
	/* read new node information from command-line */
	strcpy (new_nodename, argv[1]);
	new_node_number = atoi (argv[2]);
	node_count = atoi (argv[3]);
    }
    else
    {
	/* assume some reasonable default values */
	strcpy (new_nodename, getenv ("HOST"));
	new_node_number = 1;
	node_count = 1;
    }

    acquire_nodes_test (new_nodename, new_node_number, node_count, &nodes,
			&n_nodes);
    
    /* print out the nodes array */
#ifdef VERBOSE
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf ("nodes[%d]: %s#%d, rc=%d\n",
		      i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }
#endif

    nexus_global_pointer (&reply_gp, NULL);

    for (i = 1; i < n_nodes; i++)
    {
	nexus_init_remote_service_request (&buffer, &nodes[i].gp,
					   "acquire_finish_handler",
					   ACQUIRE_FINISH_HANDLER_HASH);
	gp_size = nexus_sizeof_global_pointer (&buffer, &reply_gp, 1,
					       &reply_gp_n_elements);
	nexus_set_buffer_size (&buffer, gp_size, reply_gp_n_elements);
	nexus_put_global_pointer (&buffer, &reply_gp, 1);
	nexus_send_remote_service_request (&buffer);
    }

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
