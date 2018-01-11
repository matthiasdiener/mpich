/*
 * nx_ver.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nx_ver.h,v 1.10 1996/10/07 04:40:06 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_NX_VERSION_H
#define _NEXUS_INCLUDE_NX_VERSION_H

#ifdef BUILD_PROFILE
extern int _nexus_linked_profile;
static int _nexus_defined_profile(void) { return _nexus_linked_profile; }
#else
extern int _nexus_linked_no_profile;
static int _nexus_defined_profile(void) { return _nexus_linked_no_profile; }
#endif

#ifdef BUILD_DEBUG
extern int _nexus_linked_debug;
static int _nexus_defined_debug(void) { return _nexus_linked_debug; }
#else
extern int _nexus_linked_no_debug;
static int _nexus_defined_debug(void) { return _nexus_linked_no_debug; }
#endif

#ifdef NEXUS_SANITY_CHECK
extern int _nexus_linked_sanity;
static int _nexus_defined_sanity(void) { return _nexus_linked_sanity; }
#else
extern int _nexus_linked_no_sanity;
static int _nexus_defined_sanity(void) { return _nexus_linked_no_sanity; }
#endif

/* two levels of static function calls are needed in order to fool
   the IBM AIX 3.2 compilers and/or linker into not optimizing away
   the linkguards (jwg).
*/
static int _nexus_force_resolution(void)
{
	return _nexus_defined_profile() + _nexus_defined_debug() +
		_nexus_defined_sanity();
}
#endif /* _NEXUS_INCLUDE_NX_VERSION_H */
