/*
 * nx_sanity.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nx_sanity.h,v 1.10 1996/10/09 23:45:25 tuecke Exp $"
 *
 * Sanity checking of structures.
 */

#ifndef _NEXUS_INCLUDE_NX_SANITY_H
#define _NEXUS_INCLUDE_NX_SANITY_H

/*
 * Sanity checking inspects the data structures and
 * tries to determine if corruption has occurred
 * before the system would crash.  Threads then
 * can be "imprisoned" so that they may be inspected.
 * Important since multithreaded applications can
 * be non-deterministic.
 * sanity.h is placed into nexus.h because the macros
 * that are here should be available to the user,
 * since they are called from macros that the user
 * calls.
 * Sanity checking is turned on/off by defining
 * NEXUS_SANITY_CHECK
 * JMP 02/28/94
 */

EXTERN_C_BEGIN

typedef enum __nx_structure_types
{
    _NX_NODE_T,
    _NX_CONTEXT_T,
    _NX_SEGMENT_T,
    _NX_STARTPOINT_T,
    _NX_ENDPOINT_T,
    _NX_ENDPOINTATTR_T,
    _NX_BUFFER_T,
    _NX_BASE_SEGMENT_T,
    _NX_DIRECT_SEGMENT_T,
#ifndef BUILD_LITE
    _NX_THREAD_T,
    _NX_MUTEX_T,
    _NX_COND_T,
    _NX_THREAD_FREELIST_T,
#endif /* BUILD_LITE */
    _NX_HANDLER_LIST_T,
    _NX_HANDLER_RECORD_T
} _nx_structure_types;

#define _NX_START_COOKIE_VAL 0x5555
#define _NX_END_COOKIE_VAL   0xaaaa

#ifdef NEXUS_SANITY_CHECK

#define NEXUS_INTERROGATE(mut, code, caller) \
	_nx_interrogate_suspect((void*)mut, code, caller);
#define _NX_START_MAGIC_COOKIE unsigned int _nx_start_magic_cookie;
#define _NX_END_MAGIC_COOKIE   unsigned int _nx_end_magic_cookie;
#define _NX_INIT_START_MAGIC_COOKIE(v1) \
         do {(v1)->_nx_start_magic_cookie=_NX_START_COOKIE_VAL;} while(0)
#define _NX_INIT_END_MAGIC_COOKIE(v1) \
         do {(v1)->_nx_end_magic_cookie=_NX_END_COOKIE_VAL;} while(0)
#define _NX_CHECK_START_MAGIC_COOKIE(tar) \
	((tar)->_nx_start_magic_cookie==_NX_START_COOKIE_VAL)
#define _NX_CHECK_END_MAGIC_COOKIE(tar) \
	((tar)->_nx_end_magic_cookie==_NX_END_COOKIE_VAL)
#define _NX_EXTRACT_START_MAGIC_COOKIE(m) (m)->_nx_start_magic_cookie
#define _NX_EXTRACT_END_MAGIC_COOKIE(m) (m)->_nx_end_magic_cookie

extern void _nx_imprison_thread(char *charges);
extern void _nx_interrogate_suspect(void *evidence,
				    _nx_structure_types suspect_type,
				    char *caller);
#else

#define NEXUS_INTERROGATE(mut, code, caller) do { int i; i=3; } while(0)
#define _NX_START_MAGIC_COOKIE
#define _NX_END_MAGIC_COOKIE
#define _NX_INIT_START_MAGIC_COOKIE(v1) \
        do { int i; i=3; } while(0)
#define _NX_INIT_END_MAGIC_COOKIE(v1) \
        do { int i; i=3; } while(0)
#define _NX_CHECK_START_MAGIC_COOKIE(tar) 0
#define _NX_CHECK_END_MAGIC_COOKIE(tar) 0
#define _NX_EXTRACT_START_MAGIC_COOKIE(m) 0
#define _NX_EXTRACT_END_MAGIC_COOKIE(m) 0

#endif /* NEXUS_SANITY_CHECK */

EXTERN_C_END

#endif /* _NEXUS_INCLUDE_NX_SANITY_H */
