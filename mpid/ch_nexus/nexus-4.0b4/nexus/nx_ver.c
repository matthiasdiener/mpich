/*
 * nx_ver.c
 *
 * Code to make sure compiled and linked code are all the same version.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/nx_ver.c,v 1.6 1996/10/07 04:40:06 tuecke Exp $";

#include "internal.h"

#ifdef BUILD_PROFILE
int _nexus_linked_profile = 0;
#else
int _nexus_linked_no_profile = 0;
#endif

#ifdef BUILD_DEBUG
int _nexus_linked_debug = 0;
#else
int _nexus_linked_no_debug = 0;
#endif

#ifdef NEXUS_SANITY_CHECK
int _nexus_linked_sanity = 0;
#else
int _nexus_linked_no_sanity = 0;
#endif

