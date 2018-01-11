/* rcsid = $Header: /nfs/globus1/src/master/nexus_dc/nexus_dc.h.in,v 1.7 1996/12/11 00:37:51 tuecke Exp $ */

#ifndef NEXUS_DC_H
#define NEXUS_DC_H

#include <memory.h>

/* EXTERN_C_BEGIN and EXTERN_C_END should surround any prototypes.
   .h files that are included by nexus.h.  This will
   This will allow C++ codes to include this header to properly link
   against the library.
*/
#if defined(__cplusplus) && !defined(EXTERN_C_BEGIN)
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#ifndef NEXUS_GLOBAL
#ifdef NEXUS_DEFINE_GLOBALS
#define NEXUS_GLOBAL
#else
#define NEXUS_GLOBAL extern
#endif
#endif

#ifndef NEXUS_TRUE
#define	NEXUS_TRUE		1
#define	NEXUS_FALSE		0
#endif

/**********************************************************************/

/*
 * little endian, with unsigned
 *  8 bit byte
 *  8 bit char
 * 16 bit short
 * 32 bit integer
 * 32 bit long
 * 32 bit pointer
 * 32 bit IEEE float
 * 64 bit IEEE double
 */
#define NEXUS_DC_FORMAT_32BIT_LE	0

/*
 * big endian, with unsigned
 *  8 bit byte
 *  8 bit char
 * 16 bit short
 * 32 bit integer
 * 32 bit long
 * 32 bit pointer
 * 32 bit IEEE float
 * 64 bit IEEE double
 */
#define NEXUS_DC_FORMAT_32BIT_BE	1

/*
 * little endian, with unsigned
 *  8 bit byte
 *  8 bit char
 * 16 bit short
 * 32 bit integer
 * 64 bit long
 * 64 bit pointer
 * 32 bit IEEE float
 * 64 bit IEEE double
 */
#define NEXUS_DC_FORMAT_64BIT_LE	2

/*
 * big endian, with unsigned
 *  8 bit byte
 *  8 bit char
 * 16 bit short
 * 32 bit integer
 * 64 bit long
 * 64 bit pointer
 * 32 bit IEEE float
 * 64 bit IEEE double
 */
#define NEXUS_DC_FORMAT_64BIT_BE	3

/*
 * big endian, with unsigned?
 *  8 bit byte
 *  8 bit char
 * 64 bit short
 * 64 bit integer
 * 64 bit long
 * 64 bit pointer
 * 32 bit Cray float
 * 64 bit Cray double
 */
#define NEXUS_DC_FORMAT_CRAYC90		4

/*
 * big endian, without unsigned
 *  8 bit byte
 * 16 bit char
 * 16 bit short
 * 32 bit integer
 * 64 bit long
 * no pointer
 * 32 bit IEEE float
 * 64 bit IEEE double
 */
#define NEXUS_DC_FORMAT_JAVA		5

#define NEXUS_DC_FORMAT_LAST		6

#define NEXUS_DC_FORMAT_UNKNOWN	99

#define NEXUS_DC_FORMAT_LOCAL NEXUS_DC_FORMAT_32BIT_BE

#define nexus_dc_format() \
    (NEXUS_DC_FORMAT_LOCAL)

#define nexus_dc_is_valid_format(Format) \
    (((Format) >= 0) && ((Format) < NEXUS_DC_FORMAT_LAST))

/**********************************************************************/

typedef unsigned char nexus_byte_t;

/**********************************************************************/

enum
{
    NEXUS_DC_DATATYPE_BYTE,
    NEXUS_DC_DATATYPE_CHAR,
    NEXUS_DC_DATATYPE_U_CHAR,
    NEXUS_DC_DATATYPE_SHORT,
    NEXUS_DC_DATATYPE_U_SHORT,
    NEXUS_DC_DATATYPE_INT,
    NEXUS_DC_DATATYPE_U_INT,
    NEXUS_DC_DATATYPE_LONG,
    NEXUS_DC_DATATYPE_U_LONG,
    NEXUS_DC_DATATYPE_FLOAT,
    NEXUS_DC_DATATYPE_DOUBLE
};

/**********************************************************************/

#define NEXUS_DC_SwapByte(a, b) \
{ \
    (a) ^= (b); \
    (b) ^= (a); \
    (a) ^= (b); \
}

/**********************************************************************/

#define NEXUS_DC_SIZEOF(TYPE, count) \
    (unsigned long)((count) * sizeof(TYPE))

#define nexus_dc_sizeof_byte(count) NEXUS_DC_SIZEOF(unsigned char, count)
#define nexus_dc_sizeof_char(count) NEXUS_DC_SIZEOF(char, count)
#define nexus_dc_sizeof_u_char(count) NEXUS_DC_SIZEOF(unsigned char, count)
#define nexus_dc_sizeof_short(count) NEXUS_DC_SIZEOF(short, count)
#define nexus_dc_sizeof_u_short(count) NEXUS_DC_SIZEOF(unsigned short, count)
#define nexus_dc_sizeof_int(count) NEXUS_DC_SIZEOF(int, count)
#define nexus_dc_sizeof_u_int(count) NEXUS_DC_SIZEOF(unsigned int, count)
#define nexus_dc_sizeof_long(count) NEXUS_DC_SIZEOF(long, count)
#define nexus_dc_sizeof_u_long(count) NEXUS_DC_SIZEOF(unsigned long, count)
#define nexus_dc_sizeof_float(count) NEXUS_DC_SIZEOF(float, count)
#define nexus_dc_sizeof_double(count) NEXUS_DC_SIZEOF(double, count)

/**********************************************************************/

#define NEXUS_DC_PUT(TYPE, buffer, data, count) \
{ \
    unsigned long c = NEXUS_DC_SIZEOF(TYPE, count); \
    memcpy(*(buffer), data, c); \
    *(buffer) += c; \
}

#define nexus_dc_put_byte(buffer, data, count) \
    NEXUS_DC_PUT(unsigned char, buffer, data, count)
#define nexus_dc_put_char(buffer, data, count) \
    NEXUS_DC_PUT(char, buffer, data, count)
#define nexus_dc_put_u_char(buffer, data, count) \
    NEXUS_DC_PUT(unsigned char, buffer, data, count)
#define nexus_dc_put_short(buffer, data, count) \
    NEXUS_DC_PUT(short, buffer, data, count)
#define nexus_dc_put_u_short(buffer, data, count) \
    NEXUS_DC_PUT(unsigned short, buffer, data, count)
#define nexus_dc_put_int(buffer, data, count) \
    NEXUS_DC_PUT(int, buffer, data, count)
#define nexus_dc_put_u_int(buffer, data, count) \
    NEXUS_DC_PUT(unsigned int, buffer, data, count)
#define nexus_dc_put_long(buffer, data, count) \
    NEXUS_DC_PUT(long, buffer, data, count)
#define nexus_dc_put_u_long(buffer, data, count) \
    NEXUS_DC_PUT(unsigned long, buffer, data, count)
#define nexus_dc_put_float(buffer, data, count) \
    NEXUS_DC_PUT(float, buffer, data, count)
#define nexus_dc_put_double(buffer, data, count) \
    NEXUS_DC_PUT(double, buffer, data, count)

/**********************************************************************/

#define nexus_dc_is_native_byte(format) (NEXUS_TRUE)

#define nexus_dc_get_byte(buffer, data, count, format) \
{ \
    unsigned long c = (count); \
    memcpy(data, *(buffer), c); \
    *(buffer) += c; \
}

#define nexus_dc_check_lost_precision_byte(buffer, count, format) (-1)

/**********************************************************************/

/* byte,char,u_char,short,u_short,int,u_int,long,u_long,float,double */
NEXUS_GLOBAL int nexus_dc_sizeof_remote_table[][11]
#ifdef NEXUS_DEFINE_GLOBALS
= { \
  {1, 1, 1, 2, 2, 4, 4, 4, 4, 4, 8}, /* NEXUS_DC_FORMAT_32BIT_LE */
  {1, 1, 1, 2, 2, 4, 4, 4, 4, 4, 8}, /* NEXUS_DC_FORMAT_32BIT_BE */
  {1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8}, /* NEXUS_DC_FORMAT_64BIT_LE */
  {1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8}, /* NEXUS_DC_FORMAT_64BIT_BE */
  {1, 1, 1, 8, 8, 8, 8, 8, 8, 8, 8}, /* NEXUS_DC_FORMAT_CRAYC90  */
  {1, 2, 2, 2, 2, 4, 4, 8, 8, 4, 8}  /* NEXUS_DC_FORMAT_JAVA     */
}
#endif
;

#define nexus_dc_sizeof_remote_byte(count, format) \
    (nexus_dc_sizeof_remote_table[format][0] * (count))

#define nexus_dc_sizeof_remote_char(count, format) \
    (nexus_dc_sizeof_remote_table[format][1] * (count))

#define nexus_dc_sizeof_remote_u_char(count, format) \
    (nexus_dc_sizeof_remote_table[format][2] * (count))

#define nexus_dc_sizeof_remote_short(count, format) \
    (nexus_dc_sizeof_remote_table[format][3] * (count))

#define nexus_dc_sizeof_remote_u_short(count, format) \
    (nexus_dc_sizeof_remote_table[format][4] * (count))

#define nexus_dc_sizeof_remote_int(count, format) \
    (nexus_dc_sizeof_remote_table[format][5] * (count))

#define nexus_dc_sizeof_remote_u_int(count, format) \
    (nexus_dc_sizeof_remote_table[format][6] * (count))

#define nexus_dc_sizeof_remote_long(count, format) \
    (nexus_dc_sizeof_remote_table[format][7] * (count))

#define nexus_dc_sizeof_remote_u_long(count, format) \
    (nexus_dc_sizeof_remote_table[format][8] * (count))

#define nexus_dc_sizeof_remote_float(count, format) \
    (nexus_dc_sizeof_remote_table[format][9] * (count))

#define nexus_dc_sizeof_remote_double(count, format) \
    (nexus_dc_sizeof_remote_table[format][10] * (count))

/**********************************************************************/

EXTERN_C_BEGIN

extern int	nexus_dc_is_native_char(int format);
extern int	nexus_dc_is_native_u_char(int format);
extern int	nexus_dc_is_native_short(int format);
extern int	nexus_dc_is_native_u_short(int format);
extern int	nexus_dc_is_native_int(int format);
extern int	nexus_dc_is_native_u_int(int format);
extern int	nexus_dc_is_native_long(int format);
extern int	nexus_dc_is_native_u_long(int format);
extern int	nexus_dc_is_native_float(int format);
extern int	nexus_dc_is_native_double(int format);

extern void	nexus_dc_get_char(nexus_byte_t **buffer,
				  char *data,
				  unsigned long count,
				  int format);
extern void	nexus_dc_get_u_char(nexus_byte_t **buffer,
				    unsigned char *data,
				    unsigned long count,
				    int format);
extern void	nexus_dc_get_short(nexus_byte_t **buffer,
				   short *data,
				   unsigned long count,
				   int format);
extern void	nexus_dc_get_u_short(nexus_byte_t **buffer,
				     unsigned short *data,
				     unsigned long count,
				     int format);
extern void	nexus_dc_get_int(nexus_byte_t **buffer,
				 int *data,
				 unsigned long count,
				 int format);
extern void	nexus_dc_get_u_int(nexus_byte_t **buffer,
				   unsigned int *data,
				   unsigned long count,
				   int format);
extern void	nexus_dc_get_long(nexus_byte_t **buffer,
				  long *data,
				  unsigned long count,
				  int format);
extern void	nexus_dc_get_u_long(nexus_byte_t **buffer,
				    unsigned long *data,
				    unsigned long count,
				    int format);
extern void	nexus_dc_get_float(nexus_byte_t **buffer,
				   float *data,
				   unsigned long count,
				   int format);
extern void	nexus_dc_get_double(nexus_byte_t **buffer,
				    double *data,
				    unsigned long count,
				    int format);

extern int	nexus_dc_check_lost_precision_char(nexus_byte_t *buffer,
						   unsigned long count,
						   int format);
extern int	nexus_dc_check_lost_precision_u_char(nexus_byte_t *buffer,
						     unsigned long count,
						     int format);
extern int	nexus_dc_check_lost_precision_short(nexus_byte_t *buffer,
						    unsigned long count,
						    int format);
extern int	nexus_dc_check_lost_precision_u_short(nexus_byte_t *buffer,
						      unsigned long count,
						      int format);
extern int	nexus_dc_check_lost_precision_int(nexus_byte_t *buffer,
						  unsigned long count,
						  int format);
extern int	nexus_dc_check_lost_precision_u_int(nexus_byte_t *buffer,
						    unsigned long count,
						    int format);
extern int	nexus_dc_check_lost_precision_long(nexus_byte_t *buffer,
						   unsigned long count,
						   int format);
extern int	nexus_dc_check_lost_precision_u_long(nexus_byte_t *buffer,
						     unsigned long count,
						     int format);
extern int	nexus_dc_check_lost_precision_float(nexus_byte_t *buffer,
						    unsigned long count,
						    int format);
extern int	nexus_dc_check_lost_precision_double(nexus_byte_t *buffer,
						     unsigned long count,
						     int format);
    
EXTERN_C_END

#endif /* NEXUS_DC_H */
