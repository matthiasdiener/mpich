/*
 * put_get.c
 *
 * nexus_put_*(), nexus_get_*(), and nexus_sizeof_*() code.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/put_get.c,v 1.2 1996/11/28 00:21:05 tuecke Exp $";

#include "internal.h"

/********************************************************************
 * nexus_put_*()
 ********************************************************************/

/*
 * put_assertions()
 */
static void put_assertions(nexus_buffer_t *buffer,
			   nexus_byte_t *data,
			   int count,
			   int datatype,
			   int sizeof_datatype)
{
    char tmp[50];
    char output[1024];
    
    /* check buffer magic */
    if ((*buffer)->current_base_segment->current - (*buffer)->current_base_segment->storage < count * sizeof_datatype)
    {
      /* error!!! */
    }
    /* at medium debug: */
    tmp[0] = '\0';
    strcat(tmp, "put type: ");
    switch(datatype)
    {
      case NEXUS_DC_DATATYPE_BYTE:
	strcat(tmp, "byte");
	break;
      case NEXUS_DC_DATATYPE_CHAR:
	strcat(tmp, "char");
	break;
      case NEXUS_DC_DATATYPE_U_CHAR:
	strcat(tmp, "unsigned char");
	break;
      case NEXUS_DC_DATATYPE_SHORT:
	strcat(tmp, "short");
	break;
      case NEXUS_DC_DATATYPE_U_SHORT:
	strcat(tmp, "unsigned short");
	break;
      case NEXUS_DC_DATATYPE_INT:
	strcat(tmp, "int");
	break;
      case NEXUS_DC_DATATYPE_U_INT:
	strcat(tmp, "unsigned int");
	break;
      case NEXUS_DC_DATATYPE_LONG:
	strcat(tmp, "long");
	break;
      case NEXUS_DC_DATATYPE_U_LONG:
	strcat(tmp, "unsigned long");
	break;
      case NEXUS_DC_DATATYPE_FLOAT:
	strcat(tmp, "float");
	break;
      case NEXUS_DC_DATATYPE_DOUBLE:
	strcat(tmp, "double");
	break;
      default: /* error!!! */
	break;
    }
    nexus_stdio_lock();
    sprintf(output,
	    tmp, "%s %s:%lx %s:%lx %s:%lx %s:%d %s:%lx %s:%lx %s:%lx %s:%d\n",
	    "data",
	    (unsigned long) data,
	    "base segment",
	    (unsigned long) (*buffer)->base_segments,
	    "current segment",
	    (unsigned long) (*buffer)->current_base_segment,
	    "current size",
	    (*buffer)->current_base_segment->size,
	    "current storage",
	    (unsigned long) (*buffer)->current_base_segment->storage,
	    "current location",
	    (unsigned long) (*buffer)->current_base_segment->current,
	    "limit",
	    (unsigned long) ((*buffer)->current_base_segment->storage
			     + (*buffer)->current_base_segment->size),
	    "count",
	    count);
    nexus_stdio_unlock();
} /* put_assertions() */


/*
 * nexus_put_*()
 */
void nexus_put_float(nexus_buffer_t *buffer,
		     float *data,
		     int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_FLOAT,
		   nexus_dc_sizeof_float(1));
#endif
    nexus_dc_put_float(&((*(buffer))->current_base_segment->current),
		       data,
		       count);
}

void nexus_put_double(nexus_buffer_t *buffer,
		      double *data,
		      int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_DOUBLE,
		   nexus_dc_sizeof_double(1));
#endif
    nexus_dc_put_double(&((*(buffer))->current_base_segment->current),
			data,
			count);
}

void nexus_put_short(nexus_buffer_t *buffer,
		     short *data,
		     int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_SHORT,
		   nexus_dc_sizeof_short(1));
#endif
    nexus_dc_put_short(&((*(buffer))->current_base_segment->current),
		       data,
		       count);
}

void nexus_put_u_short(nexus_buffer_t *buffer,
		       unsigned short *data,
		       int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_U_SHORT,
		   nexus_dc_sizeof_u_short(1));
#endif
    nexus_dc_put_u_short(&((*(buffer))->current_base_segment->current),
			 data,
			 count);
}

void nexus_put_int(nexus_buffer_t *buffer,
		   int *data,
		   int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_INT,
		   nexus_dc_sizeof_int(1));
#endif
    nexus_dc_put_int(&((*(buffer))->current_base_segment->current),
		     data,
		     count);
}

void nexus_put_u_int(nexus_buffer_t *buffer,
		     unsigned int *data,
		     int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_U_INT,
		   nexus_dc_sizeof_u_int(1));
#endif
    nexus_dc_put_u_int(&((*(buffer))->current_base_segment->current),
		       data,
		       count);
}

void nexus_put_long(nexus_buffer_t *buffer,
		    long *data,
		    int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_LONG,
		   nexus_dc_sizeof_long(1));
#endif
    nexus_dc_put_long(&((*(buffer))->current_base_segment->current),
		      data,
		      count);
}

void nexus_put_u_long(nexus_buffer_t *buffer,
		      unsigned long *data,
		      int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_U_LONG,
		   nexus_dc_sizeof_u_long(1));
#endif
    nexus_dc_put_u_long(&((*(buffer))->current_base_segment->current),
			data,
			count);
}

void nexus_put_char(nexus_buffer_t *buffer,
		    char *data,
		    int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_CHAR,
		   nexus_dc_sizeof_char(1));
#endif
    nexus_dc_put_char(&((*(buffer))->current_base_segment->current),
		      data,
		      count);
}

void nexus_put_u_char(nexus_buffer_t *buffer,
		      unsigned char *data,
		      int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_U_CHAR,
		   nexus_dc_sizeof_u_char(1));
#endif
    nexus_dc_put_u_char(&((*(buffer))->current_base_segment->current),
			data,
			count);
}

void nexus_put_byte(nexus_buffer_t *buffer,
		    nexus_byte_t *data,
		    int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_BYTE,
		   nexus_dc_sizeof_byte(1));
#endif
    nexus_dc_put_byte(&((*(buffer))->current_base_segment->current),
		      data,
		      count);
}

void nexus_put_user(nexus_buffer_t *buffer,
		    nexus_byte_t *data,
		    int count)
{
#ifdef BUILD_DEBUG
    put_assertions(buffer,
		   (nexus_byte_t *) data,
		   count,
		   NEXUS_DC_DATATYPE_BYTE,
		   nexus_dc_sizeof_byte(1));
#endif
    nexus_dc_put_byte(&((*(buffer))->current_base_segment->current),
		      data,
		      count);
}


/********************************************************************
 * nexus_get_*()
 ********************************************************************/

/*
 * get_assertions()
 */
static void get_assertions(nexus_buffer_t *buffer,
			   nexus_byte_t *dest,
			   int count,
			   int datatype,
			   int sizeof_datatype)
{
    char tmp[50];
    char output[1024];
    
    /* check buffer magic */
    if ((*buffer)->current_base_segment->current - (*buffer)->current_base_segment->storage < count * sizeof_datatype)
    {
      /* error!!! */
    }
    /* at medium debug: */
    tmp[0] = '\0';
    strcat(tmp, "get type: ");
    switch(datatype)
    {
      case NEXUS_DC_DATATYPE_BYTE:
	strcat(tmp, "byte");
	break;
      case NEXUS_DC_DATATYPE_CHAR:
	strcat(tmp, "char");
	break;
      case NEXUS_DC_DATATYPE_U_CHAR:
	strcat(tmp, "unsigned char");
	break;
      case NEXUS_DC_DATATYPE_SHORT:
	strcat(tmp, "short");
	break;
      case NEXUS_DC_DATATYPE_U_SHORT:
	strcat(tmp, "unsigned short");
	break;
      case NEXUS_DC_DATATYPE_INT:
	strcat(tmp, "int");
	break;
      case NEXUS_DC_DATATYPE_U_INT:
	strcat(tmp, "unsigned int");
	break;
      case NEXUS_DC_DATATYPE_LONG:
	strcat(tmp, "long");
	break;
      case NEXUS_DC_DATATYPE_U_LONG:
	strcat(tmp, "unsigned long");
	break;
      case NEXUS_DC_DATATYPE_FLOAT:
	strcat(tmp, "float");
	break;
      case NEXUS_DC_DATATYPE_DOUBLE:
	strcat(tmp, "double");
	break;
      default:
	/* error!!! */
	break;
    }
    nexus_stdio_lock();
    sprintf(output,
	    "%s %s:%lx %s:%lx %s:%lx %s:%d %s:%lx %s:%lx %s:%lx %s:%d\n",
	    tmp,
	    "data",
	    (unsigned long ) dest,
	    "base segment",
	    (unsigned long) (*buffer)->base_segments,
	    "current segment",
	    (unsigned long) (*buffer)->current_base_segment,
	    "current size",
	    (*buffer)->current_base_segment->size,
	    "current storage",
	    (unsigned long) (*buffer)->current_base_segment->storage,
	    "current location",
	    (unsigned long) (*buffer)->current_base_segment->current,
	    "limit",
	    (unsigned long) ((*buffer)->current_base_segment->storage
		      + (*buffer)->current_base_segment->size),
	    "count",
	    count);
    nexus_stdio_unlock();
} /* get_assertions() */


/*
 * nexus_get_*()
 */
void nexus_get_float(nexus_buffer_t *buffer,
		     float *dest,
		     int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_FLOAT,
		   nexus_dc_sizeof_float(1));
#endif
    nexus_dc_get_float(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_double(nexus_buffer_t *buffer,
		      double *dest,
		      int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_DOUBLE,
		   nexus_dc_sizeof_double(1));
#endif
    nexus_dc_get_double(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_short(nexus_buffer_t *buffer,
		     short *dest,
		     int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_SHORT,
		   nexus_dc_sizeof_short(1));
#endif
    nexus_dc_get_short(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_u_short(nexus_buffer_t *buffer,
		       unsigned short *dest,
		       int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_U_SHORT,
		   nexus_dc_sizeof_u_short(1));
#endif
    nexus_dc_get_u_short(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_int(nexus_buffer_t *buffer,
		   int *dest,
		   int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_INT,
		   nexus_dc_sizeof_int(1));
#endif
    nexus_dc_get_int(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_u_int(nexus_buffer_t *buffer,
		     unsigned int *dest,
		     int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_U_INT,
		   nexus_dc_sizeof_u_int(1));
#endif
    nexus_dc_get_u_int(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_long(nexus_buffer_t *buffer,
		    long *dest,
		    int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_LONG,
		   nexus_dc_sizeof_long(1));
#endif
    nexus_dc_get_long(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_u_long(nexus_buffer_t *buffer,
		      unsigned long *dest,
		      int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_U_LONG,
		   nexus_dc_sizeof_u_long(1));
#endif
    nexus_dc_get_u_long(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_char(nexus_buffer_t *buffer,
		    char *dest,
		    int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_CHAR,
		   nexus_dc_sizeof_char(1));
#endif
    nexus_dc_get_char(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_u_char(nexus_buffer_t *buffer,
		      unsigned char *dest,
		      int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_U_CHAR,
		   nexus_dc_sizeof_u_char(1));
#endif
    nexus_dc_get_u_char(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_byte(nexus_buffer_t *buffer,
		    nexus_byte_t *dest,
		    int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_BYTE,
		   nexus_dc_sizeof_byte(1));
#endif
    nexus_dc_get_byte(&((*(buffer))->current_base_segment->current),
		       dest,
		       count,
		       (*(buffer))->format);
}

void nexus_get_user(nexus_buffer_t *buffer,
		    nexus_byte_t *dest,
		    int count)
{
#ifdef BUILD_DEBUG
    get_assertions(buffer,
		   (nexus_byte_t *) dest,
		   count,
		   NEXUS_DC_DATATYPE_BYTE,
		   nexus_dc_sizeof_byte(1));
#endif
    nexus_dc_get_byte(&((*(buffer))->current_base_segment->current),
		      dest,
		      count,
		      (*(buffer))->format);
}


/********************************************************************
 * nexus_sizeof_*()
 ********************************************************************/

#undef nexus_sizeof_float
int nexus_sizeof_float(int count)
{
    return(nexus_macro_sizeof_float(count));
}

#undef nexus_sizeof_double
int nexus_sizeof_double(int count)
{
    return(nexus_macro_sizeof_double(count));
}

#undef nexus_sizeof_short
int nexus_sizeof_short(int count)
{
    return(nexus_macro_sizeof_short(count));
}

#undef nexus_sizeof_u_short
int nexus_sizeof_u_short(int count)
{
    return(nexus_macro_sizeof_u_short(count));
}

#undef nexus_sizeof_int
int nexus_sizeof_int(int count)
{
    return(nexus_macro_sizeof_int(count));
}

#undef nexus_sizeof_u_int
int nexus_sizeof_u_int(int count)
{
    return(nexus_macro_sizeof_u_int(count));
}

#undef nexus_sizeof_long
int nexus_sizeof_long(int count)
{
    return(nexus_macro_sizeof_long(count));
}

#undef nexus_sizeof_u_long
int nexus_sizeof_u_long(int count)
{
    return(nexus_macro_sizeof_u_long(count));
}

#undef nexus_sizeof_char
int nexus_sizeof_char(int count)
{
    return(nexus_macro_sizeof_char(count));
}

#undef nexus_sizeof_u_char
int nexus_sizeof_u_char(int count)
{
    return(nexus_macro_sizeof_u_char(count));
}

#undef nexus_sizeof_byte
int nexus_sizeof_byte(int count)
{
    return(nexus_macro_sizeof_byte(count));
}

