/*
 * buffer.c
 *
 * Buffer management code.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/buffer.c,v 1.19 1997/02/21 05:24:49 tuecke Exp $";

#include "internal.h"

#define DEFAULT_BASE_SEGMENT_SIZE	4096
#define DEFAULT_DIRECT_SEGMENT_SIZE	128

/* TODO: This value should really be calculated */
#define FIXED_HEADER_SIZE		64
#define RESERVED_HEADER_SIZE		(NEXUS_MAX_TRANSFORM_INFO_SIZE \
					 + NEXUS_MAX_LIBA_SIZE \
					 + FIXED_HEADER_SIZE)

#define MALLOC_ALIGNMENT		sizeof(double)

#ifndef BUILD_LITE
typedef struct _threaded_handler_startup_info_t
{
    nexus_context_t *			context;
    nexus_endpoint_t *			endpoint;
#ifdef BUILD_PROFILE
    int					node_id;
    int					context_id;
    int					message_length;
#endif	    
    nexus_buffer_t			buffer;
    nexus_handler_func_t		func;
} threaded_handler_startup_info_t;

static void *threaded_handler_startup(void *arg);
#endif /* BUILD_LITE */

static int			sizeof_base_segment;
static int			sizeof_direct_segment;
static unsigned long		default_base_segment_size;
static unsigned long		default_direct_segment_size;

static nexus_mutex_t		freelists_mutex;
static nexus_buffer_t		buffer_freelist_head;
static nexus_base_segment_t *	base_freelist_head;
static nexus_direct_segment_t *	direct_freelist_head;

#define LockFreelists() \
    nexus_mutex_lock(&freelists_mutex)
#define UnlockFreelists() \
    nexus_mutex_unlock(&freelists_mutex)

static nexus_buffer_t buffer_alloc();
static void buffer_free(nexus_buffer_t buffer);
static nexus_base_segment_t *base_segment_malloc(unsigned long size);
static nexus_direct_segment_t *direct_segment_malloc(unsigned long size);
static void base_segment_alloc(nexus_buffer_t buffer,
			       unsigned long size);
static void base_segments_free(nexus_base_segment_t *base_segments);
static void direct_segment_alloc(nexus_buffer_t buffer,
				 unsigned long size);
static void direct_segments_free(nexus_direct_segment_t *direct_segments);


		    
/*
 * _nx_buffer_usage_message()
 */
void _nx_buffer_usage_message(void)
{
} /* _nx_buffer_usage_message() */

/*
 * _nx_buffer_new_process_params()
 */
int _nx_buffer_new_process_params(char *buf, int size)
{
    return (0);
} /* _nx_buffer_new_process_params() */


/*
 * _nx_buffer_init()
 */
void _nx_buffer_init(int *argc, char ***argv)
{
    /*
     * TODO: Add command line arguments to
     * change default_*_segment_size
     */
    default_base_segment_size = DEFAULT_BASE_SEGMENT_SIZE;
    default_direct_segment_size = DEFAULT_DIRECT_SEGMENT_SIZE;

    sizeof_base_segment = sizeof(nexus_base_segment_t);
    if (sizeof_base_segment % MALLOC_ALIGNMENT != 0)
    {
	sizeof_base_segment += (MALLOC_ALIGNMENT
				- (sizeof_base_segment % MALLOC_ALIGNMENT));
    }
    sizeof_direct_segment = sizeof(nexus_direct_segment_t);
    if (sizeof_direct_segment % MALLOC_ALIGNMENT != 0)
    {
	sizeof_direct_segment += (MALLOC_ALIGNMENT
				- (sizeof_direct_segment % MALLOC_ALIGNMENT));
    }

    nexus_mutex_init(&freelists_mutex, (nexus_mutexattr_t *)NULL);
    buffer_freelist_head = NULL;
    base_freelist_head = NULL;
    direct_freelist_head = NULL;
    
} /* _nx_buffer_init() */


/*
 * _nx_buffer_shutdown()
 */
void _nx_buffer_shutdown(void)
{
    nexus_buffer_t buffer;
    nexus_base_segment_t *buf_segment;
    nexus_direct_segment_t *direct_seg;
    
    nexus_mutex_destroy(&freelists_mutex);

    for (buffer = buffer_freelist_head;
	 buffer;
	 buffer_freelist_head = buffer)
    {
        buffer = buffer_freelist_head->next;
	NexusFree(buffer_freelist_head);
    }

    for (buf_segment = base_freelist_head;
	 buf_segment;
	 base_freelist_head = buf_segment)
    {
        buf_segment = base_freelist_head->next;
	if (base_freelist_head->size > 0)
	{
	    NexusFree(base_freelist_head->storage);
	}
	NexusFree(base_freelist_head);
    }

    for (direct_seg = direct_freelist_head;
	 direct_seg;
	 direct_freelist_head = direct_seg)
    {
        direct_seg = direct_freelist_head->next;
	NexusFree(direct_freelist_head);
    }
} /* _nx_buffer_shutdown() */


/*
 * nexus_buffer_init()
 */
int nexus_buffer_init(nexus_buffer_t *buffer,
		      unsigned long buffer_size,
		      unsigned long num_direct_puts)
{
    LockFreelists();
    *buffer = buffer_alloc();
    base_segment_alloc(*buffer,
		       (buffer_size + RESERVED_HEADER_SIZE));
    if (num_direct_puts > 0)
    {
        direct_segment_alloc(*buffer,
			     num_direct_puts);
    }
    UnlockFreelists();

#ifdef BUILD_DEBUG
    (*buffer)->magic = NEXUS_BUFFER_MAGIC;
#endif    
    (*buffer)->reserved_header_size = RESERVED_HEADER_SIZE;
    (*buffer)->format = NEXUS_DC_FORMAT_LOCAL;
    (*buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_UNSAVED;
    (*buffer)->current_base_segment->current
	+= (*buffer)->reserved_header_size;

    return(0);
} /* nexus_buffer_init() */


/*
 * nexus_buffer_reset()
 */
int nexus_buffer_reset(nexus_buffer_t *buffer,
		       unsigned long buffer_size,
		       unsigned long num_direct_puts)
{
    if (   !(*buffer)->base_segments
	|| (*buffer)->base_segments->next
	|| (buffer_size + RESERVED_HEADER_SIZE
	    > (*buffer)->base_segments->size)
	|| (   (num_direct_puts > 0)
	    && (   !(*buffer)->direct_segments
		|| (*buffer)->direct_segments->next
		|| (num_direct_puts > (*buffer)->direct_segments->size) ) ) )
    {
	int rc;
	nexus_buffer_destroy(buffer);
	rc = nexus_buffer_init(buffer,
			       buffer_size,
			       num_direct_puts);
	return(rc);
    }

    if (   (num_direct_puts <= 0)
	&& (*buffer)->direct_segments)
    {
	LockFreelists();
	direct_segments_free((*buffer)->direct_segments);
	UnlockFreelists();
	(*buffer)->direct_segments = NULL;
	(*buffer)->current_direct_segment = NULL;
    }
    
#ifdef BUILD_DEBUG
    (*buffer)->magic = NEXUS_BUFFER_MAGIC;
#endif
    (*buffer)->funcs = NULL;
    (*buffer)->next = NULL;
    (*buffer)->reserved_header_size = RESERVED_HEADER_SIZE;
    (*buffer)->format = NEXUS_DC_FORMAT_LOCAL;
    (*buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_UNSAVED;
    (*buffer)->n_direct = 0;
    (*buffer)->current_base_segment = (*buffer)->base_segments;
    (*buffer)->current_base_segment->current
	+= (*buffer)->reserved_header_size;
    return(0);
} /* nexus_buffer_reset() */


/*
 * nexus_buffer_destroy()
 */
int nexus_buffer_destroy(nexus_buffer_t *buffer)
{
    LockFreelists();
    switch ((*buffer)->saved_state)
    {
    case NEXUS_BUFFER_SAVED_STATE_UNSAVED:
    case NEXUS_BUFFER_SAVED_STATE_SAVED_IN_HANDLER:
	/*
	 * This is being called from a non-threaded handler.
	 * So just mark the buffer as freed, and let the buffer
	 * destruction happen after the non-threaded handler returns.
	 */
	(*buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_FREED;
        break;
    case NEXUS_BUFFER_SAVED_STATE_SAVED:
	/*
	 * This is being called from a thread.
	 * So free the buffer.
	 */
	direct_segments_free((*buffer)->direct_segments);
	base_segments_free((*buffer)->base_segments);
	buffer_free(*buffer);
	break;
    case NEXUS_BUFFER_SAVED_STATE_FREED:
	/*
	 * This buffer is already freed, so do nothing.
	 */
	break;
    }
    UnlockFreelists();
    return(0);
} /* nexus_buffer_destroy() */


/*
 * nexus_check_buffer_size()
 *
 * Return:
 *	0 if there is enough room in the buffer (perhaps after resizing)
 *	-1 if malloc fails
 *	-2 if there is not enough room, but the increment is 0
 */
int nexus_check_buffer_size(nexus_buffer_t *buffer,
			    int size_needed,
			    int size_increment,
			    int num_direct_puts_needed,
			    int num_direct_puts_increment)
{
    /* Add a base segment if necessary */
    if (   (size_needed > 0)
	&& (   (!((*(buffer))->current_base_segment))
	    || (((*(buffer))->current_base_segment->current + size_needed)
		  > ((*(buffer))->current_base_segment->storage
		     + (*(buffer))->current_base_segment->size) ) ) )
    {
        int real_size;

        if (size_increment <= 0)
	{
	    return(-2);
	}

	for (real_size = 0;
	     real_size < size_needed;
	     real_size += size_increment)
	  /* do nothing */ ;
	
	LockFreelists();
        base_segment_alloc(*buffer,
			   real_size);
	UnlockFreelists();
    }
    
    /* Add a direct segment if necessary */
    if (   (num_direct_puts_needed > 0)
	&& (   (!((*(buffer))->current_direct_segment))
	    || (((*(buffer))->current_direct_segment->current
		 + num_direct_puts_needed)
		> ((*(buffer))->current_direct_segment->storage
		   + (*(buffer))->current_base_segment->size) ) ) )
    {
        int real_puts;

	if (num_direct_puts_increment <= 0)
	{
	    return(-2);
	}
	
	for (real_puts = 0;
	     real_puts < num_direct_puts_needed;
	     real_puts += num_direct_puts_increment)
	  /* do nothing */ ;
	
	LockFreelists();
        direct_segment_alloc(*buffer,
			     real_puts);
	UnlockFreelists();
    }

    return(0);
} /* nexus_check_buffer_size() */
    

/*
 * buffer_add_header()
 */
static void buffer_add_header(nexus_byte_t *buffer,
			      nexus_byte_t **header_start,
			      nexus_byte_t **transform_info_start,
			      nexus_byte_t **message_info_start,
			      unsigned long buffer_size,
			      unsigned long *transform_body_size,
			      unsigned long *full_message_size,
			      int liba_size,
			      nexus_byte_t *liba,
			      int transform_info_size,
			      int handler_id,
			      unsigned long total_direct_puts,
			      unsigned long direct_info_offset,
			      nexus_bool_t has_other_info,
			      unsigned long other_info_offset)
{
    int header_size;
    int message_info_size;
    int pad_size;
    int total_size;
    nexus_byte_t flags;
    nexus_byte_t tmp_byte;
    nexus_byte_t *b;

    /* figure out header sizes */
    header_size
	= (nexus_dc_sizeof_byte(1) * 3		/* format, flags, liba_size */
	   + nexus_dc_sizeof_u_long(1)		/* full_message_size */
	   + liba_size);
    message_info_size
	= (nexus_dc_sizeof_int(1)		/* handler_id */
	   + (total_direct_puts > 0 ? nexus_dc_sizeof_u_long(1) : 0)
	   + (has_other_info ? nexus_dc_sizeof_u_long(1) : 0) );
    total_size = header_size + transform_info_size + message_info_size;
    if (total_size % 8)
    {
        pad_size = 8 - (total_size % 8);
	total_size += pad_size;
    }
    else
    {
        pad_size = 0;
    }

    /*
     * Compute new message sizes, and figure out where to start the header.
     * It is assumed that there are enough available bytes
     * reserved before 'buffer' to hold the header.
     */
    *full_message_size = buffer_size + total_size;
    *transform_body_size = buffer_size + message_info_size;
    *header_start = buffer - total_size;
    
    /* fillin the flags byte */
    flags = pad_size;
    if (total_direct_puts > 0)
    {
        flags |= 0x10;
    }
    if (has_other_info)
    {
        flags |= 0x08;
    }
    
    /* now fill in the header bytes */
    b = *header_start;
    *b++ = (nexus_byte_t) NEXUS_DC_FORMAT_LOCAL;
    nexus_dc_put_u_long(&b, full_message_size, 1);
    nexus_dc_put_byte(&b, &flags, 1);
    tmp_byte = liba_size;
    nexus_dc_put_byte(&b, &tmp_byte, 1);
    nexus_dc_put_byte(&b, liba, liba_size);
    b += pad_size;
    *transform_info_start = b;
    b += transform_info_size;
    *message_info_start = b;
    nexus_dc_put_int(&b, &handler_id, 1);
    if (total_direct_puts > 0)
    {
        nexus_dc_put_u_long(&b, &direct_info_offset, 1);
    }
    if (has_other_info)
    {
        nexus_dc_put_u_long(&b, &other_info_offset, 1);
    }
    /* assert (b == buffer) */
} /* buffer_add_header() */


/*
 *  _nx_buffer_coalesce()
 *
 * Take 'buffer' and massage it so that it is ready to send.
 *
 * Return in *r_buffer a buffer that is ready to send:
 *	(*r_buffer)->iovec_formatted is set to NEXUS_TRUE if the contents
 *		of (*r_buffer)->base_segments a struct iovec
 *	(*r_buffer)->base_segments points to either:
 *		- A single segment with the entire base message.
 *		  That message is at:
 *			(*r_buffer)->base_segments->current
 *		  with a size of:
 *			(*r_buffer)->base_segments->size_used
 *		- A single segment containing a struct iovec.
 *	(*r_buffer)->current_base_segment = (*r_buffer)->base_segments
 *	(*r_buffer)->reserved_header_size is set to the number of
 *		unused bytes at the start of
 *		(*r_buffer)->base_segments->storage.  So
 *		( ( (*r_buffer)->base_segments->storage
 *		   + (*r_buffer)->reserved_header_size)
 *		 = (*r_buffer)->base_segments->current)
 *	(*r_buffer)->direct_segments points to a single segment which
 *		contains all of the custom direct puts, or NULL if there
 *		are no direct segments.
 *		If (*r_buffer)->iovec_formatted is NEXUS_TRUE, then this
 *		field is always NULL.
 *	(*r_buffer)->n_direct is set to the number of custom direct
 *		puts contained in (*r_buffer)->direct_segments.
 *		If this value is 0, then send_rsr()
 *		need not wait for the data to be written to the network
 *		before returning.
 *
 * Input values are:
 *	startpoint:
 *		The startpoint to which this message is being sent.
 *	handler_id:
 *		The handler_id to which this message is	being sent.
 *	total_direct_puts:
 *		This is the total number of direct puts in buffer.
 *		The protocol module send_rsr() must fill in the action
 *		of each nexus_direct_info_t, so it can count the total
 *		at that point, so that this routine need not recount.
 *	called_from_non_threaded_handler:
 *		Is this call being made from a non-threaded handler?
 *		If so, then this function will copy all data so that
 *		there are no direct componenents.  This allows the
 *		send_rsr() to complete immediately.
 *	can_use_iovec:
 *		If this is NEXUS_TRUE, the protocol module can make
 *		use of a struct iovec.  (For example, to do a writev().)
 *	destroy_buffer:
 *		If this is NEXUS_TRUE, then 'buffer' will be destroyed
 *		immediately after this send.  In that case, *r_buffer
 *		can simply be set to point to buffer, rather than
 *		allocating a whole new buffer.
 */
int _nx_buffer_coalesce(struct _nexus_buffer_t *buffer,
			struct _nexus_buffer_t **r_buffer,
			nexus_startpoint_t *startpoint,
			int handler_id,
			unsigned long total_direct_puts,
			nexus_bool_t called_from_non_threaded_handler,
			nexus_bool_t can_use_iovec,
			nexus_bool_t destroy_buffer)
{
    nexus_base_segment_t *	base_segments;
    nexus_direct_segment_t *	direct_segments;
    nexus_byte_t *		storage_start;
    nexus_byte_t *		header_start;
    nexus_byte_t *		transform_info_start;
    nexus_byte_t *		message_info_start;
    nexus_byte_t *		body_start;
    nexus_byte_t *		b;
    nexus_bool_t		has_other_info;
    unsigned long		other_info_offset;
    unsigned long		body_size;
    unsigned long		transform_body_size;
    unsigned long		total_size;
    unsigned long		size;
    nexus_bool_t		freelists_locked;
    nexus_base_segment_t *	bseg;
    nexus_direct_segment_t *	dseg;
    nexus_bool_t		transform_modifies_data;
    nexus_bool_t		transform_increases_size;
    unsigned long		transform_info_size;
    unsigned long		transform_trailer_size;

    has_other_info = NEXUS_FALSE;
    other_info_offset = 0;

    base_segments = buffer->base_segments;
    direct_segments = buffer->direct_segments;
    
    if (destroy_buffer)
    {
	/* Can return nexus_buffer_t without copying it */
	*r_buffer = buffer;
	(*r_buffer)->base_segments = (nexus_base_segment_t *) NULL;
	(*r_buffer)->direct_segments = (nexus_direct_segment_t *) NULL;
	freelists_locked = NEXUS_FALSE;
    }
    else
    {
	/* Must create new nexus_buffer_t to return */
	LockFreelists();
	freelists_locked = NEXUS_TRUE;
	*r_buffer = buffer_alloc();
    }

    nexus_transform_info(startpoint->transform_id,
			 &transform_modifies_data,
			 &transform_increases_size,
			 &transform_info_size,
			 &transform_trailer_size);
    
    if (total_direct_puts == 0)
    {
	/* There are no direct puts in this buffer */
	body_start = (base_segments->storage
		      + buffer->reserved_header_size);
	body_size = base_segments->current - body_start;
	if (base_segments->next == (nexus_base_segment_t *) NULL)
	{
	    /* All of the base data is in the first segment */
	    if (   destroy_buffer
		&& (buffer->reserved_header_size >= RESERVED_HEADER_SIZE)
		&& (!transform_modifies_data) )
		/* TODO: Check transform_trailer size */
	    {
		/* Can use segment without copying it */
		(*r_buffer)->base_segments = base_segments;
	    }
	    else
	    {
		/* Must copy segment */
		/* TODO: Add in transform_trailer size */
		base_segment_alloc(*r_buffer,
				   (body_size + RESERVED_HEADER_SIZE));
		(*r_buffer)->reserved_header_size = RESERVED_HEADER_SIZE;
		memcpy(((*r_buffer)->base_segments->storage
			+ (*r_buffer)->reserved_header_size),
		       body_start,
		       body_size);
		body_start = ((*r_buffer)->base_segments->storage
			      + (*r_buffer)->reserved_header_size);

		/* Free the old base segment if necessary */
		if (destroy_buffer)
		{
		    if (!freelists_locked)
		    {
			LockFreelists();
			freelists_locked = NEXUS_TRUE;
		    }
		    base_segments_free(base_segments);
		}
	    }
	}
	else
	{
	    /*
	     * The base data is spread across multiple segments.
	     * So consolidate it to a single segment.
	     */

	    /* Calculate the total number of bytes in the base segments */
	    total_size = body_size;
	    for (bseg = base_segments->next;
		 bseg;
		 bseg = bseg->next)
	    {
		total_size += (bseg->current - bseg->storage);
	    }

	    /* Allocate a new base segment to hold all of the data */
	    /* TODO: Add in transform_trailer size */
	    base_segment_alloc(*r_buffer,
			       (total_size + RESERVED_HEADER_SIZE));
	    (*r_buffer)->reserved_header_size = RESERVED_HEADER_SIZE;

	    /* Copy the data into the single base segment */
	    b = ((*r_buffer)->base_segments->storage
		 + (*r_buffer)->reserved_header_size);
	    memcpy(b, body_start, body_size);
	    b += body_size;
	    for (bseg = base_segments->next;
		 bseg;
		 bseg = bseg->next)
	    {
		size = (bseg->current - bseg->storage);
		memcpy(b, bseg->storage, size);
		b += size;
	    }

	    /* Reset the body variables for the new base segment */
	    body_start = ((*r_buffer)->base_segments->storage
			  + (*r_buffer)->reserved_header_size);
	    body_size = total_size;

	    /* Free the old base segments if necessary */
	    if (destroy_buffer)
	    {
		if (!freelists_locked)
		{
		    LockFreelists();
		    freelists_locked = NEXUS_TRUE;
		}
		base_segments_free(base_segments);
	    }
	}
	if (freelists_locked)
	{
	    UnlockFreelists();
	}
	buffer_add_header(body_start,
			  &header_start,
			  &transform_info_start,
			  &message_info_start,
			  body_size,
			  &transform_body_size,
			  &total_size,
			  startpoint->liba_size,
			  (startpoint->liba_is_inline
			   ? startpoint->liba.array
			   : startpoint->liba.pointer),
			  transform_info_size,
			  handler_id,
			  total_direct_puts,
			  0,
			  has_other_info,
			  0);
	
	(*r_buffer)->base_segments->current = header_start;
	(*r_buffer)->base_segments->size_used = total_size;
	(*r_buffer)->reserved_header_size
	    = (header_start - (*r_buffer)->base_segments->storage);
	(*r_buffer)->current_base_segment = (*r_buffer)->base_segments;
	(*r_buffer)->direct_segments = (nexus_direct_segment_t *) NULL;
	(*r_buffer)->n_direct = 0;
	
	if (startpoint->transform_id != NEXUS_TRANSFORM_NONE)
	{
	    /*
	    nexus_buffer_transform(message_info_start,
	                           transform_body_size);
	    */
	}
    }
    else
    {
	/* There are direct puts in this buffer */
	if (freelists_locked)
	{
	    UnlockFreelists();
	}
    }

    (*r_buffer)->iovec_formatted = NEXUS_FALSE;
    (*r_buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_SAVED;
    return(0);

#ifdef DONT_INCLUDE
    /* else (total_direct_puts > 0) */
    
    unsigned long total_size;
    unsigned long total_inline_sizes;
    unsigned long total_custom_puts;
    nexus_bool_t copy_inlines;
    nexus_bool_t copy_pointer;
    nexus_bool_t copy_custom;
    unsigned long required_header_size;
    unsigned long direct_info_size;
    unsigned long required_body_size;
    int iovec_count;
    int i, j;
    unsigned long data_size;
    nexus_byte_t *start_data;
    nexus_byte_t *cur_location;
    unsigned long cur_custom_put;
    nexus_base_segment_t *base_segment;
    nexus_direct_segment_t *direct_segment;
    nexus_byte_t tmp_byte;
    nexus_byte_t *cur_header;
    unsigned long total_message_size;
    nexus_byte_t pad_size;
    nexus_byte_t flags;
    unsigned long direct_info_offset;
    nexus_byte_t *start_transform;
    unsigned long save_header_size;
    unsigned long out_size;
    unsigned long out_header_size;
    unsigned long out_transform_info_size;
    unsigned long out_data_size;
    unsigned long out_untransform_size;
    nexus_bool_t out_must_be_freed;

    required_header_size
	= (nexus_transform_header_size(startpoint->trasform_id)
	   + startpoint->liba_size
	   + nexus_dc_sizeof_byte(1) * 3
	   + nexus_dc_sizeof_u_long(1)
	   + nexus_dc_sizeof_int(1)
	   + (total_direct_puts>0?nexus_dc_sizeof_u_long(1):0)
	   + (has_other_info?nexus_dc_sizeof_u_long(1):0));
    if (required_header_size % 8)
    {
        pad_size = 8 - (required_header_size % 8);
	required_header_size += pad_size;
    }
    else
    {
        pad_size = 0;
    }

    if (total_direct_puts > 0)
    {
	if (   !called_from_non_threaded_handler
	    && can_use_writev
	    && startpoint->transform_id == NEXUS_TRANSFORM_NONE)
	{
	    copy_inlines = NEXUS_FALSE;
	}
	else
	{
	    copy_inlines = NEXUS_TRUE;
	}

	if (   called_from_non_threaded_handler
	    || nexus_transform_modifies_data(startpoint->transform_id)
	    || nexus_transform_increases_size(startpoint->transform_id))
	{
	    copy_pointer = NEXUS_TRUE;
	    copy_custom = NEXUS_TRUE;
	}
	else
	{
	    copy_pointer = NEXUS_FALSE;
	    copy_custom = NEXUS_FALSE;
	}

	/*
	 * direct_info contains:
	 *
	 * u_int:          n_direct_components, proto_info_size
	 * byte_stream:    proto_info
	 * for (0..n_direct_components)
	 *   u_long:       size, {location,pointer,custom_info}
	 *   byte:         datatype, approach
	 * endfor
	 */
	direct_info_size = (total_direct_puts * (nexus_dc_sizeof_u_long(1) * 2
						 + nexus_dc_sizeof_byte(1) * 2)
			    + nexus_dc_sizeof_u_int(1) * 2
			    + (*buffer)->funcs->direct_info_size());
    }
    else
    {
	direct_info_size = 0;
    }
    
    required_body_size = (total_size
			  + direct_info_size
			  - (*buffer)->reserved_header_size);

    if (   called_from_non_threaded_handler
	|| !can_use_writev)
    {
        (*c_buffer)->use_writev = NEXUS_FALSE;
    }
    else
    {
        (*c_buffer)->use_writev = NEXUS_TRUE;
    }

    total_inline_sizes = 0;
    total_custom_puts = 0;
    iovec_count = 1;
    for (direct_segment = (*buffer)->direct_segments;
	 direct_segment;
	 direct_segment = direct_segment->next)
    {
        for (i = 0; i < direct_segment->size; i++)
	{
	    switch (direct_segment->storage[i].action)
	    {
	      case NEXUS_DIRECT_INFO_ACTION_INLINE:
		if (copy_inlines)
		{
		    total_inline_sizes += direct_segment->storage[i].size;
		}
		else
		{
		    iovec_count++;
		}
		break;
	      case NEXUS_DIRECT_INFO_ACTION_POINTER:
		if (copy_pointer)
		{
		    total_inline_sizes += direct_segment->storage[i].size;
		}
		break;
	      case NEXUS_DIRECT_INFO_ACTION_CUSTOM:
		if (copy_custom)
		{
		    total_inline_sizes += direct_segment->storage[i].size;
		}
		else
		{
		    iovec_count++;
		    total_custom_puts++;
		}
		break;
	      default: /* error */
		break;
	    }
	}
    }
    required_body_size += total_inline_sizes;

    data_size = required_header_size + required_body_size;
    if (   destroy_buffer
	&& required_header_size < (*buffer)->reserved_header_size
	&& (*buffer)->base_segments->size > required_body_size + (*buffer)->reserved_header_size)
    {
        /* use first segment as coalesce buffer */
        start_data = ((*buffer)->base_segments->storage
		      + (*buffer)->reserved_header_size
		      - required_header_size);
    }
    else
    {
        /* get new coalesce buffer */
        NexusMalloc(_nx_buffer_coalesce(),
		    start_data,
		    nexus_byte_t *,
		    data_size);
	cur_location = start_data + required_header_size;
	for (base_segment = (*buffer)->base_segments;
	     base_segment;
	     base_segment = base_segment->next)
	{
	    memcpy(cur_location, base_segment->storage, base_segment->size);
	    cur_location += base_segment->size;
	}
    }
    /*
     assert(we have a contiguous buffer big enough to hold the base message)
     assert(there is room for the header)
     assert(all non-direct info is in buffer)
     */
    
    if (!(*c_buffer)->use_writev)
    {
	(*c_buffer)->u.data.data = start_data;
	(*c_buffer)->u.data.data_size = data_size;
	NexusMalloc(_nx_buffer_coalesce(),
		    (*c_buffer)->u.data.custom_direct_puts,
		    nexus_direct_info_t *,
		    sizeof(nexus_direct_info_t) * total_custom_puts);
	(*c_buffer)->u.data.num_custom_direct_puts = total_custom_puts;
	
	cur_location = start_data + data_size;
	cur_custom_put = 0;
	direct_segment = (*buffer)->direct_segments;
	for (i = 0;
	     i < total_direct_puts;
	     direct_segment = direct_segment->next)
	{
	    for (j = 0; j < direct_segment->size; i++, j++)
	    {
		switch(direct_segment->storage[j].action)
		{
		  case NEXUS_DIRECT_INFO_ACTION_INLINE:
		    /* assert(copy_inlines == NEXUS_TRUE) */
		    memcpy(cur_location,
			    direct_segment->storage[j].data,
			    direct_segment->storage[j].size);
		    cur_location += direct_segment->storage[j].size;
		    break;
		  case NEXUS_DIRECT_INFO_ACTION_POINTER:
		    if (copy_pointer)
		    {
			memcpy(cur_location,
			       direct_segment->storage[j].data,
			       direct_segment->storage[j].size);
			direct_segment->storage[j].data = cur_location;
			cur_location += direct_segment->storage[j].size;
		    }
		    else
		    {
		    }
		    break;
		  case NEXUS_DIRECT_INFO_ACTION_CUSTOM:
		    if (copy_custom)
		    {
			memcpy(cur_location,
			       direct_segment->storage[j].data,
			       direct_segment->storage[j].size);
			cur_location += direct_segment->storage[j].size;
		    }
		    else
		    {
		        memcpy((*c_buffer)->u.data.custom_direct_puts[cur_custom_put++].data,
			       direct_segment->storage[j].data,
			       sizeof(nexus_direct_info_t));
		    }
		    break;
		  default: /* error */
		    break;
		}
	    }
	}
		      
	(*c_buffer)->u.data.num_custom_direct_puts = total_custom_puts;
    }
    else
    {
        /* allocate iovec for writev */
        (*c_buffer)->u.writev.iovec_count = iovec_count;
	NexusMalloc(_nx_buffer_coalesce(),
		    (*c_buffer)->u.writev.iovec,
		    struct iovec *,
		    sizeof(struct iovec) * (*c_buffer)->u.writev.iovec_count);
	/* fill in iovec */
	(*c_buffer)->u.writev.iovec[0].iov_base = (caddr_t)start_data;
	(*c_buffer)->u.writev.iovec[0].iov_len = data_size;
	
	direct_segment = (*buffer)->direct_segments;
	for (i = 0;
	     i < (*c_buffer)->u.writev.iovec_count;
	     direct_segment = direct_segment->next)
	{
	    for (j = 0; j < direct_segment->size; j++)
	    {
	        switch(direct_segment->storage[j].action)
		{
		  case NEXUS_DIRECT_INFO_ACTION_INLINE:
		    /* assert (copy_inline == NEXUS_FALSE) */
		    (*c_buffer)->u.writev.iovec[i].iov_base =
		        direct_segment->storage[j].data;
		    (*c_buffer)->u.writev.iovec[i].iov_len =
		        direct_segment->storage[j].size;
		    i++;
		    break;
		  case NEXUS_DIRECT_INFO_ACTION_POINTER:
		    /* assert (copy_pointer == NEXUS_FALSE) */
		    break;
		  case NEXUS_DIRECT_INFO_ACTION_CUSTOM:
		    /* assert (copy_custom == NEXUS_FALSE) */
		    (*c_buffer)->u.writev.iovec[i].iov_base =
		        direct_segment->storage[j].data;
		    (*c_buffer)->u.writev.iovec[i].iov_len =
		        direct_segment->storage[j].size;
		    i++;
		    break;
		  default: /* error */
		    break;
		}
	    }
	}
    }
    /*
     add direct info to segment
     if should use writev
         fill in writev segment
     else
         copy inlines to segment
     */
    save_header_size = (nexus_dc_sizeof_int(1)
			- (total_direct_puts>0 ? nexus_dc_sizeof_u_long(1) : 0)
			- (has_other_info ? nexus_dc_sizeof_u_long(1) : 0));
    cur_header = start_data + required_header_size - save_header_size;
    start_transform = cur_header;
    nexus_dc_put_int(&cur_header, &handler_id, 1);
    if (total_direct_puts > 0)
    {
        nexus_dc_put_u_long(&cur_header, &direct_info_offset, 1);
    }
    if (has_other_info)
    {
        nexus_dc_put_u_long(&cur_header, &other_info_offset, 1);
    }
    /* assert (data location == cur_header) */
    nexus_buffer_transform(start_transform,
		       startpoint->transform_state,
		       required_header_size + required_body_size,
		       required_header_size,
		       required_body_size,
		       save_header_size,
		       destroy_buffer || !nexus_transform_modifies_data(startpoint),
		       &cur_header,
		       &out_size,
		       &out_header_size,
		       &out_transform_info_size,
		       &out_data_size,
		       &out_untransform_size,
		       &out_must_be_freed);
    cur_header = start_data;
    tmp_byte = NEXUS_DC_FORMAT_LOCAL;
    nexus_dc_put_byte(&cur_header, &tmp_byte, 1);
    total_message_size = (out_header_size
			  + out_data_size
			  + out_transform_info_size);
    nexus_dc_put_u_long(&cur_header, &total_message_size, 1);
    /* set flags */
    flags = pad_size & 0x07;
    if (total_direct_puts > 0)
    {
        flags |= 0x10;
    }
    else
    {
        flags &= 0xef;
    }
    if (has_other_info)
    {
        flags |= 0x08;
    }
    else
    {
        flags &= 0xf7;
    }
    nexus_dc_put_byte(&cur_header, &flags, 1);
    tmp_byte = startpoint->liba_size;
    nexus_dc_put_byte(&cur_header, &tmp_byte, 1);
    nexus_dc_put_byte(&cur_header,
		      startpoint->liba_is_inline ?
		          startpoint->liba.array : startpoint->liba.pointer,
		      startpoint->liba_size);
    cur_header += pad_size;
    return(0);
#endif /* DONT_INCLUDE */

} /* _nx_buffer_coalesce() */


/*
 * _nx_buffer_create_from_raw()
 *
 * This yields a buffer with the same properties as the one returned
 * by _nx_buffer_coalesce().
 */
int _nx_buffer_create_from_raw(nexus_byte_t *raw_buffer,
			       unsigned long raw_size,
			       struct _nexus_buffer_t **buffer)
{
    LockFreelists();
    *buffer = buffer_alloc();
    UnlockFreelists();
    NexusMalloc(_nx_buffer_create_from_raw(),
		(*buffer)->base_segments,
		nexus_base_segment_t *,
		sizeof(nexus_base_segment_t));
    (*buffer)->base_segments->next = (nexus_base_segment_t *) NULL;
    (*buffer)->base_segments->size = raw_size;
    (*buffer)->base_segments->size_used = raw_size;
    (*buffer)->base_segments->storage = raw_buffer;
    (*buffer)->base_segments->current = raw_buffer;
    (*buffer)->base_segments->storage_is_inline = NEXUS_FALSE;

    (*buffer)->current_base_segment = (*buffer)->base_segments;
    (*buffer)->reserved_header_size = 0;
    (*buffer)->direct_segments = (nexus_direct_segment_t *) NULL;
    (*buffer)->n_direct = 0;

    return(0);
} /* _nx_buffer_create_from_raw() */


/*
 * call_nexus_unknown_handler()
 *
 * TODO: Allow this to be a threaded handler
 */
static void call_nexus_unknown_handler(nexus_endpoint_t *endpoint,
				       struct _nexus_buffer_t *buffer,
				       int handler_id)
{
    nexus_unknown_handler_func_t NexusUnknownHandler;
    nexus_context_t *save_context;

    NexusUnknownHandler = endpoint->unknown_handler;

    nexus_debug_printf(1, ("call_nexus_unknown_handler(): Handler[%d] not found. %s\n", handler_id, (NexusUnknownHandler ? "Calling unknown handler" : "Ignoring (unknown handler not registered)")));

    if (NexusUnknownHandler)
    {
	_nx_context(&save_context);
	_nx_set_context(endpoint->context);
	(*NexusUnknownHandler)(endpoint, &buffer, handler_id);
	_nx_set_context(save_context);
    }
} /* call_nexus_unknown_handler() */


/*
 * _nx_buffer_dispatch()
 *
 * TODO: Add checks to make sure the buffer is big enough, has
 * valid data, etc.
 */
int _nx_buffer_dispatch(struct _nexus_buffer_t *buffer)
{
    nexus_byte_t *		b;
    nexus_byte_t		tmp_byte;
    unsigned long		message_size;
    nexus_byte_t		flags;
    nexus_bool_t		has_direct_info;
    nexus_bool_t		has_other_info;
    nexus_byte_t		pad_size;
    int				liba_size;
    nexus_byte_t		liba[NEXUS_MAX_LIBA_SIZE];
    nexus_endpoint_t *		endpoint;
    nexus_transformstate_t *	transform_state;
    int				handler_id;
    unsigned long		direct_info_offset;
    unsigned long		other_info_offset;
    nexus_byte_t *		direct_info;
    nexus_handler_func_t	handler;
    unsigned long		header_size;
    unsigned long		data_size;
    unsigned long		save_header_size;
    unsigned long		untransform_size;
    unsigned long		out_size;
    unsigned long		out_header_size;
    unsigned long		out_data_size;
    nexus_bool_t		out_must_be_freed;
    unsigned long		ul;
    nexus_context_t *		save_context;

    /* Unpack the header */
    b = buffer->base_segments->current;
    buffer->format = (int) (*b++);
    nexus_dc_get_u_long(&b, &message_size, 1, buffer->format);
    NexusAssert2((message_size == buffer->base_segments->size_used), ("_nx_buffer_dispatch(): Buffer size (%li) does not match header size (%li)", (buffer->base_segments->size - buffer->reserved_header_size), message_size));
    nexus_dc_get_byte(&b, &flags, 1, buffer->format);
    has_direct_info = flags & 0x10;
    has_other_info = flags & 0x08;
    pad_size = flags & 0x07;
    nexus_dc_get_byte(&b, &tmp_byte, 1, buffer->format);
    liba_size = (int) tmp_byte;
    NexusAssert2((liba_size <= NEXUS_MAX_LIBA_SIZE), ("_nx_buffer_dispatch(): Liba size (%i) is greater than NEXUS_MAX_LIBA_SIZE (%i)", liba_size, NEXUS_MAX_LIBA_SIZE));
    nexus_dc_get_byte(&b, liba, liba_size, buffer->format);
    /* TODO: validate liba */
    /* TODO: get out the transform state */
    LibaUnpackEndpoint(liba, ul);
    endpoint = (nexus_endpoint_t *) ul;
    transform_state = (nexus_transformstate_t *) NULL;
    b += pad_size;

    /* Untransform the message */
    if (endpoint->transform_id != NEXUS_TRANSFORM_NONE)
    {
	/*
	nexus_buffer_untransform(buffer,
				 transform_state,
				 message_size,
				 header_size,
				 data_size,
				 save_header_size,
				 NEXUS_TRUE,
				 buffer->format,
				 untransform_size,
				 &b,
				 &out_size,
				 &out_header_size,
				 &out_data_size,
				 &out_must_be_freed);
	 */
    }

    /* Unpack the message info */
    nexus_dc_get_int(&b, &handler_id, 1, buffer->format);
    if (has_direct_info)
    {
        nexus_dc_get_u_long(&b,
			    &direct_info_offset,
			    1,
			    buffer->format);
	direct_info = b + direct_info_offset;
	/* TODO: Parse direct info header */
    }
    else
    {
	buffer->n_direct = 0;
    }
    if (has_other_info)
    {
        nexus_dc_get_u_long(&b,
			    &other_info_offset,
			    1,
			    buffer->format);
    }

    buffer->base_segments->current = b;
    buffer->reserved_header_size = (b - buffer->base_segments->storage);

    if (handler_id < 0)
    {
	/* This is a system handler */
	switch (handler_id)
	{
	case NEXUS_CONTEXT_CREATE_HANDLER_ID:
	    /* TODO: Check endpoint to see if it support context creation */
	    _nx_context(&save_context);
	    _nx_set_context(endpoint->context);
	    _nx_context_create_handler(endpoint, &buffer, NEXUS_TRUE);
	    _nx_set_context(save_context);
	    break;
	default:
	    call_nexus_unknown_handler(endpoint, buffer, handler_id);
	    break;
	}
    }
    else if (   (handler_id > endpoint->handler_table_size)
	     || !(endpoint->handler_table[handler_id].func) )
    {
	call_nexus_unknown_handler(endpoint, buffer, handler_id);
    }
    else
    {
        /* Call the handler */
        handler = endpoint->handler_table[handler_id].func;
#ifndef BUILD_LITE
        if (endpoint->handler_table[handler_id].type
	    == NEXUS_HANDLER_TYPE_THREADED)
	{
	    threaded_handler_startup_info_t *info;
	    nexus_thread_t thread;

	    NexusMalloc(_nx_buffer_dispatch(),
			info,
			threaded_handler_startup_info_t *,
			sizeof(threaded_handler_startup_info_t));
	    info->context = endpoint->context;
	    info->endpoint = endpoint;
#ifdef BUILD_PROFILE
	    info->node_id = node_id;
	    info->context_id = context_id;
	    info->message_length = message_length;
#endif
	    nexus_buffer_save(&buffer);
	    buffer->saved_state = NEXUS_BUFFER_SAVED_STATE_SAVED;
	    info->buffer = buffer;
	    info->func = handler;

	    nexus_thread_create(&thread,
				(nexus_thread_attr_t *) NULL,
				threaded_handler_startup,
				(void *) info);
	}
	else /* type == NEXUS_HANDLER_TYPE_NON_THREADED */
#endif
	{
	    _nx_context(&save_context);
	    _nx_set_context(endpoint->context);
	    (*handler)(endpoint, &buffer, NEXUS_TRUE);
	    _nx_set_context(save_context);

	    LockFreelists();
	    if (   (buffer->saved_state == NEXUS_BUFFER_SAVED_STATE_FREED)
		|| (buffer->saved_state == NEXUS_BUFFER_SAVED_STATE_UNSAVED))
	    {
		base_segments_free(buffer->base_segments);
		buffer_free(buffer);
	    }
	    else if (buffer->saved_state
		     == NEXUS_BUFFER_SAVED_STATE_SAVED_IN_HANDLER)
	    {
		buffer->saved_state = NEXUS_BUFFER_SAVED_STATE_SAVED;
	    }
	    UnlockFreelists();
	}
    }
    
    return(0);
} /* _nx_buffer_dispatch() */


/*
 * nexus_buffer_save()
 */
int nexus_buffer_save(nexus_buffer_t *buffer)
{
    LockFreelists();
    if ((*buffer)->saved_state == NEXUS_BUFFER_SAVED_STATE_UNSAVED)
    {
	(*buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_SAVED_IN_HANDLER;
	/* TODO: Save direct segments */
    }
    UnlockFreelists();
    return(0);
} /* nexus_buffer_save() */


/*
 * nexus_buffer_save_linearly()
 */
int nexus_buffer_save_linearly(nexus_buffer_t *buffer)
{
    LockFreelists();
    if ((*buffer)->saved_state == NEXUS_BUFFER_SAVED_STATE_UNSAVED)
    {
	(*buffer)->saved_state = NEXUS_BUFFER_SAVED_STATE_SAVED_IN_HANDLER;
	/* TODO: Save direct segments */
    }
    UnlockFreelists();
    return(0);
} /* nexus_buffer_save_linearly() */


#ifndef BUILD_LITE
/*
 * threaded_handler_startup()
 *
 * Startup a threaded handler.  The info about the handler is in arg.
 */
static void *threaded_handler_startup(void *arg)
{
    threaded_handler_startup_info_t *info;
    nexus_context_t *context;
    nexus_endpoint_t *endpoint;
    nexus_buffer_t buffer;
    nexus_handler_func_t func;
#ifdef BUILD_PROFILE
    int node_id;
    int context_id;
    int message_length;
#endif

    info = (threaded_handler_startup_info_t *) arg;
    context = info->context;
    endpoint = info->endpoint;
#ifdef BUILD_PROFILE
    node_id = info->node_id;
    context_id = inf->context_id;
    message_length = info->message_length;
#endif
    buffer = info->buffer;
    func = info->func;
    NexusFree(info);
    
    _nx_set_context(context);

    (*func)(endpoint, &buffer, NEXUS_FALSE);

    return(NULL);
} /* threaded_handler_startup() */
#endif /* BUILD_LITE */


/*
 * buffer_alloc()
 */
static nexus_buffer_t buffer_alloc()
{
    nexus_buffer_t buffer;
    if (buffer_freelist_head)
    {
        buffer = buffer_freelist_head;
	buffer_freelist_head = buffer_freelist_head->next;
    }
    else
    {
        NexusMalloc(buffer_alloc(),
		    buffer,
		    nexus_buffer_t,
		    sizeof(struct _nexus_buffer_t));
    }
    buffer->funcs = NULL;
    buffer->next = NULL;
    buffer->saved_state = NEXUS_BUFFER_SAVED_STATE_UNSAVED;
    buffer->base_segments = NULL;
    buffer->current_base_segment = NULL;
    buffer->direct_segments = NULL;
    buffer->current_direct_segment = NULL;
    buffer->n_direct = 0;
    return(buffer);
} /* buffer_alloc() */


/*
 * buffer_free()
 */
static void buffer_free(nexus_buffer_t buffer)
{
    buffer->next = buffer_freelist_head;
    buffer_freelist_head = buffer;
} /* buffer_free() */


/*
 * base_segment_malloc()
 */
static nexus_base_segment_t *base_segment_malloc(unsigned long size)
{
    nexus_base_segment_t *segment;
    nexus_byte_t *b;
    NexusMalloc(base_segment_malloc(),
		b,
		nexus_byte_t *,
		(sizeof_base_segment + size));
    segment = (nexus_base_segment_t *) b;
    segment->size = size;
    segment->storage = (b + sizeof_base_segment);
    segment->storage_is_inline = NEXUS_TRUE;
    return(segment);
} /* base_segment_mallco() */


/*
 * direct_segment_malloc()
 */
static nexus_direct_segment_t *direct_segment_malloc(unsigned long size)
{
    nexus_direct_segment_t *segment;
    nexus_byte_t *b;
    NexusMalloc(direct_segment_malloc(),
		b,
		nexus_byte_t *,
		(sizeof_direct_segment
		 + (size * sizeof(nexus_direct_info_t))));
    segment = (nexus_direct_segment_t *) b;
    segment->size = size;
    segment->storage = (nexus_direct_info_t *) (b + sizeof_direct_segment);
    return(segment);
} /* direct_segment_malloc() */


/*
 * base_segment_alloc()
 */
static void base_segment_alloc(nexus_buffer_t buffer,
			       unsigned long size)
{
    nexus_base_segment_t *tmp;
    if (   size <= default_base_segment_size
	&& base_freelist_head)
    {
	/* Get a default size base segment from the freelist */
        tmp = base_freelist_head;
	base_freelist_head = base_freelist_head->next;
    }
    else
    {
	tmp = base_segment_malloc(NEXUS_MAX(size, default_base_segment_size));
    }
    tmp->next = NULL;
    tmp->current = tmp->storage;
    if (buffer->current_base_segment)
    {
	buffer->current_base_segment->next = tmp;
    }
    else
    {
	buffer->base_segments = tmp;
    }
    buffer->current_base_segment = tmp;
} /* base_segment_alloc() */


/*
 * base_segments_free()
 */
static void base_segments_free(nexus_base_segment_t *base_segments)
{
    nexus_base_segment_t *segment, *tmp;
    for (segment = base_segments;
	 segment;
	 segment = tmp)
    {
	tmp = segment->next;
        if (segment->storage_is_inline)
	{
	    if (segment->size != default_base_segment_size)
	    {
		NexusFree(segment);
	    }
	    else
	    {
		segment->next = base_freelist_head;
		base_freelist_head = segment;
	    }
	}
	else
	{
	    NexusFree(segment->storage);
	    NexusFree(segment);
	}
    }
} /* base_segments_free() */


/*
 * direct_segment_alloc()
 */
static void direct_segment_alloc(nexus_buffer_t buffer,
				 unsigned long size)
{
    nexus_direct_segment_t *tmp;
    if (   size <= default_direct_segment_size
	&& direct_freelist_head)
    {
	/* Get a default size direct segment from the freelist */
        tmp = direct_freelist_head;
	direct_freelist_head = direct_freelist_head->next;
    }
    else
    {
	tmp = direct_segment_malloc(NEXUS_MAX(size,
					      default_direct_segment_size));
    }
    tmp->next = NULL;
    tmp->n_left = tmp->size;
    tmp->current = tmp->storage;
    if (buffer->current_direct_segment)
    {
	buffer->current_direct_segment->next = tmp;
    }
    else
    {
	buffer->direct_segments = tmp;
    }
    buffer->current_direct_segment = tmp;
} /* direct_segment_alloc() */


/*
 * direct_segments_free()
 */
static void direct_segments_free(nexus_direct_segment_t *direct_segments)
{
    nexus_direct_segment_t *segment, *tmp;
    for (segment = direct_segments;
	 segment;
	 segment = tmp)
    {
	tmp = segment->next;
        if (segment->size != default_direct_segment_size)
	{
	    NexusFree(segment);
	}
	else
	{
	    segment->next = direct_freelist_head;
	    direct_freelist_head = segment;
	}
    }
} /* direct_segments_free() */
