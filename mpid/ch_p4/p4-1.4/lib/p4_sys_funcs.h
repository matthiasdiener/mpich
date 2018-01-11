char *xx_malloc();
char *MD_shmalloc();
char *print_conn_type();
char *xx_shmalloc();
int MD_clock();
P4VOID get_qualified_hostname();
P4VOID MD_initenv();
P4VOID MD_initmem();
P4VOID MD_malloc_hint();
P4VOID MD_shfree();
P4VOID MD_start_cube_slaves();
int bm_start()	;
P4VOID compute_conntab();
P4VOID request_connection();
int create_bm_processes();
P4VOID net_slave_info()	;
int create_remote_processes();
P4VOID create_rm_processes()	;
P4VOID dump_conntab();
P4VOID dump_global()	;
P4VOID dump_listener()	;
P4VOID dump_local()	;
P4VOID dump_procgroup()	;
P4VOID dump_tmsg();
int establish_connection();
P4VOID exec_pgm();
int fork_p4();
P4VOID free_p4_msg();
P4VOID free_quel();
P4VOID get_inet_addr();
P4VOID get_inet_addr_str();
P4VOID dump_sockaddr();
P4VOID dump_sockinfo();
P4VOID get_pipe()	;
int getswport();
P4VOID handle_connection_interrupt();
P4BOOL shmem_msgs_available();
P4BOOL socket_msgs_available();
P4BOOL MD_tcmp_msgs_available();
P4BOOL MD_i860_msgs_available();
P4BOOL MD_CM5_msgs_available();
P4BOOL MD_NCUBE_msgs_available();
P4BOOL MD_euih_msgs_available();
int MD_i860_send();
int MD_CM5_send();
int MD_NCUBE_send();
int MD_eui_send();
int MD_euih_send();
struct p4_msg *MD_i860_recv();
struct p4_msg *MD_CM5_recv();
struct p4_msg *MD_NCUBE_recv();
struct p4_msg *MD_eui_recv();
struct p4_msg *MD_euih_recv();
P4BOOL in_same_cluster();
P4VOID init_avail_buffs();
P4VOID initialize_msg_queue()	;
int install_in_proctable();
P4VOID kill_server();
P4VOID listener();
struct listener_data *alloc_listener_info();
struct local_data *alloc_local_bm();
struct local_data *alloc_local_listener();
struct local_data *alloc_local_rm();
struct local_data *alloc_local_slave();
int myhost();
int net_accept()	;
int net_conn_to_addr_listener()	;
int net_conn_to_listener()	;
int net_conn_to_named_listener()	;
int net_create_slave()	;
int net_recv()	;
int net_send()	;
P4VOID net_setup_anon_listener()	;
P4VOID net_setup_listener()	;
P4VOID net_setup_named_listener()	;
int num_in_mon_queue();
P4VOID alloc_global();
struct p4_msg *alloc_p4_msg();
struct p4_msg *get_tmsg();
struct p4_msg *recv_message();
struct p4_msg *search_p4_queue();
struct p4_msg *shmem_recv();
struct p4_msg *socket_recv();
struct p4_msg *socket_recv_on_fd();
struct p4_queued_msg *alloc_quel();
P4VOID process_args()	;
P4BOOL process_connect_request()	;
int process_connection();
P4BOOL process_slave_message()	;
struct p4_procgroup *alloc_procgroup();
struct p4_procgroup *read_procgroup()	;
P4VOID procgroup_to_proctable();
P4VOID queue_p4_message();
P4VOID reaper();
P4VOID receive_proc_table()	;
int rm_newline();
int rm_start();
P4VOID send_ack()	;
int send_message();
P4VOID sync_with_remotes();
P4VOID send_proc_table();
P4VOID setup_conntab();
int shmem_send();
P4VOID shutdown_p4_socks();
#if defined(GP_1000) || defined(TC_2000)
int simple_lock();
int simple_unlock();
P4VOID waitspin();
#endif
P4BOOL sock_msg_avail_on_fd();
int socket_send();
int start_slave();
int subtree_broadcast_p4();
P4VOID trap_sig_errs();
P4VOID wait_for_ack()	;
int xdr_recv();
int xdr_send();
int data_representation();
P4BOOL same_data_representation();
P4VOID xx_init_shmalloc();
P4VOID xx_shfree();
P4VOID zap_p4_processes();
struct p4_msg *MD_tcmp_recv();
int MD_tcmp_send();
struct hostent *gethostbyname_p4();

#ifdef SYSV_IPC
int init_sysv_semset();
P4VOID MD_lock_init();
P4VOID MD_lock();
P4VOID MD_unlock();
P4VOID remove_sysv_ipc();
#endif
