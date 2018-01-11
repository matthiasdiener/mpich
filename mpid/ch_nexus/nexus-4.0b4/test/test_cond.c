#include <stdio.h>

#include "nexus.h"

/* uncomment the following line if you want verbose output */
/* #define VERBOSE */

#include "nexus_startup_code.h"

static nexus_mutex_t condition_test_mutex;
static nexus_mutex_t busy_mutex;
static nexus_cond_t  condition_test_cond;



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





void *condition_test_thread2 (void *arg)
/*
** DESCRIPTION: This function is executed in a thread and is used in
** conjunction with condition_test() to perform mutex and condition variable
** operations.
*/
{
    int rc;


#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: entering\n");
#endif

    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: waiting for signal \n");
#endif
    nexus_cond_wait (&condition_test_cond, &condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: awoken\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);
    
    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: waiting for broadcast\n");
#endif
    nexus_cond_wait (&condition_test_cond, &condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: awoken\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);

    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: waiting for sync\n");
#endif
    nexus_cond_wait (&condition_test_cond, &condition_test_mutex);
    nexus_mutex_unlock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: trying to acquire busy lock\n");
#endif
    rc = nexus_mutex_trylock (&busy_mutex);
    if (rc != EBUSY)
    {
	nexus_fatal ("condition_test(): ERROR: trylock returned %d (should be %d -- EBUSY)\n", rc, EBUSY);
	all_tests_okay = NEXUS_FALSE;
    }
    nexus_cond_signal (&condition_test_cond);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: busy lock not acquired\n");
#endif

#ifdef VERBOSE
    nexus_printf ("condition_test(): thread2: exiting\n");
#endif

    return (0);
} /* condition_test_thread2() */





static void condition_test (void)
/*
** DESCRIPTION: This function creates a thread that executes
** condition_test_thread2 to perform various operations with mutexes and
** condition variables.
**
** The function exercises nexus_cond_signal() and nexus_cond_broadcast()
** and then attempts to acquire a busy lock.
*/
{
    nexus_thread_t thread;
    int            rc;


#ifdef VERBOSE
    nexus_printf ("condition_test(): starting\n");
#endif
    
    nexus_mutex_init (&condition_test_mutex, (nexus_mutexattr_t *) NULL);
    nexus_mutex_init (&busy_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init (&condition_test_cond, (nexus_condattr_t *) NULL);
    
    nexus_thread_create (&thread, (nexus_thread_attr_t *) NULL,
			 condition_test_thread2, (void *) NULL);

    sleep (3);
    nexus_thread_yield ();

    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: signaling\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);
    nexus_cond_signal (&condition_test_cond);
    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: done signaling\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);

    sleep (3);
    nexus_thread_yield ();
    
    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: broadcasting\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);
    nexus_cond_broadcast (&condition_test_cond);
    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: done broadcasting\n");
#endif
    nexus_mutex_unlock (&condition_test_mutex);

    sleep (3);
    nexus_thread_yield ();

#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: trying to acquire busy lock\n");
#endif
    rc = nexus_mutex_trylock (&busy_mutex);
    if (rc != 0)
    {
	nexus_fatal ("condition_test(): ERROR: trylock returned %d (should be 0)\n",
		     rc);
	all_tests_okay = NEXUS_FALSE;
    }
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: busy lock acquired\n");
#endif
    nexus_cond_signal (&condition_test_cond);
    nexus_mutex_lock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: waiting for thread2\n");
#endif
    nexus_cond_wait (&condition_test_cond, &condition_test_mutex);
    nexus_mutex_unlock (&condition_test_mutex);
#ifdef VERBOSE
    nexus_printf ("condition_test(): thread1: unlocking busy lock\n");
#endif
    nexus_mutex_unlock (&busy_mutex);

    sleep (3);
    nexus_thread_yield ();

    nexus_mutex_destroy (&condition_test_mutex);
    nexus_mutex_destroy (&busy_mutex);
    nexus_cond_destroy (&condition_test_cond);
    
#ifdef VERBOSE
    nexus_printf ("condition_test(): complete\n");
#endif
} /* condition_test() */





void main (int argc, char **argv)
/*
** PROGRAMMER: Jonathan Geisler (?), Argonne National Laboratory
** UPDATED:    Greg Koenig, Argonne National Laboratory, 06-FEB-1996
**
** DESCRIPTION: This program exercises Nexus mutex locks and condition
** variable waits.
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
		NULL,
		NULL,
		NULL,
		NULL,
		&nodes,
		&n_nodes);

    nexus_start ();

    all_tests_okay = NEXUS_TRUE;

    condition_test ();

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
