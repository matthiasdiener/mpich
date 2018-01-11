/*
 * ring.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_examples/ring/ring.c,v 1.8 1996/02/06 19:40:04 geisler Exp $";

#include <nexus.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>

/*
#define NEXUS_ATTACH_VERSION
*/

/*
 * monitor_t
 */
typedef struct _monitor_t
{
    nexus_mutex_t mutex;
    nexus_cond_t  cond;
    int           count;
} monitor_t;

/*
 * ring_link_t
 */
typedef struct _ring_link_t
{
    nexus_global_pointer_t next;
    nexus_stashed_buffer_t stashed_buffer;
    nexus_bool_t           master_node;
} ring_link_t;

/*
 * attach_t
 */
typedef struct _attach_t
{
    nexus_mutex_t mutex;
    nexus_global_pointer_t *ring_list;
    int next;
} attach_t;


/* Global variables */
static monitor_t		monitor;
static nexus_global_pointer_t	monitor_gp;

/* Function externs */
static void connect_ring(nexus_global_pointer_t *ring_list,
			 int n_nodes);
static void send_messages(nexus_global_pointer_t *first_node,
			  ring_link_t *ring_link,
			  char *message,
			  int count);


/* Handlers */
#define REGISTER_RING_LINK_HANDLER_NAME "register_ring_link_handler"
#define REGISTER_RING_LINK_HANDLER_HASH 502
static void register_ring_link_handler(void *address, nexus_buffer_t *buffer);

#define SETUP_LINK_HANDLER_NAME "setup_link_handler"
#define SETUP_LINK_HANDLER_HASH 894
static void setup_link_handler(void *address, nexus_buffer_t *buffer);

#define SIGNAL_MONITOR_HANDLER_NAME "signal_monitor_handler"
#define SIGNAL_MONITOR_HANDLER_HASH 296
static void signal_monitor_handler(void *address, nexus_buffer_t *buffer);

#define RING_NODE_HANDLER_NAME "ring_node_handler"
#define RING_NODE_HANDLER_HASH 757
static void ring_node_handler(void *address, nexus_buffer_t *buffer);


#ifdef NEXUS_ATTACH_VERSION

/* Attachment related function */
static void server(int num_clients, char *message, int count);
static void client(char *url);
static int attach_request(void *arg, char *url, nexus_global_pointer_t *gp);

#else  /* NEXUS_ATTACH_VERSION */

/* Non-attachment related function */
static void master(nexus_node_t *nodes,
		   int n_nodes,
		   char *message,
		   int count);
static void create_contexts(nexus_global_pointer_t *context_gps,
			    nexus_node_t *nodes,
			    int n_contexts);
static void create_ring_links(nexus_global_pointer_t *context_gps,
			      int n_contexts,
			      nexus_global_pointer_t *ring_list,
			      ring_link_t *ring_link);
static void destroy_contexts_and_nodes(nexus_global_pointer_t *context_gps,
				       nexus_node_t *nodes,
				       int n_nodes);

/* Non-attachment related handlers */
#define ALLOCATE_RING_LINK_HANDLER_NAME "allocate_ring_link_handler"
#define ALLOCATE_RING_LINK_HANDLER_HASH 676
static void allocate_ring_link_handler(void *address, nexus_buffer_t *buffer);

#define SHUTDOWN_HANDLER_NAME "shutdown_handler"
#define SHUTDOWN_HANDLER_HASH 700
static void shutdown_handler(void *address, nexus_buffer_t *buffer);

#endif /* NEXUS_ATTACH_VERSION */


static nexus_handler_t handlers[] =
{
    {REGISTER_RING_LINK_HANDLER_NAME,
         REGISTER_RING_LINK_HANDLER_HASH,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) register_ring_link_handler},
    {SETUP_LINK_HANDLER_NAME,
         SETUP_LINK_HANDLER_HASH,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) setup_link_handler},
    {SIGNAL_MONITOR_HANDLER_NAME,
         SIGNAL_MONITOR_HANDLER_HASH,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) signal_monitor_handler},
    {RING_NODE_HANDLER_NAME,
         RING_NODE_HANDLER_HASH,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) ring_node_handler},
#ifndef NEXUS_ATTACH_VERSION
    {ALLOCATE_RING_LINK_HANDLER_NAME,
         ALLOCATE_RING_LINK_HANDLER_HASH,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) allocate_ring_link_handler},
    {SHUTDOWN_HANDLER_NAME,
	 SHUTDOWN_HANDLER_HASH,
	 NEXUS_HANDLER_TYPE_NON_THREADED,
	 (nexus_handler_func_t) shutdown_handler},
#endif /* NEXUS_ATTACH_VERSION */
    {(char *) NULL,
         0,
         NEXUS_HANDLER_TYPE_NON_THREADED,
         (nexus_handler_func_t) NULL},
};

#define ElapseTime(Start, Stop) \
    (  (  ( ((double)(Stop.tv_sec)) - ((double)(Start.tv_sec)) ) \
	+ (((double)(Stop.tv_usec))/1000000.0) ) \
     - (((double)(Start.tv_usec))/1000000.0) )


/*
 * NexusBoot()
 */
int NexusBoot()
{
    nexus_register_handlers(handlers);
    return (0);
} /* NexusBoot() */


/*
 * main()
 */
int main(int argc, char **argv)
{
    nexus_node_t *nodes;
    int n_nodes;
    int i;
    
    nexus_init(&argc, &argv, "NEXUS_ARGS", "nx", NULL, NULL, NULL, NULL,
	       &nodes, &n_nodes);
    nexus_start();

    nexus_mutex_init(&(monitor.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(monitor.cond), (nexus_condattr_t *) NULL);
    nexus_global_pointer(&monitor_gp, (void *) &monitor);

#ifdef NEXUS_ATTACH_VERSION
    /*
     * Attachment version
     */
    if ((argc == 5) && (strcmp(argv[1], "-server") == 0))
    {
	int num_clients = atoi(argv[2]);
	char *message = argv[3];
	int count = atoi(argv[4]);

	if (num_clients < 1)
	{
	    printf("Error: <num_clients> must be >= 1\n");
	    goto usage;
	}
	if (count < 1)
	{
	    printf("Error: <count> must be >= 1\n");
	    goto usage;
	}

	server(num_clients, message, count);
	goto done;
    }
    else if ((argc == 3) && (strcmp(argv[1], "-client") == 0))
    {
	char *url = argv[2];

	if (strncmp(url, "x-nexus://", 10) != 0)
	{
	    printf("Error: <server-url> must be for the form \"x-nexus://<host>:<port>/\"\n");
	    goto usage;
	}

	client(url);
	goto done;
    }

 usage:
    printf("Usage:\n");
    printf("      %s -server <num_clients> <message> <count>\n", argv[0]);
    printf("   or:\n");
    printf("      %s -client <server_url>\n", argv[0]);
 done:
    
#else  /* NEXUS_ATTACH_VERSION */
    /*
     * Non-attachment version
     */
    if (argc == 3)
    {
	char *message = argv[1];
	int count = atoi(argv[2]);

	if (count < 1)
	{
	    printf("Error: <count> must be >= 1\n");
	    goto usage;
	}
	if (n_nodes < 2)
	{
	    printf("Error: number of nodes must be >= 2\n");
	    goto usage;
	}
	
	for (i=0; i < n_nodes; ++i)
	{
	    nexus_printf("nodes[%d]: %s#%d, rc=%d\n", i, nodes[i].name,
			 nodes[i].number, nodes[i].return_code);
	}
	
	master(nodes, n_nodes, message, count);
	goto done;
    }
    
 usage:
    printf("Usage:\n");
    printf("      %s <message> <count>\n", argv[0]);
 done:

#endif /* NEXUS_ATTACH_VERSION */
    
    nexus_mutex_destroy(&(monitor.mutex));
    nexus_cond_destroy(&(monitor.cond));
    nexus_destroy_global_pointer(&monitor_gp);

    /* Free up the nodes structure */
    for (i=0; i < n_nodes; ++i)
    {
	nexus_free(nodes[i].name);
	nexus_destroy_global_pointer(&(nodes[i].gp));
    }
    nexus_free(nodes);

    /* Shutdown this node */
    nexus_destroy_current_context(NEXUS_FALSE);
    return(0);
} /* main() */
    

/*
 * connect_ring()
 *
 * Send a message to each ring_link, telling it to set its 'next'
 * field to the next global pointer in the ring.
 */
static void connect_ring(nexus_global_pointer_t *ring_list,
			  int n_nodes)
{
    nexus_buffer_t buffer;
    int i, next;
    int gp_size_1, gp_size_2, gp_n_elements_1, gp_n_elements_2;

    monitor.count = n_nodes;
    for (i = 0; i < n_nodes; i++)
    {
	next = (i+1)%n_nodes;
	nexus_init_remote_service_request(&buffer, &(ring_list[i]),
					  "setup_link_handler",
					  SETUP_LINK_HANDLER_HASH);
	gp_size_1 = nexus_sizeof_global_pointer(&buffer,
						&(ring_list[next]), 1,
						&gp_n_elements_1);
	gp_size_2 = nexus_sizeof_global_pointer(&buffer,
						&monitor_gp, 1,
						&gp_n_elements_2);
	nexus_set_buffer_size(&buffer,
			      (gp_size_1 + gp_size_2),
			      (gp_n_elements_1 + gp_n_elements_2) );
	nexus_put_global_pointer(&buffer, &(ring_list[next]), 1);
	nexus_put_global_pointer(&buffer, &monitor_gp, 1);
	nexus_send_remote_service_request(&buffer);
    }
       
    /* wait until all the links have been set up */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex));
    }
    nexus_mutex_unlock(&(monitor.mutex));
} /* connect_ring() */


/*
 * setup_link_handler()
 */
static void setup_link_handler(void *address, nexus_buffer_t *buffer)
{
    ring_link_t *ring_link = ((ring_link_t *) address);
    nexus_buffer_t return_buffer;
    nexus_global_pointer_t reply_monitor_gp;
    
    nexus_get_global_pointer(buffer, &(ring_link->next), 1);   
    nexus_get_global_pointer(buffer, &reply_monitor_gp, 1);   

    /*  goto node 0 where monitor is and modify it */
    nexus_init_remote_service_request(&return_buffer, &reply_monitor_gp,
				      "signal_monitor_handler",
				      SIGNAL_MONITOR_HANDLER_HASH);
    nexus_send_remote_service_request(&return_buffer);
    nexus_destroy_global_pointer(&reply_monitor_gp);
} /* setup_link_handler() */


/*
 * signal_monitor_handler()
 */
static void signal_monitor_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_mutex_lock(&(monitor.mutex));
    if (--monitor.count == 0)
    {
	nexus_cond_signal(&(monitor.cond));
    }
    nexus_mutex_unlock(&(monitor.mutex));
} /* signal_monitor_handler() */


/*
 * send_messages()
 */
static void send_messages(nexus_global_pointer_t *first_node,
			  ring_link_t *ring_link,
			  char *message,
			  int count)
{
    nexus_buffer_t buffer;
    int i;
    int message_size;
    int message_size_received;
    char *message_received;
    struct timeval start_time, stop_time;
    struct timeval start_total_time, stop_total_time;

    message_size = strlen(message)+1;
    message_received = nexus_malloc(sizeof(char)*message_size);

    /* Use the monitor to signal completion of a single lap */
    monitor.count = 1;

    gettimeofday(&start_total_time, NULL);
    for (i = 1; i <= count; i++)
    {
	nexus_printf("Message #%d\n", i);
	nexus_printf("    Master sending: %s\n", message);
	gettimeofday(&start_time, NULL);

	/* Send message to first ring link */
	nexus_init_remote_service_request(&buffer, first_node,
					  "ring_node_handler",
					  RING_NODE_HANDLER_HASH);
	nexus_set_buffer_size(&buffer,
			      (nexus_sizeof_char(&buffer, message_size)
			       + nexus_sizeof_int(&buffer, 1)),
			      2);
	nexus_put_int(&buffer, &message_size, 1);
	nexus_put_char(&buffer, message, message_size);
	nexus_send_remote_service_request(&buffer);

	/* Wait for message to get back to me */
	nexus_mutex_lock(&(monitor.mutex));
	while (monitor.count > 0)
	{
	    nexus_cond_wait(&(monitor.cond), &(monitor.mutex));
	}
	monitor.count = 1;
	nexus_mutex_unlock(&(monitor.mutex));

	/* Read the message and check it */
	nexus_get_stashed_int(&(ring_link->stashed_buffer),
			      &message_size_received, 1);
	nexus_get_stashed_char(&(ring_link->stashed_buffer),
			       message_received, message_size_received);
	gettimeofday(&stop_time, NULL);
	if (   (message_size != message_size_received)
	    || (strcmp(message, message_received) != 0) )
	{
	    nexus_fatal("Error: Message does not match\n");
	}
	nexus_printf("    Master received: %s\n", message);
	nexus_printf("    Elasped Time: %lf\n",
		     ElapseTime(start_time, stop_time) );

	nexus_free_stashed_buffer(&ring_link->stashed_buffer);
    }
    gettimeofday(&stop_total_time, NULL);
    nexus_printf("Total Elapsed Time: %lf sec\n",
		 ElapseTime(start_total_time, stop_total_time) );

    nexus_free(message_received);
    
} /* send_messages() */


/*
 * ring_node_handler()
 */
static void ring_node_handler(void *address, nexus_buffer_t *buffer)
{
    ring_link_t *ring_link = ((ring_link_t *) address);
    nexus_buffer_t send_buffer;
    int message_size;
    char *message;
    
    /* normal node, send message on to next node */
    if (ring_link->master_node == NEXUS_FALSE)
    {
	/* Non-master node: forward message on */
	
	/* Get the message from the buffer */
	nexus_get_int(buffer, &message_size, 1);
	message = nexus_malloc(sizeof(char) * message_size);
	nexus_get_char(buffer, message, message_size);
	nexus_printf("        Link received: %s\n", message);

	/* Send the message on to the next ring link */
	nexus_init_remote_service_request(&send_buffer, &(ring_link->next),
					  "ring_node_handler",
					  RING_NODE_HANDLER_HASH);
	nexus_set_buffer_size(&send_buffer,
			      (nexus_sizeof_char(&send_buffer, message_size)
			       + nexus_sizeof_int(&send_buffer, 1)),
			      2);
	nexus_put_int(&send_buffer, &message_size, 1);
	nexus_put_char(&send_buffer, message, message_size);
	nexus_send_remote_service_request(&send_buffer);
	nexus_free(message);
    }
    else
    {
	/* Master node: stash the buffer and signal the main thread */

	nexus_stash_buffer(buffer, &(ring_link->stashed_buffer));
	nexus_mutex_lock(&(monitor.mutex));
	monitor.count = 0;
	nexus_cond_signal(&(monitor.cond));
	nexus_mutex_unlock(&(monitor.mutex));
    }
} /* ring_node_handler() */


/*
 * register_ring_link_handler()
 */
static void register_ring_link_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_global_pointer_t *ring_link_gp
	= ((nexus_global_pointer_t *) address);

    nexus_get_global_pointer(buffer, ring_link_gp, 1);
    
    nexus_mutex_lock(&(monitor.mutex));
    if (--monitor.count == 0)
    {
	nexus_cond_signal(&(monitor.cond));
    }
    nexus_mutex_unlock(&(monitor.mutex));
} /* register_ring_link_handler() */


#ifdef NEXUS_ATTACH_VERSION

/****************************************************************
 *	Attachment version
 ****************************************************************/

/*
 * server()
 */
static void server(int num_clients, char *message, int count)
{
    attach_t attach;
    nexus_global_pointer_t ring_link_gp;
    ring_link_t *ring_link;
    nexus_buffer_t buffer;
    unsigned short port;
    char *host;
    int rc, i;
    nexus_global_pointer_t *ring_list;

    ring_list = nexus_malloc(sizeof(nexus_global_pointer_t) *
			     (num_clients + 1) );
    
    /* Allocate the ring link for this server context */
    ring_link = nexus_malloc(sizeof(ring_link_t));
    ring_link->master_node = NEXUS_TRUE;
    nexus_global_pointer(&ring_link_gp, (void *) ring_link);
    ring_list[0] = ring_link_gp;

    /* Setup attachment so that clients can attach into ring */
    monitor.count = num_clients;
    nexus_mutex_init(&(attach.mutex), (nexus_mutexattr_t *) NULL);
    attach.ring_list = ring_list;
    attach.next = 1;
    port = 0;
    if (nexus_allow_attach(&port, &host,
			   &attach_request,
			   (void *) &attach) != 0)
    {
	nexus_fatal("Attachment failed\n");
    }
    nexus_printf("Server URL: x-nexus://%s:%hu/\n", host, port);

    /* Wait until all the clients have given me their gps */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex));
    }
    nexus_mutex_unlock(&(monitor.mutex));
    
    nexus_disallow_attach(port);
    nexus_mutex_destroy(&(attach.mutex));

    connect_ring(ring_list, num_clients + 1);
    send_messages(&(ring_list[1]), ring_link, message, count);
    
    /*
     * Tell all the clients that we are done sending,
     * so that they can shutdown
     */
    for (i = 1; i <= num_clients; i++)
    {
	nexus_init_remote_service_request(&buffer,
					  &(ring_list[i]),
					  "signal_monitor_handler",
					  SIGNAL_MONITOR_HANDLER_HASH);
	nexus_send_remote_service_request(&buffer);
    }

    /* Destroy my global pointer to the client */
    for (i = 0; i < num_clients + 1; i++)
    {
	nexus_destroy_global_pointer(&(ring_list[i]));
    }
    nexus_free(ring_list);
    
    nexus_destroy_global_pointer(&ring_link_gp);
    nexus_destroy_global_pointer(&(ring_link->next));
    nexus_free(ring_link);

} /* server() */


/*
 * attach_request()
 */
static int attach_request(void *arg, char *url, nexus_global_pointer_t *gp)
{
    attach_t *attach = (attach_t *) arg;
    nexus_mutex_lock(&(attach->mutex));
    nexus_global_pointer(gp, (void *) &(attach->ring_list[attach->next++]));
    nexus_mutex_unlock(&(attach->mutex));
    return (0);
} /* attach_request() */


/*
 * client()
 */
static void client(char *url)
{
    nexus_global_pointer_t server_gp;
    ring_link_t *ring_link;
    nexus_global_pointer_t ring_link_gp;
    nexus_buffer_t buffer;
    int gp_size, gp_n_elements;

    /* Use the monitor to signal termination */
    monitor.count = 1;

    /* Attach to the server */
    if (nexus_attach(url, &server_gp) != 0)
    {
	nexus_fatal("Failed to attach to server\n");
    }

    /* Allocate a ring link in this context */
    ring_link = nexus_malloc(sizeof(ring_link_t));
    ring_link->master_node = NEXUS_FALSE;

    /* Register my ring link with the server */
    nexus_global_pointer(&ring_link_gp, (void *) ring_link);
    nexus_init_remote_service_request(&buffer, &server_gp,
				      "register_ring_link_handler",
				      REGISTER_RING_LINK_HANDLER_HASH);
    gp_size = nexus_sizeof_global_pointer(&buffer,
					  &ring_link_gp, 1,
					  &gp_n_elements);
    nexus_set_buffer_size(&buffer, gp_size, gp_n_elements);
    nexus_put_global_pointer(&buffer, &ring_link_gp, 1);
    nexus_send_remote_service_request(&buffer);
    nexus_destroy_global_pointer(&ring_link_gp);

    /* Wait until server tells us we are done */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex));
    }
    nexus_mutex_unlock(&(monitor.mutex));

    nexus_destroy_global_pointer(&(ring_link->next));
    nexus_free(ring_link);
    
} /* client() */

#else  /* NEXUS_ATTACH_VERSION */

/****************************************************************
 *	Non-attachment version
 ****************************************************************/

/*
 * master()
 */
static void master(nexus_node_t *nodes,
		   int n_nodes,
		   char *message,
		   int count)
{
    nexus_global_pointer_t *context_gps;
    nexus_global_pointer_t *ring_list;
    ring_link_t *ring_link;
    int i;
    
    ring_link = nexus_malloc(sizeof(ring_link_t));
    ring_link->master_node = NEXUS_TRUE;

    context_gps = nexus_malloc(sizeof(nexus_global_pointer_t)*(n_nodes-1));
    create_contexts(context_gps, nodes, n_nodes-1);
    
    ring_list = nexus_malloc(sizeof(nexus_global_pointer_t)*n_nodes);
    create_ring_links(context_gps, n_nodes-1, ring_list, ring_link);
    
    connect_ring(ring_list, n_nodes);
    send_messages(&(ring_list[1]), ring_link, message, count);
    
    destroy_contexts_and_nodes(context_gps, nodes, n_nodes);

    for (i = 0; i < n_nodes; i++)
    {
	nexus_destroy_global_pointer(&(ring_list[i]));
    }
    nexus_free(ring_list);
    for (i = 0; i < n_nodes-1; i++)
    {
	nexus_destroy_global_pointer(&(context_gps[i]));
    }
    nexus_free(context_gps);
    nexus_destroy_global_pointer(&(ring_link->next));
    nexus_free(ring_link);

} /* master() */


/*
 * create_contexts()
 */
static void create_contexts(nexus_global_pointer_t *context_gps,
			    nexus_node_t *nodes,
			    int n_contexts)
{
    int i;
    int *ret_code;
    nexus_create_context_handle_t contexts;

    ret_code = nexus_malloc(sizeof(int)*n_contexts);

    /* Create one context on each node except the master */
    nexus_init_create_context_handle(&contexts, n_contexts);
    for (i = 0; i < n_contexts; i++)
    {
	nexus_create_context(&(nodes[i+1].gp),
			     "resource_database:ring",
			     0,
			     &(context_gps[i]),
			     &(ret_code[i]),
			     &contexts);
    }
    nexus_create_context_wait(&contexts);

    /* Check the return codes */
    for (i = 1; i < n_contexts; i++)
    {
	if (ret_code[i] != 0)
	{
	    nexus_fatal("Context creation failed: node=%d, return code=%d\n",
			i, ret_code[i]);
	}
    }
    nexus_free(ret_code);
} /* create_contexts() */


/*
 * create_ring_links()
 */
static void create_ring_links(nexus_global_pointer_t *context_gps,
			      int n_contexts,
			      nexus_global_pointer_t *ring_list,
			      ring_link_t *ring_link)
{
    nexus_global_pointer_t ring_link_gp;
    nexus_buffer_t buffer;
    int i;
    int gp_size, gp_n_elements;

    monitor.count = n_contexts;
    
    nexus_global_pointer(&(ring_list[0]), (void *) ring_link);
    
    for (i = 1; i <= n_contexts; i++)
    {
	nexus_global_pointer(&ring_link_gp, (void *) &(ring_list[i]));
	nexus_init_remote_service_request(&buffer, &(context_gps[i-1]),
					  "allocate_ring_link_handler",
					  ALLOCATE_RING_LINK_HANDLER_HASH);
	gp_size = nexus_sizeof_global_pointer(&buffer,
					      &ring_link_gp, 1,
					      &gp_n_elements);
	nexus_set_buffer_size(&buffer, gp_size, gp_n_elements);
	nexus_put_global_pointer(&buffer, &ring_link_gp, 1);
	nexus_send_remote_service_request(&buffer);
	nexus_destroy_global_pointer(&ring_link_gp);
    }

    /* wait until we have all gp's of all the nodes */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex));
    }
    nexus_mutex_unlock(&(monitor.mutex));
} /* create_ring_links() */


/*
 * allocate_ring_link_handler()
 */
static void allocate_ring_link_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t reply_buffer;
    nexus_global_pointer_t reply_gp;
    nexus_global_pointer_t ring_link_gp;
    ring_link_t *ring_link;
    int gp_size, gp_n_elements;
    
    nexus_get_global_pointer(buffer, &reply_gp, 1);
    
    ring_link = nexus_malloc(sizeof(ring_link_t));
    ring_link->master_node = NEXUS_FALSE;
    nexus_global_pointer(&ring_link_gp, (void *) ring_link);
    
    nexus_init_remote_service_request(&reply_buffer, &reply_gp,
				      "register_ring_link_handler",
				      REGISTER_RING_LINK_HANDLER_HASH);
    gp_size = nexus_sizeof_global_pointer(&reply_buffer,
					  &ring_link_gp, 1,
					  &gp_n_elements);
    nexus_set_buffer_size(&reply_buffer, gp_size, gp_n_elements);
    nexus_put_global_pointer(&reply_buffer, &ring_link_gp, 1);
    nexus_send_remote_service_request(&reply_buffer);
    
    nexus_destroy_global_pointer(&ring_link_gp);
    
} /* allocate_ring_link_handler() */


/*
 * destroy_contexts_and_nodes()
 */
static void destroy_contexts_and_nodes(nexus_global_pointer_t *context_gps,
				       nexus_node_t *nodes,
				       int n_nodes)
{
    int i;
    nexus_buffer_t buffer;
    int gp_size, gp_n_elements;

    /* Tell the created contexts to shutdown */
    monitor.count = n_nodes-1;
    for (i = 0; i < n_nodes-1; i++)
    {
	nexus_init_remote_service_request(&buffer, &(context_gps[i]),
					  "shutdown_handler",
					  SHUTDOWN_HANDLER_HASH);
	gp_size = nexus_sizeof_global_pointer(&buffer,
					      &monitor_gp, 1,
					      &gp_n_elements);
	nexus_set_buffer_size(&buffer, gp_size, gp_n_elements);
	nexus_put_global_pointer(&buffer, &monitor_gp, 1);
	nexus_send_remote_service_request(&buffer);
    }

    /* Wait until all of the contexts reply */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex) );
    }
    monitor.count = 0;
    nexus_mutex_unlock(&(monitor.mutex));

    /* Tell all the nodes to shutdown */
    for (i = 1; i < n_nodes; i++)
    {
	if (nodes[i].return_code == NEXUS_NODE_NEW)        /* only kill once */
	{
	    nexus_init_remote_service_request(&buffer, &(nodes[i].gp),
					      "shutdown_handler",
					      SHUTDOWN_HANDLER_HASH);
	    gp_size = nexus_sizeof_global_pointer(&buffer,
						  &monitor_gp, 1,
						  &gp_n_elements);
	    nexus_set_buffer_size(&buffer, gp_size, gp_n_elements);
	    nexus_put_global_pointer(&buffer, &monitor_gp, 1);
	    nexus_send_remote_service_request(&buffer);
	    nexus_mutex_lock(&(monitor.mutex));
	    monitor.count++;
	    nexus_mutex_unlock(&(monitor.mutex));
	}
    }

    /* Wait until all of the nodes reply */
    nexus_mutex_lock(&(monitor.mutex));
    while (monitor.count > 0)
    {
	nexus_cond_wait(&(monitor.cond), &(monitor.mutex) );
    }
    nexus_mutex_unlock(&(monitor.mutex));

} /* destroy_contexts_and_nodes() */


/*
 * shutdown_handler()
 */
static void shutdown_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t reply_buffer;
    nexus_global_pointer_t reply_monitor_gp;

    nexus_get_global_pointer(buffer, &reply_monitor_gp, 1);
    
    nexus_init_remote_service_request(&reply_buffer, &reply_monitor_gp,
				      "signal_monitor_handler",
				      SIGNAL_MONITOR_HANDLER_HASH);
    nexus_send_remote_service_request(&reply_buffer);
    
    nexus_destroy_global_pointer(&reply_monitor_gp);
    nexus_destroy_current_context(NEXUS_TRUE);
} /* kill_node_handler() */

#endif  /* NEXUS_ATTACH_VERSION  */




