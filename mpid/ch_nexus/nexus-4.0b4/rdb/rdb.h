/*
 * rdb.h
 *
 * This header contains the exported interface of the resource database.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/rdb/rdb.h,v 1.4 1996/08/19 19:58:52 geisler Exp $"
 */

#ifndef _RDB_H
#define _RDB_H

/*
 * EXTERN_C_BEGIN and EXTERN_C_END should surround any Rdb prototypes in
 * rdb.h or Rdb .h files that are included by rdb.h.  This will
 * allow C++ codes to include rdb.h and properly link against the
 * Rdb library.
 */
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

extern void     nexus_database_init(int *argc, char **argv[]);

extern char *	nexus_database_lookup(char *name, char *key);

extern void	nexus_database_free(char *value);

extern nexus_list_t *nexus_database_get_names(char *file);

extern void	nexus_database_free_names(nexus_list_t *names);

extern int	nexus_database_shutdown(void);

extern void	nexus_database_abort(void);

extern void	nexus_database_usage_message(void);

extern int	nexus_database_new_process_params(char *buf, int size);

EXTERN_C_END

#endif /* _RDB_H */
