/*
 * db_iface.c
 *
 * Node database interface routines
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/rdb/db_iface.c,v 1.22 1995/11/15 20:07:13 geisler Exp $";

#include "internal.h"

#define DB_HASH_SIZE    100

/*
 * parsing functions
 */
static char *parse_string_until_space(char *string, char **token);


/*
 * database hash table & functions
 */
static nexus_db_hash_entry_t **hash_table;
static int hash_function(char *name, int number);

/*
 * misc.
 */
static nexus_bool_t db_init = NEXUS_FALSE;

/*
 * Database module list
 */
typedef struct _database_module_list_t
{
    char *name;
    nexus_database_funcs_t *funcs;
} database_module_list_t;

static nexus_list_t *database_module_list_head;
static nexus_list_t *database_module_list_tail;

#define AddDatabaseModuleToList(Caller, Name, Funcs) \
{ \
    database_module_list_t *module; \
    nexus_list_t *temp; \
    \
    NexusMalloc(Caller, \
		module, \
		database_module_list_t *, \
		sizeof(database_module_list_t)); \
    NexusMalloc(Caller, \
		temp, \
		nexus_list_t *, \
		sizeof(nexus_list_t)); \
    module->name = _nx_copy_string(Name); \
    module->funcs = (Funcs); \
    temp->value = (void *)module; \
    temp->next = NULL; \
    if (database_module_list_head) \
    { \
	database_module_list_tail->next = temp; \
	database_module_list_tail = temp; \
    } \
    else \
    { \
	database_module_list_head = database_module_list_tail = temp; \
    } \
}

/*
 * _nx_database_usage_message()
 *
 * Call the usage_message() function for each protocol
 */
void _nx_database_usage_message(void)
{
    nexus_list_t *database_module;

    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
        if (((database_module_list_t *)database_module->value)->funcs->usage)
	{
	    (*((database_module_list_t *)database_module->value)->funcs->usage)();
	}
    }
} /* _nx_database_usage_message() */


/*
 * _nx_database_new_process_params()
 *
 * Call the new_process_params() function for each protocol
 *
 * Each of those functions may add stuff to 'buf', returning the number
 * of characters that they added.
 *
 * Return: The total number of characters added to 'buf'.
 */
int _nx_database_new_process_params(char *buf, int size)
{
    int n_left = size;
    char *b = buf;
    int n_added;
    nexus_list_t *database_module;
    
    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
	if (((database_module_list_t *)database_module->value)->funcs->new_params)
	{
	    n_added = (*((database_module_list_t *)database_module->value)->funcs->new_params)(b, n_left);
	    b += n_added;
	    n_left -= n_added;

	}
    }

    return (size - n_left);
} /* _nx_database_new_process_params() */


/*
 * _nx_database_init()
 */
void _nx_database_init(int *argc, char ***argv,
		       nexus_module_list_t module_list[])
{
    int i, rc;
    nexus_list_t *database_module;
    nexus_database_funcs_t *database_funcs;
    
    NexusMalloc(_nx_database_init(),
	        hash_table,
	        nexus_db_hash_entry_t **,
	        sizeof(nexus_db_hash_entry_t *) * DB_HASH_SIZE);
    for(i=0; i<DB_HASH_SIZE; i++)
    {
        hash_table[i] = NULL;
    }

    database_module_list_head = database_module_list_tail = NULL;
    for (i = 0; module_list[i].family_name != NULL; i++)
    {
	if (strcmp(module_list[i].family_name, "database") == 0)
	{
	    database_funcs =
	        (nexus_database_funcs_t *)(*module_list[i].info_func)();
	    /* Why does st_iface have this rc stuff? */
	    rc = 0;
	    if (rc == 0)
	    {
		AddDatabaseModuleToList(_nx_database_init(),
					module_list[i].module_name,
					database_funcs);
	    }
	}
    }

    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
	if (((database_module_list_t *)database_module->value)->funcs->init)
	{
	    (*((database_module_list_t *)database_module->value)->funcs->init)(argc, argv);
	}
    }

    db_init = NEXUS_TRUE;
} /* _nx_database_init() */


void nexus_database_free(char *value)
{
    if (value == (char *) NULL)
	return;
    
    NexusFree(value);
} /* nexus_database_free() */


/*
 * nexus_database_lookup()
 *
 * Lookup 'node_name' in the node info database, and find the
 * information for the given 'module' type.
 *
 * Return: A comma separated string of values in a NexusMalloc()'ed
 *            string.
 *	   Return NULL if this lookup request cannot be satisfied.
 *
 * Note: The returned values should be passed to nexus_database_free() after
 * it is no longer needed.
 */
char *nexus_database_lookup(char *name, int number, char *key)
{
    char *value;
    nexus_list_t *database_module;

    if (!db_init)
    {
	/*
	 * Nothing is in the DB, and something will seg fault if we try
	 * to access the unallocated hash table
	 */
	return NULL;
    }
    nexus_debug_printf(1,("nexus_database_lookup(): looking up \"%s\" for %s#%d\n", key, name, number));
    if ((value = _nx_database_hash_table_lookup(name, number, key)))
    {
	return (_nx_copy_string(value));
    }

    nexus_debug_printf(2, ("nexus_database_lookup(): value not found in hash table--looking in other modules\n"));
    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
	if (((database_module_list_t *)database_module->value)->funcs->lookup)
	{
	    if ((value = (*((database_module_list_t *)database_module->value)->funcs->lookup)(name, number, key)))
	    {
		return (_nx_copy_string(value));
	    }
	}
    }

    nexus_debug_printf(2, ("nexus_database_lookup(): value not found\n"));
    return NULL;
} /* nexus_database_lookup */

void _nx_database_shutdown(void)
{
    nexus_list_t *database_module;

    /* We have LOTS and LOTS of memory to deallocate here */

    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
	if (((database_module_list_t *)database_module->value)->funcs->shutdown)
	{
	    (*((database_module_list_t *)database_module->value)->funcs->shutdown)();
	}
    }
} /* _nx_database_shutdown() */

void _nx_database_abort(void)
{
    nexus_list_t *database_module;

    /* Don't deallocate memory--just STOP! */

    for (database_module = database_module_list_head;
         database_module;
	 database_module = database_module->next)
    {
	if (((database_module_list_t *)database_module->value)->funcs->abort)
	{
	    (*((database_module_list_t *)database_module->value)->funcs->abort)();
	}
    }
}

char *_nx_database_hash_table_lookup(char *name, int number, char *key)
{
    nexus_db_hash_entry_t *entry;
    nexus_list_t *cur_value;
    int key_size = strlen(key);

    for (entry = hash_table[hash_function(name, number)];
    	 entry;
	 entry=entry->next)
    {
        if (   (strcmp(entry->name, name) == 0)
	    && (   (entry->number == NEXUS_NO_NODE_NUM)
		|| (   (number >= entry->number)
		    && (   (entry->count == NEXUS_NO_NODE_NUM)
			|| (number < entry->number + entry->count) ) ) ) )
	{
            for (cur_value=entry->attr; cur_value; cur_value=cur_value->next)
            {
                if (strncmp(key, (char *)cur_value->value, key_size) == 0)
                {
               	    if (((char *)cur_value->value)[key_size] == '=')
                    {
                       return ((char *)(cur_value->value) + key_size + 1);
                    }
                    else if (((char *)cur_value->value)[key_size] == '\0')
                    {
                       return ((char *)(cur_value->value) + key_size);
                    }
        	}
    	    }
	}
    }
    return NULL;
}

void _nx_database_hash_table_add(char *name,
				 int number,
				 int count,
				 nexus_list_t *attr)
{
    nexus_db_hash_entry_t *data, *entry;
    int hash;
    nexus_list_t *i;
    nexus_db_hash_entry_t *j;

    hash = hash_function(name, number);
    for (data = hash_table[hash]; data; data = data->next)
    {
	if (number == NEXUS_NO_NODE_NUM && count > 0)
	{
	    number = 0;
	}
	else if (number >= 0 && count == NEXUS_NO_NODE_NUM)
	{
	    count = 1;
	}
	if (   (strcmp(name, data->name) == 0)
	    && (   (   (number >= data->number)
		    && (number < data->number + data->count) )
		|| (   (number == NEXUS_NO_NODE_NUM)
		    && (count == NEXUS_NO_NODE_NUM)
		    && (data->number == NEXUS_NO_NODE_NUM)
		    && (data->count == NEXUS_NO_NODE_NUM) ) ) )
	{
	    for (i=data->attr; i &&i->next; i=i->next) ;
	    if (i)
	    {
	        i->next = attr;
	    }
	    else
	    {
	        i = attr;
	    }
	    return;
        }
    }
    for (j=hash_table[hash]; j && j->next; j=j->next) ;
    NexusMalloc(_nx_database_hash_table_add(),
	        entry,
		nexus_db_hash_entry_t *,
		sizeof(nexus_db_hash_entry_t));
    entry->name = _nx_copy_string(name);
    entry->number = number;
    entry->count = count;
    entry->attr = attr;
    entry->next = NULL;
    if (j)
    {
        j->next = entry;
    }
    else
    {
        hash_table[hash] = entry;
    }
}

static int hash_function(char *name, int number)
{
    char *i;
    int sum;

    sum = 0;
    for (i=name; *i; i++)
    {
	sum += *i;
    }
    return (sum % DB_HASH_SIZE);
}

static char *parse_string_until_space(char *s, char **token)
{
    /* Change this to not count '\n' as whitespace */
    while ( isspace(*s) )
        s++;
    *token = s;
    while (( *s != '\0' ) && ( !isspace(*s) ))
        s++;
    if (*s != '\0')
        *s++ = '\0';
    return (s);
} /* parse_string_until_space() */

void _nx_database_hash_table_add_nodes_with_attrs(nexus_db_hash_entry_t *nodes,
						  nexus_list_t *attrs)
{
    nexus_db_hash_entry_t *i;

    for (i = nodes; i; i = i->next)
    {
        nexus_list_t *j;

	for (j = attrs; j; j = j->next)
	{
	    nexus_list_t *temp_attr;

	    NexusMalloc(_nx_database_hash_table_add_nodes_with_attrs(),
			temp_attr,
			nexus_list_t *,
			sizeof(nexus_list_t));
	    temp_attr->value = j->value;
	    temp_attr->next = NULL;
            _nx_database_hash_table_add(i->name,
					i->number,
					i->count,
					temp_attr);
	}
    }
}

nexus_list_t *_nx_database_parse_attributes(char *buf,
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
	    for (point = start_quote; !done; point++)
	    {
		if (*(point + num_quotes) == '\"')
		{
		    quote = !quote;
		    num_quotes++;
		}
		*point = *(point + num_quotes);
		if (!*point)
		{
		    done = NEXUS_TRUE;
		}
	    }
	    string = tmp;
	}
	while (quote)
	{
	    char *temp_string;

	    tmp = parse_string_until_space(string, &tuple);

	    if (tuple[strlen(tuple) - 1] == '\"')
	    {
		quote = NEXUS_FALSE;
		tuple[strlen(tuple) - 1] = '\0';
	    }

	    NexusMalloc(_nx_database_parse_attributes(),
			temp_string,
			char *,
			sizeof(char) * (strlen(start_quote) + 1 /* space */ + strlen(tuple) + 1 /* '\0' */));
	    strcpy(temp_string, start_quote);
	    NexusFree(start_quote);
	    strcat(temp_string, " ");
	    strcat(temp_string, tuple);
	    strcat(temp_string, "\0");
	    start_quote = _nx_copy_string(temp_string);
	    NexusFree(temp_string);

	    string = tmp;
	}
	if (start_quote)
	{
	    tuple = start_quote;
	}

	if (*tuple)
	{
            NexusMalloc(_nx_database_parse_attributes(),
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
}

void _nx_database_flush_rest_of_line(char *buf,
				     int buf_len,
				     char *starting_point,
				     FILE *fp)
{
    while(buf[strlen(buf)-2] == '\\')
    {
	fgets(buf, buf_len, fp);
    }
    starting_point = buf;
}
