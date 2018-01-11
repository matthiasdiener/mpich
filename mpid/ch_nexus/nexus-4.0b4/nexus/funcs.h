/*
 * funcs.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/funcs.h,v 1.70 1997/02/21 15:18:24 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_FUNCS_H
#define _NEXUS_INCLUDE_FUNCS_H

/*
 * args.c
 */
extern void	_nx_get_args(int *argc,
			     char ***argv,
			     char *args_env_variable,
			     char *package_designator,
			     void (*usage_message_func)(void),
			     int (*new_process_params_func)(char *, int),
			     nexus_bool_t ignore_command_line_args);
extern void	_nx_args_init(int *argc, char ***argv);
extern void	_nx_args_cleanup(int *argc, char ***argv);
extern int	_nx_new_process_params(char *buf, int size);
extern int	_nx_split_args(char *arg_string,
			       int starting_argc,
			       int max_argc,
			       char **package_argv,
			       char *string_buffer);
extern char *	_nx_get_package_id_start(void);
extern char *	_nx_get_package_id_end(void);
extern char *	_nx_get_argv0(void);

/*
 * attach.c
 */
extern void	_nx_attach_usage_message(void);
extern int	_nx_attach_new_process_params(char *buf, int size);
extern void	_nx_attach_init(int *argc, char ***argv);


/*
 * buffer.c
 */
extern void _nx_buffer_init(int *argc, char ***argv);
extern int  _nx_buffer_coalesce(struct _nexus_buffer_t *buffer,
				struct _nexus_buffer_t **r_buffer,
				nexus_startpoint_t *startpoint,
				int handler_id,
				unsigned long total_direct_puts,
				nexus_bool_t called_from_non_threaded_handler,
				nexus_bool_t can_use_iovec,
				nexus_bool_t destroy_buffer);
extern int  _nx_buffer_create_from_raw(nexus_byte_t *raw_buffer,
				       unsigned long raw_size,
				       struct _nexus_buffer_t **buffer);
extern int  _nx_buffer_dispatch(struct _nexus_buffer_t *buffer);


/*
 * commlink.c
 */
extern void	_nx_commlink_usage_message(void);
extern void	_nx_commlink_init(int *argc, char ***argv);
extern int	_nx_commlink_new_process_params(char *buf, int size);
extern int	_nx_write_startpoint(int filedesc, nexus_startpoint_t *sp);
extern int	_nx_read_startpoint(int filedesc, nexus_startpoint_t *sp);


/*
 * context.c
 */
extern void	_nx_context_usage_message(void);
extern void	_nx_context_init(int *argc, char ***argv);
extern nexus_context_t *_nx_context_alloc(nexus_bool_t is_node);
extern int	_nx_context_new_process_params(char *buf, int size);
extern void	_nx_context_create_handler(nexus_endpoint_t *ep,
				nexus_buffer_t *buffer,
				nexus_bool_t called_from_non_threaded_handler);
extern void	_nx_context_checkin(nexus_startpoint_t *context_sp,
				    int nexus_boot_rc,
				    nexus_startpoint_t *reply_sp,
				    int checkin_number
#ifdef BUILD_PROFILE
				    , int need_master_sp
#endif			 
				    );
#ifdef BUILD_PROFILE
extern void _nx_wait_for_master_gp_from_creator(nexus_context_t *context);
extern void _nx_send_master_gp_to_context(nexus_startpoint_t *context_sp);
#endif

/*
 * fault.c
 */
extern void	_nx_fault_tolerance_usage_message(void);
extern int	_nx_fault_tolerance_new_process_params(char *buf, int size);
extern void	_nx_fault_tolerance_init(int *argc, char ***argv);
extern int	_nx_fault_detected(int fault_code);


/*
 * handler.c
 */
extern void	_nx_handler_usage_message(void);
extern int	_nx_handler_new_process_params(char *buf, int size);
extern void	_nx_handler_init(int *argc, char ***argv);
extern void	_nx_handle_message(int handler_id,
				   nexus_endpoint_t *endpoint,
#ifdef BUILD_PROFILE
				   int node_id,
				   int context_id,
				   int message_length,
#endif
				   void *buffer);
extern void	_nx_dump_rsr_profile_info(nexus_context_t *context);
#ifdef BUILD_PROFILE
extern void	_nx_accumulate_rsr_profile_info(nexus_context_t *context,
						nexus_handler_record_t *rec,
						int node_id,
						int context_id,
						int message_length);
#endif /* BUILD_PROFILE */


/*
 * init.c
 */
extern void	_nx_init_nexus(int *argc,
			       char ***argv,
			       char *args_env_variable,
			       char *package_designator,
			       int (*package_args_init_func)(int *argc,char ***argv),
			       void (*usage_message_func)(void),
			       int (*new_process_params_func)(char *,int),
			       nexus_module_list_t module_list[],
			       nexus_node_t **nodes,
			       int *n_nodes);
extern void	_nx_exit_transient_process(int rc);


/*
 * nl_*.c
 */
extern void	_nx_nodelock_master_init(void);
extern nexus_bool_t	_nx_nodelock_check(void);
extern void	_nx_nodelock_shutdown(void);
extern void	_nx_nodelock_cleanup(void);


/*
 * md_*.c
 */
extern void	_nx_md_usage_message(void);
extern void	_nx_md_init(int *argc, char ***argv);
extern void	_nx_md_shutdown(void);
extern void	_nx_md_abort(int return_code);
extern void	_nx_md_exit(int return_code);
extern int	_nx_md_get_command_line_args(int starting_argc,
					     int max_argc,
					     char **argv);
extern char *	_nx_md_system_error_string(int the_error);
extern char *	_nx_md_get_unique_session_string();
extern void	_nx_md_gethostname(char *name, int len);
extern int	_nx_md_getpid();
extern int	_nx_md_new_process_params(char *buf, int size);
extern int	_nx_md_fork(void);
extern int	_nx_md_block_sigchld(void);
extern void	_nx_md_deliver_sigchld(int mask);
extern void	_nx_md_wait_for_children(void);


/*
 * pr_iface.c
 */
extern void	_nx_proto_usage_message(void);
extern int	_nx_proto_new_process_params(char *buf, int size);
extern void	_nx_proto_init(int *argc,
			       char ***argv,
			       nexus_module_list_t module_list[]);
extern void	_nx_proto_shutdown(nexus_bool_t shutdown_others);
extern void	_nx_proto_abort(void);
extern void	_nx_proto_get_creator_sp_params(char *buf,
						int buf_size,
						nexus_startpoint_t *sp);
extern void	_nx_proto_construct_creator_sp(nexus_startpoint_t *creator_sp);
extern void	_nx_mi_proto_destroy(nexus_mi_proto_t *mi_proto);
extern nexus_mi_proto_t *_nx_mi_proto_table_insert(
					nexus_mi_proto_t *new_mi_proto);
extern nexus_mi_proto_t *_nx_mi_proto_create(int size,
					     nexus_byte_t *array,
					     nexus_proto_t *proto);



/*
 * pr_local.c
 */
extern void *	_nx_pr_local_info(void);


/*
 * process.c
 */
extern void	_nx_process_usage_message(void);
extern void	_nx_process_init(int *argc,
				 char ***argv,
				 char *package_designator,
				 int (*package_args_init_func)(int *argc,
							       char **argv[]),
				 nexus_module_list_t module_list[],
				 nexus_node_t **nodes,
				 int *n_nodes);
extern void	_nx_process_start(void);
extern int	_nx_process_new_process_params(char *buf, int size);
#ifdef BUILD_PROFILE
extern void	_nx_get_next_node_id_from_master(int *node_id,
						 int n_nodes,
						 nexus_context_t *context);
#endif /* BUILD_PROFILE */


/*
 * rdb_iface.c
 */
extern void	_nx_rdb_usage_message(void);
extern int	_nx_rdb_new_process_params(char *buf, int size);
extern void	_nx_rdb_init(int *argc,
			     char ***argv,
			     nexus_module_list_t module_list[]);
extern void	_nx_rdb_shutdown(void);
extern void	_nx_rdb_abort(void);
extern nexus_bool_t _nx_rdb_hash_table_lookup(char *name,
					      char *key,
					      char **value);
extern void     _nx_rdb_hash_table_add(char *name,
				       nexus_list_t *attr);
extern nexus_list_t *_nx_rdb_parse_attributes(char *buf,
					      int buf_len,
					      char *starting_point,
					      FILE *fp);
extern void     _nx_rdb_flush_rest_of_line(char *buf,
					   int buf_len,
					   char *starting_point,
					   FILE *fp);
extern void     _nx_rdb_hash_table_add_nodes_with_attrs(
			nexus_rdb_hash_entry_t *nodes,
			nexus_list_t *attrs);

/*
 * st_iface.c
 */
extern void	_nx_startup_usage_message(void);
extern int	_nx_startup_new_process_params(char *buf, int size);
extern void     _nx_startup_modify_command_for_type(nexus_startup_node_t *node,
					    	    char *cmd,
					    	    int max_cmd_length,
						    char ***environment,
						    int *n_env);
extern nexus_bool_t	_nx_startup_preinit(nexus_module_list_t module_list[]);
extern void	_nx_startup_init(int *argc, char ***argv);
extern char *	_nx_startup_get_master_node_name(void);
extern void	_nx_startup_initial_nodes(nexus_node_t *my_node,
					  nexus_node_t **nodes,
					  int *n_nodes);
extern int	_nx_startup_context(char *executable_path,
				    nexus_startpoint_t *reply_sp,
				    int checkin_number,
				    nexus_context_t **new_local_context);
extern void	_nx_startup_shutdown(nexus_bool_t shutdown_others);
extern void	_nx_startup_abort(void);
extern void	_nx_node_checkin(nexus_node_t *node,
				 int n_nodes,
				 nexus_startpoint_t *creator_sp,
				 int checkin_number
#ifdef BUILD_PROFILE
				 , int need_master_sp
#endif			 
				 );


/*
 * th_*.c
 */
#ifndef BUILD_LITE
extern void	_nx_thread_usage_message(void);
extern int	_nx_thread_new_process_params(char *buf, int size);
extern void	_nx_thread_preinit(void);
extern void	_nx_thread_init(int *argc, char ***argv);
extern void	_nx_thread_shutdown(void);
extern void	_nx_thread_abort(int return_code);
extern void	_nx_thread_create_handler_thread(void);
extern void	_nx_thread_shutdown_handler_thread(void);
#endif /* BUILD_LITE */


/*
 * trace.c
 */
extern void	_nx_traceback(void);


/*
 * transform_iface.c
 */
extern void	nexus_transform_usage_message(void);
extern int	nexus_transform_new_process_params(char *buf, int size);
extern int	nexus_transform_init(int *argc, char ***argv,
				     nexus_module_list_t module_list[]);
extern void	nexus_transform_add_module(char *module_name,
					   void *(*info_func)(void),
					   int *argc, char ***argv);
extern void     nexus_transform_shutdown(void);
extern void     nexus_transform_abort(void);
extern void	nexus_transformattr_init(int id,
					 void *info,
					 nexus_transformattr_t **attr);
extern void	nexus_transformattr_destroy(int id,
					    nexus_transformattr_t *attr);
extern void	nexus_transformattr_get_info(int id,
					     nexus_transformattr_t *attr,
					     void **info);
extern void	nexus_transformstate_init_on_endpoint(int id,
					   nexus_transformattr_t *attr,
					   nexus_transformstate_t **ep_state);
extern void	nexus_transformstate_destroy_on_endpoint(int id,
					nexus_transformstate_t *ep_state);
extern void	nexus_transformstate_init_on_startpoint(int id,
					     nexus_transformstate_t *ep_state,
					     nexus_transformstate_t **sp_state,
					     unsigned long *sp_state_id,
					     nexus_bool_t *copy_sp_locally,
					     nexus_bool_t *destroy_sp_locally);
extern void	nexus_transformstate_copy(int id,
			       nexus_transformstate_t *sp_state,
			       nexus_transformstate_t **sp_state_copy);
extern void	nexus_transformstate_destroy_on_startpoint(
					int id,
    					nexus_transformstate_t *sp_state);
extern int	nexus_transformstate_sizeof(int id,
					    nexus_transformstate_t *sp_state);
extern void	nexus_transformstate_put(int id,
					 nexus_byte_t **buffer,
					 nexus_transformstate_t *sp_state);
extern void	nexus_transformstate_get(int id,
					 nexus_byte_t **buffer,
					 int format,
					 nexus_transformstate_t **sp_state);
extern int      nexus_buffer_transform(int id,
				       nexus_byte_t *buffer,
				       nexus_transformstate_t *sp_state,
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
extern int      nexus_buffer_untransform(int id,
					 nexus_byte_t *buffer,
					 nexus_transformstate_t *ep_state,
					 unsigned long sp_state_id,
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
extern void	nexus_transform_info(int transform_id,
				     nexus_bool_t *modifies_data,
				     nexus_bool_t *increases_size,
				     unsigned long *header_size,
				     unsigned long *trailer_size);

/*
 * util.c
 */
extern char *	_nx_copy_string(char *s);
extern char *	_nx_executable_name(void);
extern char *	_nx_current_working_directory(void);
extern void	_nx_strip_tmp_mnt_from_path(char *dir);
extern char *   _nx_find_attribute(char *attr,
				   char *search_string,
				   char separator);
extern void     _nx_get_next_value(char *values,
				   char separator,
				   char **next,
				   char **this);
extern void	_nx_hex_encode_byte_array(unsigned char *bytes,
					  int length,
					  char *hex);
extern void	_nx_hex_decode_byte_array(char *hex,
					  int length,
					  unsigned char *bytes);
#ifndef HAVE_STRTOUL
extern unsigned long strtoul(const char *nptr,
			     char **endptr,
			     int base);
#endif /* HAVE_STRTOUL */
extern int	_nx_write_blocking(int fd, void *buf, size_t size);
extern int	_nx_read_blocking(int fd, void *buf, size_t size);

#endif /* _NEXUS_INCLUDE_FUNCS_H */

