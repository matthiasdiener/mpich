#ifndef __globdev_protos_details__
#define __globdev_protos_details__

/*******************/
/* TCP proto stuff */
/*******************/

enum header_type {user_data, ack, cancel_send, cancel_result};

/* header =
 * type==user_data,src,tag,contextid,dataoriginbuffsize,ssendflag,packed_flag,
 *       msgid_srcgrank(int),msgid_sec(long),msgid_usec(long),msgid_ctr(ulong),
 *       liba(ulong)
 * OR 
 * type==ack, liba(ulong) 
 */
#define TCP_HDR_N_INTS   8
#define TCP_HDR_N_LONGS  2
#define TCP_HDR_N_ULONGS 2
#define LOCAL_HEADER_LEN (globus_dc_sizeof_int(TCP_HDR_N_INTS) + \
    globus_dc_sizeof_long(TCP_HDR_N_LONGS) + \
    globus_dc_sizeof_u_long(TCP_HDR_N_ULONGS))
#define REMOTE_HEADER_LEN(format) \
    (globus_dc_sizeof_remote_int(TCP_HDR_N_INTS, (format)) + \
    globus_dc_sizeof_remote_long(TCP_HDR_N_LONGS, (format)) + \
    globus_dc_sizeof_remote_u_long(TCP_HDR_N_ULONGS, (format)))

struct tcpsendreq
{
    struct tcpsendreq *prev;
    struct tcpsendreq *next;
    enum header_type  type;
    globus_bool_t write_started; /* used only for data, not for cancel */
    void *buff;
    globus_byte_t *src;
    int count;
    struct MPIR_DATATYPE *datatype;
    int src_lrank;
    int tag;
    int context_id;
    int result;
    int dest_grank;
    long msgid_sec;
    long msgid_usec;
    unsigned long msgid_ctr;
    void * liba;
    int libasize;
    MPIR_SHANDLE *sreq;
}; 

/* 
 * INSTRUCTIONBUFFLEN must be large enough to hold 
 * 2 chars + string representation of largest rank in MPI_COMM_WORLD + '\0'
 */
#define INSTRUCTIONBUFFLEN 20

/* instructions */
#define FORMAT 'F'
#define PRIME  'P'
enum tcp_read_state {await_instructions,await_format,await_header,await_data};
struct tcp_rw_handle_t
{
    globus_io_handle_t     handle;
    enum tcp_read_state    state;
    globus_byte_t          instruction_buff[INSTRUCTIONBUFFLEN]; /* handshake */
    globus_cond_t          format_cond;                          /* handshake */
    volatile globus_bool_t recvd_format;                         /* handshake */
    globus_byte_t          remote_format;
    globus_byte_t          *incoming_header;
    globus_size_t          incoming_header_len;
    void *                 liba;
    int                    libasize;
    int                    src;
    int                    tag;
    int                    context_id;
    int                    dataorigin_bufflen;
    int                    ssend_flag;
    int                    packed_flag;
    globus_byte_t          *incoming_raw_data;
    int                    msg_id_src_grank; /* message id */
    long                   msg_id_sec;       /* message id */
    long                   msg_id_usec;      /* message id */
    unsigned long          msg_id_ctr;       /* message id */
};

struct tcp_miproto_t
{
    char                            hostname[MAXHOSTNAMELEN];
    unsigned short                  port;
    globus_io_attr_t                attr;
    volatile struct tcp_rw_handle_t *handlep;
    globus_mutex_t                  connection_lock; /* handshake */
    globus_cond_t                   connection_cond; /* handshake */

    /* 
     * 'to_self' used only when send/rcv to myself 
     * and TCP is the selected proto to myself.
     */
    struct tcp_rw_handle_t          to_self; 

    /* 
     * most of the time 'whandle' will point to &(handlep->handle).
     * where handlep is malloc'd during connection establishment.  
     * there is one case in which whandle will _not_ point there, when 
     * a proc connects to itself and TCP is the selected proto to itself.
     * in this case we need 2 distinct handles, so whandle = &(to_self.handle) 
     * (all reads will still be done using &(handlep->handle).
     */
    globus_io_handle_t              *whandle; 

    /*
     * buffer space for constructing message headers
     */
    globus_byte_t *		    header;
    
    /* 
     * access to {cancel,send}_{head,tail} must be controlled 
     * by MessageQueuesLock.  it may be possible to place a individual
     * locks into this struct, but that may be hard because when we
     * add/remove nodes from _any_ tcp_miproto_t's list we have to 
     * update the single global variable TcpOutstandingSendReqs
     * which also has it's access controlled by MessageQueuesLock.
     * didn't want to figure out how to coordinate individual tcp_miproto_t
     * locks with MessageQueuesLock so i just used MessageQueuesLock for
     * everything.
     */
    struct tcpsendreq *cancel_head;
    struct tcpsendreq *cancel_tail;
    struct tcpsendreq *send_head;
    struct tcpsendreq *send_tail;
};

/*******************/
/* MPI proto stuff */
/*******************/

struct mpi_miproto_t
{
    char unique_session_string[MAXHOSTNAMELEN+32];
    int  rank;
};

#endif
