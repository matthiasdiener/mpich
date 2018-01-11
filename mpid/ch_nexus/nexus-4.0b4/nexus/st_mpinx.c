/*
 * Nexus
 * Authors:     Steven Tuecke
 *              Argonne National Laboratory
 *
 * st_mpinx.c	- Node startup using Hubertus Franke's threaded MPI/Nexus
 *			on the IBM SP2
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_mpinx.c,v 1.11 1996/10/07 04:40:18 tuecke Exp $";

#include "internal.h"
#include "mpi.h"
#include "mpirsr.h"

#ifndef NEXUS_MPINX_NODE_NAME
#define NEXUS_MPINX_NODE_NAME "sp2_mpinx"
#endif

#define INITIAL_MSG_TAG   84

static int my_node;
static int n_nodes;

static nexus_bool_t	st_mpinx_preinit(nexus_startup_node_t **nl_head,
				 nexus_startup_node_t **nl_tail);
static char *	st_mpinx_get_master_node_name(void);
static int	st_mpinx_start_node(nexus_startup_node_t *node,
				    nexus_startpiont_t *reply_sp,
				    int first_checkin_number);
#ifndef HAVE_IBMDS
static nexus_bool_t	st_mpinx_start_context(char *executable_path,
				       nexus_startpoint_t *reply_sp,
				       int checkin_number,
				       int *return_code,
				       nexus_context_t **new_local_context);
#endif /* !HAVE_IBMDS */

static nexus_startup_funcs_t st_mpinx_funcs =
{
    st_mpinx_preinit,
    st_mpinx_get_master_node_name,
    NULL /* st_mpinx_init */,
    NULL /* st_mpinx_shutdown */,
    NULL /* st_mpinx_abort */,
    st_mpinx_start_node,
#ifdef HAVE_IBMDS    
    NULL /* st_mpinx_start_context */,
#else    
    st_mpinx_start_context,
#endif    
};


/*
 * _nx_st_mpinx_info()
 *
 * Return the function table for this module.
 */
void *_nx_st_mpinx_info(void)
{
    return ((void *) (&st_mpinx_funcs));
}


/*
 * st_mpinx_preinit()
 */
static nexus_bool_t st_mpinx_preinit(nexus_startup_node_t **nl_head,
				     nexus_startup_node_t **nl_tail)
{
    int argc;
    char *argv[1];

    *nl_head = *nl_tail = (nexus_startup_node_t *) NULL;
    
    /*
     * TODO: Need a way to sense if MPINX is being used
     */
    argc = 0;
    argv[0] = (char *) NULL;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_node);
    MPI_Comm_size(MPI_COMM_WORLD, &n_nodes);

    if (n_nodes == 1)
	return;

    if (my_node == 0)
    {
	/*
	 * This is the master node.
	 * Setup a nexus_startup_node_t structure for the rest
	 * of the MPINX nodes.  That will cause st_mpinx_start_node
	 * to be called during Nexus initialization.
	 */
	nexus_startup_node_t *initial_node;
	NexusMalloc(st_mpinx_preinit(),
		    initial_node,
		    nexus_startup_node_t *,
		    sizeof(nexus_startup_node_t));
	initial_node->name = _nx_copy_string(NEXUS_MPINX_NODE_NAME);
	initial_node->number = 0;
	initial_node->count = n_nodes - 1;
	initial_node->count_other_nodes = 0;
	initial_node->startup_module = _nx_copy_string("mpinx");
	initial_node->startup_funcs = &st_mpinx_funcs;
	initial_node->next = (nexus_startup_node_t *) NULL;
	initial_node->directory_path = (char *) NULL;
	initial_node->executable_path = (char *) NULL;
	initial_node->st_ptr = NULL;

	*nl_head = *nl_tail = initial_node;
    }
    else
    {
	/*
	 * This is a non-master node.
	 * Wait for a message from the master node that contains
	 * _nx_my_process_params for my node.
	 * That message is sent from st_mpinx_start_node.
	 */
	MPI_Status status;
	NexusMalloc(st_mpinx_preinit(),
		    _nx_my_process_params,
		    char *,
		    NEXUS_MAX_COMMAND_LENGTH);
	MPI_Recv(_nx_my_process_params,
		 NEXUS_MAX_COMMAND_LENGTH,
		 MPI_BYTE,
		 0,
		 INITIAL_MSG_TAG,
		 MPI_COMM_WORLD,
		 &status);
    }

    return(NEXUS_FALSE);
} /* st_mpinx_preinit() */


/*
 * st_mpinx_get_master_node_name()
 */
static char *st_mpinx_get_master_node_name(void)
{
    return (_nx_copy_string(NEXUS_MPINX_NODE_NAME));
} /* st_mpinx_get_master_node_name() */


/*
 * _nx_start_nodes()
 */
static int st_mpinx_start_node(nexus_startup_node_t *node,
			       nexus_global_pointer_t *reply_gp,
			       int first_checkin_number)
{
    int i;
    char cmd[NEXUS_MAX_COMMAND_LENGTH], cmd2[NEXUS_MAX_COMMAND_LENGTH];
    int cmd2_len;
    int msglen;
    
    /* Start the nodes on the machines listed in arg_host_list */
    for (i = 0; i < node->count; i++)
    {
	/* Get all arguments that need to go to this process */
	_nx_new_process_params(cmd2, NEXUS_MAX_COMMAND_LENGTH);
	cmd2_len = strlen(cmd2);
	_nx_proto_get_creator_sp_params(cmd2 + cmd2_len,
					NEXUS_MAX_COMMAND_LENGTH - cmd2_len,
					reply_sp);
	nexus_stdio_lock();
	sprintf(cmd, "%s -nx_node %s %d y -nx_node_id %d -nx_checkin %d %s %s",
		_nx_get_package_id_start(),
		node->name,
		node->number + i,
		node->id + i,
		first_checkin_number + i,
		cmd2,
		_nx_get_package_id_end() );
	nexus_stdio_unlock();
	
	msglen = strlen(cmd) + 1;
	MPI_Send(cmd,
		 msglen,
		 MPI_BYTE,
		 node->number + i,
		 INITIAL_MSG_TAG,
		 MPI_COMM_WORLD);
    }

    return 0;
} /* _nx_start_nodes() */


#ifndef HAVE_IBMDS
/*
 * st_mpinx_start_context()
 */
static nexus_bool_t st_mpinx_start_context(char *executable_path,
				     nexus_startpoint_t *reply_sp,
				     int checkin_number,
				     int *return_code,
				     nexus_context_t **new_local_context)
{
    /*
     * TODO: Should get rid of this fatal error, so that other
     * startup and protocol modules can work simultaneously.
     */
    nexus_fatal("st_mpinx_start_context(): ERROR: Cannot create other processes with MPINX\n");
    
    return (NEXUS_FALSE);
    
} /* st_mpinx_start_context() */
#endif /* !HAVE_IBMDS */
