/*
 * defs.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/defs.h,v 1.21 1996/10/22 03:07:42 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_DEFS_H
#define _NEXUS_INCLUDE_DEFS_H

/*
 * A magic number used in debugging nexus buffers
 */
#ifdef BUILD_DEBUG
#define NEXUS_BUFFER_MAGIC 12344321
#endif

/*
 * MAX_NEW_PROCESS_PARAMS
 *
 * Maximum length (in bytes) that the new_process_params string can be.
 */
#define MAX_NEW_PROCESS_PARAMS 1024

/*
 * NEXUS_MAX_COMMAND_LENGTH
 *
 * Maximum length of any command strings that are created.
 */
#define NEXUS_MAX_COMMAND_LENGTH 1024

/*
 * NEXUS_HANDLER_HASH_TABLE_SIZE
 *
 * The size of the hash table.  The hash value for handler names
 * is the sum of the ASCII values of the characters of the
 * name, modulo NEXUS_HANDLER_HASH_TABLE_SIZE.
 *
 * This should not be changed arbitrarily, since compilers need
 * to know this value in order to pre-compute hash values.
 */
#define NEXUS_HANDLER_HASH_TABLE_SIZE 1021

#ifndef NEXUS_DATABASE_PREFIX
#define NEXUS_DATABASE_PREFIX "resource_database"
#endif

#define NEXUS_DATABASE_PREFIX_SIZE (sizeof(NEXUS_DATABASE_PREFIX)-1)

#ifndef NEXUS_DBFILE
#define NEXUS_DBFILE ".resource_database"
#endif

#define NEXUS_TRANSFORM_TABLE_SIZE 256

#endif /* _NEXUS_INCLUDE_DEFS_H */
