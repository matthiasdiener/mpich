/*
 * nexus_dc_cray.c
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_dc/nexus_dc_cray.c,v 1.2 1996/11/13 20:52:40 tuecke Exp $";

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
	nexus_byte_t *tmp = (nexus_byte_t *) data;
	nexus_byte_t *end = *buffer + count;

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
 * nexus_dc_get_u_char()
 */
void nexus_dc_get_u_char(nexus_byte_t **buffer,
			 unsigned char *data,
			 unsigned long count,
			 int format)
{
    if (format == NEXUS_DC_FORMAT_JAVA)
    {
	nexus_byte_t *tmp = (nexus_byte_t *) data;
	nexus_byte_t *end = *buffer + count;

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
} /* nexus_dc_get_u_char() */


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
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
	    if (*(*buffer + 1) &0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 2;
	}
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 2;
        while (*buffer < end)
	{
	    if (**buffer & 0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *buffer += 2;
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
    return(-1);
} /* nexus_dc_check_lost_precision_short() */


/*
 * nexus_dc_is_native_u_short()
 */
int nexus_dc_is_native_u_short(int format)
{
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
            *buffer += 2;
	}
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 2;
        while (*buffer < end)
	{
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *buffer += 2;
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
    return(-1);
} /* nexus_dc_check_lost_precision_u_short() */


/*
 * nexus_dc_is_native_int()
 */
int nexus_dc_is_native_int(int format)
{
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
	    if (*(*buffer + 3) & 0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	}
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    if (**buffer & 0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 3);
            *buffer += 4;
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
    return(-1);
} /* nexus_dc_check_lost_precision_int() */


/*
 * nexus_dc_is_native_u_int()
 */
int nexus_dc_is_native_u_int(int format)
{
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 3);
	    *buffer += 4;
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
    return(-1);
} /* nexus_dc_check_lost_precision_u_int() */


/*
 * nexus_dc_is_native_long()
 */
int nexus_dc_is_native_long(int format)
{
    if (   format == NEXUS_DC_FORMAT_CRAYC90
	|| format == NEXUS_DC_FORMAT_64BIT_BE
	|| format == NEXUS_DC_FORMAT_JAVA)
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
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    if (*(*buffer + 3) & 0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_BE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    if (**buffer & 0x80)
	    {
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
		*tmp++ = 0xff;
	    }
	    else
	    {
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
		*tmp++ = 0;
	    }
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 3);
            *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_CRAYC90:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
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
    return(-1);
} /* nexus_dc_check_lost_precision_long() */


/*
 * nexus_dc_is_native_u_long()
 */
int nexus_dc_is_native_u_long(int format)
{
    if (   format == NEXUS_DC_FORMAT_CRAYC90
	|| format == NEXUS_DC_FORMAT_64BIT_BE)
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
      case NEXUS_DC_FORMAT_32BIT_LE:
	end = *buffer + count * 8;
        while (*buffer < end)
	{
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = *(*buffer + 3);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = **buffer;
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_32BIT_BE:
	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = **buffer;
	    *tmp++ = *(*buffer + 1);
	    *tmp++ = *(*buffer + 2);
	    *tmp++ = *(*buffer + 3);
	    *buffer += 4;
	}
	break;
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_CRAYC90:
      case NEXUS_DC_FORMAT_64BIT_BE:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
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
    if (format == NEXUS_DC_FORMAT_JAVA)
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
	    place += 8;
	}
    }
    return(result);
} /* nexus_dc_check_lost_precision_u_long() */


/*
 * nexus_dc_is_native_float()
 */
int nexus_dc_is_native_float(int format)
{
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      {
	unsigned char sign;
	unsigned int exponent;

	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    sign = **buffer & 0x80;
	    exponent = **buffer << 1 | *(*buffer + 1) >> 7;
	    /* 16384 (Cray) - 127 (IEEE) = 16257 */
	    exponent += 16257;
	    /* Cray has no assumed 1 on mantissa */
	    exponent++;
	    *tmp++ = sign | exponent >> 8;
	    *tmp++ = exponent & 0xff;
	    *tmp++ = *(*buffer + 1) << 1 | *(*buffer + 2) >> 7;
	    *tmp++ = *(*buffer + 2) << 1 | *(*buffer + 3) >> 7;
	    *tmp++ = *(*buffer + 3) << 1;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *buffer += 4;
	}
	break;
      }
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      {
	unsigned char sign;
	unsigned int exponent;

	end = *buffer + count * 4;
        while (*buffer < end)
	{
	    sign = *(*buffer + 3) & 0x80;
	    exponent = *(*buffer + 3) << 1 | *(*buffer + 2) >> 7;
	    /* 16384 (Cray) - 127 (IEEE) = 16257 */
	    exponent += 16257;
	    /* Cray has no assumed 1 on mantissa */
	    exponent++;
	    *tmp++ = sign | exponent >> 8;
	    *tmp++ = exponent & 0xff;
	    *tmp++ = *(*buffer + 2) << 1 | *(*buffer + 1) >> 7;
	    *tmp++ = *(*buffer + 1) << 1 | **buffer >> 7;
	    *tmp++ = **buffer << 1;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *tmp++ = 0;
	    *buffer += 4;
	}
        break;
      }
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
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
    return(-1);
} /* nexus_dc_check_lost_precision_float() */


/*
 * nexus_dc_is_native_double()
 */
int nexus_dc_is_native_double(int format)
{
    if (format == NEXUS_DC_FORMAT_CRAYC90)
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
      case NEXUS_DC_FORMAT_JAVA:
      case NEXUS_DC_FORMAT_32BIT_BE:
      case NEXUS_DC_FORMAT_64BIT_BE:
      {
	unsigned char sign;
	unsigned int exponent;

        while (*buffer < end)
	{
	    sign = **buffer & 0x80;
	    memcpy(&exponent, *buffer, 4);
	    exponent &= 0x7ff0000000000000;
	    exponent >>= 52;
	    /* 16384 (Cray) - 1023 (IEEE) = 15361 */
	    exponent += 15361;
	    /* Cray has no assumed 1 starting the mantissa */
	    exponent++;
	    *tmp++ = sign | exponent >> 8;
	    *tmp++ = (exponent & 0xff) | *(*buffer + 1) >> 4;
	    *tmp++ = *(*buffer + 1) << 4 | *(*buffer + 2) >> 4;
	    *tmp++ = *(*buffer + 2) << 4 | *(*buffer + 3) >> 4;
	    *tmp++ = *(*buffer + 3) << 4 | *(*buffer + 4) >> 4;
	    *tmp++ = *(*buffer + 4) << 4 | *(*buffer + 5) >> 4;
	    *tmp++ = *(*buffer + 5) << 4 | *(*buffer + 6) >> 4;
	    *tmp++ = *(*buffer + 6) << 4 | *(*buffer + 7) >> 4;
	    *buffer += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_64BIT_LE:
      case NEXUS_DC_FORMAT_32BIT_LE:
      {
	unsigned char sign;
	unsigned int exponent;
	int i;

	for (i = 0; i < count; i++)
	{
	    sign = *(*buffer + 7) & 0x80;
	    exponent = (int)(*(*buffer + 7) & 0x7f);
	    exponent <<= 4;
	    exponent += (int)(*(*buffer + 6) & 0x0f);
	    /* 16384 (Cray) - 1023 (IEEE) = 15361 */
	    exponent += 15361;
	    /* Cray has no assumed 1 starting the mantissa */
	    exponent++;
	    *tmp++ = sign | exponent >> 8;
	    *tmp++ = (exponent & 0xff) | *(*buffer + 6) >> 4;
	    *tmp++ = *(*buffer + 6) << 4 | *(*buffer + 5) >> 4;
	    *tmp++ = *(*buffer + 5) << 4 | *(*buffer + 4) >> 4;
	    *tmp++ = *(*buffer + 4) << 4 | *(*buffer + 3) >> 4;
	    *tmp++ = *(*buffer + 3) << 4 | *(*buffer + 2) >> 4;
	    *tmp++ = *(*buffer + 2) << 4 | *(*buffer + 1) >> 4;
	    *tmp++ = *(*buffer + 1) << 4 | **buffer >> 4;
	    *buffer += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_CRAYC90:
	memcpy(data, *buffer, count * 8);
        *buffer += count * 8;
	break;
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
      {
	nexus_byte_t *place = buffer;
	int i;

	for (i = 0; i < count; i++)
	{
	    if (*(place + 7) & 0x0f)
	    {
		result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_32BIT_LE:
      case NEXUS_DC_FORMAT_64BIT_LE:
      {
	nexus_byte_t *place = buffer;
	int i;

        for (i = 0; i < count; i++)
	{
	    if (*place & 0x0f)
	    {
	        result = i;
		break;
	    }
	    place += 8;
	}
	break;
      }
      case NEXUS_DC_FORMAT_CRAYC90:
	break;
      case NEXUS_DC_FORMAT_UNKNOWN:
	result = 0;
	break;
    }
    return(result);
} /* nexus_dc_check_lost_precision_double() */
