/*
 * types.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/types.h,v 1.48 1997/02/21 15:18:29 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_TYPES_H
#define _NEXUS_INCLUDE_TYPES_H


#ifdef NEXUS_ALIGN
#define NEXUS_SEGMENT_HEADER_SIZE \
    (sizeof(nexus_segment_t) \
     + ((NEXUS_ALIGN - (sizeof(nexus_segment_t) % NEXUS_ALIGN)) % NEXUS_ALIGN))
#else  /* NEXUS_ALIGN */
#define NEXUS_SEGMENT_HEADER_SIZE \
    sizeof(nexus_segment_t)
#endif /* NEXUS_ALIGN */

/*
 * nexus_new_nodes_monitor_t
 *
 * This monitor is used to keep track of new processes as they checkin.
 */
typedef struct _nexus_new_nodes_monitor_t
{
    nexus_mutex_t	mutex;
    nexus_cond_t	cond;
    int			total;
    int			checked_in;
    nexus_node_t *	nodes;
} nexus_new_nodes_monitor_t;


typedef struct _nexus_startup_node_t nexus_startup_node_t;
typedef struct _nexus_startup_funcs_t nexus_startup_funcs_t;

/*
 * nexus_startup_funcs_t
 *
 * This is the function indirection table used by startup modules.
 */
struct _nexus_startup_funcs_t
{
    nexus_bool_t	(*preinit)(nexus_startup_node_t **nl_head,
				   nexus_startup_node_t **nl_tail);
    char *		(*get_master_node_name)(void);
    void		(*init)(int *argc, char ***argv);
    void		(*shutdown)(nexus_bool_t shutdown_others);
    void		(*abort)(void);
    int    		(*start_node)(nexus_startup_node_t *node,
				      nexus_startpoint_t *reply_sp,
				      int first_checkin_number);
    nexus_bool_t	(*start_context)(char *executable_path,
					 nexus_startpoint_t *reply_sp,
					 int checkin_number,
					 int *return_code,
					 nexus_context_t **new_local_context);
};


/*
 * nexus_startup_node_t.
 *
 * Startup nodes structure.  st_iface.c creates this structure and
 * passes it to the start_node function of the startup module, to
 * tell it what start a node or set of nodes.
 *
 * If 'count' is > 1, then 'number' and 'id' fields are the _first_
 * of a continguous set of numbers and ids that the set of nodes should use.  
 */
struct _nexus_startup_node_t
{
    char *			name;
    int				number;
    int				count;
    int                         count_other_nodes;
    int				id;
    char *                      directory_path;
    char *			executable_path;
    char *			startup_module;
    nexus_startup_funcs_t *	startup_funcs;
    nexus_startup_node_t *	next;
    void *			st_ptr;
};


typedef struct _nexus_rdb_funcs_t nexus_rdb_funcs_t;

/* 
 * This is the function indirection table that gets loaded in
 * db_iface
 */
struct _nexus_rdb_funcs_t
{
    nexus_bool_t                (*init)(int *argc, char ***argv);
    void                        (*usage)(void);
    int                         (*new_params)(char *buf, int size);
    char *                      (*lookup)(char *node_name, char *key);
    void                        (*shutdown)(void);
    void                        (*abort)(void);
};

typedef struct _nexus_rdb_hash_entry_t nexus_rdb_hash_entry_t;

/*
 * This is the hash table stuff for database/startup file lookup
 * values.
 */
struct _nexus_rdb_hash_entry_t
{
    char *				name;
    nexus_list_t *			attr;
    struct _nexus_rdb_hash_entry_t *	next;
};

/*
 * nexus_transform_funcs_t
 *
 * This is the function indirection table used by transform modules.
 */
typedef struct _nexus_transform_funcs_t
{
    int		(*transform_id)();
    void	(*init)(int *argc, char ***argv,
			nexus_bool_t *modifies_data,
			nexus_bool_t *increases_size,
			unsigned long *header_size,
			unsigned long *trailer_size);
    void	(*shutdown)(void);
    void	(*abort)(void);
    void	(*transformattr_init)(void *info,
				      nexus_transformattr_t **attr);
    void	(*transformattr_destroy)(nexus_transformattr_t *tattr);
    void	(*transformattr_get_info)(nexus_transformattr_t *attr,
					  void **info);
    void 	(*init_endpoint_state)(nexus_transformattr_t *attr,
				       nexus_transformstate_t **ep_state);
    void	(*destroy_endpoint_state)(nexus_transformstate_t *ep_state);
    void	(*init_startpoint_state)(nexus_transformstate_t *ep_state,
					 nexus_transformstate_t **sp_state,
					 unsigned long *sp_state_id,
					 nexus_bool_t *copy_sp_locally,
					 nexus_bool_t *destroy_sp_locally);
    void 	(*copy_startpoint_state)(nexus_transformstate_t *sp_state,
				       nexus_transformstate_t **sp_state_copy);
    void	(*destroy_startpoint_state)(nexus_transformstate_t *sp_state);
    int		(*sizeof_state)(nexus_transformstate_t *s);
    void	(*put_state)(nexus_byte_t **buffer,
			     nexus_transformstate_t *state);
    void	(*get_state)(nexus_byte_t **buffer,
			     int format,
			     nexus_transformstate_t **state);
    int		(*transform)(nexus_byte_t *buffer,
			     nexus_transformstate_t *startpoint_state,
			     unsigned long buffer_size,
			     unsigned long header_size,
			     unsigned long data_size,
			     unsigned long save_header_size,
			     nexus_bool_t can_transform_inplace,
			     nexus_byte_t **out_buffer,
			     unsigned long *out_buffer_size,
			     unsigned long *out_header_size,
			     unsigned long *out_transform_info_size,
			     unsigned long *out_data_size,
			     unsigned long *out_untransform_size,
			     nexus_bool_t *out_must_be_freed);
    int		(*untransform)(nexus_byte_t *buffer,
			       nexus_transformstate_t *endpoint_state,
			       unsigned long startpoint_state_id,
			       unsigned long buffer_size,
			       unsigned long header_size,
			       unsigned long data_size,
			       unsigned long save_header_size,
			       nexus_bool_t can_untransform_inplace,
			       int format,
			       unsigned long untransform_size,
			       nexus_byte_t **out_buffer,
			       unsigned long *out_buffer_size,
			       unsigned long *out_header_size,
			       unsigned long *out_data_size,
			       nexus_bool_t *out_must_be_freed);
} nexus_transform_funcs_t ;


/*
 * nexus_transform_table_t
 */
typedef struct _nexus_transform_table_t
{
    nexus_transform_funcs_t *		funcs;
    char *				name;
    nexus_bool_t			modifies_data;
    nexus_bool_t			increases_size;
    unsigned long			header_size;
    unsigned long			trailer_size;
} nexus_transform_table_t;


/*
 * nexus_buffer_coalesce_t
 */
#include <sys/uio.h>
typedef struct _nexus_buffer_coalesce_t
{
    int					magic;
    struct _nexus_buffer_coalesce_t *	next;
    nexus_mutex_t			mutex;
    nexus_cond_t			cond;
    nexus_bool_t			done;
    nexus_bool_t                        use_writev;
    union
    {
        struct
	{
	    nexus_byte_t *              data;
	    unsigned long               data_size;
            nexus_direct_info_t *	custom_direct_puts;
            unsigned long 		num_custom_direct_puts;
	} data;
        struct
	{
	    struct iovec *              iovec;
	    int                         iovec_count;
	} writev;
    } u;
} nexus_buffer_coalesce_t;
    

#endif /* _NEXUS_INCLUDE_TYPES_H */
