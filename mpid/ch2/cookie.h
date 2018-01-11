#ifndef MPIR_COOKIE
/*****************************************************************************
*  We place "cookies" into the data structures to improve error detection and*
*  reporting of invalid objects.  In order to make this flexible, the        *
*  cookies are defined as macros.                                            *
*  If MPIR_HAS_COOKIES is not defined, then the "cookie" fields are not      *
*  set or tested                                                             *
*****************************************************************************/

#define MPIR_HAS_COOKIES

#ifdef MPIR_HAS_COOKIES
#define MPIR_COOKIE unsigned long cookie;
#define MPIR_SET_COOKIE(obj,value) (obj)->cookie = (value);
#else
#define MPIR_COOKIE
#define MPIR_SET_COOKIE(obj,value)
#endif
/****************************************************************************/

#endif
