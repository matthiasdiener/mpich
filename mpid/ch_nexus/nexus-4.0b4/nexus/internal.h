/*
 * internal.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/internal.h,v 1.36 1996/10/07 04:39:57 tuecke Exp $"
 */

#define _NX_SYSTEM_COMPILE

#include "nexus.h"


#ifndef BUILD_LITE
/*
 * Include the thread module's internal header, which should contain:
 *   1) #include's for any system headers needed internally for threads
 *   2) typedefs for internal nexus types
 *   3) macros for internal nexus routines
 */
#ifndef NEXUS_THI_HEADER
#define NEXUS_THI_HEADER "th_p0_i.h"
#endif
#include NEXUS_THI_HEADER

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
#define NEXUS_ARGS "NexusArgs" /* Environment variable */
#endif

/*
 * Now include the rest of the internal Nexus headers.
 */
#include "defs.h"
#include "macros.h"
#include "types.h"
#include "funcs.h"
#include "globals.h"
#ifdef BUILD_PROFILE
#include "pablo.h"
#endif

