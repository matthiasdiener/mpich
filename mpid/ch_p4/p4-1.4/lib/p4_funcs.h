/* to import correctly for c++ users */
#ifdef __cplusplus
#    define DOTS ...
#else
#    define DOTS
#endif

double p4_usclock(DOTS);
char *p4_shmalloc(DOTS);
P4BOOL p4_am_i_cluster_master(DOTS);
int p4_askfor(DOTS);
int p4_askfor_init(DOTS);
P4VOID p4_barrier(DOTS);
int p4_barrier_init(DOTS);
int p4_broadcastx(DOTS);
int p4_clock(DOTS);
int p4_create(DOTS);
int p4_create_procgroup(DOTS);
int p4_startup(DOTS);
P4VOID p4_dprintf(DOTS);
P4VOID p4_error(DOTS);
P4VOID p4_global_barrier(DOTS);
P4VOID p4_get_cluster_masters(DOTS);
P4VOID p4_get_cluster_ids(DOTS);
int p4_get_my_cluster_id(DOTS);
int p4_get_my_id(DOTS);
int p4_get_my_id_from_proc(DOTS);
int p4_getsub_init(DOTS);
P4VOID p4_getsubs(DOTS);
int p4_global_op(DOTS);
int p4_initenv(DOTS);
P4VOID p4_int_absmax_op(DOTS);
P4VOID p4_int_absmin_op(DOTS);
P4VOID p4_int_max_op(DOTS);
P4VOID p4_int_min_op(DOTS);
P4VOID p4_int_mult_op(DOTS);
P4VOID p4_int_sum_op(DOTS);
P4VOID p4_dbl_absmax_op(DOTS);
P4VOID p4_dbl_absmin_op(DOTS);
P4VOID p4_dbl_max_op(DOTS);
P4VOID p4_dbl_min_op(DOTS);
P4VOID p4_dbl_mult_op(DOTS);
P4VOID p4_dbl_sum_op(DOTS);
P4VOID p4_flt_sum_op(DOTS);
P4VOID p4_flt_absmax_op(DOTS);
P4VOID p4_flt_absmin_op(DOTS);
P4VOID p4_flt_max_op(DOTS);
P4VOID p4_flt_min_op(DOTS);
P4VOID p4_flt_mult_op(DOTS);
P4VOID p4_flt_sum_op(DOTS);
P4VOID p4_mcontinue(DOTS);
P4VOID p4_mdelay(DOTS);
P4VOID p4_menter(DOTS);
P4BOOL p4_messages_available(DOTS);
P4BOOL p4_any_messages_available(DOTS);
P4VOID p4_mexit(DOTS);
int p4_moninit(DOTS);
P4VOID p4_msg_free(DOTS);
char *p4_msg_alloc(DOTS);
int p4_num_cluster_ids(DOTS);
int p4_num_total_ids(DOTS);
int p4_num_total_slaves(DOTS);
P4VOID p4_probend(DOTS);
P4VOID p4_progend(DOTS);
int p4_recv(DOTS);
int p4_get_dbg_level(DOTS);
P4VOID p4_set_dbg_level(DOTS);
P4VOID p4_shfree(DOTS);
int p4_soft_errors(DOTS);
P4VOID p4_update(DOTS);
char *p4_version(DOTS);
char *p4_machine_type(DOTS);
int p4_wait_for_end(DOTS);
P4VOID p4_print_avail_buffs(DOTS);
struct p4_procgroup *p4_alloc_procgroup(DOTS);

#ifdef P4_DPRINTFL
P4VOID p4_dprintfl(DOTS);
#endif
