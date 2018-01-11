/*
 * p0_macros.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_macros.h,v 1.4 1996/01/29 22:47:27 patton Exp $"
 */

#ifndef _PORTS0_INCLUDE_MACROS_H
#define _PORTS0_INCLUDE_MACROS_H


#ifdef BUILD_DEBUG
#define Ports0Debug(Level) (_p0_debug_level >= (Level))
#endif

#ifdef BUILD_DEBUG
#define ports0_debug_printf(level, message) \
do { \
    if (Ports0Debug(level)) \
    { \
	ports0_printf message; \
    } \
} while (0)
#else
#define ports0_debug_printf(level, message)
#endif

#ifdef PORTS0_MALLOC_DEBUG
#define Ports0MallocDebug(Level) (_p0_malloc_debug_level >= (Level))
#endif

#endif /* _PORTS0_INCLUDE_MACROS_H */
