#ifndef _P4_DEFS_H_
#define _P4_DEFS_H_

struct proc_info {
    int port;
    int switch_port;
    int unix_id;
    int slave_idx;
    int group_id;
    P4BOOL am_rm;
    char host_name[HOSTNAME_LEN];
    char machine_type[16];
};

#define NUMAVAILS 8

struct p4_avail_buff {
    int size;			/* size of message portion */
    struct p4_msg *buff;
};

struct p4_global_data {
#   ifdef SYSV_IPC
    int sysv_num_semids;
    int sysv_semid[P4_MAX_SYSV_SEMIDS];
    int sysv_next_lock;
#   endif
    struct proc_info proctable[P4_MAXPROCS];
    int listener_pid;
    int listener_port;
    P4BOOL local_communication_only;
    int local_slave_count;
    int n_forked_pids;
    char my_host_name[HOSTNAME_LEN];
    struct p4_avail_buff avail_buffs[NUMAVAILS];
    p4_lock_t avail_buffs_lock;
    struct p4_queued_msg *avail_quel;
    p4_lock_t avail_quel_lock;
    struct p4_msg_queue shmem_msg_queues[P4_MAX_MSG_QUEUES];
    int num_in_proctable;
    int num_installed;
    p4_lock_t slave_lock;
    int dest_id[P4_MAXPROCS];
    int listener_fd;
    int max_connections;
    int cube_msgs_out;    /* i860 msgs not yet msgwait'ed on */
    unsigned long reference_time;  /* used in p4_initenv and p4_clock */
    int hi_cluster_id;
    int low_cluster_id;
    P4VOID *cluster_shmem;
    p4_barrier_monitor_t cluster_barrier;
    char application_id[16];
} *p4_global;


struct connection {
    int type;
    int port;
    int switch_port;
    P4BOOL same_data_rep;
};

struct local_data {		/* local to each process */
    int listener_fd;
    int my_id;
    int local_commtype;		/* cube or shmem messages */
    struct p4_msg_queue *queued_messages;
    P4BOOL am_bm;
    struct connection *conntab;	/* pointer to array of connections */
    struct p4_procgroup *procgroup;
    int soft_errors;            /* false if errors cause termination */
    char *xdr_buff;
    XDR xdr_enc;
    XDR xdr_dec;
} *p4_local;

struct listener_data {
    int listening_fd;
    int slave_fd;
} *listener_info;


/* this struct is similar to a p4_net_msg_hdr;  note that the sum of
   the sizes of the items up to the *msg is equal to some number of 
   double words, which is important on machines like bfly2 if you 
   receive doubles into the msg area.
*/
/* link, orig_len, and pad are for the buffer itself*/
/* next fields are for the current message in the buffer */
struct p4_msg {
    struct p4_msg *link;
    int orig_len;
    int pad;
    int type;                
    int to;
    int from;
    int ack_req;
    int len;
    int msg_id;		        /* for i860 messages */
    int data_type;		/* for use by xdr */
    char *msg;	/* variable length array of characters */
};

struct p4_net_msg_hdr {
    int msg_type:32;
    int to:32;
    int from:32;
    int ack_req:32;
    int msg_len:32;
    int msg_id:32;		/* for i860 messages */
    int data_type:32;		/* for use by xdr */
    int pad:32;                 /* pad field to word boundary */
};

struct net_initial_handshake {
   int pid:32;
   int rm_num:32;
   /* int pad:32; */
};

struct p4_queued_msg {
    struct p4_msg *qmsg;
    struct p4_queued_msg *next;
};


/* Messages between a listener and any other non-listener */

#define DIE   1
#define SLAVE_DYING   2     /* Unused.  Check for whole data struct. */
#define CONNECTION_REQUEST   3
#define IGNORE_THIS   4

struct slave_listener_msg {
    int type:32;
    int from:32;
    int to:32;
    int to_pid:32;
    int lport:32;
    int pad:32;
};

/* Messages between the bm and a rm at startup */

#define INITIAL_INFO            11
#define REMOTE_LISTENER_INFO    12
#define REMOTE_SLAVE_INFO       13
#define REMOTE_MASTER_INFO      14
#define REMOTE_SLAVE_INFO_END   15
#define PROC_TABLE_ENTRY        16
#define PROC_TABLE_END          17
#define SYNC_MSG                18

struct bm_rm_msg {
    int type:32;

    /* for INITIAL_INFO */
    int numslaves:32;
    int numinproctab:32;
    int memsize:32;
    int rm_num:32;
    int debug_level:32;
    int logging_flag:32;

    /* for REMOTE_LISTENER_INFO */
    int port:32;

    /* for REMOTE_SLAVE_INFO and REMOTE_MASTER_INFO */
    int slave_idx:32;
    int slave_pid:32;
    int am_rm:32;

    /* for PROC_TABLE_ENTRY */
    int unix_id:32;
    int group_id:32;
    int switch_port:32;
    /* int pad:32;  to keep number of 32 bit quantities even */
    char host_name[HOSTNAME_LEN];

    /* also for INITIAL INFO */
    char pgm[128];
    char version[8];
    char outfile[128];
    char application_id[16];
    char machine_type[16];
};

#define P4_ACK_REQ_MASK   1     /* Masks define bits set for requests */
#define P4_ACK_REPLY_MASK 2
#define P4_BROADCAST_MASK 4

struct p4_brdcst_info_struct {
/*
  This structure is initialized by init_p4_brdcst_info() which
  is automatically called by every global operation
*/
  int initialized;             /* True if structure is initialized */
  int up;                      /* Process above me in tree         */
  int left_cluster;            /* Id of left child cluster master  */
  int right_cluster;           /* Id of right child cluster master */
  int left_slave;              /* Id of left child slave           */
  int right_slave;             /* Id of right child slave          */
} p4_brdcst_info;

#endif
