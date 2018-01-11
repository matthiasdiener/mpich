/*
 * nexus_dc_32be.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_dc/nexus_dc_32be.c,v 1.3 1996/12/08 19:19:29 tuecke Exp $";

#include "nexus_dc.h"

/*
 * nexus_dc_is_native_char()
 */
int nexus_dc_is_native_char(int format)
{
    if (format == NEXUS_DC_FORMAT_JAVA)
	return(NEXUS_FALSE);
    else
	return(NEXUS_TRUE);
} /* nexus_dc_is_native_char() */


/*
 * nexus_dc_get_char()
 */
void nexus_dc_get_char(nexus_byte_t **buffer,
		       char *data,
		       unsigned long count,
		       int format)
{
    if (format == NEXUS_DC_FORMAT_JAVA)
    {
	nexus_byte_t *end = *buffer + count * 2;
	nexus_byte_t *tmp = (nexus_byte_t *) data;
        while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 1);
	    *buffer += 2;
	}
    }
    else
    {
        memcpy(data, *buffer, count);
	*buffer += count;
    }
} /* nexus_dc_get_char() */


/*
 * nexus_dc_check_lost_precision_char()
 */
int nexus_dc_check_lost_precision_char(nexus_byte_t *buffer,
				       unsigned long count,
				       int format)
{
    int result = -1;
    if (format == NEXUS_DC_FORMAT_JAVA)
    {
	nexus_byte_t *place = buffer;
	int i;
	for (i = 0; i < count; i++)
	{
	    if (*place)
	    {
		result = i;
		break;
	    }
	    place += 2;
	}
    }
    return(result);
} /* nexus_dc_check_lost_precision_char() */


/*
 * nexus_dc_is_native_u_char()
 */
int nexus_dc_is_native_u_char(int format)
{
    if (format == NEXUS_DC_FORMAT_JAVA)
	return(NEXUS_FALSE);
    else
	return(NEXUS_TRUE);
} /* nexus_dc_is_native_u_char() */



/*
 * nexus_dc_is_native_u_char()
 */
void nexus_dc_get_u_char(nexus_byte_t **buffer,
			 unsigned char *data,
			 unsigned long count,
			 int format)
{
    if (format == NEXUS_DC_FORMAT_JAVA)
    {
	nexus_byte_t *end = *buffer + count * 2;
	nexus_byte_t *tmp = (nexus_byte_t *) data;
        while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 1);
	    *buffer += 2;
	}
    }
    else
    {
        memcpy(data, *buffer, count);
	*buffer += count;
    }
} /* nexus_dc_is_native_u_char() */


/*
 * nexus_dc_check_lost_precision_u_char()
 */
int nexus_dc_check_lost_precision_u_char(nexus_byte_t *buffer,
					 unsigned long count,
					 int format)
{
    int result = -1;
    if (format == NEXUS_DC_FORMAT_JAVA)
    {
	nexus_byte_t *place = buffer;
	int i;
	for (i = 0; i < count; i++)
	{
	    if (   *place
		|| *(place + 1) & 0x80)
	    {
		result = i;
		break;
	    }
	    place += 2;
	}
    }
    return(result);
} /* nexus_dc_check_lost_precision_u_char() */


/*
 * nexus_dc_is_native_short()
 */
int nexus_dc_is_native_short(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_short() */


/*
 * nexus_dc_get_short()
 */
void nexus_dc_get_short(nexus_byte_t **buffer,
			short *data,
			unsigned long count,
			int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 2;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 2;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 2);
	*buffer += count * 2;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = (0x80 & **buffer) | *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_short() */


/*
 * nexus_dc_check_lost_precision_short()
 */
int nexus_dc_check_lost_precision_short(nexus_byte_t *buffer,
					unsigned long count,
					int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
        int i;
        for (i = 0; i < count; i++)
	{
	    if (0x80 & *place)
	    {
		if (   ~*place
		    || ~*(place + 1)
		    || ~*(place + 2)
		    || ~*(place + 3)
		    || ~*(place + 4)
		    || ~*(place + 5))
		{
		    result = i;
		    break;
		}
	    }
	    else
	    {
		if (   *place
		    || *(place + 1)
		    || *(place + 2)
		    || *(place + 3)
		    || *(place + 4)
		    || *(place + 5))
		{
		    result = i;
		    break;
		}
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_short() */


/*
 * nexus_dc_is_native_u_short()
 */
int nexus_dc_is_native_u_short(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_u_short() */


/*
 * nexus_dc_get_u_short()
 */
void nexus_dc_get_u_short(nexus_byte_t **buffer,
			  unsigned short *data,
			  unsigned long count,
			  int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 2;
        while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
            *buffer += 2;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 2);
	*buffer += count * 2;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	end = *buffer + count * 8;
        while (*buffer < end)
	{
            *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
            *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_u_short() */


/*
 * nexus_dc_check_lost_precision_u_short()
 */
int nexus_dc_check_lost_precision_u_short(nexus_byte_t *buffer,
					  unsigned long count,
					  int format)
{
    int result = -1;
    switch(format) 
    {
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_JAVA:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (*place & 0x80)
	    {
		result = i;
		break;
	    }
	    place += 2;
	}
	break;
      }
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (   *place
		|| *(place + 1)
		|| *(place + 2)
		|| *(place + 3)
		|| *(place + 4)
		|| *(place + 5))
	    {
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_u_short() */


/*
 * nexus_dc_is_native_int()
 */
int nexus_dc_is_native_int(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_int() */
    

/*
 * nexus_dc_get_int()
 */
void nexus_dc_get_int(nexus_byte_t **buffer,
		      int *data,
		      unsigned long count,
		      int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
            *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 4);
	*buffer += count * 4;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = (0x80 & **buffer) | *(*buffer + 4);
	    *tmp++ = *(*buffer + 5);
	    *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
            *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_int() */


/*
 * nexus_dc_check_lost_precision_int()
 */
int nexus_dc_check_lost_precision_int(nexus_byte_t *buffer,
				      unsigned long count,
				      int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (0x80 & *place)
	    {
		if (   ~*place
		    || ~*(place + 1)
		    || ~*(place + 2)
		    || ~*(place + 3))
		{
		    result = i;
		    break;
		}
	    }
	    else
	    {
		if (   *place
		    || *(place + 1)
		    || *(place + 2)
		    || *(place + 3))
		{
		    result = i;
		    break;
		}
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_int() */


/*
 * nexus_dc_is_native_u_int()
 */
int nexus_dc_is_native_u_int(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_u_int() */


/*
 * nexus_dc_get_u_int()
 */
void nexus_dc_get_u_int(nexus_byte_t **buffer,
			unsigned int *data,
			unsigned long count,
			int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 4;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 4);
        *buffer += count * 4;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 4);
	    *tmp++ = *(*buffer + 5);
	    *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_u_int() */
    

/*
 * nexus_dc_check_lost_precision_u_int()
 */
int nexus_dc_check_lost_precision_u_int(nexus_byte_t *buffer,
					unsigned long count,
					int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_JAVA:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (*place & 0x80)
	    {
		result = i;
		break;
	    }
	    place += 4;
	}
	break;
      }
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (   *place
		|| *(place + 1)
		|| *(place + 2)
		|| *(place + 3))
	    {
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_u_int() */


/*
 * nexus_dc_is_native_long()
 */
int nexus_dc_is_native_long(int format)
{
    if (format == NEXUS_DC_FORMAT_32BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_long() */


/*
 * nexus_dc_get_long()
 */
void nexus_dc_get_long(nexus_byte_t **buffer,
		       long *data,
		       unsigned long count,
		       int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = (0x80 & *(*buffer + 7)) | *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 4;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_CRAYC90:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = (0x80 & **buffer) | *(*buffer + 4);
	    *tmp++ = *(*buffer + 5);
	    *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_BE:
	memcpy(data, *buffer, count * 4);
        *buffer += count * 4;
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_long() */


/*
 * nexus_dc_check_lost_precision_long()
 */
int nexus_dc_check_lost_precision_long(nexus_byte_t *buffer,
				       unsigned long count,
				       int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_CRAYC90:
      case NEXUS_DC_FORMAT_64BIT_BE:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (0x80 & *place)
	    {
		if (   ~*place
		    || ~*(place + 1)
		    || ~*(place + 2)
		    || ~*(place + 3) )
		{
		    result = i;
		    break;
		}
	    }
	    else
	    {
		if (   *place
		    || *(place + 1)
		    || *(place + 2)
		    || *(place + 3))
		{
		    result = i;
		    break;
		}
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_64BIT_LE:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (0x80 & *(place + 7))
	    {
		if (   ~*(place + 4)
		    || ~*(place + 5)
		    || ~*(place + 6)
		    || ~*(place + 7))
		{
		    result = i;
		    break;
		}
	    }
	    else
	    {
		if (   *(place + 4)
		    || *(place + 5)
		    || *(place + 6)
		    || *(place + 7))
		{
		    result = i;
		    break;
		}
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_long() */


/*
 * nexus_dc_is_native_u_long()
 */
int nexus_dc_is_native_u_long(int format)
{
    if (format == NEXUS_DC_FORMAT_32BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_u_long() */


/*
 * nexus_dc_get_u_long()
 */
void nexus_dc_get_u_long(nexus_byte_t **buffer,
			 unsigned long *data,
			 unsigned long count,
			 int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
        end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 4;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_CRAYC90:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 4);
	    *tmp++ = *(*buffer + 5);
	    *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 7);
            *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_BE:
	memcpy(data, *buffer, count * 4);
        *buffer += count * 4;
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_u_long() */


/*
 * nexus_dc_check_lost_precision_u_long()
 */
int nexus_dc_check_lost_precision_u_long(nexus_byte_t *buffer,
					 unsigned long count,
					 int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (   *place
		|| *(place + 1)
		|| *(place + 2)
		|| *(place + 3)
		|| *(place + 4) & 0x80)
	    {
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (   *place
		|| *(place + 1)
		|| *(place + 2)
		|| *(place + 3))
	    {
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_64BIT_LE:
      {
	nexus_byte_t *place = buffer;
        int i;
	for (i = 0; i < count; i++)
	{
	    if (   *(place + 4)
		|| *(place + 5)
		|| *(place + 6)
		|| *(place + 7))
	    {
		 result = i;
		 break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_u_long() */


/*
 * nexus_dc_is_native_float()
 */
int nexus_dc_is_native_float(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_float() */


/*
 * nexus_dc_get_float()
 */
void nexus_dc_get_float(nexus_byte_t **buffer,
			float *data,
			unsigned long count,
			int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
        end = *buffer + count * 4;
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 4);
        *buffer += count * 4;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	unsigned char sign;
	unsigned short exponent;
	unsigned int mantissa;
	int byte, bit;
	int cur_byte, cur_bit;
	nexus_byte_t *tmp2;
	int j;
	end = *buffer + count * 8;
	while (*buffer < end)
	{
	    sign = 0x80 & **buffer;
	    tmp2 = (nexus_byte_t *)&exponent;
	    tmp2[0] = **buffer & 0x7f;
	    tmp2[1] = *(*buffer + 1);
	    /* 16384 (Cray) - 127 (IEEE) = 16257 */
	    exponent -= 16257;
	    /* 48 (Cray) - 23 (IEEE) = 25 slack digits */
	    for (j = 0; j < 25; j++)
	    {
		exponent--;
		byte = j / 8 + 2;
		bit = j % 8;
		if ((*(*buffer + byte) >> (7 - bit)) & 0x01)
		{
		    if (++bit == 8)
		    {
			bit = 0;
			byte++;
		    }
		    break;
		}
	    }
	    if (exponent & 0xff00)
	    {
		exponent = 0xffff;
	    }
	    mantissa = 0;
	    for (j = 0; j < 23; j++)
	    {
		mantissa <<= 1;
		cur_byte = byte + (bit + j) / 8;
		cur_bit = (bit + j) % 8;
		mantissa |= (*(*buffer + cur_byte) >> (7 - cur_bit) & 0x01);\
	    }
	    tmp2 = (nexus_byte_t *)&mantissa;
	    *tmp++ = sign | (exponent >> 1);
	    *tmp++ = ((exponent & 0x01) << 7) | tmp2[1];
	    *tmp++ = tmp2[2];
	    *tmp++ = tmp2[3];
            *buffer += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_float() */


/*
 * nexus_dc_check_lost_precision_float()
 */
int nexus_dc_check_lost_precision_float(nexus_byte_t *buffer,
					unsigned long count,
					int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
	int byte, bit;
	int i, j;
	for (i = 0; i < count; i++)
	{
	    if (*place & 0x5f)
	    {
		/* exponent won't fit */
		result = i;
		break;
	    }
	    for (j = 0; j < 25; j++)
	    {
		byte = j / 8 + 2;
		bit = j % 8;
		if ((*(place + byte) >> (7 - bit)) & 0x01)
		{
		    if (++bit == 8)
		    {
			bit = 0;
			byte++;
		    }
		    break;
		}
	    }
	    if (j < 25)
	    {
		/* mantissa won't fit */
		/*
		 * in actuality, we should check the last n digits for
		 * a 1, but we are assuming that one will be set most
		 * of the time.
		 */
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_float() */


/*
 * nexus_dc_is_native_double()
 */
int nexus_dc_is_native_double(int format)
{
    if (   format == NEXUS_DC_FORMAT_32BIT_BE
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
	return(NEXUS_TRUE);
    else
	return(NEXUS_FALSE);
} /* nexus_dc_is_native_double() */


/*
 * nexus_dc_get_double()
 */
void nexus_dc_get_double(nexus_byte_t **buffer,
			 double *data,
			 unsigned long count,
			 int format)
{
    nexus_byte_t *tmp = (nexus_byte_t *) data;
    nexus_byte_t *end = *buffer + count * 8;
    switch(format)
    {
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
	while (*buffer < end)
	{
	    *tmp++ = *(*buffer + 7);
	    *tmp++ = *(*buffer + 6);
	    *tmp++ = *(*buffer + 5);
	    *tmp++ = *(*buffer + 4);
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 8;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	unsigned char sign;
	unsigned short exponent;
	nexus_byte_t *tmp2;

	while (*buffer < end)
	{
	    sign = 0x80 & **buffer;
	    tmp2 = (nexus_byte_t *)&exponent;
	    tmp2[0] = **buffer & 0x7f;
	    tmp2[1] = *(*buffer + 1);
	    /* 16384 (Cray) - 1023 (IEEE) = 15361 */
	    exponent -= 15361;
	    /* Cray has no assumed 1 starting the mantissa */
	    exponent -= 1;
	    if (exponent & 0xf800)
	    {
		exponent = 0xffff;
	    }
	    tmp2 = *buffer + 2;
	    *tmp++ = sign | ((exponent >> 4) & 0x7f);
	    *tmp++ = ((exponent << 4) & 0xf0) | ((tmp2[0] >> 3) & 0x0f);
	    *tmp++ = ((tmp2[0] << 5) & 0xe0) | ((tmp2[1] >> 3) & 0x1f);
	    *tmp++ = ((tmp2[1] << 5) & 0xe0) | ((tmp2[2] >> 3) & 0x1f);
	    *tmp++ = ((tmp2[2] << 5) & 0xe0) | ((tmp2[3] >> 3) & 0x1f);
	    *tmp++ = ((tmp2[3] << 5) & 0xe0) | ((tmp2[4] >> 3) & 0x1f);
	    *tmp++ = ((tmp2[4] << 5) & 0xe0) | ((tmp2[5] >> 3) & 0x1f);
	    *tmp++ = (tmp2[5] << 5) & 0xe0;
            *buffer += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	/* DUNNO!!! */
	break;
    }
} /* nexus_dc_get_double() */


/*
 * nexus_dc_check_lost_precision_double()
 */
int nexus_dc_check_lost_precision_double(nexus_byte_t *buffer,
					 unsigned long count,
					 int format)
{
    int result = -1;
    switch(format)
    {
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
      {
	nexus_byte_t *place = buffer;
	int i;

	for (i = 0; i < count; i++)
	{
	    if (*place & 0x38)
	    {
		/* exponent doesn't fit */
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_double() */
