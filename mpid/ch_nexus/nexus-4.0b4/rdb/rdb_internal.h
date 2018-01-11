#define RDB_NO_HIT " "

typedef resource_database_list_t rdb_list_t;


/*
 * rdb_funcs_t
 *
 * This is the function indirection table that gets loaded in
 * rdb_iface
 */
typedef struct _rdb_funcs_t
{
    rdb_bool_t		(*init)(int *argc, char ***argv);
    void		(*usage)();
    int			(*new_params)(char *buf,
				      int size);
    char *		(*lookup)(char *name,
				  char *key);
    void		(*shutdown)();
    void		(*abort)();
} rdb_funcs_t;


/*
 * External declarations of various internal rdb functions
 */
extern rdb_bool_t _rdb_hash_table_lookup(char *name,
					 char *key,
					 char **value);
extern void     _rdb_hash_table_add(char *name, rdb_list_t *attr);
extern void     _rdb_hash_table_add_nodes_with_attrs(rdb_hash_entry_t *nodes,
						     rdb_list_t *attrs);
extern rdb_list_t *_rdb_parse_attributes(char *buf,
					 int buf_len,
					 char *starting_point,
					 FILE *fp);
extern void     _rdb_flush_rest_of_line(char *buf,
					int buf_len,
					char *starting_point,
					FILE *fp);
