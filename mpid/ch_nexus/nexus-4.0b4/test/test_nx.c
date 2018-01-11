/*
 * test_nx.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_test/test_nx.c,v 1.17 1997/02/24 21:58:27 tuecke Exp $";

/*
#define ARGS_TEST
*/
/*
#define CONDITION_TEST
*/
/*
#define LOCAL_COMM_TEST
*/
/*
#define ACQUIRE_NODES_TEST
*/
/*
#define NODE_COMM_TEST
*/
/*
#define ATTACH_TEST
*/
#define REMOTE_COMM_TEST
/*
*/
/*
#define TIME_NEXUS_POLL
*/
/*
#define SHUTDOWN_NODES_BEFORE_CONTEXTS
*/
/*
#define USE_THREADED_HANDLER
*/
   
/* Define this if you want main() to spin after returning from nexus_start()
#define WAIT_FOR_DEBUG
*/

#include "nexus.h"
#include "stdio.h"

#ifndef TEST_DIR
#define TEST_DIR "resource_database:test_dir"
#endif

#ifndef TEST_EXE
#define TEST_EXE "resource_database:test_exe"
#endif

#ifndef TEST_ACQUIRE_NODE_NAME
#define TEST_ACQUIRE_NODE_NAME "grumpy"
#endif

#define TEST_ACQUIRE_NODE_NUMBER 1

#ifdef ARGS_TEST
static void args_test(int argc, char **argv);
#endif /* ARGS_TEST */

#ifdef CONDITION_TEST
static void condition_test(void);
#endif /* CONDITION_TEST */

#ifdef LOCAL_COMM_TEST
#ifndef COMM_TEST
#define COMM_TEST
#endif
static void local_comm_test(void);
#endif /* LOCAL_COMM_TEST */

#ifdef NODE_COMM_TEST
#ifndef COMM_TEST
#define COMM_TEST
#endif
static void node_comm_test(nexus_node_t *nodes,
			   int n_nodes);
#endif /* NODE_COMM_TEST */

#ifdef REMOTE_COMM_TEST
#ifndef COMM_TEST
#define COMM_TEST
#endif
static void remote_comm_test(nexus_node_t *nodes,
			     int n_nodes);
#endif /* REMOTE_COMM_TEST */

#ifdef ACQUIRE_NODES_TEST
static void acquire_nodes_test(nexus_node_t **nodes,
			       int *n_nodes);
#endif /* ACQUIRE_NODES_TEST */

#ifdef TIME_NEXUS_POLL
static void time_nexus_poll(void);
#endif /* TIME_NEXUS_POLL */

#ifdef ATTACH_TEST
#ifndef COMM_TEST
#define COMM_TEST
#endif
static nexus_bool_t		attach_server_arg = NEXUS_FALSE;
static nexus_bool_t		attach_client_arg = NEXUS_FALSE;
static char *			attach_client_host;
static unsigned short		attach_client_port;
static nexus_bool_t		attach_client_leave_server = NEXUS_FALSE;
static nexus_bool_t		attach_client_die = NEXUS_FALSE;

static void attach_client(void);
static void attach_server(void);

static void attach_server_finish_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
#endif /* ATTACH_TEST */

#ifdef COMM_TEST

static void comm_test_remote_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
static void comm_test_reply_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
#endif /* COMM_TEST */

static void shutdown_nodes(nexus_node_t *nodes, int n_nodes);

void NexusUnknownHandler(nexus_endpoint_t *endpoint,
			 nexus_buffer_t *buffer,
			 int handler_id);
static void shutdown_node_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
static void shutdown_node_reply_handler(nexus_endpoint_t *endpoint,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);

static nexus_handler_t system_handlers[] =
{ \
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) shutdown_node_handler},
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) shutdown_node_reply_handler},
#ifdef COMM_TEST
#ifdef USE_THREADED_HANDLER
  {NEXUS_HANDLER_TYPE_THREADED,
   (nexus_handler_func_t) comm_test_remote_handler},
#else /* USE_THREADED_HANDLER */
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) comm_test_remote_handler},
#endif /* USE_THREADED_HANDLER */
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) comm_test_reply_handler},
#endif /* COMM_TEST */
#ifdef ATTACH_TEST
  {NEXUS_HANDLER_TYPE_NON_THREADED,
   (nexus_handler_func_t) attach_server_finish_handler},
#endif /* ATTACH_TEST */
};

static nexus_endpoint_t global_endpoint;


#ifndef MAX
#define MAX(V1,V2) (((V1) > (V2)) ? (V1) : (V2))
#define MIN(V1,V2) (((V1) < (V2)) ? (V1) : (V2))
#endif

static nexus_bool_t test_arg;

/*
extern void sleep(int);
*/



/*
 * package_args_init(void)
 */
static int package_args_init(int *argc, char ***argv)
{
    int arg_num;
    
    if ((arg_num = nexus_find_argument(argc, argv, "test", 1)) >= 0)
    {
	test_arg = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    else
    {
	test_arg = NEXUS_FALSE;
    }

#ifdef ATTACH_TEST
    if ((arg_num = nexus_find_argument(argc, argv, "server", 1)) >= 0)
    {
	attach_server_arg = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "client", 3)) >= 0)
    {
	attach_client_arg = NEXUS_TRUE;
	attach_client_host = (*argv)[arg_num + 1];
	attach_client_port = (short) atoi((*argv)[arg_num + 2]);
	nexus_remove_arguments(argc, argv, arg_num, 3);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "leave_server", 1)) >= 0)
    {
	attach_client_leave_server = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
    if ((arg_num = nexus_find_argument(argc, argv, "die", 1)) >= 0)
    {
	attach_client_die = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 1);
    }
#endif /* ATTACH_TEST */
    
    return(0);
} /* package_args_init() */


/*
 * usage_message()
 */
static void usage_message(void)
{
    printf("Test usage message goes here...\n");
} /* usage_message() */


/*
 * new_process_params()
 */
static int new_process_params(char *buf, int size)
{
    char tmp_buf1[1024];
    int n_added;

    tmp_buf1[0] = '\0';

    if (test_arg)
    {
	sprintf(tmp_buf1, "-test ");
    }

    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
	nexus_fatal("new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }

    strcpy(buf, tmp_buf1);
    
    return (n_added);
} /* new_process_params() */

/*
 * JMP test dummy function for entrypoint execution detection
 */
void falseentry(void)
{
    nexus_printf("*** falseentry() called\n");
}

/*
 * NexusExit()
 */
int NexusExit(void)
{
    nexus_printf("NexusExit(): entering\n");
    nexus_printf("NexusExit(): exiting\n");
    return 0;
} /* NexusExit() */


/*
 * NexusUnknownHandler()
 */
void NexusUnknownHandler(nexus_endpoint_t *endpoint,
			 nexus_buffer_t *buffer,
			 int handler_id)
{
    nexus_printf("NexusUnknownHandler(): handler_id=%d\n", handler_id);
    nexus_printf("NexusUnknownHandler(): exiting\n");
} /* NexusUnknownHandler() */


/*
 * NexusBoot()
 */
int NexusBoot(nexus_startpoint_t *startpoint)
{
    int n_handlers;
    nexus_endpointattr_t epattr;

    nexus_printf("NexusBoot(): entering\n");

    n_handlers = 2;
#ifdef COMM_TEST
    n_handlers += 2;
#endif
#ifdef ATTACH_TEST
    n_handlers += 1;
#endif
    
    nexus_endpointattr_init(&epattr);
    nexus_endpointattr_set_handler_table(&epattr, system_handlers, n_handlers);
    nexus_endpointattr_set_unknown_handler(&epattr,
					  NexusUnknownHandler,
					  NEXUS_HANDLER_TYPE_NON_THREADED);
    nexus_endpoint_init(&global_endpoint, &epattr);
    nexus_startpoint_bind(startpoint, &global_endpoint);

    nexus_endpointattr_destroy(&epattr);

    nexus_printf("NexusBoot(): exiting, returning %d\n", 0);

    return 0;
} /* NexusBoot() */


/*
 * NexusAcquiredAsNode()
 */
int NexusAcquiredAsNode(nexus_startpoint_t *startpoint)
{
    int rc;
    
    nexus_printf("NexusAcquiredAsNode(): entering\n");
    rc = NexusBoot(startpoint);
    nexus_printf("NexusAcquiredAsNode(): exiting, returning %d\n", rc);
    return (rc);
} /* NexusAcquiredAsNode() */


/*****************************************************************
 *		MAIN
 *****************************************************************/

/*
 * main()
 */
void main(int argc, char **argv)
{
    int my_argc = 0;
    char **my_argv;
    nexus_node_t *nodes;
    int n_nodes;
    int i;

#ifdef WAIT_FOR_DEBUG    
    int iwait4debug=1;
#endif    

    nexus_init(&my_argc,
	       &my_argv,
	       "NEXUS_ARGS",
	       "nx",
	       package_args_init,
	       usage_message,
	       new_process_params,
	       NULL,
	       &nodes,
	       &n_nodes);

#ifdef WAIT_FOR_DEBUG    
    nexus_printf("process waiting to be debugged\n");
    while( iwait4debug ) {
      ;
    }
#endif
    nexus_start();

    /* Bind a valid startpoint to nodes[0] */
    nexus_startpoint_bind(&(nodes[0].startpoint), &global_endpoint);

    /* Print out the nodes array */
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf("nodes[%d]: %s#%d, rc=%d\n",
		     i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }

#ifdef ARGS_TEST
    args_test(my_argc, my_argv);
#endif /* ARGS_TEST */    
    
#ifdef CONDITION_TEST
    condition_test();
#endif /* CONDITION_TEST */
    
#ifdef LOCAL_COMM_TEST
    local_comm_test();
#endif /* LOCAL_COMM_TEST */

#ifdef ATTACH_TEST
    if (attach_server_arg)
    {
	attach_server();
	goto alldone;
    }
    else if (attach_client_arg)
    {
	attach_client();
    }
    else
    {
	nexus_printf("Neither -server or -client was specified.  Skipping attach test\n");
    }
#endif /* ATTACH_TEST */
    
#ifdef ACQUIRE_NODES_TEST
    acquire_nodes_test(&nodes, &n_nodes);
    
    /* Print out the nodes array */
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf("nodes[%d]: %s#%d, rc=%d\n",
		     i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }
#endif /* ACQUIRE_NODES_TEST */

#ifdef NODE_COMM_TEST
    node_comm_test(nodes, n_nodes);
#endif /* NODE_COMM_TEST */

#ifdef REMOTE_COMM_TEST
    remote_comm_test(nodes, n_nodes);
#endif /* REMOTE_COMM_TEST */

#ifdef TIME_NEXUS_POLL
    time_nexus_poll();
#endif /* TIME_NEXUS_POLL */

 alldone:
    
#ifndef SHUTDOWN_NODES_BEFORE_CONTEXTS    
    if (n_nodes > 1)
    {
	shutdown_nodes(nodes, n_nodes);
    }
#endif /* SHUTDOWN_NODES_BEFORE_CONTEXTS */

    nexus_printf("main(): freeing nodes array\n");
    for(i = 0; i < n_nodes; i++)
    {
	nexus_free(nodes[i].name);
	nexus_startpoint_destroy(&(nodes[i].startpoint));
    }
    nexus_free(nodes);
    
    nexus_printf("main(): destroy context of master node\n");
    nexus_context_destroy(NEXUS_FALSE);

    printf("main(): ERROR: We should never get here\n");

} /* main() */


/*****************************************************************
 *		SHUTDOWN NODES
 *****************************************************************/

typedef struct shutdown_nodes_barrier_struct
{
    int			count;
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
} shutdown_nodes_barrier_t;

/*
 * shutdown_node_handler()
 */
static void shutdown_node_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_buffer_t reply_buffer;
    nexus_startpoint_t barrier_sp;

    nexus_get_startpoint(buffer, &barrier_sp, 1);
    
    nexus_printf("shutdown_node_handler(): sending shutdown node reply\n");
    nexus_buffer_init(&reply_buffer, 0, 0);
    if (nexus_send_rsr(&reply_buffer,
		       &barrier_sp,
		       1,
		       NEXUS_TRUE,
		       called_from_non_threaded_handler) != 0)
    {
	nexus_printf("shutdown_node_handler(): WARNING: send failed\n");
    }

    nexus_startpoint_destroy(&barrier_sp);
    
    nexus_printf("shutdown_node_handler(): destroying context\n");
    nexus_context_destroy(called_from_non_threaded_handler);
} /* shutdown_node_handler() */


/*
 * shutdown_node_reply_handler()
 */
static void shutdown_node_reply_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    shutdown_nodes_barrier_t *shutdown_nodes_barrier;

    nexus_printf("shutdown_node_reply_handler(): entering\n");

    shutdown_nodes_barrier =
        (shutdown_nodes_barrier_t *)nexus_endpoint_get_user_pointer(endpoint);
    nexus_mutex_lock(&(shutdown_nodes_barrier->mutex));
    if (--shutdown_nodes_barrier->count == 0)
    {
	nexus_cond_signal(&(shutdown_nodes_barrier->cond));
    }
    nexus_mutex_unlock(&(shutdown_nodes_barrier->mutex));

    nexus_printf("shutdown_node_reply_handler(): exiting\n");
} /* shutdown_node_reply_handler() */


/*
 * shutdown_nodes()
 */
static void shutdown_nodes(nexus_node_t *nodes, int n_nodes)
{
    int i;
    nexus_buffer_t buffer;
    nexus_startpoint_t barrier_sp;
    shutdown_nodes_barrier_t shutdown_nodes_barrier;
    int buf_size;

    shutdown_nodes_barrier.count = 0;
    nexus_mutex_init(&(shutdown_nodes_barrier.mutex),
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(shutdown_nodes_barrier.cond),
		    (nexus_condattr_t *) NULL);
    nexus_endpoint_set_user_pointer(&global_endpoint,
				    (void *)&shutdown_nodes_barrier);
    
    for (i = 1; i < n_nodes; i++)
    {
	if (nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    nexus_startpoint_bind(&barrier_sp, &global_endpoint);
	    buf_size = nexus_sizeof_startpoint(&barrier_sp, 1);
	    nexus_buffer_init(&buffer, buf_size, 0);
	    nexus_put_startpoint_transfer(&buffer, (&barrier_sp), 1);
	    nexus_printf("shutdown_nodes(): sending destroy context message to node %d\n", i);
	    if (nexus_send_rsr(&buffer,
			       &(nodes[i].startpoint),
			       0,
			       NEXUS_TRUE,
			       NEXUS_FALSE) != 0)
	    {
		nexus_printf("shutdown_nodes(): WARNING: send failed\n");
	    }
	    nexus_mutex_lock(&(shutdown_nodes_barrier.mutex));
	    shutdown_nodes_barrier.count++;
	    nexus_mutex_unlock(&(shutdown_nodes_barrier.mutex));
	}
    }

    nexus_printf("shutdown_nodes(): waiting for replies\n");
    nexus_mutex_lock(&(shutdown_nodes_barrier.mutex));
    while (shutdown_nodes_barrier.count > 0)
    {
	nexus_cond_wait(&(shutdown_nodes_barrier.cond),
			&(shutdown_nodes_barrier.mutex) );
    }
    nexus_mutex_unlock(&(shutdown_nodes_barrier.mutex));
    
    nexus_mutex_destroy(&(shutdown_nodes_barrier.mutex));
    nexus_cond_destroy(&(shutdown_nodes_barrier.cond));
    
    nexus_printf("shutdown_nodes(): done\n");

} /* shutdown_nodes() */



/*****************************************************************
 *		ARGS_TEST
 *****************************************************************/

#ifdef ARGS_TEST    
static void args_test(int argc, char **argv)
{
    int i;
    
    if (test_arg)
	nexus_printf("args_test(): Got -test argument\n");
    
    nexus_printf("args_test(): argc=%d\n", argc);
    for (i = 0; i < argc; i++)
    {
	nexus_printf("args_test(): argv[%d]=%s\n", i, argv[i]);
    }
}
#endif /* ARGS_TEST */


/*****************************************************************
 *		CONDITION_TEST
 *****************************************************************/
#ifdef CONDITION_TEST

static nexus_mutex_t	condition_test_mutex;
static nexus_mutex_t    busy_mutex;
static nexus_cond_t	condition_test_cond;
/*
static int  cond_flag1 = 0;
static int  cond_flag2 = 0;
*/
void *condition_test_thread2(void *arg)
{
    int rc;

    nexus_printf("condition_test(): thread2: entering\n");

    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread2: waiting for signal \n");
    nexus_cond_wait(&condition_test_cond, &condition_test_mutex);
    nexus_printf("condition_test(): thread2: awoken\n");
    nexus_mutex_unlock(&condition_test_mutex);
    
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread2: waiting for broadcast\n");
    nexus_cond_wait(&condition_test_cond, &condition_test_mutex);
    nexus_printf("condition_test(): thread2: awoken\n");
    nexus_mutex_unlock(&condition_test_mutex);

    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread2: waiting for sync\n");
    nexus_cond_wait(&condition_test_cond, &condition_test_mutex);
    nexus_mutex_unlock(&condition_test_mutex);
    nexus_printf("condition_test(): thread2: trying to acquire busy lock\n");
    rc = nexus_mutex_trylock(&busy_mutex);
    if (rc != EBUSY)
    {
	nexus_fatal("condition_test(): lock returned %d instead of %d (EBUSY)\n", rc, EBUSY);
    }
    nexus_cond_signal(&condition_test_cond);
    nexus_printf("condition_test(): thread2: busy lock not acquired\n");

    nexus_printf("condition_test(): thread2: exiting\n");

    return (NULL);
} /* condition_test_thread2() */

/*
 * condition_test()
 */
static void condition_test(void)
{
    nexus_thread_t thread;
    int rc;
    
    nexus_printf("condition_test(): starting\n");
    
    nexus_mutex_init(&condition_test_mutex, (nexus_mutexattr_t *) NULL);
    nexus_mutex_init(&busy_mutex, (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&condition_test_cond, (nexus_condattr_t *) NULL);
    
    nexus_thread_create(&thread,
			(nexus_thread_attr_t *) NULL,
			condition_test_thread2,
			(void *) NULL);

    sleep(3); nexus_thread_yield();
    
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: signaling\n");
    nexus_mutex_unlock(&condition_test_mutex);
    nexus_cond_signal(&condition_test_cond);
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: done signaling\n");
    nexus_mutex_unlock(&condition_test_mutex);

    sleep(3); nexus_thread_yield();
    
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: broadcasting\n");
    nexus_mutex_unlock(&condition_test_mutex);
    nexus_cond_broadcast(&condition_test_cond);
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: done broadcasting\n");
    nexus_mutex_unlock(&condition_test_mutex);

    sleep(3); nexus_thread_yield();

    nexus_printf("condition_test(): thread1: trying to acquire busy lock\n");
    rc = nexus_mutex_trylock(&busy_mutex);
    if (rc != 0)
    {
	nexus_fatal("condition_test(): trylock returned %d instead of 0 (success)\n", rc);
    }
    nexus_printf("condition_test(): thread1: busy lock acquired\n");
    nexus_cond_signal(&condition_test_cond);
    nexus_mutex_lock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: waiting for thread2\n");
    nexus_cond_wait(&condition_test_cond, &condition_test_mutex);
    nexus_mutex_unlock(&condition_test_mutex);
    nexus_printf("condition_test(): thread1: unlocking busy lock\n");
    nexus_mutex_unlock(&busy_mutex);

    sleep(3); nexus_thread_yield();

    nexus_mutex_destroy(&condition_test_mutex);
    nexus_mutex_destroy(&busy_mutex);
    nexus_cond_destroy(&condition_test_cond);
    
    nexus_printf("condition_test(): complete\n");
} /* condition_test() */

#endif /* CONDITION_TEST */


/*****************************************************************
 *		COMM_TEST
 *****************************************************************/
#ifdef COMM_TEST

typedef struct _comm_test_reply_done_t
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    nexus_bool_t		done;
} comm_test_reply_done_t;
static comm_test_reply_done_t comm_test_reply_done;

static int			comm_test_int_data = 42;

/*
 * comm_test_remote_handler()
 */
static void comm_test_remote_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    int int_data;
    int destroy_context;
    nexus_startpoint_t reply_sp;
    nexus_buffer_t reply_buffer;
    
    nexus_printf("comm_test_remote_handler(): entering\n");

    /* Get the reply global pointer */
    nexus_get_startpoint(buffer, &reply_sp, 1);
    
    /* Test nexus_startpoint_*() equality functions */
    nexus_printf("comm_test(): nexus_startpoint_to_current_context(&reply_sp)==%s (should be TRUE for local_comm_test(), and FALSE for remote_comm_test(), and depentent upon the node for node_comm_test())\n", (nexus_startpoint_to_current_context(&reply_sp) ? "TRUE" : "FALSE"));
    
    /* Get the data and check to see if it made it ok */
    nexus_get_int(buffer, &int_data, 1);
    if (int_data == comm_test_int_data)
    {
	nexus_printf("comm_test_remote_handler(): got int_data ok\n");
    }
    else
    {
	nexus_printf("comm_test_remote_handler(): ERROR: bad int_data %d, should be %d\n",
		     int_data, comm_test_int_data);
    }

    /* Get destroy_context */
    nexus_get_int(buffer, &destroy_context, 1);
    nexus_printf("comm_test_remote_handler(): destroy_context=%d\n",
		 destroy_context);

    if (!called_from_non_threaded_handler)
    {
        nexus_printf("comm_test_remote_handler(): freeing saved buffer\n");
	nexus_buffer_destroy(buffer);
    }
    
    /* Reply to the original context */
    nexus_printf("comm_test_remote_handler(): sending reply rsr\n");
    nexus_buffer_init(&reply_buffer, 0, 0);
    if (nexus_send_rsr(&reply_buffer,
		       &reply_sp,
		       3,
		       NEXUS_TRUE,
		       called_from_non_threaded_handler) != 0)
    {
	nexus_printf("comm_test_remote_handler(): WARNING: send failed\n");
	nexus_printf("comm_test_remote_handler(): nexus_startpoint_test()=%d\n",
		     nexus_startpoint_test(&reply_sp));
    }

    if (destroy_context)
    {
	nexus_printf("comm_test_remote_handler(): destroying current context\n");
	nexus_context_destroy(called_from_non_threaded_handler);
    }
} /* comm_test_remote_handler() */


/*
 * comm_test_reply_handler()
 */
static void comm_test_reply_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    struct _comm_test_reply_done_t *reply_done;
    void *address;
    
    nexus_printf("comm_test_reply_handler(): entering\n");

    address = nexus_endpoint_get_user_pointer(endpoint);
    /* The address should be to the comm_test_reply_done structure */
    if (address == (void *) (&comm_test_reply_done))
    {
	reply_done = (comm_test_reply_done_t *) address;
	nexus_printf("comm_test_reply_handler(): address ok\n");
    }
    else
    {
	reply_done = &comm_test_reply_done;
	nexus_printf("comm_test_reply_handler(): ERROR: bad address %x, should be %x\n",
		     address,
		     (&comm_test_reply_done));
    }
    
    /* Awaken the main thread */
    nexus_printf("comm_test_reply_handler(): entering monitor\n");
    nexus_mutex_lock(&(reply_done->mutex));
    reply_done->done = NEXUS_TRUE;
    nexus_printf("comm_test_reply_handler(): signaling monitor\n");
    nexus_cond_signal(&(reply_done->cond));
    nexus_mutex_unlock(&(reply_done->mutex));
   
    nexus_printf("comm_test_reply_handler(): exiting\n");

} /* comm_test_reply_handler() */


/*
 * comm_test()
 *
 * Invoke comm_test_remote_handler() in the context pointed to
 * by 'remote_gp'.  The remote handler will bounce a message
 * back to the original context using comm_test_reply_handler().
 * The reply handler will signal the original thread via a monitor.
 */
static void comm_test(nexus_startpoint_t *remote_sp,
		      int destroy_context)
{
    nexus_startpoint_t reply_done_sp;
    nexus_buffer_t buffer;
    int buf_size;
    
    nexus_printf("comm_test(): starting\n");

    /*
     * Initialize the monitor that will be used for signaling
     * completion of the reply handler.
     */
    nexus_mutex_init(&(comm_test_reply_done.mutex),
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(comm_test_reply_done.cond), (nexus_condattr_t *) NULL);
    nexus_mutex_lock(&(comm_test_reply_done.mutex));
    comm_test_reply_done.done = NEXUS_FALSE;
    nexus_mutex_unlock(&(comm_test_reply_done.mutex));

    /* Set up the reply global pointer to point at the reply done monitor */
    nexus_endpoint_set_user_pointer(&global_endpoint,
				    (void *)&comm_test_reply_done);
    nexus_startpoint_bind(&reply_done_sp, &global_endpoint);

    /* Test nexus_startpoint_*() equality functions */
    if (nexus_startpoint_to_current_context(&reply_done_sp))
    {
	nexus_endpoint_t *tmp_ep;
	nexus_printf("comm_test(): nexus_startpoint_to_current_context() works\n");
	nexus_startpoint_get_endpoint(&reply_done_sp, &tmp_ep);
	if (tmp_ep && (tmp_ep == &global_endpoint))
	{
	    nexus_printf("comm_test(): nexus_startpoint_get_endpoint() works\n");
	}
	else
	{
	    nexus_printf("comm_test(): ERROR: nexus_startpoint_get_endpoint() failed\n");
	}
    }
    else
    {
	nexus_printf("comm_test(): ERROR: nexus_startpoint_to_current_context() failed\n");
    }
    
    /* Invoke handler with a known (but bogus) address */
    buf_size = nexus_sizeof_startpoint(&reply_done_sp, 1);
    buf_size += nexus_sizeof_int(1) * 2;
    nexus_buffer_init(&buffer, buf_size, 0);

    /* Send some data */
    nexus_put_startpoint_transfer(&buffer, (&reply_done_sp), 1);
    nexus_put_int(&buffer, &comm_test_int_data, 1);
    nexus_put_int(&buffer, &destroy_context, 1);

    nexus_printf("comm_test(): sending rsr\n");
    if (nexus_send_rsr(&buffer, remote_sp, 2, NEXUS_TRUE, NEXUS_FALSE) != 0)
    {
	nexus_printf("comm_test(): WARNING: send failed\n");
    }

#ifdef ATTACH_TEST
    /* Die unexpectedly to test fault tolerance */
    if (attach_client_die)
    {
	exit(0);
    }
#endif /* ATTACH_TEST */

    /* Suspend main thread until handler completes */
    nexus_printf("comm_test(): entering monitor\n");
    nexus_mutex_lock(&(comm_test_reply_done.mutex));
    nexus_printf("comm_test(): waiting on monitor\n");
    while (!comm_test_reply_done.done)
    {
	nexus_cond_wait(&(comm_test_reply_done.cond),
			&(comm_test_reply_done.mutex));
    }
    nexus_printf("comm_test(): awoke from monitor and exiting\n");
    nexus_mutex_unlock(&(comm_test_reply_done.mutex));

    nexus_mutex_destroy(&(comm_test_reply_done.mutex));
    nexus_cond_destroy(&(comm_test_reply_done.cond));
    nexus_startpoint_destroy(&reply_done_sp);
    
    nexus_printf("comm_test(): complete\n");
    
} /* comm_test() */

#endif /* COMM_TEST */



/*****************************************************************
 *		LOCAL_COMM_TEST
 *****************************************************************/
#ifdef LOCAL_COMM_TEST

/*
 * local_comm_test()
 */
static void local_comm_test(void)
{
    nexus_startpoint_t sp;
    
    nexus_printf("local_comm_test(): starting\n");

    nexus_startpoint_bind(&sp, &global_endpoint);
    comm_test(&sp, NEXUS_FALSE);
    nexus_startpoint_destroy(&sp);
    
    nexus_printf("local_comm_test(): complete\n");
} /* local_comm_test() */

#endif /* LOCAL_COMM_TEST */



/*****************************************************************
 *		NODE_COMM_TEST
 *****************************************************************/
#ifdef NODE_COMM_TEST

/*
 * node_comm_test()
 */
static void node_comm_test(nexus_node_t *nodes,
			   int n_nodes)
{
    int i;

    nexus_printf("node_comm_test(): starting\n");

    /* Run comm_test() to each node's main context */
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf("node_comm_test(): running to node %d\n", i);
	comm_test(&(nodes[i].startpoint), NEXUS_FALSE);
	nexus_printf("node_comm_test(): completed to node %d\n", i);
    }
    
    nexus_printf("node_comm_test(): complete\n");
} /* node_comm_test() */

#endif /* NODE_COMM_TEST */


	
/*****************************************************************
 *		REMOTE_COMM_TEST
 *****************************************************************/
#ifdef REMOTE_COMM_TEST

/*
 * remote_comm_test()
 */
static void remote_comm_test(nexus_node_t *nodes,
			     int n_nodes)
{
    int i, n_contexts;
    nexus_startpoint_t sp[256];
    int rc[256];
    nexus_context_create_handle_t contexts;

    nexus_printf("remote_comm_test(): starting\n");

    /* Create one context on each node */
    n_contexts = MIN(256, n_nodes);

    nexus_context_create_init(&contexts, n_contexts);
    for (i = 0; i < n_contexts; i++)
    {
	nexus_printf("remote_comm_test(): Creating context %d\n", i);
	nexus_context_create(&(nodes[i].startpoint),
			     TEST_EXE,
			     &(sp[i]),
			     &(rc[i]),
			     &contexts);
    }

    nexus_context_create_wait(&contexts);
    nexus_printf("remote_comm_test(): All contexts created\n");

#ifdef SHUTDOWN_NODES_BEFORE_CONTEXTS    
    nexus_printf("remote_comm_test(): Shutting down nodes\n");
    if (n_nodes > 1)
    {
	shutdown_nodes(nodes, n_nodes);
    }
#endif    

    /* Check return codes and run comm_test() to each context */
    for (i = 0; i < n_contexts; i++) 
    {
	if (rc[i] != 0)
	{
	    nexus_printf("remote_comm_test(): Bad return code (%d) for context %d\n",
			 rc[i], i);
	}
	else
	{
	    nexus_printf("remote_comm_test(): Running on context %d\n", i);
	    comm_test(&(sp[i]), NEXUS_TRUE);
	    nexus_printf("remote_comm_test(): Completed on context %d\n", i);
	}
    }
    
    nexus_printf("remote_comm_test(): complete\n");
} /* remote_comm_test() */

#endif /* REMOTE_COMM_TEST */


	
/*****************************************************************
 *		ACQUIRE_NODES_TEST
 *****************************************************************/
#ifdef ACQUIRE_NODES_TEST

	
/*
 * acquire_nodes_test()
 */
static void acquire_nodes_test(nexus_node_t **nodes,
			       int *n_nodes)
{
    nexus_node_t *new_nodes, *old_nodes;
    int new_n_nodes, old_n_nodes;
    int i, loc;

    nexus_printf("acquire_nodes_test(): starting\n");

    nexus_printf("acquire_nodes_test(): acquiring: node=%s#%d\n",
		 TEST_ACQUIRE_NODE_NAME,
		 TEST_ACQUIRE_NODE_NUMBER);

    nexus_node_acquire(TEST_ACQUIRE_NODE_NAME,
		       TEST_ACQUIRE_NODE_NUMBER,
		       1,
		       TEST_DIR,
		       TEST_EXE,
		       &new_nodes,
		       &new_n_nodes);

    if (new_n_nodes != 1)
    {
	nexus_printf("acquire_nodes_test(): ERROR: failed nexus_acquire_nodes_on_host(): new_n_nodes=%d\n", new_n_nodes);
    }
    else
    {
	nexus_printf("acquire_nodes_test(): successfully acquired node %s#%d, rc=%d\n", new_nodes[0].name, new_nodes[0].number, new_nodes[0].return_code);

	/* Construct a new nodes with this acquired node */
	nexus_printf("acquire_nodes_test(): adding node to nodes\n");
	old_nodes = *nodes;
	old_n_nodes = *n_nodes;

	*n_nodes = old_n_nodes + new_n_nodes;
	*nodes = nexus_malloc(*n_nodes * sizeof(nexus_node_t));
	for (i = 0, loc = 0; i < old_n_nodes; i++, loc++)
	{
	    (*nodes)[loc] = old_nodes[i];
	}
	for (i = 0; i < new_n_nodes; i++, loc++)
	{
	    (*nodes)[loc] = new_nodes[i];
	}
	nexus_free(old_nodes);
	nexus_printf("acquire_nodes_test(): done adding node to nodes\n");
    }
    
    nexus_printf("acquire_nodes_test(): complete\n");
} /* acquire_nodes_test() */
#endif /* ACQUIRE_NODES_TEST */


/*****************************************************************
 *		TIME_NEXUS_POLL
 *****************************************************************/
#ifdef TIME_NEXUS_POLL

#define TIME_NEXUS_POLL_ITERS 10000

/*
 * time_nexus_poll()
 */
static void time_nexus_poll(void)
{
    double start, stop, elapse;
    int i;

    nexus_printf("time_nexus_poll(): starting\n");

    start = nexus_wallclock();
    for (i = 0; i < TIME_NEXUS_POLL_ITERS; i++)
    {
	nexus_poll();
    }
    stop = nexus_wallclock();
    elapse = stop - start;
    nexus_printf("time_nexus_poll(): iters=%d, elapse=%f, elapse/iters=%f\n",
		 TIME_NEXUS_POLL_ITERS,
		 elapse, 
		 (elapse/TIME_NEXUS_POLL_ITERS) );
    
    nexus_printf("time_nexus_poll(): complete\n");
} /* time_nexus_poll() */

#endif /* TIME_NEXUS_POLL */


/*****************************************************************
 *		ATTACH_TEST
 *****************************************************************/
#ifdef ATTACH_TEST

typedef struct
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    nexus_bool_t	done;
} attach_server_monitor_t;

static attach_server_monitor_t attach_server_monitor;


/*
 * fault_callback()
 */
int fault_callback(void *user_arg, int fault_code)
{
    nexus_printf("fault_callback(): fault_code=%d: %s\n",
		 fault_code, nexus_fault_strings[fault_code]);
    return(0);
} /* fault_callback() */


/*
 * attach_requested()
 */
int attach_requested(void *user_arg, char *url, nexus_startpoint_t *sp)
{
    char *host;
    unsigned short port;
    char **specs;
    int i;
    
    nexus_printf("attach_requested(): entering\n");
    nexus_printf("attach_requested(): url=%s\n", url);

    nexus_split_nexus_url(url, &host, &port, &specs);
    nexus_printf("attach_requested(): host=%s\n", host);
    nexus_printf("attach_requested(): port=%hu\n", port);
    for (i = 0; specs[i]; i++)
    {
	nexus_printf("attach_requested(): spec %d: <%s>\n", i, specs[i]);
    }
    nexus_split_nexus_url_free(&host, &specs);

    nexus_endpoint_set_user_pointer(&global_endpoint, (void *)12345);
    nexus_startpoint_bind(sp, &global_endpoint);
    
    nexus_printf("attach_requested(): exiting\n");
    return(0);
} /* attach_requested() */


/*
 * attach_server()
 */
static void attach_server(void)
{
    unsigned short port;
    char *host, *host2;
    int rc;
    
    nexus_printf("attach_server(): entering\n");

    nexus_enable_fault_tolerance(fault_callback, NULL);
    
    nexus_mutex_init(&(attach_server_monitor.mutex),
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(attach_server_monitor.cond),
		    (nexus_condattr_t *) NULL);
    attach_server_monitor.done = NEXUS_FALSE;

    nexus_printf("attach_server(): allowing attachment\n");
    port = 0;
    if ((rc = nexus_allow_attach(&port, &host, &attach_requested, NULL)) != 0)
    {
	nexus_printf("ERROR: nexus_allow_attach() returned non-0: rc=%d\n",
		     rc);
    }
    if ((rc = nexus_allow_attach(&port, &host2, &attach_requested, NULL)) != 1)
    {
	nexus_printf("ERROR: second nexus_allow_attach() did not return 1: rc=%d\n\n", rc);
    }
    
    nexus_printf("attach_server(): ATTACH TO: %s %hu\n", host, port);

    nexus_printf("attach_server(): waiting on monitor\n");
    nexus_mutex_lock(&(attach_server_monitor.mutex));
    while (!(attach_server_monitor.done))
    {
	nexus_cond_wait(&(attach_server_monitor.cond),
			&(attach_server_monitor.mutex));
    }
    nexus_mutex_unlock(&(attach_server_monitor.mutex));
    nexus_printf("attach_server(): awoke from monitor\n");
    
    nexus_disallow_attach(port);

    nexus_mutex_destroy(&(attach_server_monitor.mutex));
    nexus_cond_destroy(&(attach_server_monitor.cond));

    nexus_printf("attach_server(): exiting\n");
    
} /* attach_server() */


/*
 * attach_client()
 */
static void attach_client(void)
{
    char url[1024];
    nexus_startpoint_t sp;
    nexus_buffer_t buffer;
    int rc;

    nexus_printf("attach_client(): entering\n");

    nexus_enable_fault_tolerance(fault_callback, NULL);

    sprintf(url, "x-nexus://%s:%hu/spec1//spec\\/3/spec4",
	    attach_client_host,
	    attach_client_port);

    nexus_printf("attach_client(): attaching to url <%s>\n", url);
    rc = nexus_attach(url, &sp);
    if (rc == 0)
    {
	nexus_printf("attach_client(): running comm_test() to server\n");
	comm_test(&sp, NEXUS_FALSE);
	nexus_printf("attach_client(): done running comm_test() to server\n");

	if (!attach_client_leave_server)
	{
	    /* Send rsr to shut the server down */
	    nexus_printf("attach_client(): sending finish message to server\n");
	    nexus_buffer_init(&buffer, 0, 0);
	    if (nexus_send_rsr(&buffer, &sp, 4, NEXUS_TRUE, NEXUS_FALSE) != 0)
	    {
		nexus_printf("attach_client(): WARNING: send failed\n");
	    }
	}

	nexus_printf("attach_client(): destroying gp to server\n");
	nexus_startpoint_destroy(&sp);
    }
    else
    {
	nexus_printf("attach_client(): ERROR: nexus_attach() failed with rc=%d\n", rc);
    }
    
    nexus_printf("attach_client(): exiting\n");
    
} /* attach_client() */


/*
 * attach_server_finish_handler()
 */
static void attach_server_finish_handler(nexus_endpoint_t *endpoint,
				 nexus_buffer_t *buffer,
				 nexus_bool_t called_from_non_threaded_handler)
{
    nexus_printf("attach_server_finish_handler(): entering\n");

    nexus_printf("attach_server_finish_handler(): signaling\n");
    nexus_mutex_lock(&(attach_server_monitor.mutex));
    attach_server_monitor.done = NEXUS_TRUE;
    nexus_cond_signal(&(attach_server_monitor.cond));
    nexus_mutex_unlock(&(attach_server_monitor.mutex));

    nexus_printf("attach_server_finish_handler(): exiting\n");
} /* attach_server_finish_handler() */

#endif /* ATTACH_TEST */
