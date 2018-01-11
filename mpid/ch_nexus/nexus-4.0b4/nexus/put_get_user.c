/*
 * put_get_user.c
 *
 * nexus_direct_put_*() and nexus_direct_get_*() code
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/put_get_user.c,v 1.2 1996/11/28 00:21:05 tuecke Exp $";

#include "internal.h"


/*********************************************************************
 * nexus_user_put_DATATYPE()
 *********************************************************************/

#undef nexus_user_put_float
void nexus_user_put_float(nexus_byte_t **user_buffer,
			  float *data,
			  unsigned long count)
{
    nexus_macro_user_put_float(user_buffer, data, count);
}

#undef nexus_user_put_double
void nexus_user_put_double(nexus_byte_t **user_buffer,
			   double *data,
			   unsigned long count)
{
    nexus_macro_user_put_double(user_buffer, data, count);
}

#undef nexus_user_put_short
void nexus_user_put_short(nexus_byte_t **user_buffer,
			  short *data,
			  unsigned long count)
{
    nexus_macro_user_put_short(user_buffer, data, count);
}

#undef nexus_user_put_u_short
void nexus_user_put_u_short(nexus_byte_t **user_buffer,
			    unsigned short *data,
			    unsigned long count)
{
    nexus_macro_user_put_u_short(user_buffer, data, count);
}

#undef nexus_user_put_int
void nexus_user_put_int(nexus_byte_t **user_buffer,
			int *data,
			unsigned long count)
{
    nexus_macro_user_put_int(user_buffer, data, count);
}

#undef nexus_user_put_u_int
void nexus_user_put_u_int(nexus_byte_t **user_buffer,
			  unsigned int *data,
			  unsigned long count)
{
    nexus_macro_user_put_u_int(user_buffer, data, count);
}

#undef nexus_user_put_long
void nexus_user_put_long(nexus_byte_t **user_buffer,
			 long *data,
			 unsigned long count)
{
    nexus_macro_user_put_long(user_buffer, data, count);
}

#undef nexus_user_put_u_long
void nexus_user_put_u_long(nexus_byte_t **user_buffer,
			   unsigned long *data,
			   unsigned long count)
{
    nexus_macro_user_put_u_long(user_buffer, data, count);
}

#undef nexus_user_put_char
void nexus_user_put_char(nexus_byte_t **user_buffer,
			 char *data,
			 unsigned long count)
{
    nexus_macro_user_put_char(user_buffer, data, count);
}

#undef nexus_user_put_u_char
void nexus_user_put_u_char(nexus_byte_t **user_buffer,
			   unsigned char *data,
			   unsigned long count)
{
    nexus_macro_user_put_u_char(user_buffer, data, count);
}

#undef nexus_user_put_byte
void nexus_user_put_byte(nexus_byte_t **user_buffer,
			 nexus_byte_t *data,
			 unsigned long count)
{
    nexus_macro_user_put_byte(user_buffer, data, count);
}



/*********************************************************************
 * nexus_user_get_DATATYPE()
 *********************************************************************/

#undef nexus_user_get_float
void nexus_user_get_float(nexus_byte_t **user_buffer,
			  float *data,
			  unsigned long count,
			  int format)
{
    nexus_macro_user_get_float(user_buffer, data, count, format);
}

#undef nexus_user_get_double
void nexus_user_get_double(nexus_byte_t **user_buffer,
			   double *data,
			   unsigned long count,
			   int format)
{
    nexus_macro_user_get_double(user_buffer, data, count, format);
}

#undef nexus_user_get_short
void nexus_user_get_short(nexus_byte_t **user_buffer,
			  short *data,
			  unsigned long count,
			  int format)
{
    nexus_macro_user_get_short(user_buffer, data, count, format);
}

#undef nexus_user_get_u_short
void nexus_user_get_u_short(nexus_byte_t **user_buffer,
			    unsigned short *data,
			    unsigned long count,
			    int format)
{
    nexus_macro_user_get_u_short(user_buffer, data, count, format);
}

#undef nexus_user_get_int
void nexus_user_get_int(nexus_byte_t **user_buffer,
			int *data,
			unsigned long count,
			int format)
{
    nexus_macro_user_get_int(user_buffer, data, count, format);
}

#undef nexus_user_get_u_int
void nexus_user_get_u_int(nexus_byte_t **user_buffer,
			  unsigned int *data,
			  unsigned long count,
			  int format)
{
    nexus_macro_user_get_u_int(user_buffer, data, count, format);
}

#undef nexus_user_get_long
void nexus_user_get_long(nexus_byte_t **user_buffer,
			 long *data,
			 unsigned long count,
			 int format)
{
    nexus_macro_user_get_long(user_buffer, data, count, format);
}

#undef nexus_user_get_u_long
void nexus_user_get_u_long(nexus_byte_t **user_buffer,
			   unsigned long *data,
			   unsigned long count,
			   int format)
{
    nexus_macro_user_get_u_long(user_buffer, data, count, format);
}

#undef nexus_user_get_char
void nexus_user_get_char(nexus_byte_t **user_buffer,
			 char *data,
			 unsigned long count,
			 int format)
{
    nexus_macro_user_get_char(user_buffer, data, count, format);
}

#undef nexus_user_get_u_char
void nexus_user_get_u_char(nexus_byte_t **user_buffer,
			   unsigned char *data,
			   unsigned long count,
			   int format)
{
    nexus_macro_user_get_u_char(user_buffer, data, count, format);
}

#undef nexus_user_get_byte
void nexus_user_get_byte(nexus_byte_t **user_buffer,
			 nexus_byte_t *data,
			 unsigned long count,
			 int format)
{
    nexus_macro_user_get_byte(user_buffer, data, count, format);
}



