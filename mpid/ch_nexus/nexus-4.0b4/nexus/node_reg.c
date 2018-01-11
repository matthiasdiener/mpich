/*
 * node_reg.c
 *
 * Node registry:
 *	Maps node names to global pointers.
 *	Controls creation, acquisition, and destruction of nodes.
 *	Assigns unique integer node ids on demand.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/node_reg.c,v 1.3 1996/10/07 04:40:05 tuecke Exp $";

#include "internal.h"

static nexus_global_pointer_t	node_registry_gp;

#define MAX_SESSION_STRING_LENGTH 256
static char *			my_session_string;
static int			my_session_string_length;

/*
 * Forward typedefs
 */
typedef struct _nr_session_t	nr_session_t;
typedef enum _nr_state_t	nr_state_t;
typedef enum _nr_msg_type_t	nr_msg_type_t;
typedef struct _nr_msg_t	nr_msg_t;
typedef struct _nr_node_t	nr_node_t;


/********************************************************************
 *
 *	Session management
 *
 ********************************************************************/

struct _nr_session_t
{
    char *		name;
    int			name_length;
    int			next_node_id;
    nr_node_t **	node_table;
    nexus_mutex_t	mutex;
};


/********************************************************************
 *
 *	Node registry handler
 *
 ********************************************************************/

#define NEXUS_NODE_REGISTRY_HANDLER_NAME "nexus_node_registry_handler"
#define NEXUS_NODE_REGISTRY_HANDLER_HASH 851
static void nexus_node_registry_handler(void *address,
					nexus_buffer_t *buffer);

static nexus_handler_t node_id_handlers[] =
{
  {NEXUS_NODE_REGISTRY_HANDLER_NAME,
       NEXUS_NODE_REGISTRY_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) nexus_node_registry_handler},
  {(char *) NULL,
       0,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};

/*
 * nr_state_t
 *
 * State in the node registry state machine
 */
enum _nr_state_t
{
    NR_STATE_START,
    NR_STATE_CREATING,
    NR_STATE_NORMAL,
    NR_STATE_ACQUIRING,
    NR_STATE_DYING
};

/*
 * nr_msg_type_t
 *
 * Messages that the node registry handler may deal with
 */
enum _nr_msg_type_t
{
    NR_MSG_ACQUIRE,
    NR_MSG_ACQUIRE_SUCCESS,
    NR_MSG_ACQUIRE_FAILURE,
    NR_MSG_CREATE_SUCCESS,
    NR_MSG_CREATE_FAILURE,
    NR_MSG_DYING,
    NR_MSG_DEAD
};


/*
 * nr_msg_t
 *
 * A message that arrives at nexus_node_registry_handler() will be
 * first placed into this structure, which can then be used until
 * it is no longer need.
 */
struct _nr_msg_t
{
    nr_msg_type_t		type;
    int				node_name_length;
    char			node_name[NEXUS_MAX_NODE_NAME_LENGTH];
    int				node_number;
    nexus_stashed_buffer_t	stashed_buffer;
    nr_session_t *		session;
    struct _nr_msg_t *		next;
};

static nr_msg_t *	nr_msg_free_list;
static nexus_mutex_t	nr_msg_free_list_mutex;


/*
 * alloc_nr_msg()
 */
static nr_msg_t *alloc_nr_msg(nr_session_t *session)
{
    nr_msg_t *msg;
    nexus_mutex_lock(&nr_msg_free_list_mutex);
    if (nr_msg_free_list)
    {
	msg = nr_msg_free_list;
	nr_msg_free_list = nr_msg_free_list->next;
	nexus_mutex_unlock(&nr_msg_free_list_mutex);
    }
    else
    {
	nexus_mutex_unlock(&nr_msg_free_list_mutex);
	NexusMalloc(alloc_nr_msg(),
		    msg,
		    nr_msg_t *,
		    sizeof(nr_msg_t));
    }
    msg->session = session;
    msg->next = (nr_msg_t *) NULL;
    return(msg);
} /* alloc_nr_msg() */


/*
 * free_nr_msg()
 */
static void free_nr_msg(nr_msg_t *msg)
{
    nexus_free_stashed_buffer(msg->stashed_buffer);
    nexus_mutex_lock(&nr_msg_free_list_mutex);
    msg->next = nr_msg_free_list;
    nr_msg_free_list = msg;
    nexus_mutex_unlock(&nr_msg_free_list_mutex);
} /* free_nr_msg() */


/*
 * nr_node_t
 *
 * Structure containing the state of a particular node
 */
struct _nr_node_t
{
    nr_state_t			state;
    int				name_length;
    char *			name;
    int				count;
    nexus_global_pointer_t	gp;
    nr_msg_t *			msg;
    struct _nr_node_t *		next;
    struct _nr_node_t *		prev;
};

static nr_node_t *	nr_node_free_list;
static nexus_mutex_t	nr_node_free_list_mutex;


/*
 * alloc_nr_node()
 */
static nr_node_t *alloc_nr_node(int name_length, char *name)
{
    nr_node_t *node;
    nexus_mutex_lock(&nr_node_free_list_mutex);
    if (nr_node_free_list)
    {
	node = nr_node_free_list;
	nr_node_free_list = nr_node_free_list->next;
	nexus_mutex_unlock(&nr_node_free_list_mutex);
    }
    else
    {
	nexus_mutex_unlock(&nr_node_free_list_mutex);
	NexusMalloc(Func,
		    node,
		    nr_node_t *,
		    sizeof(nr_node_t));
    }
    node->state = NR_STATE_START;
    node->name_length = name_length;
    node->name = _nx_copy_string(name);
    node->count = -1;
    nexus_null_global_pointer(&(node->gp));
    node->msg = (nr_msg_t *) NULL;
    node->next = (nr_msg_t *) NULL;
    node->prev = (nr_msg_t *) NULL;
    return(node);
} /* alloc_nr_node() */


/*
 * free_nr_node()
 */
static void free_nr_node(nr_node_t *node)
{
    if (node->name)
    {
	NexusFree(node->name);
    }
    if (!nexus_is_null_global_pointer(&(node->gp)))
    {
	nexus_destroy_global_pointer(&(node->gp));
    }
    if (node->msg)
    {
	free_nr_msg(node->msg);
    }
    nexus_mutex_lock(&nr_node_free_list_mutex);
    node->next = nr_node_free_list;
    nr_node_free_list = node;
    nexus_mutex_unlock(&nr_node_free_list_mutex);
} /* free_nr_node() */


/*
 * nexus_node_registry_handler()
 */
static void nexus_node_registry_handler(void *address,
					nexus_buffer_t *buffer)
{
    nr_session_t *session = (nr_session_t *) address;
    nr_msg_t *msg;
    nr_node_t *node;
    nr_msg_t *msg_q_head;
    nr_msg_t *msg_q_tail;
    int node_name_hash;
    int i;

    /*
     * Read the message header
     */
    msg = alloc_nr_msg(session);
    nexus_get_int(buffer, &i, 1);
    msg->type = (nr_msg_type_t) i;
    nexus_get_int(buffer, &(msg->node_name_length), 1);
    NexusAssert2((msg->node_name_length + 1 < NEXUS_MAX_NODE_NAME_LENGTH),
		 ("nexus_node_registry_handler(): Node name too long\n"));
    nexus_get_char(buffer, msg->node_name, 1);
    msg->node_name[msg->node_name_length] = '\0';
    nexus_get_int(buffer, &(msg->node_number), 1);
    nexus_stash_buffer(buffer, &(msg->stashed_buffer));

    /*
     * Lock down this session
     */
    nexus_mutex_lock(&(session->mutex));

    /*
     * Lookup the node name
     */
    node_name_hash = hash_node_name(msg->node_name);
    for (node = session->node_table[node_name_hash];
	 node && (strcmp(node->name, msg->node_name) != 0);
	 node->next)
	;

    if (!node)
    {
	/*
	 * This node doesn't exist in the session node_table.  So create one.
	 */
	node = alloc_nr_node(msg->node_name_length, msg->node_name);

	/* Add this node to the session node_table */
	if (session->node_table[node_name_hash])
	{
	    node->next = session->node_table[node_name_hash];
	    node->next->prev = node;
	}
	session->node_table[node_name_hash] = node;
    }

    /*
     * Loop while there are messages for this node
     */
    msg_q_head = msg_q_tail = msg;
    while (msg_q_head)
    {
	/* Take the first message from the queue */
	msg = msg_q_head;
	msg_q_head = msg_q_head->next;
	if (!msg_q_head)
	{
	    msg_q_tail = (nr_msg_t *) NULL;
	}

	/* Apply this message to the node state machine */
	switch (node->state)
	{
	case NR_STATE_START:
	    break;
	case NR_STATE_CREATING:
	    break;
	case NR_STATE_NORMAL:
	    break;
	case NR_STATE_ACQUIRING:
	    break;
	case NR_STATE_DYING:
	    break;
	default:
	    nexus_fatal("nexus_node_registry_handler(): Internal error: Invalid state\n");
	    break;
	}
    }

    /*
     * Free the node if it is no longer needed
     */
    if (node->state == NR_STATE_START)
    {
	/* Remove the node from the session node_table */
	if (session->node_table[node_name_hash] == node)
	{
	    session->node_table[node_name_hash] = node->next;
	}
	if (node->next)
	{
	    node->next->prev = node->prev;
	}
	if (node->prev)
	{
	    node->prev->next = node->next;
	}

	/* Free the node */
	free_nr_node(node);
    }
    
    /*
     * Unlock this session
     */
    nexus_mutex_lock(&(session->mutex));

} /* nexus_node_registry_handler() */


/********************************************************************
 *
 *	get_next_node_id()
 *
 ********************************************************************/

typedef struct _wait_for_node_id_t
{
    int			node_id;
    nexus_bool_t	done;
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
} wait_for_node_id_t;

static int			next_node_id;
static nexus_mutex_t		next_node_id_mutex;

static int get_next_node_id(char *session, int n_nodes);

#define NEXUS_GET_NEXT_NODE_ID_HANDLER_NAME "nexus_get_next_node_id_handler"
#define NEXUS_GET_NEXT_NODE_ID_HANDLER_HASH 886
static void nexus_get_next_node_id_handler(void *address,
					   nexus_buffer_t *buffer);

#define NEXUS_GET_NEXT_NODE_ID_REPLY_HANDLER_NAME \
		"nexus_get_next_node_id_reply_handler"
#define NEXUS_GET_NEXT_NODE_ID_REPLY_HANDLER_HASH 516
static void nexus_get_next_node_id_reply_handler(void *address,
						 nexus_buffer_t *buffer);

static nexus_handler_t node_id_handlers[] =
{
  {NEXUS_GET_NEXT_NODE_ID_HANDLER_NAME,
       NEXUS_GET_NEXT_NODE_ID_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) nexus_get_next_node_id_handler},
  {NEXUS_GET_NEXT_NODE_ID_REPLY_HANDLER_NAME,
       NEXUS_GET_NEXT_NODE_ID_REPLY_HANDLER_HASH,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) nexus_get_next_node_id_reply_handler},
  {(char *) NULL,
       0,
       NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};


/*
 * get_next_node_id()
 *
 * Return the next available node id,
 * and increment next_node_id by 'n_nodes'.
 */
static int get_next_node_id(nr_session_t *session, int n_nodes)
{
    int id;
    nexus_mutex_lock(&(session->mutex));
    id = session->next_node_id;
    session->next_node_id += n_nodes;
    nexus_mutex_unlock(&(session->mutex));
    return(id);
} /* get_next_node_id() */


/*
 * nexus_get_next_node_id()
 *
 * Send a message to the node registry to get the next
 * available node id.
 */
int nexus_get_next_node_id(int n_nodes)
{
    int node_id;
    wait_for_node_id_t wait;
    nexus_global_pointer_t wait_gp;
    nexus_buffer_t buffer;
    int size, n_elements, gp_n_elements;

    /* Initialize wait structure */
    wait.done = NEXUS_FALSE;
    nexus_mutex_init(&(wait.mutex), (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(wait.cond), (nexus_condattr_t *) NULL);

    /* Send message to master node */
    nexus_global_pointer(&wait_gp, (void *) &wait);
    nexus_init_remote_service_request(&buffer,
				      &node_registry_gp,
				      "nexus_get_next_node_id_handler",
				      NEXUS_GET_NEXT_NODE_ID_HANDLER_HASH);
    size = nexus_sizeof_int(&buffer, 1);
    n_elements = 1;
    size += nexus_sizeof_global_pointer(&buffer, &wait_gp, 1,
					&gp_n_elements);
    n_elements += gp_n_elements;
    nexus_set_buffer_size(&buffer, size, n_elements);
    nexus_put_int(&buffer, &n_nodes, 1);
    nexus_put_global_pointer(&buffer, &wait_gp, 1);
    nexus_send_remote_service_request(&buffer);

    /* Wait for reply */
    nexus_mutex_lock(&(wait.mutex));
    while (!wait.done)
    {
	nexus_cond_wait(&(wait.cond), &(wait.mutex));
    }
    nexus_mutex_unlock(&(wait.mutex));

    node_id = wait.node_id;
    
    nexus_destroy_global_pointer(&wait_gp);
    nexus_mutex_destroy(&(wait.mutex));
    nexus_cond_destroy(&(wait.cond));

    return(node_id);
    
} /* nexus_get_next_node_id() */


/*
 * nexus_get_next_node_id_handler()
 */
static void nexus_get_next_node_id_handler(void *address,
					   nexus_buffer_t *buffer)
{
    nr_session_t *session = (nr_session_t *) address;
    int node_id;
    int n_nodes;
    nexus_global_pointer_t wait_gp;
    nexus_buffer_t reply_buffer;

    nexus_get_int(buffer, &n_nodes, 1);
    nexus_get_global_pointer(buffer, &wait_gp, 1);

    node_id = get_next_node_id(session, n_nodes);
    
    /* Send reply message */
    nexus_init_remote_service_request(&reply_buffer,
				      &wait_gp,
				      "nexus_get_next_node_id_reply_handler",
				    NEXUS_GET_NEXT_NODE_ID_REPLY_HANDLER_HASH);
    nexus_set_buffer_size(&reply_buffer,
			  nexus_sizeof_int(&reply_buffer, 1),
			  1 );
    nexus_put_int(&reply_buffer, &node_id, 1);
    nexus_send_remote_service_request(&reply_buffer);

    nexus_destroy_global_pointer(&wait_gp);
    
} /* nexus_get_next_node_id_handler() */


/*
 * nexus_get_next_node_id_reply_handler()
 */
static void nexus_get_next_node_id_reply_handler(void *address,
						 nexus_buffer_t *buffer)
{
    wait_for_node_id_t *wait = (wait_for_node_id_t *) address;
    int node_id;
    
    nexus_get_int(buffer, &node_id, 1);

    /* Signal completion of reply */
    nexus_mutex_lock(&(wait->mutex));
    wait->node_id = node_id;
    wait->done = NEXUS_TRUE;
    nexus_cond_signal(&(wait->cond));
    nexus_mutex_unlock(&(wait->mutex));
    
} /* nexus_get_next_node_id_reply_handler() */


/********************************************************************
 *
 *	Miscellaneous stuff
 *
 ********************************************************************/

/*
 * _nx_node_usage_message()
 */
void _nx_node_usage_message(void)
{
} /* _nx_node_usage_message() */


/*
 * _nx_node_new_process_params()
 */
int _nx_node_new_process_params(char *buf, int size)
{
    int len;
    
    if (my_session_string_length + 16 > size)
    {
	nexus_fatal("_nx_node_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }
    sprintf(buf, "-nx_session %s ", my_session_string);

    len = strlen(buf);
    return (len);
} /* _nx_node_new_process_params() */


/*
 * _nx_node_init()
 */
void _nx_node_init(int *argc, char ***argv)
{
    int arg_num;

    /*
     * Get any of this module's arguments
     */
    my_session_string = (char *) NULL;
    my_session_string_length = 0;
    if ((arg_num = nexus_find_argument(argc, argv, "nx_session", 2)) >= 0)
    {
	my_session_string = _nx_copy_string((*argv)[arg_num + 1]);
	my_session_string_length = strlen(my_session_string);
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }

} /* _nx_node_init() */


/*
 * _nx_node_register_handlers()
 */
void _nx_node_register_handlers(void)
{
    nexus_register_handlers(node_id_handlers);
    nexus_register_handlers(node_registry_handlers);
} /* _nx_node_register_handlers() */
