/*
 * p0_internal.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_internal.h,v 1.11 1996/02/28 20:44:05 patton Exp $"
 */

#define _P0_SYSTEM_COMPILE

#include "ports0.h"

#ifndef BUILD_LITE
/*
 * Include the thread module's internal header, which should contain:
 *   1) #include's for any system headers needed internally for threads
 *   2) typedefs for internal ports0 types
 *   3) macros for internal ports0 routines
 */
#define PORTS0_THI_HEADER p0_quoter(p0_cat(TH_MODULE,_i.h))

#include PORTS0_THI_HEADER

#endif /* BUILD_LITE */


/*
 * Now include all of the necessary system headers
 */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_LIBC_H
#include <libc.h>
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_ACCESS_H
#include <sys/access.h>
#endif

#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif

#ifdef TARGET_ARCH_PARAGON
#define PORTS0_ARGS "Ports0Args" /* Environment variable */
#endif

/*
 * Now include the rest of the internal Ports0 headers.
 */
#include "p0_macros.h"
#include "p0_types.h"
#include "p0_funcs.h"
#include "p0_globals.h"
#include "p0_defs.h"

