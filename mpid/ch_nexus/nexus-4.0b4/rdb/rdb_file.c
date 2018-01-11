/*
 * rdb_file.c
 *
 * Node database interface routines from ascii file
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/rdb/rdb_file.c,v 1.6 1996/10/07 18:15:41 tuecke Exp $";

#include "internal.h"

static nexus_bool_t	rdb_file_init(int *argc, char ***argv);
static void		rdb_file_usage_message(void);
static int 		rdb_file_new_process_params(char *buf, int size);
static char *		rdb_file_lookup(char *node_name, char *attr);
static void		rdb_file_shutdown(void);

static char *           arg_dbfile = (char *)NULL;

/*
 * database parsing stuff
 */
static void             rdb_file_name_found(nexus_bool_t *name_found,
					    char *node_name,
					    char **real_name,
					    char *name_ptr,
					    nexus_rdb_hash_entry_t *node);

/*
 * database file list
 */
typedef struct _rdb_file_list_t
{
    char *name;
    FILE *fp;
    struct _rdb_file_list_t *next;
} rdb_file_list_t;

static rdb_file_list_t *rdb_file_list_head;
static rdb_file_list_t *rdb_file_list_tail;

#define AddDBFileToList(Caller, Name, Fp) \
{ \
    rdb_file_list_t *temp; \
    NexusMalloc(Caller, \
		temp, \
		rdb_file_list_t *, \
		sizeof(rdb_file_list_t)); \
    temp->name = _nx_copy_string(Name); \
    temp->fp = (Fp); \
    temp->next = NULL; \
    if (rdb_file_list_head) \
    { \
	rdb_file_list_tail->next = temp; \
	rdb_file_list_tail = temp; \
    } \
    else \
    { \
	rdb_file_list_head = rdb_file_list_tail = temp; \
    } \
}


static nexus_rdb_funcs_t rdb_file_funcs =
{
    rdb_file_init,
    rdb_file_usage_message,
    rdb_file_new_process_params, 
    rdb_file_lookup,
    rdb_file_shutdown,
    NULL /* rdb_file_abort */,
};


/*
 * _nx_rdb_file_info()
 *
 * Return the function table for this module.
 */
void *_nx_rdb_file_info()
{
    return ((void *)(&rdb_file_funcs));
} /* _nx_database_file_info() */


/*
 * rdb_file_usage_message()
 */
static void rdb_file_usage_message()
{
    printf("    -dbfile <file>    :<file> is filename of database to use.\n");
} /* rdb_file_usage_message() */


/* 
 * rdb_file_new_process_params()
 */
static int rdb_file_new_process_params(char *buf, int size)
{
    int rc = 0;

    nexus_stdio_lock();

    if (arg_dbfile)
    {
	rc = 8 + strlen(arg_dbfile);

	if (rc > size)
	{
	    nexus_fatal("rdb_file_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
	}

	sprintf(buf, "-dbfile %s ", arg_dbfile);
    }

    nexus_stdio_unlock();

    return (rc);
} /* rdb_file_new_process_params() */

/*
 * rdb_file_init()
 */
static nexus_bool_t rdb_file_init(int *argc, char ***argv)
{
    int arg_num;
    char *home;
    char *global_database;
    char *filename;
    FILE *fp;
    char *rdb_env_file;
    char *rdb_path_to_file;
    char rdb_file_path[MAX_PATH_LENGTH];

    rdb_file_list_head = rdb_file_list_tail = NULL;

    /* 
     * figure out what path specifications exist for the database file
     * NEXUS_DBFILE environment variable path set by nexus_set_path()
     */
    rdb_env_file = getenv("NEXUS_DBFILE");
    rdb_path_to_file = _nx_path_array[NEXUS_PATH_TYPE_DATABASE];

    /* look for -dbfile flags and add them to db_file_list */
    if ((arg_num = nexus_find_argument(argc, argv, "dbfile", 2)) >= 0)
    {
	char *next, *filename;

	next = (*argv)[arg_num + 1];
	arg_dbfile = _nx_copy_string(next);
	while(next)
	{
	    _nx_get_next_value(next, ':', &next, &filename);
	    if ((fp = fopen(filename, "r")) == NULL)
	    {
	        nexus_fatal("rdb_file_init(): Can't open database file %s\n", filename);
	    }
	    AddDBFileToList(rdb_file_init(), filename, fp);
	}
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }

    /* now use environment variable, if present */
    if (rdb_env_file && (fp = fopen(rdb_env_file, "r")))
    {
	AddDBFileToList(rdb_file_init(), rdb_env_file, fp);
    }

    /* look at the database in the current directory */
    if ((fp = fopen(NEXUS_DBFILE, "r")) != NULL)
    {
        AddDBFileToList(rdb_file_init(), NEXUS_DBFILE, fp);
    }

    /* look at the database in the home directory */
    if ((home = getenv("HOME")) != (char *) NULL)
    {
	NexusMalloc(rdb_file_init(),
		    filename,
		    char *,
		    sizeof(char) *
		    (strlen(home) + 1 + strlen(NEXUS_DBFILE) + 1));
	filename[0] = '\0';
	strcat(filename, home);
	strcat(filename, "/");
	strcat(filename, NEXUS_DBFILE);
	if ((fp = fopen(filename, "r")) != NULL)
	{
	    AddDBFileToList(rdb_file_init(), filename, fp);
	}
    }

    if (rdb_path_to_file)
    {
	sprintf(rdb_file_path, "%s/%s",
	    rdb_path_to_file, NEXUS_DBFILE);
	
	if ((fp = fopen(rdb_file_path, "r")))
	{
	    AddDBFileToList(rdb_file_init(), rdb_file_path, fp);
	}
    }
    
    /* look at the global database */
    global_database = getenv("NEXUS_GLOBAL_DBFILE");
#ifdef NEXUS_GLOBAL_DBFILE
    if (!global_database)
    {
	global_database = _nx_copy_string(NEXUS_GLOBAL_DBFILE);
    }
#endif /* RESOURCE_DATABASE_GLOBAL_FILE */
    if (global_database && (fp = fopen(global_database, "r")) != NULL)
    {
        AddDBFileToList(rdb_file_init(), global_database, fp);
    }

    return(NEXUS_TRUE);
} /* rdb_file_init() */


/*
 * rdb_file_lookup()
 *
 * Lookup 'node_name' in the node info database, and find the
 * information for the given 'module' type.
 *
 * Return: A NexusMalloc()'ed string
 *	   Return NULL if this lookup request cannot be satisfied.
 *
 * Note: The returned values should be passed to resource_database_free()
 * after it is no longer needed.
 */
static char *rdb_file_lookup(char *node_name, char *key)
{
    rdb_file_list_t *rdb_file;

    nexus_stdio_lock();
    
    for (rdb_file = rdb_file_list_head;
         rdb_file;
	 rdb_file = rdb_file->next)
    {
	/* rewind database file */
	rewind(rdb_file->fp);

#define _RDB_FILE_LOOKUP
#include "rdb_file_parser.c"
#undef _RDB_FILE_LOOKUP

    }
    
    nexus_stdio_unlock();
    return(NULL);
} /* rdb_file_lookup */


/*
 * rdb_file_shutdown()
 */
static void rdb_file_shutdown()
{
    rdb_file_list_t *db_file;

    for (db_file = rdb_file_list_head; db_file; db_file = db_file->next)
    {
	nexus_stdio_lock();
	fclose(db_file->fp);
	nexus_stdio_unlock();
    }
    NexusFree(arg_dbfile);
} /* rdb_file_shutdown() */

/*
 * rdb_file_name_found()
 *
 * dereferenced variables have ()'s around them for quick conversion to
 * a macro.  It may be faster for this to be a macro and not a function.
 * If this is the case, it should be put before rdb_file_lookup().
 */
static void rdb_file_name_found(nexus_bool_t *name_found,
				char *node_name,
				char **real_name,
				char *name_ptr,
				nexus_rdb_hash_entry_t *node)
{ 
    if (!*(name_found)) 
    { 
	if (!*(real_name)) 
	{ 
	    *(real_name) = _nx_copy_string(name_ptr); 
	} 
	if (strcmp(*(real_name), node_name) == 0) 
	{ 
	    *(name_found) = NEXUS_TRUE; 
	    (node)->name = _nx_copy_string(*(real_name)); 
	    (node)->next = NULL; 
	    (node)->attr = NULL;
	} 
    } 
}
