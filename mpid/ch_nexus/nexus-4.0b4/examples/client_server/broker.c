/*
 * broker.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_examples/client_server/broker.c,v 1.5 1996/03/21 23:32:40 geisler Exp $";

#include "nexus.h"
#include "nx_c2c.h"

#ifndef EXECUTABLE_DIR
#define EXECUTABLE_DIR "/home/compose/nexus/cave2cave/"
#endif

#ifndef EXECUTABLE_NAME
#define EXECUTABLE_NAME "broker"
#endif

/*
 * My state
 */
static unsigned short	attach_port;
static char *		attach_host;
static char		attach_url[1024];

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

static int attach_requested(void *arg, char *url, nexus_global_pointer_t *gp);
static void allow_attachments();
static void disallow_attachments();


/*
 * Declare all of the broker's handlers here
 */
void NexusUnknownHandler(void *address, nexus_buffer_t *buffer,
			 char *handler_name, int handler_id);
static void broker_shutdown_handler(void *address,
				    nexus_buffer_t *buffer);
static void broker_query_handler(void *address,
				 nexus_buffer_t *buffer);

static nexus_handler_t handlers[] =
{ \
  {"NexusUnknownHandler",
       NEXUS_UNKNOWN_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NexusUnknownHandler},
  {BROKER_SHUTDOWN_HANDLER_NAME,
       BROKER_SHUTDOWN_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) broker_shutdown_handler},
  {BROKER_QUERY_HANDLER_NAME,
       BROKER_QUERY_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) broker_query_handler},
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
    FILE *fp;

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

    /* Setup monitor that I'll suspend on */
    nexus_mutex_init(&(sleep_monitor.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(sleep_monitor.cond), (nexus_condattr_t *) NULL);
    sleep_monitor.done = NEXUS_FALSE;

    /* Allow clients to attach */
    allow_attachments();

    if (!write_attach_file(attach_url, URL_FILE))
    {
	nexus_stdio_lock();
	fprintf(stderr, "ERROR: Could not write file: %s\n", URL_FILE);
	nexus_stdio_unlock();
	nexus_abort();
    }
	   
    /*
     * Suspend, waiting for shutdown
     * Handle all requests asynchonously in the mean time.
     */
    debug_printf(("main(): going to sleep\n"));
    nexus_mutex_lock(&(sleep_monitor.mutex));
    while (!(sleep_monitor.done))
    {
	nexus_cond_wait(&(sleep_monitor.cond), &(sleep_monitor.mutex));
    }
    nexus_mutex_unlock(&(sleep_monitor.mutex));
    
    /* Disallow clients to attach */
    disallow_attachments();

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
 * attach_requested()
 */
static int attach_requested(void *arg, char *url, nexus_global_pointer_t *gp)
{
    debug_printf(("attach_request(): Allowing attachment\n"));
    nexus_global_pointer(gp, NULL);
    return(0);
} /* attach_requested() */


/*
 * allow_attachments()
 */
static void allow_attachments()
{
    int rc;
    
    attach_port = 0;
    rc = nexus_allow_attach(&attach_port, &attach_host, &attach_requested,NULL);
    if (rc != 0)
    {
	nexus_stdio_lock();
	fprintf(stderr, "ERROR: nexus_allow_attach() failed: rc=%d\n", rc);
	nexus_stdio_unlock();
	nexus_abort();
    }

    sprintf(attach_url, "x-nexus://%s:%hu/", attach_host, attach_port);
    
} /* allow_attachments() */


/*
 * disallow_attachments()
 */
static void disallow_attachments()
{
    nexus_disallow_attach(attach_port);
} /* disallow_attachments() */


/*
 * broker_shutdown_handler()
 */
static void broker_shutdown_handler(void *address,
				    nexus_buffer_t *buffer)
{
    debug_printf(("broker_shutdown_handler(): shutting down broker\n"));

    nexus_mutex_lock(&(sleep_monitor.mutex));
    sleep_monitor.done = NEXUS_TRUE;
    nexus_cond_signal(&(sleep_monitor.cond));
    nexus_mutex_unlock(&(sleep_monitor.mutex));

} /* broker_shutdown_handler() */


/*
 * broker_query_handler()
 */
static void broker_query_handler(void *address,
				 nexus_buffer_t *buffer)
{
    int query_type;
    nexus_global_pointer_t reply_gp;
    nexus_buffer_t reply_buffer;
    int i;
    char *client1 = "client1";
    char *session1 = "session1";
    char *session2 = "session2";
    
    nexus_get_int(buffer, &query_type, 1);
    nexus_get_global_pointer(buffer, &reply_gp, 1);
    
    switch(query_type)
    {
    case BROKER_QUERY_CLIENTS:
	nexus_init_remote_service_request(&reply_buffer, &reply_gp,
				  BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_NAME,
				  BCONTROL_QUERY_CLIENTS_REPLY_HANDLER_HASH);
	nexus_set_buffer_size(&reply_buffer,
			      (nexus_sizeof_int(&reply_buffer, 2)
			       + nexus_sizeof_char(&reply_buffer,
						   strlen(client1))),
			      3);
	i = 1;
	nexus_put_int(&reply_buffer, &i, 1);
	i = strlen(client1);
	nexus_put_int(&reply_buffer, &i, 1);
	nexus_put_char(&reply_buffer, client1, i);
	nexus_send_remote_service_request(&reply_buffer);
	break;
    case BROKER_QUERY_SESSIONS:
	nexus_init_remote_service_request(&reply_buffer, &reply_gp,
				  BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_NAME,
				  BCONTROL_QUERY_SESSIONS_REPLY_HANDLER_HASH);
	nexus_set_buffer_size(&reply_buffer,
			      (nexus_sizeof_int(&reply_buffer, 3)
			       + nexus_sizeof_char(&reply_buffer,
						   strlen(session1))
			       + nexus_sizeof_char(&reply_buffer,
						   strlen(session2)) ),
			      5);
	i = 2;
	nexus_put_int(&reply_buffer, &i, 1);
	i = strlen(session1);
	nexus_put_int(&reply_buffer, &i, 1);
	nexus_put_char(&reply_buffer, session1, i);
	i = strlen(session2);
	nexus_put_int(&reply_buffer, &i, 1);
	nexus_put_char(&reply_buffer, session2, i);
	nexus_send_remote_service_request(&reply_buffer);
	break;
    default:
	nexus_stdio_lock();
	fprintf(stderr, "ERROR: broker_query_handler(): Unknown query type: %d\n", query_type);
	nexus_stdio_unlock();
	break;
    }
} /* broker_query_handler() */
