/*
 * commlink.c
 *
 * Communication Link Interface Routines
 */

#include "internal.h"


#ifdef USE_ENDPOINT_HASHTABLE

#define ENDPOINT_HASH_TABLE_SIZE 1024
#define ENDPOINT_TBL_HASH(X) ((X) % (ENDPOINT_HASH_TABLE_SIZE))

static nexus_endpoint_t *Endpointhashtable[ENDPOINT_HASH_TABLE_SIZE];
static unsigned long NextEndpointHashid = 0;
static nexus_mutex_t HashtableLock;

static void insert_endpoint_into_hashtable(nexus_endpoint_t *endpoint);

#endif /* USE_ENDPOINT_HASHTABLE */

static unsigned long next_endpoint_id;
static nexus_mutex_t next_endpoint_id_mutex;

static int same_startpoint(nexus_startpoint_t *sp1,
			   nexus_startpoint_t *sp2,
			   nexus_bool_t compare_addresses);

/*********************************************************************
 * Standard Nexus Functions
 *********************************************************************/
 
/*
 * _nx_commlink_usage_message()
 */
void _nx_commlink_usage_message(void)
{
} /* _nx_commlink_usage_message() */


/*
 * _nx_commlink_new_process_params()
 */
int _nx_commlink_new_process_params(char *buf, int size)
{
    return(0);
} /* _nx_commlink_new_process_params() */


/*
 * _nx_commlink_init()
 *
 * Initialize the handler table for the passed 'context'.
 */
void _nx_commlink_init(int *argc, char ***argv)
{

    int i;

    next_endpoint_id = 1;
    nexus_mutex_init(&next_endpoint_id_mutex, (nexus_mutexattr_t *) NULL);
    
#ifdef USE_ENDPOINT_HASHTABLE
    /* initializing endpoint hash table */
    nexus_mutex_init(&(HashtableLock), (nexus_mutexattr_t *) NULL);
    nexus_mutex_lock(&HashtableLock);
    for (i = 0; i < ENDPOINT_HASH_TABLE_SIZE; i ++)
	Endpointhashtable[i] = (nexus_endpoint_t *) NULL;
    nexus_mutex_unlock(&HashtableLock);
#endif /* USE_ENDPOINT_HASHTABLE */
    
} /* _nx_commlink_init() */


/*********************************************************************
 * Endpoint Attribute Functions
 *********************************************************************/

/*
 * nexus_endpointattr_init()
 */
int nexus_endpointattr_init(nexus_endpointattr_t *epattr)
{
    NexusAssert2((epattr), ("nexus_endpointattr_init(): rcvd NULL gpattr\n"));

    epattr->handler_table = (nexus_handler_t *) NULL;
    epattr->handler_table_size = 0;
    epattr->unknown_handler = NULL;
    epattr->unknown_handler_type = NEXUS_HANDLER_TYPE_NON_THREADED;
    epattr->transform_id = NEXUS_TRANSFORM_NONE;
    epattr->transform_attr = (nexus_transformattr_t *) NULL;
    _nx_context(&(epattr->context));

    return(0);
} /* nexus_endpointattr_init() */


/*
 * nexus_endpointattr_destroy()
 */
int nexus_endpointattr_destroy(nexus_endpointattr_t *epattr)
{
    NexusAssert2((epattr),
		 ("nexus_endpointattr_destroy(): rcvd NULL epattr\n"));

    if (   epattr->transform_id != NEXUS_TRANSFORM_NONE
	&& epattr->transform_attr)
    {
	nexus_transformattr_destroy(epattr->transform_id,
				    epattr->transform_attr);
    }

    return(0);
} /* nexus_endpointattr_destroy() */


/*
 * nexus_endpointattr_set_handler_table()
 */
int nexus_endpointattr_set_handler_table(nexus_endpointattr_t *epattr,
					 nexus_handler_t *handler_table,
					 int handler_table_size)
{
    if (!epattr || !handler_table || handler_table_size < 0)
    {
        return(-1);
    }
    
    epattr->handler_table = handler_table;
    epattr->handler_table_size = handler_table_size;

    return(0);
} /* nexus_endpointattr_set_handler_table() */


/*
 * nexus_endpointattr_get_handler_table()
 */
int nexus_endpointattr_get_handler_table(nexus_endpointattr_t *epattr, 
					 nexus_handler_t **handler_table,
					 int *handler_table_size)
{
    if (!epattr)
    {
        return(-1);
    }

    *handler_table = epattr->handler_table;
    *handler_table_size = epattr->handler_table_size;

    return(0);
} /* nexus_endpointattr_get_handler_table() */


/*
 * nexus_endpointattr_set_unknown_handler()
 */
int nexus_endpointattr_set_unknown_handler(nexus_endpointattr_t *epattr,
					   nexus_unknown_handler_func_t func,
					   nexus_handler_type_t type)
{
    epattr->unknown_handler = func;
    epattr->unknown_handler_type = type;

    return(0);
} /* nexus_endpointattr_set_unknown_handler() */


/*
 * nexus_endpointattr_get_unknown_handler()
 */
int nexus_endpointattr_get_unknown_handler(nexus_endpointattr_t *epattr,
					   nexus_unknown_handler_func_t *func,
					   nexus_handler_type_t *type)
{
    *func = epattr->unknown_handler;
    *type = epattr->unknown_handler_type;

    return(0);
} /* nexus_endpointattr_get_unknown_handler() */


/*
 * nexus_endpoinattr_set_transform()
 */
int nexus_endpointattr_set_transform(nexus_endpointattr_t *epattr,
				     int transform_id,
				     void *transform_info)
{
    epattr->transform_id = transform_id;
    nexus_transformattr_init(transform_id,
			     transform_info,
			     &(epattr->transform_attr));
    return(0);
} /* nexus_endpointattr_set_transform() */


/*
 * nexus_endpoinattr_get_transform()
 *
 * *transform_info should be freed using nexus_free()
 */
int nexus_endpointattr_get_transform(nexus_endpointattr_t *epattr,
				     int *transform_id,
				     void **transform_info)
{
    *transform_id = epattr->transform_id;
    nexus_transformattr_get_info(epattr->transform_id,
				 epattr->transform_attr,
				 transform_info);
    return(0);
} /* nexus_endpointattr_get_transform() */


/*********************************************************************
 * Endpoint Functions
 *********************************************************************/

/*
 * nexus_endpoint_init()
 *
 * this is called only for user-created endpoints ... NOT for
 * the endpoints that are created as a result of a copy.
 * 
 * NOTE: - only signed & nonspoofable endpoints are placed into the hashtable
 *       - removed from hashtable once destroyed.  in other 
 *         words, sits in hashtable iff bound to a gp
 */
int nexus_endpoint_init(nexus_endpoint_t *endpoint,
			nexus_endpointattr_t *epattr)
{
    if (!endpoint || !epattr)
    {
        return(-1);
    }

    endpoint->handler_table = epattr->handler_table;
    endpoint->handler_table_size = epattr->handler_table_size;
    endpoint->unknown_handler = epattr->unknown_handler;
    endpoint->unknown_handler_type = epattr->unknown_handler_type;
    endpoint->transform_id = epattr->transform_id;
    if (epattr->transform_id != NEXUS_TRANSFORM_NONE)
    {
	nexus_transformstate_init_on_endpoint(epattr->transform_id,
					      epattr->transform_attr,
					      &(endpoint->transform_state));
    }
    endpoint->context = epattr->context;
    endpoint->user_pointer = (void *) NULL;
    endpoint->next = (nexus_endpoint_t *) NULL;
    endpoint->prev = (nexus_endpoint_t *) NULL;

    nexus_mutex_lock(&next_endpoint_id_mutex);
    endpoint->id = next_endpoint_id++;
    nexus_mutex_unlock(&next_endpoint_id_mutex);

#ifdef USE_ENDPOINT_HASHTABLE
    /* TODO: Remove this endpoint from the hashtable */
#endif /* USE_ENDPOINT_HASHTABLE */
    
    return(0);
} /* nexus_endpoint_init() */


/*
 * nexus_endpoint_destroy()
 *
 * must destroy this one and ALL within the bind_or_copy chain
 * in which this one appears.
 *
 * this function renders the endpoint completely useless.
 * in order to use this endpoint again must call nexus_endpoint_init()
 * again
 */
int nexus_endpoint_destroy(nexus_endpoint_t *endpoint)
{
    if (!endpoint)
    {
        return(-1);
    }

#ifdef USE_ENDPOINT_HASHTABLE
    /* TODO: Remove this endpoint from the hashtable */
#endif /* USE_ENDPOINT_HASHTABLE */
    
    if (   endpoint->transform_id != NEXUS_TRANSFORM_NONE
	&& endpoint->transform_state)
    {
	nexus_transformstate_destroy_on_endpoint(endpoint->transform_id,
						 endpoint->transform_state);
    }

    return(0);
} /* nexus_endpoint_destroy() */


/*********************************************************************
 * Startpoint Management Functions
 *********************************************************************/

/*
 * nexus_startpoint_bind()
 */
int nexus_startpoint_bind(nexus_startpoint_t *startpoint,
			  nexus_endpoint_t *endpoint)
{
    unsigned long sp_state_id;
    nexus_bool_t copy_sp_locally;
    nexus_bool_t destroy_sp_locally;
    
    if (!endpoint || !startpoint)
    {
        return(-1);
    }

    /*
     * Filling mi_proto
     * Ignore the reference count on local startpoints.
     */
    startpoint->mi_proto = _nx_my_mi_proto;

    startpoint->endpoint_id = endpoint->id;
    
    /* Setup the transform stuff on this startpoint */
    startpoint->transform_id = endpoint->transform_id;
    if (startpoint->transform_id != NEXUS_TRANSFORM_NONE)
    {
	nexus_transformstate_init_on_startpoint(endpoint->transform_id,
						endpoint->transform_state,
						&(startpoint->transform_state),
						&sp_state_id,
						&copy_sp_locally,
						&destroy_sp_locally);
	startpoint->copy_locally = (copy_sp_locally ? 1 : 0);
	startpoint->destroy_locally = (destroy_sp_locally ? 1 : 0);
    }
    else
    {
	startpoint->transform_state = (nexus_transformstate_t *) NULL;
	startpoint->copy_locally = 1;
	startpoint->destroy_locally = 1;
	sp_state_id = 0;
    }

    /* Create a liba */
    /* TODO: Need to add signature to liba */
    if (sp_state_id == 0)
    {
	/*
	 * There is no startpoint transform state id.
	 * Therefore the liba just contains the endpoint.
	 */
	startpoint->liba_size = LibaSizeofWithoutState();
	if (startpoint->liba_size <= NEXUS_DEFAULT_LIBA_SIZE)
	{
	    startpoint->liba_is_inline = 1;
	    LibaPackWithoutState(startpoint->liba.array,
				 (unsigned long) endpoint);
	}
	else
	{
	    startpoint->liba_is_inline = 0;
	    NexusMalloc(bind_startpoint_to_endpoint(),
			startpoint->liba.pointer,
			nexus_byte_t *,
			startpoint->liba_size);
	    LibaPackWithoutState(startpoint->liba.pointer,
				 (unsigned long) endpoint);
	}
    }
    else
    {
	/*
	 * There is a startpoint transform state id.
	 * Therefore the liba contains both the endpoint, and the sp_state_id.
	 */
	startpoint->liba_size = LibaSizeofWithState();
	if (startpoint->liba_size <= NEXUS_DEFAULT_LIBA_SIZE)
	{
	    startpoint->liba_is_inline = 1;
	    LibaPackWithState(startpoint->liba.array,
			      (unsigned long) endpoint,
			      sp_state_id);
	}
	else
	{
	    startpoint->liba_is_inline = 0;
	    NexusMalloc(bind_startpoint_to_endpoint(),
			startpoint->liba.pointer,
			nexus_byte_t *,
			startpoint->liba_size);
	    LibaPackWithState(startpoint->liba.pointer,
			      (unsigned long) endpoint,
			      sp_state_id);
	}
    }

#ifdef BUILD_PROFILE
    _nx_node_id(&(startpoint->node_id));
    _nx_context_id(&(startpoint->context_id));
#endif /* BUILD_PROFILE */

    return(0);

} /* nexus_startpoint_bind() */


/*
 * nexus_startpoint_copy()
 *
 * Copy the startpoint, 'src_sp', to 'dest_sp'.  
 *
 * If this sp has changing transform state, then the endpoint must be
 * contacted to create a new endpoint and sp.  This may be an
 * expensive operation.  This is signalled by src_sp->copy_locally==0.
 *
 * If this sp has static transform state, then a simple copy can be
 * made locally without contacting the endpoint.
 * This is signalled by src_sp->copy_locally==0.
 */
int nexus_startpoint_copy(nexus_startpoint_t *dest_sp, 
			  nexus_startpoint_t *source_sp)
{

    if (!dest_sp || !source_sp)
    {
        return(-1);
    }

    if (source_sp->copy_locally)
    {
	/*
	 * This startpoint can be copied locally.
	 */
	dest_sp->endpoint_id = source_sp->endpoint_id;
	dest_sp->copy_locally = source_sp->copy_locally;
	dest_sp->destroy_locally = source_sp->destroy_locally;

	/* transform stuff */
	dest_sp->transform_id = source_sp->transform_id;
	if (source_sp->transform_id != NEXUS_TRANSFORM_NONE)
	{
	    nexus_transformstate_copy(source_sp->transform_id,
				      source_sp->transform_state,
				      &(dest_sp->transform_state));
	}
	else
	{
	    dest_sp->transform_state = (nexus_transformstate_t *) NULL;
	}
	    
	/* liba */
	dest_sp->liba_is_inline = source_sp->liba_is_inline;
	dest_sp->liba_size = source_sp->liba_size;
	if (source_sp->liba_is_inline)
	{
	    memcpy(dest_sp->liba.array,
		   source_sp->liba.array, 
		   source_sp->liba_size);
	}
	else
	{
	    NexusMalloc(nexus_startpoint_copy(),
			dest_sp->liba.pointer,
			nexus_byte_t *,
			source_sp->liba_size);
	    memcpy(dest_sp->liba.pointer,
		   source_sp->liba.pointer, 
		   source_sp->liba_size);
	}

#ifdef NEXUS_PROFILE
	dest_sp->node_id = source_sp->node_id;
	dest_sp->context_id = source_sp->context_id;
#endif /* NEXUS_PROFILE */

	/* mi_proto */
	dest_sp->mi_proto = source_sp->mi_proto;
	if (dest_sp->mi_proto->proto->funcs->increment_reference_count)
	{
	    (dest_sp->mi_proto->proto->funcs->increment_reference_count)
						(dest_sp->mi_proto->proto);
	}
    }
    else
    {
	/*
	 * Copying this startpoint requires a round-trip to the server.
	 * TODO: Fillin this section.
	 */
    }

    return(0);
} /* nexus_startpoint_copy() */


/*
 * nexus_startpoint_destroy()
 *
 * Destroy the startpoint, 'sp', and set it NULL.
 * NOTE: do NOT notify endpoint ... use nexus_startpoint_destroy_and_notify() 
 *       for that kind of sp destruction
 */
int nexus_startpoint_destroy(nexus_startpoint_t *sp)
{
    if (!sp)
    {
        return(-1);
    }

    if (!nexus_startpoint_is_null(sp))
    {
	_nx_mi_proto_destroy(sp->mi_proto);
	if (!(sp->liba_is_inline))
	{
	    NexusFree(sp->liba.pointer);
	}
	nexus_startpoint_set_null(sp);
    }

    return(0);
} /* nexus_startpoint_destroy() */


/*
 * nexus_startpoint_destroy_and_notify()
 *
 * Destroy the startpoint, 'sp', set it to NULL. 
 *
 * If this sp has per-sp state, contact the endpoint to free up any
 * resources associated with this sp ... wait for confirmation.
 *
 */
int nexus_startpoint_destroy_and_notify(nexus_startpoint_t *sp)
{
    if (!sp)
    {
        return(-1);
    }

    if (!sp->destroy_locally)
    {
	/* TODO: Add a round-trip to the server here */
    }
    
    /* now we can destroy the sp locally */
    nexus_startpoint_destroy(sp);

    return(0);
} /* nexus_startpoint_destroy_and_notify() */


/*********************************************************************
 * Startpoint sizeof/put/get Functions
 *********************************************************************/

/*
 * nexus_sizeof_startpoint()
 *
 * Return the size in bytes needed to encode 'count' startpoints
 * that start at 'sp_array'.
 */
int nexus_sizeof_startpoint(nexus_startpoint_t *sp_array, int count)
{
    nexus_startpoint_t *sp;
    int c;
    int size = 0;

    NexusAssert2((sp_array), 
		 ("nexus_sizeof_startpoint(): rcvd NULL sp_array\n"));
    NexusAssert2((count >= 0),
		 ("nexus_sizeof_startpoint(): rcvd count < 0\n"));
    
    for (c = 0, sp = sp_array; c < count; c++, sp++)
    {
        /* sizeof is_null flag */
        size += nexus_sizeof_int(1);

        if (!nexus_startpoint_is_null(sp))
        {
#ifdef BUILD_PROFILE
            /* sizeof the node_id, context_id */
            size += nexus_sizeof_int(1);
            size += nexus_sizeof_int(1);
#endif /* BUILD_PROFILE */    

	    /* sizeof endpoint_id */
	    size += nexus_sizeof_u_long(1);
	    
	    /* sizeof copy_locally, destroy_locally, transform_id */
	    size += 3 * nexus_sizeof_byte(1);
	    
            /* sizeof liba */
            size += nexus_sizeof_int(1); /* liba_size */
            size += nexus_sizeof_byte(sp->liba_size); /* LIBA */
            
            /* sizeof mi_proto */
            size += nexus_sizeof_int(1);
            size += nexus_sizeof_byte(sp->mi_proto->size);

	    /* sizeof transform_state */
	    if (sp->transform_id != NEXUS_TRANSFORM_NONE)
	    {
		size += nexus_transformstate_sizeof(sp->transform_id,
						    sp->transform_state);
	    }
        } /* endif */
    } /* endfor */
    
    return(size);
} /* nexus_sizeof_startpoint() */


/*
 * nexus_user_put_startpoint_transfer()
 */
int nexus_user_put_startpoint_transfer(nexus_byte_t **buffer, 
				       nexus_startpoint_t *sp_array,
				       unsigned long count)
{
    unsigned long i;
    int tmp_int;
    nexus_byte_t tmp_byte;
    nexus_startpoint_t *sp;

    if (!buffer || !sp_array || count < 0)
    {
        return(-1);
    }

    for (i = 0, sp = sp_array; i < count; i++, sp++)
    {
	/* put is_null flag */
	tmp_int = nexus_startpoint_is_null(sp);
	nexus_user_put_int(buffer, &tmp_int, 1);

	if (!nexus_startpoint_is_null(sp))
	{
#ifdef BUILD_PROFILE
	    /* put the node_id, context_id */
	    nexus_user_put_int(buffer, &(sp->node_id), 1);
	    nexus_user_put_int(buffer, &(sp->context_id), 1);
#endif /* BUILD_PROFILE */    
	
	    /* put endpoint_id */
	    nexus_user_put_u_long(buffer, &(sp->endpoint_id), 1);
	    
	    /* put copy_locally, destroy_locally, transform_id */
	    tmp_byte = sp->copy_locally;
	    nexus_user_put_byte(buffer, &tmp_byte, 1);
	    tmp_byte = sp->destroy_locally;
	    nexus_user_put_byte(buffer, &tmp_byte, 1);
	    tmp_byte = sp->transform_id;
	    nexus_user_put_byte(buffer, &tmp_byte, 1);
	    
	    /* put the liba */
	    tmp_int = sp->liba_size;
	    nexus_user_put_int(buffer, &tmp_int, 1);
	    nexus_user_put_byte(buffer,
				(sp->liba_is_inline 
				 ? (void *) sp->liba.array 
				 : (void *) sp->liba.pointer),
				sp->liba_size);
	    
	    /* put the mi_proto */
	    nexus_user_put_int(buffer, &(sp->mi_proto->size), 1);
	    nexus_user_put_byte(buffer,
				sp->mi_proto->array,
				sp->mi_proto->size);

	    /* put transform_state */
	    if (sp->transform_id != NEXUS_TRANSFORM_NONE)
	    {
		nexus_transformstate_put(sp->transform_id,
					 buffer,
					 sp->transform_state);
	    }
	    
	    nexus_startpoint_destroy(sp);
	} /* endif */
    } /* endfor */

    return(0);
} /* nexus_user_put_startpoint_transfer() */


/*
 * nexus_user_get_startpoint()
 *
 * Copy 'count' startpoints from message buffer into sp's starting at 'sp' 
 */
int nexus_user_get_startpoint(nexus_byte_t **buffer,
			      nexus_startpoint_t *sp_array, 
			      unsigned long count,
			      int format)
{
    nexus_startpoint_t *sp;
    unsigned long i;
    int tmp_int;
    nexus_byte_t tmp_byte;
    int mi_proto_size;
    nexus_mi_proto_t *mi_proto;

    if (!buffer || !sp_array || count < 0)
    {
        return(-1);
    }

    for (i = 0, sp = sp_array; i < count; i++, sp++)
    {
        /* get is_null flag */
        nexus_user_get_int(buffer, &tmp_int, 1, format);

        if (tmp_int)
	{
            nexus_startpoint_set_null(sp);
	}
        else
        {
#ifdef BUILD_PROFILE
            /* get the node_id, context_id */
            nexus_user_get_int(buffer, &(sp->node_id), 1, format);
            nexus_user_get_int(buffer, &(sp->context_id), 1, format);
#endif /* BUILD_PROFILE */    

	    /* put endpoint_id */
	    nexus_user_get_u_long(buffer, &(sp->endpoint_id), 1, format);
	    
	    /* get copy_locally, destroy_locally, transform_id */
	    nexus_user_get_byte(buffer, &tmp_byte, 1, format);
	    sp->copy_locally = (tmp_byte ? 1 : 0);
	    nexus_user_get_byte(buffer, &tmp_byte, 1, format);
	    sp->destroy_locally = (tmp_byte ? 1 : 0);
	    nexus_user_get_byte(buffer, &tmp_byte, 1, format);
	    sp->transform_id = (unsigned int) tmp_byte;
	    
            /* get the liba */
            nexus_user_get_int(buffer, &tmp_int, 1, format);
            sp->liba_size = tmp_int;
            if (sp->liba_size <= NEXUS_DEFAULT_LIBA_SIZE)
            {
                sp->liba_is_inline = 1;
                nexus_user_get_byte(buffer,
				    (nexus_byte_t *) sp->liba.array,
				    sp->liba_size,
				    format);
            }
            else
            {
                sp->liba_is_inline = 0;
                NexusMalloc(nexus_user_get_global_pointer(),
                            sp->liba.pointer,
                            nexus_byte_t *,
                            sp->liba_size);
                nexus_user_get_byte(buffer,
				    (nexus_byte_t *) sp->liba.pointer,
				    sp->liba_size,
				    format);
            } /* endif */
            
            /* get the mi_proto */
            nexus_user_get_int(buffer, &mi_proto_size, 1, format);
            mi_proto = _nx_mi_proto_create(mi_proto_size,
                                       (nexus_byte_t *) NULL,
                                       (nexus_proto_t *) NULL);
            nexus_user_get_byte(buffer,
				mi_proto->array,
				mi_proto_size,
				format);
            sp->mi_proto = _nx_mi_proto_table_insert(mi_proto);

	    /* get transform_state */
	    if (sp->transform_id != NEXUS_TRANSFORM_NONE)
	    {
		nexus_transformstate_get(sp->transform_id,
					 buffer,
					 format,
					 sp->transform_state);
	    }
	    
        } /* endif */
    } /* endfor */

    return(0);
} /* nexus_user_get_startpoint() */


/*
 * nexus_put_startpoint_transfer()
 */
int nexus_put_startpoint_transfer(nexus_buffer_t *buffer, 
				  nexus_startpoint_t *sp_array,
				  unsigned long count)
{
    return(nexus_user_put_startpoint_transfer(
				&((*(buffer))->current_base_segment->current),
				sp_array,
				count));
} /* nexus_put_startpoint_transfer() */


/*
 * nexus_get_startpoint()
 */
int nexus_get_startpoint(nexus_buffer_t *buffer,
			 nexus_startpoint_t *sp_array, 
			 unsigned long count)
{
    return(nexus_user_get_startpoint(
				&((*(buffer))->current_base_segment->current),
				sp_array,
				count,
				(*(buffer))->format));
} /* nexus_get_startpoint() */


/*      
 * _nx_write_startpoint()
 *      
 * Write the startpoint to the passed file descriptor.
 *        
 * Return:  
 *      0 on success
 *      1 if the filedesc is unexpectedly closed
 */ 
int _nx_write_startpoint(int filedesc, nexus_startpoint_t *sp)
{ 
    nexus_byte_t tmp_byte;
    int sp_size;
    int total_size;
    nexus_byte_t *buf, *b;

    if (!sp)
    {
        return(-1);
    }

    sp_size = nexus_sizeof_startpoint(sp, 1);
    total_size = (nexus_sizeof_byte(1)
		  + nexus_sizeof_int(1)
		  + sp_size);
    NexusMalloc(_nx_write_startpoint(),
		buf,
		nexus_byte_t *,
		total_size);
    b = buf;
    tmp_byte = NEXUS_DC_FORMAT_LOCAL;
    nexus_user_put_byte(&b, &tmp_byte, 1);		/* write the format */
    nexus_user_put_int(&b, &sp_size, 1);		/* write the sp size */
    nexus_user_put_startpoint_transfer(&b, sp, 1);	/* write the sp */

    if (_nx_write_blocking(filedesc, buf, total_size) != 0)
    {
	return(1);
    }

    NexusFree(buf);
    
    return(0);
} /* _nx_write_startpoint() */


/*
 * _nx_read_startpoint()
 *  
 * Read a startpoint from the passed file descriptor and fill
 * it into 'sp'.
 *      
 * Return:  
 *      0 on success
 *      1 if the filedesc is unexpectedly closed
 */  
int _nx_read_startpoint(int filedesc, nexus_startpoint_t *sp)
{       
    nexus_byte_t format;
    int int_size;
    int sp_size;
    nexus_byte_t *buf, *b;

    /* Read the format */
    if (_nx_read_blocking(filedesc, &format, 1) != 0)
    {
	return(1);
    }

    /* Read the sp size */
    int_size = nexus_dc_sizeof_remote_int(1, format);
    NexusMalloc(_nx_read_startpoint(),
		buf,
		nexus_byte_t *,
		int_size);
    if (_nx_read_blocking(filedesc, buf, int_size) != 0)
    {
	return(1);
    }
    b = buf;
    nexus_user_get_int(&b, &sp_size, 1, format);
    NexusFree(buf);

    /* Read the sp */
    NexusMalloc(_nx_read_startpoint(),
		buf,
		nexus_byte_t *,
		sp_size);
    if (_nx_read_blocking(filedesc, buf, sp_size) != 0)
    {
	return(1);
    }
    b = buf;
    nexus_user_get_startpoint(&b, sp, 1, format);
    NexusFree(buf);
    
    return(0);
} /* _nx_read_startpoint() */


/*
 * nexus_startpoint_equal()
 *
 * Return non-zero if the two startpoints point to the same address
 * in the same context on the same node, or zero otherwise.
 */
int nexus_startpoint_equal(nexus_startpoint_t *sp1, nexus_startpoint_t *sp2)
{
    if (nexus_startpoint_equal_context(sp1, sp2)
	&& sp1 && sp2 && (sp1->endpoint_id == sp2->endpoint_id) )
    {
	return(NEXUS_TRUE);
    }
    else
    {
	return(NEXUS_FALSE);
    }
} /* nexus_startpoint_equal() */


/*
 * nexus_startpoint_equal_context()
 *
 * Return non-zero if the two startpoints point to the same context
 * on the same node, or zero otherwise.
 */
int nexus_startpoint_equal_context(nexus_startpoint_t *sp1, 
				   nexus_startpoint_t *sp2)
{
    nexus_byte_t *a1, *a2;
    
    if (nexus_startpoint_is_null(sp1) && nexus_startpoint_is_null(sp2))
    {
        /* Both are NULL global pointers */
        return(NEXUS_TRUE);
    }
    if (nexus_startpoint_is_null(sp1) || nexus_startpoint_is_null(sp2))
    {
        /* One is a NULL global pointer, the other isn't */
        return(NEXUS_FALSE);
    }

    /* Compare their node numbers and context differentiators */
    /*
     * TODO: Context differentiator may need to be generalized to handle
     * multiple contexts within the same process.
     */
    a1 = sp1->mi_proto->array;
    a2 = sp2->mi_proto->array;
    if (memcmp(a1, a2, 8) != 0)
    {
        return(NEXUS_FALSE);
    }

    /* Compare their node names */
    if (strcmp((char *) (a1+4), (char *) (a2+4)) != 0)
    {
        return(NEXUS_FALSE);
    }
    
    /* They are the same context */
    return(NEXUS_TRUE);
} /* nexus_startpoint_equal_context() */


/*
 * nexus_startpoint_to_current_context()
 *
 * Return non-zero if the startpoint points to an address in the
 * context of the calling thread. 
 */
int nexus_startpoint_to_current_context(nexus_startpoint_t *sp)
{
    nexus_byte_t *a1, *a2;
    
    if (nexus_startpoint_is_null(sp))
    {
        /* It is a NULL global pointer */
        return(NEXUS_TRUE);
    }

    /* Compare their node numbers and context differentiators */
    /*
     * TODO: Context differentiator may need to be generalized to handle
     * multiple contexts within the same process.
     */
    a1 = sp->mi_proto->array;
    a2 = _nx_my_mi_proto->array;
    if (memcmp(a1, a2, 8) != 0)
    {
        return(NEXUS_FALSE);
    }

    /* Compare their node names */
    if (strcmp((char *) (a1+4), (char *) (a2+4)) != 0)
    {
        return(NEXUS_FALSE);
    }
    
    /* They are the same context */
    return(NEXUS_TRUE);
} /* nexus_startpoint_to_current_context() */


/*
 * nexus_startpoint_get_endpoint()
 *
 * Assume the startpoint is local.
 */
int nexus_startpoint_get_endpoint(nexus_startpoint_t *sp,
				  nexus_endpoint_t **ep)
{
    nexus_byte_t *liba;
    unsigned long ul;
    if (sp->liba_is_inline)
    {
	liba = sp->liba.array;
    }
    else
    {
	liba = sp->liba.pointer;
    }
    LibaUnpackEndpoint(liba, ul);
    *ep = (nexus_endpoint_t *) ul;
    return(NEXUS_SUCCESS);
} /* nexus_startpoint_get_endpoint() */


/*
 * nexus_startpoint_string()
 *
 * Return a string representation of the startpoint.
 */
int nexus_startpoint_string(nexus_startpoint_t *sp, char *string, int length)
{
    unsigned long endpoint_id;

    if (!sp)
    {
        return(-1);
    }
    if (!string)
    {
        return(-2);
    }

    /* LibaUnpackFromStartpoint(sp, flag, endpoint_id); */
    endpoint_id = 0;
    
    string[0] = '\0';

#ifdef BUILD_PROFILE
    if (length < 50 /* some free space, but not much */)
    {
        return(-3);
    }
    sprintf(string, "[STARTPOINT:%08lx:%08lx:n%d:c%d]",
            (unsigned long) sp,
            endpoint_id,
            sp->node_id,
            sp->context_id);
#else  /* BUILD_PROFILE */
    if (length < 30 /* exact */)
    {
        return(-3);
    }
    sprintf(string, "[STARTPOINT:%08lx:%08lx]",
            (unsigned long) sp,
            endpoint_id);
#endif /* BUILD_PROFILE */    
              
    return(0);
} /* nexus_startpoint_string() */


/*
 * nexus_endpoint_string()
 */
int nexus_endpoint_string(nexus_endpoint_t *ep, char *string, int length)
{
    if (!ep)
    {
        return(-1);
    }
    if (!string)
    {
        return(-2);
    }

    /* do something nice here */
    if (length < 0)
    {
        return(-3);
    }

    return(0);
} /* nexus_endpoint_string() */


/*********************************************************************
 * Local Functions
 *********************************************************************/

/*
 * same_startpoint()
 *
 * Compare two startpoints to see if they are in the same context.
 * If compare_addresses==NEXUS_TRUE, then also compare if
 * they point to the same address.
 *
 * NOTE: this unconditionally checks the liba_flag
 *
 * Return NEXUS_TRUE if they are same, or NEXUS_FALSE if they are different.
 */
static int same_startpoint(nexus_startpoint_t *sp1, 
			   nexus_startpoint_t *sp2,
			   nexus_bool_t compare_addresses)
{
    nexus_byte_t *a1, *a2;
    
    NexusAssert2((sp1), ("same_startpoint(): rcvd NULL sp1\n"));
    NexusAssert2((sp2), ("same_startpoint(): rcvd NULL sp2\n"));

    if (nexus_startpoint_is_null(sp1) && nexus_startpoint_is_null(sp2))
    {
        /* Both are NULL global pointers */
        return(NEXUS_TRUE);
    }
    if (nexus_startpoint_is_null(sp1) || nexus_startpoint_is_null(sp2))
    {
        /* One is a NULL global pointer, the other isn't */
        return(NEXUS_FALSE);
    }

    /* Compare their liba's */
    a1 = (sp1->liba_is_inline ? sp1->liba.array : sp1->liba.pointer);
    a2 = (sp2->liba_is_inline ? sp2->liba.array : sp2->liba.pointer);

    /* Compare their node numbers and context differentiators */
    /*
     * TODO: Need to add a second context differentiator to handle
     * multiple contexts within the same process.
     */
    a1 = sp1->mi_proto->array;
    a2 = sp2->mi_proto->array;
    if (memcmp(a1, a2, 8) != 0)
    {
        return(NEXUS_FALSE);
    }

    /* Compare their node names */
    if (strcmp((char *) (a1+4), (char *) (a2+4)) != 0)
    {
        return(NEXUS_FALSE);
    }
    
    /* They are the same context */
    return(NEXUS_TRUE);
} /* same_startpoint() */


/*********************************************************************
 * Endpoint hashtable functions
 *********************************************************************/

#ifdef USE_ENDPOINT_HASHTABLE
/*
 * _nx_lookup_endpoint_id()
 */
nexus_endpoint_t *_nx_lookup_endpoint_id(unsigned long endpoint_id, 
					 nexus_bool_t already_locked)
{
    int hash_idx;
    nexus_endpoint_t *endpoint;

    nexus_debug_printf(1, ("enter _nx_lookup_endpoint_id() endpoint_id %ld (NextEndpointHashid %ld) already_locked %c\n", endpoint_id, NextEndpointHashid, (already_locked ? 'T' : 'F')));

    if (!already_locked)
    {
	nexus_mutex_lock(&HashtableLock);
    }

    hash_idx = ENDPOINT_TBL_HASH(endpoint_id);
    
    nexus_debug_printf(2, ("_nx_lookup_endpoint_id() endpoint_id %ld hashes to %d Endpointhashtable[%d] = %x\n", endpoint_id, hash_idx, hash_idx, Endpointhashtable[hash_idx]));

    /* finding the endpoint in the hashtable */
    for (endpoint = Endpointhashtable[hash_idx];
	 endpoint && endpoint->hash_id != endpoint_id;
	 endpoint = endpoint->next)
    /* do nothing */ ;

    if (!already_locked)
    {
	nexus_mutex_unlock(&HashtableLock);
    }

    nexus_debug_printf(1, ("exit _nx_lookup_endpoint_id() endpoint %x\n", endpoint));
    return(endpoint);
} /* _nx_lookup_endpoint_id() */

/*
 * insert_endpoint_into_hashtable()
 *
 * assumes access to hashtable has already been locked 
 */
static void insert_endpoint_into_hashtable(nexus_endpoint_t *endpoint)
{
    int hash_idx;

    NexusAssert2((endpoint), 
		 ("insert_endpoint_into_hashtable(): rcvd NULL endpoint\n"));

    nexus_debug_printf(1, ("entering insert_endpoint_into_hashtable() with endpoint->hash_id %ld (NextEndpointHashid %ld)\n", endpoint->hash_id, NextEndpointHashid));

    hash_idx = ENDPOINT_TBL_HASH(endpoint->hash_id);

    nexus_debug_printf(2, ("insert_endpoint_into_hashtable() resolved endpoint->hash_id %ld to hash_idx %d Endpointhashtable[%d] = %x\n", endpoint->hash_id, hash_idx, hash_idx, Endpointhashtable[hash_idx]));

    if (Endpointhashtable[hash_idx])
    {
	nexus_debug_printf(2, ("insert_endpoint_into_hashtable() inserting into table with collision\n"));
	Endpointhashtable[hash_idx]->prev = endpoint;
	endpoint->next = Endpointhashtable[hash_idx];
    }
    else
    {
	nexus_debug_printf(2, ("insert_endpoint_into_hashtable() inserting into table empty slot of table\n"));
	endpoint->next = (nexus_endpoint_t *) NULL;
    } /* endif */

    endpoint->prev = (nexus_endpoint_t *) NULL;
    Endpointhashtable[hash_idx] = endpoint;

    nexus_debug_printf(1, ("exiting insert_endpoint_into_hashtable()\n"));
} /* insert_endpoint_into_hashtable() */

#endif /* USE_ENDPOINT_HASHTABLE */

