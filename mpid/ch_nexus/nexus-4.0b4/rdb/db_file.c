/*
 * db_file.c
 *
 * Node database interface routines from ascii file
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/rdb/db_file.c,v 1.14 1995/11/06 22:16:54 tuecke Exp $";

#include "internal.h"

static nexus_bool_t	database_file_init(int *argc, char ***argv);
static void		database_file_usage_message(void);
static int		database_file_new_process_params(char *buf, int size);
static char *		database_file_lookup(char *node_name,
				             int node_number,
				             char *attr);
static void		database_file_shutdown(void);

static char *		arg_dbfile = (char *) NULL;

/*
 * database parsing stuff
 */
static void             database_file_name_found(nexus_bool_t *name_found,
					         char *node_name,
					         char **real_name,
					         char *name_ptr,
					         nexus_db_hash_entry_t *node,
					         int number,
					         int count);

/*
 * database file list
 */
typedef struct _db_file_list_t
{
    char *name;
    FILE *fp;
    struct _db_file_list_t *next;
} db_file_list_t;

static db_file_list_t *db_file_list_head;
static db_file_list_t *db_file_list_tail;

#define AddDBFileToList(Caller, Name, Fp) \
{ \
    db_file_list_t *temp; \
    NexusMalloc(Caller, \
		temp, \
		db_file_list_t *, \
		sizeof(db_file_list_t)); \
    temp->name = _nx_copy_string(Name); \
    temp->fp = Fp; \
    temp->next = NULL; \
    if (db_file_list_head) \
    { \
	db_file_list_tail->next = temp; \
	db_file_list_tail = temp; \
    } \
    else \
    { \
	db_file_list_head = db_file_list_tail = temp; \
    } \
}


static nexus_database_funcs_t database_file_funcs =
{
    database_file_init,
    database_file_usage_message,
    database_file_new_process_params,
    database_file_lookup,
    database_file_shutdown,
    NULL /* database_file_abort */,
};


/*
 * _nx_database_file_info()
 *
 * Return the function table for this module.
 */
void *_nx_database_file_info(void)
{
    return ((void *)(&database_file_funcs));
}


/*
 * database_file_usage_message()
 */
static void database_file_usage_message(void)
{
    printf("    -dbfile <file>    :<file> is filename of database to use.\n");
} /* database_file_usage_message() */


/*
 * database_file_new_process_params()
 *
 * Return: The total number of characters added to 'buf'.
 */
static int database_file_new_process_params(char *buf, int size)
{
    if (arg_dbfile)
    {
	if ((strlen(arg_dbfile) + 16) > size)
	{
	    nexus_fatal("database_file_new_process_params(): Internal error: Not enough room in buffer for arguments\n");
	}

	nexus_stdio_lock();
	sprintf(buf, "-dbfile %s ", arg_dbfile);
	nexus_stdio_unlock();
    }
    
    return (strlen(buf));
    
} /* database_file_new_process_params() */


/*
 * database_file_init()
 */
static nexus_bool_t database_file_init(int *argc, char ***argv)
{
    int arg_num;
    char *home;
    char *global_database;
    char *filename;
    FILE *fp;
    char *db_env_file;
    char db_path_file[512];
    char *db_path_to_file;

    db_file_list_head = db_file_list_tail = NULL;

    /*
     * figure out what path specifications exist for the database file
     * NEXUS_DATABASE environment variable
     * path set by nexus_set_path()
     */
    db_env_file = getenv("NEXUS_DATABASE_FILE");
    db_path_to_file = _nx_path_array[NEXUS_PATH_TYPE_LISTENER];

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
		continue;
	    }
	    AddDBFileToList(database_file_init(), filename, fp);
	}
	nexus_remove_arguments(argc, argv, arg_num, 2);
    }
    /* now use environment variable if present */
    if (db_env_file && (fp=fopen(db_env_file, "r")))
    {
	AddDBFileToList(database_file_init(), db_env_file, fp);
    }

    /* look at the database in the current directory */
    if ((fp = fopen(NEXUS_DATABASE_FILE, "r")) != NULL)
    {
        AddDBFileToList(database_file_init(), NEXUS_DATABASE_FILE, fp);
    }

    /* look at the database in the home directory */
    if ((home = getenv("HOME")) != (char *) NULL)
    {
	NexusMalloc(database_file_init(),
		    filename,
		    char *,
		    sizeof(char) *
		    (strlen(home) + 1 + strlen(NEXUS_DATABASE_FILE) + 1));
	filename[0] = '\0';
	strcat(filename, home);
	strcat(filename, "/");
	strcat(filename, NEXUS_DATABASE_FILE);
	if ((fp = fopen(filename, "r")) != NULL)
	{
	    AddDBFileToList(database_file_init(), filename, fp);
	}
    }
    
    if (db_path_to_file)
    {
	sprintf(db_path_file, "%s/%s", db_path_to_file, NEXUS_DATABASE_FILE);
	if((fp=fopen(db_path_file, "r")))
	{
	    AddDBFileToList(database_file_init(), db_path_file, fp);
	}
    }
    
    /* look at the global database */
    global_database = getenv("NEXUS_GLOBAL_DATABASE_FILE");
#ifdef NEXUS_GLOBAL_DATABASE_FILE
    if (!global_database)
    {
	global_database = NEXUS_GLOBAL_DATABASE_FILE;
    }
#endif /* NEXUS_GLOBAL_DATABASE_FILE */
    if (global_database && (fp = fopen(global_database, "r")) != NULL)
    {
        AddDBFileToList(database_file_init(), global_database, fp);
    }

    return(NEXUS_TRUE);
} /* database_file_init() */


/*
 * database_file_lookup()
 *
 * Lookup 'node_name' in the node info database, and find the
 * information for the given 'module' type.
 *
 * Return: A NexusMalloc()ed string
 *	   Return NULL if this lookup request cannot be satisfied.
 *
 * Note: The returned values should be passed to nexus_database_free() after
 * it is no longer needed.
 */
static char *database_file_lookup(char *node_name, int node_number, char *key)
{
    db_file_list_t *db_file;

    nexus_stdio_lock();
    
    for (db_file = db_file_list_head; db_file; db_file = db_file->next)
    {
	/* rewind database file */
	rewind(db_file->fp);

#define _NX_DB_FILE_LOOKUP
#include "db_file_parser.c"
#undef _NX_DB_FILE_LOOKUP

    }
    
    nexus_stdio_unlock();
    return(NULL);
} /* database_file_lookup */


/*
 * database_file_shutdown()
 */
static void database_file_shutdown(void)
{
    db_file_list_t *db_file;

    for (db_file = db_file_list_head; db_file; db_file = db_file->next)
    {
	nexus_stdio_lock();
	fclose(db_file->fp);
	nexus_stdio_unlock();
    }
} /* database_file_shutdown() */

/*
 * database_file_name_found()
 *
 * dereferenced variables have ()'s around them for quick conversion to
 * a macro.  It may be faster for this to be a macro and not a function.
 * If this is the case, it should be put before database_file_lookup().
 */
static void database_file_name_found(nexus_bool_t *name_found,
			             char *node_name,
			             char **real_name,
			             char *name_ptr,
			             nexus_db_hash_entry_t *node,
			             int number,
			             int count)
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
	    (node)->number = number; 
	    (node)->count = count; 
	    (node)->next = NULL; 
	    (node)->attr = NULL;
	} 
    } 
}
