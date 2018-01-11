#include <nexus.h>

/* 
 * This program follows in the tradition of ppmulti.  It assumes that
 * node 1 is connected via some protocol other than TCP and that node 2
 * is connected solely through TCP.
 */

#define NUM_MESSAGES 100
#define CALLBACK NULL

#define SHUTDOWN_NODE_HANDLER_HASH        196
#define SHUTDOWN_NODE_REPLY_HANDLER_HASH  847

#define NODE1_STARTUP_HANDLER_HASH        140
#define NODE2_STARTUP_HANDLER_HASH        141
#define NODE1_START_MESSAGES_HANDLER_HASH 862
#define NODE2_START_MESSAGES_HANDLER_HASH 863
#define NODE1_MESSAGE_HANDLER_HASH         94
#define NODE2_MESSAGE_HANDLER_HASH         95
#define NODE2_NOTIFY_HANDLER_HASH          19
#define NODE1_NOTIFY_HANDLER_HASH          18

static nexus_mutex_t message_mutex;
static nexus_cond_t  message_cond;

static int node1_count = 0;
static int node2_count = 0;

static nexus_bool_t node1_sending = NEXUS_TRUE;
static nexus_bool_t node2_sending = NEXUS_TRUE;

static nexus_global_pointer_t node0_gp, node1_gp;

static void shutdown_nodes(nexus_node_t *nodes, int n_nodes);

static void shutdown_node_handler(void *address, nexus_buffer_t *buffer);
static void shutdown_node_reply_handler(void *address, nexus_buffer_t *buffer);

static void node1_startup_handler(void *address, nexus_buffer_t *buffer);
static void node2_startup_handler(void *address, nexus_buffer_t *buffer);
static void node1_start_messages_handler(void *address, nexus_buffer_t *buffer);
static void node2_start_messages_handler(void *address, nexus_buffer_t *buffer);
static void node1_message_handler(void *address, nexus_buffer_t *buffer);
static void node2_message_handler(void *address, nexus_buffer_t *buffer);
static void node2_notify_handler(void *address, nexus_buffer_t *buffer);
static void node1_notify_handler(void *address, nexus_buffer_t *buffer);

static nexus_handler_t system_handlers[] = 
{ \
    {"shutdown_node_handler",
	SHUTDOWN_NODE_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) shutdown_node_handler},
    {"shutdown_node_reply_handler",
	SHUTDOWN_NODE_REPLY_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) shutdown_node_reply_handler},
    {(char *) NULL,
	0,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) NULL},
};

static nexus_handler_t polling_handlers[] =
{ \
    {"node1_startup_handler",
	NODE1_STARTUP_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node1_startup_handler },
    {"node2_startup_handler",
	NODE2_STARTUP_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node2_startup_handler },
    {"node1_start_messages_handler",
	NODE1_START_MESSAGES_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node1_start_messages_handler },
    {"node2_start_messages_handler",
	NODE2_START_MESSAGES_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node2_start_messages_handler },
    {"node1_message_handler",
	NODE1_MESSAGE_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node1_message_handler },
    {"node2_message_handler",
	NODE2_MESSAGE_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node2_message_handler },
    {"node2_notify_handler",
	NODE2_NOTIFY_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node2_notify_handler },
    {"node1_notify_handler",
	NODE1_NOTIFY_HANDLER_HASH,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) node1_notify_handler },
    {(char *) NULL,
	0,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) NULL},
};

static nexus_handler_t adaptive_handlers[] =
{ \
    {(char *) NULL,
	0,
	NEXUS_HANDLER_TYPE_NON_THREADED,
	(nexus_handler_func_t) NULL},
};

int main(int argc, char **argv)
{
    nexus_buffer_t buffer;
    nexus_node_t *nodes;
    int n_nodes;
    int buf_size;
    int n_gp_elements;
    int tmp;

    nexus_init(&argc, &argv, "NEXUS_ARGS", "nx",
	       NULL, NULL, NULL, NULL,
	       &nodes, &n_nodes);
    nexus_start();

    nexus_printf("sending GP's to other nodes\n");
    nexus_init_remote_service_request(&buffer, &nodes[1].gp,
				      "node1_startup_handler",
				      NODE1_STARTUP_HANDLER_HASH);
    buf_size = nexus_sizeof_global_pointer(&buffer, &nodes[0].gp, 1, &n_gp_elements);
    nexus_set_buffer_size(&buffer, buf_size, n_gp_elements);
    nexus_put_global_pointer(&buffer, &nodes[0].gp, 1);
    nexus_send_remote_service_request(&buffer);

    nexus_init_remote_service_request(&buffer, &nodes[2].gp,
				      "node2_startup_handler",
				      NODE2_STARTUP_HANDLER_HASH);
    buf_size = nexus_sizeof_global_pointer(&buffer, &nodes[0].gp, 1, &n_gp_elements);
    buf_size += nexus_sizeof_global_pointer(&buffer, &nodes[1].gp, 1, &tmp);
    n_gp_elements += tmp;
    nexus_set_buffer_size(&buffer, buf_size, n_gp_elements);
    nexus_put_global_pointer(&buffer, &nodes[0].gp, 1);
    nexus_put_global_pointer(&buffer, &nodes[1].gp, 1);
    nexus_send_remote_service_request(&buffer);

    nexus_printf("turning off tcp polling\n");
    nexus_disable_tcp_poll();

    nexus_printf("sending message to node 2 to start messages\n");
    nexus_init_remote_service_request(&buffer, &nodes[2].gp,
				      "node2_start_messages_handler",
				      NODE2_START_MESSAGES_HANDLER_HASH);
    nexus_send_remote_service_request(&buffer);

    nexus_printf("sending message to node 1 to start messages\n");
    nexus_init_remote_service_request(&buffer, &nodes[1].gp,
				      "node2_send_messages_handler",
				      NODE1_START_MESSAGES_HANDLER_HASH);
    nexus_send_remote_service_request(&buffer);

    nexus_mutex_lock(&message_mutex);
    while (node1_sending)
    {
        nexus_cond_wait(&message_cond, &message_mutex);
    }

    nexus_printf("successfully got %d messages from node 1, and 0 messages from node 2\n", node1_count);
    nexus_mutex_unlock(&message_mutex);

    nexus_printf("turning on tcp polling\n");
    nexus_mutex_lock(&message_mutex);
    nexus_enable_tcp_poll();
    while (node2_sending)
    {
	nexus_cond_wait(&message_cond, &message_mutex);
    }
    nexus_mutex_unlock(&message_mutex);

    nexus_printf("attempting to do adaptive polling\n");
    nexus_register_skip_poll_callback(CALLBACK);

    nexus_mutex_lock(&message_mutex);
    while(NEXUS_TRUE)
    {
	nexus_cond_wait(&message_cond, &message_mutex);
    }
    nexus_mutex_unlock(&message_mutex);

    shutdown_nodes(nodes, n_nodes);
}

int NexusBoot(void)
{
    nexus_mutex_init(&message_mutex, (nexus_mutexattr_t *)NULL);
    nexus_cond_init(&message_cond, (nexus_condattr_t *)NULL);

    nexus_register_handlers(system_handlers);
    nexus_register_handlers(polling_handlers);
    nexus_register_handlers(adaptive_handlers);

    return 0;
}

static void node1_startup_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_get_global_pointer(buffer, &node0_gp, 1);
}

static void node2_startup_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_get_global_pointer(buffer, &node0_gp, 1);
    nexus_get_global_pointer(buffer, &node1_gp, 1);
}

static void node1_start_messages_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t send_buffer;

    nexus_mutex_lock(&message_mutex);
    while (node2_sending)
    {
	nexus_mutex_unlock(&message_mutex);
	nexus_init_remote_service_request(&send_buffer, &node0_gp,
					  "node1_message_handler",
					  NODE1_MESSAGE_HANDLER_HASH);
	nexus_send_remote_service_request(&send_buffer);
	nexus_mutex_lock(&message_mutex);
    }
    nexus_mutex_unlock(&message_mutex);
}

static void node2_start_messages_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t send_buffer;
    int i;

    for (i = 0; i < NUM_MESSAGES; i++)
    {
	nexus_init_remote_service_request(&send_buffer, &node0_gp,
					  "node2_message_handler",
					  NODE2_MESSAGE_HANDLER_HASH);
	nexus_send_remote_service_request(&send_buffer);
    }
    nexus_init_remote_service_request(&send_buffer, &node1_gp,
				      "node2_notify_handler",
				      NODE2_NOTIFY_HANDLER_HASH);
    nexus_send_remote_service_request(&send_buffer);
}

static void node1_message_handler (void *address, nexus_buffer_t *buffer)
{
    nexus_mutex_lock(&message_mutex);
    node1_count++;
    nexus_mutex_unlock(&message_mutex);
}

static void node2_message_handler (void *address, nexus_buffer_t *buffer)
{
    nexus_mutex_lock(&message_mutex);
    if (node1_sending)
    {
	nexus_fatal("Received TCP message with tcp_poll() disabled\n");
    }
    if (++node2_count == NUM_MESSAGES)
    {
	node2_sending = NEXUS_FALSE;
	nexus_cond_signal(&message_cond);
    }
    nexus_mutex_unlock(&message_mutex);
}

static void node2_notify_handler (void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t send_buffer;
	
    nexus_mutex_lock(&message_mutex);
    node2_sending = NEXUS_FALSE;
    nexus_mutex_unlock(&message_mutex);

    nexus_init_remote_service_request(&send_buffer, &node0_gp,
				      "node1_notify_handler",
				      NODE1_NOTIFY_HANDLER_HASH);
    nexus_send_remote_service_request(&send_buffer);
}

static void node1_notify_handler (void *address, nexus_buffer_t *buffer)
{
    nexus_mutex_lock(&message_mutex);
    node1_sending = NEXUS_FALSE;
    nexus_cond_signal(&message_cond);
    nexus_mutex_unlock(&message_mutex);
}


/****************************************************************
 *              SHUTDOWN NODES
 ****************************************************************/

typedef struct shutdown_nodes_barrier_struct
{
    
    int                 count;
    nexus_mutex_t       mutex;
    nexus_cond_t        cond;
} shutdown_nodes_barrier_t;

/* 
 * * shutdown_node_handler()
 * * 
 * */
static void shutdown_node_handler(void *address, nexus_buffer_t *buffer)
{
    nexus_buffer_t reply_buffer;
    nexus_global_pointer_t barrier_gp;

    nexus_get_global_pointer(buffer, &barrier_gp, 1);

    nexus_init_remote_service_request(&reply_buffer, &barrier_gp,
                                      "shutdown_node_reply_handler",
                                      SHUTDOWN_NODE_REPLY_HANDLER_HASH);
    nexus_printf("shutdown_node_handler(): sending shutdown node reply\n");
    if (nexus_send_remote_service_request(&reply_buffer) != 0)
    {
        nexus_printf("shutdown_node_handler(): WARNING: send failed\n");
    }

    nexus_destroy_global_pointer(&barrier_gp);

    nexus_printf("shutdown_node_handler(): destroying context\n");
    nexus_destroy_current_context(NEXUS_TRUE);
} /*  shutdown_node_handler() */


/* 
 * * shutdown_node_reply_handler()
 * * 
 * */
static void shutdown_node_reply_handler(void *address, nexus_buffer_t *buffer)
{
    shutdown_nodes_barrier_t *shutdown_nodes_barrier
        = (shutdown_nodes_barrier_t *) address;

    nexus_printf("shutdown_node_reply_handler(): entering\n");

    nexus_mutex_lock(&(shutdown_nodes_barrier->mutex));
    if (--shutdown_nodes_barrier->count == 0)
    {
        nexus_cond_signal(&(shutdown_nodes_barrier->cond));
    }
    nexus_mutex_unlock(&(shutdown_nodes_barrier->mutex));

    nexus_printf("shutdown_node_reply_handler(): exiting\n");

} /*  shutdown_node_reply_handler() */


/* 
 * * shutdown_nodes()
 * * 
 * */
static void shutdown_nodes(nexus_node_t *nodes, int n_nodes)
{
    int i;
    nexus_buffer_t buffer;
    nexus_global_pointer_t barrier_gp;
    shutdown_nodes_barrier_t shutdown_nodes_barrier;
    int gp_size, gp_n_elements;

    shutdown_nodes_barrier.count = 0;
    nexus_mutex_init(&(shutdown_nodes_barrier.mutex),
                     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(shutdown_nodes_barrier.cond),
                    (nexus_condattr_t *) NULL);
    nexus_global_pointer(&barrier_gp, &shutdown_nodes_barrier);

    for (i = 1; i < n_nodes; i++)
    {
        if (nodes[i].return_code == NEXUS_NODE_NEW)
        {
            nexus_init_remote_service_request(&buffer, &(nodes[i].gp),
                                              "shutdown_node_handler",
                                              SHUTDOWN_NODE_HANDLER_HASH);
            gp_size = nexus_sizeof_global_pointer(&buffer, &barrier_gp, 1,
                                                  &gp_n_elements);
            nexus_set_buffer_size(&buffer, gp_size, gp_n_elements);
            nexus_put_global_pointer(&buffer, (&barrier_gp), 1);
            nexus_printf("shutdown_nodes(): sending destroy context message to n
ode %d\n", i);
            if (nexus_send_remote_service_request(&buffer) != 0)
            {
                nexus_printf("shutdown_nodes(): WARNING: send failed\n");
            }
            nexus_mutex_lock(&(shutdown_nodes_barrier.mutex));
            shutdown_nodes_barrier.count++;
            nexus_mutex_unlock(&(shutdown_nodes_barrier.mutex));
        }
    }

    nexus_destroy_global_pointer(&barrier_gp);

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

} /*  shutdown_nodes() */
