/*
 * rdb_iface.c
 *
 * Database interface routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/rdb/rdb_iface.c,v 1.10 1996/10/30 03:03:52 tuecke Exp $";

#include "internal.h"

#define RDB_HASH_SIZE    1021

/*
 * parsing functions
 */
static char *parse_string_until_space(char *string, char **token);

/*
 * database hash table & functions
 */
static nexus_rdb_hash_entry_t **hash_table;
static int hash_function(char *name);
static void hash_table_add_nonexistent_key(char *name, char *key);

/*
 * misc.
 */
static void rdb_add_to_node_list(nexus_list_t **node_list, char *name);
static nexus_bool_t rdb_init = NEXUS_FALSE;

/*
 * Database module list
 */
typedef struct _rdb_module_list_t
{
    char *name;
    nexus_rdb_funcs_t *funcs;
} rdb_module_list_t;

static nexus_list_t *rdb_module_list_head;
static nexus_list_t *rdb_module_list_tail;

#define AddRDBModuleToList(Caller, Name, Funcs) \
{ \
    rdb_module_list_t *rdb_module; \
    nexus_list_t *tmp; \
 \
    NexusMalloc(Caller, \
    		rdb_module, \
    		rdb_module_list_t *, \
    		sizeof(rdb_module_list_t)); \
    NexusMalloc(Caller, \
		tmp, \
		nexus_list_t *, \
		sizeof(nexus_list_t)); \
    rdb_module->name = _nx_copy_string(Name); \
    rdb_module->funcs = (Funcs); \
    tmp->value = (void *)rdb_module; \
    tmp->next = NULL; \
 \
    if (rdb_module_list_head) \
    { \
	rdb_module_list_tail->next = tmp; \
	rdb_module_list_tail = tmp; \
    } \
    else \
    { \
	rdb_module_list_tail = rdb_module_list_head = tmp; \
    } \
}

/*
 * nexus_rdb_init()
 */
void _nx_rdb_init(int *argc,
		  char ***argv,
		  nexus_module_list_t rdb_module_list[])
{
    nexus_list_t *rdb_module;
    nexus_rdb_funcs_t *rdb_funcs;
    int i;

    /*
     * Initialize the hash table for caching the db
     */
    NexusMalloc(nexus_rdb_init(),
	        hash_table,
	        nexus_rdb_hash_entry_t **,
	        sizeof(nexus_rdb_hash_entry_t *) * RDB_HASH_SIZE);

    for(i = 0; i < RDB_HASH_SIZE; i++)
    {
        hash_table[i] = NULL;
    }

    /*
     * Call the info function for each module to get its function table.
     */
    rdb_module_list_head = rdb_module_list_tail = NULL;
    for (i = 0; rdb_module_list[i].family_name != NULL; i++)
    {
	if (strcmp(rdb_module_list[i].family_name, "rdb") == 0)
	{
	    rdb_funcs = (nexus_rdb_funcs_t *)(*rdb_module_list[i].info_func)();
	    AddRDBModuleToList(nexus_rdb_init(),
			       rdb_module_list[i].module_name,
			       rdb_funcs);
	}
    }

    /*
     * Initialize each module
     */
    for (rdb_module = rdb_module_list_head;
	 rdb_module;
	 rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->init)
	{
	    (((rdb_module_list_t *)rdb_module->value)->funcs->init)(argc, argv);
	}
    }

    rdb_init = NEXUS_TRUE;
} /* nexus_rdb_init() */


/*
 * nexus_rdb_shutdown()
 */
void _nx_rdb_shutdown(void)
{
    nexus_rdb_hash_entry_t *entry, *next_entry;
    nexus_list_t *attr, *next_attr;
    nexus_list_t *rdb_module;
    int i;

    /* We have LOTS and LOTS of memory to deallocate here */

    for (rdb_module = rdb_module_list_head;
	 rdb_module;
	 rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->shutdown)
	{
	    (((rdb_module_list_t *)rdb_module->value)->funcs->shutdown)();
	}
    }

    /* Deallocate hash_table, now */
    for (i = 0; i < RDB_HASH_SIZE; i++)
    {
	for (entry = hash_table[i]; entry; entry = next_entry)
	{
	    next_entry = entry->next;
	    NexusFree(entry->name);
	    for (attr = entry->attr; attr; attr = next_attr)
	    {
		next_attr = attr->next;
		NexusFree(attr->value);
	    }
	    NexusFree(entry->attr);
	    NexusFree(entry);
	}
    }
} /* nexus_rdb_shutdown() */


/*
 * nexus_rdb_abort()
 */
void _nx_rdb_abort(void)
{
    nexus_list_t *rdb_module;

    /* Don't deallocate memory--just STOP! */
    for (rdb_module = rdb_module_list_head;
	 rdb_module;
	 rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->abort)
	{
	    (((rdb_module_list_t *)rdb_module->value)->funcs->abort)();
	}
    }
} /* nexus_rdb_abort() */


/*
 * nexus_rdb_usage_message()
 *
 * Call the usage_message() function for each protocol
 */
void _nx_rdb_usage_message(void)
{
    nexus_list_t *rdb_module;

    for (rdb_module = rdb_module_list_head;
        rdb_module;
	rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->usage)
	{
	    (*((rdb_module_list_t *)rdb_module->value)->funcs->usage)();
	}
    }
} /* nexus_rdb_usage_message() */


/*
 * nexus_rdb_new_process_params()
 *
 * Call the new_process_params() function for each protocol
 *
 * Each of those functions may add stuff to 'buf', returning the number
 * of characters that they added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int _nx_rdb_new_process_params(char *buf, int size)
{
    nexus_list_t *rdb_module;
    int n_left = size;
    char *b = buf;
    int n_added;
    
    for (rdb_module = rdb_module_list_head;
	 rdb_module;
	 rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->new_params)
	{
	    n_added = (((rdb_module_list_t *)rdb_module->value)->funcs->new_params)(b, n_left);
	    b += n_added;
	    n_left -= n_added;
	}
    }

    return (size - n_left);
} /* nexus_rdb_new_process_params() */


/*
 * nexus_rdb_lookup()
 *
 * Lookup 'node_name' in the node info database, and find the
 * information for the given 'key' type.
 *
 * Return: An RdbMalloc()'d string containing the value.
 *	   Return NULL if this lookup request cannot be satisfied.
 *
 * Note: The returned value should be passed to nexus_database_free()
 * after it is no longer needed.
 */
char *nexus_rdb_lookup(char *name, char *key)
{
    nexus_list_t *rdb_module;
    char *value;

    if (!rdb_init)
    {
	/* 
  	 * Nothing is in the DB, and something will seg fault if we try
         * to access the unallocated hash table
         */
	return NULL;
    }

    nexus_debug_printf(1, ("nexus_rdb_lookup(): looking up \"%s\" for %s\n", key, name));
    if (_nx_rdb_hash_table_lookup(name, key, &value))
    {
	if (value)
	{
	    nexus_debug_printf(2, ("nexus_rdb_lookup(): value (%s) found\n", value));
	    return(_nx_copy_string(value));
	}
	else
	{
	    /* Got a cache hit for a non-existent key */
	    return(NULL);
	}
    }

    nexus_debug_printf(2, ("nexus_rdb_lookup(): value not found in hash table--looking in other modules\n"));
    for (rdb_module = rdb_module_list_head;
	 rdb_module;
	 rdb_module = rdb_module->next)
    {
	if (((rdb_module_list_t *)rdb_module->value)->funcs->lookup)
	{
	    if ((value = (((rdb_module_list_t *)rdb_module->value)->funcs->lookup)(name, key)))
	    {
		nexus_debug_printf(2, ("nexus_rdb_lookup(): value (%s) found\n", value));
		return (_nx_copy_string(value));
	    }
	}
    }

    /*
     * Put misses in cache, too.  We can save a lot of time for
     * functions that repeatedly call this with the same key like
     * tcp_interface.
     */
    hash_table_add_nonexistent_key(name, key);
    nexus_debug_printf(2, ("nexus_rdb_lookup(): value not found\n"));

    return (NULL);
} /* nexus_rdb_lookup */


/*
 * nexus_rdb_free()
 */
void nexus_rdb_free(char *value)
{
    if (value == (char *) NULL)
	return;
    
    NexusFree(value);
} /* nexus_rdb_free() */


/*
 * nexus_rdb_get_names()
 */
nexus_list_t *nexus_rdb_get_names(char *file)
{
    nexus_list_t *names = NULL;
    FILE *fp;

    if ((fp = fopen(file, "r")) == NULL)
    {
	return NULL;
    }
#define _RDB_GET_NAMES
#include "rdb_file_parser.c"
#undef _RDB_GET_NAMES

    return names;
} /* nexus_rdb_get_names() */


/*
 * nexus_rdb_free_names()
 */
void nexus_rdb_free_names(nexus_list_t *names)
{
    nexus_list_t *cur_name, *next_name;

    for (cur_name = names; cur_name; cur_name = next_name)
    {
	next_name = cur_name->next;
	NexusFree(cur_name->value);
	NexusFree(cur_name);
    }
}


nexus_bool_t _nx_rdb_hash_table_lookup(char *name,
				       char *key,
				       char **value)
{
    nexus_rdb_hash_entry_t *entry;
    nexus_list_t *cur_value;
    int key_size = (int)strlen(key);

    for (entry = hash_table[hash_function(name)];
    	 entry;
	 entry=entry->next)
    {
        if (strcmp(entry->name, name) == 0)
	{
            for (cur_value=entry->attr; cur_value; cur_value=cur_value->next)
            {
                if (strncmp(key, (char *)cur_value->value, key_size) == 0)
                {
               	    if (((char *)cur_value->value)[key_size] == '=')
                    {
		       *value = (char *)(cur_value->value) + key_size + 1;
		       return NEXUS_TRUE;
                    }
                    else if (((char *)cur_value->value)[key_size] == '\0')
                    {
		       *value = (char *)(cur_value->value) + key_size;
		       return NEXUS_TRUE;
                    }
		    else if (((char *)cur_value->value)[key_size] == ' ')
		    {
			*value = NULL;
			return NEXUS_TRUE;
		    }
        	}
    	    }
	}
    }
    *value = NULL;
    return NEXUS_FALSE;
} /* _nx_rdb_hash_table_lookup() */

void _nx_rdb_hash_table_add(char *name, nexus_list_t *attr)
{
    nexus_rdb_hash_entry_t *cur_entry, *entry;
    nexus_list_t *cur_attr;
    int hash;

    hash = hash_function(name);
    for (cur_entry = hash_table[hash];
         cur_entry;
	 cur_entry = cur_entry->next)
    {
	if (strcmp(name, cur_entry->name) == 0)
	{
	    for (cur_attr = cur_entry->attr;
	    	 cur_attr && cur_attr->next;
		 cur_attr = cur_attr->next) 
	        /* do nothing */ ;
	    if (cur_attr)
	    {
	        cur_attr->next = attr;
	    }
	    else
	    {
	        cur_attr = attr;
	    }
	    return;
        }
    }

    for (cur_entry = hash_table[hash];
         cur_entry && cur_entry->next;
	 cur_entry = cur_entry->next) 
	/* do nothing */ ;
    NexusMalloc(_nx_hash_table_add(),
	        entry,
		nexus_rdb_hash_entry_t *,
		sizeof(nexus_rdb_hash_entry_t));
    entry->name = _nx_copy_string(name);
    entry->attr = attr;
    entry->next = NULL;
    if (cur_entry)
    {
        cur_entry->next = entry;
    }
    else
    {
        hash_table[hash] = entry;
    }
} /* _nx_rdb_hash_table_add() */

static int hash_function(char *name)
{
    char *i;
    int sum;

    sum = 0;
    for (i=name; *i; i++)
    {
	sum += *i;
    }
    return (sum % RDB_HASH_SIZE);
} /* hash_function */

static char *parse_string_until_space(char *s, char **token)
{
    while ( *s != '\n' && isspace(*s) )
        s++;
    *token = s;
    while (( *s != '\0' ) && ( !isspace(*s) ))
        s++;
    if (*s != '\0')
        *s++ = '\0';
    return (s);
} /* parse_string_until_space() */


static void rdb_add_to_node_list(nexus_list_t **node_list, char *name)
{
    nexus_list_t *new_node;

    NexusMalloc(rdb_add_to_node_list(),
	        new_node,
	        nexus_list_t *,
	        sizeof(nexus_list_t));

    new_node->value = (void *)_nx_copy_string(name);
    new_node->next = NULL;
    if (*node_list)
    {
	nexus_list_t *cur_node;

	for(cur_node = *node_list; cur_node->next; cur_node = cur_node->next) 
	    /* do nothing */ ;
	cur_node->next = new_node;
    }
    else
    {
	*node_list = new_node;
    }
} /* rdb_add_to_node_list() */


void _nx_rdb_hash_table_add_nodes_with_attrs(nexus_rdb_hash_entry_t *nodes,
					     nexus_list_t *attrs)
{
    nexus_rdb_hash_entry_t *i;

    for (i = nodes; i; i = i->next)
    {
        nexus_list_t *j;

	for (j = attrs; j; j = j->next)
	{
	    nexus_list_t *temp_attr;

	    NexusMalloc(_nx_hash_table_add_nodes_with_attrs(),
			temp_attr,
			nexus_list_t *,
			sizeof(nexus_list_t));
	    temp_attr->value = j->value;
	    temp_attr->next = NULL;
            _nx_rdb_hash_table_add(i->name, temp_attr);
	}
    }
} /* _nx_rdb_hash_table_add_nodes_with_attrs() */

/* 
 * Enter key that has failed in all database modules into the hash.
 * This means that next time we will find the miss in the cache and not
 * have to look through all the modules every time a miss is being
 * searched for.
 *
 * We do this by adding a space at the end of the key.  This is an
 * illegal construct in a real database as spaces signify separators
 * between keys.
 */
static void hash_table_add_nonexistent_key(char *name, char *key)
{
    nexus_list_t *attr;

    NexusMalloc(_nx_hash_table_add_nonexistent_key(),
                attr,
                nexus_list_t *,
                sizeof(nexus_list_t));
    NexusMalloc(_nx_hash_table_add_nonexistent_key(),
                attr->value,
                char *,
                sizeof(char) * (strlen(key) + 2));
    sprintf(attr->value, "%s ", key);
    attr->next = NULL;
    _nx_rdb_hash_table_add(name, attr);
} /* hash_table_add_nonexistent_key() */


nexus_list_t *_nx_rdb_parse_attributes(char *buf,
				       int buf_len,
				       char *starting_point,
				       FILE *fp)
{
    nexus_list_t *attr;
    char *string;
    nexus_bool_t new_line;

    attr = NULL;
    string = starting_point;
    new_line = NEXUS_FALSE;

    /* 
     * read buf until we hit end of entry--this may include reading
     * multiple lines from fp. 
     */
    while(*string)
    {
        char *tmp;
        char *cur_attr;
	char *tuple;
	nexus_list_t *pos;
	char *start_quote;
	nexus_bool_t quote;

	quote = NEXUS_FALSE;
	start_quote = NULL;

	/* 
  	 * Get next key/value pair
         */
	tmp = parse_string_until_space(string, &tuple);
	if ((start_quote = strchr(tuple, '\"')))
	{
	    char *point;
	    int num_quotes = 0;
	    nexus_bool_t done;

	    start_quote = _nx_copy_string(tuple);
	    quote = NEXUS_FALSE;
	    /* will be changed TRUE on first quote of the while loop */

	    done = NEXUS_FALSE;
	    /* 
      	     * count quotes in current string and keep track of matching
             * pairs
             */
	    for (point = start_quote; !done; point++)
	    {
		if (*(point + num_quotes) == '\"')
		{
		    quote = !quote;
		    num_quotes++;
		}
		/* copy over quote(s) */
		*point = *(point + num_quotes);
		if (!*point)
		{
		    /* we have hit the end of the string */
		    done = NEXUS_TRUE;
		}
	    }
	    string = tmp;
	}

	/* 
  	 * If we go into this loop, we have hit a key/value pair that
         * has multiple words...continue until we find the last word in
         * the sequence
         */
	while (quote)
	{
	    char *temp_string;

	    tmp = parse_string_until_space(string, &tuple);

	    if (tuple[strlen(tuple) - 1] == '\"')
	    {
		/* 
   		 * we hit the end of the key/value pair
      		 */
		quote = NEXUS_FALSE;
		tuple[strlen(tuple) - 1] = '\0';
	    }

	    /* 
      	     * concatenate all words retrieved so far with the current
             * tuple
             */
	    NexusMalloc(_nx_parse_attributes(),
			temp_string,
			char *,
			sizeof(char) * (strlen(start_quote) + 1 /* space */ + strlen(tuple) + 1 /* '\0' */));
	    strcpy(temp_string, start_quote);
	    NexusFree(start_quote);
	    strcat(temp_string, " ");
	    strcat(temp_string, tuple);
	    start_quote = _nx_copy_string(temp_string);
	    NexusFree(temp_string);

	    string = tmp;
	    tuple = start_quote;
	}

	if (*tuple)
	{
	    /* 
      	     * Add key/value pair to the attr linked list
             */
            NexusMalloc(_nx_parse_attributes(),
	                pos,
		        nexus_list_t *,
		        sizeof (nexus_list_t));
	    if (tuple[strlen(tuple)-1] == '\\')
	    {
		new_line = NEXUS_TRUE;
		tuple[strlen(tuple)-1] = '\0';
	    }
	    cur_attr = _nx_copy_string(tuple);
            pos->value = (void *)cur_attr;
            pos->next = attr;
            attr = pos;
	}
	if (new_line || *tmp == '\\')
	{
	    /* 
      	     * read next line in the file
             */
	    fgets(buf, buf_len, fp);
	    string = buf;
	    new_line = NEXUS_FALSE;
	}
	else
	{
            string = tmp;
	}
    }
    return (attr);
} /* _nx_rdb_parse_attributes() */

void _nx_rdb_flush_rest_of_line(char *buf,
				int buf_len,
				char *starting_point,
				FILE *fp)
{
    while(starting_point[strlen(starting_point)-2] == '\\')
    {
	fgets(buf, buf_len, fp);
    }
    starting_point = buf;
} /* _nx_rdb_flush_rest_of_line() */
