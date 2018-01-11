#include "nexus.h"
#include <stdio.h>

#include "nexus_ez.h"

static void shutdown_nodes(nexus_node_t *nodes, int n_nodes);

static void shutdown_node_handler(nexus_endpoint_t *endpoint, 
			nexus_buffer_t *buffer,
			nexus_bool_t called_from_non_threaded_handler); 
#define     SHUTDOWN_NODE_HANDLER_HASH		0
static void shutdown_node_reply_handler(nexus_endpoint_t *endpoint, 
			nexus_buffer_t *buffer,
			nexus_bool_t called_from_non_threaded_handler);
#define     SHUTDOWN_NODE_REPLY_HANDLER_HASH	1

static nexus_handler_t system_handlers[] =
{ \
  { NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) shutdown_node_handler},
  { NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) shutdown_node_reply_handler},
  { NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};

static void pong_starter_handler(nexus_endpoint_t *endpoint, 
		nexus_buffer_t *buffer, nexus_bool_t called_from_non_threaded_handler); 
#define      PONG_STARTER_HANDLER_HASH 0

static nexus_handler_t pingpong_handlers[] =
{ \
    { NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) pong_starter_handler},
    { NEXUS_HANDLER_TYPE_NON_THREADED,
       (nexus_handler_func_t) NULL},
};

static nexus_endpoint_t global_endpoint;
static nexus_endpoint_t shutdown_endpoint;

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

    nexus_printf("in shutdown node handler\n");
    nexus_ez_rpc_unpack_1sided(buffer, "%p", &barrier_sp);
    nexus_buffer_init(&reply_buffer, 0, 0);
    nexus_printf("after get in shutdown node handler\n");
    nexus_send_rsr(&reply_buffer, &barrier_sp, 1, 
		   NEXUS_TRUE, called_from_non_threaded_handler);
    nexus_printf("after send rsr in shutdown node handler\n");
    nexus_startpoint_destroy(&barrier_sp);
    nexus_context_destroy(NEXUS_TRUE);
} /* shutdown_node_handler() */


/*
 * shutdown_node_reply_handler()
 */
static void shutdown_node_reply_handler(nexus_endpoint_t *endpoint, 
				nexus_buffer_t *buffer, 
				nexus_bool_t called_from_non_threaded_handler) 
{
    shutdown_nodes_barrier_t *shutdown_nodes_barrier;

    nexus_printf("in shutdown node reply handler\n");
    shutdown_nodes_barrier=
	 (shutdown_nodes_barrier_t *)nexus_endpoint_get_user_pointer(endpoint);
    nexus_printf("after endpoint get\n");
    nexus_mutex_lock(&(shutdown_nodes_barrier->mutex));
    if (--shutdown_nodes_barrier->count == 0)
    {
	nexus_cond_signal(&(shutdown_nodes_barrier->cond));
    }
    nexus_mutex_unlock(&(shutdown_nodes_barrier->mutex));
    nexus_printf("exiting shutdown node reply handler\n");
} /* shutdown_node_reply_handler() */


/*
 * shutdown_nodes()
 */
static void shutdown_nodes(nexus_node_t *nodes, int n_nodes)
{
    int i;
    nexus_startpoint_t barrier_sp;
    shutdown_nodes_barrier_t shutdown_nodes_barrier;

    nexus_printf("in shutdown stuff...\n");

    shutdown_nodes_barrier.count = 0;
    nexus_mutex_init(&(shutdown_nodes_barrier.mutex),
		     (nexus_mutexattr_t *) NULL);
    nexus_cond_init(&(shutdown_nodes_barrier.cond),
		    (nexus_condattr_t *) NULL);
    nexus_endpoint_set_user_pointer(&shutdown_endpoint, 
				(void *)(&shutdown_nodes_barrier));
    
    for (i = 1; i < n_nodes; i++)
    {
	if (nodes[i].return_code == NEXUS_NODE_NEW)
	{
	    nexus_startpoint_bind(&barrier_sp, &shutdown_endpoint);
	    nexus_ez_rpc_1sided(&(nodes[i].startpoint),SHUTDOWN_NODE_HANDLER_HASH, 
				NEXUS_FALSE, "%p", &barrier_sp);
	    nexus_mutex_lock(&(shutdown_nodes_barrier.mutex));
	    shutdown_nodes_barrier.count++;
	    nexus_mutex_unlock(&(shutdown_nodes_barrier.mutex));
	}
    }

    nexus_mutex_lock(&(shutdown_nodes_barrier.mutex));
    while (shutdown_nodes_barrier.count > 0)
    {
	nexus_cond_wait(&(shutdown_nodes_barrier.cond),
			&(shutdown_nodes_barrier.mutex) );
    }
    nexus_mutex_unlock(&(shutdown_nodes_barrier.mutex));
    
    nexus_mutex_destroy(&(shutdown_nodes_barrier.mutex));
    nexus_cond_destroy(&(shutdown_nodes_barrier.cond));
    nexus_printf("shutdown-nodes(): done\n");    

} /* shutdown_nodes() */


/*
 * NexusAcquiredAsNode()
 */
int NexusAcquiredAsNode(nexus_startpoint_t *startpoint)
{
    int rc;
  
    nexus_printf("Entering NexusAcquiredAsNode\n");
    rc = NexusBoot(startpoint);
    nexus_printf("Exiting NexusAcquiredAsNode\n");
    return (rc);
}  /* NexusAcquiredAsNode() */

/*
 * NexusBoot()
 */
int NexusBoot(nexus_startpoint_t *startpoint)
{
    nexus_endpointattr_t epattr, shutdown_attr;
    int rc;

    rc= nexus_endpointattr_init(&epattr);
    rc= nexus_endpointattr_set_handler_table(&epattr, pingpong_handlers, 1);
    rc= nexus_endpoint_init(&global_endpoint, &epattr);
    nexus_endpointattr_init(&shutdown_attr);
    nexus_endpointattr_set_handler_table(&shutdown_attr, system_handlers, 2);
    nexus_endpoint_init(&shutdown_endpoint, &shutdown_attr);
    rc= nexus_startpoint_bind(startpoint, &shutdown_endpoint);
    rc= nexus_endpointattr_destroy(&epattr);
    rc= nexus_endpointattr_destroy(&shutdown_attr);
    return (0);
} /* NexusBoot() */

/*****************************************************************
 *		MAIN
 *****************************************************************/

/*
 * main()
 */
int main(int argc, char **argv)
{
    nexus_node_t *nodes;
    int n_nodes;
    int i,j,rc;
    nexus_startpoint_t sp;

    short hi= -67;
    float l[500], check_array[500];
    unsigned long george= 98765;
    char *str;
    nexus_byte_t byte[2];
    char jimmyj[2];
    nexus_startpoint_t send_sp, recieve_sp, copy_send_sp;
    nexus_ez_rpchandle_t rpchandle;

    nexus_init(&argc, &argv,
	       "NEXUS_ARGS",
               "nx",
	       NULL, NULL, NULL,
	       NULL,
	       &nodes,
	       &n_nodes);
    nexus_start();

    /* See if we have enough nodes */
    /*
    if (n_nodes < 2)
    {
	nexus_printf("Error: Must run with 2 or more nodes.\n");
	goto shutdown;
    }
    */
    
    /* Print out the nodes array */
    for (i = 0; i < n_nodes; i++)
    {
	nexus_printf("nodes[%d]: %s#%d, rc=%d\n",
		     i, nodes[i].name, nodes[i].number, nodes[i].return_code);
    }

    if (argc < 2)
    {
	i = 42;
    }
    else
    {
        i = atoi(argv[1]);
    }

   byte[0]= 'A';
   byte[1]= 'B';
   jimmyj[0]= '0';
   jimmyj[1]= '1';
   for (j=0; j<500; j++)
      l[j]= j;
   nexus_startpoint_bind(&(nodes[0].startpoint), &shutdown_endpoint);
   nexus_startpoint_bind(&sp, &global_endpoint);
   nexus_startpoint_bind(&send_sp, &global_endpoint);
   nexus_startpoint_copy(&copy_send_sp, &send_sp);

   rc= nexus_ez_asynch(&sp,0,&rpchandle,NEXUS_TRUE, "%i%lu%hi%500f%vb%vc%p",
		&i,&george,&hi,l,2,byte,2,jimmyj, &send_sp,
		"%i%i%500f%p%Vc",&j,&i,check_array, &recieve_sp, NULL, &str);
   nexus_printf("After rpc\n");
   nexus_ez_rpc_wait(&rpchandle);
   nexus_printf("j= %i   i= %i \n",j,i);
   for (i=0; i<500; i++)
      if (l[i]!= check_array[i]) 
	 nexus_printf("array error!\n");
   if (nexus_startpoint_equal(&copy_send_sp, &recieve_sp))
     printf("good sign\n");
   else
     printf("startpoints= SATAN'S TOOL\n");
   printf("str= ");
   for (i=0; i<strlen(str); i++)
      printf("%c",*(str+i));
   printf("\n");
   nexus_ez_destroy_rpchandle(&rpchandle);

    /* Shutdown the other nodes */
    if (n_nodes > 1)
    {
        printf("calling shutdown...\n");
	shutdown_nodes(nodes, n_nodes);
    }
    
    /* Free the node list */
    for(i = 0; i < n_nodes; i++)
    {
	nexus_free(nodes[i].name);
	nexus_startpoint_destroy(&(nodes[i].startpoint));
    }
    nexus_printf("before nexus_free\n");
    nexus_free(nodes);
    nexus_printf("after nexus_free\n");
    /* Terminate this master node */
    nexus_context_destroy(NEXUS_FALSE);

    nexus_printf("main(): ERROR: We should never get here.\n");
    nexus_exit(0, NEXUS_TRUE);
    return(0);
} /* main() */

/*
 * pong_starter_handler()
 */
static void pong_starter_handler(nexus_endpoint_t *endpoint, nexus_buffer_t 
*buffer,nexus_bool_t called_from_non_threaded_handler)
{
    int i=0, y, count;
    nexus_startpoint_t reply_sp, check_sp;
    long unsigned george;
    short hi;
    float test_array[500];
    char *jimmyj;
    nexus_byte_t b[2];
    char *str= "Bus stop";

    nexus_printf("Entering handler: pong_starter\n");

    nexus_ez_rpc_unpack(buffer, &reply_sp, "%i%lu%hi%500f%vb%Vc%p", &i, 
		&george, &hi, test_array, &count, b, &count, &jimmyj, &check_sp);

    nexus_printf("i= %i\n",i);
    nexus_printf("unsigned long= %lu\n",george);
    nexus_printf("short int= %hi\n",hi);
    nexus_printf("byte= %c %c\n",b[0], b[1]);
    nexus_printf("char array= %c %c\n",jimmyj[0], jimmyj[1]);
/*    nexus_printf("str= %c%c%c\n",*str, *(str+1), *(str+2));*/
    y= i+10;

    nexus_printf("in handler: after unpack\n");

  nexus_ez_rpc_1sided(&reply_sp, 0, NEXUS_TRUE, "%i%i%500f%p%vc", &i, &y, 
		test_array, &check_sp, strlen(str), str); 
} /* pong_starter_handler() */


/*
 * NexusExit()
 */
int NexusExit(void)
{
    return 0;
} /* NexusExit() */
