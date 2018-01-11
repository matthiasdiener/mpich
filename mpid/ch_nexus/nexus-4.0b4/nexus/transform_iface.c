/*
 * transform_iface.c
 *
 * Data transform module interface routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/transform_iface.c,v 1.6 1997/02/21 15:18:28 tuecke Exp $";

#include "internal.h"

/*
 * nexus_transform_usage_message()
 *
 * Call the usage_message() function for each protocol
 */
void nexus_transform_usage_message(void)
{
} /* nexus_transform_usage_message() */


/*
 * nexus_transform_new_process_params()
 *
 * Call the new_process_params() function for each protocol
 *
 * Each of those functions may add stuff to 'buf', returning the number
 * of characters that they added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int nexus_transform_new_process_params(char *buf, int size)
{
    return (0);
} /* nexus_transform_new_process_params() */


/*
 * nexus_transform_init()
 */
int nexus_transform_init(int *argc, char ***argv,
			 nexus_module_list_t module_list[])
{
    int i;

    /* Initialize the transform table */
    for (i = 0; i < NEXUS_TRANSFORM_TABLE_SIZE; i++)
    {
	_nx_transform_table[i].funcs = (nexus_transform_funcs_t *) NULL;
	_nx_transform_table[i].name = (char *) NULL;
	_nx_transform_table[i].modifies_data = NEXUS_FALSE;
	_nx_transform_table[i].increases_size = NEXUS_FALSE;
	_nx_transform_table[i].header_size = 0;
	_nx_transform_table[i].trailer_size = 0;
    }

    /*
     * Scan the module_list looking for transform modules.
     * For each of these, get the function table,
     * and add that module to the _nx_transform_table
     */
    for (i = 0; module_list[i].family_name != (char *) NULL; i++)
    {
	if (strcmp(module_list[i].family_name, "transform") == 0)
	{
	    nexus_transform_add_module(module_list[i].module_name,
				       module_list[i].info_func,
				       argc, argv);
	}
    }

    return(0);
} /* nexus_transform_init() */


/*
 * nexus_transform_add_module()
 */
void nexus_transform_add_module(char *module_name,
				void *(*info_func)(void),
				int *argc, char ***argv)
{
    nexus_transform_funcs_t *funcs;
    int id;

    funcs = (nexus_transform_funcs_t *)(*info_func)();
    id = (*funcs->transform_id)();
    _nx_transform_table[id].funcs = funcs;
    _nx_transform_table[id].name = _nx_copy_string(module_name);
    (*funcs->init)(argc, argv,
		   &(_nx_transform_table[id].modifies_data),
		   &(_nx_transform_table[id].increases_size),
		   &(_nx_transform_table[id].header_size),
		   &(_nx_transform_table[id].trailer_size));
} /* nexus_transform_add_module() */


/*
 * nexus_transform_shutdown()
 */
void nexus_transform_shutdown(void)
{
    int i;
    for (i = 0; i < NEXUS_TRANSFORM_TABLE_SIZE; i++)
    {
	if (   _nx_transform_table[i].funcs
	    && _nx_transform_table[i].funcs->shutdown)
	{
	    (*_nx_transform_table[i].funcs->shutdown)();
	}
    }
} /* nexus_transform_shutdown() */


/*
 * nexus_transform_abort()
 */
void nexus_transform_abort(void)
{
    int i;
    for (i = 0; i < NEXUS_TRANSFORM_TABLE_SIZE; i++)
    {
	if (   _nx_transform_table[i].funcs
	    && _nx_transform_table[i].funcs->abort)
	{
	    (*_nx_transform_table[i].funcs->abort)();
	}
    }
} /* nexus_transform_abort() */


/*
 * nexus_transformattr_init()
 */
void nexus_transformattr_init(int id,
			      void *info,
			      nexus_transformattr_t **attr)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->transformattr_init)
    {
	(*_nx_transform_table[id].funcs->transformattr_init)(info,
							     attr);
    }
    else
    {
	*attr = (nexus_transformattr_t *) NULL;
    }
} /* nexus_transformattr_init() */


/*
 * nexus_transformattr_destroy()
 */
void nexus_transformattr_destroy(int id,
				 nexus_transformattr_t *attr)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->transformattr_destroy)
    {
	(*_nx_transform_table[id].funcs->transformattr_destroy)(attr);
    }
} /* nexus_transformattr_destroy() */


/*
 * nexus_transformattr_get_info()
 *
 * This basically reverses nexus_transformattr_init().  It takes
 * a nexus_transformattr_t*, and returns an info structure
 * in *info.
 *
 * *info must be created with nexus_malloc(), or set to NULL
 * if there is no info structure for this attribute.
 */
void nexus_transformattr_get_info(int id,
				  nexus_transformattr_t *attr,
				  void **info)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->transformattr_get_info)
    {
	(*_nx_transform_table[id].funcs->transformattr_get_info)(attr,
								 info);
    }
} /* nexus_transformattr_get_info() */


/*
 * nexus_transformstate_init_on_endpoint()
 *
 * This is called by nexus_endpoint_init().
 * As input it takes the nexus_transformattr_t* from the endpointattr,
 * and returns a nexus_transformstate_t* for the endpoint.
 */
void nexus_transformstate_init_on_endpoint(int id,
					   nexus_transformattr_t *attr,
					   nexus_transformstate_t **ep_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->init_endpoint_state)
    {
	(*_nx_transform_table[id].funcs->init_endpoint_state)(attr,
							      ep_state);
    }
    else
    {
	*ep_state = (nexus_transformstate_t *) NULL;
    }
} /* nexus_transformstate_init_on_endpoint() */


/*
 * nexus_transformstate_destroy_on_endpoint()
 */
void nexus_transformstate_destroy_on_endpoint(int id,
					      nexus_transformstate_t *ep_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->destroy_endpoint_state)
    {
	(*_nx_transform_table[id].funcs->destroy_endpoint_state)(ep_state);
    }
} /* nexus_transformstate_destroy_on_endpoint() */


/*
 * nexus_transformstate_init_on_startpoint()
 *
 * This is called by nexus_startpoint_bind(), taking the endpoint's
 * nexus_transformstate_t* as input.
 * It returns:
 *   - A nexus_transformstate_t* for the startpoint.
 *   - A unique id that identifies this startpoint within
 *     the endpoint state.  This should be 0 if its not needed.
 *   - A boolean specifying whether nexus_startpoint_copy() can
 *     be done locally (NEXUS_TRUE), or must be done as a round
 *     trip to the context with the endpoint to which this
 *     startpoint is bound.
 */
void nexus_transformstate_init_on_startpoint(int id,
					     nexus_transformstate_t *ep_state,
					     nexus_transformstate_t **sp_state,
					     unsigned long *sp_state_id,
					     nexus_bool_t *copy_sp_locally,
					     nexus_bool_t *destroy_sp_locally)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->init_startpoint_state)
    {
	(*_nx_transform_table[id].funcs->init_startpoint_state)(
	    						ep_state,
							sp_state,
							sp_state_id,
							copy_sp_locally,
							destroy_sp_locally);
    }
    else
    {
	*sp_state = (nexus_transformstate_t *) NULL;
	*sp_state_id = 0;
	*copy_sp_locally = NEXUS_TRUE;
	*destroy_sp_locally = NEXUS_TRUE;
    }
} /* nexus_transformstate_init_on_startpoint() */


/*
 * nexus_transformstate_copy()
 *
 * If a startpoint can be copied locally (i.e. it doesn't require a
 * roundtrip back to the endpoint to copy), then nexus_startpoint_copy()
 * calls this routine to copy its nexus_transformstate_t*.
 */
void nexus_transformstate_copy(int id,
			       nexus_transformstate_t *sp_state,
			       nexus_transformstate_t **sp_state_copy)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->copy_startpoint_state)
    {
	(*_nx_transform_table[id].funcs->copy_startpoint_state)(sp_state,
								sp_state_copy);
    }
    else
    {
	*sp_state_copy = (nexus_transformstate_t *) NULL;
    }
} /* nexus_transformstate_copy() */


/*
 * nexus_transformstate_destroy_on_startpoint()
 */
void nexus_transformstate_destroy_on_startpoint(int id,
					      nexus_transformstate_t *sp_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->destroy_startpoint_state)
    {
	(*_nx_transform_table[id].funcs->destroy_startpoint_state)(sp_state);
    }
} /* nexus_transformstate_destroy_on_startpoint() */


/*
 * nexus_transformstate_sizeof()
 */
int nexus_transformstate_sizeof(int id,
				nexus_transformstate_t *sp_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->sizeof_state)
    {
	return(*_nx_transform_table[id].funcs->sizeof_state)(sp_state);
    }
    return(0);
} /* nexus_transformstate_sizeof() */


/*
 * nexus_transformstate_put()
 *
 * Use nexus_dc_put_*() routines to put the transform state.
 */
void nexus_transformstate_put(int id,
			      nexus_byte_t **buffer,
			      nexus_transformstate_t *sp_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->put_state)
    {
	(*_nx_transform_table[id].funcs->put_state)(buffer, sp_state);
    }
} /* nexus_transformstate_put() */


/*
 * nexus_transformstate_get()
 *
 * Use nexus_dc_get_*() routines to put the transform state.
 */
void nexus_transformstate_get(int id,
			      nexus_byte_t **buffer,
			      int format,
			      nexus_transformstate_t **sp_state)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->get_state)
    {
	(*_nx_transform_table[id].funcs->get_state)(buffer,
						   format,
						   sp_state);
    }
} /* nexus_transformstate_get() */


/*
 * nexus_buffer_transform()
 */
int nexus_buffer_transform(int id,
			   nexus_byte_t *buffer,
			   nexus_transformstate_t *sp_state,
			   unsigned long buffer_size,
			   unsigned long header_size,
			   unsigned long data_size,
			   unsigned long save_header_size,
			   nexus_bool_t can_transform_inplace,
			   nexus_byte_t **out_buffer,
			   unsigned long *out_buffer_size,
			   unsigned long *out_header_size,
			   unsigned long *out_transform_info_size,
			   unsigned long *out_data_size,
			   unsigned long *out_untransform_size,
			   nexus_bool_t *out_must_be_freed)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->transform)
    {
	return(*_nx_transform_table[id].funcs->transform)(
	    				buffer,
					sp_state,
					buffer_size,
					header_size,
					data_size,
					save_header_size,
					can_transform_inplace,
					out_buffer,
					out_buffer_size,
					out_header_size,
					out_transform_info_size,
					out_data_size,
					out_untransform_size,
					out_must_be_freed);
    }
    return(-1);
} /* nexus_buffer_transform() */

/*
 * nexus_buffer_untransform()
 */
int nexus_buffer_untransform(int id,
			     nexus_byte_t *buffer,
			     nexus_transformstate_t *ep_state,
			     unsigned long sp_state_id,
			     unsigned long buffer_size,
			     unsigned long header_size,
			     unsigned long data_size,
			     unsigned long save_header_size,
			     nexus_bool_t can_untransform_inplace,
			     int format,
			     unsigned long untransform_size,
			     nexus_byte_t **out_buffer,
			     unsigned long *out_buffer_size,
			     unsigned long *out_header_size,
			     unsigned long *out_data_size,
			     nexus_bool_t *out_must_be_freed)
{
    if (   id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE
	&& _nx_transform_table[id].funcs
	&& _nx_transform_table[id].funcs->untransform)
    {
	return(*_nx_transform_table[id].funcs->untransform)(
	    				buffer,
					ep_state,
					sp_state_id,
					buffer_size,
					header_size,
					data_size,
					save_header_size,
					can_untransform_inplace,
					format,
					untransform_size,
					out_buffer,
					out_buffer_size,
					out_header_size,
					out_data_size,
					out_must_be_freed);
    }
    return(-1);
} /* nexus_buffer_untransform() */


/*
 * nexus_transform_info()
 *
 * modifies_data:	Set to NEXUS_TRUE if the transform modifies the data
 * increases_size:	Set to NEXUS_TRUE if the transform may increase the
 *			size of the data beyond header_size and trailer_size.
 * header_size:		Set to number of bytes the transform needs to add
 *			to the front of the message.
 * trailer_size:	Set to number of bytes the transform may need to
 *			add to the end of the message.
 */
void nexus_transform_info(int id,
			  nexus_bool_t *modifies_data,
			  nexus_bool_t *increases_size,
			  unsigned long *header_size,
			  unsigned long *trailer_size)
{
    if (id >= 0 && id < NEXUS_TRANSFORM_TABLE_SIZE)
    {
	*modifies_data = _nx_transform_table[id].modifies_data;
	*increases_size = _nx_transform_table[id].increases_size;
	*header_size = _nx_transform_table[id].header_size;
	*trailer_size = _nx_transform_table[id].trailer_size;
    }
    else
    {
	*modifies_data = NEXUS_FALSE;
	*increases_size = NEXUS_FALSE;
	*header_size = 0;
	*trailer_size = 0;
    }
} /* nexus_transform_header_size() */


