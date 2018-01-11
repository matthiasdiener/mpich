/*
 * handler.c
 *
 * Handler manipulation routines.
 * This file is no longer used in nexus-4.0.
 * It remains as reference for the rsr profiling stuff.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/handler.c,v 1.36 1996/12/11 05:42:33 tuecke Exp $";

#ifdef DEPRICATED_HANDLER

#include "internal.h"

#ifndef BUILD_LITE
typedef struct _threaded_handler_startup_info_t
{
    nexus_context_t *			context;
    void *				address;
#ifdef BUILD_PROFILE
    int					node_id;
    int					context_id;
    int					message_length;
#endif	    
    nexus_stashed_buffer_t		buffer;
    nexus_handler_record_t *		rec;
    nexus_bool_t			started;
    nexus_mutex_t			mutex;
    nexus_cond_t			cond;
} threaded_handler_startup_info_t;

static void *threaded_handler_startup(void *arg);
#endif /* BUILD_LITE */


static nexus_bool_t	arg_got_hash;
static int		handler_table_size;

#ifdef BUILD_PROFILE
static nexus_bool_t	arg_got_rsr_hash;
static int		profile_rsr_table_size;
#endif

/*
 * _nx_handler_usage_message()
 */
void _nx_handler_usage_message(void)
{
    printf("    -hash <integer>           : Handler hash table size.  (Do not use this\n");
    printf("                                unless you know what you are doing...)\n");
#ifdef BUILD_PROFILE
    printf("    -rsr_hash <integer>       : Remote service request profile hash\n");
    printf("                                table size.\n");
#endif /* BUILD_PROFILE */
} /* _nx_handler_usage_message() */


/*
 * _nx_handler_new_process_params()
 */
int _nx_handler_new_process_params(char *buf, int size)
{
    char tmp_buf1[1024];
#ifdef BUILD_PROFILE
    char tmp_buf2[1024];
#endif    
    int n_added;

    nexus_stdio_lock();

    tmp_buf1[0] = '\0';

    if (arg_got_hash)
    {
	sprintf(tmp_buf1, "-hash %d ", handler_table_size);
    }

#ifdef BUILD_PROFILE
    if (arg_got_rsr_hash)
    {
	sprintf(tmp_buf2, "-rsr_hash %d ", profile_rsr_table_size);
	strcat(tmp_buf1, tmp_buf2);
    }
#endif /* BUILD_PROFILE */

    n_added = strlen(tmp_buf1);
    if (n_added > size)
    {
	nexus_fatal("_nx_handler_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
    }

    strcpy(buf, tmp_buf1);

    nexus_stdio_unlock();
    
    return (n_added);

} /* _nx_handler_new_process_params() */


/*
 * _nx_handler_init()
 *
 * Initialize the handler table for the passed 'context'.
 */
void _nx_handler_init(int *argc,
		      char ***argv)
{
    int arg_num;
    
    if ((arg_num = nexus_find_argument(argc, argv, "hash", 2)) >= 0)
    {
	handler_table_size = atoi((*argv)[arg_num + 1]);
	arg_got_hash = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	handler_table_size = NEXUS_HANDLER_HASH_TABLE_SIZE;
	arg_got_hash = NEXUS_FALSE;
    }
    
#ifdef BUILD_PROFILE
    if ((arg_num = nexus_find_argument(argc, argv, "rsr_hash", 2)) >= 0)
    {
	profile_rsr_table_size = atoi((*argv)[arg_num + 1]);
	arg_got_rsr_hash = NEXUS_TRUE;
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    else
    {
	profile_rsr_table_size = NEXUS_PROFILE_RSR_HASH_TABLE_SIZE;
	arg_got_rsr_hash = NEXUS_FALSE;
    }
#endif
} /* _nx_handler_init() */


/*
 * _nx_handle_message()
 *
 * Call the handler function, 'func' function with 'endpoint' and 'buffer'
 * as arguments.  Set the thread to the given 'context' before
 * doing so.
 *
 * If type==NEXUS_HANDLER_TYPE_THREADED, then the function will be
 * called in a newly thread, and the assumed types are:
 *	nexus_stashed_buffer_t buffer
 *	nexus_threaded_handler_func_t func
 * If type==NEXUS_HANDLER_TYPE_NON_THREADED, then the function will be
 * called in this thread, and the assumed types are:
 *	nexus_buffer_t buffer
 *	nexus_non_threaded_handler_func_t func
 *
 * Note: The buffer that is passed to a threaded handler is actually of
 * type nexus_stashed_buffer_t*, which means it really wants the address
 * of the buffer argument (&buffer).  However, we need to make
 * sure that the &buffer is done from within the newly created
 * thread for that handler, so that it doesn't dereference through
 * another thread's stack.  This was a nasty bug to find...
 *
 * If 'handler_func' is NULL, then NexusUnknownHandler() is called
 * using the 'handler_name' and 'handler_id' arguments.
 * If 'handler_func' is not NULL, then the 'handler_name'
 * and 'handler_id' arguments are ignored.
 * This was done so that you could avoid copying the handler_name
 * unless _nx_lookup_handler() failed to return a function pointer.
 *
 * If the handler is non-threaded, then free the buffer
 * using nexus_buffer_destroy(), after the handler completes.
 * If the handler is threaded, then do not free the buffer.
 */
void _nx_handle_message(int handler_id,
			nexus_endpoint_t *endpoint,
#ifdef BUILD_PROFILE
			int node_id,
			int context_id,
			int message_length,
#endif
			void *buffer)
{
    nexus_context_t *context;
    nexus_handler_t rec;

    if (handler_id > 0 && handler_id < endpoint->handler_table_size)
    {
        rec = endpoint->handler_table[handler_id];
    }
    else
    {
        rec.func = NULL;
	rec.type = NEXUS_HANDLER_TYPE_NON_THREADED;
    }

    if (rec.func == NULL)
    {
	/* Call NexusUnknownHandler() */
	nexus_context_t *save_context;
	nexus_unknown_handler_func_t *NexusUnknownHandler;
	nexus_buffer_t buf = (nexus_buffer_t) buffer;
	
	NexusUnknownHandler = endpoint->unknown_handler;
	
	nexus_debug_printf(1, ("_nx_handle_message(): Handler:[%d] not found. %s\n",
			       handler_id,
			       (NexusUnknownHandler ? "Calling unknown handler" :
				"Ignoring (unknown handler not registered)")));
	
	_nx_context(&save_context);
	_nx_set_context(endpoint->context);
	
#ifdef BUILD_PROFILE	
	_nx_pablo_log_remote_service_request_receive(node_id,
						     context_id,
						     handler_name,
						     handler_id,
						     message_length);
#endif /* BUILD_PROFILE */	    
	
	if (NexusUnknownHandler)
	{
	    NexusAssert2((rec.type == NEXUS_HANDLER_TYPE_NON_THREADED),
			 ("_nx_handle_message(): NexusUnknownHandler() must be a non-threaded handler\n") );

	    (*NexusUnknownHandler)(endpoint, &buf, handler_id);
	}
	_nx_set_context(save_context);
	nexus_buffer_destroy(&buf);
    }
    else
    {
	/* Call the handler */
#ifndef BUILD_LITE
	if (endpoint->handler_table[handler_id].type == NEXUS_HANDLER_TYPE_THREADED)
	{
	    threaded_handler_startup_info_t info;
	    nexus_thread_t thread;

	    info.context = endpoint->context;
	    info.endpoint = endpoint;
#ifdef BUILD_PROFILE
	    info.node_id = node_id;
	    info.context_id = context_id;
	    info.message_length = message_length;
#endif	    
	    /*
	     * This buffer has already been saved.
	     */
	    info.buffer = buffer;
	    info.rec = rec;
	    info.started = NEXUS_FALSE;
	    nexus_mutex_init(&(info.mutex), (nexus_mutexattr_t *) NULL);
	    nexus_cond_init(&(info.cond), (nexus_condattr_t *) NULL);

	    nexus_debug_printf(2, ("_nx_handle_message(): creating thread to handle buffer:%x\n",buffer));

	    nexus_thread_create(&thread,
				(nexus_thread_attr_t *) NULL,
				threaded_handler_startup,
				(void *) (&info) );

	    nexus_debug_printf(2, ("_nx_handle_message(): thread created\n"));

	    nexus_mutex_lock(&(info.mutex));
	    while (!info.started)
	    {
		nexus_cond_wait(&(info.cond), &(info.mutex));
	    }
	    nexus_mutex_unlock(&(info.mutex));
	    nexus_mutex_destroy(&(info.mutex));
	    nexus_cond_destroy(&(info.cond));
	}
	else /* (type == NEXUS_HANDLER_TYPE_NON_THREADED) */
#endif /* BUILD_LITE */
	{
	    nexus_handler_func_t handler_func = (nexus_handler_func_t)rec.func;
	    nexus_context_t *save_context;
	    nexus_buffer_t buf = (nexus_buffer_t) buffer;
	    
	    _nx_context(&save_context);
	    _nx_set_context(context);

#ifdef BUILD_PROFILE	
	    _nx_pablo_log_remote_service_request_receive(node_id,
							 context_id,
							 rec->name,
							 handler_id,
							 message_length);
	    if (_nx_pablo_count_remote_service_requests())
	    {
		_nx_accumulate_rsr_profile_info(context,
						rec,
						node_id,
						context_id,
						message_length);
	    }
#endif /* BUILD_PROFILE */
	    nexus_debug_printf(2, ("Beginning non-threaded handler call with endpoint:%x and buffer:%x\n",endpoint, &buf));
	    (*handler_func)(endpoint, &buf, NEXUS_FALSE);
	    nexus_debug_printf(2, ("Ending non-threaded handler call\n"));
	    nexus_buffer_destroy(&buf);
	    nexus_debug_printf(2, ("Buffer freed\n"));
	    _nx_set_context(save_context);
	}
    }

} /* _nx_handle_message() */


#ifndef BUILD_LITE
/*
 * threaded_handler_startup()
 *
 * Startup a threaded handler.  The info about the handler is
 * in arg.
 */
static void *threaded_handler_startup(void *arg)
{
    threaded_handler_startup_info_t *info
	= (threaded_handler_startup_info_t *) arg;
    nexus_context_t *context;
    nexus_endpoint_t *endpoint;
    nexus_stashed_buffer_t buffer;
    nexus_handler_record_t *rec;
    nexus_handler_func_t func;
#ifdef BUILD_PROFILE    
    int node_id;
    int context_id;
    int message_length;
#endif    

    context = info->context;
    endpoint = info->endpoint;
#ifdef BUILD_PROFILE
    node_id = info->node_id;
    context_id = info->context_id;
    message_length = info->message_length;
#endif
    buffer = info->buffer;
    rec = info->rec;
    func = (nexus_handler_func_t) rec->func;
    
    _nx_set_context(context);
    
    nexus_mutex_lock(&(info->mutex));
    info->started = NEXUS_TRUE;
    nexus_cond_signal(&(info->cond));
    nexus_mutex_unlock(&(info->mutex));

#ifdef BUILD_PROFILE	
    _nx_pablo_log_remote_service_request_receive(node_id,
						 context_id,
						 rec->name,
						 rec->id,
						 message_length);
    if (_nx_pablo_count_remote_service_requests())
    {
	_nx_accumulate_rsr_profile_info(context,
					rec,
					node_id,
					context_id,
					message_length);
    }
#endif /* BUILD_PROFILE */

#ifdef BUILD_DEBUG
    if(NexusDebug(2)) {
	nexus_printf("threaded_handler_startup(): calling handler for buffer:%x\n",buffer);
    }
#endif

    (*func)(endpoint, &buffer, NEXUS_FALSE);

    return (NULL);
    
} /* threaded_handler_startup() */
#endif /* BUILD_LITE */


#ifdef BUILD_PROFILE
/*
 * _nx_accumulate_rsr_profile_info()
 *
 * Accumulate the count and message length of a message destined
 * for the handler described by 'rec', for the given source 'node_id'
 * and 'context_id'.
 */
void _nx_accumulate_rsr_profile_info(nexus_context_t *context,
				     nexus_handler_record_t *rec,
				     int node_id,
				     int context_id,
				     int message_length)
{
    nexus_profile_rsr_record_t *profile_rec;
    int i;
    int hash;

    nexus_mutex_lock(&(context->mutex));
    
    /* Allocate the profile table, if there isn't one already */
    if (rec->profile_table == (nexus_profile_rsr_record_t **) NULL)
    {
	NexusMalloc(_nx_accumulate_rsr_profile_info(),
		    rec->profile_table,
		    nexus_profile_rsr_record_t **,
		    (profile_rsr_table_size
		     * sizeof(nexus_profile_rsr_record_t *)) );
	for (i = 0; i < profile_rsr_table_size; i++)
	{
	    rec->profile_table[i] = (nexus_profile_rsr_record_t *) NULL;
	}
    }

    /* Find the appropriate profile record in the table */
    hash = (node_id + context_id) % profile_rsr_table_size;
    for (profile_rec = rec->profile_table[hash];
	 (   (profile_rec != (nexus_profile_rsr_record_t *) NULL)
	  && (   (profile_rec->node_id != node_id)
	      || (profile_rec->context_id != context_id)));
	 profile_rec = profile_rec->next)
	;

    if (profile_rec == (nexus_profile_rsr_record_t *) NULL)
    {
	/* Didn't find an entry, so allocate one and store the data */
	NexusMalloc(_nx_accumulate_rsr_profile_info(),
		    profile_rec,
		    nexus_profile_rsr_record_t *,
		    (sizeof(nexus_profile_rsr_record_t)) );
	profile_rec->node_id = node_id;
	profile_rec->context_id = context_id;
	profile_rec->count = 1;
	profile_rec->size = message_length;
	profile_rec->next = rec->profile_table[hash];
	rec->profile_table[hash] = profile_rec;
    }
    else
    {
	/* Found and entry, so bump the data */
	profile_rec->count++;
	profile_rec->size += message_length;
    }

    /* See if we need to dump the current counts */
    context->rsr_profile_count++;
    if (_nx_pablo_should_dump_remote_service_request_counts(context->rsr_profile_count))
    {
	_nx_dump_rsr_profile_info(context);
    }

    nexus_mutex_unlock(&(context->mutex));
    
} /* _nx_accumulate_rsr_profile_info() */


/*
 * _nx_dump_rsr_profile_info()
 *
 * Dump and reset the rsr profile records for the current context.
 */
void _nx_dump_rsr_profile_info(nexus_context_t *context)
{
    nexus_handler_record_t *handler_rec;
    nexus_profile_rsr_record_t *profile_rec;
    int i, j;
    int my_node_id, my_context_id;
    int snapshot_id;
    char *handler_name;
    int handler_id;

    _nx_node_id(&my_node_id);
    my_context_id = context->id;
    snapshot_id = context->snapshot_id;
    
    for (i = 0; i < handler_table_size; i++)
    {
	for (handler_rec = context->handler_table[i];
	     handler_rec;
	     handler_rec = handler_rec->next)
	{
	    if (handler_rec->profile_table)
	    {
		handler_name = handler_rec->name;
		handler_id = handler_rec->id;
		for (j = 0; j < profile_rsr_table_size; j++)
		{
		    for (profile_rec = handler_rec->profile_table[j];
			 profile_rec;
			 profile_rec = profile_rec->next)
		    {
			if (profile_rec->count > 0)
			{
			    _nx_pablo_log_remote_service_request_count(
						profile_rec->count,
						snapshot_id,
						profile_rec->node_id,
						profile_rec->context_id,
						my_node_id,
						my_context_id,
						handler_name,
						handler_id,
						profile_rec->size);
			    profile_rec->count = 0;
			    profile_rec->size = 0;
			}
		    }
		}
	    }
	}
    }

    context->snapshot_id++;
    context->rsr_profile_count = 0;
    
} /* _nx_dump_rsr_profile_info() */

#endif /* BUILD_PROFILE */

#endif /* DEPRICATED_HANDLER */
