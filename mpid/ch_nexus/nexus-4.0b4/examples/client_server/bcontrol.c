/*
 * bcontrol.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_examples/client_server/bcontrol.c,v 1.5 1996/03/21 23:32:41 geisler Exp $";

#include "nexus.h"
#include "nx_c2c.h"

#ifndef EXECUTABLE_DIR
#define EXECUTABLE_DIR "/home/compose/nexus/cave2cave/"
#endif

#ifndef EXECUTABLE_NAME
#define EXECUTABLE_NAME "bcontrol"
#endif

/*
 * My state
 */
static char			attach_url[1024];
static nexus_bool_t		do_shutdown = NEXUS_FALSE;
static nexus_bool_t		do_query = NEXUS_FALSE;
static int			query_type;
static nexus_global_pointer_t	broker_gp;

/*
 * This is what the main thread will sleep on, until told to shutdown
 */
typedef struct
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    nexus_bool_t	done;
} sleep_monitor_t;
static sleep_monitor_t sleep_monitor;

static void attach_to_broker();
static void send_broker_shutdown_message();
static void query_broker();

/*
 * Declare all of the broker's handlers here
 */
void NexusUnknownHandler(void *address, nexus_buffer_t *buffer,
			 char *handler_name, int handler_id);
static void bcontrol_query_clients_reply_handler(void *address,
						 nexus_buffer_t *buffer);
static void bcontrol_query_sessions_reply_handler(void *address,
						  nexus_buffer_t *buffer);

static nexus_handler_t handlers[] =
{ \
  {"NexusUnknownHandler",
       NEXUS_UNKNOWN_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NexusUnknownHandler},
  {BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_NAME,
       BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) bcontrol_query_clients_reply_handler},
  {BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_NAME,
       BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) bcontrol_query_sessions_reply_handler},
  {(char *) NULL,
       0,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};


/*
 * NexusUnknownHandler()
 */
void NexusUnknownHandler(void *address, nexus_buffer_t *buffer,
			 char *handler_name, int handler_id)
{
    debug_printf(("NexusUnknownHandler(): Unknown handler invoked: handler_name=%s, handler_id=%d\n", handler_name, handler_id));
} /* NexusUnknownHandler() */

/*
 * NexusBoot()
 */
int NexusBoot()
{
    nexus_register_handlers(handlers);
    return (0);
} /* NexusBoot() */


/*****************************************************************
 *		MAIN
 *****************************************************************/

/*
 * main()
 */
void main(int argc, char **argv)
{
    nexus_node_t *nodes;
    int n_nodes;
    int i;

    nexus_init(&argc,
	       &argv,
	       "NEXUS_ARGS",	/* environment_variable_name */
	       "nx",		/* package argument separator */
	       NULL,		/* package_args_init function */
	       NULL,		/* usage_message function */
	       NULL,		/* new_process_params function */
	       NULL,		/* module_list */
	       &nodes,
	       &n_nodes);
    nexus_start();

    attach_url[0] = '\0';
    for (i = 1; i < argc; i++)
    {
	if (strcmp(argv[i], "-a") == 0)
	{
	    strcpy(attach_url, argv[i+1]);
	    i++;
	}
	else if (strcmp(argv[i], "-shutdown") == 0)
	{
	    do_shutdown = NEXUS_TRUE;
	}
	else if (strcmp(argv[i], "-query") == 0)
	{
	    do_query = NEXUS_TRUE;
	    if (strcmp(argv[i+1], "clients") == 0)
	    {
		query_type = BROKER_QUERY_CLIENTS;
	    }
	    else if (strcmp(argv[i+1], "sessions") == 0)
	    {
		query_type = BROKER_QUERY_SESSIONS;
	    }
	}
    }

    if (    attach_url[0] == '\0'
	&& !read_attach_file(attach_url, URL_FILE) )
    {
	nexus_stdio_lock();
	fprintf(stderr, "ERROR: Could not read file: %s\n", URL_FILE);
	nexus_stdio_unlock();
	nexus_abort();
    }

    attach_to_broker();

    /* Setup monitor that I'll suspend on */
    nexus_mutex_init(&(sleep_monitor.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(sleep_monitor.cond), (nexus_condattr_t *) NULL);
    sleep_monitor.done = NEXUS_FALSE;

    if (do_query)
    {
	query_broker();
    }
    
    if (do_shutdown)
    {
	send_broker_shutdown_message();
    }

    nexus_destroy_global_pointer(&broker_gp);
    
 alldone:
    
    debug_printf(("main(): shutting down\n"));
    for(i = 0; i < n_nodes; i++)
    {
	nexus_free(nodes[i].name);
	nexus_destroy_global_pointer(&(nodes[i].gp));
    }
    nexus_free(nodes);
    
    nexus_destroy_current_context(NEXUS_FALSE);

} /* main() */


/*
 * attach_to_broker()
 */
static void attach_to_broker()
{
    int rc;
    rc = nexus_attach(attach_url, &broker_gp);
    if (rc != 0)
    {
	nexus_stdio_lock();
	fprintf(stderr, "ERROR: Could not attach to broker using url: %s\n",
		attach_url);
	nexus_stdio_unlock();
	nexus_abort();
    }
} /* attach_to_broker() */


/*
 * send_broker_shutdown_message()
 */
static void send_broker_shutdown_message()
{
    nexus_buffer_t buffer;

    debug_printf(("send_broker_shutdown_message(): sending shutdown message to broker\n"));
    nexus_init_remote_service_request(&buffer, &broker_gp,
				      BROKER_SHUTDOWN_HANDLER_NAME,
				      BROKER_SHUTDOWN_HANDLER_HASH);
    nexus_send_remote_service_request(&buffer);
    debug_printf(("send_broker_shutdown_message(): message sent\n"));
				      
} /* send_broker_shutdown_message() */


/*
 * query_broker()
 */
static void query_broker()
{
    nexus_buffer_t buffer;
    nexus_global_pointer_t gp;
    int gp_size, gp_n_elements;

    nexus_global_pointer(&gp, NULL);

    debug_printf(("query_broker(): sending query message to broker\n"));
    nexus_init_remote_service_request(&buffer, &broker_gp,
				      BROKER_QUERY_HANDLER_NAME,
				      BROKER_QUERY_HANDLER_HASH);
    gp_size = nexus_sizeof_global_pointer(&buffer, &gp, 1,
					  &gp_n_elements);
    nexus_set_buffer_size(&buffer,
			  gp_size + nexus_sizeof_int(&buffer, 1),
			  gp_n_elements + 1);
    nexus_put_int(&buffer, &query_type, 1);
    nexus_put_global_pointer(&buffer, &gp, 1);
    nexus_send_remote_service_request(&buffer);
    debug_printf(("query_broker(): message sent\n"));
				      
    /*
     * Suspend, waiting for reply
     */
    debug_printf(("query_broker(): waiting for reply\n"));
    nexus_mutex_lock(&(sleep_monitor.mutex));
    while (!(sleep_monitor.done))
    {
	nexus_cond_wait(&(sleep_monitor.cond), &(sleep_monitor.mutex));
    }
    sleep_monitor.done = NEXUS_FALSE;
    nexus_mutex_unlock(&(sleep_monitor.mutex));
    debug_printf(("query_broker(): got reply\n"));

} /* query_broker() */


/*
 * bcontrol_query_clients_reply_handler()
 */
static void bcontrol_query_clients_reply_handler(void *address,
						 nexus_buffer_t *buffer)
{
    int num;
    int i;
    int length;
    char string[1024];

    nexus_get_int(buffer, &num, 1);
    
    for (i = 0; i < num; i++)
    {
	nexus_get_int(buffer, &length, 1);
	nexus_get_char(buffer, string, length);
	string[length] = '\0';
	nexus_stdio_lock();
	printf("client %d: %s\n", i, string);
	nexus_stdio_unlock();
    }
    
    nexus_mutex_lock(&(sleep_monitor.mutex));
    sleep_monitor.done = NEXUS_TRUE;
    nexus_cond_signal(&(sleep_monitor.cond));
    nexus_mutex_unlock(&(sleep_monitor.mutex));
} /* bcontrol_query_clients_reply_handler() */


/*
 * bcontrol_query_sessions_reply_handler()
 */
static void bcontrol_query_sessions_reply_handler(void *address,
						  nexus_buffer_t *buffer)
{
    int num;
    int i;
    int length;
    char string[1024];

    nexus_get_int(buffer, &num, 1);
    
    for (i = 0; i < num; i++)
    {
	nexus_get_int(buffer, &length, 1);
	nexus_get_char(buffer, string, length);
	string[length] = '\0';
	nexus_stdio_lock();
	printf("session %d: %s\n", i, string);
	nexus_stdio_unlock();
    }
    
    nexus_mutex_lock(&(sleep_monitor.mutex));
    sleep_monitor.done = NEXUS_TRUE;
    nexus_cond_signal(&(sleep_monitor.cond));
    nexus_mutex_unlock(&(sleep_monitor.mutex));
} /* bcontrol_query_sessions_reply_handler() */

