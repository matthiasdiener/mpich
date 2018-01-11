/*
 * put_get_direct.c
 *
 * nexus_direct_put_*() and nexus_direct_get_*() code
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/put_get_direct.c,v 1.3 1996/11/28 00:21:05 tuecke Exp $";

#include "internal.h"


/********************************************************************
 * nexus_direct_put_*()
 ********************************************************************/

/*
 * _nx_direct_put()
 *
 * All of the nexus_direct_put_DATATYPE() call redirect through this one.
 */
void _nx_direct_put(nexus_buffer_t *buffer,
		    void *data,
		    unsigned long count,
		    int datatype,
		    int sizeof_datatype)
{
    nexus_direct_info_t *direct_info;

    /*
     * Do assertions and debug checks
     *
     * check (*buffer)->current_base_segment->n_left
     */

    /* Fill in the next direct_info slot */
    direct_info = (*buffer)->current_direct_segment->current++;
    (*buffer)->current_direct_segment->n_left--;
    direct_info->datatype = (datatype);
    direct_info->size = (count * (sizeof_datatype));
    direct_info->data = data;
    direct_info->action = NEXUS_DIRECT_INFO_ACTION_INLINE;
} /* _nx_direct_put() */
    

#undef nexus_direct_put_float
void nexus_direct_put_float(nexus_buffer_t *buffer,
			    float *data,
			    int count)
{
    nexus_macro_direct_put_float(buffer, data, count);
}

#undef nexus_direct_put_double
void nexus_direct_put_double(nexus_buffer_t *buffer,
			     double *data,
			     int count)
{
    nexus_macro_direct_put_double(buffer, data, count);
}

#undef nexus_direct_put_short
void nexus_direct_put_short(nexus_buffer_t *buffer,
			    short *data,
			    int count)
{
    nexus_macro_direct_put_short(buffer, data, count);
}

#undef nexus_direct_put_u_short
void nexus_direct_put_u_short(nexus_buffer_t *buffer,
			      unsigned short *data,
			      int count)
{
    nexus_macro_direct_put_u_short(buffer, data, count);
}

#undef nexus_direct_put_int
void nexus_direct_put_int(nexus_buffer_t *buffer,
			  int *data,
			  int count)
{
    nexus_macro_direct_put_int(buffer, data, count);
}

#undef nexus_direct_put_u_int
void nexus_direct_put_u_int(nexus_buffer_t *buffer,
			    unsigned int *data,
			    int count)
{
    nexus_macro_direct_put_u_int(buffer, data, count);
}

#undef nexus_direct_put_long
void nexus_direct_put_long(nexus_buffer_t *buffer,
			   long *data,
			   int count)
{
    nexus_macro_direct_put_long(buffer, data, count);
}

#undef nexus_direct_put_u_long
void nexus_direct_put_u_long(nexus_buffer_t *buffer,
			     unsigned long *data,
			     int count)
{
    nexus_macro_direct_put_u_long(buffer, data, count);
}

#undef nexus_direct_put_char
void nexus_direct_put_char(nexus_buffer_t *buffer,
			   char *data,
			   int count)
{
    nexus_macro_direct_put_char(buffer, data, count);
}

#undef nexus_direct_put_u_char
void nexus_direct_put_u_char(nexus_buffer_t *buffer,
			     unsigned char *data,
			     int count)
{
    nexus_macro_direct_put_u_char(buffer, data, count);
}

#undef nexus_direct_put_byte
void nexus_direct_put_byte(nexus_buffer_t *buffer,
			   unsigned char *data,
			   int count)
{
    nexus_macro_direct_put_byte(buffer, data, count);
}

#undef nexus_direct_put_user
void nexus_direct_put_user(nexus_buffer_t *buffer,
			   unsigned char *data,
			   int count)
{
    nexus_macro_direct_put_user(buffer, data, count);
}


/********************************************************************
 * nexus_direct_get_*()
 ********************************************************************/

/*
 * direct_get()
 *
 * Get the next direct component.
 */
static int direct_get(nexus_buffer_t *buffer,
		      nexus_byte_t *dest,
		      int count,
		      int n_bytes)
{
    int rc;
    unsigned long component_size;
    int component_datatype;
    int component_approach;
    unsigned long component_location;
    nexus_byte_t *component_pointer;
    nexus_byte_t b;
    unsigned long ul;
    int format = (*buffer)->format;

    /*
     * Verify:
     *   - that there is another direct component
     *   - that n_bytes is the same as the next direct component size
     *   - etc
     */
    nexus_dc_get_u_long(&(*buffer)->direct_info, &component_size, 1, format);
    nexus_dc_get_byte(&(*buffer)->direct_info, &b, 1, format);
    component_datatype = (int) b;
    nexus_dc_get_byte(&(*buffer)->direct_info, &b, 1, format);
    component_approach = (int) b;
    
    switch(component_approach)
    {
      case NEXUS_DIRECT_INFO_ACTION_INLINE:
	nexus_dc_get_u_long(&(*buffer)->direct_info,
			    &component_location,
			    1,
			    format);
	memcpy(dest,
	       ((*buffer)->current_base_segment->storage + component_location),
	       n_bytes);
	break;
      case NEXUS_DIRECT_INFO_ACTION_POINTER:
	nexus_dc_get_u_long(&(*buffer)->direct_info,
			    &ul,
			    1,
			    format);
	component_pointer = (nexus_byte_t *) ul;
	memcpy(dest,
	       component_pointer,
	       n_bytes);
	break;
      default:
	nexus_dc_get_u_long(&(*buffer)->direct_info,
			    &ul,
			    1,
			    format);
	rc = ((*buffer)->funcs->direct_get)(dest,
					    n_bytes,
					    component_approach,
					    ul);
	break;
    }

    return(0);
} /* direct_get() */


/*
 * NexusDirectGet()
 *
 * Macro for nexus_direct_get_*(), since type name is the only difference
 * between these functions.
 */
#define NexusDirectGet(Type, Buffer, Dest, Count) \
{ \
    int rc; \
    int n_bytes; \
    int format = ((*Buffer))->format; \
    n_bytes = nexus_dc_sizeof_remote_ ## Type (Count, format); \
    if (nexus_dc_is_native_ ## Type (format)) \
    { \
	rc = direct_get(Buffer, \
			(nexus_byte_t *) (Dest), \
			Count, \
			n_bytes); \
    } \
    else \
    { \
	nexus_byte_t *tmp_dest; \
	NexusMalloc(nexus_direct_get_ ## Type (), \
		    tmp_dest, \
		    nexus_byte_t *, \
		    n_bytes); \
	rc = direct_get(Buffer, tmp_dest, Count, n_bytes); \
	if (rc == 0) \
	{ \
	    nexus_dc_get_ ## Type (&tmp_dest, Dest, Count, format); \
	} \
	NexusFree(tmp_dest); \
    } \
} /* NexusDirectGet() */

void nexus_direct_get_float(nexus_buffer_t *buffer,
			    float *dest,
			    int count)
{
    NexusDirectGet(float, buffer, dest, count);
}


void nexus_direct_get_double(nexus_buffer_t *buffer,
			     double *dest,
			     int count)
{
    NexusDirectGet(double, buffer, dest, count);
}

void nexus_direct_get_short(nexus_buffer_t *buffer,
			    short *dest,
			    int count)
{
    NexusDirectGet(short, buffer, dest, count);
}

void nexus_direct_get_u_short(nexus_buffer_t *buffer,
			      unsigned short *dest,
			      int count)
{
    NexusDirectGet(u_short, buffer, dest, count);
}

void nexus_direct_get_int(nexus_buffer_t *buffer,
			  int *dest,
			  int count)
{
    NexusDirectGet(int, buffer, dest, count);
}

void nexus_direct_get_u_int(nexus_buffer_t *buffer,
			    unsigned int *dest,
			    int count)
{
    NexusDirectGet(u_int, buffer, dest, count);
}

void nexus_direct_get_long(nexus_buffer_t *buffer,
			   long *dest,
			   int count)
{
    NexusDirectGet(long, buffer, dest, count);
}

void nexus_direct_get_u_long(nexus_buffer_t *buffer,
			     unsigned long *dest,
			     int count)
{
    NexusDirectGet(u_long, buffer, dest, count);
}

void nexus_direct_get_char(nexus_buffer_t *buffer,
			   char *dest,
			   int count)
{
    NexusDirectGet(char, buffer, dest, count);
}

void nexus_direct_get_u_char(nexus_buffer_t *buffer,
			     unsigned char *dest,
			     int count)
{
    NexusDirectGet(u_char, buffer, dest, count);
}

void nexus_direct_get_byte(nexus_buffer_t *buffer,
			   unsigned char *dest,
			   int count)
{
    NexusDirectGet(byte, buffer, dest, count);
}

void nexus_direct_get_user(nexus_buffer_t *buffer,
			   unsigned char *dest,
			   int count)
{
    NexusDirectGet(byte, buffer, dest, count);
}

