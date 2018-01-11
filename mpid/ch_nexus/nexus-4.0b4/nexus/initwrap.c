/*
 * initwrap.c
 *
 * This wraps the nexus initialization routine, _nx_init_nexus().
 * Under AIX, this must be statically linked into an application
 * so that libnexus can be dynamically linked with no
 * unresolved symbols.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/initwrap.c,v 1.29 1996/12/12 19:56:11 tuecke Exp $";

#include "internal.h"

/* new way */
/*
 * Define some entries for various mix-in types
 */
#ifdef HAVE_ATM
extern void *_nx_pr_atm_info(void);
#define I_NEXUS_ATM	{"protocols", "atm", _nx_pr_atm_info},
#else
#define I_NEXUS_ATM
#endif

#ifdef HAVE_UDP
extern void *_nx_pr_udp_info(void);
#define I_NEXUS_UDP	{"protocols", "udp", _nx_pr_udp_info},
#else
#define I_NEXUS_UDP
#endif

#ifdef HAVE_SHM
extern void *_nx_pr_shm_info(void);
#define I_NEXUS_SHM	{"protocols", "shm", _nx_pr_shm_info},
#else
#define I_NEXUS_SHM
#endif

#ifdef HAVE_MN_UDP
extern void *_nx_pr_mn_udp_info(void);
#define I_NEXUS_MN_UDP	{"protocols", "mn_udp", _nx_pr_mn_udp_info},
#else
#define I_NEXUS_MN_UDP
#endif

#ifdef HAVE_ST_NEXUS_SERVER
extern void *_nx_st_nexus_server_info(void);
#define I_NEXUS_ST_NEXUS_SERVER	    {"startups", "nexus_server", _nx_st_nexus_server_info},
#else
#define I_NEXUS_ST_NEXUS_SERVER
#endif

#ifdef HAVE_SS
extern void *_nx_st_ss_info(void);
#define I_NEXUS_ST_SS	{"startups", "ss", _nx_st_ss_info},
#else
#define I_NEXUS_ST_SS
#endif

/*
 * Define a default module_list
 */
#ifdef TARGET_ARCH_PARAGON
#define DEFINED_MODULE_LIST
extern void *_nx_st_inx_info(void);
#ifdef HAVE_TCP
extern void *_nx_st_rsh_info(void);
extern void *_nx_pr_tcp_info(void);
#endif
extern void *_nx_pr_inx_info(void);
extern void *_nx_rdb_file_info(void);
static nexus_module_list_t default_module_list[] =
{
    {"startups", "inx", _nx_st_inx_info},
    I_NEXUS_ST_SS
#ifdef HAVE_TCP
    {"startups", "rsh", _nx_st_rsh_info},
#endif
    {"protocols", "inx", _nx_pr_inx_info},
#ifdef HAVE_TCP
    {"protocols", "tcp", _nx_pr_tcp_info},
#endif
    {"rdb", "file", _nx_rdb_file_info},
    {NULL, NULL, NULL},
};
#endif

#ifdef HAVE_MPL
#define DEFINED_MODULE_LIST
#ifndef BUILD_LITE
#ifdef NEXUS_ARCH_STARTUP_IBMDS
extern void *_nx_st_ibmds_info(void);
#endif
#endif /* BUILD_LITE */
extern void *_nx_st_mpl_info(void);
#ifdef HAVE_TCP
extern void *_nx_st_rsh_info(void);
extern void *_nx_pr_tcp_info(void);
#endif
extern void *_nx_pr_mpl_info(void);
extern void *_nx_rdb_file_info(void);
static nexus_module_list_t default_module_list[] =
{
#ifndef BUILD_LITE
#ifdef HAVE_IBMDS
    {"startups", "ibmds", _nx_st_ibmds_info},
#endif
#endif /* BUILD_LITE */
    {"startups", "mpl", _nx_st_mpl_info},
    I_NEXUS_ST_SS
#ifdef HAVE_TCP
    {"startups", "rsh", _nx_st_rsh_info},
#endif
    {"protocols", "mpl", _nx_pr_mpl_info},
#ifdef HAVE_TCP
    {"protocols", "tcp", _nx_pr_tcp_info},
#endif
    {"rdb", "file", _nx_rdb_file_info},
    {NULL, NULL, NULL},
};
#endif

#ifdef HAVE_MPINX
#define DEFINED_MODULE_LIST
extern void *_nx_st_mpinx_info(void);
extern void *_nx_pr_mpinx_info(void);
extern void *_nx_database_file_info(void);
static nexus_module_list_t default_module_list[] =
{
    {"startups", "mpinx", _nx_st_mpinx_info},
    {"protocols", "mpinx", _nx_pr_mpinx_info},
    {"rdb", "file", _nx_rdb_file_info},
    {NULL, NULL, NULL},
};
#endif

#ifndef DEFINED_MODULE_LIST
#ifndef BUILD_LITE
#ifdef TARGET_ARCH_SOLARIS
extern void *_nx_st_soldl_info(void);
#endif /* ARCH_SOLARIS */
#ifdef HAVE_IBMDS
extern void *_nx_st_ibmds_info(void);
#endif /* HAVE_IBMDS */
#endif /* BUILD_LITE */
extern void *_nx_st_fork_info(void);
extern void *_nx_st_rsh_info(void);
extern void *_nx_pr_tcp_info(void);
extern void *_nx_rdb_file_info(void);
static nexus_module_list_t default_module_list[] =
{
#ifndef BUILD_LITE
#ifdef TARGET_ARCH_SOLARIS
/*    {"startups", "soldl", _nx_st_soldl_info}, */
#endif
#ifdef HAVE_IBMDS
    {"startups", "ibmds", _nx_st_ibmds_info},
#endif
#endif /* BUILD_LITE */
    {"startups", "fork", _nx_st_fork_info},
    I_NEXUS_ST_SS
    {"startups", "rsh", _nx_st_rsh_info},
    I_NEXUS_ST_NEXUS_SERVER
    I_NEXUS_ATM
    I_NEXUS_UDP
    I_NEXUS_MN_UDP
    I_NEXUS_SHM
    {"protocols", "tcp", _nx_pr_tcp_info},
    {"rdb", "file", _nx_rdb_file_info},
    {NULL, NULL, NULL},
};
#endif


/*
 * nexus_init()
 */
void nexus_init(int *argc,
		char ***argv,
		char *args_env_variable,
		char *package_designator,
		int (*package_args_init_func)(int *argc, char ***argv),
		void (*usage_message_func)(void),
		int (*new_process_params_func)(char *buf, int size),
		nexus_module_list_t module_list[],
		nexus_node_t **nodes,
		int *n_nodes)
{
    _nx_NexusBoot = NexusBoot;
    _nx_NexusExit = NexusExit;
    _nx_NexusAcquiredAsNode = NexusAcquiredAsNode;
    _nx_init_nexus(argc,
		   argv,
		   args_env_variable,
		   package_designator,
		   package_args_init_func,
		   usage_message_func,
		   new_process_params_func,
		   (module_list ? module_list : default_module_list),
		   nodes,
		   n_nodes);
} /* nexus_init() */
