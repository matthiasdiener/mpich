/*
 * p0_globals.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_globals.h,v 1.6 1996/01/29 22:47:24 patton Exp $"
 */

#ifndef _PORTS0_INCLUDE_GLOBALS_H
#define _PORTS0_INCLUDE_GLOBALS_H

/*
 * This is used by the ports0 print routines for output.
 */
PORTS0_GLOBAL FILE 	*_p0_stdout;

/*
 * This is used to tell if ports0_init() has been called.
 */
PORTS0_GLOBAL ports0_bool_t _p0_ports0_init_called;

/*
 * Various debug flags...
 */
#ifdef BUILD_DEBUG
PORTS0_GLOBAL int	_p0_debug_level;
#endif

#ifdef PORTS0_MALLOC_DEBUG
PORTS0_GLOBAL int		_p0_malloc_debug_level;
PORTS0_GLOBAL int 		_p0_malloc_pad_size;
PORTS0_GLOBAL ports0_bool_t	_p0_trace_malloc;
PORTS0_GLOBAL ports0_bool_t	_p0_no_frees;
#endif /* PORTS0_MALLOC_DEBUG */

#endif /* _PORTS0_INCLUDE_GLOBALS_H */
